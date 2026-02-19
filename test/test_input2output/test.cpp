#include "pch.h"

#include "../../src/MotorControl.h"
#include "../../src/Controller.h"
#include "../helper/support4Testing.h"
#include "../../src/sysTime.h"

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

    testIn.gyro.signal.x = 0.0f;
    testIn.gyro.signal.y = 0.0f;
    testIn.gyro.signal.z = 0.0f;
    testIn.gyro.newData = true;

    testIn.loopTime = 0.000250;

    //under test start
    RunController(&testIn, &testOut);

    testMotor.throttle = testIn.rcSignals.throttle;
    testMotor.x = testOut.U.x;
    testMotor.y = testOut.U.y;
    testMotor.z = testOut.U.z;
    testMotor.armState = testOut.armState;

    UpdateMotorSpeeds(&testMotor);
    //under test end

    EXPECT_NEAR(motorCommand.FL, 125, 0.1);
    EXPECT_NEAR(motorCommand.FR, 125, 0.1);
    EXPECT_NEAR(motorCommand.RL, 125, 0.1);
    EXPECT_NEAR(motorCommand.RR, 125, 0.1);
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

    testIn.rcSignals.throttle = 1200;
    testIn.rcSignals.roll = 1500;
    testIn.rcSignals.pitch = 1500;
    testIn.rcSignals.yaw = 1500;
    testIn.rcSignals.armStateSwitch = 2000;
    testIn.rcSignals.flightModeSwitch = 1000;

    testIn.gyro.signal.x = 0.0f;
    testIn.gyro.signal.y = 0.0f;
    testIn.gyro.signal.z = 0.0f;
    testIn.gyro.newData = true;

    testIn.loopTime = 0.000250;

    //under test start
    RunController(&testIn, &testOut);

    testMotor.throttle = testIn.rcSignals.throttle;
    testMotor.x = testOut.U.x;
    testMotor.y = testOut.U.y;
    testMotor.z = testOut.U.z;
    testMotor.armState = testOut.armState;

    UpdateMotorSpeeds(&testMotor);
    //under test end

    EXPECT_NEAR(motorCommand.FL, 150, 0.1);
    EXPECT_NEAR(motorCommand.FR, 150, 0.1);
    EXPECT_NEAR(motorCommand.RL, 150, 0.1);
    EXPECT_NEAR(motorCommand.RR, 150, 0.1);
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RA, 1575);	//diff: 1 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RB, 1575);	//diff: 1 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RA, 1575);	//diff: 1 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RB, 1575);	//diff: 1 -> OK
}

TEST(test_input2output_due4drone, 2000)
{
    controllerIn_st testIn;
    controllerOut_st testOut;
    MotorInput testMotor;

    SetupController();

    testIn.rcSignals.throttle = 2000;
    testIn.rcSignals.roll = 1500;
    testIn.rcSignals.pitch = 1500;
    testIn.rcSignals.yaw = 1500;
    testIn.rcSignals.armStateSwitch = 2000;
    testIn.rcSignals.flightModeSwitch = 1000;

    testIn.gyro.signal.x = 0.0f;
    testIn.gyro.signal.y = 0.0f;
    testIn.gyro.signal.z = 0.0f;
    testIn.gyro.newData = true;

    testIn.loopTime = 0.000250;

    //under test start
    RunController(&testIn, &testOut);

    testMotor.throttle = testIn.rcSignals.throttle;
    testMotor.x = testOut.U.x;
    testMotor.y = testOut.U.y;
    testMotor.z = testOut.U.z;
    testMotor.armState = testOut.armState;

    UpdateMotorSpeeds(&testMotor);
    //under test end

    EXPECT_NEAR(motorCommand.FL, 250, 0.1);
    EXPECT_NEAR(motorCommand.FR, 250, 0.1);
    EXPECT_NEAR(motorCommand.RL, 250, 0.1);
    EXPECT_NEAR(motorCommand.RR, 250, 0.1);
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RA, 2625);	//diff: 1 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RB, 2625);	//diff: 1 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RA, 2625);	//diff: 1 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RB, 2625);	//diff: 1 -> OK
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

    testIn.gyro.signal.x = 0.0f;
    testIn.gyro.signal.y = 0.0f;
    testIn.gyro.signal.z = 0.0f;
    testIn.gyro.newData = true;

    testIn.loopTime = 0.000250;

    pidRate.DTermC = 333 / 90.0;

    //under test start
    RunController(&testIn, &testOut);

    testMotor.throttle = testIn.rcSignals.throttle;
    testMotor.x = testOut.U.x;
    testMotor.y = testOut.U.y;
    testMotor.z = testOut.U.z;
    testMotor.armState = testOut.armState;

    UpdateMotorSpeeds(&testMotor);
    //under test end

    EXPECT_NEAR(motorCommand.FR, 149, 0.5);		//diff:  2 -> OK
    EXPECT_NEAR(motorCommand.FL, 151, 0.5);		//diff: -2 -> OK
    EXPECT_NEAR(motorCommand.RL, 151, 0.5);		//diff: -2 -> OK
    EXPECT_NEAR(motorCommand.RR, 149, 0.5);		//diff:  2 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RA, 1588);	//diff: 28 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RB, 1588);	//diff: 28 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RA, 1561);	//diff: 28 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RB, 1561);	//diff: 28 -> OK
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

    testIn.gyro.signal.x = 0.0f;
    testIn.gyro.signal.y = 0.0f;
    testIn.gyro.signal.z = 0.0f;
    testIn.gyro.newData = true;

    testIn.loopTime = 0.000250;

    pidRate.DTermC = 333 / 90.0;

    //under test start
    RunController(&testIn, &testOut);

    testMotor.throttle = testIn.rcSignals.throttle;
    testMotor.x = testOut.U.x;
    testMotor.y = testOut.U.y;
    testMotor.z = testOut.U.z;
    testMotor.armState = testOut.armState;

    UpdateMotorSpeeds(&testMotor);
    //under test end

    EXPECT_NEAR(motorCommand.FL, 150, 0.4);		//diff:  0 -> OK
    EXPECT_NEAR(motorCommand.FR, 147, 0.4);		//diff:  5 -> OK
    EXPECT_NEAR(motorCommand.RL, 153, 0.4);		//diff: -5 -> OK
    EXPECT_NEAR(motorCommand.RR, 150, 0.4);		//diff:  0 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RA, 1573);	//rr, diff:   1 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RB, 1603);	//fl, diff: -45 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RA, 1546);	//rl, diff:   2 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RB, 1576);	//fr, diff:  45 -> OK
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

    testIn.gyro.signal.x = 0.0f;
    testIn.gyro.signal.y = 0.0f;
    testIn.gyro.signal.z = 0.0f;
    testIn.gyro.newData = true;

    testIn.loopTime = 0.000250;

    pidRate.DTermC = 333 / 90.0;

    //under test start
    RunController(&testIn, &testOut);

    testMotor.throttle = testIn.rcSignals.throttle;
    testMotor.x = testOut.U.x;
    testMotor.y = testOut.U.y;
    testMotor.z = testOut.U.z;
    testMotor.armState = testOut.armState;

    UpdateMotorSpeeds(&testMotor);
    //under test end

    EXPECT_NEAR(motorCommand.FL, 155, 0.8);		//diff: -6 -> OK
    EXPECT_NEAR(motorCommand.FR, 142, 0.8);		//diff:  9 -> OK
    EXPECT_NEAR(motorCommand.RL, 148, 0.8);		//diff:  1 -> OK
    EXPECT_NEAR(motorCommand.RR, 155, 0.8);		//diff: -5 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RA, 1625);	//rr, diff: -57 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RB, 1550);	//fl, diff:   9 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RA, 1493);	//rl, diff: -53 -> OK
    EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RB, 1629);	//fr, diff: 101 -> OK
}