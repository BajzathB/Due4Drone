// Calulation of loop time with TC0 channel 2 counter

#include "pch.h"

//to switch header on hardware and unit test compilation
#ifdef UNIT_TEST

#include "../test/helper/support4Testing.h"
#include "../test/helper/support4Testing.hpp"

extern DummySerial Serial;
extern DummySerial SerialUSB;
extern Tc* TC0;

#else

#include "arduino.h"
#include "variant.h"

#endif

#include "sysTime.h"

sysTime sysTimer;

void SetupSysTimer(void)
{
	pmc_enable_periph_clk(ID_TC2);  //enable peripheral clock for TC0-channel 2

	TC0->TC_WPMR = 0x504D4300;  //disable write protection mode
	TC0->TC_CHANNEL[2].TC_CCR |= TC_CCR_CLKEN;  //enable clock
	TC0->TC_CHANNEL[2].TC_CMR |= TC_CMR_TCCLKS_TIMER_CLOCK2;  //clock selection MCK/8=10.5 MHz -> 1tick=95.238 nanosec
	TC0->TC_WPMR = 0x504D4301;  //reenable write protection mode
	//software triggering
	TC0->TC_CHANNEL[2].TC_CCR |= TC_CCR_SWTRG;
}

void UpdateSysTime(droneTimes_st* times)
{
  //read raw counter value
	sysTimer.raw = TC0->TC_CHANNEL[2].TC_CV;
	//software trigger to restart counter
	TC0->TC_CHANNEL[2].TC_CCR |= TC_CCR_SWTRG;

	sysTimer.sysTick += (uint64_t)sysTimer.raw;

	times->loopTick = sysTimer.raw;
	times->sysTick = sysTimer.sysTick;

//	Serial.print(sysTimer.const_raw2real,9); Serial.print("\t");
//	SerialUSB.print(sysTimer.raw);  SerialUSB.print("\t");
//  SerialUSB.print(sysTimer.loopTime, 6);  SerialUSB.print("\t");
//	SerialUSB.print(sysTimer.sysTime,6);SerialUSB.print("\t");
//  SerialUSB.println();

}

uint64_t getSysTick(void)
{
	return sysTimer.sysTick;
}

float getTimeSinceReset(void)
{
	return (float)TC0->TC_CHANNEL[2].TC_CV * 0.095238095;
}

float calcDeltaTime(float t1, float t2)
{
	return (t1 < t2) ? t2 - t1 : 0.0f;
}

float getSysTime(void)
{
  return float(sysTimer.sysTick) / float(sysTimer.divider);
}
