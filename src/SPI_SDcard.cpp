//SD Card setup in windows
//Format with 16kb or 32kb
//manually create PARAM.txt and save dummy data into it
//check boot info: block/cluster=64, root=0x2000, FAT=0x25A
//
//SD Card startup commands
//    cmd  ( arg,4byte, crc ) = response
//1:  CMD0 (      0x00, 0x95) = 0x01
//2:  CMD8 (0x000001AA, 0x87) = 0x01 + 0x000001AA
//3:  CMD58(       0x0,  any) = 0x01 + R3 - 32bit
//4a: CMD55(       0x0,  any) = 0x01 ok go 4b, 0x05 means old card
//4b:ACMD41(0x40000000,  any) = 0x00 ok, 0x01 go back 4a, 0x05 old card
//read 1 block
//    CMD17(      addr,  any) = 0x01 + 512 bytes
//	  after CMD17 0xFE needs to be looked for with dummy reads
//write 1 block
//    CMD24(      addr,  any) = 0x00, then send 0xFE + 512 bytes, read status token byte, and wait till response is non 0x00
//
//FAT32 basics
//0th read -> boot
//root is at 0x2000
//FAT1 is at 0x25A
//there is a value 2 offset between the cluster number value and the actually place of data
//example PARAM content is marked at number 6 in root but the content is found in number 4,
//example: MEAS1.txt start cluster offset is 7 -> 0x2000*512 + (7-2)*64*512 = 428000byte is data start
//
//data storing principles:
//  1st file is param.txt, 1st cluster, to hold possible parameters for the future
//  after param starts the measurement files
//  nameing convention: MEAS1.txt, MEAS2.txt,... MEASXXXX.txt
//  storing meas in a linear fasion, clusters follow each other, also means FAT is linear
//
//measurement taking principles:
//	header:
//		1st line: R for rate
//		2nd line: Px,I,D,Py,I,D,Pz,I,FFrx,y,FFdrx,y,satI,satPID,DTermC\n
//		3rd line: values
//		4th line: C for cacade
//		...
//		...
//		xth line: header for measured signals, example: sysTick,Graw,Gpt1,Gpt2,Accraw,Accpt1,Accpt2,Akf,Akfaccpt1\n
// 
//	1st meas is sysTick
//	every other measured value is separeted by comma(",")
//	after last measured value is a "\n"
//		x+th lines are the measurement values
//		
//		


#include "pch.h"
#include "SPI_SDcard.h"
#include "LED.h"
#include "sysTime.h"
#include "Controller.h"

//to switch header on hardware and unit test compilation
#ifdef UNIT_TEST

#include "../test/helper/support4Testing.h"
#include "../test/helper/support4Testing.hpp"

extern DummySerial SerialUSB;
extern Spi* SPI0;
extern Dmac* DMAC;

#else

#include "arduino.h"
#include "variant.h"

#endif

#define POSITION_BLOCK_PER_CLUSTER 0X00D
#define POSITION_RESERVED_SECTORS  0X00E
#define POSITION_NUMBER_OF_FATS    0X010
#define POSITION_SECTOR_PER_FAT    0X024
#define POSITION_FIRST_SECTOR      0x1C6

#define WRITE_BLOCK_TIMEOUT 3150000 //300ms
#define DATA_SAVE_DELAY 105000u //10ms
#define ROOT_FAT_WRITE_DELAY 525000u //50ms
#define POST_INIT_DELAY 105000  //10ms
#define INIT_TIMEOUT 10500 //1ms

//#define LOG_MEAS_DATA
//#define LOG_SD_INIT
//#define LOG_SD_WRITE
//#define LOG_SAVED_DATA


// Precomputed powers of 10
const uint32_t powOf10[] = {
	1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000, 10000000000
};

extern spi_st SPI;

SpiSDcard_st SDcard;
Meas2Card meas2Card;

void InitSDCard()
{
	//default global time value
	SDcard.globalDateAndTime.year = 21;
	SDcard.globalDateAndTime.month = 5;
	SDcard.globalDateAndTime.day = 15;
	SDcard.globalDateAndTime.hour = 12;
	SDcard.globalDateAndTime.min = 0;
	SDcard.globalDateAndTime.sec = 0;
}

void RunSdCard()
{
	rcSignals_st rcSig;

	getRcChannels(&rcSig);

	//if (getSysTick() > (3.2 * 10500000))
	//{
	//	rcSig.measurementSwitch = 1000;
	//}
	//if (getSysTick() > (3.5 * 10500000))
	//{
	//	rcSig.measurementSwitch = 2000;
	//}
	//if (getSysTick() > (10.5 * 10500000))
	//{
	//	rcSig.measurementSwitch = 1000;
	//}

    DetectReInitAndWrite(rcSig.Switch2Way);

	switch (SDcard.MainState)
	{
		case SD_PRE_INIT:
		{
			//SDcard.MainState = SetupSdCard();

			//break;
		}
		case SD_WAIT_4_MEASUREMENT:
		{
			if (rcSig.measurementSwitch > 1500)
			{
				addMeasHeader();

                LEDSDOn();

				SDcard.MainState = SD_MEASUREMENT_ONGOING;
				SDcard.measTickPrev = getSysTick();
			}

			break;
		}
		case SD_MEASUREMENT_ONGOING:
		{
			//save data every x ms
			if (getSysTick() - SDcard.measTickPrev >= DATA_SAVE_DELAY)
			{
				SDcard.measTickPrev = getSysTick();

#ifdef LOG_MEAS_DATA
                SerialUSB.print("measBufferCtr: "); SerialUSB.println(SDcard.measBufferCtr);
#endif

				saveMeasData();
			}

            //set to slow blink if buffer ctr at max
            if (checkCtrs() < 0)
            {
#ifdef LOG_MEAS_DATA
                SerialUSB.println("measBufferCtr limit reached!");
#endif
                LEDSDBlinkSlow();
            }

            //finish measurement
            if (rcSig.measurementSwitch < 1500 && rcSig.armStateSwitch < 1500)
            {
#ifdef LOG_MEAS_DATA
                SerialUSB.print("measBufferCtr: "); SerialUSB.println(SDcard.measBufferCtr);
                SerialUSB.print("measDataCtr before append: "); SerialUSB.println(SDcard.measDataCtr);
#endif
                
				//clean end of meas data
				StripToLastLineEnd();
				AppendTrailingZeros();

                SDcard.MainState = SD_POST_INIT;
                SDcard.SDInitStatus = SDINIT_CMD0; 
                SDcard.SDCommandState = SDCOMMAND_SEND;
                SDcard.acmd41TryCtr = 0;

                LEDSDOn();

                //disable gyro/acc interrupt while sd init and write
                DisableGyroAccInt();

                SDcard.postInitTick = getSysTick();

#ifdef LOG_MEAS_DATA
                for (uint8_t i = 0; i < SDcard.measBufferCtr; i++)
                {
                    for (uint16_t j = 0; j < 512; j++)
                    {
                        SerialUSB.print(char(SDcard.measBuffer[i].data[j]));
                    }
                }
#endif
            }

			break;
		}
        case SD_POST_INIT:
        {
            if (getSysTick() - SDcard.postInitTick > POST_INIT_DELAY)
            {
                SDcard.MainState = SetupSdCard();
            }

            break;
        }
        case SD_WRITE_DATA:
        {
            //TODO: use multi block write CMD25
            writeData(getSysTick());

            break;
        }
		case SD_WRITE_ROOT:
		{
			writeRoot(getSysTick());

			break;
		}
		case SD_WRITE_FAT:
		{
			writeFAT(getSysTick());

			break;
		}
		case SD_DO_NOTHING:
		{
			//do nothing
			break;
		}
		default:
		{

		}

	}
}

E_SDMainStates SetupSdCard(void)
{
	E_SDMainStates returnVal{ SD_POST_INIT };

	switch (SDcard.SDInitStatus)
	{
	case SDINIT_CMD0:
	{
		SDcard.SDInitStatus = CMD0();
		break;
	}
	case SDINIT_CMD8:
	{
		SDcard.SDInitStatus = CMD8();
		break;
	}
	case SDINIT_CMD58:
	{
		SDcard.SDInitStatus = CMD58();
		break;
	}
	case SDINIT_CMD55:
	{
		SDcard.SDInitStatus = CMD55();
		break;
	}
	case SDINIT_ACMD41:
	{
		SDcard.SDInitStatus = ACMD41();
		break;
	}
	case SDINIT_READ_00:    //0x0000
	{
		//SDReadStates l_readState = readBlock(0x00000000, rxBuffer);
		//
		//if (E_SDREAD_FINISHED == l_readState)
		//{
		//    bootSectorAddr  = uint32_t(rxBuffer[POSITION_FIRST_SECTOR]);
		//    bootSectorAddr |= uint32_t(rxBuffer[POSITION_FIRST_SECTOR + 1]) << 8;
		//    bootSectorAddr |= uint32_t(rxBuffer[POSITION_FIRST_SECTOR + 2]) << 16;
		//    bootSectorAddr |= uint32_t(rxBuffer[POSITION_FIRST_SECTOR + 3]) << 24;
		//
		//    //go and read boot
		//    SDReadState = E_SDREAD_START;
		//    SDInitStatus = E_SDINIT_READ_BOOT;
		//}
		//else if (E_SDREAD_FAILED == l_readState)
		//{
		//    SDInitStatus = E_SDINIT_FAILURE;
		//}
		//else
		//{
		//    //do nothing
		//}

		//due to changes in FAT32 boot, reading 00 is BOOT already
		SDcard.bootSectorAddr = 0x00000000;

		//go and read boot
		SDcard.SDReadState = SDREAD_START;
		SDcard.SDInitStatus = SDINIT_READ_BOOT;

		//modifiy sd card com logic
		//SDcard.canSDCardBeSuspended = false;

#ifdef LOG_SD_INIT 
		SerialUSB.println("READ_00 done");
#endif
		break;
	}
	case SDINIT_READ_BOOT:    //old: 0x2000, now: 0x0000
	{
		SDcard.SDInitStatus = readBoot();
		break;
	}
	case SDINIT_READ_ROOTDIR:    //old: 0x4000, now:2000
	{
		SDcard.SDInitStatus = readRoot();
		break;
	}
	case SDINIT_READ_FAT:     //old: 0x225C, now: 0x025A
	{
		SDcard.SDInitStatus = readFAT();
		break;
	}
	case SDINIT_SUCCESS:
	{
		returnVal = SD_WRITE_DATA;

        SDcard.sendBufferIndex = 0;
        prepSendingBuffer();
        SDcard.SDWriteState = SDWRITE_START;

		break;
	}
	case SDINIT_FAILURE:
	{
		LEDSDBlink();
        EnableGyroAccInt();
        
		returnVal = SD_DO_NOTHING;
		break;
	}
	default:
	{
		//do nothing
	}
	}

	return returnVal;
}

void triggerSDRxTx(volatile uint32_t* txBuff, volatile uint8_t* rxBuff, uint32_t ctr)
{
	if (ACTIVE != SPI.spiActivityGyro && ACTIVE != SPI.spiActivityAcc && INACTIVE == SDcard.spiActivitySDCard)
	{
		SDcard.spiActivitySDCard = ACTIVE;
		SpiDmaTxRx(txBuff, rxBuff, ctr, DMAC_CHANNEL_SDCARD);
	}
	else if (ACTIVE == SDcard.spiActivitySDCard)
	{
		//already active, should not reach, do nothing
	}
	else
	{
		SDcard.spiActivitySDCard = PENDING;
		SDcard.nextTxBuffer = txBuff;
		SDcard.nextRxBuffer = rxBuff;
		SDcard.nextCtr = ctr;
	}
}

void intSafeTriggerSDRxTx(volatile uint32_t* txBuff, volatile uint8_t* rxBuff, uint32_t ctr)
{
	//disable DMAC interrupt
	NVIC_DisableIRQ(PIOA_IRQn);
	NVIC_DisableIRQ(PIOC_IRQn);
	NVIC_DisableIRQ(DMAC_IRQn);
  
	triggerSDRxTx(txBuff, rxBuff, ctr);

	//reenable DMAC interrupt
	NVIC_EnableIRQ(DMAC_IRQn);
	NVIC_EnableIRQ(PIOC_IRQn);
	NVIC_EnableIRQ(PIOA_IRQn);
}

E_SDInitStates CMD0(void)
{
	E_SDInitStates returnValCMD0 = SDINIT_CMD0;

	if (SDCOMMAND_SEND == SDcard.SDCommandState)
	{
		SDcard.SdCtr = 0;
		//clock sync data
		for (SDcard.SdCtr = 0; SDcard.SdCtr < 80; SDcard.SdCtr++)
		{
			SDcard.SdTx[SDcard.SdCtr] = 0xFF | SPI_TDR_PCS(CS_SDCARD);
		}

		SDcard.SdTx[SDcard.SdCtr++] = 0x40 | SPI_TDR_PCS(CS_SDCARD); //CMD0
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD); //arg 1-4
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0x95 | SPI_TDR_PCS(CS_SDCARD); //CRC
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //wait
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //should receive answer

		intSafeTriggerSDRxTx(SDcard.SdTx, SDcard.SdRx, SDcard.SdCtr);

		//set command state to waiting
		SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	}
	else
	{
		if (INACTIVE == SDcard.spiActivitySDCard)
		{
			if (1 == SDcard.SdRx[SDcard.SdCtr - 1])
			{
				SDcard.SDCommandState = SDCOMMAND_SEND;
				returnValCMD0 = SDINIT_CMD8;
#ifdef LOG_SD_INIT 
				SerialUSB.println("CMD0 done");
#endif
			}
			else
			{
				returnValCMD0 = SDINIT_FAILURE;
                
#ifdef LOG_SD_INIT 
				SerialUSB.println("CMD0 failed");
#endif
			}

		}
	}

	return returnValCMD0;
}

E_SDInitStates CMD8(void)
{
	E_SDInitStates returnValCMD8 = SDINIT_CMD8;

	if (SDCOMMAND_SEND == SDcard.SDCommandState)
	{
		SDcard.SdCtr = 0;
		SDcard.SdTx[SDcard.SdCtr++] = 0x48 | SPI_TDR_PCS(CS_SDCARD); //CMD8
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD); //arg 1-4
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0x01 | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0xAA | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0x87 | SPI_TDR_PCS(CS_SDCARD); //CRC
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //wait
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //should receive answer
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //should receive answer
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //should receive answer
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //should receive answer
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //should receive answer

		intSafeTriggerSDRxTx(SDcard.SdTx, SDcard.SdRx, SDcard.SdCtr);
		//set command state to waiting
		SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	}
	else
	{
		if (INACTIVE == SDcard.spiActivitySDCard)
		{
			if (1 == SDcard.SdRx[SDcard.SdCtr - 5])
			{
				SDcard.SDCommandState = SDCOMMAND_SEND;
				returnValCMD8 = SDINIT_CMD58;
#ifdef LOG_SD_INIT 
				SerialUSB.println("CMD8 done");
#endif
			}
			else
			{
				returnValCMD8 = SDINIT_FAILURE;
#ifdef LOG_SD_INIT 
				SerialUSB.println("CMD8 failed");
#endif
			}
		}
	}

	return returnValCMD8;
}

E_SDInitStates CMD58(void)
{
	E_SDInitStates returnValCMD58 = SDINIT_CMD58;

	if (SDCOMMAND_SEND == SDcard.SDCommandState)
	{
		SDcard.SdCtr = 0;
		SDcard.SdTx[SDcard.SdCtr++] = 0x7A | SPI_TDR_PCS(CS_SDCARD); //CMD58
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD); //arg 1-4
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD); //CRC
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //wait
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //should receive answer
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //should receive answer
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //should receive answer
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //should receive answer
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //should receive answer

		intSafeTriggerSDRxTx(SDcard.SdTx, SDcard.SdRx, SDcard.SdCtr);
		//set command state to waiting
		SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	}
	else
	{
		if (INACTIVE == SDcard.spiActivitySDCard)
		{
			if (1 == SDcard.SdRx[SDcard.SdCtr - 5])
			{
				SDcard.SDCommandState = SDCOMMAND_SEND;
				returnValCMD58 = SDINIT_CMD55;
#ifdef LOG_SD_INIT 
				SerialUSB.println("CMD58 done");
#endif
			}
			else
			{
				returnValCMD58 = SDINIT_FAILURE;
#ifdef LOG_SD_INIT 
				SerialUSB.println("CMD58 failed");
#endif
			}
		}
	}

	return returnValCMD58;
}

E_SDInitStates CMD55(void)
{
	E_SDInitStates returnValCMD55 = SDINIT_CMD55;

	if (SDCOMMAND_SEND == SDcard.SDCommandState)
	{
		SDcard.SdCtr = 0;
		SDcard.SdTx[SDcard.SdCtr++] = 0x77 | SPI_TDR_PCS(CS_SDCARD); //CMD55
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD); //arg 1-4
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD); //CRC
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //wait
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //should receive answer here

		intSafeTriggerSDRxTx(SDcard.SdTx, SDcard.SdRx, SDcard.SdCtr);
		//set command state to waiting
		SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
        //set timeout time
        SDcard.timeoutTick = getSysTick();
	}
	else
	{
		if (INACTIVE == SDcard.spiActivitySDCard)
		{
            if (1 == SDcard.SdRx[SDcard.SdCtr - 1])
            {
                SDcard.SDCommandState = SDCOMMAND_SEND;
                returnValCMD55 = SDINIT_ACMD41;
            }
            else if (getSysTick() - SDcard.timeoutTick > INIT_TIMEOUT)
            {
                returnValCMD55 = SDINIT_FAILURE;
            }
            else
            {
                //send dummy to check for response
                SDcard.SdCtr = 0;
                SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //response
                intSafeTriggerSDRxTx(SDcard.SdTx, SDcard.SdRx, SDcard.SdCtr);


                //SDcard.SDCommandState = SDCOMMAND_SEND;
            }

#ifdef LOG_SD_INIT 
			if (returnValCMD55 == SDINIT_ACMD41)
			{
				SerialUSB.println("CMD55 done");
			}
            else if (returnValCMD55 == SDINIT_CMD55)
            {
                SerialUSB.println("CMD55 waiting");
            }
			else
			{
				SerialUSB.println("CMD55 failed");
			}
#endif
		}
	}

	return returnValCMD55;
}

E_SDInitStates ACMD41(void)
{
	E_SDInitStates returnValACMD41 = SDINIT_ACMD41;


	if (SDCOMMAND_SEND == SDcard.SDCommandState)
	{
		SDcard.SdCtr = 0;
		SDcard.SdTx[SDcard.SdCtr++] = 0x69 | SPI_TDR_PCS(CS_SDCARD); //ACMD41
		SDcard.SdTx[SDcard.SdCtr++] = 0x40 | SPI_TDR_PCS(CS_SDCARD); //arg 1-4
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0x00 | SPI_TDR_PCS(CS_SDCARD); //CRC
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //wait
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //should receive answer
        SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //space
        SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //space
        SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //space
        SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //space
        SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //space

		intSafeTriggerSDRxTx(SDcard.SdTx, SDcard.SdRx, SDcard.SdCtr);
		//set command state to waiting
		SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	}
	else
	{
		if (INACTIVE == SDcard.spiActivitySDCard)
		{
            if (0 == SDcard.SdRx[SDcard.SdCtr - 6])
            {
                SDcard.SDCommandState = SDCOMMAND_SEND;
                returnValACMD41 = SDINIT_READ_00;
            }
            else
            {
				//instead go back to 55 and retry
				SDcard.SDCommandState = SDCOMMAND_SEND;
                returnValACMD41 = SDINIT_CMD55;
            }

            //stop if in 100x none was successful
            if (SDcard.acmd41TryCtr++ > 100)
            {
                returnValACMD41 = SDINIT_FAILURE;
            }

#ifdef LOG_SD_INIT 
			if (returnValACMD41 == SDINIT_READ_00)
			{
				SerialUSB.println("ACMD41 done");
			}
            else if(returnValACMD41 == SDINIT_FAILURE)
            {
                SerialUSB.println("ACMD41 failed 100x, stopping");
            }
			else
			{
				SerialUSB.println("ACMD41 failed, back to CMD55");
			}
#endif
		}
	}

	return returnValACMD41;
}

E_SDInitStates readBoot(void)
{
	E_SDInitStates returnValBootState{ SDINIT_READ_BOOT };

	E_SDReadStates readStateBoot = readBlock(SDcard.bootSectorAddr, SDcard.SdRx);

	if (SDREAD_FINISHED == readStateBoot)
	{
		uint16_t reservedSectors = 0;
		uint8_t numberOfFATs = 0;
		uint32_t sectorsPerFAT = 0;

		//assemble block per cluster value
		SDcard.blockPerCluster = uint32_t(SDcard.SdRx[POSITION_BLOCK_PER_CLUSTER]);
		//assemble receved sector value
		reservedSectors = uint16_t(SDcard.SdRx[POSITION_RESERVED_SECTORS]);
		reservedSectors |= uint16_t(SDcard.SdRx[POSITION_RESERVED_SECTORS + 1]) << 8;
		//assemble number of FATs value
		numberOfFATs = SDcard.SdRx[POSITION_NUMBER_OF_FATS];
		//assemble sectors per FAT value
		sectorsPerFAT = uint32_t(SDcard.SdRx[POSITION_SECTOR_PER_FAT]);
		sectorsPerFAT |= uint32_t(SDcard.SdRx[POSITION_SECTOR_PER_FAT + 1]) << 8;
		sectorsPerFAT |= uint32_t(SDcard.SdRx[POSITION_SECTOR_PER_FAT + 2]) << 16;
		sectorsPerFAT |= uint32_t(SDcard.SdRx[POSITION_SECTOR_PER_FAT + 3]) << 24;
		//calculate FAT1 and root addresses
		SDcard.FAT1Addr = SDcard.bootSectorAddr + reservedSectors;
		SDcard.rootDirAddr = SDcard.bootSectorAddr + reservedSectors + sectorsPerFAT * numberOfFATs;

#ifdef LOG_SD_INIT 
        SerialUSB.println("Boot data:");
        SerialUSB.print("block/cluster  : "); SerialUSB.println(SDcard.blockPerCluster); //64
        SerialUSB.print("reserved sector: "); SerialUSB.println(reservedSectors); //602
        SerialUSB.print("number of FATs : "); SerialUSB.println(numberOfFATs);    //2
        SerialUSB.print("sectors/FAT    : "); SerialUSB.println(sectorsPerFAT);   //3795
        SerialUSB.print("FAT1 address   : 0x"); SerialUSB.println(SDcard.FAT1Addr, HEX);   //0x25A
        SerialUSB.print("RootDir address: 0x"); SerialUSB.println(SDcard.rootDirAddr, HEX);//0x2000
#endif

		if (0x2000 != SDcard.rootDirAddr && 0x25A != SDcard.FAT1Addr)
		{
			returnValBootState = SDINIT_FAILURE;
#ifdef LOG_SD_INIT 
			SerialUSB.println("readBoot failed, wrong root and FAT addresses");
#endif
		}
		else
		{
			//go and read root dir
			SDcard.SDReadState = SDREAD_START;
			returnValBootState = SDINIT_READ_ROOTDIR;

#ifdef LOG_SD_INIT 
			SerialUSB.println("readBoot done");
#endif
		}
	}
	else if (SDREAD_FAILED == readStateBoot)
	{
		returnValBootState = SDINIT_FAILURE;
        LEDSDBlink();
#ifdef LOG_SD_INIT 
		SerialUSB.println("readBoot failed");
#endif
	}
	else
	{
		//SerialUSB.println("-");
		//do nothing
	}

	return returnValBootState;
}

E_SDInitStates readRoot(void)
{
	E_SDInitStates returnValRootState{ SDINIT_READ_ROOTDIR };

	E_SDReadStates readStateRoot = readBlock(SDcard.rootDirAddr + SDcard.rootDirEmptyBlockNumber, SDcard.rootDirInfo_8b);

	//SerialUSB.print(l_readStateRoot);
	//SerialUSB.print('\t');
	//SerialUSB.println(SDcard.rootDirEmptyBlockNumber);

	if (SDREAD_FINISHED == readStateRoot)
	{
		//find 1st 0x00 entry within a block
		bool emptySlotFound = true;

		while (!(0x00 == SDcard.rootDirInfo_8b[SDcard.rootDirEmptySlotNumber * 32]))
		{
			SDcard.lastFile = getFileInfo(&SDcard.rootDirInfo_8b[SDcard.rootDirEmptySlotNumber * 32]);

			//SerialUSB.println(rootDirEmptySlotNumber); //SerialUSB.print('\t');
			//SerialUSB.print("iter file: "); printFileInfo(&lastFile);

			SDcard.rootDirEmptySlotNumber++;
			//check if at the end of block
			if (15 < SDcard.rootDirEmptySlotNumber)
			{
				//set back slot counter to zero (there seems to be a bug being a local and not reseting)
				SDcard.rootDirEmptySlotNumber = 0;

				//increment block number and read new block
				SDcard.rootDirEmptyBlockNumber++;

				//go and read new root dir
				SDcard.SDReadState = SDREAD_START;

				//set slot found false so it skips the next step
				emptySlotFound = false;

				break;
			}
		}

		//if empty slot found
		if (true == emptySlotFound)
		{
			//SerialUSB.print("RootDir empty block: "); SerialUSB.println(rootDirEmptyBlockNumber);
			//SerialUSB.print("RootDir empty slot: "); SerialUSB.println(rootDirEmptySlotNumber);

			//calculate the FAT offset value of lastFile
			SDcard.FATBlockOffset = SDcard.lastFile.clusters[SDcard.lastFile.numberOfClusters - 1] / 128;

			//SerialUSB.print("FATBlockOffset: "); SerialUSB.println(FATBlockOffset);


			//copy rootDir info from 8bit to 32bit and set SD card CS, 0th is 0xFE(start token)
			for (uint16_t i = 0; i < 516; i++)
			{
				SDcard.rootDirInfo[i+1] = uint32_t(SDcard.rootDirInfo_8b[i]);
			}
			appendCsSdCard(SDcard.rootDirInfo, 516);

			//go and read FAT
			SDcard.SDReadState = SDREAD_START;
			returnValRootState = SDINIT_READ_FAT;
#ifdef LOG_SD_INIT 
			SerialUSB.print("readRoot done, rootDirEmptySlotNumber: ");
			SerialUSB.println(SDcard.rootDirEmptyBlockNumber);
#endif
		}
		else
		{
			//SerialUSB.println("Failed to find 1st empty slot!!!");
			//SerialUSB.println("Goto next rootDir block");
		}
	}
	else if (SDREAD_FAILED == readStateRoot)
	{
		returnValRootState = SDINIT_FAILURE;
        LEDSDBlink();
#ifdef LOG_SD_INIT 
		SerialUSB.println("readRoot failed");
#endif
	}
	else
	{
		//do nothing
	}

	return returnValRootState;
}

E_SDInitStates readFAT(void)
{
	E_SDInitStates returnValFATState{ SDINIT_READ_FAT };

	E_SDReadStates readStateFAT = readBlock(SDcard.FAT1Addr + SDcard.FATBlockOffset, SDcard.FAT1Info_8b);

	if (SDREAD_FINISHED == readStateFAT)
	{
		if (true == getAllFileClusters(SDcard.FAT1Info_8b, &SDcard.lastFile))
		{
			//copy FAT1Info info from 8bit to 32bit and set SD card CS
			for (uint16_t i = 0; i < 516; i++)
			{
				SDcard.FAT1Info[i+1] = uint32_t(SDcard.FAT1Info_8b[i]);
			}
			appendCsSdCard(SDcard.FAT1Info, 516);

			//set file name
			if (SDcard.lastFile.name[0] == 'M' &&
				SDcard.lastFile.name[1] == 'E' &&
				SDcard.lastFile.name[2] == 'A' &&
				SDcard.lastFile.name[3] == 'S')
			{
				//set number in name
				SDcard.newFile.numberInName = SDcard.lastFile.numberInName + 1;
			}
			else if (SDcard.lastFile.name[0] == 'P' &&	//case if SD card cleared and only PARAM is on it
				SDcard.lastFile.name[1] == 'A' &&
				SDcard.lastFile.name[2] == 'R' &&
				SDcard.lastFile.name[3] == 'A' &&
				SDcard.lastFile.name[4] == 'M')
			{
				//set number in name
				SDcard.newFile.numberInName = 1;
			}
			else
			{
                // if deleted file or failure
			    SerialUSB.println("readFAT, did not find the last file");
                return SDINIT_FAILURE;
			}

			//set name
			SDcard.newFile.name[0] = 'M';
			SDcard.newFile.name[1] = 'E';
			SDcard.newFile.name[2] = 'A';
			SDcard.newFile.name[3] = 'S';
			//set cluster info
			SDcard.newFile.clusters[0] = SDcard.lastFile.clusters[SDcard.lastFile.numberOfClusters - 1] + 1;
			SDcard.newFile.numberOfClusters = 1;
			//set size to zero
			SDcard.newFile.size = 0;

			returnValFATState = SDINIT_SUCCESS;

#ifdef LOG_SD_INIT 
			SerialUSB.println("readFAT done");
			SerialUSB.print("FATBlockOffset: "); SerialUSB.println(SDcard.FATBlockOffset);
			SerialUSB.print("lastfile: "); printFileInfo(&SDcard.lastFile);
			SerialUSB.print("newfile: "); printFileInfo(&SDcard.newFile);
#endif
		}
		else
		{
			//if last slot is not yet found read the next block
			SDcard.SDReadState = SDREAD_START;
#ifdef LOG_SD_INIT 
			SerialUSB.println("readFAT slot not yet found, read next block");
#endif
		}
	}
	else if (SDREAD_FAILED == readStateFAT)
	{
		returnValFATState = SDINIT_FAILURE;
        LEDSDBlink();
#ifdef LOG_SD_INIT 
		SerialUSB.println("readFAT failed");
#endif
	}
	else
	{
		//do nothing
	}

	return returnValFATState;
}

E_SDReadStates readBlock(uint32_t blockaddr, volatile uint8_t* rxBuf)
{
	switch (SDcard.SDReadState)
	{
		case SDREAD_START:
		{
			SDcard.SdCtr = 0;
			SDcard.SdTx[SDcard.SdCtr++] = 0x51 | SPI_TDR_PCS(CS_SDCARD); //CMD17
			SDcard.SdTx[SDcard.SdCtr++] = ((blockaddr & 0xFF000000) >> 24) | SPI_TDR_PCS(CS_SDCARD); //arg 1-4
			SDcard.SdTx[SDcard.SdCtr++] = ((blockaddr & 0x00FF0000) >> 16) | SPI_TDR_PCS(CS_SDCARD);
			SDcard.SdTx[SDcard.SdCtr++] = ((blockaddr & 0x0000FF00) >> 8) | SPI_TDR_PCS(CS_SDCARD);
			SDcard.SdTx[SDcard.SdCtr++] = ((blockaddr & 0x000000FF)) | SPI_TDR_PCS(CS_SDCARD);
			SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //CRC
			SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //wait
			SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //response maybe here
			//send read command
			intSafeTriggerSDRxTx(SDcard.SdTx, SDcard.SdRx, SDcard.SdCtr);
			//go to wait state
			SDcard.SDReadState = SDREAD_WAIT_RESPONSE;

			break;
		}
		case SDREAD_WAIT_RESPONSE:
		{
			if (INACTIVE == SDcard.spiActivitySDCard)
			{
				if (0x00 == SDcard.SdRx[SDcard.SdCtr-1])
				{
					//send dummy to check for 0xFE
					SDcard.SdCtr = 0;
					SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //response
					intSafeTriggerSDRxTx(SDcard.SdTx, SDcard.SdRx, SDcard.SdCtr);
					//go to wait state
					SDcard.SDReadState = SDREAD_WAIT_FE;
                    //set timeout time
                    SDcard.timeoutTick = getSysTick();
				}
				else
				{
					SDcard.SDReadState = SDREAD_FAILED;
                    LEDSDBlink();
#ifdef LOG_SD_INIT 
					SerialUSB.println("readBlock failed");
#endif
				}
			}
			break;
		}
		case SDREAD_WAIT_FE:
		{
			if (INACTIVE == SDcard.spiActivitySDCard)
			{
				//if FE is received go for read data
				if (0xFE == (SDcard.SdRx[0] & 0xFF))  //to reset rx full flag
				{
					//512 data + 2 CRC
					intSafeTriggerSDRxTx(NULL, &rxBuf[0], 514);
					SDcard.SDReadState = SDREAD_WAIT_DATA;
				}
                else if (getSysTick() - SDcard.timeoutTick > INIT_TIMEOUT)
                {
                    SDcard.SDReadState = SDREAD_FAILED;
                }
				else
				{
					//send dummy to check for response
					SDcard.SdCtr = 0;
					SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //response
					intSafeTriggerSDRxTx(SDcard.SdTx, SDcard.SdRx, SDcard.SdCtr);
				}
			}
			break;
		}
		case SDREAD_WAIT_DATA:
		{
			if (INACTIVE == SDcard.spiActivitySDCard)
			{
				//SerialUSB.println("Readblock finished!!!");
				SDcard.SDReadState = SDREAD_FINISHED;
#ifdef LOG_SD_INIT 
				SerialUSB.println("readBlock done");
#endif
			}
			break;
		}
		case SDREAD_FINISHED:
		{

			break;
		}
		case SDREAD_FAILED:
		{

			break;
		}
		default:
		{
			//do nothing
		}
	}

	return SDcard.SDReadState;
}

void SDWriteWaitResponse(void)
{
	if (0x00 == SDcard.SdRx[7])
	{
		//512 data + 2byte CRC + status token + filler (-1 due to ctr=0 is not sent)
  	    SDcard.SdCtr = 516;
		triggerSDRxTx(SDcard.nextTxBuffer4Data, SDcard.SdRx, SDcard.SdCtr);
		SDcard.SDWriteState = SDWRITE_WAIT_DATA;

        SDcard.writeStartTick = getSysTick();
#ifdef LOG_SD_WRITE 
        SerialUSB.println("a");
#endif
	}
	else
	{
		SDcard.SDWriteState = SDWRITE_FAILED;
#ifdef LOG_SD_WRITE 
		SerialUSB.println("writeBlock failed, wrong cmd24 response");
#endif
	}
}

void SDWriteWaitData(void)
{
    if (0b0100 == (SDcard.SdRx[SDcard.SdCtr - 1] & 0b1110))
    {
        SDcard.writeStartTick = getSysTick();

        SDcard.SdTx[0] = 0xFF | SPI_TDR_PCS(CS_SDCARD);
        triggerSDRxTx(SDcard.SdTx, SDcard.SdRx, 1);
        SDcard.SDWriteState = SDWRITE_WAIT_WRITE_FINISH;
#ifdef LOG_SD_WRITE 
        SerialUSB.println("b");
#endif
    }
    else
    {
        //timeout check
        if (getSysTick() - SDcard.writeStartTick > WRITE_BLOCK_TIMEOUT)
        {
            SDcard.SDWriteState = SDWRITE_FAILED;
#ifdef LOG_SD_WRITE
            SerialUSB.print("writeBlock failed, wrong data end flag or timed out: ");
            SerialUSB.println(SDcard.SdRx[SDcard.SdCtr - 1]);
#endif
        }
        //send new dummy byte
        SDcard.SdTx[0] = 0xFF | SPI_TDR_PCS(CS_SDCARD);
        SDcard.SdCtr = 1;
        triggerSDRxTx(SDcard.SdTx, SDcard.SdRx, SDcard.SdCtr);
    }
}

E_SDWriteStates writeBlock(uint32_t blockOffset, volatile uint32_t* txBuf)
{
	switch (SDcard.SDWriteState)
	{
	case SDWRITE_START:
	{
		if (SDcard.FAT1Addr > blockOffset)
		{
#ifdef LOG_SD_WRITE 
            SerialUSB.print("writeBlock fail: offset smaller than FAT1");
#endif
			return SDWRITE_FAILED;
		}

#ifdef LOG_SD_WRITE 
        SerialUSB.print("writeBlock to offset: "); SerialUSB.println(blockOffset);
#endif

		SDcard.SdCtr = 0;
		//CMD24
		SDcard.SdTx[SDcard.SdCtr++] = 0x58 | SPI_TDR_PCS(CS_SDCARD); //CMD24
		SDcard.SdTx[SDcard.SdCtr++] = ((blockOffset & 0xFF000000) >> 24) | SPI_TDR_PCS(CS_SDCARD); //arg 1-4
		SDcard.SdTx[SDcard.SdCtr++] = ((blockOffset & 0x00FF0000) >> 16) | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = ((blockOffset & 0x0000FF00) >> 8) | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = ((blockOffset & 0x000000FF)) | SPI_TDR_PCS(CS_SDCARD);
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //CRC
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //wait
		SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //answer
		//go to wait state
		SDcard.SDWriteState = SDWRITE_WAIT_RESPONSE;
		//set and save next tx data buffer
        SDcard.nextTxBuffer4Data = txBuf;
		SDcard.nextTxBuffer4Data[513] = 0xFF;
		SDcard.nextTxBuffer4Data[514] = 0xFF;
		SDcard.nextTxBuffer4Data[515] = 0xFF;
		appendCsSdCard(SDcard.nextTxBuffer4Data, 516);
		//send write command
		intSafeTriggerSDRxTx(SDcard.SdTx, SDcard.SdRx, SDcard.SdCtr);

		break;
	}
//	case SDWRITE_WAIT_RESPONSE: -> HANDLED IN DMAC_Handler with SDWriteWaitResponse
//	case SDWRITE_WAIT_DATA: -> HANDLED IN DMAC_Handler with SDWriteWaitData
	case SDWRITE_WAIT_WRITE_FINISH:
	{
		if (INACTIVE == SDcard.spiActivitySDCard)
		{
			if (SDcard.SdRx[0] > 0)
			{
				SDcard.SDWriteState = SDWRITE_FINISHED;
#ifdef LOG_SD_WRITE 
				SerialUSB.println("writeBlock done");
#endif
			}
			else
			{
				//timeout check
				if (getSysTick() - SDcard.writeStartTick > WRITE_BLOCK_TIMEOUT)
				{
					SDcard.SDWriteState = SDWRITE_FAILED;
#ifdef LOG_SD_WRITE 
					SerialUSB.println("writeBlock failed, timed out");
#endif
					break;
				}
				//send new dummy byte
				SDcard.SdTx[0] = 0xFF | SPI_TDR_PCS(CS_SDCARD);
				intSafeTriggerSDRxTx(SDcard.SdTx, SDcard.SdRx, 1);
			}
		}

		break;
	}
	case SDWRITE_FINISHED:
	{

		break;
	}
	case SDWRITE_FAILED:
	{
		break;
	}
	default:
	{
		//do nothing
	}
	}

	return SDcard.SDWriteState;
}

fileInfo getFileInfo(volatile uint8_t* rawFileData)
{
	fileInfo fileInfo;

	//if file is not deleted
	if (0xE5 != rawFileData[0])
	{
		//assemble name
		uint8_t out = 0;
		uint8_t in;
		for (in = 0; in < 9; in++)
		{
			//if capital chars
			if ('A' <= rawFileData[in] && rawFileData[in] <= 'Z')
			{
				fileInfo.name[out++] = char(rawFileData[in]);
			}
			else
			{
				break;
			}
		}
		//fill up remaining chars
		for (uint8_t i = out; i < 11; i++)
		{
			fileInfo.name[out++] = 0;
		}

		//assemble nameNumber
		//count the number of number-chars
		uint8_t numberOfNumbers = 0;
		for (uint8_t itr = in; itr < 9; itr++)
		{
			//if numbers
			if ('0' <= rawFileData[itr] && rawFileData[itr] <= '9')
			{
				numberOfNumbers++;
			}
			else if ('_' == rawFileData[itr])
			{
				//do nothing, only inc in
				in++;
			}
			else
			{
				break;
			}
		}
		//if there was no '_', it should hold 11<11 which should prevent entering into nameNumber
		fileInfo.numberInName = 0;
		for (uint8_t exitNum = in + numberOfNumbers; in < exitNum; in++)
		{
			fileInfo.numberInName += (rawFileData[in] - '0') * pow(10, --numberOfNumbers);
		}

		//assemble clusters[0]
		fileInfo.clusters[0] = uint32_t(rawFileData[26]);
		fileInfo.clusters[0] |= uint32_t(rawFileData[27]) << 8;
		fileInfo.clusters[0] |= uint32_t(rawFileData[20]) << 16;
		fileInfo.clusters[0] |= uint32_t(rawFileData[21]) << 24;
		//assigning number of cluster with 1
		fileInfo.numberOfClusters = 1;
		//assemble size
		fileInfo.size = uint32_t(rawFileData[28]);
		fileInfo.size |= uint32_t(rawFileData[29]) << 8;
		fileInfo.size |= uint32_t(rawFileData[30]) << 16;
		fileInfo.size |= uint32_t(rawFileData[31]) << 24;
    //reset blocksize
		fileInfo.blockCount = 0;
	}
	else
	{
		fileInfo.name[0] = 'D';
		fileInfo.name[1] = 'E';
		fileInfo.name[2] = 'L';
		fileInfo.name[3] = 'E';
		fileInfo.name[4] = 'T';
		fileInfo.name[5] = 'E';
		fileInfo.name[6] = 'D';
		fileInfo.name[7] = 0;
		fileInfo.name[8] = 0;
		fileInfo.name[9] = 0;
		fileInfo.name[10] = 0;
		fileInfo.numberInName = 0xFFFF;
		fileInfo.clusters[0] = 0;
		fileInfo.size = 0;
		fileInfo.blockCount = 0;
	}

	return fileInfo;
}

bool getAllFileClusters(volatile uint8_t* rawFileData, fileInfo* fileInfo)
{
	//return if number of cluster is 0
	if (0 == fileInfo->numberOfClusters || NULL == rawFileData) return true;

	bool allClusterFound = false;
	uint32_t clusterPos = fileInfo->clusters[fileInfo->numberOfClusters - 1] % 128; //at 1st entry numberOfClusters should only be 1
	uint32_t clusterVal = 0;

	do
	{
		//assemble cluster value
		clusterVal = rawFileData[4 * clusterPos];
		clusterVal |= rawFileData[4 * clusterPos + 1] << 8;
		clusterVal |= rawFileData[4 * clusterPos + 2] << 16;
		clusterVal |= rawFileData[4 * clusterPos + 3] << 24;

		//SerialUSB.print("l_clusterPos: "); SerialUSB.print(l_clusterPos);SerialUSB.print("    l_clusterVal: "); SerialUSB.println(l_clusterVal);

		//check if this is end cluster
		if (0x0FFFFFFF <= clusterVal)
		{
			allClusterFound = true;

			//if cluster ends at the very end of the block
			//if (127 == l_clusterPos % 128)
			//{
			//    FATBlockOffset++;
			//    return false;
			//}
		}
		else
		{
			//save cluster value into the next position and increment counter
			fileInfo->clusters[fileInfo->numberOfClusters] = clusterVal;
			fileInfo->numberOfClusters++;

			//check if this round's cluster positon is the last of the current block
			if (127 == clusterPos % 128)
			{
				SDcard.FATBlockOffset++;
				return false;
			}

			//assign next cluster position inside the block
			clusterPos = clusterVal % 128;
		}
	} while (!allClusterFound);

	return allClusterFound;
}

void setFileTime(volatile uint32_t* block, uint64_t sysTick)
{

	////set dummy creat time, 15-11 hours, 10-5 minute, 4-0 second, 12-00-00 = 01100-000000-00000 = 01100000-00000000
	//block[SDcard.rootDirEmptySlotNumber * 32 + 14] = 0b00000000;
	//block[SDcard.rootDirEmptySlotNumber * 32 + 15] = 0b01100000;
	////set dummy create date, 15-9 year, 8-5 month, 4-0 day, 2021.05.15 = 41-5-15 ?= 0101001-0101-01111 = 01010010-10101111
	//block[SDcard.rootDirEmptySlotNumber * 32 + 16] = 0b10101111;
	//block[SDcard.rootDirEmptySlotNumber * 32 + 17] = 0b01010010;
	////set dummy last access date, same as create date
	//block[SDcard.rootDirEmptySlotNumber * 32 + 18] = 0b10101111;
	//block[SDcard.rootDirEmptySlotNumber * 32 + 19] = 0b01010010;
	////high byte set is later
	////set dummy modify time, same as create time
	//block[SDcard.rootDirEmptySlotNumber * 32 + 22] = 0b00000000;
	//block[SDcard.rootDirEmptySlotNumber * 32 + 23] = 0b01100000; 
	////set dummy last modify time, same as create date
	//block[SDcard.rootDirEmptySlotNumber * 32 + 24] = 0b10101111;
	//block[SDcard.rootDirEmptySlotNumber * 32 + 25] = 0b01010010;


	uint64_t elapsedTimeSinceSync = (sysTick - SDcard.sysTickAtGlobalTick) / 10500000;
	date currentGlobalTime{};
	uint8_t intermidiateVal{ 0 };

	intermidiateVal = SDcard.globalDateAndTime.sec + (elapsedTimeSinceSync % 60);
	currentGlobalTime.sec = (intermidiateVal % 60) / 2;	// 1/2 due to standard
	intermidiateVal /= 60;
	intermidiateVal += SDcard.globalDateAndTime.min + ((elapsedTimeSinceSync / 60) % 60);
	currentGlobalTime.min = intermidiateVal % 60;
	intermidiateVal /= 60;
	intermidiateVal += SDcard.globalDateAndTime.hour + ((elapsedTimeSinceSync / 3600) % 60);
	currentGlobalTime.hour = intermidiateVal % 60;
	//just copy the rest, not expected to go over
	currentGlobalTime.day = SDcard.globalDateAndTime.day;
	currentGlobalTime.month = SDcard.globalDateAndTime.month;
	currentGlobalTime.year = SDcard.globalDateAndTime.year + 20;	// +20 due to standard

	//second
	block[SDcard.rootDirEmptySlotNumber * 32 + 14] = currentGlobalTime.sec & 0x1F;
	//minute
	block[SDcard.rootDirEmptySlotNumber * 32 + 14] |= (currentGlobalTime.min & 0x07) << 5;
	block[SDcard.rootDirEmptySlotNumber * 32 + 15] = (currentGlobalTime.min & 0x38) >> 3;
	//hour
	block[SDcard.rootDirEmptySlotNumber * 32 + 15] |= (currentGlobalTime.hour & 0x1F) << 3;
	//day
	block[SDcard.rootDirEmptySlotNumber * 32 + 16] = currentGlobalTime.day & 0x1F;
	//month
	block[SDcard.rootDirEmptySlotNumber * 32 + 16] |= (currentGlobalTime.month & 0x07) << 5;
	block[SDcard.rootDirEmptySlotNumber * 32 + 17] = (currentGlobalTime.month & 0x08) >> 3;
	//year
	block[SDcard.rootDirEmptySlotNumber * 32 + 17] |= (currentGlobalTime.year & 0x7F) << 1;


	//set dummy last access date, same as create date
	block[SDcard.rootDirEmptySlotNumber * 32 + 18] = block[SDcard.rootDirEmptySlotNumber * 32 + 16];
	block[SDcard.rootDirEmptySlotNumber * 32 + 19] = block[SDcard.rootDirEmptySlotNumber * 32 + 17];
	//high byte set is later
	//set dummy modify time, same as create time
	block[SDcard.rootDirEmptySlotNumber * 32 + 22] = block[SDcard.rootDirEmptySlotNumber * 32 + 14];
	block[SDcard.rootDirEmptySlotNumber * 32 + 23] = block[SDcard.rootDirEmptySlotNumber * 32 + 15];
	//set dummy last modify time, same as create date
	block[SDcard.rootDirEmptySlotNumber * 32 + 24] = block[SDcard.rootDirEmptySlotNumber * 32 + 16];
	block[SDcard.rootDirEmptySlotNumber * 32 + 25] = block[SDcard.rootDirEmptySlotNumber * 32 + 17];
}

void addFileInfo2RootDir(volatile uint32_t* block, fileInfo* file, uint64_t sysTick)
{
	//return if something is null pointer
	if (NULL == block || NULL == file) return;

	//check if all slots are filled, if so go for next block
	if (15 < SDcard.rootDirEmptySlotNumber)
	{
		//reset to 0
		SDcard.rootDirEmptySlotNumber = 0;
		//increment to have next block as empty block
		SDcard.rootDirEmptyBlockNumber++;
		//clear block content
		for (uint16_t i = 0; i < 512; i++)
		{
			block[i] = 0x00;
		}
	}

	//fill string part of name
	uint8_t index;
	for (index = 0; index < 4; index++)
	{
		block[SDcard.rootDirEmptySlotNumber * 32 + index] = file->name[index];
	}

	//fill number part of name
	float numberInName = file->numberInName + 0.001;
	uint8_t numberCounter = 0;
	while (numberInName >= 10.0)
	{
		numberInName /= 10;
		numberCounter++;
	}
	//doing like convert to string
	for (uint8_t i = 0; i <= numberCounter; i++)
	{
		uint8_t currentInteger;
		//creat the integer part, add a little bit due to float number
		currentInteger = (uint8_t)numberInName;

		//set correct ASCII char for sending
		block[SDcard.rootDirEmptySlotNumber * 32 + index++] = '0' + currentInteger;

		//substitute it from the original
		numberInName -= (float)currentInteger;

		//multiplie by 10 for the next round
		numberInName *= 10;
	}
	//fill rest of the file name with 'space'
	for (uint8_t emptyIndex = index; emptyIndex < 8; emptyIndex++)
	{
		block[SDcard.rootDirEmptySlotNumber * 32 + emptyIndex] = ' ';
	}
	//fill file type with txt
	block[SDcard.rootDirEmptySlotNumber * 32 + 8] = 'T';
	block[SDcard.rootDirEmptySlotNumber * 32 + 9] = 'X';
	block[SDcard.rootDirEmptySlotNumber * 32 + 10] = 'T';

	//set file attribute, archive
	block[SDcard.rootDirEmptySlotNumber * 32 + 11] = 0x20; // or 0x00;
	//disable checksums?
	block[SDcard.rootDirEmptySlotNumber * 32 + 12] = 0x10; // 0x18;

	block[SDcard.rootDirEmptySlotNumber * 32 + 13] = 0x4E;    //?

	//set time
	setFileTime(block, sysTick);

	//low byte set
	//set position-low and high
	block[SDcard.rootDirEmptySlotNumber * 32 + 21] = (file->clusters[0] & 0xFF000000) >> 24;
	block[SDcard.rootDirEmptySlotNumber * 32 + 20] = (file->clusters[0] & 0x00FF0000) >> 16;
	block[SDcard.rootDirEmptySlotNumber * 32 + 27] = (file->clusters[0] & 0x0000FF00) >> 8;
	block[SDcard.rootDirEmptySlotNumber * 32 + 26] =  file->clusters[0] & 0x000000FF;
	//set size
	block[SDcard.rootDirEmptySlotNumber * 32 + 28] =  file->size & 0x000000FF;
	block[SDcard.rootDirEmptySlotNumber * 32 + 29] = (file->size & 0x0000FF00) >> 8;
	block[SDcard.rootDirEmptySlotNumber * 32 + 30] = (file->size & 0x00FF0000) >> 16;
	block[SDcard.rootDirEmptySlotNumber * 32 + 31] = (file->size & 0xFF000000) >> 24;

	//increment to have the next as empty slot
	SDcard.rootDirEmptySlotNumber++;
}

bool addFileFATInfo(volatile uint32_t* block, fileInfo* file, E_SDFATWriteStates FATState)
{
	static uint32_t blockDiff = 0;
	static uint16_t iterCluster = 0;

	bool returnVal = false;
	bool isFinished = false;
	uint8_t iterBeg = 0;
	uint8_t iterEnd = 0;
	bool shortFileWithinCluster = true;

	//return if something is null pointer
	if (NULL == block || NULL == file) return false;

	switch (FATState)
	{
		case E_SDFATWRITE_FIRST_CALL:
		{
			//if 1st slot is not the very beginnig of a block check the previous file ending
			if (0 != (file->clusters[0] % 128))
			{
				uint32_t lastFileLastSlot = (file->clusters[0] - 1) % 128;
				//check if last file last slot is an end cluster
				if (0xFF != uint8_t(block[4 * lastFileLastSlot + 0]) &&
					0xFF != uint8_t(block[4 * lastFileLastSlot + 1]) &&
					0xFF != uint8_t(block[4 * lastFileLastSlot + 2]) &&
					0x0F >= uint8_t(block[4 * lastFileLastSlot + 3]))
				{
					//SerialUSB.println("Error with last File's last FAT slot!!!");
					return false;
				}
			}
			else
			{
				//if new file starts at 1st slot then clean the full block
				for (uint16_t i = 0; i < 512; i++)
				{
					block[i] = 0;
				}
				//increament FAT block number
				SDcard.FATBlockOffset++;
			}

			//calcu block difference
			uint32_t firstBlock = file->clusters[0];
			uint32_t lastBlock = file->clusters[file->numberOfClusters - 1];
			blockDiff = lastBlock - firstBlock;
			iterCluster = 1;  //set to 1 to already copy the next cluster value
			iterBeg = firstBlock % 128;

			//check if blockDiff is smaller than 128 (1 cluster)
			if (blockDiff <= 128)
			{
				//check if the short file is within 1 cluster
				uint32_t firstBlockCluster = firstBlock / 128;
				uint32_t lastBlockCluster = lastBlock / 128;
				if (firstBlockCluster != lastBlockCluster)
				{
					shortFileWithinCluster = false;
				}
			}

#ifdef LOG_SD_WRITE 
			SerialUSB.print("addFileFATInfo - firstCluster: "); SerialUSB.print(firstBlock);
			SerialUSB.print(", lastCluster: "); SerialUSB.print(lastBlock);
			SerialUSB.print(", iterBeg: "); SerialUSB.println(iterBeg);
#endif
			//set return value to signal potencial multi call
			returnVal = true;

			break;
		}
		case E_SDFATWRITE_CONSECUTIVE_CALL:
		{
			//clean the full block
			for (uint16_t i = 0; i < 512; i++)
			{
				block[i] = 0;
			}
			//increament FAT block number
			SDcard.FATBlockOffset++;

			//set return value to signal potencial multicall
			returnVal = true;

			break;
		}
		default:
		{
			//do nothing, should not reach
			break;
		}
	}

	//SerialUSB.print("blockDiff: "); SerialUSB.println(blockDiff);

	if (128 > blockDiff && true == shortFileWithinCluster)
	{
		iterEnd = file->clusters[file->numberOfClusters - 1] % 128;
		//in case the end cluster is 128 or multiplicity of it positioned at the 128. (end) place
		if (iterEnd < iterBeg)
		{
			iterEnd = 127;
		}
	}
	else
	{
		iterEnd = 128;
	}

	//SerialUSB.print("iterEnd: "); SerialUSB.println(iterEnd);

	//fill block with cluster numbers
	for (uint16_t i = iterBeg; i < iterEnd; i++)
	{
		block[4 * i + 0] =  file->clusters[iterCluster] & 0x000000FF;
		block[4 * i + 1] = (file->clusters[iterCluster] & 0x0000FF00) >> 8;
		block[4 * i + 2] = (file->clusters[iterCluster] & 0x00FF0000) >> 16;
		block[4 * i + 3] = (file->clusters[iterCluster] & 0xFF000000) >> 24;
		iterCluster++;
#ifdef LOG_SD_WRITE 
        SerialUSB.print("addFileFATInfo - "); SerialUSB.print(4 * i + 0); SerialUSB.print(" slot: ");
		SerialUSB.print(block[4 * i + 0]); SerialUSB.print(" ");
		SerialUSB.print(block[4 * i + 1]); SerialUSB.print(" ");
		SerialUSB.print(block[4 * i + 2]); SerialUSB.print(" ");
		SerialUSB.println(block[4 * i + 3]);
#endif
	}

	if (128 > blockDiff && iterEnd < 128)
	{
		//fill last slot with end value
		block[4 * iterEnd + 0] = 0xFF;
		block[4 * iterEnd + 1] = 0xFF;
		block[4 * iterEnd + 2] = 0xFF;
		block[4 * iterEnd + 3] = 0x0F;
		//reset everything
		returnVal = false;
	}
	else
	{
		blockDiff -= iterEnd - iterBeg;
	}

	return returnVal;
}

void printFileInfo(fileInfo* fileInfo)
{
	if (fileInfo != NULL &&
		!(fileInfo->name[0] == 'D' &&
			fileInfo->name[1] == 'E' &&
			fileInfo->name[2] == 'L' &&
			fileInfo->name[3] == 'E' &&
			fileInfo->name[4] == 'T' &&
			fileInfo->name[5] == 'E' &&
			fileInfo->name[6] == 'D'))
	{
		SerialUSB.print(fileInfo->name);
		SerialUSB.print(" - ");
		SerialUSB.print(fileInfo->numberInName);
		SerialUSB.print(" - numberofClusters: "); SerialUSB.print(fileInfo->numberOfClusters);
		SerialUSB.print(" - blockcount: "); SerialUSB.print(fileInfo->blockCount);
		SerialUSB.print(" - size: "); SerialUSB.print(fileInfo->size);
		SerialUSB.print(" - clusters number(s): ");
		SerialUSB.print(fileInfo->clusters[0]);
		for (uint16_t i = 1; i < fileInfo->numberOfClusters; i++)
		{
			SerialUSB.print(", "); SerialUSB.print(fileInfo->clusters[i]);
		}
		SerialUSB.println();
	}
}

void appendCsSdCard(volatile uint32_t* block, uint16_t blockSize)
{
	for (uint16_t i = 0; i < blockSize; i++)
	{
		block[i] |= SPI_TDR_PCS(CS_SDCARD);
	}
}

int8_t checkCtrs(void)
{
    if (SDcard.measBufferCtr >= MAX_MEAS_BUFFER_SIZE)
        return -1;

    if (SDcard.measDataCtr >= 512)
    {
        SDcard.measDataCtr = 0;

        if (++SDcard.measBufferCtr >= MAX_MEAS_BUFFER_SIZE)
            return -1;
    }
    return 0;
}

void appendChar(const char c)
{
    if (checkCtrs() < 0) 
        return;

    SDcard.measBuffer[SDcard.measBufferCtr].data[SDcard.measDataCtr++] = c;
}

void measureData(bool* isCommaed, float data, uint8_t numberOfFrac, bool isExplicitPlus, char* debugName)
{
    uint8_t tempBuffer[30];
    uint8_t numberOfCharacters{ 0 };

	if (*isCommaed)
	{
		appendChar(',');
	}
	else
	{
		*isCommaed = true;
	}

    convert2CharStream(tempBuffer, &numberOfCharacters, data, numberOfFrac, isExplicitPlus);
    loadData2Buffer(tempBuffer, numberOfCharacters);

#ifdef LOG_SAVED_DATA
    SerialUSB.print(debugName);SerialUSB.println(data, numberOfFrac);
#endif
}

void addMeasValueHeader(bool* comma, const int32_t data)
{
	measureData(comma, data, 0, false, "");
}

void measureDiffData(bool* comma, int64_t current, int64_t* last, char* debugName)
{
	measureData(comma, current - *last, 0, false, debugName);
	*last = current;
}

void measureDiffData(bool* comma, int32_t current, int32_t* last, char* debugName)
{
	measureData(comma, current - *last, 0, false, debugName);
	*last = current;
}

void saveMeasData()
{
    accData_st* accData{ getAccData() };
    pid_st* pidData{ getPIDrates() };
	spi_st* spiData{ getSPI() };

	//reset flag
	meas2Card.commaFlag = false;

    //timestamp
	if (meas2Card.measureSysTick) measureDiffData(&meas2Card.commaFlag, getSysTick() / 10500, &meas2Card.lastSysTick, "tickMs: ");
    //gyro
    if(meas2Card.measureGyroRawX) measureDiffData(&meas2Card.commaFlag, spiData->gyro.signals.x, &meas2Card.lastGyroRawX, "gyroRawX_i: ");
    if(meas2Card.measureGyroRawY) measureDiffData(&meas2Card.commaFlag, spiData->gyro.signals.y, &meas2Card.lastGyroRawY, "gyroRawY_i: ");
    if(meas2Card.measureGyroRawZ) measureDiffData(&meas2Card.commaFlag, spiData->gyro.signals.z, &meas2Card.lastGyroRawZ, "gyroRawZ_i: ");
    if(meas2Card.measureGyroPT1X) measureDiffData(&meas2Card.commaFlag, spiData->gyro.signalsPT1.x, &meas2Card.lastGyroPT1X, "gyroPT1X_i: ");
    if(meas2Card.measureGyroPT1Y) measureDiffData(&meas2Card.commaFlag, spiData->gyro.signalsPT1.y, &meas2Card.lastGyroPT1Y, "gyroPT1Y_i: ");
    if(meas2Card.measureGyroPT1Z) measureDiffData(&meas2Card.commaFlag, spiData->gyro.signalsPT1.z, &meas2Card.lastGyroPT1Z, "gyroPT1Z_i: ");
    //if(meas2Card.measureGyroRealX) measureDiffData(&meas2Card.commaFlag, calcRealFromInt(&SPI.gyro, E_direction::X, false), 3, false, "gyroRealX: ");
    //if(meas2Card.measureGyroRealY) measureDiffData(&meas2Card.commaFlag, calcRealFromInt(&SPI.gyro, E_direction::Y, false), 3, false, "gyroRealY: ");
    //if(meas2Card.measureGyroRealZ) measureDiffData(&meas2Card.commaFlag, calcRealFromInt(&SPI.gyro, E_direction::Z, false), 3, false, "gyroRealZ: ");
    //if(meas2Card.measureGyroRealPT1X) measureDiffData(&meas2Card.commaFlag, calcRealFromInt(&SPI.gyro, E_direction::X, true), 3, false, "gyroRealPT1X: ");
    //if(meas2Card.measureGyroRealPT1Y) measureDiffData(&meas2Card.commaFlag, calcRealFromInt(&SPI.gyro, E_direction::Y, true), 3, false, "gyroRealPT1Y: ");
    //if(meas2Card.measureGyroRealPT1Z) measureDiffData(&meas2Card.commaFlag, calcRealFromInt(&SPI.gyro, E_direction::Z, true), 3, false, "gyroRealPT1Z: ");
    //acc
    if(meas2Card.measureAccRawX) measureDiffData(&meas2Card.commaFlag, spiData->acc.signals.x, &meas2Card.lastAccRawX, "accRawX_i: ");
    if(meas2Card.measureAccRawY) measureDiffData(&meas2Card.commaFlag, spiData->acc.signals.y, &meas2Card.lastAccRawY, "accRawY_i: ");
    if(meas2Card.measureAccRawZ) measureDiffData(&meas2Card.commaFlag, spiData->acc.signals.z, &meas2Card.lastAccRawZ, "accRawZ_i: ");
    if(meas2Card.measureAccPT1X) measureDiffData(&meas2Card.commaFlag, spiData->acc.signalsPT1.x, &meas2Card.lastAccPT1X, "accPT1X_i: ");
    if(meas2Card.measureAccPT1Y) measureDiffData(&meas2Card.commaFlag, spiData->acc.signalsPT1.y, &meas2Card.lastAccPT1Y, "accPT1Y_i: ");
    if(meas2Card.measureAccPT1Z) measureDiffData(&meas2Card.commaFlag, spiData->acc.signalsPT1.z, &meas2Card.lastAccPT1Z, "accPT1Z_i: ");
	//if(meas2Card.measureAccRealX) measureData(&meas2Card.commaFlag, calcRealFromInt(&SPI.acc, E_direction::X, false), 3, false, "accRealX: ");
	//if(meas2Card.measureAccRealY) measureData(&meas2Card.commaFlag, calcRealFromInt(&SPI.acc, E_direction::Y, false), 3, false, "accRealY: ");
	//if(meas2Card.measureAccRealZ) measureData(&meas2Card.commaFlag, calcRealFromInt(&SPI.acc, E_direction::Z, false), 3, false, "accRealZ: ");
	//if(meas2Card.measureAccRealPT1X) measureData(&meas2Card.commaFlag, calcRealFromInt(&SPI.acc, E_direction::X, true), 3, false, "accRealPT1X: ");
	//if(meas2Card.measureAccRealPT1Y) measureData(&meas2Card.commaFlag, calcRealFromInt(&SPI.acc, E_direction::Y, true), 3, false, "accRealPT1Y: ");
	//if(meas2Card.measureAccRealPT1Z) measureData(&meas2Card.commaFlag, calcRealFromInt(&SPI.acc, E_direction::Z, true), 3, false, "accRealPT1Z: ");
    //angle
    //measureData(meas2Card.measureAngleRawRoll, true, accData->rollAngle, 3, false, "rollAngleRaw: ");
    //measureData(meas2Card.measureAngleRawPitch, true, accData->pitchAngle, 3, false, "pitchAngleRaw: ");
    if(meas2Card.measureAnglePT1Roll) measureDiffData(&meas2Card.commaFlag, accData->rollPT1_i, &meas2Card.lastAnglePT1Roll, "rollAnglePT1: ");
    if(meas2Card.measureAnglePT1Pitch) measureDiffData(&meas2Card.commaFlag, accData->pitchPT1_i, &meas2Card.lastAnglePT1Pitch, "pitchAnglePT2: ");
 //   measureData(meas2Card.measureAnglePT2Roll, true, accData->rollAnglePT2Acc, 3, false, "rollAnglePT2: ");
 //   measureData(meas2Card.measureAnglePT2Pitch, true, accData->pitchAnglePT2Acc, 3, false, "pitchAnglePT2: ");
 //   measureData(meas2Card.measureAngleKFRawRoll, true, accData->angleKF.roll.angle, 3, false, "angleKFRoll: ");
 //   measureData(meas2Card.measureAngleKFRawPitch, true, accData->angleKF.pitch.angle, 3, false, "angleKFPitch: ");
    if(meas2Card.measureAngleKFPT11Roll) measureDiffData(&meas2Card.commaFlag, accData->angleKF.roll.angle, &meas2Card.lastAngleKFPT11Roll, "angleKFPT11Roll: ");
    if(meas2Card.measureAngleKFPT11Pitch) measureDiffData(&meas2Card.commaFlag, accData->angleKF.pitch.angle, &meas2Card.lastAngleKFPT11Pitch, "angleKFPT11Pitch: ");
	//measureData(meas2Card.measureAngleCFRawRoll, true, accData->rollAngleCF, 3, false, "angleCFRoll: ");
	//measureData(meas2Card.measureAngleCFRawPitch, true, accData->pitchAngleCF, 3, false, "angleCFPitch: ");
	//measureData(meas2Card.measureAngleCFPT10Roll, true, accData->rollAngleCF10, 3, false, "angleCFPT10Roll: ");
	//measureData(meas2Card.measureAngleCFPT10Pitch, true, accData->pitchAngleCF10, 3, false, "angleCFPT10Pitch: ");
	//measureData(meas2Card.measureAngleCFPT11Roll, true, accData->rollAngleCF11, 3, false, "angleCFPT11Roll: ");
	//measureData(meas2Card.measureAngleCFPT11Pitch, true, accData->pitchAngleCF11, 3, false, "angleCFPT11Pitch: ");
	//measureData(meas2Card.measureAngleCFWeightedRawRoll, true, accData->rollAngleCFw, 3, false, "angleCFWeightedRoll: ");
	//measureData(meas2Card.measureAngleCFWeightedRawPitch, true, accData->pitchAngleCFw, 3, false, "angleCFWeightedPitch: ");
	//measureData(meas2Card.measureAngleCFWeightedPT01Roll, true, accData->rollAngleCFw01, 3, false, "angleCFWeightedPT01Roll: ");
	//measureData(meas2Card.measureAngleCFWeightedPT01Pitch, true, accData->pitchAngleCFw01, 3, false, "angleCFWeightedPT01 Pitch: ");
    //PID control
	if(meas2Card.measurePIDRefsigX) measureDiffData(&meas2Card.commaFlag, pidData->refSig_i.x, &meas2Card.lastPIDRefsigX, "PIDRefSigXi: ");
	if(meas2Card.measurePIDRefsigY) measureDiffData(&meas2Card.commaFlag, pidData->refSig_i.y, &meas2Card.lastPIDRefsigY, "PIDRefSigYi: ");
	if(meas2Card.measurePIDRefsigZ) measureDiffData(&meas2Card.commaFlag, pidData->refSig_i.z, &meas2Card.lastPIDRefsigZ, "PIDRefSigZi: ");
	if(meas2Card.measurePIDSensorX) measureDiffData(&meas2Card.commaFlag, pidData->sensor.signalPT1.x, &meas2Card.lastPIDSensorX, "PIDSensorXi: ");
	if(meas2Card.measurePIDSensorY) measureDiffData(&meas2Card.commaFlag, pidData->sensor.signalPT1.y, &meas2Card.lastPIDSensorY, "PIDSensorYi: ");
	if(meas2Card.measurePIDSensorZ) measureDiffData(&meas2Card.commaFlag, pidData->sensor.signalPT1.z, &meas2Card.lastPIDSensorZ, "PIDSensorZi: ");
	if(meas2Card.measurePIDPoutX) measureDiffData(&meas2Card.commaFlag, pidData->Pout_i.x, &meas2Card.lastPIDPoutX, "PIDPoutXi: ");
	if(meas2Card.measurePIDPoutY) measureDiffData(&meas2Card.commaFlag, pidData->Pout_i.y, &meas2Card.lastPIDPoutY, "PIDPoutYi: ");
	if(meas2Card.measurePIDPoutZ) measureDiffData(&meas2Card.commaFlag, pidData->Pout_i.z, &meas2Card.lastPIDPoutZ, "PIDPoutZi: ");
	if(meas2Card.measurePIDIoutX) measureDiffData(&meas2Card.commaFlag, pidData->Iout_i.x, &meas2Card.lastPIDIoutX, "PIDIoutXi: ");
	if(meas2Card.measurePIDIoutY) measureDiffData(&meas2Card.commaFlag, pidData->Iout_i.y, &meas2Card.lastPIDIoutY, "PIDIoutYi: ");
	if(meas2Card.measurePIDIoutZ) measureDiffData(&meas2Card.commaFlag, pidData->Iout_i.z, &meas2Card.lastPIDIoutZ, "PIDIoutZi: ");
	if(meas2Card.measurePIDDoutX) measureDiffData(&meas2Card.commaFlag, pidData->Dout_i.x, &meas2Card.lastPIDDoutX, "PIDDoutXi: ");
	if(meas2Card.measurePIDDoutY) measureDiffData(&meas2Card.commaFlag, pidData->Dout_i.y, &meas2Card.lastPIDDoutY, "PIDDoutYi: ");
	if(meas2Card.measurePIDDoutZ) measureDiffData(&meas2Card.commaFlag, pidData->Dout_i.z, &meas2Card.lastPIDDoutZ, "PIDDoutZi: ");
	if(meas2Card.measurePIDFFoutX) measureDiffData(&meas2Card.commaFlag, pidData->FFout_i.x, &meas2Card.lastPIDFFoutX, "PIDFFoutXi: ");
	if(meas2Card.measurePIDFFoutY) measureDiffData(&meas2Card.commaFlag, pidData->FFout_i.y, &meas2Card.lastPIDFFoutY, "PIDFFoutYi: ");
	if(meas2Card.measurePIDFFoutZ) measureDiffData(&meas2Card.commaFlag, pidData->FFout_i.z, &meas2Card.lastPIDFFoutZ, "PIDFFoutZi: ");
	if(meas2Card.measurePIDUX) measureDiffData(&meas2Card.commaFlag, pidData->u_i.x, &meas2Card.lastPIDUX, "PIDUXi: ");
	if(meas2Card.measurePIDUY) measureDiffData(&meas2Card.commaFlag, pidData->u_i.y, &meas2Card.lastPIDUY, "PIDUYi: ");
	if(meas2Card.measurePIDUZ) measureDiffData(&meas2Card.commaFlag, pidData->u_i.z, &meas2Card.lastPIDUZ, "PIDUZi: ");
	//PID internals
	if(meas2Card.measurePIDrefSigDotPT1X) measureDiffData(&meas2Card.commaFlag, pidData->refSigDotPT1_i.x, &meas2Card.lastPIDrefSigDotPT1X, "PIDRefDotPT1Xi: ");
	if(meas2Card.measurePIDrefSigDotPT1Y) measureDiffData(&meas2Card.commaFlag, pidData->refSigDotPT1_i.y, &meas2Card.lastPIDrefSigDotPT1Y, "PIDRefDotPT1Yi: ");
	if(meas2Card.measurePIDrefSigDotPT1Z) measureDiffData(&meas2Card.commaFlag, pidData->refSigDotPT1_i.z, &meas2Card.lastPIDrefSigDotPT1Z, "PIDRefDotPT1Zi: ");
	if(meas2Card.measurePIDiRelaxWeightX) measureDiffData(&meas2Card.commaFlag, pidData->iRelaxWeight.x, &meas2Card.lastPIDiRelaxWeightX, "PIDiRelaxWeightX: ");
	if(meas2Card.measurePIDiRelaxWeightY) measureDiffData(&meas2Card.commaFlag, pidData->iRelaxWeight.y, &meas2Card.lastPIDiRelaxWeightY, "PIDiRelaxWeightY: ");
	if(meas2Card.measurePIDiRelaxWeightZ) measureDiffData(&meas2Card.commaFlag, pidData->iRelaxWeight.z, &meas2Card.lastPIDiRelaxWeightZ, "PIDiRelaxWeightZ: ");

	appendChar('\n');
}

void convert2CharStream(uint8_t* buffer, uint8_t* numberOfChar, float value, uint8_t numberOfFractions, bool explicitPlusSign)
{
	if (value < 0)
	{
		buffer[(*numberOfChar)++] = '-';
		value = -value;
	}
	else if (explicitPlusSign)
	{
		buffer[(*numberOfChar)++] = '+';
	}

	uint32_t scale = powOf10[numberOfFractions];
	uint32_t scaled = (uint32_t)(value * scale + 0.49f);	//0.49 increment is to solve the numerical accurecy problem

	uint32_t intPart = scaled / scale;

	// --- Integer part ---
	char temp[10];
	int i = 0;

	do {
		temp[i++] = '0' + (intPart % 10);
		intPart /= 10;
	} while (intPart);

	while (i--)
		buffer[(*numberOfChar)++] = temp[i];

	// --- Fractional part ---
	if (numberOfFractions > 0)
	{
		uint32_t fracPart = scaled % scale;

		buffer[(*numberOfChar)++] = '.';

		for (int j = numberOfFractions - 1; j >= 0; j--)
		{
			uint32_t div = powOf10[j];
			buffer[(*numberOfChar)++] = '0' + (fracPart / div);
			fracPart %= div;
		}
	}
}

void loadData2Buffer(uint8_t* chars2Add, uint8_t numberOfChar)
{
    while (numberOfChar)
    {
        if (SDcard.measBufferCtr >= MAX_MEAS_BUFFER_SIZE)
            return;

        uint16_t space = 512 - SDcard.measDataCtr;
        uint16_t count = (numberOfChar < space) ? numberOfChar : space;

        memcpy(
            &SDcard.measBuffer[SDcard.measBufferCtr].data[SDcard.measDataCtr],
            chars2Add,
            count
        );

        SDcard.measDataCtr += count;
        chars2Add += count;
        numberOfChar -= count;

        if (SDcard.measDataCtr == 512)
        {
            SDcard.measDataCtr = 0;
            SDcard.measBufferCtr++;
        }
    }
}

void addMeasNameHeader(bool* isCommaed, char* name, uint8_t numberOfChar)
{
    uint8_t tempBuffer[30];
    uint8_t numberOfCharacters{ 0 };

	if (*isCommaed)
	{
		appendChar(',');
	}
	else
	{
		*isCommaed = true;
	}

    for (uint8_t i = 0; i < numberOfChar; i++)
    {
        tempBuffer[numberOfCharacters++] = name[i];
    }

    loadData2Buffer(tempBuffer, numberOfCharacters);

#ifdef LOG_SAVED_DATA
    SerialUSB.print("Measured name: "); SerialUSB.print(name);
	SerialUSB.print(", loadingDataCounter: ");SerialUSB.println(SDcard.loadingDataCounter);
#endif
}

//1st line : R for rate
//2nd line: Px,I,D,Py,I,D,Pz,I,FFrx,y,FFdrx,y,satI,satPID,DTermC\n
//3rd line: values
//4th line: C for cacade
//...
//...
//xth line: header for measured signals, example: sysTick,Grawx,Gpt1x,Gpt2x,Accrawx,Accpt1x,Accpt2x,Akfx,Akfaccpt1x\n
void addMeasHeader(void)
{
    SDcard.measDataCtr = 0;
    SDcard.measBufferCtr = 0;
	//1st line
	SDcard.measBuffer[SDcard.measBufferCtr].data[SDcard.measDataCtr++] = 'R';
	SDcard.measBuffer[SDcard.measBufferCtr].data[SDcard.measDataCtr++] = '\n';
	//2nd line
	{
		bool commaFlag{ false };

        addMeasNameHeader(&commaFlag, "StartTickMs", 11);
        addMeasNameHeader(&commaFlag, "measCycleMs", 11);
        addMeasNameHeader(&commaFlag, "Px", 2);
        addMeasNameHeader(&commaFlag, "Ix", 2);
        addMeasNameHeader(&commaFlag, "Dx", 2);
        addMeasNameHeader(&commaFlag, "Py", 2);
        addMeasNameHeader(&commaFlag, "Iy", 2);
        addMeasNameHeader(&commaFlag, "Dy", 2);
        addMeasNameHeader(&commaFlag, "Pz", 2);
        addMeasNameHeader(&commaFlag, "Iz", 2);
        addMeasNameHeader(&commaFlag, "FFrx", 4);
        addMeasNameHeader(&commaFlag, "FFry", 4);
        addMeasNameHeader(&commaFlag, "FFdrx", 5);
        addMeasNameHeader(&commaFlag, "FFdry", 5);
        addMeasNameHeader(&commaFlag, "satI", 4);
        addMeasNameHeader(&commaFlag, "satPID", 6);
        addMeasNameHeader(&commaFlag, "KFQAng", 6);
        addMeasNameHeader(&commaFlag, "KFQbias", 7);
        addMeasNameHeader(&commaFlag, "KFRmeas", 7);
        addMeasNameHeader(&commaFlag, "CPx", 3);
        addMeasNameHeader(&commaFlag, "CIx", 3);
		addMeasNameHeader(&commaFlag, "CPy", 3);
		addMeasNameHeader(&commaFlag, "CIy", 3);
		addMeasNameHeader(&commaFlag, "CsatI", 5);
		addMeasNameHeader(&commaFlag, "CsatPID", 7);
		addMeasNameHeader(&commaFlag, "CFFdrx", 6);
		addMeasNameHeader(&commaFlag, "CFFdry", 6);
		addMeasNameHeader(&commaFlag, "IRelaxX", 7);
		addMeasNameHeader(&commaFlag, "IRelaxY", 7);
		addMeasNameHeader(&commaFlag, "DmaxR", 5);
		addMeasNameHeader(&commaFlag, "DmaxE", 5);
		addMeasNameHeader(&commaFlag, "DMx", 3);
		addMeasNameHeader(&commaFlag, "DMy", 3);
        appendChar('\n');
#ifdef LOG_SAVED_DATA
		SerialUSB.print("End of 2nd line, loadingDataCounter: ");SerialUSB.println(SDcard.loadingDataCounter);
#endif
	}
	//3rd line
	{
		pid_st* pidRate{ getPIDrates() };
		pid_st* pidCascade{ getPIDcascade() };
        accData_st* acc{ getAccData() };
		bool commaFlag{ false };

		addMeasValueHeader(&commaFlag, getSysTick() / 10500);
		addMeasValueHeader(&commaFlag, DATA_SAVE_DELAY / 10500);
		addMeasValueHeader(&commaFlag, pidRate->P_i.x);
		addMeasValueHeader(&commaFlag, pidRate->I_i.x);
		addMeasValueHeader(&commaFlag, pidRate->D_i.x);
		addMeasValueHeader(&commaFlag, pidRate->P_i.y);
		addMeasValueHeader(&commaFlag, pidRate->I_i.y);
		addMeasValueHeader(&commaFlag, pidRate->D_i.y);
		addMeasValueHeader(&commaFlag, pidRate->P_i.z);
		addMeasValueHeader(&commaFlag, pidRate->I_i.z);
		addMeasValueHeader(&commaFlag, pidRate->D_i.z);
		addMeasValueHeader(&commaFlag, pidRate->FFr_i.x);
		addMeasValueHeader(&commaFlag, pidRate->FFr_i.y);
		addMeasValueHeader(&commaFlag, pidRate->FFdr_i.x);
		addMeasValueHeader(&commaFlag, pidRate->FFdr_i.y);
		addMeasValueHeader(&commaFlag, pidRate->satI_i);
		addMeasValueHeader(&commaFlag, pidRate->satPID_i);
		addMeasValueHeader(&commaFlag, acc->angleKF.qAngleTick);
		addMeasValueHeader(&commaFlag, acc->angleKF.qBiasTick);
		addMeasValueHeader(&commaFlag, acc->angleKF.rMeasTick);
		addMeasValueHeader(&commaFlag, pidCascade->P_i.x);
		addMeasValueHeader(&commaFlag, pidCascade->I_i.x);
		addMeasValueHeader(&commaFlag, pidCascade->P_i.x);
		addMeasValueHeader(&commaFlag, pidCascade->I_i.x);
		addMeasValueHeader(&commaFlag, pidCascade->satI_i);
		addMeasValueHeader(&commaFlag, pidCascade->satPID_i);
		addMeasValueHeader(&commaFlag, pidCascade->FFdr_i.x);
		addMeasValueHeader(&commaFlag, pidCascade->FFdr_i.y);
		addMeasValueHeader(&commaFlag, pidRate->iRelaxWeight.x);
		addMeasValueHeader(&commaFlag, pidRate->iRelaxWeight.y);
		addMeasValueHeader(&commaFlag, pidRate->dMaxRefThold_i);
		addMeasValueHeader(&commaFlag, pidRate->dMaxErrThold_i);
		addMeasValueHeader(&commaFlag, pidRate->Dmax_i.x);
		addMeasValueHeader(&commaFlag, pidRate->Dmax_i.y);

		appendChar('\n');

#ifdef LOG_SAVED_DATA
		SerialUSB.print("End of 3rd line, loadingDataCounter: ");SerialUSB.println(SDcard.loadingDataCounter);
#endif
	}
	//4th line: measured value names
	{
		bool commaFlag{ false };
        //systick
        if(meas2Card.measureSysTick) addMeasNameHeader(&commaFlag, "sysTickMs", 9);
        //gyro
        if(meas2Card.measureGyroRawX) addMeasNameHeader(&commaFlag, "GRawX", 5);
        if(meas2Card.measureGyroRawY) addMeasNameHeader(&commaFlag, "GRawY", 5);
        if(meas2Card.measureGyroRawZ) addMeasNameHeader(&commaFlag, "GRawZ", 5);
        if(meas2Card.measureGyroPT1X) addMeasNameHeader(&commaFlag, "GPT1X", 5);
        if(meas2Card.measureGyroPT1Y) addMeasNameHeader(&commaFlag, "GPT1Y", 5);
        if(meas2Card.measureGyroPT1Z) addMeasNameHeader(&commaFlag, "GPT1Z", 5);
        //if(meas2Card.measureGyroRealX) addMeasNameHeader(&commaFlag, "GRealX", 6);
        //if(meas2Card.measureGyroRealY) addMeasNameHeader(&commaFlag, "GRealY", 6);
        //if(meas2Card.measureGyroRealZ) addMeasNameHeader(&commaFlag, "GRealZ", 6);
        //if(meas2Card.measureGyroRealPT1X) addMeasNameHeader(&commaFlag, "GRealPT1X", 9);
        //if(meas2Card.measureGyroRealPT1Y) addMeasNameHeader(&commaFlag, "GRealPT1Y", 9);
        //if(meas2Card.measureGyroRealPT1Z) addMeasNameHeader(&commaFlag, "GRealPT1Z", 9);
        //acc
        if(meas2Card.measureAccRawX) addMeasNameHeader(&commaFlag, "ARawX", 5);
        if(meas2Card.measureAccRawY) addMeasNameHeader(&commaFlag, "ARawY", 5);
        if(meas2Card.measureAccRawZ) addMeasNameHeader(&commaFlag, "ARawZ", 5);
        if(meas2Card.measureAccPT1X) addMeasNameHeader(&commaFlag, "APT1X", 5);
        if(meas2Card.measureAccPT1Y) addMeasNameHeader(&commaFlag, "APT1Y", 5);
        if(meas2Card.measureAccPT1Z) addMeasNameHeader(&commaFlag, "APT1Z", 5);
		//if(meas2Card.measureAccRealX) addMeasNameHeader(&commaFlag, "ARealX", 6);
		//if(meas2Card.measureAccRealY) addMeasNameHeader(&commaFlag, "ARealY", 6);
		//if(meas2Card.measureAccRealZ) addMeasNameHeader(&commaFlag, "ARealZ", 6);
		//if(meas2Card.measureAccRealPT1X) addMeasNameHeader(&commaFlag, "ARealPT1X", 9);
		//if(meas2Card.measureAccRealPT1Y) addMeasNameHeader(&commaFlag, "ARealPT1Y", 9);
		//if(meas2Card.measureAccRealPT1Z) addMeasNameHeader(&commaFlag, "ARealPT1Z", 9);
        //angle
        if(meas2Card.measureAnglePT1Roll) addMeasNameHeader(&commaFlag, "aPT1R", 5);
        if(meas2Card.measureAnglePT1Pitch) addMeasNameHeader(&commaFlag, "aPT1P", 5);
        if(meas2Card.measureAngleKFPT11Roll) addMeasNameHeader(&commaFlag, "aKFPT11R", 8);
        if(meas2Card.measureAngleKFPT11Pitch) addMeasNameHeader(&commaFlag, "aKFPT11P", 8);
  //      addMeasNameHeader(meas2Card.measureAngleCFRawRoll, true, "aCFRawR", 7);
  //      addMeasNameHeader(meas2Card.measureAngleCFRawPitch, true, "aCFRawP", 7);
		//addMeasNameHeader(meas2Card.measureAngleCFPT10Roll, true, "aCFPT10R", 8);
		//addMeasNameHeader(meas2Card.measureAngleCFPT10Pitch, true, "aCFPT10P", 8);
		//addMeasNameHeader(meas2Card.measureAngleCFPT11Roll, true, "aCFPT11R", 8);
		//addMeasNameHeader(meas2Card.measureAngleCFPT11Pitch, true, "aCFPT11P", 8);
		//addMeasNameHeader(meas2Card.measureAngleCFWeightedRawRoll, true, "aCFwRawR", 8);
		//addMeasNameHeader(meas2Card.measureAngleCFWeightedRawPitch, true, "aCFwRawP", 8);
		//addMeasNameHeader(meas2Card.measureAngleCFWeightedPT01Roll, true, "aCFwPT01R", 9);
		//addMeasNameHeader(meas2Card.measureAngleCFWeightedPT01Pitch, true, "aCFwPT01P", 9);
        //PID control
		if(meas2Card.measurePIDRefsigX) addMeasNameHeader(&commaFlag, "PIDRefXi", 8);
		if(meas2Card.measurePIDRefsigY) addMeasNameHeader(&commaFlag, "PIDRefYi", 8);
		if(meas2Card.measurePIDRefsigZ) addMeasNameHeader(&commaFlag, "PIDRefZi", 8);
		if(meas2Card.measurePIDSensorX) addMeasNameHeader(&commaFlag, "PIDSensXi", 9);
		if(meas2Card.measurePIDSensorY) addMeasNameHeader(&commaFlag, "PIDSensYi", 9);
		if(meas2Card.measurePIDSensorZ) addMeasNameHeader(&commaFlag, "PIDSensZi", 9);
		if(meas2Card.measurePIDPoutX) addMeasNameHeader(&commaFlag, "PIDPoutXi", 9);
		if(meas2Card.measurePIDPoutY) addMeasNameHeader(&commaFlag, "PIDPoutYi", 9);
		if(meas2Card.measurePIDPoutZ) addMeasNameHeader(&commaFlag, "PIDPoutZi", 9);
		if(meas2Card.measurePIDIoutX) addMeasNameHeader(&commaFlag, "PIDIoutXi", 9);
		if(meas2Card.measurePIDIoutY) addMeasNameHeader(&commaFlag, "PIDIoutYi", 9);
		if(meas2Card.measurePIDIoutZ) addMeasNameHeader(&commaFlag, "PIDIoutZi", 9);
		if(meas2Card.measurePIDDoutX) addMeasNameHeader(&commaFlag, "PIDDoutXi", 9);
		if(meas2Card.measurePIDDoutY) addMeasNameHeader(&commaFlag, "PIDDoutYi", 9);
		if(meas2Card.measurePIDDoutZ) addMeasNameHeader(&commaFlag, "PIDDoutZi", 9);
		if(meas2Card.measurePIDFFoutX) addMeasNameHeader(&commaFlag, "PIDFFoutXi", 10);
		if(meas2Card.measurePIDFFoutY) addMeasNameHeader(&commaFlag, "PIDFFoutYi", 10);
		if(meas2Card.measurePIDFFoutZ) addMeasNameHeader(&commaFlag, "PIDFFoutZi", 10);
		if(meas2Card.measurePIDUX) addMeasNameHeader(&commaFlag, "PIDUXi", 6);
		if(meas2Card.measurePIDUY) addMeasNameHeader(&commaFlag, "PIDUYi", 6);
		if(meas2Card.measurePIDUZ) addMeasNameHeader(&commaFlag, "PIDUZi", 6);

		if(meas2Card.measurePIDrefSigDotPT1X) addMeasNameHeader(&commaFlag, "PIDRefDotPT1Xi", 14);
		if(meas2Card.measurePIDrefSigDotPT1Y) addMeasNameHeader(&commaFlag, "PIDRefDotPT1Yi", 14);
		if(meas2Card.measurePIDrefSigDotPT1Z) addMeasNameHeader(&commaFlag, "PIDRefDotPT1Zi", 14);
		if(meas2Card.measurePIDiRelaxWeightX) addMeasNameHeader(&commaFlag, "PIDiRelaxWeightX", 16);
		if(meas2Card.measurePIDiRelaxWeightY) addMeasNameHeader(&commaFlag, "PIDiRelaxWeightY", 16);
		if(meas2Card.measurePIDiRelaxWeightZ) addMeasNameHeader(&commaFlag, "PIDiRelaxWeightZ", 16);

        appendChar('\n');

#ifdef LOG_SAVED_DATA
				SerialUSB.print("End of 4th line, loadingDataCounter: ");SerialUSB.println(SDcard.loadingDataCounter);
#endif
	}
	//5th line: first measured absolute values
	{
		accData_st* accData{ getAccData() };
		pid_st* pidData{ getPIDrates() };
		spi_st* spiData{ getSPI() };
		bool commaFlag{ false };
		//timestamp
		if (meas2Card.measureSysTick) { meas2Card.lastSysTick = getSysTick() / 10500; addMeasValueHeader(&commaFlag, meas2Card.lastSysTick); }
		//gyro
		if (meas2Card.measureGyroRawX) { meas2Card.lastGyroRawX = spiData->gyro.signals.x; addMeasValueHeader(&commaFlag, meas2Card.lastGyroRawX); }
		if (meas2Card.measureGyroRawY) { meas2Card.lastGyroRawY = spiData->gyro.signals.y; addMeasValueHeader(&commaFlag, meas2Card.lastGyroRawY); }
		if (meas2Card.measureGyroRawZ) { meas2Card.lastGyroRawZ = spiData->gyro.signals.z; addMeasValueHeader(&commaFlag, meas2Card.lastGyroRawZ); }
		if (meas2Card.measureGyroPT1X) { meas2Card.lastGyroPT1X = spiData->gyro.signalsPT1.x; addMeasValueHeader(&commaFlag, meas2Card.lastGyroPT1X); }
		if (meas2Card.measureGyroPT1Y) { meas2Card.lastGyroPT1Y = spiData->gyro.signalsPT1.y; addMeasValueHeader(&commaFlag, meas2Card.lastGyroPT1Y); }
		if (meas2Card.measureGyroPT1Z) { meas2Card.lastGyroPT1Z = spiData->gyro.signalsPT1.z; addMeasValueHeader(&commaFlag, meas2Card.lastGyroPT1Z); }
		//if (meas2Card.measureGyroRealX) meas2Card.lastGyroRealX = calcRealFromInt(&SPI.gyro, E_direction::X, false); addMeasValueHeader(&commaFlag, meas2Card.lastGyroRealX);
		//if (meas2Card.measureGyroRealY) meas2Card.lastGyroRealY = calcRealFromInt(&SPI.gyro, E_direction::Y, false); addMeasValueHeader(&commaFlag, meas2Card.lastGyroRealY);
		//if (meas2Card.measureGyroRealZ) meas2Card.lastGyroRealZ = calcRealFromInt(&SPI.gyro, E_direction::Z, false); addMeasValueHeader(&commaFlag, meas2Card.lastGyroRealZ);
		//if (meas2Card.measureGyroRealPT1X) meas2Card.lastGyroRealPT1X = calcRealFromInt(&SPI.gyro, E_direction::X, true); addMeasValueHeader(&commaFlag, meas2Card.lastGyroRealPT1X);
		//if (meas2Card.measureGyroRealPT1Y) meas2Card.lastGyroRealPT1Y = calcRealFromInt(&SPI.gyro, E_direction::Y, true); addMeasValueHeader(&commaFlag, meas2Card.lastGyroRealPT1Y);
		//if (meas2Card.measureGyroRealPT1Z) meas2Card.lastGyroRealPT1Z = calcRealFromInt(&SPI.gyro, E_direction::Z, true); addMeasValueHeader(&commaFlag, meas2Card.lastGyroRealPT1Z);
		//acc
		if (meas2Card.measureAccRawX) { meas2Card.lastAccRawX = spiData->acc.signals.x; addMeasValueHeader(&commaFlag, meas2Card.lastAccRawX); }
		if (meas2Card.measureAccRawY) { meas2Card.lastAccRawY = spiData->acc.signals.y; addMeasValueHeader(&commaFlag, meas2Card.lastAccRawY); }
		if (meas2Card.measureAccRawZ) { meas2Card.lastAccRawZ = spiData->acc.signals.z; addMeasValueHeader(&commaFlag, meas2Card.lastAccRawZ); }
		if (meas2Card.measureAccPT1X) { meas2Card.lastAccPT1X = spiData->acc.signalsPT1.x; addMeasValueHeader(&commaFlag, meas2Card.lastAccPT1X); }
		if (meas2Card.measureAccPT1Y) { meas2Card.lastAccPT1Y = spiData->acc.signalsPT1.y; addMeasValueHeader(&commaFlag, meas2Card.lastAccPT1Y); }
		if (meas2Card.measureAccPT1Z) { meas2Card.lastAccPT1Z = spiData->acc.signalsPT1.z; addMeasValueHeader(&commaFlag, meas2Card.lastAccPT1Z); }
		//if (meas2Card.measureAccRealX) meas2Card.lastAccRealX = calcRealFromInt(&SPI.acc, E_direction::X, false); addMeasValueHeader(&meas2Card.commaFlag, meas2Card.lastAccRealX);
		//if (meas2Card.measureAccRealY) meas2Card.lastAccRealY = calcRealFromInt(&SPI.acc, E_direction::Y, false); addMeasValueHeader(&meas2Card.commaFlag, meas2Card.lastAccRealY);
		//if (meas2Card.measureAccRealZ) meas2Card.lastAccRealZ = calcRealFromInt(&SPI.acc, E_direction::Z, false); addMeasValueHeader(&meas2Card.commaFlag, meas2Card.lastAccRealZ);
		//if (meas2Card.measureAccRealPT1X) meas2Card.measureAccRealPT1X = calcRealFromInt(&SPI.acc, E_direction::X, true); addMeasValueHeader(&meas2Card.commaFlag, meas2Card.measureAccRealPT1X);
		//if (meas2Card.measureAccRealPT1Y) meas2Card.measureAccRealPT1Y = calcRealFromInt(&SPI.acc, E_direction::Y, true); addMeasValueHeader(&meas2Card.commaFlag, meas2Card.measureAccRealPT1Y);
		//if (meas2Card.measureAccRealPT1Z) meas2Card.measureAccRealPT1Z = calcRealFromInt(&SPI.acc, E_direction::Z, true); addMeasValueHeader(&meas2Card.commaFlag, meas2Card.measureAccRealPT1Z);
		//angle
		if (meas2Card.measureAnglePT1Roll) { meas2Card.lastAnglePT1Roll = accData->rollPT1_i; addMeasValueHeader(&commaFlag, meas2Card.lastAnglePT1Roll); }
		if (meas2Card.measureAnglePT1Pitch) { meas2Card.lastAnglePT1Pitch = accData->pitchPT1_i; addMeasValueHeader(&commaFlag, meas2Card.lastAnglePT1Pitch); }
		if (meas2Card.measureAngleKFPT11Roll) { meas2Card.lastAngleKFPT11Roll = accData->angleKF.roll.angle; addMeasValueHeader(&commaFlag, meas2Card.lastAngleKFPT11Roll); }
		if (meas2Card.measureAngleKFPT11Pitch) { meas2Card.lastAngleKFPT11Pitch = accData->angleKF.pitch.angle; addMeasValueHeader(&commaFlag, meas2Card.lastAngleKFPT11Pitch); }
		//PID control
		if (meas2Card.measurePIDRefsigX) { meas2Card.lastPIDRefsigX = pidData->refSig_i.x; addMeasValueHeader(&commaFlag, meas2Card.lastPIDRefsigX); }
		if (meas2Card.measurePIDRefsigY) { meas2Card.lastPIDRefsigY = pidData->refSig_i.y; addMeasValueHeader(&commaFlag, meas2Card.lastPIDRefsigY); }
		if (meas2Card.measurePIDRefsigZ) { meas2Card.lastPIDRefsigZ = pidData->refSig_i.z; addMeasValueHeader(&commaFlag, meas2Card.lastPIDRefsigZ); }
		if (meas2Card.measurePIDSensorX) { meas2Card.lastPIDSensorX = pidData->sensor.signalPT1.x; addMeasValueHeader(&commaFlag, meas2Card.lastPIDSensorX); }
		if (meas2Card.measurePIDSensorY) { meas2Card.lastPIDSensorY = pidData->sensor.signalPT1.y; addMeasValueHeader(&commaFlag, meas2Card.lastPIDSensorY); }
		if (meas2Card.measurePIDSensorZ) { meas2Card.lastPIDSensorZ = pidData->sensor.signalPT1.z; addMeasValueHeader(&commaFlag, meas2Card.lastPIDSensorZ); }
		if (meas2Card.measurePIDPoutX) { meas2Card.lastPIDPoutX = pidData->Pout_i.x; addMeasValueHeader(&commaFlag, meas2Card.lastPIDPoutX); }
		if (meas2Card.measurePIDPoutY) { meas2Card.lastPIDPoutY = pidData->Pout_i.y; addMeasValueHeader(&commaFlag, meas2Card.lastPIDPoutY); }
		if (meas2Card.measurePIDPoutZ) { meas2Card.lastPIDPoutZ = pidData->Pout_i.z; addMeasValueHeader(&commaFlag, meas2Card.lastPIDPoutZ); }
		if (meas2Card.measurePIDIoutX) { meas2Card.lastPIDIoutX = pidData->Iout_i.x; addMeasValueHeader(&commaFlag, meas2Card.lastPIDIoutX); }
		if (meas2Card.measurePIDIoutY) { meas2Card.lastPIDIoutY = pidData->Iout_i.y; addMeasValueHeader(&commaFlag, meas2Card.lastPIDIoutY); }
		if (meas2Card.measurePIDIoutZ) { meas2Card.lastPIDIoutZ = pidData->Iout_i.z; addMeasValueHeader(&commaFlag, meas2Card.lastPIDIoutZ); }
		if (meas2Card.measurePIDDoutX) { meas2Card.lastPIDDoutX = pidData->Dout_i.x; addMeasValueHeader(&commaFlag, meas2Card.lastPIDDoutX); }
		if (meas2Card.measurePIDDoutY) { meas2Card.lastPIDDoutY = pidData->Dout_i.y; addMeasValueHeader(&commaFlag, meas2Card.lastPIDDoutY); }
		if (meas2Card.measurePIDDoutZ) { meas2Card.lastPIDDoutZ = pidData->Dout_i.z; addMeasValueHeader(&commaFlag, meas2Card.lastPIDDoutZ); }
		if (meas2Card.measurePIDFFoutX) { meas2Card.lastPIDFFoutX = pidData->FFout_i.x; addMeasValueHeader(&commaFlag, meas2Card.lastPIDFFoutX); }
		if (meas2Card.measurePIDFFoutY) { meas2Card.lastPIDFFoutY = pidData->FFout_i.y; addMeasValueHeader(&commaFlag, meas2Card.lastPIDFFoutY); }
		if (meas2Card.measurePIDFFoutZ) { meas2Card.lastPIDFFoutZ = pidData->FFout_i.z; addMeasValueHeader(&commaFlag, meas2Card.lastPIDFFoutZ); }
		if (meas2Card.measurePIDUX) { meas2Card.lastPIDUX = pidData->u_i.x; addMeasValueHeader(&commaFlag, meas2Card.lastPIDUX); }
		if (meas2Card.measurePIDUY) { meas2Card.lastPIDUY = pidData->u_i.y; addMeasValueHeader(&commaFlag, meas2Card.lastPIDUY); }
		if (meas2Card.measurePIDUZ) { meas2Card.lastPIDUZ = pidData->u_i.z; addMeasValueHeader(&commaFlag, meas2Card.lastPIDUZ); }
		//PID internals
		if (meas2Card.measurePIDrefSigDotPT1X) { meas2Card.lastPIDrefSigDotPT1X = pidData->refSigDotPT1_i.x; addMeasValueHeader(&commaFlag, meas2Card.lastPIDrefSigDotPT1X); }
		if (meas2Card.measurePIDrefSigDotPT1Y) { meas2Card.lastPIDrefSigDotPT1Y = pidData->refSigDotPT1_i.y; addMeasValueHeader(&commaFlag, meas2Card.lastPIDrefSigDotPT1Y); }
		if (meas2Card.measurePIDrefSigDotPT1Z) { meas2Card.lastPIDrefSigDotPT1Z = pidData->refSigDotPT1_i.z; addMeasValueHeader(&commaFlag, meas2Card.lastPIDrefSigDotPT1Z); }
		if (meas2Card.measurePIDiRelaxWeightX) { meas2Card.lastPIDiRelaxWeightX = pidData->iRelaxWeight.x; addMeasValueHeader(&commaFlag, meas2Card.lastPIDiRelaxWeightX); }
		if (meas2Card.measurePIDiRelaxWeightY) { meas2Card.lastPIDiRelaxWeightY = pidData->iRelaxWeight.y; addMeasValueHeader(&commaFlag, meas2Card.lastPIDiRelaxWeightY); }
		if (meas2Card.measurePIDiRelaxWeightZ) { meas2Card.lastPIDiRelaxWeightZ = pidData->iRelaxWeight.z; addMeasValueHeader(&commaFlag, meas2Card.lastPIDiRelaxWeightZ); }

		appendChar('\n');
	}
#ifdef LOG_SAVED_DATA
	SerialUSB.print("End of 5th line, loadingDataCounter: "); SerialUSB.println(SDcard.loadingDataCounter);
#endif
}

void prepSendingBuffer()
{
    //SDcard.sendingBuffer[0] = 0xFE //start token
    for (uint16_t i = 0; i < 512; i++)
    {
        SDcard.sendingBuffer[i + 1] = SDcard.measBuffer[SDcard.sendBufferIndex].data[i];
    }
    SDcard.sendBufferIndex++;
}

void writeData(uint64_t sysTick)
{
	//desired block? = 0x4000 + cluster*64 + blockcount
	//6th block is set in root, found data in 4th pos, a hardcoded -2 offset is here
    E_SDWriteStates l_writeState = writeBlock(
        SDcard.rootDirAddr + 
        (SDcard.newFile.clusters[SDcard.newFile.numberOfClusters - 1] - 2) * SDcard.blockPerCluster + 
        SDcard.newFile.blockCount, 
        SDcard.sendingBuffer);

	if (SDWRITE_FINISHED == l_writeState)
	{
		//if more data
		if (SDcard.sendBufferIndex < SDcard.measBufferCtr)
		{
            SDcard.newFile.blockCount++;
            SDcard.newFile.size += 512;
            if (SDcard.newFile.blockCount >= SDcard.blockPerCluster)
            {
                SDcard.newFile.clusters[SDcard.newFile.numberOfClusters] = SDcard.newFile.clusters[SDcard.newFile.numberOfClusters - 1] + 1;
                SDcard.newFile.blockCount = 0;
                SDcard.newFile.numberOfClusters++;
            }
            prepSendingBuffer();

			SDcard.SDWriteState = SDWRITE_START;
#ifdef LOG_SD_WRITE 
			SerialUSB.println("writeData done, write more MEAS");
			SerialUSB.print("newfile: "); printFileInfo(&SDcard.newFile);
#endif
		}
		else    //else goto write root then FAT
		{
			SDcard.MainState = SD_WRITE_ROOT;
			SDcard.SDWriteState = SDWRITE_START;

			//add newfile to rootdir
			addFileInfo2RootDir(&SDcard.rootDirInfo[1], &SDcard.newFile, sysTick);
            SDcard.rootOrFatWriteTick = sysTick;

#ifdef LOG_SD_WRITE 
            SerialUSB.println("writeData done, write ROOT");
			SerialUSB.print("newfile: "); printFileInfo(&SDcard.newFile);
#endif

			//testing
			//SDcard.MainState = SD_DO_NOTHING;
		}
	}
	else if (SDWRITE_FAILED == l_writeState)
	{
		//if data write failed finish, go next hoping it will be fine, 1 empty block
        
		SDcard.SDWriteState = SDWRITE_START;
#ifdef LOG_SD_WRITE 
		SerialUSB.println("writeData failed, skip this and continue");
#endif
	}
	else
	{
		//do nothing
	}
}

void writeRoot(uint64_t sysTick)
{
    if (sysTick - SDcard.rootOrFatWriteTick > ROOT_FAT_WRITE_DELAY)
    {
        E_SDWriteStates l_writeState = writeBlock(SDcard.rootDirAddr + SDcard.rootDirEmptyBlockNumber, SDcard.rootDirInfo);

        if (SDWRITE_FINISHED == l_writeState)
        {
            SDcard.MainState = SD_WRITE_FAT;
            SDcard.SDWriteState = SDWRITE_START;

            //add newfile to FAT
            SDcard.writingMultiFATBlock = addFileFATInfo(&SDcard.FAT1Info[1], &SDcard.newFile, E_SDFATWRITE_FIRST_CALL);
            SDcard.rootOrFatWriteTick = sysTick;

#ifdef LOG_SD_WRITE 
            SerialUSB.println("writeRoot done, write FAT");
#endif
}
        else if (SDWRITE_FAILED == l_writeState)
        {
            SDcard.MainState = SD_DO_NOTHING;
            LEDSDBlink();
            EnableGyroAccInt();
#ifdef LOG_SD_WRITE 
            SerialUSB.println("writeRoot failed");
#endif
        }
        else
        {
            //do nothing
        }
    }
}

void writeFAT(uint64_t sysTick)
{
    if (sysTick - SDcard.rootOrFatWriteTick > ROOT_FAT_WRITE_DELAY)
    {
        E_SDWriteStates l_writeState = writeBlock(SDcard.FAT1Addr + SDcard.FATBlockOffset, SDcard.FAT1Info);

        if (SDWRITE_FINISHED == l_writeState)
        {
            if (true == SDcard.writingMultiFATBlock)
            {
                //add newfile to FAT
                SDcard.writingMultiFATBlock = addFileFATInfo(&SDcard.FAT1Info[1], &SDcard.newFile, E_SDFATWRITE_CONSECUTIVE_CALL);

                //testing
                //SerialUSB.print("FATBlockOffset after: "); SerialUSB.println(FATBlockOffset);

                SDcard.SDWriteState = SDWRITE_START;
#ifdef LOG_SD_WRITE
                SerialUSB.println("writeFAT done, write next block");
#endif
            }
            else
            {
#ifdef LOG_SD_WRITE
                SerialUSB.print("newfile info before reset: "); printFileInfo(&SDcard.newFile);
#endif
                //finished writing everything of newfile, set up a new newFile
                SDcard.newFile.numberInName++;
                SDcard.newFile.clusters[0] = SDcard.newFile.clusters[SDcard.newFile.numberOfClusters - 1] + 1;
                SDcard.newFile.numberOfClusters = 1;
                SDcard.newFile.blockCount = 0;
                SDcard.newFile.size = 0;
                //reset variables
                SDcard.measBufferCtr = 0;
                SDcard.measDataCtr = 0;
                SDcard.sendBufferIndex = 0;
                SDcard.rootDirEmptyBlockNumber = 0;
                SDcard.rootDirEmptySlotNumber = 0;
                SDcard.FATBlockOffset = 0;

                //go back to wait meas
                SDcard.MainState = SD_WAIT_4_MEASUREMENT;
                LEDSDOff();
                EnableGyroAccInt();

#ifdef LOG_SD_WRITE
                SerialUSB.print("newfile info after reset: "); printFileInfo(&SDcard.newFile);
                SerialUSB.println("writeFAT done, go wait4Meas");
#endif

                //testing
                //SDcard.MainState = SD_DO_NOTHING;
            }
        }
        else if (SDWRITE_FAILED == l_writeState)
        {
            SDcard.MainState = SD_DO_NOTHING;
            LEDSDBlink();
            EnableGyroAccInt();
#ifdef LOG_SD_WRITE
            SerialUSB.println("writeFAT failed");
#endif
        }
        else
        {
            //do nothing?
        }
    }
}

SpiSDcard_st* getSPISdCard(void)
{
	return &SDcard;
}

void TriggerNextSdCardTxRx(void)
{
	SpiDmaTxRx(SDcard.nextTxBuffer, SDcard.nextRxBuffer, SDcard.nextCtr, DMAC_CHANNEL_SDCARD);
}

Meas2Card* getMeas2Card(void)
{
	return &meas2Card;
}

E_SDMainStates ResetMeasurement(void)
{
    SDcard.MainState = SD_WAIT_4_MEASUREMENT;
    SDcard.newFile.numberOfClusters = 1;
    SDcard.newFile.blockCount = 0;
    SDcard.newFile.size = 0;
    LEDSDOff();

    return SDcard.MainState;
}

E_SDMainStates ReinitSDCard(void)
{
    SDcard.MainState = SD_POST_INIT;
    SDcard.SDInitStatus = SDINIT_CMD0;
    SDcard.SDCommandState = SDCOMMAND_SEND;
    SDcard.SDReadState = SDREAD_START;

    SDcard.acmd41TryCtr = 0;

    return SDcard.MainState;
}

void setGlobalTime(const date newTime, const uint64_t currentSysTick)
{
	SDcard.globalDateAndTime.hour = newTime.hour;
	SDcard.globalDateAndTime.min = newTime.min;
	SDcard.globalDateAndTime.sec = newTime.sec;

	SDcard.sysTickAtGlobalTick = currentSysTick;
}

void setGlobalDate(const date newTime)
{
	SDcard.globalDateAndTime.year = newTime.year;
	SDcard.globalDateAndTime.month = newTime.month;
	SDcard.globalDateAndTime.day = newTime.day;
}

void DetectReInitAndWrite(const uint16_t switch2way)
{
    //detect switching down
    if (switch2way < 1500 && SDcard.lastSwitch2Way > 1500)
    {
        SDcard.MainState = SD_POST_INIT;
        SDcard.SDInitStatus = SDINIT_CMD0;
        SDcard.SDCommandState = SDCOMMAND_SEND;
        SDcard.acmd41TryCtr = 0;

        LEDSDOn();

        //disable gyro/acc interrupt while sd init and write
        DisableGyroAccInt();

        SDcard.postInitTick = getSysTick();
    }

    SDcard.lastSwitch2Way = switch2way;
}

void StripToLastLineEnd(void)
{
	uint16_t index = SDcard.measDataCtr;
	while (1)
	{
		while (index > 0)
		{
			index--;

			if (SDcard.measBuffer[SDcard.measBufferCtr].data[index] == '\n')
			{
				SDcard.measDataCtr = index + 1;
				return;
			}
		}

		// go to previous block
		if (SDcard.measBufferCtr == 0)
		{
			// No previous block exists
			SDcard.measDataCtr = 0;
			return;
		}

		SDcard.measBufferCtr--;
		index = 512;
	}
}

void AppendTrailingZeros(void)
{
	for (uint16_t i = SDcard.measDataCtr; i < 512; i++)
	{
		appendChar(0x00);
	}
}