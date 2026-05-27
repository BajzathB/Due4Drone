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
    droneTimes_st droneTimes;
}controllerIn_st;

typedef struct controllerOut_st 
{
    axis U;
    axis_i32 U_int;
    E_armState armState{DISARMED};
}controllerOut_st;

typedef struct pid_st
{
    //axis P, I, D, Dmax, FFr, FFdr;
    //axis error, errorSum, errorDot, errorPrev, errorDotFiltered;
    //axis Pout, Iout, Dout, FFout;
    //axis refSignal, refSignalPrev, refSignalDot, refSignalDotFiltered;
    //axis u;
    //float saturationI, saturationPID, DTermC, FFDTermC;
    //float iRelaxRefThreshhold;
    //float iRelaxErrThreshhold;
    //float dMaxRefThreshhold;
    //float dMaxErrThreshhold;
    sigOut sensor;
    //sigOut sensorPrev;
	//float deltaT{0.1f};
    //float PFactor, IFactor, DFactor, FFrFactor, FFdrFactor;

    axis_i32 error_int, errorSum_int, errorDot_int, errorPrev_int, errorDotPT1_int;
    axis_i32 refSignal_int, refSignalPrev_int, refSignalDot_int, refSignalDotPT1_int;
    axis_i32 P_int, I_int, D_int, Dmax_int, FFr_int, FFdr_int, u_int;
    axis_i32 Pout_int, Iout_int, Dout_int, FFout_int;
    uint32_t deltaTicks{ 1 };
    int32_t inverseDt{1};
    int32_t satI_int, satPID_int;
    axis_i32 signalPT1Prev_int;
    uint64_t runCtrlTickPrev{ 0 };
}pid_st;

// Kalman Filter Struct
typedef struct kalmanfilter_st 
{
    float value{0.0f};  // Estimated value
    float p{1.0f};  // Estimation error covariance
    float k{0.0f};  // Kalman gain
} kalmanfilter_st;

typedef struct kalmanfilter3d_st 
{
    kalmanfilter_st x;
    kalmanfilter_st y;
    kalmanfilter_st z;
    float q{ 0.001f };  // Process noise covariance
    float r{ 4.0f };  // Measurement noise covariance
} kalmanfilter3d_st;

typedef struct gyroData_st
{
    float paramC{ 10 };     //2000/x sampleRate/cutoffRate
    sigOut PT1;
    sigOut PT2;
    sigOut PT3;
    kalmanfilter3d_st KF;
};

// Kalman filter struct
typedef struct 
{
    double angle{ 0.0 };     // The angle calculated by the Kalman filter
    double bias{ 0.0 };      // The gyro bias calculated by the Kalman filter
    double rate{ 0.0 };      // Unbiased rate

    double P[2][2];   // Error covariance matrix
} kalmanFilterAngle_st;

typedef struct 
{

    kalmanFilterAngle_st roll;
    kalmanFilterAngle_st pitch;
}kalmanFilterAngle3d_st;

typedef struct accData_st
{
    float paramC{ 64 };     //1600/x sampleRate/cutoffRate
    sigOut PT1;
    sigOut PT2;
    float rollAngle;
    float pitchAngle;
    float rollAnglePT1Acc;
    float pitchAnglePT1Acc;
    float rollAnglePT2Acc;
    float pitchAnglePT2Acc;

    float alpha{0.995};
    float rollAngleCF;
    float pitchAngleCF;
    float rollAngleCF10;
    float pitchAngleCF10;
    float rollAngleCF11;
    float pitchAngleCF11;
    float rollAngleCFw;
    float pitchAngleCFw;
    float rollAngleCFw01;
    float pitchAngleCFw01;

    double q_angle{ 0.0001 };   // Process noise variance for angle
    double q_bias{ 0.003 };    // Process noise variance for gyro bias
    double r_measure{ 50.0 }; // Measurement noise variance
    kalmanFilterAngle3d_st angleKF;
    kalmanFilterAngle3d_st angleKFPT10;
    kalmanFilterAngle3d_st angleKFPT20;
    kalmanFilterAngle3d_st angleKFPT11;
    kalmanFilterAngle3d_st angleKFPT21;
    kalmanFilterAngle3d_st angleKFPT22;

};


// Method to set initial values for controller variable
void SetupController(void);

// Method to run controller logic cyclically
void RunController(const controllerIn_st* ctrlIn, controllerOut_st* ctrlOut);

// Function to evaluate the arm status based on switch state
void EvalArmState(const rcSignals_st* rcSig);

// Function to evaluate the flight mode state based on switch state
E_flightMode EvalFlightMode(const uint16_t flightModeChannel);

// Function to calculate parabolical scaling of stick value
float ParabolicScale(const uint16_t channel);

// Function to calculate expo scaling for stick value
inline int32_t expo(const uint16_t channel);

// Function to linear interpolate stick value
float LinearInterpol(const uint16_t xn, const uint16_t x0, const uint16_t x1, const float y0, const float y1);

// Function to optimally linear interpolate to ~-8192-8192
inline int32_t linearScale_8192(uint16_t ch);

// Method to calculate low-pass filtered value of a signal
//void PT1Filter(float* yOut, const float xIn, const float paramC );

// Method to calculate PID "u" output based on "pidSt" input, avoiding derivative kick
//void CalcPID_wo_Dkick(pid_st* pidSt, axis* u);

// Method to calculate PID "u" output based on "pidSt" input, avoiding derivative kick
void CalcPID_wo_Dkick_FF(pid_st* pidSt, axis* u);

// Method to calculate PID "u" output based on "pidSt" input, avoiding derivative kick
void CalcPID_wo_Dkick_FF_IRelax_Dmax(pid_st* pidSt, axis* u, uint16_t twoWayswitch);

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

// Method to calculate kalman filter of a signal
void KalmanFilter(kalmanfilter_st* kf, float xIn);

//Method to calculate kalman filter of acc signal
void KalmanFilterAngle(kalmanFilterAngle_st* kf, const float accAngle, const float gyroIn, const float looptime);

//Method to calculate complementary filter of acc angle
void ComplementryFilterAngle(float* yOut, const float accAngle, const float gyroIn, const float looptime, const float alpha);

//Method to calculate weighted complementary filter of acc angle
//void ComplementryFilterAngleWeighted(float* yOut, const float accAngle, const float gyroIn, const float looptime, const float alpha, const axis* acc);

// Function to return lower value
float minVal(float value1, float value2);

// Function to return higher value
float maxVal(float value1, float value2);

// Method to calculate I relax factor value
void calcIRelaxFactor(axis* factor, pid_st* pidSt, uint16_t twoWaySwitch);

// Method to calculate Dmax factor
void calcDmaxFactor(axis* dDynamic, pid_st* pidSt);

// Function to claculate the wobble amplitude value
float wobble(uint16_t pot1, uint16_t poti2);

// Function to clamp value inbetween bounds
inline int32_t clamp_i32(int32_t x, int32_t min, int32_t max);
inline int64_t clamp_i64(int64_t x, int64_t min, int64_t max);

#endif // !CONTROLLER_HEADER
