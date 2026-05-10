

#include "pch.h"
#include "SPI_common.h"

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

// param C = dataRate/cutoffFreq
void PT1Filter(volatile float* yOut, const volatile float xIn, const volatile float paramC)
{
    *yOut = (xIn + paramC * (*yOut)) / (paramC + 1);
}

void signalPT1Filter(volatile signal* sig)
{
    PT1Filter(&sig->signalsPT1.x, sig->offset.x, sig->paramC);
    PT1Filter(&sig->signalsPT1.y, sig->offset.y, sig->paramC);
    PT1Filter(&sig->signalsPT1.z, sig->offset.z, sig->paramC);
}

//y=alpha(x-y) where alpha=1/(1+dataRate/cutoffFreq)
//>>15 is equal 1/32768, 2979/32768 = 1/(1+2000/200)
inline int32_t gyroPT1_200(int32_t y, const int32_t x)
{
    return y + ((2979 * (x - y)) >> 15);    //equivalent to 200Hz cutoff
}
inline int32_t gyroPT1_133(int32_t y, const int32_t x)
{
    return y + ((x - y) >> 4);    //equivalent to 133Hz cutoff
}

void gyroSignalPT1(volatile signal* sig)
{
    sig->signalsPT1_int_200.x = gyroPT1_200(sig->signalsPT1_int_200.x, sig->signals_int.x);
    sig->signalsPT1_int_200.y = gyroPT1_200(sig->signalsPT1_int_200.y, sig->signals_int.y);
    sig->signalsPT1_int_200.z = gyroPT1_200(sig->signalsPT1_int_200.z, sig->signals_int.z);
    
    sig->signalsPT1_int_133.x = gyroPT1_133(sig->signalsPT1_int_133.x, sig->signals_int.x);
    sig->signalsPT1_int_133.y = gyroPT1_133(sig->signalsPT1_int_133.y, sig->signals_int.y);
    sig->signalsPT1_int_133.z = gyroPT1_133(sig->signalsPT1_int_133.z, sig->signals_int.z);
}