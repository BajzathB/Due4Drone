#include "pch.h"
#include "../../src/RC.h"
#include "../helper/support4Testing.h"
#include "../helper/support4Testing.hpp"

extern Usart* USART2;
extern rc_st RC;
extern Wdt* WDT;

TEST(test_RC, RC_Call)
{
    SetupRC();
    SetInitValues();
    SetupUsart2Pin();
    USART2_Handler();
    SetChannels2Init();
    uint8_t testVector[32];
    IsIbusHeaderReceived(testVector);
    rcSignals_st testOut;
    getRcChannels(&testOut);
    watchdogSetup();
    RestartWDCtr();
    WDT_Handler();
}

TEST(test_RC, IsIbusHeaderReceived_Test)
{
    uint8_t testVector[5];
    testVector[0] = 0x20;
    testVector[1] = 0x40;
    testVector[2] = 0x00;
    testVector[3] = 0x20;
    testVector[4] = 0x40;

    //1st: argument is 0th position
    EXPECT_TRUE(IsIbusHeaderReceived(testVector));

    //2nd: argument is 1th position
    EXPECT_FALSE(IsIbusHeaderReceived(&testVector[1]));

    //3rd: argument is 2th position
    EXPECT_FALSE(IsIbusHeaderReceived(&testVector[2]));

    //4th: argument is 4th position
    EXPECT_TRUE(IsIbusHeaderReceived(&testVector[3]));
}

TEST(test_RC, USART2_Handler_Test)
{
    USART2->US_CSR = US_CSR_ENDRX;
    RC.IBUS_received[0] = 0x20; //header
    RC.IBUS_received[1] = 0x40;
    RC.IBUS_received[2] = 0xDC; //ch1
    RC.IBUS_received[3] = 0x05;
    RC.IBUS_received[4] = 0xDC; //ch2
    RC.IBUS_received[5] = 0x05;
    RC.IBUS_received[6] = 0xE8; //ch3
    RC.IBUS_received[7] = 0x03;
    RC.IBUS_received[8] = 0x08; //ch4
    RC.IBUS_received[9] = 0x07;
    for (uint8_t i = 10; i < 29; i++)
    {
        RC.IBUS_received[i] = 0x00;
    }

    //1st: header ok, good crc
    RC.IBUS_received[30] = 0xE3; //crc
    RC.IBUS_received[31] = 0xFC;
    USART2_Handler();
    EXPECT_EQ(RC.IBUS_channel[0],1500);
    EXPECT_EQ(RC.IBUS_channel[1],1500);
    EXPECT_EQ(RC.IBUS_channel[2],1000);
    EXPECT_EQ(RC.IBUS_channel[3],1800);
    EXPECT_EQ(RC.isCrcValid,true);

    //2nd: bad crc
    RC.IBUS_received[30] = 0x00; //crc
    RC.IBUS_received[31] = 0x00;
    USART2_Handler();
    EXPECT_EQ(RC.IBUS_channel[0], 1500);
    EXPECT_EQ(RC.IBUS_channel[1], 1500);
    EXPECT_EQ(RC.IBUS_channel[2], 1000);
    EXPECT_EQ(RC.IBUS_channel[3], 1500);
    EXPECT_EQ(RC.isCrcValid, false);

    //3rd: bad header
    RC.IBUS_received[0] = 0x00;
    RC.IBUS_received[1] = 0x00;
    RC.IBUS_received[2] = 0x00;
    RC.IBUS_received[3] = 0x20; //header
    RC.IBUS_received[4] = 0x40;
    RC.IBUS_received[5] = 0x01; //ch1
    RC.IBUS_received[6] = 0x02;
    RC.IBUS_received[7] = 0x03; //ch2
    RC.IBUS_received[8] = 0x04;
    RC.IBUS_received[9] = 0x05; //ch3
    RC.IBUS_channel[0] = 1300;
    RC.isCrcValid = true;
    USART2_Handler();
    EXPECT_EQ(RC.IBUS_received[0], 0x20); //header
    EXPECT_EQ(RC.IBUS_received[1], 0x40);
    EXPECT_EQ(RC.IBUS_received[2], 0x01); //ch1
    EXPECT_EQ(RC.IBUS_received[3], 0x02);
    EXPECT_EQ(RC.IBUS_received[4], 0x03); //ch2
    EXPECT_EQ(RC.IBUS_received[5], 0x04);
    EXPECT_EQ(RC.IBUS_received[6], 0x05); //ch3
    EXPECT_EQ(RC.IBUS_channel[0], 1500);
    EXPECT_EQ(RC.isCrcValid, false);
    EXPECT_EQ(USART2->US_RCR, 3);

    //4th: header is 31-0
    RC.IBUS_received[0] = 0x40;
    RC.IBUS_received[1] = 0x12;
    RC.IBUS_received[31] = 0x20;
    RC.IBUS_channel[1] = 1800;
    RC.isCrcValid = true;
    USART2_Handler();
    EXPECT_EQ(RC.IBUS_received[0], 0x20); //header
    EXPECT_EQ(RC.IBUS_received[1], 0x12);
    EXPECT_EQ(RC.IBUS_channel[1], 1500);
    EXPECT_EQ(USART2->US_RCR, 31);

    //5th: no header
    for (uint8_t i = 0; i < 31; i++)
    {
        RC.IBUS_received[i] = 0x00;
    }
    RC.IBUS_channel[2] = 1300;
    RC.isCrcValid = true;
    USART2_Handler();
    EXPECT_EQ(RC.isCrcValid, false);
    EXPECT_EQ(RC.IBUS_channel[2], 1000);
    EXPECT_EQ(USART2->US_RCR, 32);
}

TEST(test_RC, getRcChannels_Test)
{
    rcSignals_st testOut;

    //1st: not valid channel data
    RC.isCrcValid = false;
    RC.IBUS_channel[0] = 1250;
    RC.IBUS_channel[1] = 1450;
    RC.IBUS_channel[2] = 1200;
    RC.IBUS_channel[3] = 1650;
    getRcChannels(&testOut);
    EXPECT_EQ(testOut.roll, 1500);
    EXPECT_EQ(testOut.pitch, 1500);
    EXPECT_EQ(testOut.throttle, 1000);
    EXPECT_EQ(testOut.yaw, 1500);

    //2nd: valid data
    RC.isCrcValid = true;
    getRcChannels(&testOut);
    EXPECT_EQ(testOut.roll, 1250);
    EXPECT_EQ(testOut.pitch, 1450);
    EXPECT_EQ(testOut.throttle, 1200);
    EXPECT_EQ(testOut.yaw, 1650);
}

TEST(test_RC, WDT_Handler_Test)
{
    //1st: all status 0
    WDT->WDT_SR = 0;
    RC.IBUS_channel[2] = 1200;
    WDT_Handler();
    EXPECT_EQ(RC.IBUS_channel[2], 1200);

    //2nd: wdt error
    WDT->WDT_SR = WDT_SR_WDERR;
    RC.IBUS_channel[2] = 1200;
    WDT_Handler();
    EXPECT_EQ(RC.IBUS_channel[2], 1200);

    //3rd: wdt underflow
    WDT->WDT_SR = WDT_SR_WDUNF;
    RC.IBUS_channel[2] = 1200;
    WDT_Handler();
    EXPECT_EQ(RC.IBUS_channel[2], 1000);
}