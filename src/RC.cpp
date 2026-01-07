// RC IBus receive - PB21 A peri - USART2 RXD2
// 32 byte packets: header + 14channel + CRC
//  2 byte header: 0x20 + 0x40
//  14 channel in 2 bytes 
//  2 byte CRC
// CRC calc from channel values:
//  start from 0xFFFF
//  crc = 0xFFFF - 0x20
//  crc = crc - 0x40
//  crc = crc - ch[1] - ch[0] for each channel
//  check CRC == crc
//
// RC channels:
//  0 - roll
//  1 - pitch
//  2 - throttle
//  3 - yaw
//  4 - arm/disarm
//  5 - 2-state switch
//  6 - potmeter
//  7 - potmeter
//  8 - 3-state switch
//  9 - 2-state switch


#include "pch.h"
#include "RC.h"
#include "LED.h"

#ifdef UNIT_TEST

#include "../test/helper/support4Testing.h"
#include "../test/helper/support4Testing.hpp"

extern DummySerial SerialUSB;
extern Pio* PIOB;
extern Usart* USART2;
extern Wdt* WDT;

#else

#include "arduino.h"
#include "variant.h"

#endif

rc_st RC;

//uint16_t testCtrIntOutter = 0;
//uint16_t testCtrIntInner = 0;

void SetupRC(void)
{
    SetInitValues();

    SetupUsart2Pin();

    //watchdog is set up via arduino defined watchdogSetup function
}

void SetInitValues(void)
{
    for (uint8_t iter = 0; iter < 32; iter++)
    {
        RC.IBUS_received[iter] = 0x00;
    }

    SetChannels2Init();

    RC.isCrcValid = false;
}

void SetChannels2Init(void)
{
    RC.IBUS_channel[0] = 1500;  //roll
    RC.IBUS_channel[1] = 1500;  //pitch
    RC.IBUS_channel[2] = 1000;  //throttle
    RC.IBUS_channel[3] = 1500;  //yaw
    RC.IBUS_channel[4] = 1000;  //arm/disarm
    RC.IBUS_channel[5] = 1000;
    RC.IBUS_channel[6] = 1000;
    RC.IBUS_channel[7] = 1000;
    RC.IBUS_channel[8] = 1000;
    RC.IBUS_channel[9] = 1000;
    RC.IBUS_channel[11] = 1000;
    RC.IBUS_channel[12] = 1000;
    RC.IBUS_channel[13] = 1000;
    RC.IBUS_channel[10] = 1000;
}
 
void SetupUsart2Pin(void)
{
    //Setup peripherial to USART2
    PIOB->PIO_WPSR = 0x50494F00; //disable write protection mode on parallel io line B
    PIOB->PIO_PDR |= PIO_PDR_P21; //enable peripheral control
    PIOB->PIO_ABSR &= ~PIO_ABSR_P21;  //select peripheral A
    PIOB->PIO_WPSR = 0x50494F01; //reenable write protection mode on parallel io line B

    pmc_enable_periph_clk(ID_USART2);

    //software reset RX
    USART2->US_CR |= US_CR_RSTRX;

    USART2->US_WPMR = 0x55534100; //disable write protection on usart
    USART2->US_MR |= US_MR_USART_MODE_NORMAL //set normal mode
        | US_MR_USCLKS_MCK  //master clock selected
        | US_MR_CHRL_8_BIT  //8 bit data
        | US_MR_PAR_NO  //no parity bit
        | US_MR_NBSTOP_1_BIT  //1 stop bit
        | US_MR_OVER;  // 8x oversampling for achiving minimal error between actual and expected baud rate

    //calculated baud rate is 115226,3, error is 0,022%
    USART2->US_BRGR = US_BRGR_CD(91) | US_BRGR_FP(1);
    USART2->US_WPMR = 0x55534101; //enable write protection on usart

    NVIC_SetPriority(USART2_IRQn, 2);
    NVIC_EnableIRQ(USART2_IRQn);

    //start looking for 32 byte
    USART2->US_CR |= US_CR_RXEN;//enable receive
    USART2->US_RPR = (uint32_t)RC.IBUS_received;
    USART2->US_RCR = 32u;
    USART2->US_PTCR |= US_PTCR_RXTEN; //start pdc receive
    USART2->US_IER |= US_IER_ENDRX; //enable endrx interrupt
}

// Interrupt handler of USART2
// Commenting out USART2_Handler function in the following file!!!
// c:\Users\Bence_Laptop\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\variants\arduino_due_x\variant.cpp
void USART2_Handler(void)
{
    uint32_t usart_status = USART2->US_CSR;
    uint8_t in, out;

    //testing
    //testCtrIntOutter++;

    if (usart_status & US_CSR_ENDRX)
    {
        RestartWDCtr();

        if (IsIbusHeaderReceived(RC.IBUS_received))
        {
            //testing
            //testCtrIntInner++;

            uint16_t calcultedCRC{0xFFFF};
            calcultedCRC -= ((uint16_t)RC.IBUS_received[0] + (uint16_t)RC.IBUS_received[1]);

            for (in = 2, out = 0; in < 30; in += 2, out++)  //in: 2-28, out: 0-14
            {
                //calculation of channel value
                RC.IBUS_channel[out] = (uint16_t)RC.IBUS_received[in] | ((uint16_t)(RC.IBUS_received[in + 1]) << 8);
                //calcualtion of CRC
                calcultedCRC -= ((uint16_t)RC.IBUS_received[in] + (uint16_t)RC.IBUS_received[in + 1]);
            }
            //calculation of received CRC
            uint16_t receivedCRC = (uint16_t)RC.IBUS_received[30] | ((uint16_t)(RC.IBUS_received[31]) << 8);
            if (receivedCRC == calcultedCRC)
            {
                RC.isCrcValid = true;
            }
            else
            {
                RC.isCrcValid = false;
                SetChannels2Init();
            }

            //look for the next 32 byte
            USART2->US_RPR = (uint32_t)RC.IBUS_received;
            USART2->US_RCR = 32u;
        }
        else
        {
            //attempt to find header's 1st byte
            uint8_t head = 1;

            while (head < 31)
            {
                if (IsIbusHeaderReceived(&RC.IBUS_received[head]))
                {
                    break;
                }
                head++;
            }

            // if didnt find header check 31-0 combination
            if (31 == head)
            {
                //if not even the 31-0 combination is header
                if (!(0x20 == RC.IBUS_received[31] && 0x40 == RC.IBUS_received[0]))
                {
                    //read new 32 byte for retry
                    USART2->US_RPR = (uint32_t)RC.IBUS_received;
                    USART2->US_RCR = 32u;
                    //set channels to init
                    RC.isCrcValid = false;
                    SetChannels2Init();

                    return;
                }
            }

            //shift good data into correct position in received vector
            uint8_t before_ctr = head;
            uint8_t after_ctr = 0;

            for (/*empty*/; before_ctr <= 31; before_ctr++, after_ctr++)
            {
                RC.IBUS_received[after_ctr] = RC.IBUS_received[before_ctr];
            }

            //look for the rest of the bytes, head amount of bytes are missing
            USART2->US_RPR = (uint32_t)(&RC.IBUS_received[after_ctr + 1]);
            USART2->US_RCR = head;

            //set channels to init
            RC.isCrcValid = false;
            SetChannels2Init();
        }
    }
}

bool IsIbusHeaderReceived(uint8_t* IbusVector)
{
    return (0x20 == IbusVector[0] && 0x40 == IbusVector[1]);
}

void getRcChannels(rcSignals_st* rcOut)
{
    //disable USART2 interrupt
    NVIC_DisableIRQ(USART2_IRQn);

    if (RC.isCrcValid)
    {
        rcOut->roll              = RC.IBUS_channel[0];
        rcOut->pitch             = RC.IBUS_channel[1];
        rcOut->throttle          = RC.IBUS_channel[2];
        rcOut->yaw               = RC.IBUS_channel[3];
        rcOut->armStateSwitch    = RC.IBUS_channel[4];
        rcOut->measurementSwitch = RC.IBUS_channel[5];
        rcOut->Poti1             = RC.IBUS_channel[6];
        rcOut->Poti2             = RC.IBUS_channel[7];
        rcOut->flightModeSwitch  = RC.IBUS_channel[8];
        rcOut->Switch2Way        = RC.IBUS_channel[9];

        //4 empty channel remaining
    }
    else
    {
        rcOut->roll              = 1500;
        rcOut->pitch             = 1500;
        rcOut->throttle          = 1000;
        rcOut->yaw               = 1500;
        rcOut->armStateSwitch    = 1000;
        rcOut->measurementSwitch = 1000;
        rcOut->Poti1             = 1000;
        rcOut->Poti2             = 1000;
        rcOut->flightModeSwitch  = 1000;
        rcOut->Switch2Way        = 1000;

        //4 empty channel remaining
    }   

    //reenable USART2 interrupt
    NVIC_EnableIRQ(USART2_IRQn);
}


// this function has to be present, otherwise watchdog won't work
void watchdogSetup(void) 
{ 
    uint32_t wdt_mr = WDT_MR_WDV(128)  //32768hz, 4096=16sec, 128=0.5s?
        | WDT_MR_WDFIEN //enable interrupt
        | WDT_MR_WDD(2000); //make delta value high->no error fault
    WDT->WDT_MR = wdt_mr;
    
    NVIC_SetPriority(WDT_IRQn, 3);
    NVIC_EnableIRQ(WDT_IRQn);
    
    RestartWDCtr();
}

inline void RestartWDCtr(void)
{
    //WDT->WDT_CR = WDT_CR_WDRSTT | WDT_CR_KEY(0xA5);
    WDT->WDT_CR = 0xA5000001;
}

void WDT_Handler(void)
{
    uint32_t wdt_status = WDT->WDT_SR;

    if(wdt_status & WDT_SR_WDUNF)
    {
        SetChannels2Init();

        LEDAllBlink();
    }
}
