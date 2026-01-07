

#include "pch.h"
#include "SPI_common.h"

//to switch header on hardware and unit test compilation
#ifdef UNIT_TEST

#include "test/helper/support4Testing.h"
#include "test/helper/support4Testing.hpp"

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

const uint32_t DMAC_CHTX_PRE_CTRLA = DMAC_CTRLA_SRC_WIDTH_WORD | DMAC_CTRLA_DST_WIDTH_WORD;
const uint32_t DMAC_CHRX_PRE_CTRLA = DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
const uint32_t DMAC_CHTX_PER_CTRLB = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_MEM2PER_DMA_FC | DMAC_CTRLB_DST_INCR_FIXED;


void SpiDmaTxRx(volatile uint32_t* txBuff, volatile uint8_t* rxBuff, uint32_t ctr, E_DMACChannels neededDMAC)
{
	uint32_t txSrcIncr{ DMAC_CTRLB_SRC_INCR_INCREMENTING };
	uint8_t txCh{ 1 };
	uint8_t rxCh{ 2 };
	uint32_t triggerCh{ DMAC_CHER_ENA1 | DMAC_CHER_ENA2 };

	//condition for sending a lot of 0xFF for SD card
	if (NULL == txBuff)
	{
		static uint32_t  ff = 0xFF | SPI_TDR_PCS(CS_SDCARD);
		txBuff = &ff;
		txSrcIncr = DMAC_CTRLB_SRC_INCR_FIXED;
	}

	//overwrite default channel if sdcard dmac needed
	if (DMAC_CHANNEL_SDCARD == neededDMAC)
	{
		txCh = 0;
		rxCh = 4;
		triggerCh = DMAC_CHER_ENA0 | DMAC_CHER_ENA4;
	}
	else;	//do nothing

	//set TX
	DMAC->DMAC_CH_NUM[txCh].DMAC_SADDR = (uint32_t)txBuff;
	DMAC->DMAC_CH_NUM[txCh].DMAC_DADDR = (uint32_t)&SPI0->SPI_TDR;
	DMAC->DMAC_CH_NUM[txCh].DMAC_CTRLA = ctr | DMAC_CHTX_PRE_CTRLA;	// overwriting register to prevent bit mismatch problem
	DMAC->DMAC_CH_NUM[txCh].DMAC_CTRLB = txSrcIncr | DMAC_CHTX_PER_CTRLB;	// overwriting register to prevent bit mismatch problem
	//set RX
	DMAC->DMAC_CH_NUM[rxCh].DMAC_SADDR = (uint32_t)&SPI0->SPI_RDR;
	DMAC->DMAC_CH_NUM[rxCh].DMAC_DADDR = (uint32_t)rxBuff;
	DMAC->DMAC_CH_NUM[rxCh].DMAC_CTRLA = ctr | DMAC_CHRX_PRE_CTRLA;	// overwriting register to prevent bit mismatch problem

	//trigger AHB DMAC
	DMAC->DMAC_CHER |= triggerCh;
}