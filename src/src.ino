// This is the code for due4drone designed PCB


#include "LED.h"
#include "sysTime.h"
#include "MotorControl.h"
#include "SPI.h"
#include "RC.h"
#include "Controller.h"
#include "BT.h"



void setup()
{
  
  SerialUSB.begin(250000);
  Serial.begin(250000);

  SerialUSB.println("D4D startup!");
  
  SetupMotorPins();
  
  SetupLED();

  //TODO: remove LED turnoff when sensor config is ready
  LEDMagOff();
  LEDSDOff();
  LEDGPSOff();

  SetupSysTimer();

  SetupSPI();

  SetupRC();

  SetupController();

  SetupBT();
}

void loop()
{
  //SPIInput spiInput;
  //SPIOutput spiOutput;
  //controllerIn_st controlIn;
  //controllerOut_st controlOut;
  droneTimes_st times;

  UpdateSysTime(&times);
  //spiInput.sysTick = times.sysTick;

  ControllerDebug();
  //RunController(&controlIn, &controlOut);

  RunBT();

  RunSPI();	//for sd card
  
  //delay(2);
}
