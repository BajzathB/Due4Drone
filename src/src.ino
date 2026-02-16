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
  SPIInput spiInput;
  SPIOutput spiOutput;
  controllerIn_st controlIn;
  controllerOut_st controlOut;
  MotorInput motorInput;

  UpdateSysTime();
  controlIn.loopTime = getSysLoopTime();
  controlIn.sysTime = getSysTime();
  spiInput.sysTime = getSysTime();

  getRcChannels(&controlIn.rcSignals);
  getGyroAndAcc(&controlIn.gyro, &controlIn.acc);
  
  //debug
  // controlIn.rcSignals.throttle = 1000;
  // controlIn.rcSignals.roll = 1000;
  // controlIn.rcSignals.pitch = 1000;
  // controlIn.rcSignals.yaw = 1000;
  // controlIn.rcSignals.armStateSwitch = 1000;
  // controlIn.rcSignals.flightModeSwitch = 1000;
  // controlIn.rcSignals.Poti1 = 1000;
  // controlIn.rcSignals.Poti2 = 2000;
  // controlIn.rcSignals.Switch3Way2 = 1000;
  // controlIn.rcSignals.Switch2Way = 2000;

  RunController(&controlIn, &controlOut);

  motorInput.throttle = controlIn.rcSignals.throttle;
  motorInput.x = controlOut.U.x;
  motorInput.y = controlOut.U.y;
  motorInput.z = controlOut.U.z;
  motorInput.armState = controlOut.armState;
  motorInput.sysTime = spiInput.sysTime;
  motorInput.poti1 = controlIn.rcSignals.Poti1;
  motorInput.poti2 = controlIn.rcSignals.Poti2;
  motorInput.twoWaySwitch2 = controlIn.rcSignals.Switch2Way;
  
  UpdateMotorSpeeds(&motorInput);

  RunBT(&controlIn, &controlOut);

  spiInput.gyro = controlIn.gyro;
  spiInput.acc = controlIn.acc;
  spiInput.rcSignals = controlIn.rcSignals;

  //testing
  // if(spiInput.sysTime > 6)
  // {
	//   spiInput.rcSignals.measurementSwitch = 2000;
  //   spiInput.rcSignals.armStateSwitch = 2000;
  // }  
  // if(spiInput.sysTime > 12)
  // {
	//   spiInput.rcSignals.measurementSwitch = 1000;
  //   spiInput.rcSignals.armStateSwitch = 1000;
  // }

  RunSPI(&spiInput, &spiOutput);	//for sd card
  
  //SerialUSB.print(controlIn.rcSignals.roll); SerialUSB.print("\t");
  //SerialUSB.print(controlIn.rcSignals.pitch); SerialUSB.print("\t");
  //SerialUSB.print(controlIn.rcSignals.throttle); SerialUSB.print("\t");
  //SerialUSB.print(controlIn.rcSignals.yaw); SerialUSB.print("\t");
  //SerialUSB.print(controlIn.rcSignals.armStateSwitch); SerialUSB.print("\t");
  //SerialUSB.print(controlIn.rcSignals.flightModeSwitch); SerialUSB.print("\t");
  //SerialUSB.print(controlIn.rcSignals.Poti1); SerialUSB.print("\t");
  //SerialUSB.print(controlIn.rcSignals.Poti2); SerialUSB.print("\t");
  //SerialUSB.print(controlIn.rcSignals.Switch3Way2); SerialUSB.print("\t");
  //SerialUSB.println(controlIn.rcSignals.Switch2Way);

  //delay(2);
}
