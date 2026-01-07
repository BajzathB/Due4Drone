//TC1 channel 0-1
//S1 - PB0- B peri - TIOA3
//S2 - PB1- B peri - TIOB3
//S3 - PB2- B peri - TIOA4
//S4 - PB3- B peri - TIOB4
//
//original sensor pulse is 1000-2000 ms
//1shut pulse is 150-250 us


#include "pch.h"
#include "MotorControl.h"
#include "sysTime.h"

//to switch header on hardware and unit test compilation
#ifdef UNIT_TEST

#include "../test/helper/support4Testing.h"
#include "../test/helper/support4Testing.hpp"

extern DummySerial SerialUSB;
extern Pio* PIOB;
extern Tc* TC1;

#else

#include "arduino.h"
#include "variant.h"

#endif

//Controling the four DLBC motor by oneshot pulses with TC

#define MICROSEC_250 0.00025f

MotorCommander motorCommand;

void UpdateMotorSpeeds(const MotorInput* motorInput)
{
	//calc motor speeds
	CalcMotorSpeeds(motorInput, &motorCommand);
  
	//set TC compare registers accordingly
	SetTcCompareRegister(&motorCommand);

	motorCommand.sysTime = motorInput->sysTime;

	//trigger TC channels, not earlier than 250 us
	TriggerTcRegisters(&motorCommand);
//  SerialUSB.print(motorCommand.FL);SerialUSB.print("\t");
//  SerialUSB.print(motorCommand.FR);SerialUSB.print("\t");
//  SerialUSB.print(motorCommand.RL);SerialUSB.print("\t");
//  SerialUSB.println(motorCommand.RR);

}


void CalcMotorSpeeds(const MotorInput* motorInput, MotorCommander* motorCmd)
{
	if (E_armState::ARMED == motorInput->armState)
	{
		//calculation of the four motor speed, 8 divider for oneshot protocol
		motorCmd->FL = motorInput->throttle/8 + motorInput->x + motorInput->y + motorInput->z;
		motorCmd->FR = motorInput->throttle/8 - motorInput->x + motorInput->y - motorInput->z;
		motorCmd->RL = motorInput->throttle/8 + motorInput->x - motorInput->y - motorInput->z;
		motorCmd->RR = motorInput->throttle/8 - motorInput->x - motorInput->y + motorInput->z;

		//saturation if values are out of 125-250 us band
		if (motorCmd->FL < 125.0) motorCmd->FL = 125.0;
		else if (motorCmd->FL > 250.0) motorCmd->FL = 250.0;
		else;//do nothing
		if (motorCmd->FR < 125.0) motorCmd->FR = 125.0;
		else if (motorCmd->FR > 250.0) motorCmd->FR = 250.0;
		else;//do nothing
		if (motorCmd->RL < 125.0) motorCmd->RL = 125.0;
		else if (motorCmd->RL > 250.0) motorCmd->RL = 250.0;
		else;//do nothing
		if (motorCmd->RR < 125.0) motorCmd->RR = 125.0;
		else if (motorCmd->RR > 250.0) motorCmd->RR = 250.0;
		else;//do nothing
	}
	else
	{
        if (motorInput->twoWaySwitch2 > 1800)
        {
			    handleBeeps(motorInput, motorCmd);
        }
        else
        {
            motorCmd->FL = 125.0;
            motorCmd->FR = 125.0;
            motorCmd->RL = 125.0;
            motorCmd->RR = 125.0;
        }
	}
}

void handleBeeps(const MotorInput* motorInput, MotorCommander* motorCmd)
{
	switch (motorCmd->beepState)
	{
		case BEEP_TRIGGER:
		{
			float beepTime = float(motorInput->poti1) / 1000.0f - 1.0f;
			beepTime = (beepTime > 0.1f) ? beepTime : 0.1f;
			if ((motorInput->sysTime - motorCmd->lastBeepTime) > beepTime)    //beep moment
			{
				motorCmd->FL = motorInput->poti2 / 8;
				motorCmd->FR = motorInput->poti2 / 8;
				motorCmd->RL = motorInput->poti2 / 8;
				motorCmd->RR = motorInput->poti2 / 8;

				motorCmd->lastBeepTime = motorInput->sysTime;
				motorCmd->beepState = BEEP_HOLD;
			}
			else    //non-beep waiting
			{
				motorCmd->FL = 125.0;
				motorCmd->FR = 125.0;
				motorCmd->RL = 125.0;
				motorCmd->RR = 125.0;
			}

			break;
		}
		case BEEP_HOLD:
		{
			if ((motorInput->sysTime - motorCmd->lastBeepTime) > (15 * MICROSEC_250))
			{
				motorCmd->FL = 125.0;
				motorCmd->FR = 125.0;
				motorCmd->RL = 125.0;
				motorCmd->RR = 125.0;

				motorCmd->beepState = BEEP_TRIGGER;
			}
			else
			{
				motorCmd->FL = motorInput->poti2 / 8;
				motorCmd->FR = motorInput->poti2 / 8;
				motorCmd->RL = motorInput->poti2 / 8;
				motorCmd->RR = motorInput->poti2 / 8;
			}
			break;
		}
		default:
			break;
	}
}

void SetupMotorPins(void)
{
    // initalize internal variables
    motorCommand.sysTime = 0;
    motorCommand.lastpulseTime = 0;

    //PB0-PB1 pin
    Setup_PB0_PB1_for_oneshot_pulse();

    //PB2-PB3 pin
    Setup_PB2_PB3_for_oneshot_pulse();

    //software triggering TC1
    TC1->TC_BCR = TC_BCR_SYNC;
}

void TriggerTcRegisters(MotorCommander* motorCmd)
{
	 //SerialUSB.print(motorCommand.sysTime - motorCommand.lastpulseTime,6); SerialUSB.print("\t");
	 //SerialUSB.println(motorCommand.lastpulseTime,6);

	if ((motorCmd->sysTime - motorCmd->lastpulseTime) > MICROSEC_250)
	{
		motorCmd->lastpulseTime = motorCmd->sysTime;

		//software triggering TC1
		TC1->TC_BCR = TC_BCR_SYNC;
	}
}

void SetTcCompareRegister(MotorCommander* motorCmd)
{
	motorCmd->FL_tick = motorCmd->FL * motorCmd->const_TC_clock_freq;
	motorCmd->FR_tick = motorCmd->FR * motorCmd->const_TC_clock_freq;
	motorCmd->RL_tick = motorCmd->RL * motorCmd->const_TC_clock_freq;
	motorCmd->RR_tick = motorCmd->RR * motorCmd->const_TC_clock_freq;

	//setting TIO0 RA value which corresponds to FRONT LEFT
	TC1->TC_CHANNEL[0].TC_RA = uint32_t(motorCmd->FL_tick);
	//setting TIO6 RB value which corresponds to FRONT LEFT motor
	TC1->TC_CHANNEL[0].TC_RB = uint32_t(motorCmd->RL_tick);
	//setting TIO6 RA value which corresponds to REAR RIGHT
	TC1->TC_CHANNEL[1].TC_RA = uint32_t(motorCmd->FR_tick);
	//setting TIO7 RA value which corresponds to REAR RIGHT
	TC1->TC_CHANNEL[1].TC_RB = uint32_t(motorCmd->RR_tick);
}

void Setup_PB0_PB1_for_oneshot_pulse(void)
{
	PIOB->PIO_WPSR = 0x50494F00; //enable write mode
	PIOB->PIO_PDR |= PIO_PDR_P0 | PIO_PDR_P1; //disable PIO
	PIOB->PIO_ABSR |= PIO_ABSR_P0 | PIO_ABSR_P1;  //select peripheral B
	PIOB->PIO_WPSR = 0x50494F01; //disable write mode

	pmc_enable_periph_clk(ID_TC3);  //enable peripheral clock for TC1-channel 0

	TC1->TC_WPMR = 0x504D4300;  //disable write protection mode
	TC1->TC_CHANNEL[0].TC_CCR |= TC_CCR_CLKEN;  //enable clock
	TC1->TC_CHANNEL[0].TC_CMR |= TC_CMR_TCCLKS_TIMER_CLOCK2;  //clock selection MCK/8=10.5 MHz
    TC1->TC_CHANNEL[0].TC_CMR |= TC_CMR_EEVT_XC0; //enable TIOB as output
	TC1->TC_CHANNEL[0].TC_CMR |= TC_CMR_WAVSEL_UP;  //counter increments without automatic trigger on RC
	TC1->TC_CHANNEL[0].TC_CMR |= TC_CMR_WAVE; //waveform mode selection
	TC1->TC_CHANNEL[0].TC_CMR |= TC_CMR_ACPA_CLEAR; //RA compare clears TIOA
	TC1->TC_CHANNEL[0].TC_CMR |= TC_CMR_ASWTRG_SET; //software trigger sets TIOA
	TC1->TC_CHANNEL[0].TC_CMR |= TC_CMR_BCPB_CLEAR; //RB compare clears TIOB
	TC1->TC_CHANNEL[0].TC_CMR |= TC_CMR_BSWTRG_SET; //software trigger sets TIOB
	TC1->TC_CHANNEL[0].TC_CMR |= TC_CMR_CPCSTOP;  //counter stops on RC compare
	TC1->TC_CHANNEL[0].TC_RC = 2625;
	TC1->TC_WPMR = 0x504D4301;  //reenable write protection mode

	//setting RA and RB value
	TC1->TC_CHANNEL[0].TC_RA = 1312;
	TC1->TC_CHANNEL[0].TC_RB = 1312;
	//software triggering
}

void Setup_PB2_PB3_for_oneshot_pulse(void)
{
	PIOB->PIO_WPSR = 0x50494F00; //enable write mode
	PIOB->PIO_PDR |= PIO_PDR_P2 | PIO_PDR_P3; //disable PIO
	PIOB->PIO_ABSR |= PIO_ABSR_P2 | PIO_ABSR_P3;  //select peripheral B
	PIOB->PIO_WPSR = 0x50494F01; //disable write mode

	pmc_enable_periph_clk(ID_TC4);  //enable peripheral clock for TC0-channel 0

	TC1->TC_WPMR = 0x504D4300;  //disable write protection mode
	TC1->TC_CHANNEL[1].TC_CCR |= TC_CCR_CLKEN;  //enable clock
	TC1->TC_CHANNEL[1].TC_CMR |= TC_CMR_TCCLKS_TIMER_CLOCK2;  //clock selection MCK/8=10.5 MHz
    TC1->TC_CHANNEL[1].TC_CMR |= TC_CMR_EEVT_XC0; //enable TIOB as output
	TC1->TC_CHANNEL[1].TC_CMR |= TC_CMR_WAVSEL_UP;  //counter increments without automatic trigger on RC
	TC1->TC_CHANNEL[1].TC_CMR |= TC_CMR_WAVE; //waveform mode selection
	TC1->TC_CHANNEL[1].TC_CMR |= TC_CMR_ACPA_CLEAR; //RA compare clears TIOA
	TC1->TC_CHANNEL[1].TC_CMR |= TC_CMR_ASWTRG_SET; //software trigger sets TIOA
	TC1->TC_CHANNEL[1].TC_CMR |= TC_CMR_BCPB_CLEAR; //RB compare clears TIOB
	TC1->TC_CHANNEL[1].TC_CMR |= TC_CMR_BSWTRG_SET; //software trigger sets TIOB
	TC1->TC_CHANNEL[1].TC_CMR |= TC_CMR_CPCSTOP;  //counter stops on RC compare
	TC1->TC_CHANNEL[1].TC_RC = 2625;
	TC1->TC_WPMR = 0x504D4301;  //reenable write protection mode

	//setting RA value
	TC1->TC_CHANNEL[1].TC_RA = 1312;
	TC1->TC_CHANNEL[1].TC_RB = 1312;
	//software triggering
}

//set output for bluetooth transmission
void getMotorSpeeds(MotorSpeeds* motorSpeeds)
{
    motorSpeeds->FL = motorCommand.FL;
    motorSpeeds->FR = motorCommand.FR;
    motorSpeeds->RL = motorCommand.RL;
    motorSpeeds->RR = motorCommand.RR;
    motorSpeeds->FL_tick = motorCommand.FL_tick;
    motorSpeeds->FR_tick = motorCommand.FR_tick;
    motorSpeeds->RL_tick = motorCommand.RL_tick;
    motorSpeeds->RR_tick = motorCommand.RR_tick;
}
