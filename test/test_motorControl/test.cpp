#include "pch.h"

#include "../../src/MotorControl.h"
#include "../helper/support4Testing.h"
#include "../../src/sysTime.h"


extern Tc* TC1;
extern sysTime sysTimer;

// testing compilation
TEST(test_MotorControl, SetupMotorPins_Call)
{
	SetupMotorPins();
	MotorInput testMotorInput;
    UpdateMotorSpeeds(&testMotorInput);
	Setup_PB0_PB1_for_oneshot_pulse();
	Setup_PB2_PB3_for_oneshot_pulse();
	SetupMotorPins();
	MotorCommander testCmd;
	SetTcCompareRegister(&testCmd);
	TriggerTcRegisters(&testCmd);
	CalcMotorSpeeds(&testMotorInput, &testCmd);
	handleBeeps(&testMotorInput, &testCmd);
}

// testing SetTcCompareRegister method
TEST(test_MotorControl, SetTcCompareRegister_Commands)
{
	MotorCommander testCmd;

	testCmd.FL = 125;
	testCmd.FR = 150;
	testCmd.RL = 200;
	testCmd.RR = 250;

	SetTcCompareRegister(&testCmd);

	EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RA, uint32_t(125 * testCmd.const_TC_clock_freq));
	EXPECT_EQ(TC1->TC_CHANNEL[0].TC_RB, uint32_t(200 * testCmd.const_TC_clock_freq));
	EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RA, uint32_t(150 * testCmd.const_TC_clock_freq));
	EXPECT_EQ(TC1->TC_CHANNEL[1].TC_RB, uint32_t(250 * testCmd.const_TC_clock_freq));
}

// testing TriggerMotorRegisters method
TEST(test_MotorControl, TriggerTcRegisters_Tests)
{
	MotorCommander testCmd;
	testCmd.sysTime = 0;
	testCmd.lastpulseTime = 0;

	//1st
	TC1->TC_BCR = 0;
    testCmd.sysTime = 0.000050;
	TriggerTcRegisters(&testCmd);
	EXPECT_EQ(TC1->TC_BCR, 0);

	//2nd
	TC1->TC_BCR = 0;
    testCmd.sysTime = 0.000250;
	TriggerTcRegisters(&testCmd);
	EXPECT_EQ(TC1->TC_BCR, 0);

	//3rd
	TC1->TC_BCR = 0;
    testCmd.sysTime = 0.000300;
	TriggerTcRegisters(&testCmd);
	EXPECT_EQ(TC1->TC_BCR, 1);

}

// testing SetMotorSpeeds method
TEST(test_MotorControl, SetMotorSpeeds_Tests)
{
	MotorInput testMotorInput;
	MotorCommander testCmd;

	//1st: disarmed
	testMotorInput.armState = E_armState::DISARMED;

	CalcMotorSpeeds(&testMotorInput, &testCmd);
	EXPECT_NEAR(testCmd.FL, 125.0, 0.5);
	EXPECT_NEAR(testCmd.FR, 125.0, 0.5);
	EXPECT_NEAR(testCmd.RL, 125.0, 0.5);
	EXPECT_NEAR(testCmd.RR, 125.0, 0.5);

	//2nd: armed 1000-0-0-0
	testMotorInput.armState = E_armState::ARMED;
	testMotorInput.throttle = 1000;
	testMotorInput.x = 0;
	testMotorInput.y = 0;
	testMotorInput.z = 0;

	CalcMotorSpeeds(&testMotorInput, &testCmd);
	EXPECT_NEAR(testCmd.FL, 125.0, 0.5);
	EXPECT_NEAR(testCmd.FR, 125.0, 0.5);
	EXPECT_NEAR(testCmd.RL, 125.0, 0.5);
	EXPECT_NEAR(testCmd.RR, 125.0, 0.5);

	//3rd: 1200-0-0-0
	testMotorInput.armState = E_armState::ARMED;
	testMotorInput.throttle = 1200;
	testMotorInput.x = 0;
	testMotorInput.y = 0;
	testMotorInput.z = 0;

	CalcMotorSpeeds(&testMotorInput, &testCmd);
	EXPECT_NEAR(testCmd.FL, 150.0, 0.5);
	EXPECT_NEAR(testCmd.FR, 150.0, 0.5);
	EXPECT_NEAR(testCmd.RL, 150.0, 0.5);
	EXPECT_NEAR(testCmd.RR, 150.0, 0.5);

	//4th: 2000-0-0-0
	testMotorInput.armState = E_armState::ARMED;
	testMotorInput.throttle = 2000;
	testMotorInput.x = 0;
	testMotorInput.y = 0;
	testMotorInput.z = 0;

	CalcMotorSpeeds(&testMotorInput, &testCmd);
	EXPECT_NEAR(testCmd.FL, 250.0, 0.5);
	EXPECT_NEAR(testCmd.FR, 250.0, 0.5);
	EXPECT_NEAR(testCmd.RL, 250.0, 0.5);
	EXPECT_NEAR(testCmd.RR, 250.0, 0.5);

	//5th: 1000-10-0-0
	testMotorInput.armState = E_armState::ARMED;
	testMotorInput.throttle = 1000;
	testMotorInput.x = 10;
	testMotorInput.y = 0;
	testMotorInput.z = 0;

	CalcMotorSpeeds(&testMotorInput, &testCmd);
	EXPECT_NEAR(testCmd.FL, 135.0, 0.5);
	EXPECT_NEAR(testCmd.FR, 125.0, 0.5);
	EXPECT_NEAR(testCmd.RL, 135.0, 0.5);
	EXPECT_NEAR(testCmd.RR, 125.0, 0.5);

	//6th: 1200-10-0-0
	testMotorInput.armState = E_armState::ARMED;
	testMotorInput.throttle = 1200;
	testMotorInput.x = 10;
	testMotorInput.y = 0;
	testMotorInput.z = 0;

	CalcMotorSpeeds(&testMotorInput, &testCmd);
	EXPECT_NEAR(testCmd.FL, 160.0, 0.5);
	EXPECT_NEAR(testCmd.FR, 140.0, 0.5);
	EXPECT_NEAR(testCmd.RL, 160.0, 0.5);
	EXPECT_NEAR(testCmd.RR, 140.0, 0.5);

	//7th: 1200-10--10-0
	testMotorInput.armState = E_armState::ARMED;
	testMotorInput.throttle = 1200;
	testMotorInput.x = 10;
	testMotorInput.y = -10;
	testMotorInput.z = 0;

	CalcMotorSpeeds(&testMotorInput, &testCmd);
	EXPECT_NEAR(testCmd.FL, 150.0, 0.5);
	EXPECT_NEAR(testCmd.FR, 130.0, 0.5);
	EXPECT_NEAR(testCmd.RL, 170.0, 0.5);
	EXPECT_NEAR(testCmd.RR, 150.0, 0.5);

	//8th: 1000-10--10-5
	testMotorInput.armState = E_armState::ARMED;
	testMotorInput.throttle = 1200;
	testMotorInput.x = 10;
	testMotorInput.y = -10;
	testMotorInput.z = 5;

	CalcMotorSpeeds(&testMotorInput, &testCmd);
	EXPECT_NEAR(testCmd.FL, 155.0, 0.5);
	EXPECT_NEAR(testCmd.FR, 125.0, 0.5);
	EXPECT_NEAR(testCmd.RL, 165.0, 0.5);
	EXPECT_NEAR(testCmd.RR, 155.0, 0.5);


	//9th: 2000-10-10-5
	testMotorInput.armState = E_armState::ARMED;
	testMotorInput.throttle = 2000;
	testMotorInput.x = 10;
	testMotorInput.y = 10;
	testMotorInput.z = 5;

	CalcMotorSpeeds(&testMotorInput, &testCmd);
	EXPECT_NEAR(testCmd.FL, 250.0, 0.5);
	EXPECT_NEAR(testCmd.FR, 245.0, 0.5);
	EXPECT_NEAR(testCmd.RL, 245.0, 0.5);
	EXPECT_NEAR(testCmd.RR, 235.0, 0.5);

}

// testing UpdateMotorSpeeds method output
TEST(test_MotorControl, getMotorSpeeds_Test)
{
    MotorInput testMotorInput;
    MotorSpeeds testMotorSpeeds;

    //1st: default input
    getMotorSpeeds(&testMotorSpeeds);
    EXPECT_NEAR(testMotorSpeeds.FL, 125, 0.5);
    EXPECT_NEAR(testMotorSpeeds.FR, 125, 0.5);
    EXPECT_NEAR(testMotorSpeeds.RL, 125, 0.5);
    EXPECT_NEAR(testMotorSpeeds.RR, 125, 0.5);

    //2nd: added throttle
    testMotorInput.armState = E_armState::ARMED;
    testMotorInput.throttle = 1300;
    testMotorInput.x = 0;
    testMotorInput.y = 0;
    testMotorInput.z = 0;
    UpdateMotorSpeeds(&testMotorInput);
    getMotorSpeeds(&testMotorSpeeds);
    EXPECT_NEAR(testMotorSpeeds.FL, 162.5, 0.5);
    EXPECT_NEAR(testMotorSpeeds.FR, 162.5, 0.5);
    EXPECT_NEAR(testMotorSpeeds.RL, 162.5, 0.5);
    EXPECT_NEAR(testMotorSpeeds.RR, 162.5, 0.5);

}

TEST(test_MptorControl, handleBeeps_Test)
{
	MotorInput testMotorInput;
	MotorCommander testMotorCommander;

	//test: default input
	handleBeeps(&testMotorInput, &testMotorCommander);
	EXPECT_NEAR(testMotorCommander.FL, 125, 0.5);
	EXPECT_NEAR(testMotorCommander.FR, 125, 0.5);
	EXPECT_NEAR(testMotorCommander.RL, 125, 0.5);
	EXPECT_NEAR(testMotorCommander.RR, 125, 0.5);
	EXPECT_EQ(testMotorCommander.beepState, BEEP_TRIGGER);

	//test: under beep time
	testMotorInput.poti1 = 1100;
	testMotorInput.poti2 = 2000;
	testMotorInput.sysTime = 0.5;
	testMotorCommander.lastBeepTime = 0.4;
	handleBeeps(&testMotorInput, &testMotorCommander);
	EXPECT_NEAR(testMotorCommander.FL, 125, 0.5);
	EXPECT_NEAR(testMotorCommander.FR, 125, 0.5);
	EXPECT_NEAR(testMotorCommander.RL, 125, 0.5);
	EXPECT_NEAR(testMotorCommander.RR, 125, 0.5);
	EXPECT_EQ(testMotorCommander.beepState, BEEP_TRIGGER);

	//test: trigger beep
	testMotorInput.poti1 = 1100;
	testMotorInput.poti2 = 2000;
	testMotorInput.sysTime = 0.51;
	testMotorCommander.lastBeepTime = 0.4;
	handleBeeps(&testMotorInput, &testMotorCommander);
	EXPECT_NEAR(testMotorCommander.FL, 250, 0.5);
	EXPECT_NEAR(testMotorCommander.FR, 250, 0.5);
	EXPECT_NEAR(testMotorCommander.RL, 250, 0.5);
	EXPECT_NEAR(testMotorCommander.RR, 250, 0.5);
	EXPECT_EQ(testMotorCommander.beepState, BEEP_HOLD);

	//test: hold beep
	testMotorInput.poti1 = 1100;
	testMotorInput.poti2 = 2000;
	testMotorInput.sysTime = 0.40370;
	testMotorCommander.lastBeepTime = 0.4;
	handleBeeps(&testMotorInput, &testMotorCommander);
	EXPECT_NEAR(testMotorCommander.FL, 250, 0.5);
	EXPECT_NEAR(testMotorCommander.FR, 250, 0.5);
	EXPECT_NEAR(testMotorCommander.RL, 250, 0.5);
	EXPECT_NEAR(testMotorCommander.RR, 250, 0.5);
	EXPECT_EQ(testMotorCommander.beepState, BEEP_HOLD);

	//test: stop beep
	testMotorInput.poti1 = 1100;
	testMotorInput.poti2 = 2000;
	testMotorInput.sysTime = 0.40376;
	testMotorCommander.lastBeepTime = 0.4;
	handleBeeps(&testMotorInput, &testMotorCommander);
	EXPECT_NEAR(testMotorCommander.FL, 125, 0.5);
	EXPECT_NEAR(testMotorCommander.FR, 125, 0.5);
	EXPECT_NEAR(testMotorCommander.RL, 125, 0.5);
	EXPECT_NEAR(testMotorCommander.RR, 125, 0.5);
	EXPECT_EQ(testMotorCommander.beepState, BEEP_TRIGGER);

	//test: stay in trigger state
	testMotorInput.poti1 = 1100;
	testMotorInput.poti2 = 2000;
	testMotorInput.sysTime = 0.404;
	testMotorCommander.lastBeepTime = 0.4;
	handleBeeps(&testMotorInput, &testMotorCommander);
	EXPECT_NEAR(testMotorCommander.FL, 125, 0.5);
	EXPECT_NEAR(testMotorCommander.FR, 125, 0.5);
	EXPECT_NEAR(testMotorCommander.RL, 125, 0.5);
	EXPECT_NEAR(testMotorCommander.RR, 125, 0.5);
	EXPECT_EQ(testMotorCommander.beepState, BEEP_TRIGGER);

	//test: trigger beep again
	testMotorInput.poti1 = 1100;
	testMotorInput.poti2 = 2000;
	testMotorInput.sysTime = 0.51;
	testMotorCommander.lastBeepTime = 0.4;
	handleBeeps(&testMotorInput, &testMotorCommander);
	EXPECT_NEAR(testMotorCommander.FL, 250, 0.5);
	EXPECT_NEAR(testMotorCommander.FR, 250, 0.5);
	EXPECT_NEAR(testMotorCommander.RL, 250, 0.5);
	EXPECT_NEAR(testMotorCommander.RR, 250, 0.5);
	EXPECT_EQ(testMotorCommander.beepState, BEEP_HOLD);
}