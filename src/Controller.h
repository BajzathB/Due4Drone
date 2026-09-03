// Header to configure regulate and calculate controll loop, for example PID

#ifndef CONTROLLER_HEADER
#define CONTROLLER_HEADER

#include "stdint.h"
#include "SPI.h"
#include "RC.h"
#include "sysTime.h"

typedef enum E_flightMode : uint8_t
{
    RATE_CTRL_PT1 = 0,
    RATE_CTRL_PT1_IRelax_Dmax= 1,
    ANGLE_CASCADE_CTRL = 2,
    GPS_CTRL = 3,
    RATE_CTRL_INT = 4,
};

typedef struct controllerIn_st 
{
	sigOut gyro;
	sigOut acc;
  rcSignals_st rcSignals;
}controllerIn_st;

typedef struct controllerOut_st 
{
    axis_i32 U_i;
    E_armState armState{DISARMED};
}controllerOut_st;

typedef struct pid_st
{
    sigOut sensor;
    axis_i32 error_i, errorDot_i, errorPrev_i, errorDotPT1_i;
    axis_i64 errorSum_i;
    axis_i32 refSig_i, refSigPrev_i, refSigDot_i, refSigDotPT1_i;
    axis_i32 P_i, I_i, D_i, Dmax_i, FFr_i, FFdr_i, u_i;
    axis_i32 Ki_i, Kd_i, Kffr_i, Kffdr_i;
    axis_i32 Pout_i, Iout_i, Dout_i, FFout_i;
    int32_t satI_i, satPID_i;
    axis_i32 signalPT1Prev_i;
    axis_i32 iRelaxWeight;

    const int32_t deltaTicks{ 5250 };  //0.5ms
    const int32_t inverseDt{ 2000 };  //2kHz

    int32_t iRelaxRefThold_i;
    int32_t iRelaxErrThold_i;
    int32_t dMaxRefThold_i;
    int32_t dMaxErrThold_i;
}pid_st;

typedef struct gyroData_st
{
    sigOut PT1;
};

// Kalman filter struct
typedef struct
{
    int32_t angle;      // internal angle units (30� = 78747)
    int32_t bias;       // gyro bias (raw gyro counts)

    int64_t P00{1024};
    int64_t P01;
    int64_t P10;
    int64_t P11{1024};
}kfAngle_st;

typedef struct
{
    kfAngle_st roll;
    kfAngle_st pitch;

    int32_t qAngleTick{ 10 };
    int32_t qBiasTick{ 1 };
    int32_t rMeasTick{ 1000 };
}kfAngle2d_st;

typedef struct 
{
    double angle{ 0.0 };     // The angle calculated by the Kalman filter
    double bias{ 0.0 };      // The gyro bias calculated by the Kalman filter
    double rate{ 0.0 };      // Unbiased rate

    double P[2][2];   // Error covariance matrix
} kalmanFilterAngle_st;
//
//typedef struct 
//{
//    kalmanFilterAngle_st roll;
//    kalmanFilterAngle_st pitch;
//}kalmanFilterAngle3d_st;

typedef struct accData_st
{
    int32_t rollPT1_i;
    int32_t pitchPT1_i;

    int32_t accumulatedGyroRoll_i;

    kfAngle2d_st angleKF;



    //float rollAnglePT1Acc;
    //float pitchAnglePT1Acc;
    //float rollAnglePT2Acc;
    //float pitchAnglePT2Acc;

    float alpha{0.995};
    //float rollAngleCF;
    //float pitchAngleCF;
    //float rollAngleCF10;
    //float pitchAngleCF10;
    //float rollAngleCF11;
    //float pitchAngleCF11;
    //float rollAngleCFw;
    //float pitchAngleCFw;
    //float rollAngleCFw01;
    //float pitchAngleCFw01;

    double q_angle{ 0.1 };   // Process noise variance for angle
    double q_bias{ 0.003 };    // Process noise variance for gyro bias
    double r_measure{ 20.0 }; // Measurement noise variance
    //kalmanFilterAngle3d_st angleKF;
    //kalmanFilterAngle3d_st angleKFPT10;


};


// Method to set initial values for controller variable
void SetupController(void);

// Method to set up timer for control loop
void SetupCtrlLoopTimer(void);

void ControllerDebug(void);

// Method to run controller logic cyclically
void RunController(const controllerIn_st* ctrlIn, controllerOut_st* ctrlOut);

// Function to evaluate the arm status based on switch state
E_armState EvalArmState(const rcSignals_st* rcSig);

// Function to evaluate the flight mode state based on switch state
E_flightMode EvalFlightMode(const uint16_t flightModeChannel);

// Function to calculate expo scaling for stick value
inline int32_t expo(const uint16_t channel);

// Function to optimally linear interpolate to ~-8192-8192
inline int32_t linearScale_8192(uint16_t ch);

// Method to calculate PID "u" output based on "pidSt" input, avoiding derivative kick
//void CalcPID_wo_Dkick_FF_IRelax_Dmax(pid_st* pidSt, axis* u, uint16_t twoWayswitch);

// Method to calc PID integer based
void CalcPID_int(pid_st* pidSt, axis_i32* u);

// Method to scale input up
void ScalePIDinput_int(pid_st* pid, uint8_t shift);

// Method to calc PID proportional part
void CalcProportional_int(pid_st* pid);

// Method to calc PID integral part
void CalcIntegral_int(pid_st* pid);

// Method to calc PID derivative part
void CalcDerivative_int(pid_st* pid);

// Method to calc PID feedforward part
void CalcFeedforward_int(pid_st* pid);

// Method to calc PID feedforward part
void CalcPIDoutput_int(pid_st* pid, axis_i32* u);

// Method to scale output down
void ScalePIDoutput_int(axis_i32* u, uint8_t shift);

// Function to return PID rate values
pid_st* getPIDrates();

// Function to return PID cascade outter values
pid_st* getPIDcascade();

// Function to return gyro data values
gyroData_st* getGyroData();

// Function to return acc data values
accData_st* getAccData();

//Method to calculate angle from accelerometer data
int32_t CalcAccAngle(const int32_t numerator, const int32_t denominator1, const int32_t denominator2);

//Method to calculate kalman filter angle from gyro and acc sources
void CalcKFAngle(kfAngle_st* kf, const int32_t accAngle, const int32_t gyro, const int32_t ticks);

void KalmanFilterAngle(kalmanFilterAngle_st* kf, const float accAngle, const float gyroIn, const float looptime);

//Method to calculate complementary filter of acc angle
//void ComplementryFilterAngle(float* yOut, const float accAngle, const float gyroIn, const float looptime, const float alpha);

//Method to calculate weighted complementary filter of acc angle
//void ComplementryFilterAngleWeighted(float* yOut, const float accAngle, const float gyroIn, const float looptime, const float alpha, const axis* acc);

// Function to return lower value
float minVal(float value1, float value2);
int32_t minVal(int32_t value1, int32_t value2);

// Function to return higher value
float maxVal(float value1, float value2);

// Method to calculate I relax factor value
void calcIRelaxWeight(pid_st* pidSt);

// Method to calculate Dmax factor
//void calcDmaxFactor(axis* dDynamic, pid_st* pidSt);

// Function to claculate the wobble amplitude value
float wobble(uint16_t pot1, uint16_t poti2);

// Function to clamp value inbetween bounds
inline int32_t clamp_i32(int32_t x, int32_t min, int32_t max);

void setPIDParam(int32_t value, E_pid pid, E_direction dir);

#endif // !CONTROLLER_HEADER
