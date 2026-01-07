// Header to configure regulate and calculate controll loop, for example PID

#ifndef CONTROLLER_HEADER
#define CONTROLLER_HEADER

#include "stdint.h"
#include "SPI.h"
#include "RC.h"

typedef enum E_flightMode : uint8_t
{
    RATE_CTRL = 0,
    ANGLE_CTRL = 1,
    ANGLE_CASCADE_CTRL = 2,
    GPS_CTRL = 3,

    PT1_WO_DERIVATIVE_KICK = 4,
    PT2_FILTER = 5,
    PT2_WO_DERIVATIVE_KICK = 6,
    PT2_WO_DERIV_KICK_FF = 7,
    KALMAN_FILTER = 8
};

typedef struct controllerIn_st 
{
	sigOut gyro;
	sigOut acc;
    rcSignals_st rcSignals;
    float loopTime{0};
}controllerIn_st;

typedef struct controllerOut_st 
{
    axis U;
    E_armState armState{DISARMED};
}controllerOut_st;

typedef struct pid_st
{
    axis P, I, D, FFr, FFdr;
    axis error, errorSum, errorDot, errorPrev, errorDotFiltered;
    axis Pout, Iout, Dout, FFout;
    axis refSignal, refSignalPrev, refSignalDot, refSignalDotFiltered;
    axis u;
	float saturationI, saturationPID, DTermC;
    sigOut sensor;
    sigOut sensorPrev;
	float deltaT{0.1f};
    float PFactor, IFactor, DFactor, FFrFactor, FFdrFactor;
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
typedef struct {
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
    float paramC{ 32 };     //1600/x sampleRate/cutoffRate
    sigOut PT1;
    sigOut PT2;
    float rollAngle;
    float pitchAngle;
    float rollAnglePT1Acc;
    float pitchAnglePT1Acc;
    float rollAnglePT2Acc;
    float pitchAnglePT2Acc;

    double q_angle{ 0.00001 };   // Process noise variance for angle
    double q_bias{ 0.00003 };    // Process noise variance for gyro bias
    double r_measure{ 20.0 }; // Measurement noise variance
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
E_armState EvalArmState(const uint16_t armStateChannel);

// Function to evaluate the flight mode state based on switch state
E_flightMode EvalFlightMode(const uint16_t flightModeChannel);

// Function to calculate parabolical scaling of stick value
float ParabolicScale(const uint16_t channel);

// Function to linear interpolate stick value
float LinearInterpol(const uint16_t xn, const uint16_t x0, const uint16_t x1, const float y0, const float y1);

// Method to calculate low-pass filtered value of a signal
void PT1Filter(float* yOut, float xIn, float paramC );

// Method to calculate PID "u" output based on "pidSt" input, avoiding derivative kick
void CalcPID_wo_Dkick(pid_st* pidSt, axis* u);

// Method to calculate PID "u" output based on "pidSt" input, avoiding derivative kick
void CalcPID_wo_Dkick_FF(pid_st* pidSt, axis* u);

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

#endif // !CONTROLLER_HEADER
