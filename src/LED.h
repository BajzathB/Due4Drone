// header to configure and set indicator LEDs

//LED_GYRO   - PC25 - TIOA6 - WHITE
//LED_ACC    - PC26 - TIOB6 - YELLOW
//LED_MAG    - PC28 - TIOA7 - RED
//LED_SDCARD - PD7  - TIOA8 - BLUE
//LED_GPS    - PD28 - TIOB8 - GREEN

#ifndef LED_HEADER
#define LED_HEADER

// Method to set up LED pins to work with TC peripherial
void SetupLED(void);

// Method to turn on Gyro LED
void LEDGyroOn(void);

// Method to turn off Gyro LED
void LEDGyroOff(void);

// Method to set blinking Gyro LED
void LEDGyroBlink(void);

// Method to turn on Acc LED
void LEDAccOn(void);

// Method to turn off Acc LED
void LEDAccOff(void);

// Method to set blinking Acc LED
void LEDAccBlink(void);

// Method to turn on Mag LED
void LEDMagOn(void);

// Method to turn off Mag LED
void LEDMagOff(void);

// Method to set blinking Mag LED
void LEDMagBlink(void);

// Method to turn on SD LED
void LEDSDOn(void);

// Method to turn off SD LED
void LEDSDOff(void);

// Method to set blinking SD LED
void LEDSDBlink(void);

// Method to set slow blinking SD LED
void LEDSDBlinkSlow(void);

// Method to turn on GPS LED
void LEDGPSOn(void);

// Method to turn off GPS LED
void LEDGPSOff(void);

// Method to set blinking GPS LED
void LEDGPSBlink(void);

// Method to set off to all LED
void LEDAllOff(void);

// Method to set blinking to all LED
void LEDAllBlink(void);

#endif // LED_HEADER
