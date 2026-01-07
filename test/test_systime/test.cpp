#include "pch.h"

#include "../../src/sysTime.h"
#include "../helper/support4Testing.h"

extern sysTime sysTimer;
extern Tc* TC0;

// testing compilation
TEST(test_sysTime, SetupSysTimer_Call)
{
    SetupSysTimer();
	UpdateSysTime();
	getSysTime();
	getSysLoopTime();
    
    EXPECT_TRUE(true);
}

// testing time values without presetting
TEST(test_sysTime, UpdateSysTime_Time0)
{
    UpdateSysTime();

    EXPECT_EQ(sysTimer.raw,0);
    EXPECT_EQ(sysTimer.loopTime, 0);
    EXPECT_EQ(sysTimer.sysTime, 0);
    EXPECT_NEAR(sysTimer.const_raw2real, 1/10500000.0,1/1000000.0);
}

// testing time values with presetting
TEST(test_sysTime, UpdateSysTime_TimeValue)
{
    // 1st call
    TC0->TC_CHANNEL[2].TC_CV = 500000;

    UpdateSysTime();

    EXPECT_EQ(sysTimer.raw, 500000);
    EXPECT_NEAR(sysTimer.loopTime, 500000 / 10500000.0, 0.00001);
    EXPECT_NEAR(sysTimer.sysTime, 500000 / 10500000.0, 0.00001);

    // 2nd call
    TC0->TC_CHANNEL[2].TC_CV = 700000;

    UpdateSysTime();

    EXPECT_EQ(sysTimer.raw, 700000);
    EXPECT_NEAR(sysTimer.loopTime, 700000 / 10500000.0, 0.00001);
    EXPECT_NEAR(sysTimer.sysTime, 1200000 / 10500000.0, 0.00001);
}

// testing sysTime getter
TEST(test_sysTime, getSysTime)
{
    // 1st call
    EXPECT_NEAR(sysTimer.sysTime, 1200000 / 10500000.0, 0.00001);

    // 2nd call
    sysTimer.sysTime = 0.5;

    EXPECT_NEAR(getSysTime(), 0.5, 0.00001);
}


// testing sysLoopTime getter
TEST(test_sysTime, getSysLoopTime)
{
    // 1st call
    EXPECT_NEAR(sysTimer.loopTime, 700000 / 10500000.0, 0.00001);

    // 2nd call
    sysTimer.loopTime = 0.1;

    EXPECT_NEAR(getSysLoopTime(), 0.1, 0.00001);
}