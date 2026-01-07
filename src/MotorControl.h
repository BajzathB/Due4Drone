// Header to configure and set motor control pins


#ifndef MOTOR_CONTROL_HEADER
#define MOTOR_CONTROL_HEADER

#include "Controller.h"
#include "SPI.h"

typedef enum E_BeepState : uint8_t
{
    BEEP_TRIGGER = 0,
    BEEP_HOLD = 1
}E_BeepState;

typedef struct MotorCommander
{
	float sysTime;
	float lastpulseTime;
	float lift_force;
	float FL, FR, RL, RR;  //frontleft,frontright,rearleft,rearright
	float FL_tick, FR_tick, RL_tick, RR_tick;
	const float const_TC_clock_freq = 10.5f; //the same as RC.TC_clock_freq

    float lastBeepTime{ 0 };
    E_BeepState beepState{ BEEP_TRIGGER };
}motorCommander;

typedef struct MotorInput
{
  // throttle stick value
  uint16_t throttle;
  // angular rotation values of each axis
  float x,y,z;
  E_armState armState{DISARMED};
  // current sys time
  float sysTime;

  // controll signals for beep sound
  uint16_t poti1, poti2, twoWaySwitch2;
  
}MotorInput;

typedef struct MotorSpeeds
{
    float FL, FR, RL, RR;  //frontleft,frontright,rearleft,rearright
    float FL_tick, FR_tick, RL_tick, RR_tick;
}MotorSpeeds;

// Method to setup motor signal pins
void SetupMotorPins(void);

// Method to set motor signal values
void UpdateMotorSpeeds(const MotorInput* motorInput);

//Method to calculate motor speeds
void CalcMotorSpeeds(const MotorInput* motorInput, MotorCommander* motorCmd);

// Method to setup pb0 and pb1 pins for 1shot pulse
void Setup_PB0_PB1_for_oneshot_pulse(void);

// Method setup pb2 and pb3 pins for 1shot pulse
void Setup_PB2_PB3_for_oneshot_pulse(void);

// Method to set motor registers
void SetTcCompareRegister(MotorCommander* motorCmd);

// Method to SW trigger TC1
void TriggerTcRegisters(MotorCommander* motorCmd);

// Method to get motor speeds
void getMotorSpeeds(MotorSpeeds* motorSpeeds);

// Method to handle motor beeps
void handleBeeps(const MotorInput* motorInput, MotorCommander* motorCmd);

#endif // !MOTOR_CONTROL_HEADER
