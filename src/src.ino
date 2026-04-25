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
   
  //testing
  // if(spiInput.sysTime > 9)
  // {
  //   controlIn.rcSignals.throttle = 1100;
  //   controlIn.rcSignals.roll = 1500;
  //   controlIn.rcSignals.pitch = 1500;
  //   controlIn.rcSignals.yaw = 1500;
  //   controlIn.rcSignals.armStateSwitch = 2000;
  //   controlIn.rcSignals.measurementSwitch = 1000;
  //   controlIn.rcSignals.Poti1 = 1500;
  //   controlIn.rcSignals.Poti2 = 1600;
  //   controlIn.rcSignals.flightModeSwitch = 1000;
  //   controlIn.rcSignals.Switch2Way = 1000;
  // }
  // if(spiInput.sysTime > 12)
  // {
  //   controlIn.rcSignals.throttle = 1100;
  //   controlIn.rcSignals.roll = 1500;
  //   controlIn.rcSignals.pitch = 1500;
  //   controlIn.rcSignals.yaw = 1500;
  //   controlIn.rcSignals.armStateSwitch = 2000;
  //   controlIn.rcSignals.measurementSwitch = 1000;
  //   controlIn.rcSignals.Poti1 = 1500;
  //   controlIn.rcSignals.Poti2 = 1600;
  //   controlIn.rcSignals.flightModeSwitch = 1500;
  //   controlIn.rcSignals.Switch2Way = 1000;
  // }
  // if(spiInput.sysTime > 15)
  // {
  //   controlIn.rcSignals.throttle = 1000;
  //   controlIn.rcSignals.roll = 1500;
  //   controlIn.rcSignals.pitch = 1500;
  //   controlIn.rcSignals.yaw = 1500;
  //   controlIn.rcSignals.armStateSwitch = 1000;
  //   controlIn.rcSignals.measurementSwitch = 1000;
  //   controlIn.rcSignals.Poti1 = 1000;
  //   controlIn.rcSignals.Poti2 = 1000;
  //   controlIn.rcSignals.flightModeSwitch = 1000;
  //   controlIn.rcSignals.Switch2Way = 1000;
  // }

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
  spiInput.rcSignals.throttle = controlIn.rcSignals.throttle;
  spiInput.rcSignals.roll = controlIn.rcSignals.roll;
  spiInput.rcSignals.pitch = controlIn.rcSignals.pitch;
  spiInput.rcSignals.yaw = controlIn.rcSignals.yaw;
  spiInput.rcSignals.armStateSwitch = controlIn.rcSignals.armStateSwitch;
  spiInput.rcSignals.measurementSwitch = controlIn.rcSignals.measurementSwitch;
  spiInput.rcSignals.Poti1 = controlIn.rcSignals.Poti1;
  spiInput.rcSignals.Poti2 = controlIn.rcSignals.Poti2;
  spiInput.rcSignals.flightModeSwitch = controlIn.rcSignals.flightModeSwitch;
  spiInput.rcSignals.Switch2Way = controlIn.rcSignals.Switch2Way;

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
