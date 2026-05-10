#pragma once

#include "RC.h"

typedef struct axis
{
	float x{ 0 };
	float y{ 0 };
	float z{ 0 };
}axis;

typedef struct axis_int32
{
    int32_t x{ 0 };
    int32_t y{ 0 };
    int32_t z{ 0 };
}axis_int32;

typedef struct sigOut
{
	axis signal;
    axis_int32 signal_int;
    axis_int32 signalPT1_200;
    axis_int32 signalPT1_133;    
    int32_t raw2realMulti;
    int32_t raw2realShift;
	bool newData{ false };
}sigOut;

typedef enum E_armState : uint8_t
{
	DISARMED = 0,
	IN_ARMING_STAGE = 1,
	ARMED = 2
}E_armState;

typedef enum E_SPIActivity : uint8_t
{
	INACTIVE = 0,
	ACTIVE = 1,
	PENDING = 2
};

//enum for chip select targets
//number should represents the CS channel, but PCSDEC=0 docu part seems to work
typedef enum E_ChipSelect : uint32_t
{
	CS_GYRO = 0b0000,
	CS_ACC = 0b0001,
	CS_SDCARD = 0b0111
};

typedef struct SpiInput
{
	sigOut gyro, acc;
	rcSignals_st rcSignals;
	float sysTime{0};
	E_armState armState{ DISARMED };
}SPIInput;

typedef struct SpiOutput
{

}SPIOutput;

typedef struct signal
{
    axis signals;
    axis offset;
    axis signalsPT1;
    float paramC{1};

    float const_raw2real{ 0 };

    int32_t raw2realMultiplier{1};
    int32_t raw2realBitshift{0};
    axis_int32 offset_int;
    axis_int32 signals_int;
    axis_int32 signalsPT1_int_200;
    axis_int32 signalsPT1_int_133;

	bool offsetCalcDone{ false };
	bool newData{ false };
}signal;

typedef struct spi_st
{
	volatile uint32_t sensorTx[20];
	volatile uint8_t sensorRx[20];
	volatile E_SPIActivity spiActivityGyro{ INACTIVE };
	volatile E_SPIActivity spiActivityAcc{ INACTIVE };

	volatile signal gyro;
	volatile signal acc;
}spi_str;

typedef enum E_DMACChannels : uint8_t
{
	DMAC_CHANNEL_MEMS = 0,
	DMAC_CHANNEL_SDCARD = 1
};

// Method to trigger TX-RX SPI communication
void SpiDmaTxRx(volatile uint32_t* txBuff, volatile uint8_t* rxBufff, uint32_t ctr, E_DMACChannels neededDMAC);

// Method to filter value with PT1
void PT1Filter(volatile float* yOut, const volatile float xIn, const volatile float paramC);

// Method to filter 3axis signal with PT1
void signalPT1Filter(volatile signal* sig);

// Method to filter integer gyro value with PT1
inline int32_t gyroPT1_200(int32_t y, const int32_t x);
inline int32_t gyroPT1_133(int32_t y, const int32_t x);

// Method to filter 3axis integer gyro signal with PT1
void gyroSignalPT1(volatile signal* sig);