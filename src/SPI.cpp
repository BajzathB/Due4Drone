//SPI0
//MISO - PA25 - peri A
//MOSI - PA26 - peri A
//SPCK - PA27 - peri A
//CS0  - PA28 - peri A - gyro
//CS1  - PA29 - peri A - acc
//CS3  - PB23 - peri B - SDcard
//Int gyro - INT4 - PC19
//Int acc  - INT2 - PA19
//AHB DMA: channel 1 - SPI tx
//         channel 2 - SPI rx
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
//FAT  is at 0x25A
//there is a value 2 offset between the cluster number value and the actually place of data
//example PARAM content is marked at number 6 in root but the content is found in number 4,
//
//data storing princibles:
//  1st file is param.txt, 1st cluster, to hold possible parameters for the future
//  after param starts the measurement files
//  nameing convention: MEAS1.txt, MEAS2.txt,... MEASXXXX.txt
//  storing meas in a linear fasion, clusters follow each other, also means FAT is linear

#include "pch.h"
#include "SPI.h"
#include "LED.h"
#include "sysTime.h"
#include "SPI_SDcard.h"

//to switch header on hardware and unit test compilation
#ifdef UNIT_TEST

#include "../test/helper/support4Testing.h"
#include "../test/helper/support4Testing.hpp"

extern DummySerial SerialUSB;
extern Pio* PIOA;
extern Pio* PIOB;
extern Pio* PIOC;
extern Tc* TC1;
extern Spi* SPI0;
extern Dmac* DMAC;

#else

#include "arduino.h"
#include "variant.h"

#endif

spi_st SPI;

volatile uint32_t testCtrGyroActive{0};
volatile uint32_t testCtrGyroPending{0};
volatile uint32_t testCtrGyroFinish{0};
volatile uint32_t testCtrGyroGoActive{0};

volatile uint32_t testCtrAccActive{0};
volatile uint32_t testCtrAccPending{0};
volatile uint32_t testCtrAccFinish{0};
volatile uint32_t testCtrAccGoActive{0};

void SetupSPI(void)
{
	SetupSPIPins();
	
	SetupDMAC();

	SetupGyro();

	SetupAcc();

	EnableIntHandling();

	InitSDCard();

	//SPI.SdCtr = 0;
	////clock sync data
	//for (SPI.SdCtr = 0; SPI.SdCtr < 200; SPI.SdCtr++)
	//{
	//	SPI.SdTx[SPI.SdCtr] = (0xFF) | SPI_TDR_PCS(CS_SDCARD);
	//}

}

void RunSPI(SPIInput* spiInput, SPIOutput* spiOutput)
{
	if (getSPISdCard()->sdCardInitFinished)
	{
		SetupGyroIntPin();
		SetupAccIntPin();

		getSPISdCard()->sdCardInitFinished = false;
	}

	if (spiInput->sysTime > 5.0f)
	{
		RunSdCard(spiInput, spiOutput);
	}

  // SerialUSB.print(testCtrGyroActive); SerialUSB.print("\t");
  // SerialUSB.print(testCtrGyroPending); SerialUSB.print("\t");
  // SerialUSB.print(testCtrGyroFinish); SerialUSB.print("\t");
  // SerialUSB.print(testCtrGyroGoActive); SerialUSB.print("\t");

  // SerialUSB.print(testCtrAccActive); SerialUSB.print("\t");
  // SerialUSB.print(testCtrAccPending); SerialUSB.print("\t");
  // SerialUSB.print(testCtrAccFinish); SerialUSB.print("\t");
  // SerialUSB.println(testCtrAccGoActive);

}

//Handling interrupt for gyro
//Interrupt handler of PIOC
//The following file was changed
//C:/users/bence/appdata/local/arduino15/packages/arduino/hardware/sam/1.6.11/cores/arduino/WInterrupts.c
//The PIOx_Handler functions are commented out!!!
void PIOC_Handler(void)
{
	//reading pioa status
	uint32_t status_pioc = PIOC->PIO_ISR;
	
	if (status_pioc & PIO_ISR_P19)
	{
		if (ACTIVE == SPI.spiActivityAcc)
		{
			SPI.spiActivityGyro = PENDING;
            testCtrGyroPending++;
		}
		else if (ACTIVE == getSPISdCard()->spiActivitySDCard)
		{
			SPI.spiActivityGyro = PENDING;
		}
		else if (ACTIVE != SPI.spiActivityGyro)
		{
			SPI.spiActivityGyro = ACTIVE;
			ReadMEMSMeas(CS_GYRO, 0x02);
            testCtrGyroActive++;
		}
		else
		{
			//do nothing
		}
	}
}

//Handling interrupt for acc
//Interrupt handler of PIOB
void PIOA_Handler(void)
{
	//reading pioa status
	uint32_t status_pioa = PIOA->PIO_ISR;

	if (status_pioa & PIO_ISR_P19)
	{
		if (ACTIVE == SPI.spiActivityGyro)
		{
			SPI.spiActivityAcc = PENDING;
            testCtrAccPending++;
		}
		else if (ACTIVE == getSPISdCard()->spiActivitySDCard)
		{
			SPI.spiActivityAcc = PENDING;
		}
        else if (ACTIVE != SPI.spiActivityAcc)
        {
			SPI.spiActivityAcc = ACTIVE;
			ReadMEMSMeas(CS_ACC, 0x12);
            testCtrAccActive++;
        }
		else
		{
            //do nothing
		}
	}
}

// Interrupt handler of SPI0
void DMAC_Handler(void)
{
	//reading dma status
	uint32_t status_dma = DMAC->DMAC_EBCISR;

	if ((status_dma & DMAC_EBCISR_CBTC2) || (status_dma & DMAC_EBCISR_CBTC4))
	{
		bool isGyroReadFinished{ false };
		bool isAccReadFinished{ false };

		//Disable READ flag of the given device
		if (ACTIVE == SPI.spiActivityGyro)
		{
            SPI.spiActivityGyro = INACTIVE;
			isGyroReadFinished = true;
            testCtrGyroFinish++;
		}
		else if (ACTIVE == SPI.spiActivityAcc)
		{
            SPI.spiActivityAcc = INACTIVE;
			isAccReadFinished = true;
            testCtrAccFinish++;
		}
		else if (ACTIVE == getSPISdCard()->spiActivitySDCard)
		{
			getSPISdCard()->spiActivitySDCard = INACTIVE;
			//if writing, do response and data response check
			if (SDWRITE_WAIT_RESPONSE == getSPISdCard()->SDWriteState)
			{
				SDWriteWaitResponse();
			}
            else if (SDWRITE_WAIT_DATA == getSPISdCard()->SDWriteState)
            {
                SDWriteWaitData();
            }
		}
		else
		{
			//should not reach, do nothing
		}

		//trigger pending reads
		if (PENDING == SPI.spiActivityGyro && ACTIVE != getSPISdCard()->spiActivitySDCard)
		{
			SPI.spiActivityGyro = ACTIVE;
			ReadMEMSMeas(CS_GYRO, 0x02);
            testCtrGyroGoActive++;
		}
		else if (PENDING == SPI.spiActivityAcc && ACTIVE != getSPISdCard()->spiActivitySDCard)
		{
			SPI.spiActivityAcc = ACTIVE;
			ReadMEMSMeas(CS_ACC, 0x12);
            testCtrAccGoActive++;
		}
		else if (PENDING == getSPISdCard()->spiActivitySDCard)
		{
			getSPISdCard()->spiActivitySDCard = ACTIVE;
			TriggerNextSdCardTxRx();
		}
		else
		{
			//do nothing
		}

		if (isGyroReadFinished)
		{
			calcSignalGyro(&SPI.gyro, SPI.sensorRx);
			calcOffsetGyro(&SPI.gyro);
			compensateData(&SPI.gyro);
		}
		else if (isAccReadFinished)
		{
			calcSignalAcc(&SPI.acc, SPI.sensorRx);
			compensateData(&SPI.acc);
		}
		else
		{
			//do nothing
		}
	}
}

void SetupSPIPins(void)
{
	//Setup peripherial to SPI
	PIOA->PIO_WPSR = 0x50494F00; //disable write protection mode
	PIOA->PIO_PDR |= PIO_PDR_P25 | PIO_PDR_P26 | PIO_PDR_P27 | PIO_PDR_P28 | PIO_PDR_P29; //enable peripheral control
	PIOA->PIO_ABSR &= ~(PIO_ABSR_P25 | PIO_ABSR_P26 | PIO_ABSR_P27 | PIO_ABSR_P28 | PIO_ABSR_P29);  //select peripheral A
	PIOA->PIO_WPSR = 0x50494F01; //reenable write protection mode
	PIOB->PIO_WPSR = 0x50494F00; //disable write protection mode
	PIOB->PIO_PDR |= PIO_PDR_P23; //enable peripheral control
	PIOB->PIO_ABSR |= PIO_ABSR_P23;  //select peripheral B
	PIOB->PIO_WPSR = 0x50494F01; //reenable write protection mode

	//Setup SPI 
	pmc_enable_periph_clk(ID_SPI0);
	SPI0->SPI_CR |= SPI_CR_SPIDIS /*| SPI_CR_SWRST*/; //SPI reset
	SPI0->SPI_WPMR = 0x53504900; //disable write protection
	SPI0->SPI_MR |= SPI_MR_MSTR | SPI_MR_PS | SPI_MR_MODFDIS | SPI_MR_DLYBCS(6);
	SPI0->SPI_CSR[0] |= SPI_CSR_NCPHA | SPI_CSR_SCBR(9) | SPI_CSR_DLYBS(1) | SPI_CSR_DLYBCT(1); //10 -> ~8.4MHz clock, 9 -> 9.3333MHz, delay between transmission needed for gyro
	SPI0->SPI_CSR[1] |= SPI_CSR_NCPHA | SPI_CSR_SCBR(9) | SPI_CSR_DLYBS(1) | SPI_CSR_DLYBCT(0); //10 -> ~8.4MHz clock, 9 -> 9.3333MHz
	SPI0->SPI_CSR[3] |= SPI_CSR_NCPHA | SPI_CSR_SCBR(5) | SPI_CSR_DLYBS(1) | SPI_CSR_DLYBCT(1); //? -> ~?MHz clock
	SPI0->SPI_WPMR = 0x53504901; //reenable write protection
	SPI0->SPI_CR |= SPI_CR_SPIEN; //SPI enable
}

void SetupDMAC(void)
{
	//Setup AHB DMA 
	pmc_enable_periph_clk(ID_DMAC);
	DMAC->DMAC_WPMR = 0x444D414300; //disable write protection
	DMAC->DMAC_GCFG = DMAC_GCFG_ARB_CFG_FIXED;
	DMAC->DMAC_CHDR |= DMAC_CHDR_DIS0 | DMAC_CHDR_DIS1 | DMAC_CHDR_DIS2 | DMAC_CHDR_DIS4;
	DMAC->DMAC_EN |= DMAC_EN_ENABLE; //AHB DMA enable

	//channel 1(tx) and 2(rx) for MEMS
	DMAC->DMAC_CH_NUM[1].DMAC_DSCR = 0;
	DMAC->DMAC_CH_NUM[1].DMAC_CFG = DMAC_CFG_DST_PER(1) | DMAC_CFG_DST_H2SEL_HW | DMAC_CFG_SOD_ENABLE | DMAC_CFG_FIFOCFG_ALAP_CFG;	//spi tx line, destination handshaking, hw handshake, disable on done, 
	DMAC->DMAC_CH_NUM[2].DMAC_DSCR = 0;
	DMAC->DMAC_CH_NUM[2].DMAC_CFG = DMAC_CFG_SRC_PER(2) | DMAC_CFG_SRC_H2SEL_HW | DMAC_CFG_SOD_ENABLE | DMAC_CFG_FIFOCFG_ASAP_CFG;	//spi rx line
	DMAC->DMAC_CH_NUM[2].DMAC_CTRLB = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_PER2MEM_DMA_FC | DMAC_CTRLB_SRC_INCR_FIXED | DMAC_CTRLB_DST_INCR_INCREMENTING;	// perif2mem, increment destination addrs


	//channel 0(tx) and 4(rx) for SD card
	DMAC->DMAC_CH_NUM[0].DMAC_DSCR = 0;
	DMAC->DMAC_CH_NUM[0].DMAC_CFG = DMAC_CFG_DST_PER(1) | DMAC_CFG_DST_H2SEL_HW | DMAC_CFG_SOD_ENABLE | DMAC_CFG_FIFOCFG_ALAP_CFG;	//spi tx line, destination handshaking, hw handshake, disable on done, 
	DMAC->DMAC_CH_NUM[4].DMAC_DSCR = 0;
	DMAC->DMAC_CH_NUM[4].DMAC_CFG = DMAC_CFG_SRC_PER(2) | DMAC_CFG_SRC_H2SEL_HW | DMAC_CFG_SOD_ENABLE | DMAC_CFG_FIFOCFG_ASAP_CFG;	//spi rx line
	DMAC->DMAC_CH_NUM[4].DMAC_CTRLB = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_PER2MEM_DMA_FC | DMAC_CTRLB_SRC_INCR_FIXED | DMAC_CTRLB_DST_INCR_INCREMENTING;	// perif2mem, increment destination addrs


	DMAC->DMAC_EBCIER |= DMAC_EBCIER_CBTC2 | DMAC_EBCIER_CBTC4;	//enable DMAC interrupt on multi transfer complete
}

void SetupGyro(void)
{
	// setting default gyro values
    SPI.gyro.const_raw2real = 2000.0f / 32767.0f;
    // offset calculated after startup
    SPI.gyro.offset.x = 0;
    SPI.gyro.offset.y = 0;
    SPI.gyro.offset.z = 0;

	// flag for bad configuration
	uint8_t badRegContent = false;

	//Software reset: addr 0x14, value 0xB6, 30ms wait needed
	//WriteToMEMS(CS_GYRO, 0x14, 0xB6);
	//WaitRxDone();
	//delay(35);

	//Range: addr 0x0F, value 0x00(2000 �/s)
	//Bandwith: addr 0x10, value 0x00(532Hz)
	//Power: addr 0x11, value 0x00(normal)

	//Interrupt ctrl: addr 0x15, value 0x80(enabled)
	WriteToMEMS(CS_GYRO, 0x15, 0x80);
	WaitRxDone();
	ReadFromMEMS(CS_GYRO, 0x15);
	WaitRxDone();
	//check if register is correctly writen
	badRegContent |= (0x80 == SPI.sensorRx[1]) ? 0x00 : 0x01;

	//Interrupt config: addr 0x16, value 0x07(int4 active high push-pull)
	WriteToMEMS(CS_GYRO, 0x16, 0x07);
	WaitRxDone();
	ReadFromMEMS(CS_GYRO, 0x16);
	WaitRxDone();
	//check if register is correctly writen
	badRegContent |= (0x07 == SPI.sensorRx[1]) ? 0x00 : 0x01;

	//Interrupt map: addr 0x18, value 0x80(int4 on output)
	WriteToMEMS(CS_GYRO, 0x18, 0x80);
	WaitRxDone();
	ReadFromMEMS(CS_GYRO, 0x18);
	WaitRxDone();
	//check if register is correctly writen
	badRegContent |= (0x80 == SPI.sensorRx[1]) ? 0x00 : 0x01;

  ////Gyro selftest: addr: 0x3C
  WriteToMEMS(CS_GYRO, 0x3C, 0x01); //start selftest
  WaitRxDone();
  delay(3);   // 3ms as a delay proved ok
  ReadFromMEMS(CS_GYRO, 0x3C);
  WaitRxDone();
  ReadFromMEMS(CS_GYRO, 0x3C);
  WaitRxDone();
	ReadFromMEMS(CS_GYRO, 0x3C);
	WaitRxDone();
  //check if register is correctly writen, ???1 ?01?
  badRegContent |= (0x02 == (SPI.sensorRx[1] & 0x02)) ? 0x00 : 0x01;
  badRegContent |= (0x00 == (SPI.sensorRx[1] & 0x04)) ? 0x00 : 0x01;
  badRegContent |= (0x10 == (SPI.sensorRx[1] & 0x10)) ? 0x00 : 0x01;

	//set LED according to flag
	badRegContent ? LEDGyroBlink() : LEDGyroOff();
}

void SetupGyroIntPin(void)
{
	pmc_enable_periph_clk(ID_PIOC);

	PIOC->PIO_WPSR = 0x50494F00; //enable write mode
	PIOC->PIO_PER |= PIO_PER_P19; //enable PIO 19 pin
	PIOC->PIO_ODR |= PIO_ODR_P19; //disable output-->pin19 is input
	PIOC->PIO_AIMER |= PIO_AIMER_P19; //enable additional interrupt mode
	PIOC->PIO_ESR |= PIO_ESR_P19; //enable edge interrupt
	PIOC->PIO_REHLSR |= PIO_REHLSR_P19; //set rising edge detection
	PIOC->PIO_IFER |= PIO_IFER_P19; //enable glitch filter on pin
	PIOC->PIO_SCIFSR |= PIO_SCIFSR_P19; //filtering glitches with less than Tmck/2 time
	PIOC->PIO_WPSR = 0x50494F01; //disable write mode
	PIOC->PIO_IER |= PIO_IER_P19; //enable interrupt on pin19
}

void SetupAcc(void)
{
	// setting default acc values
    SPI.acc.const_raw2real = 24.0f * 9.81 / 32768.0f;
    //offset manually tuned
    SPI.acc.offset.x = 0.0125;
    SPI.acc.offset.y = 0.022;
    SPI.acc.offset.z = 0.005;
    SPI.acc.offsetCalcDone = true;  //this is done manually
    SPI.acc.newData = false;

	// flag for bad configuration
	uint8_t badRegContent = false;

  //NOTE: acc response's 1st byte is invalid, after comes the expected values

	////reset
	//delay(2);

	//ID: 0x00, read it to trigger SPI mode for acc
  ReadFromMEMS(CS_ACC, 0x00);
  WaitRxDone();

  //Power: addr 0x7D, 0x04(active)
  delay(1);
  WriteToMEMS(CS_ACC, 0x7D, 0x04);
  WaitRxDone();
  delay(50);

  //Configuration: addr 0x40, low-pass 0x00(145Hz) + odr 0x0C(1600Hz) = 1 000 1100 = 0x8C
  WriteToMEMS(CS_ACC, 0x40, 0x8C);
  WaitRxDone();
  ReadFromMEMS(CS_ACC, 0x40);
  WaitRxDone();
  badRegContent |= (0x8C == SPI.sensorRx[2]) ? 0x00 : 0x01;

  //Range: addr 0x41, 0x03(24g) = ??????03
  WriteToMEMS(CS_ACC, 0x41, 0x03);
  WaitRxDone();
  ReadFromMEMS(CS_ACC, 0x41);
  WaitRxDone();
  badRegContent |= (0x03 == SPI.sensorRx[2]) ? 0x00 : 0x01;

  //Interrupt 2 IO config: addr 0x54, 0x01(active high) + 0x00(push-pull) + 0x01(output) = ???0101?
  WriteToMEMS(CS_ACC, 0x54, 0x0A);
  WaitRxDone();
  ReadFromMEMS(CS_ACC, 0x54);
  WaitRxDone();
  badRegContent |= (0x0A == SPI.sensorRx[2]) ? 0x00 : 0x01;

  //Interrupt map: addr 0x58, 0x00(INT1 not) + 0x01(INT2) = ?1???0??
  WriteToMEMS(CS_ACC, 0x58, 0x40);
  WaitRxDone();
  ReadFromMEMS(CS_ACC, 0x58);
  WaitRxDone();
  badRegContent |= (0x40 == SPI.sensorRx[2]) ? 0x00 : 0x01;

  //Error register: good values: ???000?0 
  ReadFromMEMS(CS_ACC, 0x02);
  WaitRxDone();
  badRegContent |= (0x00 == SPI.sensorRx[2]) ? 0x00 : 0x01;

  //set LED according to flag
  badRegContent ? LEDAccBlink() : LEDAccOff();
}

void SetupAccIntPin(void)
{
	pmc_enable_periph_clk(ID_PIOA);

	PIOA->PIO_WPSR = 0x50494F00; //enable write mode
	PIOA->PIO_PER |= PIO_PER_P19; //enable PIO 19 pin
	PIOA->PIO_ODR |= PIO_ODR_P19; //disable output-->pin19 is input
	PIOA->PIO_AIMER |= PIO_AIMER_P19; //enable additional interrupt mode
	PIOA->PIO_ESR |= PIO_ESR_P19; //enable edge interrupt
	PIOA->PIO_REHLSR |= PIO_REHLSR_P19; //set rising edge detection
	PIOA->PIO_IFER |= PIO_IFER_P19; //enable glitch filter on pin
	PIOA->PIO_SCIFSR |= PIO_SCIFSR_P19; //filtering glitches with less than Tmck/2 time
	PIOA->PIO_WPSR = 0x50494F01; //disable write mode
	PIOA->PIO_IER |= PIO_IER_P19; //enable interrupt on pin19
}

void EnableIntHandling(void)
{
	//clear DMAC, PIOC and PIOA interrupt status registers by read
	uint32_t status_dma = DMAC->DMAC_EBCISR;
	uint32_t status_pioc = PIOC->PIO_ISR;
	uint32_t status_pioa = PIOA->PIO_ISR;

	//clear random pending interrupts
	NVIC_ClearPendingIRQ(DMAC_IRQn);
	NVIC_ClearPendingIRQ(PIOC_IRQn);
	NVIC_ClearPendingIRQ(PIOA_IRQn);

	//set priority for DMAC, PIOC and PIOA
	//interrupts have same prio to prevent lockup
	NVIC_SetPriority(DMAC_IRQn, 1);
	NVIC_SetPriority(PIOC_IRQn, 1);
	NVIC_SetPriority(PIOA_IRQn, 1);

	//enabling interrupts for DMAC, PIOC and PIOA
	NVIC_EnableIRQ(DMAC_IRQn);
	NVIC_EnableIRQ(PIOC_IRQn);
	NVIC_EnableIRQ(PIOA_IRQn);
}

bool IsRxDone(void)
{
	//rx done when status register goes to 0
	return ((DMAC->DMAC_CHSR & DMAC_CHSR_ENA2) > 0) ? false : true;
}

void WaitRxDone(void)
{
	while (!IsRxDone())
	{
		// do nothing
	}
}

void ReadFromMEMS(E_ChipSelect CS, uint32_t adr2read)
{
  uint8_t ctr{0};
  SPI.sensorTx[ctr++] = SPI_MEMS_READ | adr2read | SPI_TDR_PCS(CS);
  SPI.sensorTx[ctr++] = SPI_TDR_PCS(CS);
  if(CS_ACC == CS) SPI.sensorTx[ctr++] = SPI_TDR_PCS(CS);	//extra byte read due to acc comm logic
  SpiDmaTxRx(SPI.sensorTx, SPI.sensorRx, ctr, DMAC_CHANNEL_MEMS);
}

void WriteToMEMS(E_ChipSelect CS, uint32_t adr2write, uint32_t data2write)
{
  uint8_t ctr{0};
  SPI.sensorTx[ctr++] = SPI_MEMS_WRITE | adr2write | SPI_TDR_PCS(CS);
  SPI.sensorTx[ctr++] = data2write | SPI_TDR_PCS(CS);
  SpiDmaTxRx(SPI.sensorTx, SPI.sensorRx, ctr, DMAC_CHANNEL_MEMS);
}

void ReadMEMSMeas(E_ChipSelect CS, uint32_t adr2read)
{
  uint32_t ctr{0};
  SPI.sensorTx[ctr++] = SPI_MEMS_READ | adr2read | SPI_TDR_PCS(CS);
  SPI.sensorTx[ctr++] = SPI_TDR_PCS(CS);	//filler byte to receive data
  SPI.sensorTx[ctr++] = SPI_TDR_PCS(CS);	//filler byte to receive data
  SPI.sensorTx[ctr++] = SPI_TDR_PCS(CS);	//filler byte to receive data
  SPI.sensorTx[ctr++] = SPI_TDR_PCS(CS);	//filler byte to receive data
  SPI.sensorTx[ctr++] = SPI_TDR_PCS(CS);	//filler byte to receive data
  SPI.sensorTx[ctr++] = SPI_TDR_PCS(CS);	//filler byte to receive data
  if(CS_ACC == CS) SPI.sensorTx[ctr++] = SPI_TDR_PCS(CS);	//extra filler byte to receive data
  SpiDmaTxRx(SPI.sensorTx, SPI.sensorRx, ctr, DMAC_CHANNEL_MEMS);
}

void calcSignalGyro(volatile signal* gyroSig, volatile uint8_t* buffer)
{
	int16_t x = (buffer[2] << 8) | buffer[1];
	int16_t y = (buffer[4] << 8) | buffer[3];
	int16_t z = (buffer[6] << 8) | buffer[5];
	
	gyroSig->signals.x = (float)x * gyroSig->const_raw2real;
	gyroSig->signals.y = (float)y * gyroSig->const_raw2real;
	gyroSig->signals.z = (float)z * gyroSig->const_raw2real;

}

void calcSignalAcc(volatile signal* accSig, volatile uint8_t* buffer)
{
    //note difference from gyro is that valid data start from 2nd posiion
    int16_t x = (buffer[3] << 8) | buffer[2];
    int16_t y = (buffer[5] << 8) | buffer[4];
    int16_t z = (buffer[7] << 8) | buffer[6];

    accSig->signals.x = (float)x * accSig->const_raw2real;
    accSig->signals.y = (float)y * accSig->const_raw2real;
    accSig->signals.z = (float)z * accSig->const_raw2real;
}

void calcOffsetGyro(volatile signal* gyroSig)
{
	static const uint16_t startMeas = 1000;
	static const uint16_t endMeas = 2000;
	static uint16_t n = 0;

	if (false == gyroSig->offsetCalcDone)
	{
		if (startMeas <= n && n < endMeas)
		{
			gyroSig->offset.x += gyroSig->signals.x;
			gyroSig->offset.y += gyroSig->signals.y;
			gyroSig->offset.z += gyroSig->signals.z;
		}
		else if (endMeas < n)
		{
			// divide by the number of measurements
			// multiply by -1, positive offset should be extracted
			gyroSig->offset.x /= float(endMeas - startMeas);
			gyroSig->offset.x *= -1.0f;
			gyroSig->offset.y /= float(endMeas - startMeas);
			gyroSig->offset.y *= -1.0f;
			gyroSig->offset.z /= float(endMeas - startMeas);
			gyroSig->offset.z *= -1.0f;
			// set calc done flag
			gyroSig->offsetCalcDone = true;
			// reset n, if a 2nd calc retrigger needed
			n = 0;
		}
		else
		{
			// do nothing when n < startMeas
		}

		n++;
	}
	else
	{
		// do nothing, offset calculation done
	}
}

void compensateData(volatile signal* Sig)
{
    if (true == Sig->offsetCalcDone)
    {
        Sig->signals.x += Sig->offset.x;
        Sig->signals.y += Sig->offset.y;
        Sig->signals.z += Sig->offset.z;
        Sig->newData = true;
    }
    else
    {
        //do nothing
    }
}

void getGyroAndAcc(sigOut* sigGyro, sigOut* sigAcc)
{
    //disable DMAC interrupt
	NVIC_DisableIRQ(DMAC_IRQn);

    //copy data
    sigGyro->signal.x = SPI.gyro.signals.x;
    sigGyro->signal.y = SPI.gyro.signals.y;
    sigGyro->signal.z = SPI.gyro.signals.z;
    sigGyro->newData = SPI.gyro.newData;
    sigAcc->signal.x  = SPI.acc.signals.x;
    sigAcc->signal.y  = SPI.acc.signals.y;
    sigAcc->signal.z  = SPI.acc.signals.z;
    sigAcc->newData = SPI.acc.newData;

	//reset newData flag
	//newData flag only set when new data is calculated in interrupt
	SPI.gyro.newData = false;
	SPI.acc.newData = false;

    //reenable DMAC interrupt
	NVIC_EnableIRQ(DMAC_IRQn);
}

spi_st* getSPI()
{
  return &SPI;
}
