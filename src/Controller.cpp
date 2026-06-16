//initial PID tuning
//increase D until its "restless"/shacky, lower back to stable, then take 0.75 of it
//increase P until its oscillates quickly, take 0.5 of it
//increase I until iss slow oscillates, take 0.5 of it
//now start to increase P then D maybe I until it has still good dinamic

#include "pch.h"

#include "Controller.h"
#include "RC.h"
#include "sysTime.h"
#include "MotorControl.h"
#include "SPI.h"
#include "RC.h"
#include <cmath>

#ifdef UNIT_TEST

#include "../test/helper/support4Testing.h"
#include "../test/helper/support4Testing.hpp"

extern DummySerial SerialUSB;
extern Tc* TC0;

#else

#include "arduino.h"
#include "variant.h"

#endif

//element: 1th order(x), 3rd order(x^3), 5th order(x^5) 
//float parabolicConst4Rate[3];
float maxYawRate = 500.0f;
int32_t maxYawRate_int = 500;

#define CTRL_CYCLE_TICK 5250 //0.005ms in ticks
#define HALFPI 1.57079f
#define PI 3.14159f
#define TWOPI 6.28318f
#define LUT_SIZE 21
const float sinTime[LUT_SIZE] = { 0.0f, 0.4f, 0.75f, 1.05f, 1.3f, 1.55f, 1.8f, 2.05f, 2.35f, 2.7f,
    3.141f, 3.541f, 3.891f, 4.191f, 4.441f, 4.691f, 4.941f, 5.191f, 5.491f, 5.841f, 6.28318f};
const float sinWave[LUT_SIZE] = { 0.0f, 0.3894183f, 0.6816387f, 0.8674232f, 0.9635581f, 0.9997837f,
    0.9738476f, 0.8873623f, 0.7114733f, 0.4273798f, 0.0005926f, -0.3888724f, -0.6812050f,
    -0.8671281f, -0.9633994f, -0.9997712f, -0.9739821f, -0.8876354f, -0.7118896f, -0.4279156f, 0.0f};
float wobbleTime{ 0.0f };

volatile E_armState armState{ DISARMED };
controllerIn_st controlIn;
controllerOut_st controlOut;
MotorInput motorInput;

pid_st pidRate;
pid_st pidCascade;
gyroData_st gyroData;
accData_st accData;

float timer1;
float timer2;

void SetupController(void)
{
    //RATE
    pidRate.P_i.x = 35;
    pidRate.I_i.x = 1;
    pidRate.D_i.x = 25;
    pidRate.P_i.y = 35;
    pidRate.I_i.y = 1;
    pidRate.D_i.y = 25;
    pidRate.P_i.z = 200;
    pidRate.I_i.z = 0;
    pidRate.D_i.z = 0;
    pidRate.FFr_i.x = 0;
    pidRate.FFr_i.y = 0;
    pidRate.FFr_i.z = 0;
    pidRate.FFdr_i.x = 0;
    pidRate.FFdr_i.y = 0;
    pidRate.FFdr_i.z = 0;
    pidRate.satI_i = 0x7FFFFFFF;  //int32 close to max
    pidRate.satPID_i = 10000000;
    pidRate.Ki_i.x = pidRate.I_i.x * pidRate.deltaTicks;
    pidRate.Ki_i.y = pidRate.I_i.y * pidRate.deltaTicks;
    pidRate.Ki_i.z = pidRate.I_i.z * pidRate.deltaTicks;
    pidRate.Kd_i.x = pidRate.D_i.x * pidRate.inverseDt;
    pidRate.Kd_i.y = pidRate.D_i.y * pidRate.inverseDt;
    pidRate.Kd_i.z = pidRate.D_i.z * pidRate.inverseDt;
    pidRate.Kffr_i.x = pidRate.FFr_i.x * pidRate.inverseDt;
    pidRate.Kffr_i.y = pidRate.FFr_i.y * pidRate.inverseDt;
    pidRate.Kffr_i.z = pidRate.FFr_i.z * pidRate.inverseDt;
    pidRate.Kffdr_i.x = pidRate.FFdr_i.x * pidRate.inverseDt;
    pidRate.Kffdr_i.y = pidRate.FFdr_i.y * pidRate.inverseDt;
    pidRate.Kffdr_i.z = pidRate.FFdr_i.z * pidRate.inverseDt;
 //   //iRelax
 //   pidRate.iRelaxRefThreshhold = 6000.0f;
 //   pidRate.iRelaxErrThreshhold = 500.0f;
 //   //Dmax
 //   pidRate.Dmax.x = pidRate.D.x;
 //   pidRate.Dmax.y = pidRate.D.y;
 //   pidRate.Dmax.z = pidRate.D.z;
 //   pidRate.dMaxRefThreshhold = 300.0f;
 //   pidRate.dMaxErrThreshhold = 100.0f;

	//PID
	pidRate.errorSum_i.x = 0.0f;
	pidRate.errorSum_i.y = 0.0f;
	pidRate.errorSum_i.z = 0.0f;
	pidRate.errorPrev_i.x = 0.0f;
	pidRate.errorPrev_i.y = 0.0f;
	pidRate.errorPrev_i.z = 0.0f;
	pidRate.errorDotPT1_i.x = 0.0f;
	pidRate.errorDotPT1_i.y = 0.0f;
	pidRate.errorDotPT1_i.z = 0.0f;

    //CASCADE
    //pidCascade.P.x = 30.0f;
    //pidCascade.I.x = 0.0f;
    //pidCascade.D.x = 0.0f;
    //pidCascade.P.y = 30.0f;
    //pidCascade.I.y = 0.0f;
    //pidCascade.D.y = 0.0f;
    //pidCascade.P.z = 0.0f;
    //pidCascade.I.z = 0.0f;
    //pidCascade.D.z = 0.0f;
    //pidCascade.saturationI = 75.0f;
    //pidCascade.saturationPID = 300.0f;
    //pidCascade.DTermC = 1600 / 25;	//datarate/filterrate = 2000hz/500hz
    //pidCascade.PFactor = 10.0f;
    //pidCascade.IFactor = 10.0f;
    //pidCascade.DFactor = 1.0f;
    //pidCascade.FFrFactor = 1.0f;
    //pidCascade.FFdrFactor = 1.0f;

    //PID
    //pidCascade.errorSum.x = 0.0f;
    //pidCascade.errorSum.y = 0.0f;
    //pidCascade.errorSum.z = 0.0f;
    //pidCascade.errorPrev.x = 0.0f;
    //pidCascade.errorPrev.y = 0.0f;
    //pidCascade.errorPrev.z = 0.0f;
    //pidCascade.errorDotFiltered.x = 0.0f;
    //pidCascade.errorDotFiltered.y = 0.0f;
    //pidCascade.errorDotFiltered.z = 0.0f;

    SetupCtrlLoopTimer();
}

void SetupCtrlLoopTimer(void)
{
    pmc_enable_periph_clk(ID_TC0);  //enable peripheral clock for TC0-channel 0

    TC0->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC0->TC_CHANNEL[0].TC_CCR |= TC_CCR_CLKEN;  //enable clock
    TC0->TC_CHANNEL[0].TC_CMR |= TC_CMR_TCCLKS_TIMER_CLOCK2;  //clock selection MCK/8=10.5 MHz -> 1tick=95.238 nanosec
    TC0->TC_CHANNEL[0].TC_CMR |= TC_CMR_CPCTRG; // RC compare resets counter and starts clock
    TC0->TC_CHANNEL[0].TC_RC = TC_RC_RC(5250);  //setting RC to 0.5ms
    TC0->TC_CHANNEL[0].TC_IER |= TC_IER_CPCS;   //enable RC compare interrupt
    TC0->TC_WPMR = 0x504D4301;  //reenable write protection mode

    //enable interrupt
    NVIC_SetPriority(TC0_IRQn, 4);
    NVIC_EnableIRQ(TC0_IRQn);

    TC0->TC_CHANNEL[0].TC_CCR |= TC_CCR_SWTRG;  //software triggering
}

void TC0_Handler(void)
{
    uint32_t tc0_status = TC0->TC_CHANNEL[0].TC_SR;

    if (tc0_status & TC_SR_CPCS)    //RC compare status
    {
        timer1 = getTimeSinceReset();

        getRcChannels(&controlIn.rcSignals);
        getGyroAndAcc(&controlIn.gyro, &controlIn.acc);
      
        //testing
        static uint64_t counter{0};
        if(counter > 10000) //5sec
        {
          controlIn.rcSignals.throttle = 1000;
          controlIn.rcSignals.roll = 1500;
          controlIn.rcSignals.pitch = 1500;
          controlIn.rcSignals.yaw = 1500;
          controlIn.rcSignals.armStateSwitch = 2000;
          controlIn.rcSignals.measurementSwitch = 1000;
          controlIn.rcSignals.Poti1 = 1500;
          controlIn.rcSignals.Poti2 = 1600;
          controlIn.rcSignals.flightModeSwitch = 1000;
          controlIn.rcSignals.Switch2Way = 1000;
        }
        if(counter > 12000) //6sec
        {
          controlIn.rcSignals.throttle = 1100;
          controlIn.rcSignals.roll = 1500;
          controlIn.rcSignals.pitch = 1500;
          controlIn.rcSignals.yaw = 1500;
          controlIn.rcSignals.armStateSwitch = 2000;
          controlIn.rcSignals.measurementSwitch = 1000;
          controlIn.rcSignals.Poti1 = 1500;
          controlIn.rcSignals.Poti2 = 1600;
          controlIn.rcSignals.flightModeSwitch = 1500;
          controlIn.rcSignals.Switch2Way = 1000;
        }
        if(counter > 20000)  //10sec
        {
          controlIn.rcSignals.throttle = 1000;
          controlIn.rcSignals.roll = 1500;
          controlIn.rcSignals.pitch = 1500;
          controlIn.rcSignals.yaw = 1500;
          controlIn.rcSignals.armStateSwitch = 1000;
          controlIn.rcSignals.measurementSwitch = 1000;
          controlIn.rcSignals.Poti1 = 1000;
          controlIn.rcSignals.Poti2 = 1000;
          controlIn.rcSignals.flightModeSwitch = 1000;
          controlIn.rcSignals.Switch2Way = 1000;
        }
        counter++;

        RunController(&controlIn, &controlOut);

        motorInput.throttle = controlIn.rcSignals.throttle;
        motorInput.x = controlOut.U_i.x;
        motorInput.y = controlOut.U_i.y;
        motorInput.z = controlOut.U_i.z;
        motorInput.armState = controlOut.armState;
        motorInput.poti1 = controlIn.rcSignals.Poti1;
        motorInput.poti2 = controlIn.rcSignals.Poti2;
        motorInput.twoWaySwitch2 = controlIn.rcSignals.Switch2Way;

        UpdateMotorSpeeds(&motorInput);
        
        timer2 = getTimeSinceReset();
    }
}

void ControllerDebug(void)
{
    SerialUSB.print("2-1: ");
    SerialUSB.println(calcDeltaTime(timer1, timer2), 3);
}

void RunController(const controllerIn_st* ctrlIn, controllerOut_st* ctrlOut)
{
    E_flightMode flightMode = EvalFlightMode(ctrlIn->rcSignals.flightModeSwitch);

    EvalArmState(&ctrlIn->rcSignals);

    //{
    //    accData.rollAngle =  atan2(ctrlIn->acc.signal.y,  
    //        sqrt(ctrlIn->acc.signal.x * ctrlIn->acc.signal.x + ctrlIn->acc.signal.z * ctrlIn->acc.signal.z)) * 180 / 3.14;
    //    accData.pitchAngle = atan2(-ctrlIn->acc.signal.x, 
    //        sqrt(ctrlIn->acc.signal.y * ctrlIn->acc.signal.y + ctrlIn->acc.signal.z * ctrlIn->acc.signal.z)) * 180 / 3.14;
    //
    //    accData.rollAnglePT1Acc = atan2(accData.PT1.signal.y,
    //        sqrt(accData.PT1.signal.x * accData.PT1.signal.x + accData.PT1.signal.z * accData.PT1.signal.z)) * 180 / 3.14;
    //    accData.pitchAnglePT1Acc = atan2(-accData.PT1.signal.x, 
    //        sqrt(accData.PT1.signal.y * accData.PT1.signal.y + accData.PT1.signal.z * accData.PT1.signal.z)) * 180 / 3.14;
    //    accData.rollAnglePT2Acc = atan2(accData.PT2.signal.y,
    //        sqrt(accData.PT2.signal.x * accData.PT2.signal.x + accData.PT2.signal.z * accData.PT2.signal.z)) * 180 / 3.14;
    //    accData.pitchAnglePT2Acc = atan2(-accData.PT2.signal.x, 
    //        sqrt(accData.PT2.signal.y * accData.PT2.signal.y + accData.PT2.signal.z * accData.PT2.signal.z)) * 180 / 3.14;
    //    //SerialUSB.print(accData.rollAngle); SerialUSB.print('\t');
    //    //SerialUSB.println(accData.pitchAngle);
    //}
    ////kalman filter angle
    //{
    //    KalmanFilterAngle(&accData.angleKF.roll, accData.rollAngle, ctrlIn->gyro.signal.x, ctrlIn->droneTimes.loopTime);
    //    KalmanFilterAngle(&accData.angleKF.pitch, accData.pitchAngle, ctrlIn->gyro.signal.y, ctrlIn->droneTimes.loopTime);
    //
    //    KalmanFilterAngle(&accData.angleKFPT10.roll, accData.rollAnglePT1Acc, ctrlIn->gyro.signal.x, ctrlIn->droneTimes.loopTime);
    //    KalmanFilterAngle(&accData.angleKFPT10.pitch, accData.pitchAnglePT1Acc, ctrlIn->gyro.signal.y, ctrlIn->droneTimes.loopTime);
    //}
    ////complementary filter angle
    //{
    //    ComplementryFilterAngle(&accData.rollAngleCF, accData.rollAngle, ctrlIn->gyro.signal.x, ctrlIn->droneTimes.loopTime, accData.alpha);
    //    ComplementryFilterAngle(&accData.pitchAngleCF, accData.pitchAngle, ctrlIn->gyro.signal.y, ctrlIn->droneTimes.loopTime, accData.alpha);
    //    ComplementryFilterAngle(&accData.rollAngleCF10, accData.rollAnglePT1Acc, ctrlIn->gyro.signal.x, ctrlIn->droneTimes.loopTime, accData.alpha);
    //    ComplementryFilterAngle(&accData.pitchAngleCF10, accData.pitchAnglePT1Acc, ctrlIn->gyro.signal.y, ctrlIn->droneTimes.loopTime, accData.alpha);
    //    //SerialUSB.print(accData.rollAngleCFw, 3); SerialUSB.print('\t');
    //    //SerialUSB.println(accData.rollAngleCFw01, 3);
    //}

    //control when armed and on high throttle
    if (ARMED == armState && ctrlIn->rcSignals.throttle > 1010)
    {
        int32_t rollScaled_int{ expo(ctrlIn->rcSignals.roll) };
        int32_t pitchScaled_int{ expo(ctrlIn->rcSignals.pitch) };
        int32_t yawScaled_int{ linearScale_8192(ctrlIn->rcSignals.yaw) };

        switch (flightMode)
        {
        case RATE_CTRL_INT:
        {
            pidRate.refSig_i.x = rollScaled_int;
            pidRate.refSig_i.y = -pitchScaled_int;
            pidRate.refSig_i.z = yawScaled_int;
            pidRate.sensor.signalPT1.x = -ctrlIn->gyro.signalPT1.x;
            pidRate.sensor.signalPT1.y = ctrlIn->gyro.signalPT1.y;
            pidRate.sensor.signalPT1.z = -ctrlIn->gyro.signalPT1.z;
            pidRate.sensor.newData = ctrlIn->gyro.newData;

            CalcPID_int(&pidRate, &ctrlOut->U_i);

            //SerialUSB.print("ct: ");
            //SerialUSB.println(pidRate.refSignal_int.x);
            //SerialUSB.println(ctrlOut->U_int.x);

            break;
        }
        //case RATE_CTRL_PT1_IRelax_Dmax:
        //{
        //    pidRate.refSignal.x = rollScaled;
        //    pidRate.refSignal.y = -pitchScaled;
        //    pidRate.refSignal.z = yawScaled;
        //    pidRate.sensor.signal.x = -gyroData.PT1.signal.x;    //x-y swap/+-1 due to orientation of IMU
        //    pidRate.sensor.signal.y = gyroData.PT1.signal.y;
        //    pidRate.sensor.signal.z = -gyroData.PT1.signal.z;
        //    pidRate.sensor.newData = ctrlIn->gyro.newData;
        //    pidRate.deltaT = ctrlIn->droneTimes.loopTime;
        //
        //    CalcPID_wo_Dkick_FF_IRelax_Dmax(&pidRate, &controlSignal, ctrlIn->rcSignals.Switch2Way);
        //
        //    break;
        //}
        //case ANGLE_CASCADE_CTRL:
        //{
        //    float rollAngle{ LinearInterpol(ctrlIn->rcSignals.roll, 1000u,2000u, -30.0f , 30.0f) };
        //    float pitchAngle{ LinearInterpol(ctrlIn->rcSignals.pitch, 1000u,2000u, 30.0f , -30.0f) };
        //    axis intermidiateSignal;
        //
        //    //outter cascade: angle
        //    pidCascade.refSignal.x = rollAngle;
        //    pidCascade.refSignal.y = pitchAngle;
        //    pidCascade.sensor.signal.x = -accData.angleKF.roll.angle;
        //    pidCascade.sensor.signal.y = accData.angleKF.pitch.angle;
        //    pidCascade.sensor.newData = false;  //no D term
        //    pidCascade.deltaT = ctrlIn->droneTimes.loopTime;
        //
        //    CalcPID_wo_Dkick_FF(&pidCascade, &intermidiateSignal);
        //
        //    //inner cascade: rate
        //    pidRate.refSignal.x = intermidiateSignal.x;
        //    pidRate.refSignal.y = intermidiateSignal.y;
        //    pidRate.refSignal.z = yawScaled;
        //    pidRate.sensor.signal.x = -gyroData.PT1.signal.x;    //x-y swap/+-1 due to orientation of IMU
        //    pidRate.sensor.signal.y = gyroData.PT1.signal.y;
        //    pidRate.sensor.signal.z = -gyroData.PT1.signal.z;
        //    pidRate.sensor.newData = ctrlIn->gyro.newData;
        //    pidRate.deltaT = ctrlIn->droneTimes.loopTime;
        //
        //    CalcPID_wo_Dkick_FF(&pidRate, &controlSignal);
        //
        //    break;
        //}
        case GPS_CTRL:
        {
            break;
        }
        default:
            // should not reach
            break;
        }
    }
    else
    {
        //stop control, no control
        ctrlOut->U_i.x = 0;
        ctrlOut->U_i.y = 0;
        ctrlOut->U_i.z = 0;
        //reset RATE
        pidRate.error_i.x = 0;
        pidRate.error_i.y = 0;
        pidRate.error_i.z = 0;
        pidRate.errorSum_i.x = 0;
        pidRate.errorSum_i.y = 0;
        pidRate.errorSum_i.z = 0;
        pidRate.errorDot_i.x = 0;
        pidRate.errorDot_i.y = 0;
        pidRate.errorDot_i.z = 0;
        pidRate.errorPrev_i.x = 0;
        pidRate.errorPrev_i.y = 0;
        pidRate.errorPrev_i.z = 0;
        pidRate.errorDotPT1_i.x = 0;
        pidRate.errorDotPT1_i.y = 0;
        pidRate.errorDotPT1_i.z = 0;
        pidRate.signalPT1Prev_i.x = 0;
        pidRate.signalPT1Prev_i.y = 0;
        pidRate.signalPT1Prev_i.z = 0;
        //reset CASCADE
        //pidCascade.error.x = 0.0f;
        //pidCascade.error.y = 0.0f;
        //pidCascade.error.z = 0.0f;
        //pidCascade.errorSum.x = 0.0f;
        //pidCascade.errorSum.y = 0.0f;
        //pidCascade.errorSum.z = 0.0f;
        //pidCascade.errorDot.x = 0.0f;
        //pidCascade.errorDot.y = 0.0f;
        //pidCascade.errorDot.z = 0.0f;
        //pidCascade.errorPrev.x = 0.0f;
        //pidCascade.errorPrev.y = 0.0f;
        //pidCascade.errorPrev.z = 0.0f;
        //pidCascade.errorDotFiltered.x = 0.0f;
        //pidCascade.errorDotFiltered.y = 0.0f;
        //pidCascade.errorDotFiltered.z = 0.0f;
        //pidCascade.sensorPrev.signal.x = 0.0f;
        //pidCascade.sensorPrev.signal.y = 0.0f;
        //pidCascade.sensorPrev.signal.z = 0.0f;

    }

    ctrlOut->armState = armState;
}

void EvalArmState(const rcSignals_st* rcSig)
{
    if (rcSig->armStateSwitch > 1800u)
    {
        if (rcSig->throttle <= 1010)
        {
            armState = E_armState::ARMED;
        }
        else
        {
            //keep previous state
        }
    }
    else
    {
        armState = E_armState::DISARMED;
    }
}

E_flightMode EvalFlightMode(const uint16_t flightModeChannel)
{
    if (1800u < flightModeChannel)
    {
        return ANGLE_CASCADE_CTRL;
    }
    else if (1450u < flightModeChannel && flightModeChannel < 1550u)
    {
        return RATE_CTRL_PT1;
    }
    else
    {
        return RATE_CTRL_INT;
    }
}

//[-17453 ... +17453]=~-1000...-1000
inline int32_t expo(const uint16_t channel)
{
    int64_t x = (int32_t)channel - 1500;
    int32_t linear = x * 3;
    int64_t power5 = (int64_t)x * x * x * x * x;
    return linear + (int32_t)(power5 >> 31);
}

//~[-8192 - 8192]=[-500 - 500]
inline int32_t linearScale_8192(uint16_t ch)
{
    int32_t x = (int32_t)ch - 1500;

    return (x << 4) + (x >> 2) + (x >> 3);
}

//basic improvement: 
//1st: trapezoidal integral
//2nd: https://en.wikipedia.org/wiki/Proportional%E2%80%93integral%E2%80%93derivative_controller
//void CalcPID(pid_st* pidSt, axis* u)
//{
//    //error(P)
//    pidSt->error.x = pidSt->refSignal.x - pidSt->sensor.signal.x;
//    pidSt->error.y = pidSt->refSignal.y - pidSt->sensor.signal.y;
//    pidSt->error.z = pidSt->refSignal.z - pidSt->sensor.signal.z;
//
//    //errorSum(I)
//	pidSt->errorSum.x += pidSt->error.x * pidSt->deltaT;
//	pidSt->errorSum.y += pidSt->error.y * pidSt->deltaT;
//	pidSt->errorSum.z += pidSt->error.z * pidSt->deltaT;
//	//anti-windup of integral(saturation)
//	if (pidSt->errorSum.x > pidSt->saturationI) pidSt->errorSum.x = pidSt->saturationI;
//	else if (pidSt->errorSum.x < -pidSt->saturationI) pidSt->errorSum.x = -pidSt->saturationI;
//	else; //do nothing
//	if (pidSt->errorSum.y > pidSt->saturationI) pidSt->errorSum.y = pidSt->saturationI;
//	else if (pidSt->errorSum.y < -pidSt->saturationI) pidSt->errorSum.y = -pidSt->saturationI;
//	else; //do nothing
//	if (pidSt->errorSum.z > pidSt->saturationI) pidSt->errorSum.z = pidSt->saturationI;
//	else if (pidSt->errorSum.z < -pidSt->saturationI) pidSt->errorSum.z = -pidSt->saturationI;
//	else; //do nothing	
//
//    //errorDelta(D)
//	if (true == pidSt->sensor.newData)
//	{
//		pidSt->errorDelta.x = (pidSt->error.x - pidSt->errorPrev.x) / pidSt->deltaT;
//		pidSt->errorDelta.y = (pidSt->error.y - pidSt->errorPrev.y) / pidSt->deltaT;
//		pidSt->errorDelta.z = (pidSt->error.z - pidSt->errorPrev.z) / pidSt->deltaT;
//		pidSt->errorPrev.x = pidSt->error.x;
//		pidSt->errorPrev.y = pidSt->error.y;
//		pidSt->errorPrev.z = pidSt->error.z;
//        //PT1 filtering D
//        PT1Filter(&pidSt->errorDeltaFiltered.x, pidSt->errorDelta.x, pidSt->DTermC);
//        PT1Filter(&pidSt->errorDeltaFiltered.y, pidSt->errorDelta.y, pidSt->DTermC);
//        PT1Filter(&pidSt->errorDeltaFiltered.z, pidSt->errorDelta.z, pidSt->DTermC);
//
//	    pidSt->sensor.newData = false;
//	}
//	else; //do nothing, keep previously calculated delta
//
//
//	//output(PID), divider due to handleable PID values
//	u->x = pidSt->P.x / 10000.0f * pidSt->error.x 
//		+ pidSt->I.x / 100.0f * pidSt->errorSum.x
//		+ pidSt->D.x / 1000000.0f * pidSt->errorDeltaFiltered.x;
//	u->y = pidSt->P.y / 10000.0f * pidSt->error.y
//		+ pidSt->I.y / 100.0f * pidSt->errorSum.y
//		+ pidSt->D.y / 1000000.0f * pidSt->errorDeltaFiltered.y;
//	u->z = pidSt->P.z / 10000.0f * pidSt->error.z
//		+ pidSt->I.z / 100.0f * pidSt->errorSum.z
//		+ pidSt->D.z / 1000000.0f * pidSt->errorDeltaFiltered.z;
//	//limit output(saturation)
//	if (u->x > pidSt->saturationPID) u->x = pidSt->saturationPID;
//	else if (u->x < -pidSt->saturationPID) u->x = -pidSt->saturationPID;
//	else; //do nothing
//	if (u->y > pidSt->saturationPID) u->y = pidSt->saturationPID;
//	else if (u->y < -pidSt->saturationPID) u->y = -pidSt->saturationPID;
//	else; //do nothing
//	if (u->z > pidSt->saturationPID) u->z = pidSt->saturationPID;
//	else if (u->z < -pidSt->saturationPID) u->z = -pidSt->saturationPID;
//	else; //do nothing
//}

//void CalcPID_wo_Dkick(pid_st* pidSt, axis* u)
//{
//    //error
//    pidSt->error.x = pidSt->refSignal.x - pidSt->sensor.signal.x;
//    pidSt->error.y = pidSt->refSignal.y - pidSt->sensor.signal.y;
//    pidSt->error.z = pidSt->refSignal.z - pidSt->sensor.signal.z;
//    //Pout
//    pidSt->Pout.x = pidSt->P.x / pidSt->PFactor * pidSt->error.x;
//    pidSt->Pout.y = pidSt->P.y / pidSt->PFactor * pidSt->error.y;
//    pidSt->Pout.z = pidSt->P.z / pidSt->PFactor * pidSt->error.z;
//
//    //errorSum
//    pidSt->errorSum.x += pidSt->error.x * pidSt->deltaT;
//    pidSt->errorSum.y += pidSt->error.y * pidSt->deltaT;
//    pidSt->errorSum.z += pidSt->error.z * pidSt->deltaT;
//    //anti-windup of integral(saturation)
//    if (pidSt->errorSum.x > pidSt->saturationI) pidSt->errorSum.x = pidSt->saturationI;
//    else if (pidSt->errorSum.x < -pidSt->saturationI) pidSt->errorSum.x = -pidSt->saturationI;
//    else; //do nothing
//    if (pidSt->errorSum.y > pidSt->saturationI) pidSt->errorSum.y = pidSt->saturationI;
//    else if (pidSt->errorSum.y < -pidSt->saturationI) pidSt->errorSum.y = -pidSt->saturationI;
//    else; //do nothing
//    if (pidSt->errorSum.z > pidSt->saturationI) pidSt->errorSum.z = pidSt->saturationI;
//    else if (pidSt->errorSum.z < -pidSt->saturationI) pidSt->errorSum.z = -pidSt->saturationI;
//    else; //do nothing	
//    //Iout
//    pidSt->Iout.x = pidSt->I.x / pidSt->IFactor * pidSt->errorSum.x;
//    pidSt->Iout.y = pidSt->I.y / pidSt->IFactor * pidSt->errorSum.y;
//    pidSt->Iout.z = pidSt->I.z / pidSt->IFactor * pidSt->errorSum.z;
//
//    //errorDelta, avoid kick
//    if (true == pidSt->sensor.newData)
//    {
//        pidSt->errorDot.x = (pidSt->sensor.signal.x - pidSt->sensorPrev.signal.x) / pidSt->deltaT;
//        pidSt->errorDot.y = (pidSt->sensor.signal.y - pidSt->sensorPrev.signal.y) / pidSt->deltaT;
//        pidSt->errorDot.z = (pidSt->sensor.signal.z - pidSt->sensorPrev.signal.z) / pidSt->deltaT;
//        pidSt->sensorPrev.signal.x = pidSt->sensor.signal.x;
//        pidSt->sensorPrev.signal.y = pidSt->sensor.signal.y;
//        pidSt->sensorPrev.signal.z = pidSt->sensor.signal.z;
//        //PT1 filtering D
//        PT1Filter(&pidSt->errorDotFiltered.x, pidSt->errorDot.x, pidSt->DTermC);
//        PT1Filter(&pidSt->errorDotFiltered.y, pidSt->errorDot.y, pidSt->DTermC);
//        PT1Filter(&pidSt->errorDotFiltered.z, pidSt->errorDot.z, pidSt->DTermC);
//
//        pidSt->sensor.newData = false;
//    }
//    else; //do nothing, keep previously calculated delta
//    //Dout
//    pidSt->Dout.x = pidSt->D.x / pidSt->DFactor * pidSt->errorDotFiltered.x;
//    pidSt->Dout.y = pidSt->D.y / pidSt->DFactor * pidSt->errorDotFiltered.y;
//    pidSt->Dout.z = pidSt->D.z / pidSt->DFactor * pidSt->errorDotFiltered.z;
//
//    //output(PID)
//	pidSt->u.x = pidSt->Pout.x + pidSt->Iout.x - pidSt->Dout.x;
//	pidSt->u.y = pidSt->Pout.y + pidSt->Iout.y - pidSt->Dout.y;
//	pidSt->u.z = pidSt->Pout.z + pidSt->Iout.z - pidSt->Dout.z;
//    u->x = pidSt->u.x;
//    u->y = pidSt->u.y;
//    u->z = pidSt->u.z;
//    //limit output(saturation)
//    if (u->x > pidSt->saturationPID) u->x = pidSt->saturationPID;
//    else if (u->x < -pidSt->saturationPID) u->x = -pidSt->saturationPID;
//    else; //do nothing
//    if (u->y > pidSt->saturationPID) u->y = pidSt->saturationPID;
//    else if (u->y < -pidSt->saturationPID) u->y = -pidSt->saturationPID;
//    else; //do nothing
//    if (u->z > pidSt->saturationPID) u->z = pidSt->saturationPID;
//    else if (u->z < -pidSt->saturationPID) u->z = -pidSt->saturationPID;
//    else; //do nothing
//
////    SerialUSB.print(u->x); SerialUSB.print("\t");
//    //    SerialUSB.print(pidSt->error.x); SerialUSB.print("\t");
//    //    SerialUSB.print(pidSt->errorSum.x); SerialUSB.print("\t");
//        //SerialUSB.print(pidSt->error.x, 6); SerialUSB.print("\t");
//        //SerialUSB.print(pidSt->errorPrev.x, 6); SerialUSB.print("\t");
//        //SerialUSB.print(pidSt->deltaT, 6); SerialUSB.print("\t");
////        SerialUSB.print(pidSt->errorDelta.x); SerialUSB.print("\t");
////        SerialUSB.print(pidSt->errorDeltaFiltered.x); SerialUSB.print("\t");
////    SerialUSB.println();
//}
// 
//void CalcPID_wo_Dkick_FF(pid_st* pidSt, axis* u)
//{
//    //feedforward(FF)
//    pidSt->refSignalDot.x = (pidSt->refSignal.x - pidSt->refSignalPrev.x) / pidSt->deltaT;
//    pidSt->refSignalDot.y = (pidSt->refSignal.y - pidSt->refSignalPrev.y) / pidSt->deltaT;
//    pidSt->refSignalDot.z = (pidSt->refSignal.z - pidSt->refSignalPrev.z) / pidSt->deltaT;
//    pidSt->refSignalPrev.x = pidSt->refSignal.x;
//    pidSt->refSignalPrev.y = pidSt->refSignal.y;
//    pidSt->refSignalPrev.z = pidSt->refSignal.z;    
//    PT1Filter(&pidSt->refSignalDotFiltered.x, pidSt->refSignalDot.x, pidSt->FFDTermC);
//    PT1Filter(&pidSt->refSignalDotFiltered.y, pidSt->refSignalDot.y, pidSt->FFDTermC);
//    PT1Filter(&pidSt->refSignalDotFiltered.z, pidSt->refSignalDot.z, pidSt->FFDTermC);
//    pidSt->FFout.x = pidSt->FFr.x / pidSt->FFrFactor * pidSt->refSignal.x + pidSt->FFdr.x / pidSt->FFdrFactor * pidSt->refSignalDotFiltered.x;
//    pidSt->FFout.y = pidSt->FFr.y / pidSt->FFrFactor * pidSt->refSignal.y + pidSt->FFdr.y / pidSt->FFdrFactor * pidSt->refSignalDotFiltered.y;
//    pidSt->FFout.z = pidSt->FFr.z / pidSt->FFrFactor * pidSt->refSignal.z + pidSt->FFdr.z / pidSt->FFdrFactor * pidSt->refSignalDotFiltered.z;
//
//    //error
//    pidSt->error.x = pidSt->refSignal.x - pidSt->sensor.signal.x;
//    pidSt->error.y = pidSt->refSignal.y - pidSt->sensor.signal.y;
//    pidSt->error.z = pidSt->refSignal.z - pidSt->sensor.signal.z;
//    //Pout
//    pidSt->Pout.x = pidSt->P.x / pidSt->PFactor * pidSt->error.x;
//    pidSt->Pout.y = pidSt->P.y / pidSt->PFactor * pidSt->error.y;
//    pidSt->Pout.z = pidSt->P.z / pidSt->PFactor * pidSt->error.z;
//
//    //errorSum
//    pidSt->errorSum.x += pidSt->error.x * pidSt->deltaT;
//    pidSt->errorSum.y += pidSt->error.y * pidSt->deltaT;
//    pidSt->errorSum.z += pidSt->error.z * pidSt->deltaT;
//    //anti-windup of integral(saturation)
//    if (pidSt->errorSum.x > pidSt->saturationI) pidSt->errorSum.x = pidSt->saturationI;
//    else if (pidSt->errorSum.x < -pidSt->saturationI) pidSt->errorSum.x = -pidSt->saturationI;
//    else; //do nothing
//    if (pidSt->errorSum.y > pidSt->saturationI) pidSt->errorSum.y = pidSt->saturationI;
//    else if (pidSt->errorSum.y < -pidSt->saturationI) pidSt->errorSum.y = -pidSt->saturationI;
//    else; //do nothing
//    if (pidSt->errorSum.z > pidSt->saturationI) pidSt->errorSum.z = pidSt->saturationI;
//    else if (pidSt->errorSum.z < -pidSt->saturationI) pidSt->errorSum.z = -pidSt->saturationI;
//    else; //do nothin
//    //Iout
//    pidSt->Iout.x = pidSt->I.x / pidSt->IFactor * pidSt->errorSum.x;
//    pidSt->Iout.y = pidSt->I.y / pidSt->IFactor * pidSt->errorSum.y;
//    pidSt->Iout.z = pidSt->I.z / pidSt->IFactor * pidSt->errorSum.z;
//
//    //errorDelta(D), avoid kick
//    if (true == pidSt->sensor.newData)
//    {
//        pidSt->errorDot.x = (pidSt->sensor.signal.x - pidSt->sensorPrev.signal.x) / pidSt->deltaT;
//        pidSt->errorDot.y = (pidSt->sensor.signal.y - pidSt->sensorPrev.signal.y) / pidSt->deltaT;
//        pidSt->errorDot.z = (pidSt->sensor.signal.z - pidSt->sensorPrev.signal.z) / pidSt->deltaT;
//        pidSt->sensorPrev.signal.x = pidSt->sensor.signal.x;
//        pidSt->sensorPrev.signal.y = pidSt->sensor.signal.y;
//        pidSt->sensorPrev.signal.z = pidSt->sensor.signal.z;
//        //PT1 filtering D
//        PT1Filter(&pidSt->errorDotFiltered.x, pidSt->errorDot.x, pidSt->DTermC);
//        PT1Filter(&pidSt->errorDotFiltered.y, pidSt->errorDot.y, pidSt->DTermC);
//        PT1Filter(&pidSt->errorDotFiltered.z, pidSt->errorDot.z, pidSt->DTermC);
//
//        pidSt->sensor.newData = false;
//    }
//    else; //do nothing, keep previously calculated delta
//    //Dout
//    pidSt->Dout.x = pidSt->D.x / pidSt->DFactor * pidSt->errorDotFiltered.x;
//    pidSt->Dout.y = pidSt->D.y / pidSt->DFactor * pidSt->errorDotFiltered.y;
//    pidSt->Dout.z = pidSt->D.z / pidSt->DFactor * pidSt->errorDotFiltered.z;
//
//
//    //output(PID)
//    u->x = pidSt->FFout.x + pidSt->Pout.x + pidSt->Iout.x - pidSt->Dout.x;
//    u->y = pidSt->FFout.y + pidSt->Pout.y + pidSt->Iout.y - pidSt->Dout.y;
//    u->z = pidSt->FFout.z + pidSt->Pout.z + pidSt->Iout.z - pidSt->Dout.z;
//    pidSt->u.x = u->x;
//    pidSt->u.y = u->y;
//    pidSt->u.z = u->z;
//    //limit output(saturation)
//    if (u->x > pidSt->saturationPID) u->x = pidSt->saturationPID;
//    else if (u->x < -pidSt->saturationPID) u->x = -pidSt->saturationPID;
//    else; //do nothing
//    if (u->y > pidSt->saturationPID) u->y = pidSt->saturationPID;
//    else if (u->y < -pidSt->saturationPID) u->y = -pidSt->saturationPID;
//    else; //do nothing
//    if (u->z > pidSt->saturationPID) u->z = pidSt->saturationPID;
//    else if (u->z < -pidSt->saturationPID) u->z = -pidSt->saturationPID;
//    else; //do nothing
//
////    SerialUSB.print(u->x); SerialUSB.print("\t");
////        SerialUSB.print(refSignalDot.x); SerialUSB.print("\t");
//    //SerialUSB.print(u_ff.x); SerialUSB.print("\t");
////        SerialUSB.print(pidSt->error.x); SerialUSB.print("\t");
//    //    SerialUSB.print(pidSt->errorSum.x); SerialUSB.print("\t");
//        //SerialUSB.print(pidSt->error.x, 6); SerialUSB.print("\t");
//        //SerialUSB.print(pidSt->errorPrev.x, 6); SerialUSB.print("\t");
//        //SerialUSB.print(pidSt->deltaT, 6); SerialUSB.print("\t");
////        SerialUSB.print(pidSt->errorDelta.x); SerialUSB.print("\t");
////        SerialUSB.print(pidSt->errorDeltaFiltered.x); SerialUSB.print("\t");
//    //SerialUSB.print(pidSt->P.x / 10000.0f * pidSt->error.x); SerialUSB.print("\t");
//    //SerialUSB.println();
//}
// 
//void CalcPID_wo_Dkick_FF_IRelax_Dmax(pid_st* pidSt, axis* u, uint16_t twoWayswitch)
//{
//    //feedforward(FF)
//    pidSt->refSignalDot.x = (pidSt->refSignal.x - pidSt->refSignalPrev.x) / pidSt->deltaT;
//    pidSt->refSignalDot.y = (pidSt->refSignal.y - pidSt->refSignalPrev.y) / pidSt->deltaT;
//    pidSt->refSignalDot.z = (pidSt->refSignal.z - pidSt->refSignalPrev.z) / pidSt->deltaT;
//    pidSt->refSignalPrev.x = pidSt->refSignal.x;
//    pidSt->refSignalPrev.y = pidSt->refSignal.y;
//    pidSt->refSignalPrev.z = pidSt->refSignal.z;
//    PT1Filter(&pidSt->refSignalDotFiltered.x, pidSt->refSignalDot.x, pidSt->FFDTermC);
//    PT1Filter(&pidSt->refSignalDotFiltered.y, pidSt->refSignalDot.y, pidSt->FFDTermC);
//    PT1Filter(&pidSt->refSignalDotFiltered.z, pidSt->refSignalDot.z, pidSt->FFDTermC);
//    pidSt->FFout.x = pidSt->FFr.x / pidSt->FFrFactor * pidSt->refSignal.x + pidSt->FFdr.x / pidSt->FFdrFactor * pidSt->refSignalDotFiltered.x;
//    pidSt->FFout.y = pidSt->FFr.y / pidSt->FFrFactor * pidSt->refSignal.y + pidSt->FFdr.y / pidSt->FFdrFactor * pidSt->refSignalDotFiltered.y;
//    pidSt->FFout.z = pidSt->FFr.z / pidSt->FFrFactor * pidSt->refSignal.z + pidSt->FFdr.z / pidSt->FFdrFactor * pidSt->refSignalDotFiltered.z;
//
//    //error
//    pidSt->error.x = pidSt->refSignal.x - pidSt->sensor.signal.x;
//    pidSt->error.y = pidSt->refSignal.y - pidSt->sensor.signal.y;
//    pidSt->error.z = pidSt->refSignal.z - pidSt->sensor.signal.z;
//    //Pout
//    pidSt->Pout.x = pidSt->P.x / pidSt->PFactor * pidSt->error.x;
//    pidSt->Pout.y = pidSt->P.y / pidSt->PFactor * pidSt->error.y;
//    pidSt->Pout.z = pidSt->P.z / pidSt->PFactor * pidSt->error.z;
//
//    //I relax factor calc
//    axis relaxFactor;
//    relaxFactor.x = 1.0f;
//    relaxFactor.y = 1.0f;
//    relaxFactor.z = 1.0f;
//    calcIRelaxFactor(&relaxFactor, pidSt, twoWayswitch);
//
//    //errorSum
//    pidSt->errorSum.x += relaxFactor.x * pidSt->error.x * pidSt->deltaT;
//    pidSt->errorSum.y += relaxFactor.y * pidSt->error.y * pidSt->deltaT;
//    pidSt->errorSum.z += relaxFactor.z * pidSt->error.z * pidSt->deltaT;
//    //anti-windup of integral(saturation)
//    if (pidSt->errorSum.x > pidSt->saturationI) pidSt->errorSum.x = pidSt->saturationI;
//    else if (pidSt->errorSum.x < -pidSt->saturationI) pidSt->errorSum.x = -pidSt->saturationI;
//    else; //do nothing
//    if (pidSt->errorSum.y > pidSt->saturationI) pidSt->errorSum.y = pidSt->saturationI;
//    else if (pidSt->errorSum.y < -pidSt->saturationI) pidSt->errorSum.y = -pidSt->saturationI;
//    else; //do nothing
//    if (pidSt->errorSum.z > pidSt->saturationI) pidSt->errorSum.z = pidSt->saturationI;
//    else if (pidSt->errorSum.z < -pidSt->saturationI) pidSt->errorSum.z = -pidSt->saturationI;
//    else; //do nothin
//    //Iout
//    pidSt->Iout.x = pidSt->I.x / pidSt->IFactor * pidSt->errorSum.x;
//    pidSt->Iout.y = pidSt->I.y / pidSt->IFactor * pidSt->errorSum.y;
//    pidSt->Iout.z = pidSt->I.z / pidSt->IFactor * pidSt->errorSum.z;
//
//    //errorDelta(D), avoid kick
//    if (true == pidSt->sensor.newData)
//    {
//        pidSt->errorDot.x = (pidSt->sensor.signal.x - pidSt->sensorPrev.signal.x) / pidSt->deltaT;
//        pidSt->errorDot.y = (pidSt->sensor.signal.y - pidSt->sensorPrev.signal.y) / pidSt->deltaT;
//        pidSt->errorDot.z = (pidSt->sensor.signal.z - pidSt->sensorPrev.signal.z) / pidSt->deltaT;
//        pidSt->sensorPrev.signal.x = pidSt->sensor.signal.x;
//        pidSt->sensorPrev.signal.y = pidSt->sensor.signal.y;
//        pidSt->sensorPrev.signal.z = pidSt->sensor.signal.z;
//        //PT1 filtering D
//        PT1Filter(&pidSt->errorDotFiltered.x, pidSt->errorDot.x, pidSt->DTermC);
//        PT1Filter(&pidSt->errorDotFiltered.y, pidSt->errorDot.y, pidSt->DTermC);
//        PT1Filter(&pidSt->errorDotFiltered.z, pidSt->errorDot.z, pidSt->DTermC);
//
//        pidSt->sensor.newData = false;
//    }
//    else; //do nothing, keep previously calculated delta
//    //Dmax-dynamic
//    axis dDynamic;
//    calcDmaxFactor(&dDynamic, pidSt);
//    //Dout
//    pidSt->Dout.x = dDynamic.x / pidSt->DFactor * pidSt->errorDotFiltered.x;
//    pidSt->Dout.y = dDynamic.y / pidSt->DFactor * pidSt->errorDotFiltered.y;
//    pidSt->Dout.z = dDynamic.z / pidSt->DFactor * pidSt->errorDotFiltered.z;
//
//    //output(PID)
//    u->x = pidSt->FFout.x + pidSt->Pout.x + pidSt->Iout.x - pidSt->Dout.x;
//    u->y = pidSt->FFout.y + pidSt->Pout.y + pidSt->Iout.y - pidSt->Dout.y;
//    u->z = pidSt->FFout.z + pidSt->Pout.z + pidSt->Iout.z - pidSt->Dout.z;
//    pidSt->u.x = u->x;
//    pidSt->u.y = u->y;
//    pidSt->u.z = u->z;
//    //limit output(saturation)
//    if (u->x > pidSt->saturationPID) u->x = pidSt->saturationPID;
//    else if (u->x < -pidSt->saturationPID) u->x = -pidSt->saturationPID;
//    else; //do nothing
//    if (u->y > pidSt->saturationPID) u->y = pidSt->saturationPID;
//    else if (u->y < -pidSt->saturationPID) u->y = -pidSt->saturationPID;
//    else; //do nothing
//    if (u->z > pidSt->saturationPID) u->z = pidSt->saturationPID;
//    else if (u->z < -pidSt->saturationPID) u->z = -pidSt->saturationPID;
//    else; //do nothing
//
////    SerialUSB.print(u->x); SerialUSB.print("\t");
////        SerialUSB.print(refSignalDot.x); SerialUSB.print("\t");
//    //SerialUSB.print(u_ff.x); SerialUSB.print("\t");
////        SerialUSB.print(pidSt->error.x); SerialUSB.print("\t");
//    //    SerialUSB.print(pidSt->errorSum.x); SerialUSB.print("\t");
//        //SerialUSB.print(pidSt->error.x, 6); SerialUSB.print("\t");
//        //SerialUSB.print(pidSt->errorPrev.x, 6); SerialUSB.print("\t");
//        //SerialUSB.print(pidSt->deltaT, 6); SerialUSB.print("\t");
////        SerialUSB.print(pidSt->errorDelta.x); SerialUSB.print("\t");
////        SerialUSB.print(pidSt->errorDeltaFiltered.x); SerialUSB.print("\t");
//    //SerialUSB.print(pidSt->P.x / 10000.0f * pidSt->error.x); SerialUSB.print("\t");
//    //SerialUSB.println();
//}

void CalcPID_int(pid_st* pidSt, axis_i32* u)
{
    //scale up inputs by 1024
    ScalePIDinput_int(pidSt, 10);

    //FF
    CalcFeedforward_int(pidSt);
    
    //P
    CalcProportional_int(pidSt);

    //I relax
    //todo
    
    //I
    CalcIntegral_int(pidSt);

    //Dmax
    //todo

    //D
    CalcDerivative_int(pidSt);

    //PID
    CalcPIDoutput_int(pidSt, u);

    //scale down output
    ScalePIDoutput_int(u, 12);


    //  SerialUSB.print(float(pidSt->refSignal_int.x)); SerialUSB.print("\t");
    //  SerialUSB.print(float(pidSt->sensor.signalPT1_int.x)); SerialUSB.print("\t");
    // // SerialUSB.print(float(pidSt->error_int.x)); SerialUSB.print("\t");
    //  SerialUSB.print(float(pidSt->Pout_int.x)); SerialUSB.print("\t");
    // // SerialUSB.print(float(pidSt->errorSum_int.x)); SerialUSB.print("\t");
    //  SerialUSB.print(float(pidSt->Iout_int.x)); SerialUSB.print("\t");
    // // SerialUSB.print(float(pidSt->errorDot_int.x)); SerialUSB.print("\t");
    // // SerialUSB.print(float(pidSt->errorDotPT1_int.x)); SerialUSB.print("\t");
    //  SerialUSB.print(float(pidSt->Dout_int.x)); SerialUSB.print("\t");
    //  SerialUSB.print(float(pidSt->u_int.x)); SerialUSB.print("\t");
      //SerialUSB.print(float(u->x)); SerialUSB.print("\t");
      //SerialUSB.println();
}

void ScalePIDinput_int(pid_st* pid, uint8_t shift)
{
    pid->refSig_i.x <<= shift;
    pid->refSig_i.y <<= shift;
    pid->refSig_i.z <<= shift;
    pid->sensor.signalPT1.x <<= shift;
    pid->sensor.signalPT1.y <<= shift;
    pid->sensor.signalPT1.z <<= shift;
}

void CalcProportional_int(pid_st* pid)
{
    //error
    pid->error_i.x = pid->refSig_i.x - pid->sensor.signalPT1.x;
    pid->error_i.y = pid->refSig_i.y - pid->sensor.signalPT1.y;
    pid->error_i.z = pid->refSig_i.z - pid->sensor.signalPT1.z;
    //P
    //PFactor = 1024, 2x for scaling
    pid->Pout_i.x = (pid->P_i.x * pid->error_i.x) >> 9;
    pid->Pout_i.y = (pid->P_i.y * pid->error_i.y) >> 9;
    pid->Pout_i.z = (pid->P_i.z * pid->error_i.z) >> 9;
}

void CalcIntegral_int(pid_st* pid)
{
    //errorSum, shift due to tick
    int64_t x_i64 = (int64_t)pid->errorSum_i.x + (int64_t)pid->error_i.x;
    int64_t y_i64 = (int64_t)pid->errorSum_i.y + (int64_t)pid->error_i.y;
    int64_t z_i64 = (int64_t)pid->errorSum_i.z + (int64_t)pid->error_i.z;
    //anit-windup
    pid->errorSum_i.x = clamp_i64(x_i64, -pid->satI_i, pid->satI_i);
    pid->errorSum_i.y = clamp_i64(y_i64, -pid->satI_i, pid->satI_i);
    pid->errorSum_i.z = clamp_i64(z_i64, -pid->satI_i, pid->satI_i);
    //I
    //IFactor + 3x scaling, +10 due to tick
    pid->Iout_i.x = (int32_t)(((int64_t)pid->Ki_i.x * (int64_t)pid->errorSum_i.x) >> 30);
    pid->Iout_i.y = (int32_t)(((int64_t)pid->Ki_i.y * (int64_t)pid->errorSum_i.y) >> 30);
    pid->Iout_i.z = (int32_t)(((int64_t)pid->Ki_i.z * (int64_t)pid->errorSum_i.z) >> 30);
}

void CalcDerivative_int(pid_st* pid)
{
    //errorDot
    if (true == pid->sensor.newData)
    {
        pid->errorDot_i.x = pid->sensor.signalPT1.x - pid->signalPT1Prev_i.x;
        pid->errorDot_i.y = pid->sensor.signalPT1.y - pid->signalPT1Prev_i.y;
        pid->errorDot_i.z = pid->sensor.signalPT1.z - pid->signalPT1Prev_i.z;
        pid->signalPT1Prev_i.x = pid->sensor.signalPT1.x;
        pid->signalPT1Prev_i.y = pid->sensor.signalPT1.y;
        pid->signalPT1Prev_i.z = pid->sensor.signalPT1.z;

        //PT1 errorDot
        pid->errorDotPT1_i.x = PT1_133Hz(pid->errorDotPT1_i.x, pid->errorDot_i.x);
        pid->errorDotPT1_i.y = PT1_133Hz(pid->errorDotPT1_i.y, pid->errorDot_i.y);
        pid->errorDotPT1_i.z = PT1_133Hz(pid->errorDotPT1_i.z, pid->errorDot_i.z);

        //D
        //DFactor = 8192, + scaling
        pid->Dout_i.x = (int32_t)(((int64_t)pid->Kd_i.x * (int64_t)pid->errorDotPT1_i.x) >> 16);
        pid->Dout_i.y = (int32_t)(((int64_t)pid->Kd_i.y * (int64_t)pid->errorDotPT1_i.y) >> 16);
        pid->Dout_i.z = (int32_t)(((int64_t)pid->Kd_i.z * (int64_t)pid->errorDotPT1_i.z) >> 16);

        pid->sensor.newData = false;
    }
}

void CalcFeedforward_int(pid_st* pid)
{
    //refDot
    pid->refSigDot_i.x = pid->refSig_i.x - pid->refSigPrev_i.x;
    pid->refSigDot_i.y = pid->refSig_i.y - pid->refSigPrev_i.y;
    pid->refSigDot_i.z = pid->refSig_i.z - pid->refSigPrev_i.z;
    pid->refSigPrev_i.x = pid->refSig_i.x;
    pid->refSigPrev_i.y = pid->refSig_i.y;
    pid->refSigPrev_i.z = pid->refSig_i.z;
    //PT1 refDot
    pid->refSigDotPT1_i.x = PT1_133Hz(pid->refSigDotPT1_i.x, pid->refSigDot_i.x);
    pid->refSigDotPT1_i.y = PT1_133Hz(pid->refSigDotPT1_i.y, pid->refSigDot_i.y);
    pid->refSigDotPT1_i.z = PT1_133Hz(pid->refSigDotPT1_i.z, pid->refSigDot_i.z);
    //FF
    //FFrFactor  = 1024
    //FFdrFactor = 8192
    pid->FFout_i.x = ((pid->Kffr_i.x * pid->refSig_i.x) >> 10) + ((pid->Kffdr_i.x * pid->refSigDotPT1_i.x) >> 13);
    pid->FFout_i.y = ((pid->Kffr_i.y * pid->refSig_i.y) >> 10) + ((pid->Kffdr_i.y * pid->refSigDotPT1_i.y) >> 13);
    pid->FFout_i.z = ((pid->Kffr_i.z * pid->refSig_i.z) >> 10) + ((pid->Kffdr_i.z * pid->refSigDotPT1_i.z) >> 13);
}

void CalcPIDoutput_int(pid_st* pid, axis_i32* u)
{
    //PID
    pid->u_i.x = /*pid->FFout.x + */pid->Pout_i.x + pid->Iout_i.x - pid->Dout_i.x;
    pid->u_i.y = /*pid->FFout.y + */pid->Pout_i.y + pid->Iout_i.y - pid->Dout_i.y;
    pid->u_i.z = /*pid->FFout.z + */pid->Pout_i.z + pid->Iout_i.z - pid->Dout_i.z;
    //PID clamp    
    pid->u_i.x = clamp_i32(pid->u_i.x, -pid->satPID_i, pid->satPID_i);
    pid->u_i.y = clamp_i32(pid->u_i.y, -pid->satPID_i, pid->satPID_i);
    pid->u_i.z = clamp_i32(pid->u_i.z, -pid->satPID_i, pid->satPID_i);
    u->x = pid->u_i.x;
    u->y = pid->u_i.y;
    u->z = pid->u_i.z;
}

void ScalePIDoutput_int(axis_i32* u, uint8_t shift)
{
    u->x >>= shift;
    u->y >>= shift;
    u->z >>= shift;
}

inline int32_t clamp_i32(int32_t x, int32_t min, int32_t max)
{
    if (x > max) return max;
    if (x < min) return min;

    return x;
}

inline int64_t clamp_i64(int64_t x, int64_t min, int64_t max)
{
    if (x > max) return max;
    if (x < min) return min;

    return x;
}

pid_st* getPIDrates()
{
	return &pidRate;
}

pid_st* getPIDcascade()
{
    return &pidCascade;
}

gyroData_st* getGyroData()
{
    return &gyroData;
}    

accData_st* getAccData()
{
    return &accData;
}

void KalmanFilterAngle(kalmanFilterAngle_st* kf, const float accAngle, const float gyroIn, const float looptime)
{
    // Predict
    kf->rate = gyroIn - kf->bias;
    kf->angle += looptime * kf->rate;

    // Update error covariance matrix
    kf->P[0][0] += looptime * (looptime * kf->P[1][1] - kf->P[0][1] - kf->P[1][0] + accData.q_angle);
    kf->P[0][1] -= looptime * kf->P[1][1];
    kf->P[1][0] -= looptime * kf->P[1][1];
    kf->P[1][1] += accData.q_bias * looptime;

    // Compute Kalman gain
    double S = kf->P[0][0] + accData.r_measure;
    double K[2];
    K[0] = kf->P[0][0] / S;
    K[1] = kf->P[1][0] / S;

    // Update estimate with measurement
    double y = accAngle - kf->angle;
    kf->angle += K[0] * y;
    kf->bias += K[1] * y;

    // Update error covariance matrix
    double P00_temp = kf->P[0][0];
    double P01_temp = kf->P[0][1];

    kf->P[0][0] -= K[0] * P00_temp;
    kf->P[0][1] -= K[0] * P01_temp;
    kf->P[1][0] -= K[1] * P00_temp;
    kf->P[1][1] -= K[1] * P01_temp;
}

void ComplementryFilterAngle(float* yOut, const float accAngle, const float gyroIn, const float looptime, const float alpha)
{
   *yOut = alpha * (*yOut + gyroIn * looptime) + (1 - alpha) * accAngle;
}

//void ComplementryFilterAngleWeighted(float* yOut, const float accAngle, const float gyroIn, const float looptime, const float alpha, const axis* acc)
//{
//    float accMag = sqrt(acc->x * acc->x + acc->y * acc->y + acc->z * acc->z);
//    float w = 1.0f - (abs(accMag - 9.81f) / 9.81f);
//    if (w > 1) w = 1;
//    if (w < 0) w = 0;
//
//    *yOut = alpha * (*yOut + gyroIn * looptime) + (1.0 - alpha) * w * accAngle + (1.0 - w) * (*yOut);
//
//
//    SerialUSB.print(accMag, 3); SerialUSB.print('\t');
//    SerialUSB.print(w, 3); SerialUSB.print('\t');
//    SerialUSB.print((1.0 - alpha) * w * accAngle, 3); SerialUSB.print('\t');
//    SerialUSB.println((1.0 - w) * (*yOut), 3);
//}

float minVal(float value1, float value2)
{
    return (value1 < value2) ? value1 : value2;
}

float maxVal(float value1, float value2)
{
    return (value1 < value2) ? value2 : value1;
}
//
//void calcIRelaxFactor(axis* factor, pid_st* pidSt, uint16_t twoWaySwitch)
//{
//    if (twoWaySwitch < 1500)
//    {
//        if (abs(pidSt->refSignalDotFiltered.x) > pidSt->iRelaxRefThreshhold || abs(pidSt->error.x) > pidSt->iRelaxErrThreshhold) factor->x = 0.0f;
//        if (abs(pidSt->refSignalDotFiltered.y) > pidSt->iRelaxRefThreshhold || abs(pidSt->error.y) > pidSt->iRelaxErrThreshhold) factor->y = 0.0f;
//        if (abs(pidSt->refSignalDotFiltered.z) > pidSt->iRelaxRefThreshhold || abs(pidSt->error.z) > pidSt->iRelaxErrThreshhold) factor->z = 0.0f;
//    }
//    else
//    {
//        axis spNorm;
//        axis errNorm;
//        axis maxNorm;
//        //normalize values
//        spNorm.x = abs(pidSt->refSignalDotFiltered.x) / pidSt->iRelaxRefThreshhold;
//        spNorm.y = abs(pidSt->refSignalDotFiltered.y) / pidSt->iRelaxRefThreshhold;
//        spNorm.z = abs(pidSt->refSignalDotFiltered.z) / pidSt->iRelaxRefThreshhold;
//        errNorm.x = abs(pidSt->error.x) / pidSt->iRelaxErrThreshhold;
//        errNorm.y = abs(pidSt->error.y) / pidSt->iRelaxErrThreshhold;
//        errNorm.z = abs(pidSt->error.z) / pidSt->iRelaxErrThreshhold;
//        //find max
//        maxNorm.x = maxVal(spNorm.x, errNorm.x);
//        maxNorm.y = maxVal(spNorm.y, errNorm.y);
//        maxNorm.z = maxVal(spNorm.z, errNorm.z);
//        //constraint 0-1
//        maxNorm.x = minVal(maxNorm.x, 1);
//        maxNorm.y = minVal(maxNorm.y, 1);
//        maxNorm.z = minVal(maxNorm.z, 1);
//        //maxNorm.x = maxVal(0, maxNorm.x);
//        //maxNorm.y = maxVal(0, maxNorm.y);
//        //maxNorm.z = maxVal(0, maxNorm.z);
//        //factor
//        factor->x = 1.0f - maxNorm.x;
//        factor->y = 1.0f - maxNorm.y;
//        factor->z = 1.0f - maxNorm.z;
//    }
//}
//
//void calcDmaxFactor(axis* dDynamic, pid_st* pidSt)
//{
//    axis spNorm;
//    axis errNorm;
//    axis factor;
//    //normalize values
//    spNorm.x = abs(pidSt->refSignalDotFiltered.x) / pidSt->dMaxRefThreshhold;
//    spNorm.y = abs(pidSt->refSignalDotFiltered.y) / pidSt->dMaxRefThreshhold;
//    spNorm.z = abs(pidSt->refSignalDotFiltered.z) / pidSt->dMaxRefThreshhold;
//    errNorm.x = abs(pidSt->error.x) / pidSt->dMaxErrThreshhold;
//    errNorm.y = abs(pidSt->error.y) / pidSt->dMaxErrThreshhold;
//    errNorm.z = abs(pidSt->error.z) / pidSt->dMaxErrThreshhold;
//    //find max
//    factor.x = maxVal(spNorm.x, errNorm.x);
//    factor.y = maxVal(spNorm.y, errNorm.y);
//    factor.z = maxVal(spNorm.z, errNorm.z);
//    //constraint 0-1
//    factor.x = minVal(factor.x, 1);
//    factor.y = minVal(factor.y, 1);
//    factor.z = minVal(factor.z, 1);
//
//    dDynamic->x = pidSt->D.x + factor.x * (pidSt->Dmax.x - pidSt->D.x);
//    dDynamic->y = pidSt->D.y + factor.y * (pidSt->Dmax.y - pidSt->D.y);
//    dDynamic->z = pidSt->D.z + factor.z * (pidSt->Dmax.z - pidSt->D.z);
//}

float wobble(uint16_t pot1, uint16_t poti2)
{
    float piIncrement = float(pot1 - 1000) / 50000.0f * HALFPI; //0--pi/100
    uint16_t wobbleAplitude = (poti2 - 1000)/4;   //0-250

    wobbleTime += piIncrement;
    if (wobbleTime > TWOPI) wobbleTime -= TWOPI;

    uint8_t itr{ 0 };
    for (; itr < LUT_SIZE; itr++)
    {
        if (wobbleTime < sinTime[itr]) break;
    }

    uint8_t i = (itr >  0) ? itr-1 : 0;
    float frac = (wobbleTime - sinTime[i])/(sinTime[i+1] - sinTime[i]);
    if (frac < 0) frac = 0;

    float a = sinWave[i];
    float b = sinWave[i + 1];

    //linear interpolation
    float interpolatedSin = a + (b - a) * frac;

    return wobbleAplitude* interpolatedSin;
}

void setPIDParam(int32_t value, E_direction dir, E_pid pid)
{
    switch (pid)
    {
    case E_pid::I:
    {
        switch (dir)
        {
        case E_direction::X:
        {
            pidRate.I_i.x = value;
            pidRate.Ki_i.x = pidRate.I_i.x * pidRate.deltaTicks;
            break;
        }
        case E_direction::Y:
        {
            pidRate.I_i.y = value;
            pidRate.Ki_i.y = pidRate.I_i.y * pidRate.deltaTicks;
            break;
        }
        case E_direction::Z:
        {
            pidRate.I_i.z = value;
            pidRate.Ki_i.z = pidRate.I_i.z * pidRate.deltaTicks;
            break;
        }
        default:
        {
            break;
        }
        }
        break;
    }
    case E_pid::D:
    {
        switch (dir)
        {
        case E_direction::X:
        {
            pidRate.D_i.x = value;
            pidRate.Kd_i.x = pidRate.I_i.x * pidRate.deltaTicks;
            break;
        }
        case E_direction::Y:
        {
            pidRate.D_i.y = value;
            pidRate.Kd_i.y = pidRate.I_i.y * pidRate.deltaTicks;
            break;
        }
        case E_direction::Z:
        {
            pidRate.D_i.z = value;
            pidRate.Kd_i.z = pidRate.I_i.z * pidRate.deltaTicks;
            break;
        }
        default:
        {
            break;
        }
        }
        break;
    }
    default:
    {
        break;
    }
    }
}