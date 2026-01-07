//TC2 channel 0-1-2
//LED_GYRO   - PC25 - TIOA6 - WHITE
//LED_ACC    - PC26 - TIOB6 - YELLOW
//LED_MAG    - PC28 - TIOA7 - RED
//LED_SDCARD - PD7  - TIOA8 - BLUE
//LED_GPS    - PD8  - TIOB8 - GREEN

#include "pch.h"
#include "LED.h"

#ifdef UNIT_TEST

#include "test/helper/support4Testing.h"
#include "test/helper/support4Testing.hpp"

extern DummySerial SerialUSB;
extern Pio* PIOC;
extern Pio* PIOD;
extern Tc* TC2;

#else

#include "arduino.h"
#include "variant.h"

#endif


void SetupLED(void)
{    
    //setup PC25-PC26
    PIOC->PIO_WPMR = 0x50494F00; //enable write mode
    PIOC->PIO_PDR |= PIO_PDR_P25 | PIO_PDR_P26; //disable digital pin, turns peripheral on
    PIOC->PIO_ABSR |= PIO_ABSR_P25 | PIO_ABSR_P26;  //select peripheral B
    PIOC->PIO_WPMR = 0x50494F01; //disable write mode

    pmc_enable_periph_clk(ID_TC6);  //enable peripheral clock for TC2-channel 0
  
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    
    TC2->TC_CHANNEL[0].TC_CCR |= TC_CCR_CLKEN;  //enable clock
    TC2->TC_CHANNEL[0].TC_CMR |= TC_CMR_TCCLKS_TIMER_CLOCK4; //clock selection MCK/128 = 656250 Hz
    TC2->TC_CHANNEL[0].TC_CMR |= TC_CMR_EEVT_XC0; //enable TIOB as output
    TC2->TC_CHANNEL[0].TC_CMR |= TC_CMR_WAVSEL_UP_RC;  //counter increments with automatic trigger on RC
    TC2->TC_CHANNEL[0].TC_CMR |= TC_CMR_WAVE; //waveform mode selection
    TC2->TC_CHANNEL[0].TC_CMR |= TC_CMR_ACPA_SET; //on RA set voltage to TIOA
    TC2->TC_CHANNEL[0].TC_CMR |= TC_CMR_ACPC_CLEAR; //on RC clear voltage from TIOA
    TC2->TC_CHANNEL[0].TC_CMR |= TC_CMR_BCPB_SET; //on RB set voltage to TIOB
    TC2->TC_CHANNEL[0].TC_CMR |= TC_CMR_BCPC_CLEAR; //on RC clear voltage from TIOB
    
    TC2->TC_CHANNEL[0].TC_RA = 1;
    TC2->TC_CHANNEL[0].TC_RB = 328125; //0.5sec
    TC2->TC_CHANNEL[0].TC_RC = 656250; //1sec cycle

    //setup PC28
    PIOC->PIO_WPMR = 0x50494F00; //enable write mode
    PIOC->PIO_PDR  |= PIO_PDR_P28; //disable digital pin, turns peripheral on
    PIOC->PIO_ABSR |= PIO_ABSR_P28;  //select peripheral B
    PIOC->PIO_WPMR = 0x50494F01; //disable write mode

    pmc_enable_periph_clk(ID_TC7);  //enable peripheral clock for TC2-channel 1
  
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[1].TC_CCR |= TC_CCR_CLKEN;  //enable clock
    TC2->TC_CHANNEL[1].TC_CMR |= TC_CMR_TCCLKS_TIMER_CLOCK4; //clock selection MCK/128 = 656250 Hz
    TC2->TC_CHANNEL[1].TC_CMR |= TC_CMR_EEVT_XC0; //enable TIOB as output
    TC2->TC_CHANNEL[1].TC_CMR |= TC_CMR_WAVSEL_UP_RC;  //counter increments with automatic trigger on RC
    TC2->TC_CHANNEL[1].TC_CMR |= TC_CMR_WAVE; //waveform mode selection
    TC2->TC_CHANNEL[1].TC_CMR |= TC_CMR_ACPA_SET; //on RA set voltage to TIOA
    TC2->TC_CHANNEL[1].TC_CMR |= TC_CMR_ACPC_CLEAR; //on RC clear voltage from TIOA
    
    TC2->TC_CHANNEL[1].TC_RA = 420000;
    TC2->TC_CHANNEL[1].TC_RC = 656250; //1sec cycle

    //setup PD7-PD28
    PIOD->PIO_WPMR = 0x50494F00; //enable write mode
    PIOD->PIO_PDR  |= PIO_PDR_P7 | PIO_PDR_P8; //disable digital pin, turns peripheral on
    PIOD->PIO_ABSR |= PIO_PDR_P7 | PIO_ABSR_P8;  //select peripheral B
    PIOD->PIO_WPMR = 0x50494F01; //disable write mode

    pmc_enable_periph_clk(ID_TC8);  //enable peripheral clock for TC2-channel 2
  
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[2].TC_CCR |= TC_CCR_CLKEN;  //enable clock
    TC2->TC_CHANNEL[2].TC_CMR |= TC_CMR_TCCLKS_TIMER_CLOCK4; //clock selection MCK/128 = 656250 Hz
    TC2->TC_CHANNEL[2].TC_CMR |= TC_CMR_EEVT_XC0; //enable TIOB as output
    TC2->TC_CHANNEL[2].TC_CMR |= TC_CMR_WAVSEL_UP_RC;  //counter increments with automatic trigger on RC
    TC2->TC_CHANNEL[2].TC_CMR |= TC_CMR_WAVE; //waveform mode selection
    TC2->TC_CHANNEL[2].TC_CMR |= TC_CMR_ACPA_SET; //on RA set voltage to TIOA
    TC2->TC_CHANNEL[2].TC_CMR |= TC_CMR_ACPC_CLEAR; //on RC clear voltage from TIOA
    TC2->TC_CHANNEL[2].TC_CMR |= TC_CMR_BCPB_SET; //on RB set voltage to TIOB
    TC2->TC_CHANNEL[2].TC_CMR |= TC_CMR_BCPC_CLEAR; //on RC clear voltage from TIOB
    
    TC2->TC_CHANNEL[2].TC_RA = 500000;
    TC2->TC_CHANNEL[2].TC_RB = 550000;
    TC2->TC_CHANNEL[2].TC_RC = 656250; //1sec cycle

    
    //software triggering will be at PC25-PC26
    TC2->TC_CHANNEL[0].TC_CCR |= TC_CCR_SWTRG; 
    //software triggering will be at PC28
    TC2->TC_CHANNEL[1].TC_CCR |= TC_CCR_SWTRG;
    //software triggering will be at PD7-PD28
    TC2->TC_CHANNEL[2].TC_CCR |= TC_CCR_SWTRG;
    
    TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode
}

void LEDGyroOn(void)
{
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[0].TC_RA = 1;
    TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode
}

void LEDGyroOff(void)
{
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[0].TC_RA = 656251; //RA will not be reached due to RC resets earlier
    TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode

    //TODO: disable TC channel if both gyro and acc set to off
}

void LEDGyroBlink(void)
{
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[0].TC_RA = 328125; //half of RC value, 0.5sec blink
    TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode
}

void LEDAccOn(void)
{  
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[0].TC_RB = 1;
    TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode
}

void LEDAccOff(void)
{
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[0].TC_RB = 656251; //RA will not be reached due to RC resets earlier
    TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode

    //TODO: disable TC channel if both gyro and acc set to off
}

void LEDAccBlink(void)
{
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[0].TC_RB = 328125; //half of RC value, 0.5sec blink
    TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode
}

void LEDMagOn(void)
{
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[1].TC_RA = 1;
    TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode
}

void LEDMagOff(void)
{
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[1].TC_RA = 656251; //RA will not be reached due to RC resets earlier
    TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode

    //TODO: disable TC channel if both gyro and acc set to off
}

void LEDMagBlink(void)
{
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[1].TC_RA = 328125; //half of RC value, 0.5sec blink
    TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode
}

void LEDSDOn(void)
{
  TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
  TC2->TC_CHANNEL[2].TC_RA = 1;
  TC2->TC_CHANNEL[2].TC_RC = 656250; //1sec cycle
  TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode
}

void LEDSDOff(void)
{
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[2].TC_RA = 656251; //RA will not be reached due to RC resets earlier
	TC2->TC_CHANNEL[2].TC_RC = 656250; //1sec cycle
    TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode

    //TODO: disable TC channel if both gyro and acc set to off
}

void LEDSDBlink(void)
{
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[2].TC_RA = 328125; //half of RC value, 0.5sec blink
	TC2->TC_CHANNEL[2].TC_RC = 656250; //1sec cycle
    TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode
}

void LEDSDBlinkSlow(void)
{
	TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
	TC2->TC_CHANNEL[2].TC_RA = 328125*3; //half of RC value, 0.5sec blink
	TC2->TC_CHANNEL[2].TC_RC = 656250*2; //1sec cycle
	TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode
}

void LEDGPSOn(void)
{
  TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
  TC2->TC_CHANNEL[2].TC_RB = 1;
  TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode
}

void LEDGPSOff(void)
{
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[2].TC_RB = 656251; //RA will not be reached due to RC resets earlier
    TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode

    //TODO: disable TC channel if both gyro and acc set to off
}

void LEDGPSBlink(void)
{
    TC2->TC_WPMR = 0x504D4300;  //disable write protection mode
    TC2->TC_CHANNEL[2].TC_RB = 328125; //half of RC value, 0.5sec blink
    TC2->TC_WPMR = 0x504D4301;  //reenable write protection mode
}

void LEDAllOff(void)
{
    LEDGyroOff();
    LEDAccOff();
    LEDMagOff();
    LEDSDOff();
    LEDGPSOff();
}

void LEDAllBlink(void)
{
    LEDGyroBlink();
    LEDAccBlink();
    LEDMagBlink();
    LEDSDBlink();
    LEDGPSBlink();
}