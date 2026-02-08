//initial PID tuning
//increase D until its "restless"/shacky, lower back to stable, then take 0.75 of it
//increase P until its oscillates quickly, take 0.5 of it
//increase I until iss slow oscillates, take 0.5 of it
//now start to increase P then D maybe I until it has still good dinamic

#include "pch.h"

#include "Controller.h"
#include "RC.h"
#include <cmath>

#ifdef UNIT_TEST

#include "../test/helper/support4Testing.hpp"

extern DummySerial SerialUSB;

#else

#include "arduino.h"
#include "variant.h"

#endif

//element: 1th order(x), 3rd order(x^3), 5th order(x^5) 
float parabolicConst4Rate[3];
float maxYawRate = 500.0f;

E_armState armState{ DISARMED };

pid_st pidRate;
pid_st pidCascade;
gyroData_st gyroData;
accData_st accData;

void SetupController(void)
{
    //values comming from basic excel evaluation
    parabolicConst4Rate[0] = 0.1486979167f;         //1st order
    parabolicConst4Rate[1] = 0.000005052083f;       //3rd order
    parabolicConst4Rate[2] = 0.000000000007813f;    //5th order

    //RATE
    pidRate.P.x = 175.0f;
    pidRate.I.x = 25.0f;
    pidRate.D.x = 1900.0f;
    pidRate.P.y = 200.0f;
    pidRate.I.y = 25.0f;
    pidRate.D.y = 2100.0f;
    pidRate.P.z = 500.0f;
    pidRate.I.z = 150.0f;
    pidRate.D.z = 0.0f;
    pidRate.FFr.x = 0.0f;
    pidRate.FFr.y = 0.0f;
    pidRate.FFdr.x = 0.0f;
    pidRate.FFdr.y = 0.0f;
  	pidRate.saturationI = 7.0f;
  	pidRate.saturationPID = 75.0f;
  	pidRate.DTermC = 2000 / 200;	//datarate/filterrate = 2000hz/500hz
    pidRate.PFactor = 10000.0f;
    pidRate.IFactor = 100.0f;
    pidRate.DFactor = 1000000.0f;
    pidRate.FFrFactor = 1000.0f;
    pidRate.FFdrFactor = 10000.0f;

	//PID
	pidRate.errorSum.x = 0.0f;
	pidRate.errorSum.y = 0.0f;
	pidRate.errorSum.z = 0.0f;
	pidRate.errorPrev.x = 0.0f;
	pidRate.errorPrev.y = 0.0f;
	pidRate.errorPrev.z = 0.0f;
	pidRate.errorDotFiltered.x = 0.0f;
	pidRate.errorDotFiltered.y = 0.0f;
	pidRate.errorDotFiltered.z = 0.0f;

    //CASCADE
    pidCascade.P.x = 0.0f;	//55-60;
    pidCascade.I.x = 0.0f;
    pidCascade.D.x = 0.0f;	//110-114
    pidCascade.P.y = 0.0f;	//60
    pidCascade.I.y = 0.0f;
    pidCascade.D.y = 0.0f;   //132
    pidCascade.P.z = 0.0f;
    pidCascade.I.z = 0.0f;
    pidCascade.D.z = 0.0f;
    pidCascade.saturationI = 10.0f;
    pidCascade.saturationPID = 75.0f;
    pidCascade.DTermC = 1600 / 80;	//datarate/filterrate = 2000hz/500hz
    pidCascade.PFactor = 10.0f;
    pidCascade.IFactor = 1.0f;
    pidCascade.DFactor = 1.0f;
    pidCascade.FFrFactor = 1.0f;
    pidCascade.FFdrFactor = 1.0f;

    //PID
    pidCascade.errorSum.x = 0.0f;
    pidCascade.errorSum.y = 0.0f;
    pidCascade.errorSum.z = 0.0f;
    pidCascade.errorPrev.x = 0.0f;
    pidCascade.errorPrev.y = 0.0f;
    pidCascade.errorPrev.z = 0.0f;
    pidCascade.errorDotFiltered.x = 0.0f;
    pidCascade.errorDotFiltered.y = 0.0f;
    pidCascade.errorDotFiltered.z = 0.0f;

}

void RunController(const controllerIn_st* ctrlIn, controllerOut_st* ctrlOut)
{
    E_flightMode flightMode = EvalFlightMode(ctrlIn->rcSignals.flightModeSwitch);
    axis controlSignal;

    EvalArmState(&ctrlIn->rcSignals);

//    //debug PID
//    if (ctrlIn->rcSignals.Switch2Way > 1800)
//    {
//        if (ctrlIn->rcSignals.Switch3Way2 > 1800)
//        {
//            pidRate.D.x = LinearInterpol(ctrlIn->rcSignals.Poti1, 1000, 2000, 20, 200);
//            pidRate.D.y = LinearInterpol(ctrlIn->rcSignals.Poti2, 1000, 2000, 20, 200);
//        }
//        else if (ctrlIn->rcSignals.Switch3Way2 > 1300)
//        {
//            pidRate.I.x = LinearInterpol(ctrlIn->rcSignals.Poti1, 1000, 2000, 10, 70);
//            pidRate.I.y = LinearInterpol(ctrlIn->rcSignals.Poti2, 1000, 2000, 10, 70);
//        }
//        else
//        {
//            pidRate.P.x = LinearInterpol(ctrlIn->rcSignals.Poti1, 1000, 2000, 20, 100);
//            pidRate.P.y = LinearInterpol(ctrlIn->rcSignals.Poti2, 1000, 2000, 20, 100);
//        }
//    }
//    else
//    {
//    }

    //gyro filter
    if (true == ctrlIn->gyro.newData)
    {
        PT1Filter(&gyroData.PT1.signal.x, ctrlIn->gyro.signal.x, gyroData.paramC);  //2000/100 sampleRate/cutoffRate
        PT1Filter(&gyroData.PT1.signal.y, ctrlIn->gyro.signal.y, gyroData.paramC);
        PT1Filter(&gyroData.PT1.signal.z, ctrlIn->gyro.signal.z, gyroData.paramC);
    
        PT1Filter(&gyroData.PT2.signal.x, gyroData.PT1.signal.x, gyroData.paramC);
        PT1Filter(&gyroData.PT2.signal.y, gyroData.PT1.signal.y, gyroData.paramC);
        PT1Filter(&gyroData.PT2.signal.z, gyroData.PT1.signal.z, gyroData.paramC);
    
        //PT1Filter(&gyroData.PT3.signal.x, gyroData.PT2.signal.x, gyroData.paramC);
        //PT1Filter(&gyroData.PT3.signal.y, gyroData.PT2.signal.y, gyroData.paramC);
        //PT1Filter(&gyroData.PT3.signal.z, gyroData.PT2.signal.z, gyroData.paramC);
    
        //KalmanFilter(&gyroData.KF.x, ctrlIn->gyro.signal.x);
        //KalmanFilter(&gyroData.KF.y, ctrlIn->gyro.signal.y);
        //KalmanFilter(&gyroData.KF.z, ctrlIn->gyro.signal.z);
    }
    //acc filter
    if (true == ctrlIn->acc.newData)
    {
        PT1Filter(&accData.PT1.signal.x, ctrlIn->acc.signal.x, accData.paramC);
        PT1Filter(&accData.PT1.signal.y, ctrlIn->acc.signal.y, accData.paramC);
        PT1Filter(&accData.PT1.signal.z, ctrlIn->acc.signal.z, accData.paramC);

        PT1Filter(&accData.PT2.signal.x, accData.PT1.signal.x, accData.paramC);
        PT1Filter(&accData.PT2.signal.y, accData.PT1.signal.y, accData.paramC);
        PT1Filter(&accData.PT2.signal.z, accData.PT1.signal.z, accData.paramC);
    
        accData.rollAngle =  atan2(ctrlIn->acc.signal.y,  
            sqrt(ctrlIn->acc.signal.x * ctrlIn->acc.signal.x + ctrlIn->acc.signal.z * ctrlIn->acc.signal.z)) * 180 / 3.14;
        accData.pitchAngle = atan2(-ctrlIn->acc.signal.x, 
            sqrt(ctrlIn->acc.signal.y * ctrlIn->acc.signal.y + ctrlIn->acc.signal.z * ctrlIn->acc.signal.z)) * 180 / 3.14;
    
        accData.rollAnglePT1Acc = atan2(accData.PT1.signal.y,
            sqrt(accData.PT1.signal.x * accData.PT1.signal.x + accData.PT1.signal.z * accData.PT1.signal.z)) * 180 / 3.14;
        accData.pitchAnglePT1Acc = atan2(-accData.PT1.signal.x, 
            sqrt(accData.PT1.signal.y * accData.PT1.signal.y + accData.PT1.signal.z * accData.PT1.signal.z)) * 180 / 3.14;
  
        accData.rollAnglePT2Acc = atan2(accData.PT2.signal.y,
            sqrt(accData.PT2.signal.x * accData.PT2.signal.x + accData.PT2.signal.z * accData.PT2.signal.z)) * 180 / 3.14;
        accData.pitchAnglePT2Acc = atan2(-accData.PT2.signal.x, 
            sqrt(accData.PT2.signal.y * accData.PT2.signal.y + accData.PT2.signal.z * accData.PT2.signal.z)) * 180 / 3.14;

        //SerialUSB.print(ctrlIn->acc.signal.x); SerialUSB.print('\t');
        //SerialUSB.print(ctrlIn->acc.signal.y); SerialUSB.print('\t');
        //SerialUSB.print(ctrlIn->acc.signal.z); SerialUSB.print('\t');
        //SerialUSB.print(accData.rollAngle); SerialUSB.print('\t');
        //SerialUSB.println(accData.pitchAngle);
    }
    //kalman filter angle
    {
        KalmanFilterAngle(&accData.angleKF.roll, accData.rollAngle, ctrlIn->gyro.signal.x, ctrlIn->loopTime);
        KalmanFilterAngle(&accData.angleKF.pitch, accData.pitchAngle, ctrlIn->gyro.signal.y, ctrlIn->loopTime);
    
        KalmanFilterAngle(&accData.angleKFPT10.roll, accData.rollAnglePT1Acc, ctrlIn->gyro.signal.x, ctrlIn->loopTime);
        KalmanFilterAngle(&accData.angleKFPT10.pitch, accData.pitchAnglePT1Acc, ctrlIn->gyro.signal.y, ctrlIn->loopTime);

        KalmanFilterAngle(&accData.angleKFPT20.roll, accData.rollAnglePT2Acc, ctrlIn->gyro.signal.x, ctrlIn->loopTime);
        KalmanFilterAngle(&accData.angleKFPT20.pitch, accData.pitchAnglePT2Acc, ctrlIn->gyro.signal.y, ctrlIn->loopTime);
    
        KalmanFilterAngle(&accData.angleKFPT11.roll, accData.rollAnglePT1Acc, gyroData.PT1.signal.x, ctrlIn->loopTime);
        KalmanFilterAngle(&accData.angleKFPT11.pitch, accData.pitchAnglePT1Acc, gyroData.PT1.signal.y, ctrlIn->loopTime);

        KalmanFilterAngle(&accData.angleKFPT21.roll, accData.rollAnglePT2Acc, gyroData.PT1.signal.x, ctrlIn->loopTime);
        KalmanFilterAngle(&accData.angleKFPT21.pitch, accData.pitchAnglePT2Acc, gyroData.PT1.signal.y, ctrlIn->loopTime);

        KalmanFilterAngle(&accData.angleKFPT22.roll, accData.rollAnglePT2Acc, gyroData.PT2.signal.x, ctrlIn->loopTime);
        KalmanFilterAngle(&accData.angleKFPT22.pitch, accData.pitchAnglePT2Acc, gyroData.PT2.signal.y, ctrlIn->loopTime);
    }

    //control when armed and on high throttle
    if (ARMED == armState && ctrlIn->rcSignals.throttle > 1010)
    {
        float rollScaled{ ParabolicScale(ctrlIn->rcSignals.roll) };
        float pitchScaled{ ParabolicScale(ctrlIn->rcSignals.pitch) };
        float yawScaled{ LinearInterpol(ctrlIn->rcSignals.yaw, 1000u,2000u, -maxYawRate , maxYawRate) };

        switch (flightMode)
        {
        case PT2_WO_DERIVATIVE_KICK:
        {
            pidRate.refSignal.x =  rollScaled;
            pidRate.refSignal.y = -pitchScaled;
            pidRate.refSignal.z =  yawScaled;
            pidRate.sensor.signal.x = -gyroData.PT2.signal.x;    //x-y swap/+-1 due to orientation of IMU
            pidRate.sensor.signal.y =  gyroData.PT2.signal.y;
            pidRate.sensor.signal.z = -gyroData.PT2.signal.z;
            pidRate.sensor.newData = ctrlIn->gyro.newData;
            pidRate.deltaT = ctrlIn->loopTime;

            CalcPID_wo_Dkick(&pidRate, &controlSignal);

            break;
        }

        case PT2_WO_DERIV_KICK_FF:
        {
            pidRate.refSignal.x =  rollScaled;
            pidRate.refSignal.y = -pitchScaled;
            pidRate.refSignal.z =  yawScaled;
            pidRate.sensor.signal.x = -gyroData.PT2.signal.x;    //x-y swap/+-1 due to orientation of IMU
            pidRate.sensor.signal.y =  gyroData.PT2.signal.y;
            pidRate.sensor.signal.z = -gyroData.PT2.signal.z;
            pidRate.sensor.newData = ctrlIn->gyro.newData;
            pidRate.deltaT = ctrlIn->loopTime;

            CalcPID_wo_Dkick_FF(&pidRate, &controlSignal);

            break;
        }
        case PT1_WO_DERIVATIVE_KICK:
        {
            pidRate.refSignal.x =  rollScaled;
            pidRate.refSignal.y = -pitchScaled;
            pidRate.refSignal.z =  yawScaled;
            pidRate.sensor.signal.x = -gyroData.PT1.signal.x;    //x-y swap/+-1 due to orientation of IMU
            pidRate.sensor.signal.y =  gyroData.PT1.signal.y;
            pidRate.sensor.signal.z = -gyroData.PT1.signal.z;
            pidRate.sensor.newData = ctrlIn->gyro.newData;
            pidRate.deltaT = ctrlIn->loopTime;

            CalcPID_wo_Dkick(&pidRate, &controlSignal);

            break;
        }
        case KALMAN_FILTER:
        {
            break;
        }
        case ANGLE_CASCADE_CTRL:
        {
             float rollAngle{ LinearInterpol(ctrlIn->rcSignals.roll, 1000u,2000u, -30.0f , 30.0f) };
             float pitchAngle{ LinearInterpol(ctrlIn->rcSignals.pitch, 1000u,2000u, 30.0f , -30.0f) };
             axis intermidiateSignal;

             //outter cascade: angle
             pidCascade.refSignal.x = rollAngle;
             pidCascade.refSignal.y = pitchAngle;
             pidCascade.sensor.signal.x = -accData.angleKFPT10.roll.angle;
             pidCascade.sensor.signal.y = accData.angleKFPT10.pitch.angle;
             pidCascade.sensor.newData = false;  //no D term
             pidCascade.deltaT = ctrlIn->loopTime;

             CalcPID_wo_Dkick_FF(&pidCascade, &intermidiateSignal);

             //inner cascade: rate
             pidRate.refSignal.x = intermidiateSignal.x;
             pidRate.refSignal.y = intermidiateSignal.y;
             pidRate.refSignal.z = yawScaled;
             pidRate.sensor.signal.x = -gyroData.PT1.signal.x;    //x-y swap/+-1 due to orientation of IMU
             pidRate.sensor.signal.y = gyroData.PT1.signal.y;
             pidRate.sensor.signal.z = -gyroData.PT1.signal.z;
             pidRate.sensor.newData = ctrlIn->gyro.newData;
             pidRate.deltaT = ctrlIn->loopTime;

             CalcPID_wo_Dkick_FF(&pidRate, &controlSignal);
             
            break;
        }
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
      controlSignal.x = 0.0f;
      controlSignal.y = 0.0f;
      controlSignal.z = 0.0f;
      //reset RATE
      pidRate.error.x = 0.0f;
      pidRate.error.y = 0.0f;
      pidRate.error.z = 0.0f;
      pidRate.errorSum.x = 0.0f;
      pidRate.errorSum.y = 0.0f;
      pidRate.errorSum.z = 0.0f;
      pidRate.errorDot.x = 0.0f;
      pidRate.errorDot.y = 0.0f;
      pidRate.errorDot.z = 0.0f;
      pidRate.errorPrev.x = 0.0f;
      pidRate.errorPrev.y = 0.0f;
      pidRate.errorPrev.z = 0.0f;
      pidRate.errorDotFiltered.x = 0.0f;
      pidRate.errorDotFiltered.y = 0.0f;
      pidRate.errorDotFiltered.z = 0.0f;
      pidRate.sensorPrev.signal.x = 0.0f;
      pidRate.sensorPrev.signal.y = 0.0f;
      pidRate.sensorPrev.signal.z = 0.0f;
      //reset CASCADE
      pidCascade.error.x = 0.0f;
      pidCascade.error.y = 0.0f;
      pidCascade.error.z = 0.0f;
      pidCascade.errorSum.x = 0.0f;
      pidCascade.errorSum.y = 0.0f;
      pidCascade.errorSum.z = 0.0f;
      pidCascade.errorDot.x = 0.0f;
      pidCascade.errorDot.y = 0.0f;
      pidCascade.errorDot.z = 0.0f;
      pidCascade.errorPrev.x = 0.0f;
      pidCascade.errorPrev.y = 0.0f;
      pidCascade.errorPrev.z = 0.0f;
      pidCascade.errorDotFiltered.x = 0.0f;
      pidCascade.errorDotFiltered.y = 0.0f;
      pidCascade.errorDotFiltered.z = 0.0f;
      pidCascade.sensorPrev.signal.x = 0.0f;
      pidCascade.sensorPrev.signal.y = 0.0f;
      pidCascade.sensorPrev.signal.z = 0.0f;

    }

    ctrlOut->U.x = controlSignal.x;
    ctrlOut->U.y = controlSignal.y;
    ctrlOut->U.z = controlSignal.z;
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
        return PT1_WO_DERIVATIVE_KICK;
    }
    else
    {
        return PT2_WO_DERIVATIVE_KICK;
    }
}

float ParabolicScale(const uint16_t channel)
{
    float shiftedChannel = float(channel) - 1500.0f;

    return float(shiftedChannel) * parabolicConst4Rate[0] + float(pow(shiftedChannel, 3)) * parabolicConst4Rate[1] + float(pow(shiftedChannel, 5)) * parabolicConst4Rate[2];
}

float LinearInterpol(const uint16_t xn, const uint16_t x0, const uint16_t x1, const float y0, const float y1)
{
    float x10 = float(x1 - x0);
    float xn0 = float(xn - x0);
    float y10 = y1 - y0;

    if (0 == x10)
    {
        return y0;
    }
    else if (0 == y10)
    {
        return y0;
    }
    else
    {
        return xn0 * y10 / x10 + y0;
    }
}

// param C = dataRate/cutoffFreq
void PT1Filter(float* yOut, float xIn, float paramC)
{
	*yOut = (xIn + paramC * (*yOut)) / (paramC + 1);
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

void CalcPID_wo_Dkick(pid_st* pidSt, axis* u)
{
    //error
    pidSt->error.x = pidSt->refSignal.x - pidSt->sensor.signal.x;
    pidSt->error.y = pidSt->refSignal.y - pidSt->sensor.signal.y;
    pidSt->error.z = pidSt->refSignal.z - pidSt->sensor.signal.z;
    //Pout
    pidSt->Pout.x = pidSt->P.x / pidSt->PFactor * pidSt->error.x;
    pidSt->Pout.y = pidSt->P.y / pidSt->PFactor * pidSt->error.y;
    pidSt->Pout.z = pidSt->P.z / pidSt->PFactor * pidSt->error.z;

    //errorSum
    pidSt->errorSum.x += pidSt->error.x * pidSt->deltaT;
    pidSt->errorSum.y += pidSt->error.y * pidSt->deltaT;
    pidSt->errorSum.z += pidSt->error.z * pidSt->deltaT;
    //anti-windup of integral(saturation)
    if (pidSt->errorSum.x > pidSt->saturationI) pidSt->errorSum.x = pidSt->saturationI;
    else if (pidSt->errorSum.x < -pidSt->saturationI) pidSt->errorSum.x = -pidSt->saturationI;
    else; //do nothing
    if (pidSt->errorSum.y > pidSt->saturationI) pidSt->errorSum.y = pidSt->saturationI;
    else if (pidSt->errorSum.y < -pidSt->saturationI) pidSt->errorSum.y = -pidSt->saturationI;
    else; //do nothing
    if (pidSt->errorSum.z > pidSt->saturationI) pidSt->errorSum.z = pidSt->saturationI;
    else if (pidSt->errorSum.z < -pidSt->saturationI) pidSt->errorSum.z = -pidSt->saturationI;
    else; //do nothing	
    //Iout
    pidSt->Iout.x = pidSt->I.x / pidSt->IFactor * pidSt->errorSum.x;
    pidSt->Iout.y = pidSt->I.y / pidSt->IFactor * pidSt->errorSum.y;
    pidSt->Iout.z = pidSt->I.z / pidSt->IFactor * pidSt->errorSum.z;

    //errorDelta, avoid kick
    if (true == pidSt->sensor.newData)
    {
        pidSt->errorDot.x = (pidSt->sensor.signal.x - pidSt->sensorPrev.signal.x) / pidSt->deltaT;
        pidSt->errorDot.y = (pidSt->sensor.signal.y - pidSt->sensorPrev.signal.y) / pidSt->deltaT;
        pidSt->errorDot.z = (pidSt->sensor.signal.z - pidSt->sensorPrev.signal.z) / pidSt->deltaT;
        pidSt->sensorPrev.signal.x = pidSt->sensor.signal.x;
        pidSt->sensorPrev.signal.y = pidSt->sensor.signal.y;
        pidSt->sensorPrev.signal.z = pidSt->sensor.signal.z;
        //PT1 filtering D
        PT1Filter(&pidSt->errorDotFiltered.x, pidSt->errorDot.x, pidSt->DTermC);
        PT1Filter(&pidSt->errorDotFiltered.y, pidSt->errorDot.y, pidSt->DTermC);
        PT1Filter(&pidSt->errorDotFiltered.z, pidSt->errorDot.z, pidSt->DTermC);

        pidSt->sensor.newData = false;
    }
    else; //do nothing, keep previously calculated delta
    //Dout
    pidSt->Dout.x = pidSt->D.x / pidSt->DFactor * pidSt->errorDotFiltered.x;
    pidSt->Dout.y = pidSt->D.y / pidSt->DFactor * pidSt->errorDotFiltered.y;
    pidSt->Dout.z = pidSt->D.z / pidSt->DFactor * pidSt->errorDotFiltered.z;

    //output(PID)
	pidSt->u.x = pidSt->Pout.x + pidSt->Iout.x - pidSt->Dout.x;
	pidSt->u.y = pidSt->Pout.y + pidSt->Iout.y - pidSt->Dout.y;
	pidSt->u.z = pidSt->Pout.z + pidSt->Iout.z - pidSt->Dout.z;
    u->x = pidSt->u.x;
    u->y = pidSt->u.y;
    u->z = pidSt->u.z;
    //limit output(saturation)
    if (u->x > pidSt->saturationPID) u->x = pidSt->saturationPID;
    else if (u->x < -pidSt->saturationPID) u->x = -pidSt->saturationPID;
    else; //do nothing
    if (u->y > pidSt->saturationPID) u->y = pidSt->saturationPID;
    else if (u->y < -pidSt->saturationPID) u->y = -pidSt->saturationPID;
    else; //do nothing
    if (u->z > pidSt->saturationPID) u->z = pidSt->saturationPID;
    else if (u->z < -pidSt->saturationPID) u->z = -pidSt->saturationPID;
    else; //do nothing

//    SerialUSB.print(u->x); SerialUSB.print("\t");
    //    SerialUSB.print(pidSt->error.x); SerialUSB.print("\t");
    //    SerialUSB.print(pidSt->errorSum.x); SerialUSB.print("\t");
        //SerialUSB.print(pidSt->error.x, 6); SerialUSB.print("\t");
        //SerialUSB.print(pidSt->errorPrev.x, 6); SerialUSB.print("\t");
        //SerialUSB.print(pidSt->deltaT, 6); SerialUSB.print("\t");
//        SerialUSB.print(pidSt->errorDelta.x); SerialUSB.print("\t");
//        SerialUSB.print(pidSt->errorDeltaFiltered.x); SerialUSB.print("\t");
//    SerialUSB.println();
}

void CalcPID_wo_Dkick_FF(pid_st* pidSt, axis* u)
{
    //feedforward(FF)
    pidSt->refSignalDot.x = (pidSt->refSignal.x - pidSt->refSignalPrev.x) / pidSt->deltaT;
    pidSt->refSignalDot.y = (pidSt->refSignal.y - pidSt->refSignalPrev.y) / pidSt->deltaT;
    pidSt->refSignalDot.z = (pidSt->refSignal.z - pidSt->refSignalPrev.z) / pidSt->deltaT;
    pidSt->refSignalPrev.x = pidSt->refSignal.x;
    pidSt->refSignalPrev.y = pidSt->refSignal.y;
    pidSt->refSignalPrev.z = pidSt->refSignal.z;    
    PT1Filter(&pidSt->refSignalDotFiltered.x, pidSt->refSignalDot.x, 10.0f);
    PT1Filter(&pidSt->refSignalDotFiltered.y, pidSt->refSignalDot.y, 10.0f);
    PT1Filter(&pidSt->refSignalDotFiltered.z, pidSt->refSignalDot.z, 10.0f);
    pidSt->FFout.x = pidSt->FFr.x / pidSt->FFrFactor * pidSt->refSignal.x + pidSt->FFdr.x / pidSt->FFdrFactor * pidSt->refSignalDotFiltered.x;
    pidSt->FFout.y = pidSt->FFr.y / pidSt->FFrFactor * pidSt->refSignal.y + pidSt->FFdr.y / pidSt->FFdrFactor * pidSt->refSignalDotFiltered.y;
    pidSt->FFout.z = pidSt->FFr.z / pidSt->FFrFactor * pidSt->refSignal.z + pidSt->FFdr.z / pidSt->FFdrFactor * pidSt->refSignalDotFiltered.z;

    //error
    pidSt->error.x = pidSt->refSignal.x - pidSt->sensor.signal.x;
    pidSt->error.y = pidSt->refSignal.y - pidSt->sensor.signal.y;
    pidSt->error.z = pidSt->refSignal.z - pidSt->sensor.signal.z;
    //Pout
    pidSt->Pout.x = pidSt->P.x / pidSt->PFactor * pidSt->error.x;
    pidSt->Pout.y = pidSt->P.y / pidSt->PFactor * pidSt->error.y;
    pidSt->Pout.z = pidSt->P.z / pidSt->PFactor * pidSt->error.z;

    //errorSum
    pidSt->errorSum.x += pidSt->error.x * pidSt->deltaT;
    pidSt->errorSum.y += pidSt->error.y * pidSt->deltaT;
    pidSt->errorSum.z += pidSt->error.z * pidSt->deltaT;
    //anti-windup of integral(saturation)
    if (pidSt->errorSum.x > pidSt->saturationI) pidSt->errorSum.x = pidSt->saturationI;
    else if (pidSt->errorSum.x < -pidSt->saturationI) pidSt->errorSum.x = -pidSt->saturationI;
    else; //do nothing
    if (pidSt->errorSum.y > pidSt->saturationI) pidSt->errorSum.y = pidSt->saturationI;
    else if (pidSt->errorSum.y < -pidSt->saturationI) pidSt->errorSum.y = -pidSt->saturationI;
    else; //do nothing
    if (pidSt->errorSum.z > pidSt->saturationI) pidSt->errorSum.z = pidSt->saturationI;
    else if (pidSt->errorSum.z < -pidSt->saturationI) pidSt->errorSum.z = -pidSt->saturationI;
    else; //do nothin
    //Iout
    pidSt->Iout.x = pidSt->I.x / pidSt->IFactor * pidSt->errorSum.x;
    pidSt->Iout.y = pidSt->I.y / pidSt->IFactor * pidSt->errorSum.y;
    pidSt->Iout.z = pidSt->I.z / pidSt->IFactor * pidSt->errorSum.z;

    //errorDelta(D), avoid kick
    if (true == pidSt->sensor.newData)
    {
        pidSt->errorDot.x = (pidSt->sensor.signal.x - pidSt->sensorPrev.signal.x) / pidSt->deltaT;
        pidSt->errorDot.y = (pidSt->sensor.signal.y - pidSt->sensorPrev.signal.y) / pidSt->deltaT;
        pidSt->errorDot.z = (pidSt->sensor.signal.z - pidSt->sensorPrev.signal.z) / pidSt->deltaT;
        pidSt->sensorPrev.signal.x = pidSt->sensor.signal.x;
        pidSt->sensorPrev.signal.y = pidSt->sensor.signal.y;
        pidSt->sensorPrev.signal.z = pidSt->sensor.signal.z;
        //PT1 filtering D
        PT1Filter(&pidSt->errorDotFiltered.x, pidSt->errorDot.x, pidSt->DTermC);
        PT1Filter(&pidSt->errorDotFiltered.y, pidSt->errorDot.y, pidSt->DTermC);
        PT1Filter(&pidSt->errorDotFiltered.z, pidSt->errorDot.z, pidSt->DTermC);

        pidSt->sensor.newData = false;
    }
    else; //do nothing, keep previously calculated delta
    //Dout
    pidSt->Dout.x = pidSt->D.x / pidSt->DFactor * pidSt->errorDotFiltered.x;
    pidSt->Dout.y = pidSt->D.y / pidSt->DFactor * pidSt->errorDotFiltered.y;
    pidSt->Dout.z = pidSt->D.z / pidSt->DFactor * pidSt->errorDotFiltered.z;


    //output(PID)
    u->x = pidSt->FFout.x + pidSt->Pout.x + pidSt->Iout.x - pidSt->Dout.x;
    u->y = pidSt->FFout.y + pidSt->Pout.y + pidSt->Iout.y - pidSt->Dout.y;
    u->z = pidSt->FFout.z + pidSt->Pout.z + pidSt->Iout.z - pidSt->Dout.z;
    //limit output(saturation)
    if (u->x > pidSt->saturationPID) u->x = pidSt->saturationPID;
    else if (u->x < -pidSt->saturationPID) u->x = -pidSt->saturationPID;
    else; //do nothing
    if (u->y > pidSt->saturationPID) u->y = pidSt->saturationPID;
    else if (u->y < -pidSt->saturationPID) u->y = -pidSt->saturationPID;
    else; //do nothing
    if (u->z > pidSt->saturationPID) u->z = pidSt->saturationPID;
    else if (u->z < -pidSt->saturationPID) u->z = -pidSt->saturationPID;
    else; //do nothing

//    SerialUSB.print(u->x); SerialUSB.print("\t");
//        SerialUSB.print(refSignalDot.x); SerialUSB.print("\t");
    //SerialUSB.print(u_ff.x); SerialUSB.print("\t");
//        SerialUSB.print(pidSt->error.x); SerialUSB.print("\t");
    //    SerialUSB.print(pidSt->errorSum.x); SerialUSB.print("\t");
        //SerialUSB.print(pidSt->error.x, 6); SerialUSB.print("\t");
        //SerialUSB.print(pidSt->errorPrev.x, 6); SerialUSB.print("\t");
        //SerialUSB.print(pidSt->deltaT, 6); SerialUSB.print("\t");
//        SerialUSB.print(pidSt->errorDelta.x); SerialUSB.print("\t");
//        SerialUSB.print(pidSt->errorDeltaFiltered.x); SerialUSB.print("\t");
    //SerialUSB.print(pidSt->P.x / 10000.0f * pidSt->error.x); SerialUSB.print("\t");
    //SerialUSB.println();
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

void KalmanFilter(kalmanfilter_st* kf, float xIn)
{
    //prediction update
    kf->p = kf->p + gyroData.KF.q;

    //mesurement update
    kf->k = kf->p / (kf->p + gyroData.KF.r);
    kf->value = kf->value + kf->k * (xIn - kf->value);
    kf->p = (1 - kf->k) * kf->p;
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
