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
//		xth line: header for measured signals, example: sysTime,Graw,Gpt1,Gpt2,Accraw,Accpt1,Accpt2,Akf,Akfaccpt1\n
// 
//	1st meas is systime
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

#define WRITE_BLOCK_TIMEOUT 0.3f

//#define LOG_SD_INIT
//#define LOG_SD_WRITE
//#define LOG_SAVED_DATA

extern spi_st SPI;

//fileInfo param = { 0 };
//uint8_t rootWriteDelayCycle = 0;    //number of cycles to delay writing root
//uint8_t FATWriteDelayCycle = 0;     //number of cycles to delay writing FAT 

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

void RunSdCard(SpiInput* spiInput, SPIOutput* spiOutput)
{
	switch (SDcard.MainState)
	{
		case SD_INIT:
		{
			SDcard.MainState = SetupSdCard();
			break;
		}
		case SD_WAIT_4_MEASUREMENT:
		{
			if (spiInput->rcSignals.measurementSwitch > 1500)
			{
				addMeasHeader();

                LEDSDOn();

				SDcard.MainState = SD_MEASUREMENT_ONGOING;
			}
			break;
		}
		case SD_MEASUREMENT_ONGOING:
		{
			//save data every 10ms
			if (spiInput->sysTime - SDcard.measTimePrev >= 0.01f)
			{
				saveMeasData(spiInput, spiOutput);

				SDcard.measTimePrev = spiInput->sysTime;
			}

			if (SDcard.writeMeasData)
			{
				writeData(spiInput->rcSignals.measurementSwitch, spiInput->sysTime);
			}

			break;
		}
		case SD_WRITE_ROOT:
		{
			writeRoot();

			break;
		}
		case SD_WRITE_FAT:
		{
			writeFAT();
			
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
	E_SDMainStates returnVal{ SD_INIT };

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
		LEDSDOff();
		returnVal = SD_WAIT_4_MEASUREMENT;
		break;
	}
	case SDINIT_FAILURE:
	{
		LEDSDBlink();
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
		//reset init flag
		SDcard.sdCardInitFinished = false;

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
                
                //allow gyro and acc to run even if no card
                SDcard.sdCardInitFinished = true;

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
				returnValCMD55 = SDINIT_ACMD41;
#ifdef LOG_SD_INIT 
				SerialUSB.println("CMD55 done");
#endif
			}
			else
			{
				returnValCMD55 = SDINIT_FAILURE;
#ifdef LOG_SD_INIT 
				SerialUSB.println("CMD55 failed");
#endif
			}
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

		intSafeTriggerSDRxTx(SDcard.SdTx, SDcard.SdRx, SDcard.SdCtr);
		//set command state to waiting
		SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	}
	else
	{
		if (INACTIVE == SDcard.spiActivitySDCard)
		{
			if (0 == SDcard.SdRx[SDcard.SdCtr - 1])
			{
				SDcard.SDCommandState = SDCOMMAND_SEND;
				returnValACMD41 = SDINIT_READ_00;
#ifdef LOG_SD_INIT 
				SerialUSB.println("ACMD41 done");
#endif
			}
			else
			{
				//instead go back to 55 and retry
				SDcard.SDCommandState = SDCOMMAND_SEND;
				returnValACMD41 = SDINIT_CMD55;
#ifdef LOG_SD_INIT 
				SerialUSB.println("ACMD41 failed, back to CMD55");
#endif
			}
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

		//testing
		//SerialUSB.println("Boot data:");
		//SerialUSB.print("block/cluster  : "); SerialUSB.println(blockPerCluster); //64
		//SerialUSB.print("reserved sector: "); SerialUSB.println(reservedSectors); //602
		//SerialUSB.print("number of FATs : "); SerialUSB.println(numberOfFATs);    //2
		//SerialUSB.print("sectors/FAT    : "); SerialUSB.println(sectorsPerFAT);   //3795
		//SerialUSB.print("FAT1 address   : 0x"); SerialUSB.println(FAT1Addr, HEX);   //0x25A
		//SerialUSB.print("RootDir address: 0x"); SerialUSB.println(rootDirAddr, HEX);//0x2000

		if (0x2000 != SDcard.rootDirAddr && 0x25A != SDcard.FAT1Addr)
		{
			returnValBootState = SDINIT_FAILURE;
#ifdef LOG_SD_INIT 
			SerialUSB.println("readBoot failed, wrong addresses");
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
		//testing
#if 1==SD_DELETE_ROOT_ON_READ
		if (SDcard.rootDirEmptyBlockNumber == 1)
		{
			for (uint16_t i = 0 * 32; i < 3 * 32; i++)
			{
				SDcard.rootDirInfo_8b[i] = 0;
			}
		}
#endif              

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
		//testing
		//for (uint16_t i = 119 * 4; i < 128 * 4; i++)
		//{
		//    FAT1Info_8b[i] = 0;
		//}

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
				//do nothing, shouldnt enter
				//SerialUSB.println("Error: in a shouldnt enter else!");
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
			SDcard.sdCardInitFinished = true;

#ifdef LOG_SD_WRITE 
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
		}
	}
	else if (SDREAD_FAILED == readStateFAT)
	{
		returnValFATState = SDINIT_FAILURE;
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
			SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //answer
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
					//send dummy to check for response
					SDcard.SdCtr = 0;
					SDcard.SdTx[SDcard.SdCtr++] = 0xFF | SPI_TDR_PCS(CS_SDCARD); //response
					intSafeTriggerSDRxTx(SDcard.SdTx, SDcard.SdRx, SDcard.SdCtr);
					//go to wait state
					SDcard.SDReadState = SDREAD_WAIT_FE;
				}
				else
				{
					SDcard.SDReadState = SDREAD_FAILED;
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

        SDcard.writeStartTime = getSysTime();
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
        SDcard.writeStartTime = getSysTime();

        SDcard.SdTx[0] = 0xFF | SPI_TDR_PCS(CS_SDCARD);
        triggerSDRxTx(SDcard.SdTx, SDcard.SdRx, 1);
        SDcard.SDWriteState = SDWRITE_WAIT_WRITE_FINISH;
    }
    else
    {
        //timeout check
        if (getSysTime() - SDcard.writeStartTime > WRITE_BLOCK_TIMEOUT)
        {
            SDcard.SDWriteState = SDWRITE_FAILED;
#ifdef LOG_SD_WRITE
            SerialUSB.print("writeBlock failed, wrong data end flag timed out: ");
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
			return SDWRITE_FAILED;
		}

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
#ifdef LOG_SD_WRITE 
			SerialUSB.print("writeBlock to offset: "); SerialUSB.println(blockOffset);
#endif

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
				if (getSysTime() - SDcard.writeStartTime > WRITE_BLOCK_TIMEOUT)
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

void setFileTime(volatile uint32_t* block, float sysTime)
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


	uint16_t elapsedTimeSinceSync = uint16_t(sysTime - SDcard.sysTimeAtGlobalTime);
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

void addFileInfo2RootDir(volatile uint32_t* block, fileInfo* file, float sysTime)
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
	setFileTime(block, sysTime);

	//low byte set
	//size set
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

			//SerialUSB.print("firstBlock: "); SerialUSB.println(firstBlock);
			//SerialUSB.print("lastBlock: "); SerialUSB.println(lastBlock);
			//SerialUSB.print("iterBeg: "); SerialUSB.println(iterBeg);

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

		/*SerialUSB.print(block[4 * i + 0]); SerialUSB.print(" ");
		SerialUSB.print(block[4 * i + 1]); SerialUSB.print(" ");
		SerialUSB.print(block[4 * i + 2]); SerialUSB.print(" ");
		SerialUSB.println(block[4 * i + 3]);*/
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
		SerialUSB.print(" - clusters: ");
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

void appendComma(void)
{
	SDcard.loadingDataPointer[SDcard.loadingDataCounter++] = ',';
}

void appendNewLine(void)
{
	SDcard.loadingDataPointer[SDcard.loadingDataCounter++] = '\n';
}

void measureData(bool isMeasured, bool isCommaed, float data, uint8_t numberOfFrac, bool isExplicitPlus, char* debugName)
{
    if (isMeasured)
    {
        uint32_t tempBuffer[30];
        uint8_t numberOfCharacters{ 0 };

        if(isCommaed) appendComma();
        convert2String(tempBuffer, &numberOfCharacters, data, numberOfFrac, isExplicitPlus);
        loadData2Buffer(tempBuffer, numberOfCharacters);
#ifdef LOG_SAVED_DATA
        SerialUSB.print(debugName);SerialUSB.println(data, numberOfFrac);
#endif
    }
}

void saveMeasData(SpiInput* spiInput, SPIOutput* spiOutput)
{
    pid_st* pidRate{ getPIDrates() };
    gyroData_st* gyroData{ getGyroData() };
    accData_st* accData{ getAccData() };
    pid_st* pidData{ getPIDrates() };
    
    //timestamp
	//todo: handle case if systime is not measured
    measureData(meas2Card.measureSysTime, false, spiInput->sysTime, 3, false, "timestamp: ");
    //gyro
    measureData(meas2Card.measureGyroRawX, true, spiInput->gyro.signal.x, 3, false, "gyroRawX: ");
    measureData(meas2Card.measureGyroRawY, true, spiInput->gyro.signal.y, 3, false, "gyroRawY: ");
    measureData(meas2Card.measureGyroRawZ, true, spiInput->gyro.signal.z, 3, false, "gyroRawZ: ");
    measureData(meas2Card.measureGyroPT1X, true, gyroData->PT1.signal.x, 3, false, "gyroPT1X: ");
    measureData(meas2Card.measureGyroPT1Y, true, gyroData->PT1.signal.y, 3, false, "gyroPT1Y: ");
    measureData(meas2Card.measureGyroPT1Z, true, gyroData->PT1.signal.z, 3, false, "gyroPT1Z: ");
    measureData(meas2Card.measureGyroPT2X, true, gyroData->PT2.signal.x, 3, false, "gyroPT2X: ");
    measureData(meas2Card.measureGyroPT2Y, true, gyroData->PT2.signal.y, 3, false, "gyroPT2Y: ");
    measureData(meas2Card.measureGyroPT2Z, true, gyroData->PT2.signal.z, 3, false, "gyroPT2Z: ");
    //acc
    measureData(meas2Card.measureAccRawX, true, spiInput->acc.signal.x, 3, false, "accRawX: ");
    measureData(meas2Card.measureAccRawY, true, spiInput->acc.signal.y, 3, false, "accRawY: ");
    measureData(meas2Card.measureAccRawZ, true, spiInput->acc.signal.z, 3, false, "accRawZ: ");
    measureData(meas2Card.measureAccPT1X, true, accData->PT1.signal.x, 3, false, "accPT1X: ");
    measureData(meas2Card.measureAccPT1Y, true, accData->PT1.signal.y, 3, false, "accPT1Y: ");
    measureData(meas2Card.measureAccPT1Z, true, accData->PT1.signal.z, 3, false, "accPT1Z: ");
    measureData(meas2Card.measureAccPT2X, true, accData->PT2.signal.x, 3, false, "accPT2X: ");
    measureData(meas2Card.measureAccPT2Y, true, accData->PT2.signal.y, 3, false, "accPT2Y: ");
    measureData(meas2Card.measureAccPT2Z, true, accData->PT2.signal.z, 3, false, "accPT2Z: ");
    //angle
    measureData(meas2Card.measureAngleRawRoll, true, accData->rollAngle, 3, false, "rollAngleRaw: ");
    measureData(meas2Card.measureAngleRawPitch, true, accData->pitchAngle, 3, false, "pitchAngleRaw: ");
    measureData(meas2Card.measureAnglePT1Roll, true, accData->rollAnglePT1Acc, 3, false, "rollAnglePT1: ");
    measureData(meas2Card.measureAnglePT1Pitch, true, accData->pitchAnglePT1Acc, 3, false, "pitchAnglePT2: ");
    measureData(meas2Card.measureAnglePT2Roll, true, accData->rollAnglePT2Acc, 3, false, "rollAnglePT2: ");
    measureData(meas2Card.measureAnglePT2Pitch, true, accData->pitchAnglePT2Acc, 3, false, "pitchAnglePT2: ");
    measureData(meas2Card.measureAngleKFRawRoll, true, accData->angleKF.roll.angle, 3, false, "angleKFRoll: ");
    measureData(meas2Card.measureAngleKFRawPitch, true, accData->angleKF.pitch.angle, 3, false, "angleKFPitch: ");
    measureData(meas2Card.measureAngleKFPT10Roll, true, accData->angleKFPT10.roll.angle, 3, false, "angleKFPT10Roll: ");
    measureData(meas2Card.measureAngleKFPT10Pitch, true, accData->angleKFPT10.pitch.angle, 3, false, "angleKFPT10Pitch: ");
    measureData(meas2Card.measureAngleKFPT20Roll, true, accData->angleKFPT20.roll.angle, 3, false, "angleKFPT20Roll: ");
    measureData(meas2Card.measureAngleKFPT20Pitch, true, accData->angleKFPT20.pitch.angle, 3, false, "angleKFPT20Pitch: ");
    measureData(meas2Card.measureAngleKFPT11Roll, true, accData->angleKFPT11.roll.angle, 3, false, "angleKFPT11Roll: ");
    measureData(meas2Card.measureAngleKFPT11Pitch, true, accData->angleKFPT11.pitch.angle, 3, false, "angleKFPT11Pitch: ");
    measureData(meas2Card.measureAngleKFPT21Roll, true, accData->angleKFPT21.roll.angle, 3, false, "angleKFPT21Roll: ");
    measureData(meas2Card.measureAngleKFPT21Pitch, true, accData->angleKFPT21.pitch.angle, 3, false, "angleKFPT21Pitch: ");
    measureData(meas2Card.measureAngleKFPT22Roll, true, accData->angleKFPT22.roll.angle, 3, false, "angleKFPT22Roll: ");
    measureData(meas2Card.measureAngleKFPT22Pitch, true, accData->angleKFPT22.pitch.angle, 3, false, "angleKFPT22Pitch: ");
	measureData(meas2Card.measureAngleCFRawRoll, true, accData->rollAngleCF, 3, false, "angleCFRoll: ");
	measureData(meas2Card.measureAngleCFRawPitch, true, accData->pitchAngleCF, 3, false, "angleCFPitch: ");
	measureData(meas2Card.measureAngleCFPT10Roll, true, accData->rollAngleCF10, 3, false, "angleCFPT10Roll: ");
	measureData(meas2Card.measureAngleCFPT10Pitch, true, accData->pitchAngleCF10, 3, false, "angleCFPT10Pitch: ");
	measureData(meas2Card.measureAngleCFPT11Roll, true, accData->rollAngleCF11, 3, false, "angleCFPT11Roll: ");
	measureData(meas2Card.measureAngleCFPT11Pitch, true, accData->pitchAngleCF11, 3, false, "angleCFPT11Pitch: ");
	measureData(meas2Card.measureAngleCFWeightedRawRoll, true, accData->rollAngleCFw, 3, false, "angleCFWeightedRoll: ");
	measureData(meas2Card.measureAngleCFWeightedRawPitch, true, accData->pitchAngleCFw, 3, false, "angleCFWeightedPitch: ");
	measureData(meas2Card.measureAngleCFWeightedPT01Roll, true, accData->rollAngleCFw01, 3, false, "angleCFWeightedPT01Roll: ");
	measureData(meas2Card.measureAngleCFWeightedPT01Pitch, true, accData->pitchAngleCFw01, 3, false, "angleCFWeightedPT01 Pitch: ");
    //PID control
    measureData(meas2Card.measurePIDRefsigX, true, pidData->refSignal.x, 3, false, "PIDRefSigX: ");
    measureData(meas2Card.measurePIDRefsigY, true, pidData->refSignal.y, 3, false, "PIDRefSigY: ");
    measureData(meas2Card.measurePIDRefsigZ, true, pidData->refSignal.z, 3, false, "PIDRefSigZ: ");
    measureData(meas2Card.measurePIDSensorX, true, pidData->sensor.signal.x, 3, false, "PIDSensorX: ");
    measureData(meas2Card.measurePIDSensorY, true, pidData->sensor.signal.y, 3, false, "PIDSensorY: ");
    measureData(meas2Card.measurePIDSensorZ, true, pidData->sensor.signal.z, 3, false, "PIDSensorZ: ");
    measureData(meas2Card.measurePIDPoutX, true, pidData->Pout.x, 3, false, "PIDPoutX: ");
    measureData(meas2Card.measurePIDPoutY, true, pidData->Pout.y, 3, false, "PIDPoutY: ");
    measureData(meas2Card.measurePIDPoutZ, true, pidData->Pout.z, 3, false, "PIDPoutZ: ");
    measureData(meas2Card.measurePIDIoutX, true, pidData->Iout.x, 3, false, "PIDIoutX: ");
    measureData(meas2Card.measurePIDIoutY, true, pidData->Iout.y, 3, false, "PIDIoutY: ");
    measureData(meas2Card.measurePIDIoutZ, true, pidData->Iout.z, 3, false, "PIDIoutZ: ");
    measureData(meas2Card.measurePIDDoutX, true, pidData->Dout.x, 3, false, "PIDDoutX: ");
    measureData(meas2Card.measurePIDDoutY, true, pidData->Dout.y, 3, false, "PIDDoutY: ");
    measureData(meas2Card.measurePIDDoutZ, true, pidData->Dout.z, 3, false, "PIDDoutZ: ");
    measureData(meas2Card.measurePIDFFoutX, true, pidData->FFout.x, 3, false, "PIDFFoutX: ");
    measureData(meas2Card.measurePIDFFoutY, true, pidData->FFout.y, 3, false, "PIDFFoutY: ");
    measureData(meas2Card.measurePIDFFoutZ, true, pidData->FFout.z, 3, false, "PIDFFoutZ: ");
    measureData(meas2Card.measurePIDUX, true, pidData->u.x, 3, false, "PIDUX: ");
    measureData(meas2Card.measurePIDUY, true, pidData->u.y, 3, false, "PIDUY: ");
    measureData(meas2Card.measurePIDUZ, true, pidData->u.z, 3, false, "PIDUZ: ");

	appendNewLine();
}

void convert2String(uint32_t* buffer, uint8_t* numberOfChar, float value2Convert, uint8_t numberOfFractions, bool explicitPlusSign)
{
	if (value2Convert < 0) //if its negative, append a - sign and make it positive so the rest of the algo is the same
	{
		buffer[(*numberOfChar)++] = '-';
		value2Convert *= -1;
	}
	else if (true == explicitPlusSign) //if flag is true append a + sign explicitly
	{
		buffer[(*numberOfChar)++] = '+';
	}

	//increment with a small value to solve the numerical accurecy problem, (without this a 0.005 would be 0.0049)
	value2Convert += pow(10, -(numberOfFractions + 2));

	if (value2Convert >= 1.0)  //if the value is is not smaller then 1
	{
		uint8_t current_counter = 0;

		while (value2Convert >= 10.0)
		{
			value2Convert /= 10;
			current_counter++;
		}

		for (uint8_t i = 0; i <= current_counter; i++) //creating integer part
		{
			uint8_t current_integer;

			//creat the integer part
			current_integer = (uint8_t)value2Convert;

			//set correct ASCII char for sending
			buffer[(*numberOfChar)++] = '0' + current_integer;

			//substitute it from the original
			value2Convert -= (float)current_integer;

			//multiplie by 10 for the next round
			value2Convert *= 10;
		}

		if (numberOfFractions > 0) //if we need fractional part
		{
			buffer[(*numberOfChar)++] = '.'; //append dot

			for (uint8_t f = 0; f < numberOfFractions; f++)  //creating fractional part
			{
				uint8_t current_fractional;

				current_fractional = (uint8_t)value2Convert;  //creat the integer part
				buffer[(*numberOfChar)++] = '0' + current_fractional; //set correct ASCII char for sending
				value2Convert -= (float)current_fractional; //substitute it from the original
				value2Convert *= 10;  //multiplie by 10 for the next round
			}
		}
	}
	else  //if the value is small then 1
	{
		buffer[(*numberOfChar)++] = '0'; //append zero

		if (numberOfFractions > 0) //if we need fractional part
		{
			buffer[(*numberOfChar)++] = '.'; //append dot

			for (uint8_t f = 0; f < numberOfFractions; f++)  //creating fractional part
			{
				uint8_t current_fractional;

				//move decimal value up to integer part
				value2Convert *= 10;

				//creat the integer part
				current_fractional = (uint8_t)value2Convert;

				//write to returnString
				buffer[(*numberOfChar)++] = '0' + current_fractional; //set correct ASCII char for sending

				//substitute it from the original
				value2Convert -= (float)current_fractional;
			}
		}
	}
}

void loadData2Buffer(uint32_t* chars2Add, uint8_t numberOfChar)
{
	//if data can fit into loadingDataBuffer
	if (SDcard.loadingDataCounter <= (513 - numberOfChar))    //1+512-8
	{
		//load data over
		for (uint16_t ctr = 0; ctr < numberOfChar; ctr++)
		{
			SDcard.loadingDataPointer[SDcard.loadingDataCounter++] = chars2Add[ctr];
		}

		if (SDcard.loadingDataCounter == 513)
		{
			swapDataBufferPointers();
		}
	}
	else  //if data needs to be split
	{
		uint8_t l_remainingSize = 513 - SDcard.loadingDataCounter;

		//fill remaining data spots
		for (uint16_t ctr = 0; ctr < l_remainingSize; ctr++)
		{
			SDcard.loadingDataPointer[SDcard.loadingDataCounter++] = chars2Add[ctr];
		}

		swapDataBufferPointers();

		//fill remaining data to fresh buffer
		for (uint16_t ctrRemaining = l_remainingSize; ctrRemaining < numberOfChar; ctrRemaining++)
		{
			SDcard.loadingDataPointer[SDcard.loadingDataCounter++] = chars2Add[ctrRemaining];
		}
	}
}

void swapDataBufferPointers(void)
{
	uint32_t* l_tempDataPointer = SDcard.loadingDataPointer;

	SDcard.loadingDataPointer = SDcard.sendingDataPointer;
	SDcard.sendingDataPointer = l_tempDataPointer;
	SDcard.loadingDataCounter = 1;	//0th is start token

	//check if blockcount reached the limit of 64
	//check is done before the increment as blockcount is used later for writing into SD!
	if (SDcard.newFile.blockCount >= 64)
	{
		SDcard.newFile.clusters[SDcard.newFile.numberOfClusters] = SDcard.newFile.clusters[SDcard.newFile.numberOfClusters - 1] + 1;
		SDcard.newFile.blockCount = 0;
		SDcard.newFile.numberOfClusters++;
	}

	SDcard.newFile.size += 512;
	SDcard.newFile.blockCount++;

	//reset for write
	SDcard.writeMeasData = true;
	SDcard.SDWriteState = SDWRITE_START;
}

void addMeasNameHeader(bool isMeasured, bool isCommaed, char* name, uint8_t numberOfChar)
{
    if (isMeasured)
    {
        uint32_t tempBuffer[10];
        uint8_t numberOfCharacters{ 0 };

        if (isCommaed) appendComma();

        for (uint8_t i = 0; i < numberOfChar; i++)
        {
            tempBuffer[numberOfCharacters++] = name[i];
        }

        loadData2Buffer(tempBuffer, numberOfCharacters);
#ifdef LOG_SAVED_DATA
        SerialUSB.print("Measured name: "); SerialUSB.println(name);
#endif
    }
}

//1st line : R for rate
//2nd line: Px,I,D,Py,I,D,Pz,I,FFrx,y,FFdrx,y,satI,satPID,DTermC\n
//3rd line: values
//4th line: C for cacade
//...
//...
//xth line: header for measured signals, example: sysTime,Grawx,Gpt1x,Gpt2x,Accrawx,Accpt1x,Accpt2x,Akfx,Akfaccpt1x\n
void addMeasHeader(void)
{
	//1st line
	SDcard.loadingDataCounter = 1;	//0th is start token($)
	SDcard.loadingDataPointer[SDcard.loadingDataCounter++] = 'R';
	SDcard.loadingDataPointer[SDcard.loadingDataCounter++] = '\n';
	//2nd line
	{
        addMeasNameHeader(true, false, "Px", 2);
        addMeasNameHeader(true, true, "Ix", 2);
        addMeasNameHeader(true, true, "Dx", 2);
        addMeasNameHeader(true, true, "Py", 2);
        addMeasNameHeader(true, true, "Iy", 2);
        addMeasNameHeader(true, true, "Dy", 2);
        addMeasNameHeader(true, true, "Pz", 2);
        addMeasNameHeader(true, true, "Iz", 2);
        addMeasNameHeader(true, true, "FFrx", 4);
        addMeasNameHeader(true, true, "FFry", 4);
        addMeasNameHeader(true, true, "FFdrx", 5);
        addMeasNameHeader(true, true, "FFdry", 5);
        addMeasNameHeader(true, true, "satI", 4);
        addMeasNameHeader(true, true, "satPID", 6);
        addMeasNameHeader(true, true, "DTermC", 6);
        addMeasNameHeader(true, true, "GparamC", 7);
        addMeasNameHeader(true, true, "AparamC", 7);
        addMeasNameHeader(true, true, "KFQAng", 6);
        addMeasNameHeader(true, true, "KFQbias", 7);
        addMeasNameHeader(true, true, "KFRmeas", 7);
        addMeasNameHeader(true, true, "CPx", 3);
        addMeasNameHeader(true, true, "CIx", 3);
		addMeasNameHeader(true, true, "CPy", 3);
		addMeasNameHeader(true, true, "CIy", 3);
		addMeasNameHeader(true, true, "CsatI", 5);
		addMeasNameHeader(true, true, "CsatPID", 7);
		addMeasNameHeader(true, true, "CFFdrx", 6);
		addMeasNameHeader(true, true, "CFFdry", 6);
		SDcard.loadingDataPointer[SDcard.loadingDataCounter++] = '\n';
	}
	//3rd line
	{
		uint32_t tempBuffer[100];
		uint8_t numberOfCharacters{ 0 };
		pid_st* pidRate{ getPIDrates() };
		pid_st* pidCascade{ getPIDcascade() };
        gyroData_st* gyro{ getGyroData() };
        accData_st* acc{ getAccData() };

		convert2String(tempBuffer, &numberOfCharacters, pidRate->P.x, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidRate->I.x, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidRate->D.x, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidRate->P.y, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidRate->I.y, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidRate->D.y, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidRate->P.z, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidRate->I.z, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidRate->FFr.x, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidRate->FFr.y, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidRate->FFdr.x, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidRate->FFdr.y, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidRate->saturationI, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidRate->saturationPID, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidRate->DTermC, 0, false);
        tempBuffer[numberOfCharacters++] = ',';
        convert2String(tempBuffer, &numberOfCharacters, gyro->paramC, 1, false);
        tempBuffer[numberOfCharacters++] = ',';
        convert2String(tempBuffer, &numberOfCharacters, acc->paramC, 1, false);
        tempBuffer[numberOfCharacters++] = ',';
        convert2String(tempBuffer, &numberOfCharacters, acc->q_angle, 6, false);
        tempBuffer[numberOfCharacters++] = ',';
        convert2String(tempBuffer, &numberOfCharacters, acc->q_bias, 6, false);
        tempBuffer[numberOfCharacters++] = ',';
        convert2String(tempBuffer, &numberOfCharacters, acc->r_measure, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidCascade->P.x, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidCascade->I.x, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidCascade->P.y, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidCascade->I.y, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidCascade->saturationI, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidCascade->saturationPID, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidCascade->FFdr.x, 0, false);
		tempBuffer[numberOfCharacters++] = ',';
		convert2String(tempBuffer, &numberOfCharacters, pidCascade->FFdr.y, 0, false);
		tempBuffer[numberOfCharacters++] = '\n';

		loadData2Buffer(tempBuffer, numberOfCharacters);
	}
	//4th line: measured values
	{
        //systime
        addMeasNameHeader(meas2Card.measureSysTime, false, "sysTime", 7);
        //gyro
        addMeasNameHeader(meas2Card.measureGyroRawX, true, "GRawX", 5);
        addMeasNameHeader(meas2Card.measureGyroRawY, true, "GRawY", 5);
        addMeasNameHeader(meas2Card.measureGyroRawZ, true, "GRawZ", 5);
        addMeasNameHeader(meas2Card.measureGyroPT1X, true, "GPT1X", 5);
        addMeasNameHeader(meas2Card.measureGyroPT1Y, true, "GPT1Y", 5);
        addMeasNameHeader(meas2Card.measureGyroPT1Z, true, "GPT1Z", 5);
        addMeasNameHeader(meas2Card.measureGyroPT2X, true, "GPT2X", 5);
        addMeasNameHeader(meas2Card.measureGyroPT2Y, true, "GPT2Y", 5);
        addMeasNameHeader(meas2Card.measureGyroPT2Z, true, "GPT2Z", 5);
        //acc
        addMeasNameHeader(meas2Card.measureAccRawX, true, "ARawX", 5);
        addMeasNameHeader(meas2Card.measureAccRawY, true, "ARawY", 5);
        addMeasNameHeader(meas2Card.measureAccRawZ, true, "ARawZ", 5);
        addMeasNameHeader(meas2Card.measureAccPT1X, true, "APT1X", 5);
        addMeasNameHeader(meas2Card.measureAccPT1Y, true, "APT1Y", 5);
        addMeasNameHeader(meas2Card.measureAccPT1Z, true, "APT1Z", 5);
        addMeasNameHeader(meas2Card.measureAccPT2X, true, "APT2X", 5);
        addMeasNameHeader(meas2Card.measureAccPT2Y, true, "APT2Y", 5);
        addMeasNameHeader(meas2Card.measureAccPT2Z, true, "APT2Z", 5);
        //angle
        addMeasNameHeader(meas2Card.measureAngleRawRoll, true, "aRawR", 5);
        addMeasNameHeader(meas2Card.measureAngleRawPitch, true, "aRawP", 5);
        addMeasNameHeader(meas2Card.measureAnglePT1Roll, true, "aPT1R", 5);
        addMeasNameHeader(meas2Card.measureAnglePT1Pitch, true, "aPT1P", 5);
        addMeasNameHeader(meas2Card.measureAnglePT2Roll, true, "aPT2R", 5);
        addMeasNameHeader(meas2Card.measureAnglePT2Pitch, true, "aPT2P", 5);
        addMeasNameHeader(meas2Card.measureAngleKFRawRoll, true, "aKFRawR", 7);
        addMeasNameHeader(meas2Card.measureAngleKFRawPitch, true, "aKFRawP", 7);
        addMeasNameHeader(meas2Card.measureAngleKFPT10Roll, true, "aKFPT10R", 8);
        addMeasNameHeader(meas2Card.measureAngleKFPT10Pitch, true, "aKFPT10P", 8);
        addMeasNameHeader(meas2Card.measureAngleKFPT20Roll, true, "aKFPT20R", 8);
        addMeasNameHeader(meas2Card.measureAngleKFPT20Pitch, true, "aKFPT20P", 8);
        addMeasNameHeader(meas2Card.measureAngleKFPT11Roll, true, "aKFPT11R", 8);
        addMeasNameHeader(meas2Card.measureAngleKFPT11Pitch, true, "aKFPT11P", 8);
        addMeasNameHeader(meas2Card.measureAngleKFPT21Roll, true, "aKFPT21R", 8);
        addMeasNameHeader(meas2Card.measureAngleKFPT21Pitch, true, "aKFPT21P", 8);
        addMeasNameHeader(meas2Card.measureAngleKFPT22Roll, true, "aKFPT22R", 8);
        addMeasNameHeader(meas2Card.measureAngleKFPT22Pitch, true, "aKFPT22P", 8);
        addMeasNameHeader(meas2Card.measureAngleCFRawRoll, true, "aCFRawR", 7);
        addMeasNameHeader(meas2Card.measureAngleCFRawPitch, true, "aCFRawP", 7);
		addMeasNameHeader(meas2Card.measureAngleCFPT10Roll, true, "aCFPT10R", 8);
		addMeasNameHeader(meas2Card.measureAngleCFPT10Pitch, true, "aCFPT10P", 8);
		addMeasNameHeader(meas2Card.measureAngleCFPT11Roll, true, "aCFPT11R", 8);
		addMeasNameHeader(meas2Card.measureAngleCFPT11Pitch, true, "aCFPT11P", 8);
		addMeasNameHeader(meas2Card.measureAngleCFWeightedRawRoll, true, "aCFwRawR", 8);
		addMeasNameHeader(meas2Card.measureAngleCFWeightedRawPitch, true, "aCFwRawP", 8);
		addMeasNameHeader(meas2Card.measureAngleCFWeightedPT01Roll, true, "aCFwPT01R", 9);
		addMeasNameHeader(meas2Card.measureAngleCFWeightedPT01Pitch, true, "aCFwPT01P", 9);
        //PID control
        addMeasNameHeader(meas2Card.measurePIDRefsigX, true, "PIDRefX", 7);
        addMeasNameHeader(meas2Card.measurePIDRefsigY, true, "PIDRefY", 7);
        addMeasNameHeader(meas2Card.measurePIDRefsigZ, true, "PIDRefZ", 7);
        addMeasNameHeader(meas2Card.measurePIDSensorX, true, "PIDSensX", 8);
        addMeasNameHeader(meas2Card.measurePIDSensorY, true, "PIDSensY", 8);
        addMeasNameHeader(meas2Card.measurePIDSensorZ, true, "PIDSensZ", 8);
        addMeasNameHeader(meas2Card.measurePIDPoutX, true, "PIDPoutX", 8);
        addMeasNameHeader(meas2Card.measurePIDPoutY, true, "PIDPoutY", 8);
        addMeasNameHeader(meas2Card.measurePIDPoutZ, true, "PIDPoutZ", 8);
        addMeasNameHeader(meas2Card.measurePIDIoutX, true, "PIDIoutX", 8);
        addMeasNameHeader(meas2Card.measurePIDIoutY, true, "PIDIoutY", 8);
        addMeasNameHeader(meas2Card.measurePIDIoutZ, true, "PIDIoutZ", 8);
        addMeasNameHeader(meas2Card.measurePIDDoutX, true, "PIDDoutX", 8);
        addMeasNameHeader(meas2Card.measurePIDDoutY, true, "PIDDoutY", 8);
        addMeasNameHeader(meas2Card.measurePIDDoutZ, true, "PIDDoutZ", 8);
        addMeasNameHeader(meas2Card.measurePIDFFoutX, true, "PIDFFoutX", 9);
        addMeasNameHeader(meas2Card.measurePIDFFoutY, true, "PIDFFoutY", 9);
        addMeasNameHeader(meas2Card.measurePIDFFoutZ, true, "PIDFFoutZ", 9);
        addMeasNameHeader(meas2Card.measurePIDUX, true, "PIDUX", 5);
        addMeasNameHeader(meas2Card.measurePIDUY, true, "PIDUY", 5);
        addMeasNameHeader(meas2Card.measurePIDUZ, true, "PIDUZ", 5);


        appendNewLine();
	}
}

void writeData(uint16_t measSwitch, float sysTime)
{
	//desired block? = 0x4000 + cluster*64 + blockcount
	//6th block is set in root, found data in 4th pos, a hardcoded -2 offset is here --------------------------------------------->|<-
    E_SDWriteStates l_writeState = writeBlock(SDcard.rootDirAddr + (SDcard.newFile.clusters[SDcard.newFile.numberOfClusters - 1] - 2) * SDcard.blockPerCluster + SDcard.newFile.blockCount - 1, SDcard.sendingDataPointer);

	if (SDWRITE_FINISHED == l_writeState)
	{
		//reset meas data flag
		SDcard.writeMeasData = false;

		//if more data
		if (measSwitch > 1800)
		{
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
			addFileInfo2RootDir(&SDcard.rootDirInfo[1], &SDcard.newFile, sysTime);
#ifdef LOG_SD_WRITE 
			SerialUSB.println("writeData done, write ROOT");
			SerialUSB.print("newfile: "); printFileInfo(&SDcard.newFile);
#endif

			//testing
			//SDcard.MainState = SD_DO_NOTHING;

#if 1==SD_DELETE_ROOT_ON_WRITE
			for (uint16_t i = 0 * 32; i < 3 * 32; i++)
			{
				rootDirInfo[i+1] = 0 | SPI_TDR_PCS(CS_SDCARD);
			}
#endif
		}
	}
	else if (SDWRITE_FAILED == l_writeState)
	{
		//if data write failed finish, go next hoping it will be fine, 1 empty block

		//reset meas data flag
		SDcard.writeMeasData = false;

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

void writeRoot(void)
{
	E_SDWriteStates l_writeState = writeBlock(SDcard.rootDirAddr + SDcard.rootDirEmptyBlockNumber, SDcard.rootDirInfo);

	if (SDWRITE_FINISHED == l_writeState)
	{
		SDcard.MainState = SD_WRITE_FAT;
		SDcard.SDWriteState = SDWRITE_START;

		//testing
#if 1==SD_DELETE_FAT_ON_WRITE
		for (uint16_t i = 0 * 4; i < 10 * 4; i++)
		{
			FAT1Info[i+1] = 0 | SPI_TDR_PCS(CS_SDCARD);
		}
#endif

		//add newfile to FAT
		SDcard.writingMultiFATBlock = addFileFATInfo(&SDcard.FAT1Info[1], &SDcard.newFile, E_SDFATWRITE_FIRST_CALL);
#ifdef LOG_SD_WRITE 
		SerialUSB.println("writeRoot done, write FAT");
#endif
	}
	else if (SDWRITE_FAILED == l_writeState)
	{
		SDcard.MainState = SD_DO_NOTHING;
        LEDSDBlink();
#ifdef LOG_SD_WRITE 
		SerialUSB.println("writeRoot failed");
#endif
	}
	else
	{
		//do nothing?
	}
}

void writeFAT(void)
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
			//go back to wait meas
			SDcard.MainState = SD_WAIT_4_MEASUREMENT;
			//reset variables for safety
			SDcard.loadingDataCounter = 1;
			for (uint16_t i = 1; i < 517; i++)
			{
				SDcard.loadingDataPointer[i] = 0xFF;
				SDcard.sendingDataPointer[i] = 0xFF;
			}

#ifdef LOG_SD_WRITE
			SerialUSB.print("newfile info after reset: "); printFileInfo(&SDcard.newFile);
			SerialUSB.println("writeFAT done, go wait4Meas");
#endif

			LEDSDOff();

			//testing
			//SDcard.MainState = SD_DO_NOTHING;
		}
	}
	else if (SDWRITE_FAILED == l_writeState)
	{
		SDcard.MainState = SD_DO_NOTHING;
        LEDSDBlink();
#ifdef LOG_SD_WRITE
			SerialUSB.println("writeFAT failed");
#endif
	}
	else
	{
		//do nothing?
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
    SDcard.MainState = SD_INIT;
    SDcard.SDInitStatus = SDINIT_CMD0;
    SDcard.SDCommandState = SDCOMMAND_SEND;
    SDcard.SDReadState = SDREAD_START;

    return SDcard.MainState;
}

void setGlobalTime(const date newTime, const float currentSysTime)
{
	SDcard.globalDateAndTime.hour = newTime.hour;
	SDcard.globalDateAndTime.min = newTime.min;
	SDcard.globalDateAndTime.sec = newTime.sec;

	SDcard.sysTimeAtGlobalTime = currentSysTime;
}

void setGlobalDate(const date newTime)
{
	SDcard.globalDateAndTime.year = newTime.year;
	SDcard.globalDateAndTime.month = newTime.month;
	SDcard.globalDateAndTime.day = newTime.day;
}