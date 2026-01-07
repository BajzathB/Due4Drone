#include "pch.h"

#include "../../src/Controller.h"

extern pid_st pidRate;
extern pid_st pidCascade;
extern E_armState armState;

TEST(test_Controller, Controller_Call) {
    SetupController();
    controllerIn_st testIn;
    controllerOut_st testOut;
    RunController(&testIn, &testOut);
	rcSignals_st testSig;
    EvalArmState(&testSig);
    EvalFlightMode(1000);
    ParabolicScale(1000);
    LinearInterpol(1000,1000,2000,-45,45);
	float y{ 0 }, x{ 0 }, paramC{ 1 };
	PT1Filter(&y, x, paramC);
    pid_st testPID;
    axis testU;
	CalcPID_wo_Dkick(&testPID, &testU);
	CalcPID_wo_Dkick_FF(&testPID, &testU);
	getPIDrates();
	getGyroData();
	kalmanfilter_st testKalman;
	KalmanFilter(&testKalman, 0);
	kalmanFilterAngle_st testKalmanAngle;
	KalmanFilterAngle(&testKalmanAngle, 0,0,0);
}

TEST(test_Controller, EvalArmState_Test)
{
	rcSignals_st testSig;
    
    //1st: lower than 1800
	testSig.armStateSwitch = 1000;
	EvalArmState(&testSig);
    EXPECT_EQ(armState, DISARMED);

	//2nd: higher than 1800, throttle also high
	testSig.armStateSwitch = 2000;
	testSig.throttle = 1500;
	EvalArmState(&testSig);
	EXPECT_EQ(armState, DISARMED);

    //3rd: higher than 1800
	testSig.armStateSwitch = 2000;
	testSig.throttle = 1010;
	EvalArmState(&testSig);
    EXPECT_EQ(armState, ARMED);

	//4th: keep being armed
	testSig.throttle = 1250;
	EvalArmState(&testSig);
	EXPECT_EQ(armState, ARMED);

	//5th: disarm
	testSig.armStateSwitch = 1000;
	testSig.throttle = 1250;
	EvalArmState(&testSig);
	EXPECT_EQ(armState, DISARMED);
}

TEST(test_Controller, EvalFlightMode_Test)
{
    uint16_t testFlightMode{ 2000u };

    //1st: lower than 1950
    EXPECT_EQ(EvalFlightMode(testFlightMode), ANGLE_CASCADE_CTRL);

    //2nd: inbetween 1450-1550
    testFlightMode = 1500u;
    EXPECT_EQ(EvalFlightMode(testFlightMode), PT1_WO_DERIVATIVE_KICK);

    //3rd: none of the above two tests
    testFlightMode = 1200u;
    EXPECT_EQ(EvalFlightMode(testFlightMode), PT2_WO_DERIVATIVE_KICK);
}

TEST(test_Controller, ParabolicScale_Test)
{
    //note: values from parabolicStickScale excel
    uint16_t testChannel{ 1500 };
    //1st: middle
    EXPECT_NEAR(ParabolicScale(testChannel), 0, 0.1);

    //2nd: 100 to right
    testChannel = 1600;
    EXPECT_NEAR(ParabolicScale(testChannel), 20, 0.1);

    //3rd: 100 to left
    testChannel = 1400;
    EXPECT_NEAR(ParabolicScale(testChannel), -20, 0.1);

    //4th: 300 to right
    testChannel = 1800;
    EXPECT_NEAR(ParabolicScale(testChannel), 200, 0.1);

    //5th: 300 to left
    testChannel = 1200;
    EXPECT_NEAR(ParabolicScale(testChannel), -200, 0.1);
}

TEST(test_Controller, LinearInterpol_Test)
{
    //1st: 1000 with -45<->45
    EXPECT_NEAR(LinearInterpol(1000, 1000, 2000, -45, 45), -45, 0.1);
    //2nd: 2000 with -45<->45
    EXPECT_NEAR(LinearInterpol(2000, 1000, 2000, -45, 45), 45, 0.1);
    //3rd: 1500 with -45<->45
    EXPECT_NEAR(LinearInterpol(1500, 1000, 2000, -45, 45), 0, 0.1);
    //4th: 1200 with 30<->60
    EXPECT_NEAR(LinearInterpol(1200, 1000, 2000, 30, 60), 36, 0.1);
}

TEST(test_Controller, PT1Filter_Test)
{
	float y{ 0 }, x{ 0 }, paramC{ 1 };

	//1st
	y = 0.0f;
	x = 5.0f;
	paramC = 2.0f;
	PT1Filter(&y, x, paramC);
	EXPECT_NEAR(y, 5.0/3.0, 0.1);

	//2nd
	y = 1.0f;
	x = 3.0f;
	paramC = 1.5f;
	PT1Filter(&y, x, paramC);
	EXPECT_NEAR(y, 4.5 / 2.5, 0.1);
}

TEST(test_Controller, CalcPID_wo_Dkick_Test)
{
    pid_st testPID;
    axis testU;

	//1st
	testPID.refSignal.x = 20.0f;
	testPID.refSignal.y = 25.0f;
	testPID.refSignal.z = -30.0f;
	testPID.sensor.signal.x = 10.0f;
	testPID.sensor.signal.y = 40.0f;
	testPID.sensor.signal.z = -5.0f;
	testPID.sensor.newData = true;
	testPID.P.x = 10000.0f;
	testPID.P.y = 50000.0f;
	testPID.P.z = 100000.0f;
	testPID.I.x = 2000.0f;
	testPID.I.y = 6000.0f;
	testPID.I.z = 12000.0f;
	testPID.D.x = 30000000.0f;
	testPID.D.y = 15000000.0f;
	testPID.D.z = 200000000.0f;
	testPID.error.x = 0.0f;
	testPID.error.y = 0.0f;
	testPID.error.z = 0.0f;
	testPID.errorSum.x = 0.0f;
	testPID.errorSum.y = 0.0f;
	testPID.errorSum.z = 0.0f;
	testPID.errorDot.x = 0.0f;
	testPID.errorDot.y = 0.0f;
	testPID.errorDot.z = 0.0f;
	testPID.errorPrev.x = 0.0f;
	testPID.errorPrev.y = 0.0f;
	testPID.errorPrev.z = 0.0f;
	testPID.errorDotFiltered.x = 0.0f;
	testPID.errorDotFiltered.y = 0.0f;
	testPID.errorDotFiltered.z = 0.0f;
	testPID.deltaT = 0.1;
	testPID.DTermC = 2.0f;
	testPID.saturationI = 10.0f;
	testPID.saturationPID = 200.0f;
    testPID.PFactor = 10000.0f;
    testPID.IFactor = 100.0f;
    testPID.DFactor = 1000000.0f;
    testPID.FFrFactor = 1000.0f;
    testPID.FFdrFactor = 10000.0f;
	CalcPID_wo_Dkick(&testPID, &testU);
	EXPECT_NEAR(testPID.error.x, 10, 0.1);
	EXPECT_NEAR(testPID.error.y, -15, 0.1);
	EXPECT_NEAR(testPID.error.z, -25, 0.1);
	EXPECT_NEAR(testPID.errorSum.x, 1, 0.1);
	EXPECT_NEAR(testPID.errorSum.y, -1.5, 0.1);
	EXPECT_NEAR(testPID.errorSum.z, -2.5, 0.1);
	EXPECT_NEAR(testPID.errorDot.x, 100.0, 0.1);
	EXPECT_NEAR(testPID.errorDot.y, 400.0, 0.1);
	EXPECT_NEAR(testPID.errorDot.z, -50.0, 0.1);
	EXPECT_NEAR(testPID.errorPrev.x, 0.0, 0.1);
	EXPECT_NEAR(testPID.errorPrev.y, 0.0, 0.1);
	EXPECT_NEAR(testPID.errorPrev.z, 0.0, 0.1);
	EXPECT_NEAR(testPID.errorDotFiltered.x, 100.0/3.0, 0.1);
	EXPECT_NEAR(testPID.errorDotFiltered.y, 400.0/3.0, 0.1);
	EXPECT_NEAR(testPID.errorDotFiltered.z, -50.0/3.0, 0.1);
	EXPECT_NEAR(testPID.Pout.x, 10, 0.1);
	EXPECT_NEAR(testPID.Pout.y, -75, 0.1);
	EXPECT_NEAR(testPID.Pout.z, -250, 0.1);
	EXPECT_NEAR(testPID.Iout.x, 20, 0.1);
	EXPECT_NEAR(testPID.Iout.y, -90, 0.1);
	EXPECT_NEAR(testPID.Iout.z, -300, 0.1);
	EXPECT_NEAR(testPID.Dout.x, 1000, 0.1);
	EXPECT_NEAR(testPID.Dout.y, 2000, 0.1);
	EXPECT_NEAR(testPID.Dout.z, -3333.3f, 0.1);
	EXPECT_FALSE(testPID.sensor.newData);
	EXPECT_NEAR(testU.x, -200, 0.1);
	EXPECT_NEAR(testU.y, -200, 0.1);
	EXPECT_NEAR(testU.z, 200, 0.1);

	//2nd: sum increament, delta stays as no newdata
	CalcPID_wo_Dkick(&testPID, &testU);
	EXPECT_NEAR(testPID.errorSum.x, 2, 0.1);
	EXPECT_NEAR(testPID.errorSum.y, -3, 0.1);
	EXPECT_NEAR(testPID.errorSum.z, -5, 0.1);
	EXPECT_NEAR(testPID.errorDotFiltered.x, 100.0 / 3.0, 0.1);
	EXPECT_NEAR(testPID.errorDotFiltered.y, 400.0 / 3.0, 0.1);
	EXPECT_NEAR(testPID.errorDotFiltered.z, -50.0 / 3.0, 0.1);

	//3rd: I and overall saturation
	testPID.sensor.signal.x = -5.0f;
	testPID.sensor.signal.y = 80.0f;
	testPID.sensor.signal.z = 20.0f;
	testPID.sensor.newData = true;
	testPID.saturationI = 3.0f;
	testPID.saturationPID = 200.0f;
	CalcPID_wo_Dkick(&testPID, &testU);
	EXPECT_NEAR(testPID.errorSum.x, testPID.saturationI, 0.1);
	EXPECT_NEAR(testPID.errorSum.y, -testPID.saturationI, 0.1);
	EXPECT_NEAR(testPID.errorSum.z, -testPID.saturationI, 0.1);
	EXPECT_NEAR(testU.x, testPID.saturationPID, 0.1);
	EXPECT_NEAR(testU.y, -testPID.saturationPID, 0.1);
	EXPECT_NEAR(testU.z, -testPID.saturationPID, 0.1);
}

TEST(test_Controller, CalcPID_wo_Dkick_FF_Test)
{
	pid_st testPIDff;
	axis testUff;

	//1st
	testPIDff.refSignal.x = 20.0f;
	testPIDff.refSignal.y = 25.0f;
	testPIDff.refSignal.z = -30.0f;
	testPIDff.refSignalPrev.x = 0.0f;
	testPIDff.refSignalPrev.y = 0.0f;
	testPIDff.refSignalPrev.z = 0.0f;
	testPIDff.sensor.signal.x = 10.0f;
	testPIDff.sensor.signal.y = 40.0f;
	testPIDff.sensor.signal.z = -5.0f;
	testPIDff.sensor.newData = true;
	testPIDff.P.x = 10000.0f;
	testPIDff.P.y = 50000.0f;
	testPIDff.P.z = 100000.0f;
	testPIDff.I.x = 2000.0f;
	testPIDff.I.y = 6000.0f;
	testPIDff.I.z = 12000.0f;
	testPIDff.D.x = 30000000.0f;
	testPIDff.D.y = 15000000.0f;
	testPIDff.D.z = 200000000.0f;
	testPIDff.FFr.x = 100000.0f;
	testPIDff.FFr.y = 100000.0f;
	testPIDff.FFr.z = 100000.0f;
	testPIDff.FFdr.x = 10000.0f;
	testPIDff.FFdr.y = 10000.0f;
	testPIDff.FFdr.z = 10000.0f;
	testPIDff.error.x = 0.0f;
	testPIDff.error.y = 0.0f;
	testPIDff.error.z = 0.0f;
	testPIDff.errorSum.x = 0.0f;
	testPIDff.errorSum.y = 0.0f;
	testPIDff.errorSum.z = 0.0f;
	testPIDff.errorDot.x = 0.0f;
	testPIDff.errorDot.y = 0.0f;
	testPIDff.errorDot.z = 0.0f;
	testPIDff.errorPrev.x = 0.0f;
	testPIDff.errorPrev.y = 0.0f;
	testPIDff.errorPrev.z = 0.0f;
	testPIDff.errorDotFiltered.x = 0.0f;
	testPIDff.errorDotFiltered.y = 0.0f;
	testPIDff.errorDotFiltered.z = 0.0f;
	testPIDff.deltaT = 0.1;
	testPIDff.DTermC = 2.0f;
	testPIDff.saturationI = 10.0f;
	testPIDff.saturationPID = 200.0f;
    testPIDff.PFactor = 10000.0f;
    testPIDff.IFactor = 100.0f;
    testPIDff.DFactor = 1000000.0f;
    testPIDff.FFrFactor = 1000.0f;
    testPIDff.FFdrFactor = 10000.0f;
	CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
	EXPECT_NEAR(testPIDff.error.x, 10, 0.1);
	EXPECT_NEAR(testPIDff.error.y, -15, 0.1);
	EXPECT_NEAR(testPIDff.error.z, -25, 0.1);
	EXPECT_NEAR(testPIDff.errorSum.x, 1, 0.1);
	EXPECT_NEAR(testPIDff.errorSum.y, -1.5, 0.1);
	EXPECT_NEAR(testPIDff.errorSum.z, -2.5, 0.1);
	EXPECT_NEAR(testPIDff.errorDot.x, 100.0, 0.1);
	EXPECT_NEAR(testPIDff.errorDot.y, 400.0, 0.1);
	EXPECT_NEAR(testPIDff.errorDot.z, -50.0, 0.1);
	EXPECT_NEAR(testPIDff.errorPrev.x, 0.0, 0.1);
	EXPECT_NEAR(testPIDff.errorPrev.y, 0.0, 0.1);
	EXPECT_NEAR(testPIDff.errorPrev.z, 0.0, 0.1);
	EXPECT_NEAR(testPIDff.errorDotFiltered.x, 100.0 / 3.0, 0.1);
	EXPECT_NEAR(testPIDff.errorDotFiltered.y, 400.0 / 3.0, 0.1);
	EXPECT_NEAR(testPIDff.errorDotFiltered.z, -50.0 / 3.0, 0.1);
	EXPECT_NEAR(testPIDff.refSignalDot.x, 200.0, 0.1);
	EXPECT_NEAR(testPIDff.refSignalDot.y, 250.0, 0.1);
	EXPECT_NEAR(testPIDff.refSignalDot.z, -300.0, 0.1);
	EXPECT_NEAR(testPIDff.refSignalDotFiltered.x, 18.18, 0.1);
	EXPECT_NEAR(testPIDff.refSignalDotFiltered.y, 22.72, 0.1);
	EXPECT_NEAR(testPIDff.refSignalDotFiltered.z, -27.27, 0.1);
	EXPECT_NEAR(testPIDff.Pout.x, 10, 0.1);
	EXPECT_NEAR(testPIDff.Pout.y, -75, 0.1);
	EXPECT_NEAR(testPIDff.Pout.z, -250, 0.1);
	EXPECT_NEAR(testPIDff.Iout.x, 20, 0.1);
	EXPECT_NEAR(testPIDff.Iout.y, -90, 0.1);
	EXPECT_NEAR(testPIDff.Iout.z, -300, 0.1);
	EXPECT_NEAR(testPIDff.Dout.x, 1000, 0.1);
	EXPECT_NEAR(testPIDff.Dout.y, 2000, 0.1);
	EXPECT_NEAR(testPIDff.Dout.z, -3333.3f, 0.1);
	EXPECT_NEAR(testPIDff.FFout.x, 2018.18, 0.1);
	EXPECT_NEAR(testPIDff.FFout.y, 2522.72, 0.1);
	EXPECT_NEAR(testPIDff.FFout.z, -3027.27, 0.1);
	EXPECT_FALSE(testPIDff.sensor.newData);
	EXPECT_NEAR(testUff.x, 200, 0.1);
	EXPECT_NEAR(testUff.y, 200, 0.1);
	EXPECT_NEAR(testUff.z, -200, 0.1);

	//2nd: sum increament, delta stays as no newdata
	CalcPID_wo_Dkick(&testPIDff, &testUff);
	EXPECT_NEAR(testPIDff.errorSum.x, 2, 0.1);
	EXPECT_NEAR(testPIDff.errorSum.y, -3, 0.1);
	EXPECT_NEAR(testPIDff.errorSum.z, -5, 0.1);
	EXPECT_NEAR(testPIDff.errorDotFiltered.x, 100.0 / 3.0, 0.1);
	EXPECT_NEAR(testPIDff.errorDotFiltered.y, 400.0 / 3.0, 0.1);
	EXPECT_NEAR(testPIDff.errorDotFiltered.z, -50.0 / 3.0, 0.1);

	//3rd: I and overall saturation
	testPIDff.sensor.signal.x = -5.0f;
	testPIDff.sensor.signal.y = 80.0f;
	testPIDff.sensor.signal.z = 20.0f;
	testPIDff.sensor.newData = true;
	testPIDff.saturationI = 3.0f;
	testPIDff.saturationPID = 200.0f;
	CalcPID_wo_Dkick(&testPIDff, &testUff);
	EXPECT_NEAR(testPIDff.errorSum.x, testPIDff.saturationI, 0.1);
	EXPECT_NEAR(testPIDff.errorSum.y, -testPIDff.saturationI, 0.1);
	EXPECT_NEAR(testPIDff.errorSum.z, -testPIDff.saturationI, 0.1);
	EXPECT_NEAR(testUff.x, testPIDff.saturationPID, 0.1);
	EXPECT_NEAR(testUff.y, -testPIDff.saturationPID, 0.1);
	EXPECT_NEAR(testUff.z, -testPIDff.saturationPID, 0.1);
}

TEST(test_Controller, RunController_Test)
{
	controllerIn_st testIn;
	controllerOut_st testOut;

	//1st: disamred
	testIn.rcSignals.armStateSwitch = 1000;
	testIn.rcSignals.flightModeSwitch = 1500;
	testIn.rcSignals.throttle = 1000;
	testOut.U.x = 1.0f;
	testOut.U.y = 1.0f;
	testOut.U.z = 1.0f;
	RunController(&testIn, &testOut);
	EXPECT_NEAR(testOut.U.x, 0.0, 0.1);
	EXPECT_NEAR(testOut.U.y, 0.0, 0.1);
	EXPECT_NEAR(testOut.U.z, 0.0, 0.1);
	EXPECT_EQ(testOut.armState, DISARMED);

	//2nd: armed but low throttle
	testIn.rcSignals.armStateSwitch = 2000;
	testIn.rcSignals.flightModeSwitch = 1500;
	testIn.rcSignals.throttle = 1000;
	testOut.U.x = 1.0f;
	testOut.U.y = 1.0f;
	testOut.U.z = 1.0f;
	RunController(&testIn, &testOut);
	EXPECT_NEAR(testOut.U.x, 0.0, 0.1);
	EXPECT_NEAR(testOut.U.y, 0.0, 0.1);
	EXPECT_NEAR(testOut.U.z, 0.0, 0.1);
	EXPECT_EQ(testOut.armState, ARMED);

	//3rd: armed in angle
	testIn.rcSignals.armStateSwitch = 2000;
	testIn.rcSignals.flightModeSwitch = 1500;
	testIn.rcSignals.throttle = 1100;
	testOut.U.x = 1.0f;
	testOut.U.y = 1.0f;
	testOut.U.z = 1.0f;
	RunController(&testIn, &testOut);
	EXPECT_NEAR(testOut.U.x, 0.0, 0.1);
	EXPECT_NEAR(testOut.U.y, 0.0, 0.1);
	EXPECT_NEAR(testOut.U.z, 0.0, 0.1);
	EXPECT_EQ(testOut.armState, ARMED);

	//4th: armed in rate
	testIn.rcSignals.armStateSwitch = 2000;
	testIn.rcSignals.flightModeSwitch = 1000;
	testIn.rcSignals.throttle = 1100;
	testIn.gyro.signal.x = 10.0f;
	testIn.gyro.signal.y = 10.0f;
	testIn.gyro.signal.z = 10.0f;
	testIn.gyro.newData = true;
	testIn.loopTime = 0.1f;
	pidRate.P.x = 10.0f;
	pidRate.P.y = 10.0f;
	pidRate.P.z = 10.0f;
	pidRate.saturationPID = 100.0f;
	testOut.U.x = 1.0f;
	testOut.U.y = 1.0f;
	testOut.U.z = 1.0f;
	RunController(&testIn, &testOut);
	EXPECT_NE(testOut.U.x, 0.0f);
	EXPECT_NE(testOut.U.y, 0.0f);
	EXPECT_NE(testOut.U.z, 0.0f);
    EXPECT_EQ(testOut.armState, ARMED);

    //5th: armed in angle cascade
    testIn.rcSignals.armStateSwitch = 2000;
    testIn.rcSignals.flightModeSwitch = 2000;
    testIn.rcSignals.throttle = 1100;
    testIn.acc.signal.x = 1.0f;
    testIn.acc.signal.y = 1.0f;
    testIn.acc.signal.z = 1.0f;
    testIn.acc.newData = false;
    testIn.gyro.signal.x = 10.0f;
    testIn.gyro.signal.y = 10.0f;
    testIn.gyro.signal.z = 10.0f;
    testIn.gyro.newData = true;
    testIn.loopTime = 0.1f;
    pidCascade.P.x = 10.0f;
    pidCascade.P.y = 10.0f;
    pidCascade.P.z = 10.0f;
    pidRate.P.x = 10.0f;
    pidRate.P.y = 10.0f;
    pidRate.P.z = 10.0f;
    pidRate.saturationPID = 100.0f;
    testOut.U.x = 1.0f;
    testOut.U.y = 1.0f;
    testOut.U.z = 1.0f;
    RunController(&testIn, &testOut);
    EXPECT_NE(testOut.U.x, 0.0f);
    EXPECT_NE(testOut.U.y, 0.0f);
    EXPECT_NE(testOut.U.z, 0.0f);
    EXPECT_EQ(testOut.armState, ARMED);

}

TEST(test_Controller, KalmanFilter_Test)
{
	kalmanfilter_st testKalman;
	float testInput{0};

	//1st: 
	KalmanFilter(&testKalman, testInput);
	EXPECT_NEAR(testKalman.value, 0.0f, 0.5f);

	//2nd: 
	testInput = 1.0f;
	KalmanFilter(&testKalman, testInput);
	EXPECT_NEAR(testKalman.value, 0.16f, 0.5f);

	//3rd: 
	testInput = 1.5f;
	KalmanFilter(&testKalman, testInput);
	EXPECT_NEAR(testKalman.value, 0.36f, 0.5f);

	//4th: 
	testInput = 5.0f;
	KalmanFilter(&testKalman, testInput);
	EXPECT_NEAR(testKalman.value, 0.95f, 0.5f);
}

TEST(test_Controller, KalmanFilterAngle_Test)
{
	kalmanFilterAngle_st testKalmanAngle;
	float testAccAngle{ 0 };
	float testGyroIn{ 0 };
	float testLoopTime{ 0 };

	//1st: 0 angle 0 rate
	testKalmanAngle.P[0][0] = 0;
	testKalmanAngle.P[0][1] = 0;
	testKalmanAngle.P[1][0] = 0;
	testKalmanAngle.P[1][1] = 0;
	testAccAngle = 0;
	testGyroIn = 0;
	testLoopTime = 0.1;
	KalmanFilterAngle(&testKalmanAngle, testAccAngle, testGyroIn, testLoopTime);
	EXPECT_NEAR(testKalmanAngle.angle, 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.bias, 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.rate, 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.P[0][0], 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.P[1][0], 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.P[0][1], 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.P[1][1], 0.0f, 0.01f);

	//2nd: 10 angle 0 rate
	testKalmanAngle.P[0][0] = 0;
	testKalmanAngle.P[0][1] = 0;
	testKalmanAngle.P[1][0] = 0;
	testKalmanAngle.P[1][1] = 0;
	testAccAngle = 10;
	testGyroIn = 0;
	testLoopTime = 0.1;
	KalmanFilterAngle(&testKalmanAngle, testAccAngle, testGyroIn, testLoopTime);
	EXPECT_NEAR(testKalmanAngle.angle, 0.0000005f, 0.0000001f);
	EXPECT_NEAR(testKalmanAngle.bias, 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.rate, 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.P[0][0], 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.P[1][0], 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.P[0][1], 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.P[1][1], 0.0f, 0.01f);

	//3rd: 10 angle 100 rate
	testKalmanAngle.P[0][0] = 0;
	testKalmanAngle.P[0][1] = 0;
	testKalmanAngle.P[1][0] = 0;
	testKalmanAngle.P[1][1] = 0;
	testAccAngle = 10;
	testGyroIn = 100;
	testLoopTime = 0.1;
	KalmanFilterAngle(&testKalmanAngle, testAccAngle, testGyroIn, testLoopTime);
	EXPECT_NEAR(testKalmanAngle.angle, 10.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.bias, 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.rate, 100.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.P[0][0], 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.P[1][0], 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.P[0][1], 0.0f, 0.01f);
	EXPECT_NEAR(testKalmanAngle.P[1][1], 0.0f, 0.01f);
}
