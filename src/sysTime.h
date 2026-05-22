// Calulation of loop time with TC0 channel 2 counter

#ifndef LOOP_TIMER_HEADER
#define LOOP_TIMER_HEADER

#include "stdint.h"

typedef struct droneTimes_st
{
	float loopTime;
	float sysTime;
	uint32_t loopTick;	//1tick=95.23nanosec
	uint64_t sysTick;
	uint32_t divider{10500000};
};

typedef struct sysTime {
	uint32_t raw;
	float loopTime;
	float sysTime;
	const float const_raw2real = 0.000000095238095; //1/10.5 MHz?

	uint64_t sysTick;
} sysTime;

//  Method to setup system time
void SetupSysTimer(void);

// Method to update system time value
void UpdateSysTime(droneTimes_st* times);

// Method to get drone times
void getDroneTimes(droneTimes_st* times);

// Function to get system time in second
float getSysTime(void);

// Function to get system loop time, the time between last and current cycle in second
float getSysLoopTime(void);

// Function to get time since timer restarted in UpdateSysTime function, output in microsec
float getTimeSinceReset(void);

// Function to calculate time difference, t2 has to be bigger otherwise 0.0 returned
//example:
//timer1 = getTimeSinceReset();
//...
//timer2 = getTimeSinceReset();
//SerialUSB.print("2-1: "); SerialUSB.println(calcDeltaTime(timer1, timer2), 3);
float calcDeltaTime(float t1, float t2);

#endif // LOOP_TIMER_HEADER
