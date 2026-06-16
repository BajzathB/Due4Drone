#include "pch.h"

#include "../../src/Controller.h"
#include "../../src/SPI_common.h"

extern pid_st pidRate;
extern pid_st pidCascade;
extern E_armState armState;
extern float wobbleTime;

TEST(test_Controller, Controller_Call) {
    SetupController();
    controllerIn_st testIn;
    controllerOut_st testOut;
    RunController(&testIn, &testOut);
    rcSignals_st testSig;
    EvalArmState(&testSig);
    EvalFlightMode(1000);
    pid_st testPID;
    //CalcPID_wo_Dkick(&testPID, &testU);
    //CalcPID_wo_Dkick_FF(&testPID, &testU);
    getPIDrates();
    getGyroData();
    kalmanFilterAngle_st testKalmanAngle;
    KalmanFilterAngle(&testKalmanAngle, 0, 0, 0);
    //ComplementryFilterAngle(&y, 0, 0, 0, 0);
    //ComplementryFilterAngleWeighted(&y, 0, 0, 0, 0, &testU);
    maxVal(0,1);
    minVal(0,1);
    //CalcPID_wo_Dkick_FF_IRelax_Dmax(&testPID, &testU, 1000);
    //calcIRelaxFactor(&testU, &testPID, 1000);
    //calcDmaxFactor(&testU, &testPID);
    wobble(1000, 1000);
    CalcProportional_int(&testPID);
    CalcIntegral_int(&testPID);
    CalcDerivative_int(&testPID);
    CalcFeedforward_int(&testPID);
    axis_i32 testU_i32;
    ScalePIDinput_int(&testPID, 0);
    CalcPIDoutput_int(&testPID, &testU_i32);
    expo(1000);
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
    auto test1 = EvalFlightMode(testFlightMode);

    //2nd: inbetween 1450-1550
    testFlightMode = 1500u;
    auto test2 = EvalFlightMode(testFlightMode);
    EXPECT_NE(test2, test1);

    //3rd: none of the above two tests
    testFlightMode = 1200u;
    auto test3 = EvalFlightMode(testFlightMode);
    EXPECT_NE(test2, test1);
    EXPECT_NE(test3, test1);
}

TEST(test_Controller, expo_Test)
{
    EXPECT_EQ(expo(1500), 0);
    EXPECT_EQ(expo(2000), 16051);
    EXPECT_EQ(expo(1000), -16052);
    EXPECT_EQ(expo(1200), -2032);
    EXPECT_EQ(expo(1700), 749);
}

//TEST(test_Controller, CalcPID_wo_Dkick_Test)
//{
//    pid_st testPID;
//    axis testU;
//
//    //1st
//    testPID.refSignal.x = 20.0f;
//    testPID.refSignal.y = 25.0f;
//    testPID.refSignal.z = -30.0f;
//    testPID.sensor.signal.x = 10.0f;
//    testPID.sensor.signal.y = 40.0f;
//    testPID.sensor.signal.z = -5.0f;
//    testPID.sensor.newData = true;
//    testPID.P.x = 10000.0f;
//    testPID.P.y = 50000.0f;
//    testPID.P.z = 100000.0f;
//    testPID.I.x = 2000.0f;
//    testPID.I.y = 6000.0f;
//    testPID.I.z = 12000.0f;
//    testPID.D.x = 30000000.0f;
//    testPID.D.y = 15000000.0f;
//    testPID.D.z = 200000000.0f;
//    testPID.error.x = 0.0f;
//    testPID.error.y = 0.0f;
//    testPID.error.z = 0.0f;
//    testPID.errorSum.x = 0.0f;
//    testPID.errorSum.y = 0.0f;
//    testPID.errorSum.z = 0.0f;
//    testPID.errorDot.x = 0.0f;
//    testPID.errorDot.y = 0.0f;
//    testPID.errorDot.z = 0.0f;
//    testPID.errorPrev.x = 0.0f;
//    testPID.errorPrev.y = 0.0f;
//    testPID.errorPrev.z = 0.0f;
//    testPID.errorDotFiltered.x = 0.0f;
//    testPID.errorDotFiltered.y = 0.0f;
//    testPID.errorDotFiltered.z = 0.0f;
//    testPID.deltaT = 0.1;
//    testPID.DTermC = 2.0f;
//    testPID.saturationI = 10.0f;
//    testPID.saturationPID = 200.0f;
//    testPID.PFactor = 10000.0f;
//    testPID.IFactor = 100.0f;
//    testPID.DFactor = 1000000.0f;
//    testPID.FFrFactor = 1000.0f;
//    testPID.FFdrFactor = 10000.0f;
//    CalcPID_wo_Dkick(&testPID, &testU);
//    EXPECT_NEAR(testPID.error.x, 10, 0.1);
//    EXPECT_NEAR(testPID.error.y, -15, 0.1);
//    EXPECT_NEAR(testPID.error.z, -25, 0.1);
//    EXPECT_NEAR(testPID.errorSum.x, 1, 0.1);
//    EXPECT_NEAR(testPID.errorSum.y, -1.5, 0.1);
//    EXPECT_NEAR(testPID.errorSum.z, -2.5, 0.1);
//    EXPECT_NEAR(testPID.errorDot.x, 100.0, 0.1);
//    EXPECT_NEAR(testPID.errorDot.y, 400.0, 0.1);
//    EXPECT_NEAR(testPID.errorDot.z, -50.0, 0.1);
//    EXPECT_NEAR(testPID.errorPrev.x, 0.0, 0.1);
//    EXPECT_NEAR(testPID.errorPrev.y, 0.0, 0.1);
//    EXPECT_NEAR(testPID.errorPrev.z, 0.0, 0.1);
//    EXPECT_NEAR(testPID.errorDotFiltered.x, 100.0 / 3.0, 0.1);
//    EXPECT_NEAR(testPID.errorDotFiltered.y, 400.0 / 3.0, 0.1);
//    EXPECT_NEAR(testPID.errorDotFiltered.z, -50.0 / 3.0, 0.1);
//    EXPECT_NEAR(testPID.Pout.x, 10, 0.1);
//    EXPECT_NEAR(testPID.Pout.y, -75, 0.1);
//    EXPECT_NEAR(testPID.Pout.z, -250, 0.1);
//    EXPECT_NEAR(testPID.Iout.x, 20, 0.1);
//    EXPECT_NEAR(testPID.Iout.y, -90, 0.1);
//    EXPECT_NEAR(testPID.Iout.z, -300, 0.1);
//    EXPECT_NEAR(testPID.Dout.x, 1000, 0.1);
//    EXPECT_NEAR(testPID.Dout.y, 2000, 0.1);
//    EXPECT_NEAR(testPID.Dout.z, -3333.3f, 0.1);
//    EXPECT_FALSE(testPID.sensor.newData);
//    EXPECT_NEAR(testU.x, -200, 0.1);
//    EXPECT_NEAR(testU.y, -200, 0.1);
//    EXPECT_NEAR(testU.z, 200, 0.1);
//
//    //2nd: sum increament, delta stays as no newdata
//    CalcPID_wo_Dkick(&testPID, &testU);
//    EXPECT_NEAR(testPID.errorSum.x, 2, 0.1);
//    EXPECT_NEAR(testPID.errorSum.y, -3, 0.1);
//    EXPECT_NEAR(testPID.errorSum.z, -5, 0.1);
//    EXPECT_NEAR(testPID.errorDotFiltered.x, 100.0 / 3.0, 0.1);
//    EXPECT_NEAR(testPID.errorDotFiltered.y, 400.0 / 3.0, 0.1);
//    EXPECT_NEAR(testPID.errorDotFiltered.z, -50.0 / 3.0, 0.1);
//
//    //3rd: I and overall saturation
//    testPID.sensor.signal.x = -5.0f;
//    testPID.sensor.signal.y = 80.0f;
//    testPID.sensor.signal.z = 20.0f;
//    testPID.sensor.newData = true;
//    testPID.saturationI = 3.0f;
//    testPID.saturationPID = 200.0f;
//    CalcPID_wo_Dkick(&testPID, &testU);
//    EXPECT_NEAR(testPID.errorSum.x, testPID.saturationI, 0.1);
//    EXPECT_NEAR(testPID.errorSum.y, -testPID.saturationI, 0.1);
//    EXPECT_NEAR(testPID.errorSum.z, -testPID.saturationI, 0.1);
//    EXPECT_NEAR(testU.x, testPID.saturationPID, 0.1);
//    EXPECT_NEAR(testU.y, -testPID.saturationPID, 0.1);
//    EXPECT_NEAR(testU.z, -testPID.saturationPID, 0.1);
//}
//
//TEST(test_Controller, CalcPID_wo_Dkick_FF_Test)
//{
//    pid_st testPIDff;
//    axis testUff;
//
//    //1st
//    testPIDff.refSignal.x = 20.0f;
//    testPIDff.refSignal.y = 25.0f;
//    testPIDff.refSignal.z = -30.0f;
//    testPIDff.refSignalPrev.x = 0.0f;
//    testPIDff.refSignalPrev.y = 0.0f;
//    testPIDff.refSignalPrev.z = 0.0f;
//    testPIDff.sensor.signal.x = 10.0f;
//    testPIDff.sensor.signal.y = 40.0f;
//    testPIDff.sensor.signal.z = -5.0f;
//    testPIDff.sensor.newData = true;
//    testPIDff.P.x = 10000.0f;
//    testPIDff.P.y = 50000.0f;
//    testPIDff.P.z = 100000.0f;
//    testPIDff.I.x = 2000.0f;
//    testPIDff.I.y = 6000.0f;
//    testPIDff.I.z = 12000.0f;
//    testPIDff.D.x = 30000000.0f;
//    testPIDff.D.y = 15000000.0f;
//    testPIDff.D.z = 200000000.0f;
//    testPIDff.FFr.x = 100000.0f;
//    testPIDff.FFr.y = 100000.0f;
//    testPIDff.FFr.z = 100000.0f;
//    testPIDff.FFdr.x = 10000.0f;
//    testPIDff.FFdr.y = 10000.0f;
//    testPIDff.FFdr.z = 10000.0f;
//    testPIDff.error.x = 0.0f;
//    testPIDff.error.y = 0.0f;
//    testPIDff.error.z = 0.0f;
//    testPIDff.errorSum.x = 0.0f;
//    testPIDff.errorSum.y = 0.0f;
//    testPIDff.errorSum.z = 0.0f;
//    testPIDff.errorDot.x = 0.0f;
//    testPIDff.errorDot.y = 0.0f;
//    testPIDff.errorDot.z = 0.0f;
//    testPIDff.errorPrev.x = 0.0f;
//    testPIDff.errorPrev.y = 0.0f;
//    testPIDff.errorPrev.z = 0.0f;
//    testPIDff.errorDotFiltered.x = 0.0f;
//    testPIDff.errorDotFiltered.y = 0.0f;
//    testPIDff.errorDotFiltered.z = 0.0f;
//    testPIDff.deltaT = 0.1;
//    testPIDff.DTermC = 2.0f;
//    testPIDff.FFDTermC = 10.0f;
//    testPIDff.saturationI = 10.0f;
//    testPIDff.saturationPID = 200.0f;
//    testPIDff.PFactor = 10000.0f;
//    testPIDff.IFactor = 100.0f;
//    testPIDff.DFactor = 1000000.0f;
//    testPIDff.FFrFactor = 1000.0f;
//    testPIDff.FFdrFactor = 10000.0f;
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    EXPECT_NEAR(testPIDff.error.x, 10, 0.1);
//    EXPECT_NEAR(testPIDff.error.y, -15, 0.1);
//    EXPECT_NEAR(testPIDff.error.z, -25, 0.1);
//    EXPECT_NEAR(testPIDff.errorSum.x, 1, 0.1);
//    EXPECT_NEAR(testPIDff.errorSum.y, -1.5, 0.1);
//    EXPECT_NEAR(testPIDff.errorSum.z, -2.5, 0.1);
//    EXPECT_NEAR(testPIDff.errorDot.x, 100.0, 0.1);
//    EXPECT_NEAR(testPIDff.errorDot.y, 400.0, 0.1);
//    EXPECT_NEAR(testPIDff.errorDot.z, -50.0, 0.1);
//    EXPECT_NEAR(testPIDff.errorPrev.x, 0.0, 0.1);
//    EXPECT_NEAR(testPIDff.errorPrev.y, 0.0, 0.1);
//    EXPECT_NEAR(testPIDff.errorPrev.z, 0.0, 0.1);
//    EXPECT_NEAR(testPIDff.errorDotFiltered.x, 100.0 / 3.0, 0.1);
//    EXPECT_NEAR(testPIDff.errorDotFiltered.y, 400.0 / 3.0, 0.1);
//    EXPECT_NEAR(testPIDff.errorDotFiltered.z, -50.0 / 3.0, 0.1);
//    EXPECT_NEAR(testPIDff.refSignalDot.x, 200.0, 0.1);
//    EXPECT_NEAR(testPIDff.refSignalDot.y, 250.0, 0.1);
//    EXPECT_NEAR(testPIDff.refSignalDot.z, -300.0, 0.1);
//    EXPECT_NEAR(testPIDff.refSignalDotFiltered.x, 18.18, 0.1);
//    EXPECT_NEAR(testPIDff.refSignalDotFiltered.y, 22.72, 0.1);
//    EXPECT_NEAR(testPIDff.refSignalDotFiltered.z, -27.27, 0.1);
//    EXPECT_NEAR(testPIDff.Pout.x, 10, 0.1);
//    EXPECT_NEAR(testPIDff.Pout.y, -75, 0.1);
//    EXPECT_NEAR(testPIDff.Pout.z, -250, 0.1);
//    EXPECT_NEAR(testPIDff.Iout.x, 20, 0.1);
//    EXPECT_NEAR(testPIDff.Iout.y, -90, 0.1);
//    EXPECT_NEAR(testPIDff.Iout.z, -300, 0.1);
//    EXPECT_NEAR(testPIDff.Dout.x, 1000, 0.1);
//    EXPECT_NEAR(testPIDff.Dout.y, 2000, 0.1);
//    EXPECT_NEAR(testPIDff.Dout.z, -3333.3f, 0.1);
//    EXPECT_NEAR(testPIDff.FFout.x, 2018.18, 0.1);
//    EXPECT_NEAR(testPIDff.FFout.y, 2522.72, 0.1);
//    EXPECT_NEAR(testPIDff.FFout.z, -3027.27, 0.1);
//    EXPECT_FALSE(testPIDff.sensor.newData);
//    EXPECT_NEAR(testUff.x, 200, 0.1);
//    EXPECT_NEAR(testUff.y, 200, 0.1);
//    EXPECT_NEAR(testUff.z, -200, 0.1);
//
//    //2nd: sum increament, delta stays as no newdata
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    EXPECT_NEAR(testPIDff.errorSum.x, 2, 0.1);
//    EXPECT_NEAR(testPIDff.errorSum.y, -3, 0.1);
//    EXPECT_NEAR(testPIDff.errorSum.z, -5, 0.1);
//    EXPECT_NEAR(testPIDff.errorDotFiltered.x, 100.0 / 3.0, 0.1);
//    EXPECT_NEAR(testPIDff.errorDotFiltered.y, 400.0 / 3.0, 0.1);
//    EXPECT_NEAR(testPIDff.errorDotFiltered.z, -50.0 / 3.0, 0.1);
//
//    //3rd: I and overall saturation
//    testPIDff.sensor.signal.x = -5.0f;
//    testPIDff.sensor.signal.y = 80.0f;
//    testPIDff.sensor.signal.z = 20.0f;
//    testPIDff.sensor.newData = true;
//    testPIDff.saturationI = 3.0f;
//    testPIDff.saturationPID = 200.0f;
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    EXPECT_NEAR(testPIDff.errorSum.x, testPIDff.saturationI, 0.1);
//    EXPECT_NEAR(testPIDff.errorSum.y, -testPIDff.saturationI, 0.1);
//    EXPECT_NEAR(testPIDff.errorSum.z, -testPIDff.saturationI, 0.1);
//    EXPECT_NEAR(testUff.x, testPIDff.saturationPID, 0.1);
//    EXPECT_NEAR(testUff.y, -testPIDff.saturationPID, 0.1);
//    EXPECT_NEAR(testUff.z, -testPIDff.saturationPID, 0.1);
//
//    //4th pid comparisant
//    pid_st testPIDff2;
//    axis testUff2;
//    testPIDff.refSignal.x = 20.0f;
//    testPIDff.refSignal.y = 20.0f;
//    testPIDff.refSignal.z = 20.0f;
//    testPIDff.refSignalPrev.x = 0.0f;
//    testPIDff.refSignalPrev.y = 0.0f;
//    testPIDff.refSignalPrev.z = 0.0f;
//    testPIDff.sensor.signal.x = 10.0f;
//    testPIDff.sensor.signal.y = 10.0f;
//    testPIDff.sensor.signal.z = 10.0f;
//    testPIDff.sensor.newData = true;
//    testPIDff.P.x = 33.0f;
//    testPIDff.P.y = 43.0f;
//    testPIDff.P.z = 350.0f;
//    testPIDff.I.x = 30.0f;
//    testPIDff.I.y = 30.0f;
//    testPIDff.I.z = 10.0f;
//    testPIDff.D.x = 25.0f;
//    testPIDff.D.y = 25.0f;
//    testPIDff.D.z = 150.0f;
//    testPIDff.FFr.x = 0;
//    testPIDff.FFr.y = 0;
//    testPIDff.FFr.z = 0;
//    testPIDff.FFdr.x = 0;
//    testPIDff.FFdr.y = 0;
//    testPIDff.FFdr.z = 0;
//    testPIDff.deltaT = 0.1;
//    testPIDff.DTermC = 10.0f;
//    testPIDff.FFDTermC = 10.0f;
//    testPIDff.saturationI = 7.0f;
//    testPIDff.saturationPID = 75.0f;
//    testPIDff.PFactor = 1000.0f;
//    testPIDff.IFactor = 100.0f;
//    testPIDff.DFactor = 10000.0f;
//    testPIDff.FFrFactor = 1000.0f;
//    testPIDff.FFdrFactor = 10000.0f;
//    testPIDff.iRelaxRefThreshhold = 300.0f;
//    testPIDff.iRelaxErrThreshhold = 100.0f;
//    testPIDff.Dmax.x = 25.0f;
//    testPIDff.Dmax.y = 25.0f;
//    testPIDff.Dmax.z = 150.0f;
//    testPIDff.dMaxRefThreshhold = 300.0f;
//    testPIDff.dMaxErrThreshhold = 100.0f;
//    testPIDff.errorSum.x = 0;
//    testPIDff.errorSum.y = 0;
//    testPIDff.errorSum.z = 0;
//    testPIDff.sensorPrev.signal.x = 0;
//    testPIDff.sensorPrev.signal.y = 0;
//    testPIDff.sensorPrev.signal.z = 0;
//    testPIDff.errorDotFiltered.x = 0;
//    testPIDff.errorDotFiltered.y = 0;
//    testPIDff.errorDotFiltered.z = 0;
//    testPIDff2 = testPIDff;
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    CalcPID_wo_Dkick_FF_IRelax_Dmax(&testPIDff2, &testUff2, 1000);
//    EXPECT_NEAR(testUff.x, testUff2.x, 0,00000001);
//    EXPECT_NEAR(testUff.y, testUff2.y, 0,00000001);
//    EXPECT_NEAR(testUff.z, testUff2.z, 0,00000001);
//}

TEST(test_Controller, ScalePIDinput_int_Test)
{
    pid_st pid;

    //
    pid.refSig_i.x = 1;
    pid.refSig_i.y = 20;
    pid.refSig_i.z = 150;
    pid.sensor.signalPT1.x = -3;
    pid.sensor.signalPT1.y = 14;
    pid.sensor.signalPT1.z = 150;
    ScalePIDinput_int(&pid, 2);
    EXPECT_EQ(pid.refSig_i.x, 4);
    EXPECT_EQ(pid.refSig_i.y, 80);
    EXPECT_EQ(pid.refSig_i.z, 600);
    EXPECT_EQ(pid.sensor.signalPT1.x, -12);
    EXPECT_EQ(pid.sensor.signalPT1.y, 56);
    EXPECT_EQ(pid.sensor.signalPT1.z, 600);

    //
    pid.refSig_i.x = 10;
    pid.refSig_i.y = 25;
    pid.refSig_i.z = 321;
    pid.sensor.signalPT1.x = -123;
    pid.sensor.signalPT1.y = 50;
    pid.sensor.signalPT1.z = 90;
    ScalePIDinput_int(&pid, 10);
    EXPECT_EQ(pid.refSig_i.x, 10240);
    EXPECT_EQ(pid.refSig_i.y, 25600);
    EXPECT_EQ(pid.refSig_i.z, 328704);
    EXPECT_EQ(pid.sensor.signalPT1.x, -125952);
    EXPECT_EQ(pid.sensor.signalPT1.y, 51200);
    EXPECT_EQ(pid.sensor.signalPT1.z, 92160);
}

TEST(test_Controller, ScalePIDoutput_int_Test)
{
    axis_i32 testU;

    //2 shift
    testU.x = 32;
    testU.y = 64;
    testU.z = 256;
    ScalePIDoutput_int(&testU, 2);
    EXPECT_EQ(testU.x, 8);
    EXPECT_EQ(testU.y, 16);
    EXPECT_EQ(testU.z, 64);

    //10 shift
    testU.x = 1024;
    testU.y = 2048;
    testU.z = 8196;
    ScalePIDoutput_int(&testU, 10);
    EXPECT_EQ(testU.x, 1);
    EXPECT_EQ(testU.y, 2);
    EXPECT_EQ(testU.z, 8);
}

//TEST(test_Controller, PID_compare)
//{
//    pid_st testPIDff;
//    axis testUff;
//    axis_i32 testUff_i32;
//
//    testPIDff.P.x = 45.0f;
//    testPIDff.I.x = 20.0f;
//    testPIDff.D.x = 25.0f;
//    testPIDff.PFactor = 1000.0f;
//    testPIDff.IFactor = 100.0f;
//    testPIDff.DFactor = 10000.0f;
//    testPIDff.P_int.x = 46.0f;
//    testPIDff.I_int.x = 26.0f;
//    testPIDff.D_int.x = 26.0f;
//    testPIDff.errorSum.x = 0.0f;
//    testPIDff.errorSum_int.x = 0;
//    testPIDff.saturationI = 35.0f;
//    testPIDff.satI_int = 0x7FFFFFFF;
//    testPIDff.sensor.signal.x = 0.0f;
//    testPIDff.sensor.signalPT1_int.x = 0;
//    testPIDff.errorDotFiltered.x = 0.0f;
//    testPIDff.errorDotPT1_int.x = 0;
//    testPIDff.DTermC = 10;
//    testPIDff.saturationPID = 200.0f;
//    testPIDff.satPID_int = 10000000;
//
//    //0-0
//    testPIDff.refSignal.x = 0.0f;
//    testPIDff.refSignal_int.x = 0;
//    testPIDff.sensor.signal.x = 0.0f;
//    testPIDff.sensor.signalPT1_int.x = 0;
//    testPIDff.deltaT = 0.002;
//    testPIDff.deltaTicks = 0.002 * 10500000;
//    testPIDff.inverseDt = (10500000 / testPIDff.deltaTicks);
//    testPIDff.sensor.newData = true;
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    CalcProportional_int(&testPIDff);
//    CalcIntegral_int(&testPIDff);
//    testPIDff.sensor.newData = true;
//    CalcDerivative_int(&testPIDff);
//    CalcPIDoutput_int(&testPIDff, &testUff_i32);
//    ScalePIDoutput_int(&testUff_i32, 11);
//    EXPECT_NEAR(testPIDff.Pout.x, 0, 0.1);
//    EXPECT_EQ(testPIDff.Pout_int.x, 0);
//    EXPECT_NEAR(testPIDff.Iout.x, 0.0, 0.1);
//    EXPECT_EQ(testPIDff.Iout_int.x, 0);
//    EXPECT_NEAR(testPIDff.Dout.x, 0.0, 0.1);
//    EXPECT_EQ(testPIDff.Dout_int.x, 0);
//    EXPECT_NEAR(testPIDff.u.x, 0.0, 0.1);
//    EXPECT_EQ(testPIDff.u_int.x, 0);
//    EXPECT_EQ(testUff_i32.x, 0);
//
//    //50-0
//    testPIDff.refSignal.x = 50.0f;
//    testPIDff.refSignal_int.x = (50 * 32768 / 2000);
//    testPIDff.sensor.signal.x = 0.0f;
//    testPIDff.sensor.signalPT1_int.x = 0;
//    testPIDff.deltaT = 0.002;
//    testPIDff.deltaTicks = 0.002 * 10500000;
//    testPIDff.inverseDt = (10500000 / testPIDff.deltaTicks);
//    testPIDff.sensor.newData = true;
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    CalcProportional_int(&testPIDff);
//    CalcIntegral_int(&testPIDff);
//    testPIDff.sensor.newData = true;
//    CalcDerivative_int(&testPIDff);
//    CalcPIDoutput_int(&testPIDff, &testUff_i32);
//    ScalePIDoutput_int(&testUff_i32, 9);
//    EXPECT_NEAR(testPIDff.Pout.x, 2.25, 0.1);
//    EXPECT_EQ(testPIDff.Pout_int.x, 37674);
//    EXPECT_NEAR(testPIDff.Iout.x, 0.02, 0.1);
//    EXPECT_EQ(testPIDff.Iout_int.x, 3411);
//    EXPECT_NEAR(testPIDff.Dout.x, 0.0, 0.1);
//    EXPECT_EQ(testPIDff.Dout_int.x, 0);
//    EXPECT_NEAR(testPIDff.u.x, 2.27, 0.1);
//    EXPECT_EQ(testPIDff.u_int.x, 41085);
//    EXPECT_EQ(testUff_i32.x, 80);
//
//    //200-50
//    testPIDff.refSignal.x = 200.0f;
//    testPIDff.refSignal_int.x = (200 * 32768 / 2000);
//    testPIDff.sensor.signal.x = 50.0f;
//    testPIDff.sensor.signalPT1_int.x = (50 * 32768 / 2000);
//    testPIDff.deltaT = 0.003;
//    testPIDff.deltaTicks = 0.003 * 10500000;
//    testPIDff.inverseDt = (10500000 / testPIDff.deltaTicks);
//    testPIDff.sensor.newData = true;
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    CalcProportional_int(&testPIDff);
//    CalcIntegral_int(&testPIDff);
//    testPIDff.sensor.newData = true;
//    CalcDerivative_int(&testPIDff);
//    CalcPIDoutput_int(&testPIDff, &testUff_i32);
//    ScalePIDoutput_int(&testUff_i32, 9);
//    EXPECT_NEAR(testPIDff.Pout.x, 6.75, 0.1);
//    EXPECT_EQ(testPIDff.Pout_int.x, 113022);
//    EXPECT_NEAR(testPIDff.Iout.x, 0.11, 0.1);
//    EXPECT_EQ(testPIDff.Iout_int.x, 18764);
//    EXPECT_NEAR(testPIDff.Dout.x, 3.7878, 0.1);
//    EXPECT_EQ(testPIDff.Dout_int.x, 55397);
//    EXPECT_NEAR(testPIDff.u.x, 3.0721, 0.1);
//    EXPECT_EQ(testPIDff.u_int.x, 76389);
//    EXPECT_EQ(testUff_i32.x, 149);
//
//    //450-140
//    testPIDff.refSignal.x = 450.0f;
//    testPIDff.refSignal_int.x = (450 * 32768 / 2000);
//    testPIDff.sensor.signal.x = 140.0f;
//    testPIDff.sensor.signalPT1_int.x = (140 * 32768 / 2000);
//    testPIDff.deltaT = 0.002;
//    testPIDff.deltaTicks = 0.002 * 10500000;
//    testPIDff.inverseDt = (10500000 / testPIDff.deltaTicks);
//    testPIDff.sensor.newData = true;
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    CalcProportional_int(&testPIDff);
//    CalcIntegral_int(&testPIDff);
//    testPIDff.sensor.newData = true;
//    CalcDerivative_int(&testPIDff);
//    CalcPIDoutput_int(&testPIDff, &testUff_i32);
//    ScalePIDoutput_int(&testUff_i32, 9);
//    EXPECT_NEAR(testPIDff.Pout.x, 13.95, 0.1);
//    EXPECT_EQ(testPIDff.Pout_int.x, 233634);
//    EXPECT_NEAR(testPIDff.Iout.x, 0.234, 0.1);
//    EXPECT_EQ(testPIDff.Iout_int.x, 39921);
//    EXPECT_NEAR(testPIDff.Dout.x, 13.6707, 0.1);
//    EXPECT_EQ(testPIDff.Dout_int.x, 201638);
//    EXPECT_NEAR(testPIDff.u.x, 0.5132, 0.1);
//    EXPECT_EQ(testPIDff.u_int.x, 71917);
//    EXPECT_EQ(testUff_i32.x, 140);
//
//    //620-260
//    testPIDff.refSignal.x = 620.0f;
//    testPIDff.refSignal_int.x = (620 * 32768 / 2000);
//    testPIDff.sensor.signal.x = 260.0f;
//    testPIDff.sensor.signalPT1_int.x = (260 * 32768 / 2000);
//    testPIDff.deltaT = 0.003;
//    testPIDff.deltaTicks = 0.003 * 10500000;
//    testPIDff.inverseDt = (10500000 / testPIDff.deltaTicks);
//    testPIDff.sensor.newData = true;
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    CalcProportional_int(&testPIDff);
//    CalcIntegral_int(&testPIDff);
//    testPIDff.sensor.newData = true;
//    CalcDerivative_int(&testPIDff);
//    CalcPIDoutput_int(&testPIDff, &testUff_i32);
//    ScalePIDoutput_int(&testUff_i32, 9);
//    EXPECT_NEAR(testPIDff.Pout.x, 16.2, 0.1);
//    EXPECT_EQ(testPIDff.Pout_int.x, 271354);
//    EXPECT_NEAR(testPIDff.Iout.x, 0.45, 0.1);
//    EXPECT_EQ(testPIDff.Iout_int.x, 76781);
//    EXPECT_NEAR(testPIDff.Dout.x, 21.5189, 0.1);
//    EXPECT_EQ(testPIDff.Dout_int.x, 322017);
//    EXPECT_NEAR(testPIDff.u.x, -4.8689, 0.1);
//    EXPECT_EQ(testPIDff.u_int.x, 26118);
//    EXPECT_EQ(testUff_i32.x, 51);
//
//    //800-420
//    testPIDff.refSignal.x = 800.0f;
//    testPIDff.refSignal_int.x = (800 * 32768 / 2000);
//    testPIDff.sensor.signal.x = 420.0f;
//    testPIDff.sensor.signalPT1_int.x = (420 * 32768 / 2000);
//    testPIDff.deltaT = 0.003;
//    testPIDff.deltaTicks = 0.003 * 10500000;
//    testPIDff.inverseDt = (10500000 / testPIDff.deltaTicks);
//    testPIDff.sensor.newData = true;
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    testPIDff.sensor.newData = true;
//    CalcPID_int(&testPIDff, &testUff_i32);
//    //ScalePIDinput_int(&testPIDff, 10);
//    //CalcProportional_int(&testPIDff);
//    //CalcIntegral_int(&testPIDff);
//    //CalcDerivative_int(&testPIDff);
//    //CalcPIDoutput_int(&testPIDff, &testUff_i32);
//    //ScalePIDoutput_int(&testUff_i32, 9);
//    EXPECT_NEAR(testPIDff.Pout.x, 17.1, 0.1);
//    EXPECT_EQ(testPIDff.Pout_int.x, 286396);
//    EXPECT_NEAR(testPIDff.Iout.x, 0.678, 0.1);
//    EXPECT_EQ(testPIDff.Iout_int.x, 115684);
//    EXPECT_NEAR(testPIDff.Dout.x, 31.6838, 0.1);
//    EXPECT_EQ(testPIDff.Dout_int.x, 479245);
//    EXPECT_NEAR(testPIDff.u.x, -13.9059, 0.1);
//    EXPECT_EQ(testPIDff.u_int.x, -77165);
//    EXPECT_EQ(testUff_i32.x, -151);
//}
//
//TEST(test_Controller, PID_compare_p)
//{
//    pid_st testPIDff;
//    axis_i32 testUff_i32;
//
//    testPIDff.P_i.x = 50.0f;
//    testPIDff.errorDotPT1_i.x = 0;
//    testPIDff.satPID_i = 10000000;
//    testPIDff.refSig_i.x = (1000 * 32768 / 2000);
//    testPIDff.sensor.signalPT1.x = (0 * 32768 / 2000);
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcProportional_int(&testPIDff);
//    CalcPIDoutput_int(&testPIDff, &testUff_i32);
//    ScalePIDoutput_int(&testUff_i32, 11);
//    EXPECT_NEAR(testUff_i32.x, 30*32768/2000, 100);
//}
//
//TEST(test_Controller, PID_compare_pd)
//{
//    pid_st testPIDff;
//    axis_i32 testUff_i32;
//
//    testPIDff.P_i.x = 39.0f;
//    testPIDff.I_i.x = 0.0f;
//    testPIDff.D_i.x = 26.0f;
//    testPIDff.errorSum_i.x = 0;
//    testPIDff.satI_i = 0x7FFFFFFF;
//    testPIDff.errorDotPT1_i.x = 0;
//    testPIDff.satPID_i = 10000000;
//
//    //
//    float measuredPout = 5.736 / 2000 * 32768 * 1024;
//    float measuredDout = 0.065 / 2000 * 32768 * 1024;
//    float measuredDoutPrev = 0.23 / 2000 * 32768 * 1024;
//    float measPD = measuredPout - measuredDout;
//    testPIDff.refSig_i.x = (158.2 * 32768 / 2000);
//    testPIDff.sensor.signalPT1.x = (7.312 * 32768 / 2000);
//    testPIDff.signalPT1Prev_i.x = int32_t(8.186 * 32768 / 2000) << 10;
//    testPIDff.errorDotPT1_i.x = int32_t(measuredDoutPrev/ testPIDff.D_i.x) << 9;
//    testPIDff.sensor.newData = true;
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcProportional_int(&testPIDff);
//    CalcDerivative_int(&testPIDff);
//    CalcPIDoutput_int(&testPIDff, &testUff_i32);
//    ScalePIDoutput_int(&testUff_i32, 9);
//    auto pRatio = (testPIDff.Pout_i.x - measuredPout) / measuredPout*100;
//    auto dRatio = (testPIDff.Dout_i.x - measuredDout) / measuredDout*100;
//    auto pdRatio = (testPIDff.u_i.x - measPD)/ measPD*100;
//    EXPECT_LT(abs(pRatio), 2);
//    EXPECT_LT(abs(dRatio), 2);
//    EXPECT_LT(abs(pdRatio), 2);
//
//    //
//    measuredPout = -3.197 / 2000 * 32768 * 1024;
//    measuredDout = 4.779 / 2000 * 32768 * 1024;
//    measuredDoutPrev = 4.461 / 2000 * 32768 * 1024;
//    measPD = measuredPout - measuredDout;
//    testPIDff.refSignal_i.x = (48.78 * 32768 / 2000);
//    testPIDff.sensor.signalPT1_i.x = (132.9 * 32768 / 2000);
//    testPIDff.signalPT1Prev_int.x = int32_t(117.6 * 32768 / 2000) << 10;
//    testPIDff.errorDotPT1_int.x = int32_t(measuredDoutPrev) << 8;
//    testPIDff.deltaTicks = 0.005 * 10500000;
//    testPIDff.inverseDt = (10500000 / testPIDff.deltaTicks);
//    testPIDff.sensor.newData = true;
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcProportional_int(&testPIDff);
//    CalcDerivative_int(&testPIDff);
//    CalcPIDoutput_int(&testPIDff, &testUff_i32);
//    ScalePIDoutput_int(&testUff_i32, 9);
//    pRatio = (testPIDff.Pout_int.x - measuredPout) / measuredPout*100;
//    dRatio = (testPIDff.Dout_int.x - measuredDout) / measuredDout*100;
//    pdRatio = (testPIDff.u_int.x - measPD) / measPD*100;
//    EXPECT_LT(abs(pRatio), 2);
//    EXPECT_LT(abs(dRatio), 2);
//    EXPECT_LT(abs(pdRatio), 2);
//
//    //
//    measuredPout = 30.85 / 2000 * 32768 * 1024;
//    measuredDout = 18.66 / 2000 * 32768 * 1024;
//    measuredDoutPrev = 15.57 / 2000 * 32768 * 1024;
//    measPD = measuredPout - measuredDout;
//    testPIDff.refSignal_int.x = (918.5 * 32768 / 2000);
//    testPIDff.sensor.signalPT1_int.x = (106.7 * 32768 / 2000);
//    testPIDff.signalPT1Prev_int.x = int32_t(35.33 * 32768 / 2000) << 10;
//    testPIDff.errorDotPT1_int.x = int32_t(measuredDoutPrev) << 8;
//    testPIDff.deltaTicks = 0.005 * 10500000;
//    testPIDff.inverseDt = (10500000 / testPIDff.deltaTicks);
//    testPIDff.sensor.newData = true;
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcProportional_int(&testPIDff);
//    CalcDerivative_int(&testPIDff);
//    CalcPIDoutput_int(&testPIDff, &testUff_i32);
//    ScalePIDoutput_int(&testUff_i32, 9);
//    pRatio = (testPIDff.Pout_int.x - measuredPout) / measuredPout * 100;
//    dRatio = (testPIDff.Dout_int.x - measuredDout) / measuredDout * 100;
//    pdRatio = (testPIDff.u_int.x - measPD) / measPD * 100;
//    EXPECT_LT(abs(pRatio), 2);
//    EXPECT_LT(abs(dRatio), 2);
//    EXPECT_LT(abs(pdRatio), 2);
//}

TEST(test_Controller, CalcIntegral_int_overflow)
{
    pid_st testPid;

    testPid.I_i.x = 26.0f;
    testPid.satI_i = 0x7FFFFFFF;
    testPid.refSig_i.x = (1000 * 32768 / 2000);
    testPid.sensor.signalPT1.x = 0;
    ScalePIDinput_int(&testPid, 10);
    CalcProportional_int(&testPid);
    CalcIntegral_int(&testPid);
    CalcIntegral_int(&testPid);
    CalcIntegral_int(&testPid);
    CalcIntegral_int(&testPid);
    CalcIntegral_int(&testPid);
    EXPECT_LE(testPid.errorSum_i.x, 0x7FFFFFFF);
    EXPECT_GT(testPid.errorSum_i.x, 0);
}

//TEST(test_Controller, PID_compare_I)
//{
//    pid_st testPIDff;
//    axis testUff;
//
//    testPIDff.P.x = 45.0f;
//    testPIDff.I.x = 20.0f;
//    testPIDff.PFactor = 1000.0f;
//    testPIDff.IFactor = 100.0f;
//    testPIDff.P_int.x = 46.0f;
//    testPIDff.I_int.x = 26.0f;
//    testPIDff.errorSum.x = 0.0f;
//    testPIDff.errorSum_int.x = 0;
//    testPIDff.saturationI = 35.0f;
//    testPIDff.satI_int = 0x7FFFFFFF;
//
//    //0-0
//    testPIDff.refSignal.x = 0.0f;
//    testPIDff.refSignal_int.x = (0 * 32768 / 2000);
//    testPIDff.sensor.signal.x = 0.0f;
//    testPIDff.sensor.signalPT1_int.x = (0 * 32768 / 2000);
//    testPIDff.deltaT = 0.002;
//    testPIDff.deltaTicks = 0.002 * 10500000;
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcProportional_int(&testPIDff);
//    CalcIntegral_int(&testPIDff);
//    EXPECT_NEAR(testPIDff.Iout.x, 0.0, 0.1);
//    EXPECT_EQ(testPIDff.Iout_int.x, 0);
//
//    //50-0
//    testPIDff.refSignal.x = 50.0f;
//    testPIDff.refSignal_int.x = (50 * 32768 / 2000);
//    testPIDff.sensor.signal.x = 0.0f;
//    testPIDff.sensor.signalPT1_int.x = (0 * 32768 / 2000);
//    testPIDff.deltaT = 0.002;
//    testPIDff.deltaTicks = 0.002 * 10500000;
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    CalcProportional_int(&testPIDff);
//    CalcIntegral_int(&testPIDff);
//    EXPECT_NEAR(testPIDff.Iout.x, 0.02, 0.1);
//    EXPECT_EQ(testPIDff.Iout_int.x, 3411);
//
//    //200-50
//    testPIDff.refSignal.x = 200.0f;
//    testPIDff.refSignal_int.x = (200 * 32768 / 2000);
//    testPIDff.sensor.signal.x = 50.0f;
//    testPIDff.sensor.signalPT1_int.x = (50 * 32768 / 2000);
//    testPIDff.deltaT = 0.003;
//    testPIDff.deltaTicks = 0.003 * 10500000;
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    CalcProportional_int(&testPIDff);
//    CalcIntegral_int(&testPIDff);
//    CalcIntegral_int(&testPIDff);
//
//    //450-140
//    testPIDff.refSignal.x = 450.0f;
//    testPIDff.refSignal_int.x = (450 * 32768 / 2000);
//    testPIDff.sensor.signal.x = 140.0f;
//    testPIDff.sensor.signalPT1_int.x = (140 * 32768 / 2000);
//    testPIDff.deltaT = 0.002;
//    testPIDff.deltaTicks = 0.002 * 10500000;
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    CalcProportional_int(&testPIDff);
//    CalcIntegral_int(&testPIDff);
//
//    //620-260
//    testPIDff.refSignal.x = 620.0f;
//    testPIDff.refSignal_int.x = (620 * 32768 / 2000);
//    testPIDff.sensor.signal.x = 260.0f;
//    testPIDff.sensor.signalPT1_int.x = (260 * 32768 / 2000);
//    testPIDff.deltaT = 0.003;
//    testPIDff.deltaTicks = 0.003 * 10500000;
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    CalcProportional_int(&testPIDff);
//    CalcIntegral_int(&testPIDff);
//
//    //800-420
//    testPIDff.refSignal.x = 800.0f;
//    testPIDff.refSignal_int.x = (800 * 32768 / 2000);
//    testPIDff.sensor.signal.x = 420.0f;
//    testPIDff.sensor.signalPT1_int.x = (420 * 32768 / 2000);
//    testPIDff.deltaT = 0.003;
//    testPIDff.deltaTicks = 0.003 * 10500000;
//    ScalePIDinput_int(&testPIDff, 10);
//    CalcPID_wo_Dkick_FF(&testPIDff, &testUff);
//    CalcProportional_int(&testPIDff);
//    CalcIntegral_int(&testPIDff);
//}

TEST(test_Controller, RunController_Test)
{
    controllerIn_st testIn;
    controllerOut_st testOut;

    //1st: disamred
    testIn.rcSignals.armStateSwitch = 1000;
    testIn.rcSignals.flightModeSwitch = 1500;
    testIn.rcSignals.throttle = 1000;
    testOut.U_i.x = 1;
    testOut.U_i.y = 1;
    testOut.U_i.z = 1;
    RunController(&testIn, &testOut);
    EXPECT_EQ(testOut.U_i.x, 0);
    EXPECT_EQ(testOut.U_i.y, 0);
    EXPECT_EQ(testOut.U_i.z, 0);
    EXPECT_EQ(testOut.armState, DISARMED);

    //2nd: armed but low throttle
    testIn.rcSignals.armStateSwitch = 2000;
    testIn.rcSignals.flightModeSwitch = 1500;
    testIn.rcSignals.throttle = 1000;
    testOut.U_i.x = 1;
    testOut.U_i.y = 1;
    testOut.U_i.z = 1;
    RunController(&testIn, &testOut);
    EXPECT_EQ(testOut.U_i.x, 0);
    EXPECT_EQ(testOut.U_i.y, 0);
    EXPECT_EQ(testOut.U_i.z, 0);
    EXPECT_EQ(testOut.armState, ARMED);

    ////3rd: armed in angle
    //testIn.rcSignals.armStateSwitch = 2000;
    //testIn.rcSignals.flightModeSwitch = 1500;
    //testIn.rcSignals.throttle = 1100;
    //testOut.U_i.x = 1;
    //testOut.U_i.y = 1;
    //testOut.U_i.z = 1;
    //RunController(&testIn, &testOut);
    //EXPECT_EQ(testOut.U_i.x, 0);
    //EXPECT_EQ(testOut.U_i.y, 0);
    //EXPECT_EQ(testOut.U_i.z, 0);
    //EXPECT_EQ(testOut.armState, ARMED);

    //4th: armed in rate
    testIn.rcSignals.armStateSwitch = 2000;
    testIn.rcSignals.flightModeSwitch = 1000;
    testIn.rcSignals.throttle = 1100;
    testIn.gyro.signalPT1.x = 1000;
    testIn.gyro.signalPT1.y = 1000;
    testIn.gyro.signalPT1.z = 1000;
    testIn.gyro.newData = true;
    pidRate.P_i.x = 10;
    pidRate.P_i.y = 10;
    pidRate.P_i.z = 10;
    pidRate.satPID_i = 1000000;
    testOut.U_i.x = 0;
    testOut.U_i.y = 0;
    testOut.U_i.z = 0;
    RunController(&testIn, &testOut);
    EXPECT_NE(testOut.U_i.x, 0);
    EXPECT_NE(testOut.U_i.y, 0);
    EXPECT_NE(testOut.U_i.z, 0);
    EXPECT_EQ(testOut.armState, ARMED);

    ////5th: armed in angle cascade
    //testIn.rcSignals.armStateSwitch = 2000;
    //testIn.rcSignals.flightModeSwitch = 2000;
    //testIn.rcSignals.throttle = 1100;
    //testIn.acc.signalPT1.x = 1.0f;
    //testIn.acc.signalPT1.y = 1.0f;
    //testIn.acc.signalPT1.z = 1.0f;
    //testIn.acc.newData = false;
    //testIn.gyro.signalPT1.x = 10.0f;
    //testIn.gyro.signalPT1.y = 10.0f;
    //testIn.gyro.signalPT1.z = 10.0f;
    //testIn.gyro.newData = true;
    //pidCascade.P_i.x = 10.0f;
    //pidCascade.P_i.y = 10.0f;
    //pidCascade.P_i.z = 10.0f;
    //pidRate.P_i.x = 10.0f;
    //pidRate.P_i.y = 10.0f;
    //pidRate.P_i.z = 10.0f;
    //pidRate.satPID_i = 100.0f;
    //testOut.U_i.x = 1.0f;
    //testOut.U_i.y = 1.0f;
    //testOut.U_i.z = 1.0f;
    //RunController(&testIn, &testOut);
    //EXPECT_NE(testOut.U_i.x, 0);
    //EXPECT_NE(testOut.U_i.y, 0);
    //EXPECT_NE(testOut.U_i.z, 0);
    //EXPECT_EQ(testOut.armState, ARMED);

    //wobble
    //testIn.rcSignals.armStateSwitch = 2000;
    //testIn.rcSignals.flightModeSwitch = 1000;
    //testIn.rcSignals.throttle = 1100;
    //testIn.rcSignals.Switch2Way = 2000;
    //testIn.rcSignals.Poti1 = 1250;
    //testIn.rcSignals.Poti2 = 1600;
    //testIn.droneTimes.sysTime = 2.0;
    //RunController(&testIn, &testOut);
    //testIn.droneTimes.sysTime = 3.0;
    //RunController(&testIn, &testOut);
    //testIn.droneTimes.sysTime = 4.0;
    //RunController(&testIn, &testOut);
    //EXPECT_EQ(true, false);
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
    EXPECT_NEAR(testKalmanAngle.angle, 0.0000005f, 0.00001f);
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

//TEST(test_Controller, ComplementryFilterAngle_Test)
//{
//    float CFAngle{ 0 };
//    float CFAngleW{ 0 };
//    axis acc;
//
//    //zero inputs
//    acc.x = 0;
//    acc.y = 0;
//    acc.z = 0;
//    ComplementryFilterAngle(&CFAngle, 0, 0, 0, 0.5);
//    //ComplementryFilterAngleWeighted(&CFAngleW, 0, 0, 0, 0.5, &acc);
//    EXPECT_NEAR(CFAngle, 0.0f, 0.01f);
//    //EXPECT_NEAR(CFAngleW, 0.0f, 0.01f);
//
//    //only accangle
//    CFAngle = 0;
//    CFAngleW = 0;
//    acc.x = 9.81;
//    acc.y = 0;
//    acc.z = 0;
//    ComplementryFilterAngle(&CFAngle, 5, 0, 0, 0.5);
//    //ComplementryFilterAngleWeighted(&CFAngleW, 5, 0, 0, 0.5, &acc);
//    EXPECT_NEAR(CFAngle, 2.5f, 0.01f);
//    //EXPECT_NEAR(CFAngleW, 2.5f, 0.01f);
//
//    //only gyro
//    CFAngle = 0;
//    CFAngleW = 0;
//    acc.x = 9.81;
//    acc.y = 0;
//    acc.z = 0;
//    ComplementryFilterAngle(&CFAngle, 0, 1000, 0.001, 0.5);
//    //ComplementryFilterAngleWeighted(&CFAngleW, 0, 1000, 0.001, 0.5, &acc);
//    EXPECT_NEAR(CFAngle, 0.5f, 0.01f);
//    //EXPECT_NEAR(CFAngleW, 0.5f, 0.01f);
//
//    //weighting
//    CFAngleW = 0;
//    acc.x = 9.81;
//    acc.y = 9.81;
//    acc.z = 0;
//    //ComplementryFilterAngleWeighted(&CFAngleW, 10, 0, 0, 0.5, &acc);
//    //EXPECT_NEAR(CFAngleW, 2.92f, 0.01f);
//}

TEST(test_Controller, minmax_Test)
{
    EXPECT_NEAR(maxVal(0.0, 1.0), 1.0, 0.01);
    EXPECT_NEAR(minVal(0.0, 1.0), 0.0, 0.01);
           
    EXPECT_NEAR(maxVal(0.5, 0.4), 0.5, 0.01);
    EXPECT_NEAR(minVal(0.5, 0.4), 0.4, 0.01);
           
    EXPECT_NEAR(maxVal(-0.1, 1.4), 1.4, 0.01);
    EXPECT_NEAR(minVal(-0.1, 1.4), -0.1, 0.01);

    EXPECT_NEAR(maxVal(0.1, -1.4), 0.1, 0.01);
    EXPECT_NEAR(minVal(0.1, -1.4), -1.4, 0.01);
           
    EXPECT_NEAR(maxVal(0.7, 0.7), 0.7, 0.01);
    EXPECT_NEAR(minVal(0.7, 0.7), 0.7, 0.01);
}

//TEST(test_Controller, calcIRelaxFactor_Test)
//{
//    axis factor;
//    pid_st pid;
//
//    // on/off, within treshhold
//    factor.x = 1.0f;
//    factor.y = 1.0f;
//    factor.z = 1.0f;
//    pid.refSignalDotFiltered.x = 50;
//    pid.refSignalDotFiltered.y = 50;
//    pid.refSignalDotFiltered.z = 50;
//    pid.error.x = 50;
//    pid.error.y = 50;
//    pid.error.z = 50;
//    pid.iRelaxRefThreshhold = 500;
//    pid.iRelaxErrThreshhold = 250;
//    calcIRelaxFactor(&factor, &pid, 1000);
//    EXPECT_NEAR(factor.x, 1.0f, 0.01);
//    EXPECT_NEAR(factor.y, 1.0f, 0.01);
//    EXPECT_NEAR(factor.z, 1.0f, 0.01);
//
//    // on/off, refDot out of treshhold
//    factor.x = 1.0f;
//    factor.y = 1.0f;
//    factor.z = 1.0f;
//    pid.refSignalDotFiltered.x = 600;
//    pid.refSignalDotFiltered.y = 50;
//    pid.refSignalDotFiltered.z = 550;
//    pid.error.x = 50;
//    pid.error.y = 50;
//    pid.error.z = 50;
//    pid.iRelaxRefThreshhold = 500;
//    pid.iRelaxErrThreshhold = 250;
//    calcIRelaxFactor(&factor, &pid, 1000);
//    EXPECT_NEAR(factor.x, 0.0f, 0.01);
//    EXPECT_NEAR(factor.y, 1.0f, 0.01);
//    EXPECT_NEAR(factor.z, 0.0f, 0.01);
//
//    // on/off, error out of treshhold
//    factor.x = 1.0f;
//    factor.y = 1.0f;
//    factor.z = 1.0f;
//    pid.refSignalDotFiltered.x = 50;
//    pid.refSignalDotFiltered.y = 50;
//    pid.refSignalDotFiltered.z = 50;
//    pid.error.x = 260;
//    pid.error.y = 270;
//    pid.error.z = 50;
//    pid.iRelaxRefThreshhold = 500;
//    pid.iRelaxErrThreshhold = 250;
//    calcIRelaxFactor(&factor, &pid, 1000);
//    EXPECT_NEAR(factor.x, 0.0f, 0.01);
//    EXPECT_NEAR(factor.y, 0.0f, 0.01);
//    EXPECT_NEAR(factor.z, 1.0f, 0.01);
//
//    // norm, refDot out of treshhold
//    factor.x = 1.0f;
//    factor.y = 1.0f;
//    factor.z = 1.0f;
//    pid.refSignalDotFiltered.x = 500;
//    pid.refSignalDotFiltered.y = 100;
//    pid.refSignalDotFiltered.z = 200;
//    pid.error.x = 0;
//    pid.error.y = 0;
//    pid.error.z = 0;
//    pid.iRelaxRefThreshhold = 400;
//    pid.iRelaxErrThreshhold = 125;
//    calcIRelaxFactor(&factor, &pid, 2000);
//    EXPECT_NEAR(factor.x, 0.0f, 0.01);
//    EXPECT_NEAR(factor.y, 0.75f, 0.01);
//    EXPECT_NEAR(factor.z, 0.5f, 0.01);
//
//    // norm, error out of treshhold
//    factor.x = 1.0f;
//    factor.y = 1.0f;
//    factor.z = 1.0f;
//    pid.refSignalDotFiltered.x = 0;
//    pid.refSignalDotFiltered.y = 0;
//    pid.refSignalDotFiltered.z = 0;
//    pid.error.x = -75;
//    pid.error.y = 250;
//    pid.error.z = -10;
//    pid.iRelaxRefThreshhold = 400;
//    pid.iRelaxErrThreshhold = 125;
//    calcIRelaxFactor(&factor, &pid, 2000);
//    EXPECT_NEAR(factor.x, 0.4f, 0.01);
//    EXPECT_NEAR(factor.y, 0.0f, 0.01);
//    EXPECT_NEAR(factor.z, 0.92f, 0.01);
//
//    // norm, error out of treshholds
//    factor.x = 1.0f;
//    factor.y = 1.0f;
//    factor.z = 1.0f;
//    pid.refSignalDotFiltered.x = 200;
//    pid.refSignalDotFiltered.y = 300;
//    pid.refSignalDotFiltered.z = 80;
//    pid.error.x = 75;
//    pid.error.y = 250;
//    pid.error.z = 25;
//    pid.iRelaxRefThreshhold = 400;
//    pid.iRelaxErrThreshhold = 125;
//    calcIRelaxFactor(&factor, &pid, 2000);
//    EXPECT_NEAR(factor.x, 0.4f, 0.01);
//    EXPECT_NEAR(factor.y, 0.0f, 0.01);
//    EXPECT_NEAR(factor.z, 0.80f, 0.01);
//}

//TEST(test_Controller, calcDmaxFactor_Test)
//{
//    axis dDynamic;
//    pid_st pid;
//
//    // ref based
//    dDynamic.x = 1.0f;
//    dDynamic.y = 1.0f;
//    dDynamic.z = 1.0f;
//    pid.refSignalDotFiltered.x = 0;
//    pid.refSignalDotFiltered.y = 200;
//    pid.refSignalDotFiltered.z = -700;
//    pid.error.x = 0;
//    pid.error.y = 0;
//    pid.error.z = 0;
//    pid.dMaxRefThreshhold = 600;
//    pid.dMaxErrThreshhold = 300;
//    pid.D.x = 100;
//    pid.D.y = 100;
//    pid.D.z = 100;
//    pid.Dmax.x = 100;
//    pid.Dmax.y = 100;
//    pid.Dmax.z = 200;
//    calcDmaxFactor(&dDynamic, &pid);
//    EXPECT_NEAR(dDynamic.x, 100.0f, 0.1);
//    EXPECT_NEAR(dDynamic.y, 100.0f, 0.1);
//    EXPECT_NEAR(dDynamic.z, 200.0f, 0.1);
//
//    // error based
//    dDynamic.x = 1.0f;
//    dDynamic.y = 1.0f;
//    dDynamic.z = 1.0f;
//    pid.refSignalDotFiltered.x = 0;
//    pid.refSignalDotFiltered.y = 0;
//    pid.refSignalDotFiltered.z = 0;
//    pid.error.x = 10;
//    pid.error.y = -100;
//    pid.error.z = 350;
//    pid.dMaxRefThreshhold = 600;
//    pid.dMaxErrThreshhold = 300;
//    pid.D.x = 100;
//    pid.D.y = 100;
//    pid.D.z = 100;
//    pid.Dmax.x = 300;
//    pid.Dmax.y = 400;
//    pid.Dmax.z = 500;
//    calcDmaxFactor(&dDynamic, &pid);
//    EXPECT_NEAR(dDynamic.x, 106.6f, 0.1);
//    EXPECT_NEAR(dDynamic.y, 200.0f, 0.1);
//    EXPECT_NEAR(dDynamic.z, 500.0f, 0.1);
//}

//TEST(test_Controller, wobble_Test)
//{
//    float pi{ 3.14 };
//    float amplitde{100};
//    //test: 0 increment
//    wobbleTime = 0.0f;
//    EXPECT_EQ(wobble(1000, 1000), 0);
//
//    //test: pi/10 step, 800 amp
//    wobbleTime = 0.0f;
//    amplitde = 800;
//    EXPECT_NEAR(wobble(1200, 1200), amplitde * sin(pi * 1 / 10), 10);
//    EXPECT_NEAR(wobble(1200, 1200), amplitde * sin(pi * 2 / 10), 10);
//    EXPECT_NEAR(wobble(1200, 1200), amplitde * sin(pi * 3 / 10), 10);
//    EXPECT_NEAR(wobble(1200, 1200), amplitde * sin(pi * 4 / 10), 10);
//    EXPECT_NEAR(wobble(1200, 1200), amplitde * sin(pi * 5 / 10), 10);
//    EXPECT_NEAR(wobble(1200, 1200), amplitde * sin(pi * 6 / 10), 10);
//    EXPECT_NEAR(wobble(1200, 1200), amplitde * sin(pi * 7 / 10), 10);
//    EXPECT_NEAR(wobble(1200, 1200), amplitde * sin(pi * 8 / 10), 10);
//    EXPECT_NEAR(wobble(1200, 1200), amplitde * sin(pi * 9 / 10), 10);
//    EXPECT_NEAR(wobble(1200, 1200), amplitde * sin(pi * 10 / 10), 10);
//    EXPECT_NEAR(wobbleTime, pi, 0.01);
//
//    //test: pi/4 step, 2400 amp
//    wobbleTime = 0.0f;
//    amplitde = 2400;
//    EXPECT_NEAR(wobble(1500, 1600), amplitde * sin(pi * 1 / 4), 10);
//    EXPECT_NEAR(wobble(1500, 1600), amplitde * sin(pi * 2 / 4), 10);
//    EXPECT_NEAR(wobble(1500, 1600), amplitde * sin(pi * 3 / 4), 10);
//    EXPECT_NEAR(wobble(1500, 1600), amplitde * sin(pi * 4 / 4), 10);
//    EXPECT_NEAR(wobble(1500, 1600), amplitde * sin(pi * 5 / 4), 10);
//    EXPECT_NEAR(wobble(1500, 1600), amplitde * sin(pi * 6 / 4), 10);
//    EXPECT_NEAR(wobble(1500, 1600), amplitde * sin(pi * 7 / 4), 10);
//    EXPECT_NEAR(wobble(1500, 1600), amplitde * sin(pi * 8 / 4), 10);
//    EXPECT_NEAR(wobbleTime, 2*pi, 0.01);
//
//    //test: continue, pi/20 step, 4000 amp
//    amplitde = 4000;
//    EXPECT_NEAR(wobble(1100, 2000), amplitde * sin(pi * 1 / 20), 10);
//    EXPECT_NEAR(wobble(1100, 2000), amplitde * sin(pi * 2 / 20), 10);
//    EXPECT_NEAR(wobble(1100, 2000), amplitde * sin(pi * 3 / 20), 10);
//}