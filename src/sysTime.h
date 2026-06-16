// Calulation of loop time with TC0 channel 2 counter

#ifndef LOOP_TIMER_HEADER
#define LOOP_TIMER_HEADER

#include "stdint.h"

typedef struct droneTimes_st
{
	uint32_t loopTick;	//1tick=95.23nanosec
	uint64_t sysTick;
};

typedef struct sysTime {
	uint32_t raw;
	uint64_t sysTick;
	const uint32_t divider{ 10500000 };	//10.5MHz

} sysTime;

//  Method to setup system time
void SetupSysTimer(void);

// Method to update system time value
void UpdateSysTime(droneTimes_st* times);

// Function to get system tick
uint64_t getSysTick(void);

// Function to get system time in second
float getSysTime(void);

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
