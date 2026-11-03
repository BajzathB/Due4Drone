#include <gtest/gtest.h>

#include "../src/MotorControl.h"
#include "../src/Controller.h"
#include "helper/support4Testing.h"

extern motorCommander motorCommand;
extern Tc* TC1;
extern pid_st pidRate;

TEST(test_input2output_due4drone, disarmed)
{
    controllerIn_st testIn;
    controllerOut_st testOut;
    MotorInput testMotor;

    SetupController();

    testIn.rcSignals.throttle = 1200;
    testIn.rcSignals.roll = 1500;
    testIn.rcSignals.pitch = 1500;
    testIn.rcSignals.yaw = 1500;
    testIn.rcSignals.armStateSwitch = 1000;
    testIn.rcSignals.flightModeSwitch = 1000;

    testIn.gyro.signalPT1.x = 0.0f;
    testIn.gyro.signalPT1.y = 0.0f;
    testIn.gyro.signalPT1.z = 0.0f;
    testIn.gyro.newData = true;

    //under test start
    RunController(&testIn, &testOut);

    testMotor.throttle = testIn.rcSignals.throttle;
    testMotor.x = testOut.U_i.x;
    testMotor.y = testOut.U_i.y;
    testMotor.z = testOut.U_i.z;
    testMotor.armState = testOut.armState;

    UpdateMotorSpeeds(&testMotor);
    //under test end

    EXPECT_EQ(motorCommand.FL_tick, 1312);
    EXPECT_EQ(motorCommand.FR_tick, 1312);
    EXPECT_EQ(motorCommand.RL_tick, 1312);
    EXPECT_EQ(motorCommand.RR_tick, 1312);
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RA, 1312);
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RB, 1312);
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RA, 1312);
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RB, 1312);
}

TEST(test_input2output_due4drone, 1200)
{
    controllerIn_st testIn;
    controllerOut_st testOut;
    MotorInput testMotor;

    SetupController();

    testIn.rcSignals.throttle = 1000;
    testIn.rcSignals.roll = 1500;
    testIn.rcSignals.pitch = 1500;
    testIn.rcSignals.yaw = 1500;
    testIn.rcSignals.armStateSwitch = 2000;
    testIn.rcSignals.flightModeSwitch = 1000;
    testIn.gyro.signalPT1.x = 0.0f;
    testIn.gyro.signalPT1.y = 0.0f;
    testIn.gyro.signalPT1.z = 0.0f;
    testIn.gyro.newData = true;
    pidRate.refSigPrev_i.x = 0;
    pidRate.refSigPrev_i.y = 0;
    pidRate.refSigPrev_i.z =0;
    pidRate.refSigDotPT1_i.x = 0;
    pidRate.refSigDotPT1_i.y = 0;
    pidRate.refSigDotPT1_i.z = 0;

    //under test start
    RunController(&testIn, &testOut);
    testIn.rcSignals.throttle = 1200;   //trigger arm state
    RunController(&testIn, &testOut);

    testMotor.throttle = testIn.rcSignals.throttle;
    testMotor.x = testOut.U_i.x;
    testMotor.y = testOut.U_i.y;
    testMotor.z = testOut.U_i.z;
    testMotor.armState = testOut.armState;

    UpdateMotorSpeeds(&testMotor);
    //under test end

    EXPECT_EQ(motorCommand.FL_tick, 1574);
    EXPECT_EQ(motorCommand.FR_tick, 1574);
    EXPECT_EQ(motorCommand.RL_tick, 1574);
    EXPECT_EQ(motorCommand.RR_tick, 1574);
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RA, 1574);
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RB, 1574);
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RA, 1574);
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RB, 1574);
}

TEST(test_input2output_due4drone, 2000)
{
    controllerIn_st testIn;
    controllerOut_st testOut;
    MotorInput testMotor;

    SetupController();

    testIn.rcSignals.throttle = 1000;
    testIn.rcSignals.roll = 1500;
    testIn.rcSignals.pitch = 1500;
    testIn.rcSignals.yaw = 1500;
    testIn.rcSignals.armStateSwitch = 2000;
    testIn.rcSignals.flightModeSwitch = 1000;

    testIn.gyro.signalPT1.x = 0.0f;
    testIn.gyro.signalPT1.y = 0.0f;
    testIn.gyro.signalPT1.z = 0.0f;
    testIn.gyro.newData = true;

    //under test start
    RunController(&testIn, &testOut);
    testIn.rcSignals.throttle = 2000;   //trigger arm state
    RunController(&testIn, &testOut);

    testMotor.throttle = testIn.rcSignals.throttle;
    testMotor.x = testOut.U_i.x;
    testMotor.y = testOut.U_i.y;
    testMotor.z = testOut.U_i.z;
    testMotor.armState = testOut.armState;

    UpdateMotorSpeeds(&testMotor);
    //under test end

    EXPECT_EQ(motorCommand.FL_tick, 2624);
    EXPECT_EQ(motorCommand.FR_tick, 2624);
    EXPECT_EQ(motorCommand.RL_tick, 2624);
    EXPECT_EQ(motorCommand.RR_tick, 2624);
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RA, 2624);
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RB, 2624);
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RA, 2624);
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RB, 2624);
}

TEST(test_input2output_due4drone, 1200_roll)
{
    controllerIn_st testIn;
    controllerOut_st testOut;
    MotorInput testMotor;

    SetupController();

    testIn.rcSignals.throttle = 1200;
    testIn.rcSignals.roll = 1700;
    testIn.rcSignals.pitch = 1500;
    testIn.rcSignals.yaw = 1500;
    testIn.rcSignals.armStateSwitch = 2000;
    testIn.rcSignals.flightModeSwitch = 1000;

    testIn.gyro.signalPT1.x = 0.0f;
    testIn.gyro.signalPT1.y = 0.0f;
    testIn.gyro.signalPT1.z = 0.0f;
    testIn.gyro.newData = true;

    //under test start
    RunController(&testIn, &testOut);

    testMotor.throttle = testIn.rcSignals.throttle;
    testMotor.x = testOut.U_i.x;
    testMotor.y = testOut.U_i.y;
    testMotor.z = testOut.U_i.z;
    testMotor.armState = testOut.armState;

    UpdateMotorSpeeds(&testMotor);
    //under test end

    EXPECT_EQ(motorCommand.FR_tick, 1496);	
    EXPECT_EQ(motorCommand.FL_tick, 1652);	
    EXPECT_EQ(motorCommand.RL_tick, 1652);	
    EXPECT_EQ(motorCommand.RR_tick, 1496);	
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RA, 1652);
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RB, 1652);
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RA, 1496);
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RB, 1496);
}

TEST(test_input2output_due4drone, 1200_roll_pitch)
{
    controllerIn_st testIn;
    controllerOut_st testOut;
    MotorInput testMotor;

    SetupController();

    testIn.rcSignals.throttle = 1200;
    testIn.rcSignals.roll = 1700;
    testIn.rcSignals.pitch = 1700;
    testIn.rcSignals.yaw = 1500;
    testIn.rcSignals.armStateSwitch = 2000;
    testIn.rcSignals.flightModeSwitch = 1000;

    testIn.gyro.signalPT1.x = 0.0f;
    testIn.gyro.signalPT1.y = 0.0f;
    testIn.gyro.signalPT1.z = 0.0f;
    testIn.gyro.newData = true;

    //under test start
    RunController(&testIn, &testOut);

    testMotor.throttle = testIn.rcSignals.throttle;
    testMotor.x = testOut.U_i.x;
    testMotor.y = testOut.U_i.y;
    testMotor.z = testOut.U_i.z;
    testMotor.armState = testOut.armState;

    UpdateMotorSpeeds(&testMotor);
    //under test end

    EXPECT_EQ(motorCommand.FL_tick, 1570);
    EXPECT_EQ(motorCommand.FR_tick, 1420);
    EXPECT_EQ(motorCommand.RL_tick, 1728);
    EXPECT_EQ(motorCommand.RR_tick, 1578);
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RA, 1570);
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RB, 1728);
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RA, 1420);
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RB, 1578);
}

TEST(test_input2output_due4drone, 1200_roll_pitch_yaw)
{
    controllerIn_st testIn;
    controllerOut_st testOut;
    MotorInput testMotor;

    SetupController();

    testIn.rcSignals.throttle = 1200;
    testIn.rcSignals.roll = 1700;
    testIn.rcSignals.pitch = 1700;
    testIn.rcSignals.yaw = 1600;
    testIn.rcSignals.armStateSwitch = 2000;
    testIn.rcSignals.flightModeSwitch = 1000;

    testIn.gyro.signalPT1.x = 0.0f;
    testIn.gyro.signalPT1.y = 0.0f;
    testIn.gyro.signalPT1.z = 0.0f;
    testIn.gyro.newData = true;

    //under test start
    RunController(&testIn, &testOut);

    testMotor.throttle = testIn.rcSignals.throttle;
    testMotor.x = testOut.U_i.x;
    testMotor.y = testOut.U_i.y;
    testMotor.z = testOut.U_i.z;
    testMotor.armState = testOut.armState;

    UpdateMotorSpeeds(&testMotor);
    //under test end

    EXPECT_EQ(motorCommand.FL_tick, 1970);
    EXPECT_EQ(motorCommand.FR_tick, 1312);
    EXPECT_EQ(motorCommand.RL_tick, 1324);
    EXPECT_EQ(motorCommand.RR_tick, 1976);
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RA, 1970);
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RB, 1324);
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RA, 1312);
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RB, 1976);
}