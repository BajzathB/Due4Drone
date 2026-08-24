#include "pch.h"

#include "../../src/SPI.h"
#include "../../src/SPI_common.h"
#include "../../src/SPI_SDcard.h"
#include "../helper/support4Testing.h"
#include "../helper/support4Testing.hpp"

extern NVIC_Type* NVIC;
extern Pio* PIOC;
extern Pio* PIOA;
extern Dmac* DMAC;
extern volatile uint8_t sensorRx[20];
extern spi_st SPI;
extern SpiSDcard_st SDcard;

//extern E_SDMainStates MainState;
//extern E_SDInitStates SDInitStatus;
//extern E_SDCommandStates SDCommandState;
//extern E_SDReadStates SDReadState;
//extern E_SDWriteStates SDWriteState;

// testing compilation
TEST(test_SPI, SPI_Call)
{
    SetupSPI();
    SetupSPIPins();
    SetupDMAC();
    SetupGyro();
    SetupGyroIntPin();
    SetupAcc();
    SetupAccIntPin();
    EnableIntHandling();
    RunSPI();
    IsRxDone();
    WaitRxDone();
    uint32_t testTx[10];
    uint8_t testRx[10];
    uint32_t ctr = 5;
    SpiDmaTxRx(testTx, testRx, ctr, DMAC_CHANNEL_MEMS);
    ReadFromMEMS(CS_GYRO, 0x00);
    WriteToMEMS(CS_GYRO, 0x00, 0x00);
    ReadMEMSMeas(CS_GYRO, 0x00);
    signal testSig;
    calcSignalGyro(&testSig, testRx);
    calcSignalAcc(&testSig, testRx);
    calcOffsetGyro(&testSig);
    compensateData(&testSig);
    PIOC_Handler();
    PIOA_Handler();
    DMAC_Handler();
    sigOut testAxisGyro, testAxisAcc;
    getGyroAndAcc(&testAxisGyro, &testAxisAcc);
    float y{1}, x{ 1 }, paramC{ 1 };
    calcRealFromInt(&testSig, E_direction::X, false);
    PT1_133Hz(1,1);
    gyroSignalPT1_133Hz(&testSig);
    PT1_25Hz(1, 1);
    accSignalPT1_25Hz(&testSig);
}

TEST(test_SPI, PIOC_Handler_Test)
{
    //1st: no status register set
    PIOC->PIO_ISR = 0;
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, INACTIVE);

    //2nd: all inactive
    PIOC->PIO_ISR = PIO_ISR_P19;
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);

    //3rd: acc already active
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = ACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);

    //4th: sd card already active
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = ACTIVE;
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);

    //5th: gyro and acc pending
    SPI.spiActivityGyro = PENDING;
    SPI.spiActivityAcc = PENDING;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);

    //6th: all pending
    SPI.spiActivityGyro = PENDING;
    SPI.spiActivityAcc = PENDING;
    SDcard.spiActivitySDCard = PENDING;
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);

    //other variations
    //7th
    SPI.spiActivityGyro = PENDING;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);
    //8th
    SPI.spiActivityGyro = ACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);
    //9th
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = PENDING;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);
    //10th
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = PENDING;
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);
    //11th
    SPI.spiActivityGyro = PENDING;
    SPI.spiActivityAcc = ACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    //12th
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = ACTIVE;
    SDcard.spiActivitySDCard = PENDING;
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
}

TEST(test_SPI, calcSignalGyro_Test)
{
    uint8_t testRx[10];
    signal testSig;

    // 0 bytes
    testRx[1] = 0;
    testRx[2] = 0;
    testRx[3] = 0;
    testRx[4] = 0;
    testRx[5] = 0;
    testRx[6] = 0;
    calcSignalGyro(&testSig, testRx);
    EXPECT_EQ(testSig.signals.x, 0);
    EXPECT_EQ(testSig.signals.y, 0);
    EXPECT_EQ(testSig.signals.z, 0);

    // with values
    testRx[1] = 200;
    testRx[2] = 0;
    testRx[3] = 100;
    testRx[4] = 2;
    testRx[5] = 50;
    testRx[6] = 100;
    calcSignalGyro(&testSig, testRx);
    EXPECT_EQ(testSig.signals.x, 200);
    EXPECT_EQ(testSig.signals.y, 612);
    EXPECT_EQ(testSig.signals.z, 25650);
}

TEST(test_SPI, calcGyroOffset_Test)
{
    signal testSig;

    //1st: 1st call
    calcOffsetGyro(&testSig);
    EXPECT_EQ(testSig.offsetCalcDone, false);
    EXPECT_EQ(testSig.offset.x, 0);
    EXPECT_EQ(testSig.offset.y, 0);
    EXPECT_EQ(testSig.offset.z, 0);

    //2nd: reaching measurement
    testSig.signals.x = 5;
    testSig.signals.y = 10;
    testSig.signals.z = 21;
    for (uint16_t i = 0; i < 999; i++)
    {
        calcOffsetGyro(&testSig);
    }
    EXPECT_EQ(testSig.offsetCalcDone, false);
    EXPECT_EQ(testSig.offset.x, 5);
    EXPECT_EQ(testSig.offset.y, 10);
    EXPECT_EQ(testSig.offset.z, 21);

    //3rd: finish measurement
    for (uint16_t i = 0; i < 1025; i++)
    {
        calcOffsetGyro(&testSig);
    }
    EXPECT_EQ(testSig.offsetCalcDone, true);
    EXPECT_EQ(testSig.offset.x, -5);
    EXPECT_EQ(testSig.offset.y, -10);
    EXPECT_EQ(testSig.offset.z, -21);

    //4th: retrigger measurement
    testSig.offsetCalcDone = false;
    testSig.offset.x = 0;
    testSig.offset.y = 0;
    testSig.offset.z = 0;
    for (uint16_t i = 0; i < 1000; i++)
    {
        calcOffsetGyro(&testSig);
    }
    EXPECT_EQ(testSig.offsetCalcDone, false);
    EXPECT_EQ(testSig.offset.x, 5);
    EXPECT_EQ(testSig.offset.y, 10);
    EXPECT_EQ(testSig.offset.z, 21);

}

TEST(test_SPI, compensateData_Test)
{
    signal testSig;

    //1st: offset calc not done
    testSig.offsetCalcDone = false;
    testSig.signals.x = 5;
    testSig.signals.y = 10;
    testSig.signals.z = -8;
    testSig.offset.x = 5;
    testSig.offset.y = -4;
    testSig.offset.z = -3;
    compensateData(&testSig);
    EXPECT_EQ(testSig.signals.x, 5);
    EXPECT_EQ(testSig.signals.y, 10);
    EXPECT_EQ(testSig.signals.z, -8);
    EXPECT_FALSE(testSig.newData);

    //2nd: offset calc done
    testSig.offsetCalcDone = true;
    compensateData(&testSig);
    EXPECT_EQ(testSig.signals.x, 10);
    EXPECT_EQ(testSig.signals.y, 6);
    EXPECT_EQ(testSig.signals.z, -11);
    EXPECT_TRUE(testSig.newData);

}

TEST(test_SPI, PIOA_Handler_Test)
{
    //1st: no status register set
    PIOA->PIO_ISR = 0;
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);

    //2nd: all inactive
    PIOA->PIO_ISR = PIO_ISR_P19;
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);

    //3rd: gyro already active
    SPI.spiActivityGyro = ACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);

    //4th: sd card already active
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = ACTIVE;
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);

    //5th: gyro and acc pending
    SPI.spiActivityGyro = PENDING;
    SPI.spiActivityAcc = PENDING;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);

    //6th: all pending
    SPI.spiActivityGyro = PENDING;
    SPI.spiActivityAcc = PENDING;
    SDcard.spiActivitySDCard = PENDING;
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);

    //other variations
    //7th
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = PENDING;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);
    //8th
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = ACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);
    //9th
    SPI.spiActivityGyro = PENDING;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);
    //10th
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = PENDING;
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);
    //11th
    SPI.spiActivityGyro = ACTIVE;
    SPI.spiActivityAcc = PENDING;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    //12th
    SPI.spiActivityGyro = ACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = PENDING;
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
}

TEST(test_SPI, InterruptPriority_Test)
{
    uint32_t testTx[530];
    uint8_t testRx[10];
    uint32_t ctr = 5;

    //1st: gyro read
    PIOC->PIO_ISR = PIO_ISR_P19;
    DMAC->DMAC_EBCISR = DMAC_EBCISR_CBTC2 | DMAC_EBCISR_CBTC4;
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, INACTIVE);

    //2nd: acc read
    PIOA->PIO_ISR = PIO_ISR_P19;
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);

    //3rd: trigger gyro then acc
    PIOC_Handler();
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, INACTIVE);
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, INACTIVE);
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);

    //4th: trigger acc then gyro
    PIOA_Handler();
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);
    EXPECT_EQ(SPI.spiActivityGyro, INACTIVE);

    //5th: trigger sd card
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    SDcard.SDWriteState = SDWRITE_START;
    triggerSDRxTx(testTx, testRx, ctr);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    DMAC_Handler();
    EXPECT_EQ(SDcard.spiActivitySDCard, INACTIVE);

    //6th: trigger acc then sd card
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    SDcard.SDWriteState = SDWRITE_START;
    PIOA_Handler();
    triggerSDRxTx(testTx, testRx, ctr);
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, INACTIVE);

    //6.5th: trigger acc then sd card in write wait reponse state
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    SDcard.SDWriteState = SDWRITE_WAIT_RESPONSE;
    PIOA_Handler();
    triggerSDRxTx(testTx, testRx, ctr);
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);

    //7th: trigger acc then gyro then sd card
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    SDcard.SDWriteState = SDWRITE_START;
    PIOA_Handler();
    PIOC_Handler();
    triggerSDRxTx(testTx, testRx, ctr);
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);
    EXPECT_EQ(SPI.spiActivityGyro, INACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);
    EXPECT_EQ(SPI.spiActivityGyro, INACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, INACTIVE);

    //7.5th: trigger acc then gyro then sd card in write wait response state
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    SDcard.SDWriteState = SDWRITE_WAIT_RESPONSE;
    PIOA_Handler();
    PIOC_Handler();
    triggerSDRxTx(testTx, testRx, ctr);
    EXPECT_EQ(SPI.spiActivityAcc, ACTIVE);
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);
    EXPECT_EQ(SPI.spiActivityGyro, INACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);
    EXPECT_EQ(SPI.spiActivityGyro, INACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);

    //8th: sd card cant be suspended by gyro
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    SDcard.SDWriteState = SDWRITE_START;
    triggerSDRxTx(testTx, testRx, ctr);
    EXPECT_EQ(SPI.spiActivityGyro, INACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, INACTIVE);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, INACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, INACTIVE);

    //11th: triggering sd card multiple times during active
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    triggerSDRxTx(testTx, testRx, ctr);
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    triggerSDRxTx(testTx, testRx, ctr);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    DMAC_Handler();
    EXPECT_EQ(SDcard.spiActivitySDCard, INACTIVE);

    //test: trying to force deadlock
    SPI.spiActivityGyro = INACTIVE;
    SPI.spiActivityAcc = INACTIVE;
    SDcard.spiActivitySDCard = INACTIVE;
    triggerSDRxTx(testTx, testRx, ctr);
    EXPECT_EQ(SPI.spiActivityGyro, INACTIVE);
    EXPECT_EQ(SPI.spiActivityAcc, INACTIVE);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, INACTIVE);
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    PIOA_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, INACTIVE);
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    triggerSDRxTx(testTx, testRx, ctr);
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    PIOA_Handler();
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    SDcard.SDWriteState = SDWRITE_WAIT_RESPONSE;
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    PIOA_Handler();
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    SDcard.SdRx[7] = 0b0100;
    SDcard.SdCtr = 8;
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    PIOA_Handler();
    PIOC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, PENDING);
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
    DMAC_Handler();
    EXPECT_EQ(SPI.spiActivityGyro, ACTIVE);
    EXPECT_EQ(SPI.spiActivityAcc, PENDING);
    EXPECT_EQ(SDcard.spiActivitySDCard, INACTIVE);
}

TEST(test_SPI, getGyroAndAcc_Test)
{

    sigOut testAxisGyro, testAxisAcc;
    //1st
    SPI.gyro.signals.x = 5;
    SPI.gyro.signals.y = 4;
    SPI.gyro.signals.z = 3;
    SPI.gyro.signalsPT1.x = 5;
    SPI.gyro.signalsPT1.y = 4;
    SPI.gyro.signalsPT1.z = 3;
    SPI.gyro.newData = true;
    SPI.acc.signals.x = -2;
    SPI.acc.signals.y = -4;
    SPI.acc.signals.z = 10;
    SPI.acc.signalsPT1.x = -2;
    SPI.acc.signalsPT1.y = -4;
    SPI.acc.signalsPT1.z = 10;
    SPI.acc.newData = true;
    getGyroAndAcc(&testAxisGyro, &testAxisAcc);
    EXPECT_EQ(testAxisGyro.signalPT1.x, 5);
    EXPECT_EQ(testAxisGyro.signalPT1.y, 4);
    EXPECT_EQ(testAxisGyro.signalPT1.z, 3);
    EXPECT_TRUE(testAxisGyro.newData);
    EXPECT_EQ(testAxisAcc.signalPT1.x, -2);
    EXPECT_EQ(testAxisAcc.signalPT1.y, -4);
    EXPECT_EQ(testAxisAcc.signalPT1.z, 10);
    EXPECT_TRUE(testAxisAcc.newData);

    //2nd
    getGyroAndAcc(&testAxisGyro, &testAxisAcc);
    EXPECT_FALSE(testAxisGyro.newData);
    EXPECT_FALSE(testAxisAcc.newData);
}

TEST(test_SPI, IsRxDone_Test)
{
    DMAC->DMAC_CHSR = DMAC_CHSR_ENA2;
    EXPECT_EQ(IsRxDone(), false);
    DMAC->DMAC_CHSR = 0;
    EXPECT_EQ(IsRxDone(), true);
}

TEST(test_SPI, gyroPT1_133Hz_Test)
{
    EXPECT_EQ(PT1_133Hz(100,150), 103);
    EXPECT_EQ(PT1_133Hz(100,-3000), -94);
}

TEST(test_SPI, gyroSignalPT1_Test)
{
    volatile signal testSig;

    //test: 
    testSig.signals.x = 100;
    testSig.signals.y = -200;
    testSig.signals.z = 500;
    testSig.signalsPT1.x = 0;
    testSig.signalsPT1.y = 0;
    testSig.signalsPT1.z = 0;
    gyroSignalPT1_133Hz(&testSig);
    EXPECT_EQ(testSig.signalsPT1.x, 6);
    EXPECT_EQ(testSig.signalsPT1.y, -13);
    EXPECT_EQ(testSig.signalsPT1.z, 31);
}

TEST(test_SPI, accPT1_25Hz_Test)
{
    EXPECT_EQ(PT1_25Hz(100, 150), 100);
    EXPECT_EQ(PT1_25Hz(-100, 30000), 370);
}

TEST(test_SPI, accSignalPT1_25Hz_Test)
{
    volatile signal testSig;

    //test: 
    testSig.signals.x = 100;
    testSig.signals.y = -2000;
    testSig.signals.z = 20000;
    testSig.signalsPT1.x = 0;
    testSig.signalsPT1.y = 0;
    testSig.signalsPT1.z = 0;
    accSignalPT1_25Hz(&testSig);
    EXPECT_EQ(testSig.signalsPT1.x, 1);
    EXPECT_EQ(testSig.signalsPT1.y, -32);
    EXPECT_EQ(testSig.signalsPT1.z, 312);
}

TEST(test_SPI, calcRealFromInt_Test)
{
    signal sig;

    //
    sig.raw2realMultiplier = 100;
    sig.raw2realDivider = 2;
    sig.signals.x = 50;
    sig.signalsPT1.y = -1000;
    sig.signalsPT1.z = 5000;
    EXPECT_NEAR(calcRealFromInt(&sig, E_direction::X, false), 2500.0, 0.1);
    EXPECT_NEAR(calcRealFromInt(&sig, E_direction::Y ,true), -50000.0, 0.1);
    EXPECT_NEAR(calcRealFromInt(&sig, E_direction::Z ,true), 250000, 0.1);
}

TEST(test_SPI, calcMovingAverage_Test)
{
    MovingAverage ma;
    axis_i32 value;

    //test: 0
    calcMovingAverage(&ma, &value);
    EXPECT_EQ(ma.averageX, 0);
    EXPECT_EQ(ma.count, 1);
    EXPECT_EQ(ma.index, 1);

    //test 
    for (uint8_t i = 0; i < 50; i++)
    {
        ma.dataX[i] = 100;
        ma.sumX += 100;
    }
    for (uint8_t i = 50; i < 99; i++)
    {
        ma.dataX[i] = 200;
        ma.sumX += 200;
    }
    ma.count = 99;
    ma.index = 99;
    value.x = 200.0f;
    calcMovingAverage(&ma, &value);
    EXPECT_EQ(ma.averageX, 150);
    EXPECT_EQ(ma.count, 100);
    EXPECT_EQ(ma.index, 0);
}