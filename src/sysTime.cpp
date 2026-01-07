// Calulation of loop time with TC1 channel 2 counter

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
    // initalize internal variable
	sysTimer.loopTime = 0.001f;

	pmc_enable_periph_clk(ID_TC2);  //enable peripheral clock for TC1-channel 2

	TC0->TC_WPMR = 0x504D4300;  //disable write protection mode
	TC0->TC_CHANNEL[2].TC_CCR |= TC_CCR_CLKEN;  //enable clock
	TC0->TC_CHANNEL[2].TC_CMR |= TC_CMR_TCCLKS_TIMER_CLOCK2;  //clock selection MCK/8=10.5 MHz
	TC0->TC_WPMR = 0x504D4301;  //reenable write protection mode
	//software triggering
	TC0->TC_CHANNEL[2].TC_CCR |= TC_CCR_SWTRG;
}

void UpdateSysTime(void)
{
    //read raw counter value
	sysTimer.raw = TC0->TC_CHANNEL[2].TC_CV;
	//software trigger to restart counter
	TC0->TC_CHANNEL[2].TC_CCR |= TC_CCR_SWTRG;
	//calculation of real value
	sysTimer.loopTime = (float)sysTimer.raw * sysTimer.const_raw2real;
	sysTimer.sysTime += sysTimer.loopTime;

//	Serial.print(sysTimer.const_raw2real,9); Serial.print("\t");
//	SerialUSB.print(sysTimer.raw);  SerialUSB.print("\t");
//  SerialUSB.print(sysTimer.loopTime, 6);  SerialUSB.print("\t");
//	SerialUSB.print(sysTimer.sysTime,6);SerialUSB.print("\t");
//  SerialUSB.println();

}

float getSysTime(void)
{
  return sysTimer.sysTime;
}

float getSysLoopTime(void)
{
  return sysTimer.loopTime;
}
