// Calulation of loop time with TC1 channel 2 counter

#ifndef LOOP_TIMER_HEADER
#define LOOP_TIMER_HEADER

#include "stdint.h"

typedef struct sysTime {
	uint32_t raw;
	float loopTime;
	float sysTime;
	const float const_raw2real = 0.000000095238095; //1/10.5 MHz?
} sysTime;

//  Method to setup system time
void SetupSysTimer(void);

// Method to update system time value
void UpdateSysTime(void);

// Function to get system time in second
float getSysTime(void);

// Function to get system loop time, the time between last and current cycle in second
float getSysLoopTime(void);


#endif // LOOP_TIMER_HEADER
