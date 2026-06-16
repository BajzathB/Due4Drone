#include "pch.h"

#include "../../src/sysTime.h"
#include "../helper/support4Testing.h"

extern sysTime sysTimer;
extern Tc* TC0;

// testing compilation
TEST(test_sysTime, SetupSysTimer_Call)
{
    droneTimes_st testTimes;
    SetupSysTimer();
    UpdateSysTime(&testTimes);
    getSysTick();
    getSysTime();
    getTimeSinceReset();
    calcDeltaTime(1.0f, 2.0f);

    EXPECT_TRUE(true);
}

// testing time values without presetting
TEST(test_sysTime, UpdateSysTime_Time0)
{
    droneTimes_st testTimes;
    UpdateSysTime(&testTimes);

    EXPECT_EQ(sysTimer.raw, 0);
    EXPECT_EQ(sysTimer.sysTick, 0);
    EXPECT_EQ(testTimes.loopTick, 0);
    EXPECT_EQ(testTimes.sysTick, 0);
}

// testing time values with presetting
TEST(test_sysTime, UpdateSysTime_TimeValue)
{
    droneTimes_st testTimes;

    // 1st call
    TC0->TC_CHANNEL[2].TC_CV = 500000;

    UpdateSysTime(&testTimes);

    EXPECT_EQ(sysTimer.raw, 500000);
    EXPECT_EQ(sysTimer.sysTick, 500000);

    // 2nd call
    TC0->TC_CHANNEL[2].TC_CV = 700000;

    UpdateSysTime(&testTimes);

    EXPECT_EQ(sysTimer.raw, 700000);
    EXPECT_EQ(sysTimer.sysTick, 1200000);
}

// testing sysTime getter
TEST(test_sysTime, getSysTime)
{
    //
    sysTimer.sysTick = 1200000;
    EXPECT_NEAR(getSysTime(), float(1200000)/10500000, 0.00001);
    //
    sysTimer.sysTick = 29500000;
    EXPECT_NEAR(getSysTime(), float(29500000) / 10500000, 0.00001);
}

TEST(test_sysTime, getSysTick)
{
    //
    sysTimer.sysTick = 1200000;
    EXPECT_EQ(getSysTick(), 1200000);
    //
    sysTimer.sysTick = 29500000;
    EXPECT_EQ(getSysTick(), 29500000);
}

TEST(test_sysTime, getTimeSinceReset)
{
    float time;

    // 1st
    TC0->TC_CHANNEL[2].TC_CV = 123456;
    time = getTimeSinceReset();
    EXPECT_NEAR(time, 123456 / 10.5, 0.01);

    // 2nd
    TC0->TC_CHANNEL[2].TC_CV = 9876543;
    time = getTimeSinceReset();
    EXPECT_NEAR(time, 9876543 / 10.5, 0.1);
}

TEST(test_sysTime, calcDeltaTime)
{
    // 1st
    EXPECT_NEAR(calcDeltaTime(1.0f,2.0f), 1, 0.01);
    // 2nd
    EXPECT_NEAR(calcDeltaTime(100.5f, 135.0f), 34.5f, 0.01);
}