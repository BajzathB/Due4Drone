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
    SPIInput testInput;
    SPIOutput testOutput;
    RunSPI(&testInput, &testOutput);
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

TEST(test_SPI, calcGyroOffset_Test)
{
    signal testSig = { testSig.const_raw2real = 2000.0f / 32767.0f, };

    //1st: 1st call
    calcOffsetGyro(&testSig);
    EXPECT_EQ(testSig.offsetCalcDone, false);
    EXPECT_EQ(testSig.offset.x, 0.0f);
    EXPECT_EQ(testSig.offset.y, 0.0f);
    EXPECT_EQ(testSig.offset.z, 0.0f);

    //2nd: reaching measurement
    testSig.signals.x = 0.5f;
    testSig.signals.y = 1.0f;
    testSig.signals.z = 0.2f;
    for (uint16_t i = 0; i < 999; i++)
    {
        calcOffsetGyro(&testSig);
    }
    EXPECT_EQ(testSig.offsetCalcDone, false);
    EXPECT_NEAR(testSig.offset.x, 0.5, 0.1);
    EXPECT_NEAR(testSig.offset.y, 1.0, 0.1);
    EXPECT_NEAR(testSig.offset.z, 0.2, 0.1);

    //3rd: finish measurement
    for (uint16_t i = 0; i < 1001; i++)
    {
        calcOffsetGyro(&testSig);
    }
    EXPECT_EQ(testSig.offsetCalcDone, true);
    EXPECT_NEAR(testSig.offset.x, -0.5, 0.1);
    EXPECT_NEAR(testSig.offset.y, -1.0, 0.1);
    EXPECT_NEAR(testSig.offset.z, -0.2, 0.1);

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
    EXPECT_NEAR(testSig.offset.x, 0.5, 0.1);
    EXPECT_NEAR(testSig.offset.y, 1.0, 0.1);
    EXPECT_NEAR(testSig.offset.z, 0.2, 0.1);

}

TEST(test_SPI, compensateData_Test)
{
    signal testSig;

    //1st: offset calc not done
    testSig.offsetCalcDone = false;
    testSig.signals.x = 5.0f;
    testSig.signals.y = 10.0f;
    testSig.signals.z = -8.0f;
    testSig.offset.x = 5.0f;
    testSig.offset.y = -4.0f;
    testSig.offset.z = -3.0f;
    compensateData(&testSig);
    EXPECT_NEAR(testSig.signals.x, 5.0f, 0.1f);
    EXPECT_NEAR(testSig.signals.y, 10.0f, 0.1f);
    EXPECT_NEAR(testSig.signals.z, -8.0f, 0.1f);
    EXPECT_FALSE(testSig.newData);

    //2nd: offset calc done
    testSig.offsetCalcDone = true;
    compensateData(&testSig);
    EXPECT_NEAR(testSig.signals.x, 10.0f, 0.1f);
    EXPECT_NEAR(testSig.signals.y, 6.0f, 0.1f);
    EXPECT_NEAR(testSig.signals.z, -11.0f, 0.1f);
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
}

TEST(test_SPI, getGyroAndAcc_Test)
{

    sigOut testAxisGyro, testAxisAcc;
    //1st
    SPI.gyro.signals.x = 5.0f;
    SPI.gyro.signals.y = 4.5f;
    SPI.gyro.signals.z = 3.0f;
    SPI.gyro.newData = true;
    SPI.acc.signals.x = -2.0;
    SPI.acc.signals.y = -4.2f;
    SPI.acc.signals.z = -10.9f;
    SPI.acc.newData = true;
    getGyroAndAcc(&testAxisGyro, &testAxisAcc);
    EXPECT_NEAR(testAxisGyro.signal.x, 5, 0.1);
    EXPECT_NEAR(testAxisGyro.signal.y, 4.5, 0.1);
    EXPECT_NEAR(testAxisGyro.signal.z, 3, 0.1);
    EXPECT_TRUE(testAxisGyro.newData);
    EXPECT_NEAR(testAxisAcc.signal.x, -2, 0.1);
    EXPECT_NEAR(testAxisAcc.signal.y, -4.2, 0.1);
    EXPECT_NEAR(testAxisAcc.signal.z, -10.9, 0.1);
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

TEST(test_SPI, RunSPI_Test)
{
    SPIInput testIn;
    SPIOutput testOut;

    //sd card not finished
    getSPISdCard()->sdCardInitFinished = false;
    RunSPI(&testIn, &testOut);
    EXPECT_EQ(getSPISdCard()->sdCardInitFinished, false);

    //sd card finished
    getSPISdCard()->sdCardInitFinished = true;
    RunSPI(&testIn, &testOut);
    EXPECT_EQ(getSPISdCard()->sdCardInitFinished, false);
}
