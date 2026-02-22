// Header to configure and set USART2 for RC IBUS data receive


#ifndef RC_HPP
#define RC_HPP

#include "stdint.h"

typedef struct rc_st {
    // bytes from RC stored here
    uint8_t IBUS_received[32];
    // calculated channel values stored here
    volatile uint16_t IBUS_channel[14];
    // evaluated CRC state st
    bool isCrcValid;
}rc_st;

typedef struct rcSignals_st {
    uint16_t roll{ 1500 };
    uint16_t pitch{ 1500 };
    uint16_t throttle{ 1000 };
    uint16_t yaw{ 1500 };
    uint16_t armStateSwitch{ 1000 };
    uint16_t measurementSwitch{ 1000 };
    uint16_t Poti1{ 1000 };
    uint16_t Poti2{ 1000 };
    uint16_t flightModeSwitch{ 1000 };
    uint16_t Switch2Way{ 1000 };
}rcSignals_st;

// Method to set up USART2 for RC IBus protocol receive
void SetupRC(void);

// Method to set RC values to init
void SetInitValues(void);

// Method to setup USART2 pin 
void SetupUsart2Pin(void);

// Method to set RC channels to init value
void SetChannels2Init(void);

// Function checks if IBus header 0x20-0x40 bytes are received on 0 and 1 position
// IbusVector must have at least to element 
bool IsIbusHeaderReceived(uint8_t* IbusVector);

// Method to get RC channel data consistently
// Must provide a vector with 14 elements
void getRcChannels(rcSignals_st* rcOut);

// Method to restart WDT timer to prevent interrupt
inline void RestartWDCtr(void);

#endif // !RC_HPP
