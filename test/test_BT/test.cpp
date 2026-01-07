#include "pch.h"

#include "../../src/BT.h"
#include "../../src/BT_IDs.h"
#include "../../src/Controller.h"
#include "../../src/sysTime.h"
#include "../helper/support4Testing.h"

extern bt BT;
extern Usart* USART1;
extern pid_st pidRate;
extern sysTime sysTimer;

TEST(test_BT, BT_Call) 
{
	SetupBT();
	controllerIn_st testIn;
	controllerOut_st testOut;
	RunBT(&testIn, &testOut);
	USART1_Handler();
	BTReceive();
	BTTransmit(&testIn, &testOut);
	ProcessRxFrame();
	buffer_ testBuffer;
	testBuffer.vector[3] = '1';
	testBuffer.vector[4] = '\r';
	testBuffer.vector[5] = '\n';
	testBuffer.ctr = 6;
	ConvertStrToBool(&testBuffer);
	ConvertStrToUint8(&testBuffer);
	ConvertStrToUint16(&testBuffer);
	ConvertStrToFloat(&testBuffer);
	SetSteamFlag(0);
	SetSteamFlag2(0);
	SetStreamData(0,0);
}

TEST(test_BT, USART1_Handler_Test)
{
	//1st
	USART1->US_CSR = 0;
	BT.rxDataState = WAITING_FOR_NUMBEROFBYTES;
	USART1_Handler();
	EXPECT_EQ(BT.rxDataState, WAITING_FOR_NUMBEROFBYTES);

	//2nd
	USART1->US_CSR = US_CSR_ENDRX;
	BT.rxDataState = WAITING_FOR_NUMBEROFBYTES;
	USART1_Handler();
	EXPECT_EQ(BT.rxDataState, RECEVING_DATA_BYTES);
	USART1_Handler();
	EXPECT_EQ(BT.rxDataState, FRAME_RECEIVED);
	
	//3rd
	BT.rxDataState = WAITING_FOR_NUMBEROFBYTES;
	USART1_Handler();
	EXPECT_EQ(BT.rxDataState, RECEVING_DATA_BYTES);
}

TEST(test_BT, ConvertStrToBool_Test)
{
	buffer_ testBuffer;

	//1st: false
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '0';
	testBuffer.vector[4] = '\r';
	testBuffer.vector[5] = '\n';
	testBuffer.ctr = 6;
	EXPECT_EQ(ConvertStrToBool(&testBuffer), false);

	//2nd: true
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '1';
	testBuffer.vector[4] = '\r';
	testBuffer.vector[5] = '\n';
	testBuffer.ctr = 6;
	EXPECT_EQ(ConvertStrToBool(&testBuffer), true);

	//3rd: true
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '2';
	testBuffer.vector[4] = '\r';
	testBuffer.vector[5] = '\n';
	testBuffer.ctr = 6;
	EXPECT_EQ(ConvertStrToBool(&testBuffer), true);
}

TEST(test_BT, ConvertStrToUint8_Test)
{
	buffer_ testBuffer;
	//1st
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '1';
	testBuffer.vector[4] = '2';
	testBuffer.vector[5] = '3';
	testBuffer.vector[6] = '\r';
	testBuffer.vector[7] = '\n';
	testBuffer.ctr = 8;
	EXPECT_EQ(ConvertStrToUint8(&testBuffer), 123);

	//2nd
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '9';
	testBuffer.vector[4] = '8';
	testBuffer.vector[5] = '\r';
	testBuffer.vector[6] = '\n';
	testBuffer.ctr = 7;
	EXPECT_EQ(ConvertStrToUint8(&testBuffer), 98);

	//3rd
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '2';
	testBuffer.vector[4] = '5';
	testBuffer.vector[5] = '5';
	testBuffer.vector[6] = '\r';
	testBuffer.vector[7] = '\n';
	testBuffer.ctr = 8;
	EXPECT_EQ(ConvertStrToUint8(&testBuffer), 255);
}

TEST(test_BT, ConvertStrToUint16_Test)
{
	buffer_ testBuffer;
	//1st
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '1';
	testBuffer.vector[4] = '2';
	testBuffer.vector[5] = '3';
	testBuffer.vector[6] = '\r';
	testBuffer.vector[7] = '\n';
	testBuffer.ctr = 8;
	EXPECT_EQ(ConvertStrToUint16(&testBuffer), 123);

	//2nd
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '5';
	testBuffer.vector[4] = '\r';
	testBuffer.vector[5] = '\n';
	testBuffer.ctr = 6;
	EXPECT_EQ(ConvertStrToUint16(&testBuffer), 5);

	//3rd
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '2';
	testBuffer.vector[4] = '3';
	testBuffer.vector[5] = '4';
	testBuffer.vector[6] = '5';
	testBuffer.vector[7] = '6';
	testBuffer.vector[8] = '\r';
	testBuffer.vector[9] = '\n';
	testBuffer.ctr = 10;
	EXPECT_EQ(ConvertStrToUint16(&testBuffer), 23456);

	//4th
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '6';
	testBuffer.vector[4] = '5';
	testBuffer.vector[5] = '5';
	testBuffer.vector[6] = '3';
	testBuffer.vector[7] = '4';
	testBuffer.vector[8] = '\r';
	testBuffer.vector[9] = '\n';
	testBuffer.ctr = 10;
	EXPECT_EQ(ConvertStrToUint16(&testBuffer), 65534);
}

TEST(test_BT, ConvertStrToFloat_Test)
{
	buffer_ testBuffer;

	//1st
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '1';
	testBuffer.vector[4] = '2';
	testBuffer.vector[5] = '3';
	testBuffer.vector[6] = '\r';
	testBuffer.vector[7] = '\n';
	testBuffer.ctr = 8;
	EXPECT_EQ(ConvertStrToFloat(&testBuffer), 123);

	//2nd
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '4';
	testBuffer.vector[4] = '3';
	testBuffer.vector[5] = '2';
	testBuffer.vector[6] = '1';
	testBuffer.vector[7] = '0';
	testBuffer.vector[8] = '9';
	testBuffer.vector[9] = '\r';
	testBuffer.vector[10] = '\n';
	testBuffer.ctr = 11;
	EXPECT_EQ(ConvertStrToFloat(&testBuffer), 432109);

	//3rd
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '2';
	testBuffer.vector[4] = '.';
	testBuffer.vector[5] = '5';
	testBuffer.vector[6] = '\r';
	testBuffer.vector[7] = '\n';
	testBuffer.ctr = 8;
	EXPECT_EQ(ConvertStrToFloat(&testBuffer), 2.5);

	//4th
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '-';
	testBuffer.vector[4] = '6';
	testBuffer.vector[5] = '7';
	testBuffer.vector[6] = '.';
	testBuffer.vector[7] = '9';
	testBuffer.vector[8] = '3';
	testBuffer.vector[9] = '1';
	testBuffer.vector[10] = '\r';
	testBuffer.vector[11] = '\n';
	testBuffer.ctr = 12;
	EXPECT_NEAR(ConvertStrToFloat(&testBuffer), -67.931, 0.001);

	//5th
	testBuffer.vector[0] = 0;
	testBuffer.vector[1] = 0;
	testBuffer.vector[2] = 0;
	testBuffer.vector[3] = '2';
	testBuffer.vector[4] = '8';
	testBuffer.vector[5] = '3';
	testBuffer.vector[6] = '1';
	testBuffer.vector[7] = '.';
	testBuffer.vector[8] = '9';
	testBuffer.vector[9] = '3';
	testBuffer.vector[10] = '1';
	testBuffer.vector[11] = '7';
	testBuffer.vector[12] = '4';
	testBuffer.vector[13] = '\r';
	testBuffer.vector[14] = '\n';
	testBuffer.ctr = 15;
	EXPECT_NEAR(ConvertStrToFloat(&testBuffer), 2831.93174, 0.0001);
}

TEST(test_BT, SetSteamFlag_Test)
{

    //1st
    BT.input.vector[3] = 'O';
    SetSteamFlag(ID_pidRate_sensor_signal_X);
    EXPECT_EQ(BT.txFrame.streamDataFlags, 1);

	//2nd
	BT.input.vector[3] = 'F';
	SetSteamFlag(ID_pidRate_sensor_signal_X);
	EXPECT_EQ(BT.txFrame.streamDataFlags, 0);

	//3rd
	BT.input.vector[3] = 'O';
	SetSteamFlag(ID_pidRate_sensor_signal_Y);
	EXPECT_EQ(BT.txFrame.streamDataFlags, 1 << 1);

	//4th
	BT.input.vector[3] = 'F';
	SetSteamFlag(ID_pidRate_sensor_signal_Y);
	EXPECT_EQ(BT.txFrame.streamDataFlags, 0);

	//5th
	BT.input.vector[3] = 'O';
	SetSteamFlag(ID_pidRate_sensor_signal_Z);
	EXPECT_EQ(BT.txFrame.streamDataFlags, 1 << 2);

	//6th
	BT.input.vector[3] = 'F';
	SetSteamFlag(ID_pidRate_sensor_signal_Z);
	EXPECT_EQ(BT.txFrame.streamDataFlags, 0);

	//7th
	BT.input.vector[3] = 'O';
	SetSteamFlag(ID_gyro_signal_X);
	EXPECT_EQ(BT.txFrame.streamDataFlags, 1 << 3);

	//8th
	BT.input.vector[3] = 'F';
	SetSteamFlag(ID_gyro_signal_X);
	EXPECT_EQ(BT.txFrame.streamDataFlags, 0);

	//9th
	BT.input.vector[3] = 'O';
	SetSteamFlag(ID_gyro_signal_Y);
	EXPECT_EQ(BT.txFrame.streamDataFlags, 1 << 4);

	//10th
	BT.input.vector[3] = 'F';
	SetSteamFlag(ID_gyro_signal_Y);
	EXPECT_EQ(BT.txFrame.streamDataFlags, 0);

	//11th
	BT.input.vector[3] = 'O';
	SetSteamFlag(ID_gyro_signal_Z);
	EXPECT_EQ(BT.txFrame.streamDataFlags, 1 << 5);

	//12th
	BT.input.vector[3] = 'F';
	SetSteamFlag(ID_gyro_signal_Z);
	EXPECT_EQ(BT.txFrame.streamDataFlags, 0);

    //1st
    BT.input.vector[3] = 'O';
    SetSteamFlag(ID_pidRate_sensor_signal_X);
    SetSteamFlag(ID_gyro_signal_X);
    SetSteamFlag(ID_gyro_signal_Z);
    EXPECT_EQ(BT.txFrame.streamDataFlags, 0b101001);
}

TEST(test_BT, SetStreamData_Test)
{
	BT.output.vector[0] = '$';

	//1st
	BT.output.vector[1] = 0;
	BT.output.vector[2] = 0;
	BT.output.vector[3] = 0;
	BT.output.vector[4] = 0;
	BT.output.vector[5] = 0;
	BT.output.vector[6] = 0;
	BT.output.vector[7] = 0;
	BT.output.vector[8] = 0;
	BT.output.vector[9] = 0;
	BT.output.vector[10] = 0;
	BT.output.vector[11] = 0;
	BT.output.ctr = 1;
	SetStreamData(10, 1.234);
	EXPECT_EQ(BT.output.vector[1], 10);
	EXPECT_EQ(BT.output.vector[2], 0);
	EXPECT_EQ(BT.output.vector[3], '1');
	EXPECT_EQ(BT.output.vector[4], '.');
	EXPECT_EQ(BT.output.vector[5], '2');
	EXPECT_EQ(BT.output.vector[6], '3');
	EXPECT_EQ(BT.output.vector[7], '4');
	EXPECT_EQ(BT.output.vector[8], '!');

	//2nd
	BT.output.vector[1] = 0;
	BT.output.vector[2] = 0;
	BT.output.vector[3] = 0;
	BT.output.vector[4] = 0;
	BT.output.vector[5] = 0;
	BT.output.vector[6] = 0;
	BT.output.vector[7] = 0;
	BT.output.vector[8] = 0;
	BT.output.vector[9] = 0;
	BT.output.vector[10] = 0;
	BT.output.vector[11] = 0;
	BT.output.ctr = 1;
	SetStreamData(234, 12.345);
	EXPECT_EQ(BT.output.vector[1], 234);
	EXPECT_EQ(BT.output.vector[2], 0);
	EXPECT_EQ(BT.output.vector[3], '1');
	EXPECT_EQ(BT.output.vector[4], '2');
	EXPECT_EQ(BT.output.vector[5], '.');
	EXPECT_EQ(BT.output.vector[6], '3');
	EXPECT_EQ(BT.output.vector[7], '4');
	EXPECT_EQ(BT.output.vector[8], '5');
	EXPECT_EQ(BT.output.vector[9], '!');

	//3rd
	BT.output.vector[1] = 0;
	BT.output.vector[2] = 0;
	BT.output.vector[3] = 0;
	BT.output.vector[4] = 0;
	BT.output.vector[5] = 0;
	BT.output.vector[6] = 0;
	BT.output.vector[7] = 0;
	BT.output.vector[8] = 0;
	BT.output.vector[9] = 0;
	BT.output.vector[10] = 0;
	BT.output.vector[11] = 0;
	BT.output.ctr = 1;
	SetStreamData(3003, 987.654);
	EXPECT_EQ(BT.output.vector[1], 0xBB);
	EXPECT_EQ(BT.output.vector[2], 0x0B);
	EXPECT_EQ(BT.output.vector[3], '9');
	EXPECT_EQ(BT.output.vector[4], '8');
	EXPECT_EQ(BT.output.vector[5], '7');
	EXPECT_EQ(BT.output.vector[6], '.');
	EXPECT_EQ(BT.output.vector[7], '6');
	EXPECT_EQ(BT.output.vector[8], '5');
	EXPECT_EQ(BT.output.vector[9], '4');
	EXPECT_EQ(BT.output.vector[10], '!');

	//4th
	BT.output.vector[1] = 0;
	BT.output.vector[2] = 0;
	BT.output.vector[3] = 0;
	BT.output.vector[4] = 0;
	BT.output.vector[5] = 0;
	BT.output.vector[6] = 0;
	BT.output.vector[7] = 0;
	BT.output.vector[8] = 0;
	BT.output.vector[9] = 0;
	BT.output.vector[10] = 0;
	BT.output.vector[11] = 0;
	BT.output.ctr = 1;
	SetStreamData(4000, 1875.000);
	EXPECT_EQ(BT.output.vector[1], 0xA0);
	EXPECT_EQ(BT.output.vector[2], 0x0F);
	EXPECT_EQ(BT.output.vector[3], '1');
	EXPECT_EQ(BT.output.vector[4], '8');
	EXPECT_EQ(BT.output.vector[5], '7');
	EXPECT_EQ(BT.output.vector[6], '5');
	EXPECT_EQ(BT.output.vector[7], '.');
	EXPECT_EQ(BT.output.vector[8], '0');
	EXPECT_EQ(BT.output.vector[9], '0');
	EXPECT_EQ(BT.output.vector[10], '0');
	EXPECT_EQ(BT.output.vector[11], '!');
}

TEST(test_BT, ProcessRxFrame_Test)
{
	pid_st* testPID{ getPIDrates() };

	//1st: no frame end
	BT.input.vector[0] = CMD_NONE;
	BT.input.vector[1] = 0;
	BT.input.vector[2] = 0;
	BT.input.vector[3] = '1';
	BT.input.ctr = 4;
	BT.rxFrame.cmd = CMD_NONE;
	BT.rxFrame.id = 0;
	ProcessRxFrame();
	EXPECT_EQ(BT.rxFrame.cmd, CMD_NONE);
	EXPECT_EQ(BT.rxFrame.id, 0);

	//2nd: no cmd
	BT.input.vector[0] = CMD_NONE;
	BT.input.vector[1] = 0;
	BT.input.vector[2] = 0;
	BT.input.vector[3] = '1';
	BT.input.vector[4] = '\r';
	BT.input.vector[5] = '\n';
	BT.input.ctr = 6;
	BT.rxFrame.cmd = CMD_NONE;
	BT.rxFrame.id = 0;
	ProcessRxFrame();
	EXPECT_EQ(BT.rxFrame.cmd, CMD_NONE);
	EXPECT_EQ(BT.rxFrame.id, 0);

	//3rd: get
	BT.input.vector[0] = CMD_GET;
	BT.input.vector[1] = 0xCF;
	BT.input.vector[2] = 7;
	BT.input.vector[3] = '1';
	BT.input.vector[4] = '\r';
	BT.input.vector[5] = '\n';
	BT.input.ctr = 6;
	BT.rxFrame.cmd = CMD_NONE;
	BT.rxFrame.id = 0;
	ProcessRxFrame();
	EXPECT_EQ(BT.rxFrame.cmd, CMD_GET);
	EXPECT_EQ(BT.rxFrame.id, 1999);
	EXPECT_EQ(BT.txFrame.sendParam, false);

	//4th: get pid rate p x
	BT.input.vector[0] = CMD_GET;
	BT.input.vector[1] = 0xD0;
	BT.input.vector[2] = 0x07;
	BT.input.vector[3] = '\r';
	BT.input.vector[4] = '\n';
	BT.input.ctr = 5;
	BT.rxFrame.cmd = CMD_NONE;
	BT.rxFrame.id = 0;
	testPID->P.x = 55.0f;
	ProcessRxFrame();
	EXPECT_EQ(BT.rxFrame.cmd, CMD_GET);
	EXPECT_EQ(BT.rxFrame.id, 2000);
	EXPECT_EQ(BT.txFrame.sendParam, true);
	EXPECT_EQ(BT.txFrame.paramData, 55.0f);

	//5th: stream
	BT.input.vector[0] = CMD_STREAM;
	BT.input.vector[1] = 5;
	BT.input.vector[2] = 0;
	BT.input.vector[3] = '1';
	BT.input.vector[4] = '\r';
	BT.input.vector[5] = '\n';
	BT.input.ctr = 6;
	BT.rxFrame.cmd = CMD_NONE;
	BT.rxFrame.id = 0;
	ProcessRxFrame();
	EXPECT_EQ(BT.rxFrame.cmd, CMD_STREAM);
	EXPECT_EQ(BT.rxFrame.id, 5);

	//5th set
	BT.input.vector[0] = CMD_SET;
	BT.input.vector[1] = 100;
	BT.input.vector[2] = 0;
	BT.input.vector[3] = '1';
	BT.input.vector[4] = '\r';
	BT.input.vector[5] = '\n';
	BT.input.ctr = 6;
	BT.rxFrame.cmd = CMD_NONE;
	BT.rxFrame.id = 0;
	ProcessRxFrame();
	EXPECT_EQ(BT.rxFrame.cmd, CMD_SET);
	EXPECT_EQ(BT.rxFrame.id, 100);
	
	//6th set pid rate p x
	BT.input.vector[0] = CMD_SET;
	BT.input.vector[1] = 0xD0;
	BT.input.vector[2] = 0x07;
	BT.input.vector[3] = '4';
	BT.input.vector[4] = '8';
	BT.input.vector[5] = '.';
	BT.input.vector[6] = '9';
	BT.input.vector[7] = '2';
	BT.input.vector[8] = '\r';
	BT.input.vector[9] = '\n';
	BT.input.ctr = 10;
	BT.rxFrame.cmd = CMD_NONE;
	BT.rxFrame.id = 0;
	ProcessRxFrame();
	EXPECT_EQ(BT.rxFrame.cmd, CMD_SET);
	EXPECT_EQ(BT.rxFrame.id, ID_control_PID_rate_P_X);
	EXPECT_NEAR(testPID->P.x, 48.92, 0.01);

	//7th stream sensor x
	BT.txFrame.streamDataFlags = 0;
	BT.input.vector[0] = CMD_STREAM;
	BT.input.vector[1] = 0xB8;
	BT.input.vector[2] = 0x0B;
	BT.input.vector[3] = 'O';
	BT.input.vector[4] = '\r';
	BT.input.vector[5] = '\n';
	BT.input.ctr = 6;
	BT.rxFrame.cmd = CMD_NONE;
	BT.rxFrame.id = 0;
	ProcessRxFrame();
	EXPECT_EQ(BT.rxFrame.cmd, CMD_STREAM);
	EXPECT_EQ(BT.rxFrame.id, ID_pidRate_sensor_signal_X);
	EXPECT_EQ(BT.txFrame.streamDataFlags, 1);

	//8th stream gyro x too
	BT.input.vector[0] = CMD_STREAM;
	BT.input.vector[1] = 0xBB;
	BT.input.vector[2] = 0x0B;
	BT.input.vector[3] = 'O';
	BT.input.vector[8] = '\r';
	BT.input.vector[9] = '\n';
	BT.input.ctr = 10;
	BT.rxFrame.cmd = CMD_NONE;
	BT.rxFrame.id = 0;
	ProcessRxFrame();
	EXPECT_EQ(BT.rxFrame.cmd, CMD_STREAM);
	EXPECT_EQ(BT.rxFrame.id, ID_gyro_signal_X);
	EXPECT_EQ(BT.txFrame.streamDataFlags, 9);

    //9th stream sensor x, turn off
    BT.input.vector[0] = CMD_STREAM;
    BT.input.vector[1] = 0xB8;
    BT.input.vector[2] = 0x0B;
    BT.input.vector[3] = 'F';
    BT.input.vector[4] = '\r';
    BT.input.vector[5] = '\n';
    BT.input.ctr = 6;
    BT.rxFrame.cmd = CMD_NONE;
    BT.rxFrame.id = 0;
    ProcessRxFrame();
    EXPECT_EQ(BT.rxFrame.cmd, CMD_STREAM);
    EXPECT_EQ(BT.rxFrame.id, ID_pidRate_sensor_signal_X);
    EXPECT_EQ(BT.txFrame.streamDataFlags, 8);
}

TEST(test_BT, CalcCharAndFillOutput_Test)
{
	float testNumber{ 0 };
	uint8_t numberOfFrac{ 0 };

	//1st: 1 digit intiger
	testNumber = 4;
	numberOfFrac = 0;
	BT.output.ctr = 0;
	BT.output.vector[BT.output.ctr++] = 0;
	BT.output.vector[BT.output.ctr++] = 'P';
	CalcCharAndFillOutput(testNumber, numberOfFrac);
	EXPECT_EQ(BT.output.vector[2], '4');

	//2nd: 2 digit intiger
	testNumber = 67;
	numberOfFrac = 0;
	BT.output.ctr = 0;
	BT.output.vector[BT.output.ctr++] = 0;
	BT.output.vector[BT.output.ctr++] = 'P';
	CalcCharAndFillOutput(testNumber, numberOfFrac);
	EXPECT_EQ(BT.output.vector[2], '6');
	EXPECT_EQ(BT.output.vector[3], '7');

	//3rd: 3 digit intiger
	testNumber = 159;
	numberOfFrac = 0;
	BT.output.ctr = 0;
	BT.output.vector[BT.output.ctr++] = 0;
	BT.output.vector[BT.output.ctr++] = 'P';
	CalcCharAndFillOutput(testNumber, numberOfFrac);
	EXPECT_EQ(BT.output.vector[2], '1');
	EXPECT_EQ(BT.output.vector[3], '5');
	EXPECT_EQ(BT.output.vector[4], '9');

	//4th: 4 digit intiger
	testNumber = 1548;
	numberOfFrac = 0;
	BT.output.ctr = 0;
	BT.output.vector[BT.output.ctr++] = 0;
	BT.output.vector[BT.output.ctr++] = 'P';
	CalcCharAndFillOutput(testNumber, numberOfFrac);
	EXPECT_EQ(BT.output.vector[2], '1');
	EXPECT_EQ(BT.output.vector[3], '5');
	EXPECT_EQ(BT.output.vector[4], '4');
	EXPECT_EQ(BT.output.vector[5], '8');

	//5th: 1-1
	testNumber = 1.6;
	numberOfFrac = 1;
	BT.output.ctr = 0;
	BT.output.vector[BT.output.ctr++] = 0;
	BT.output.vector[BT.output.ctr++] = 'P';
	CalcCharAndFillOutput(testNumber, numberOfFrac);
	EXPECT_EQ(BT.output.vector[2], '1');
	EXPECT_EQ(BT.output.vector[3], '.');
	EXPECT_EQ(BT.output.vector[4], '6');

	//6th: 2-2
	testNumber = 75.26;
	numberOfFrac = 2;
	BT.output.ctr = 0;
	BT.output.vector[BT.output.ctr++] = 0;
	BT.output.vector[BT.output.ctr++] = 'P';
	CalcCharAndFillOutput(testNumber, numberOfFrac);
	EXPECT_EQ(BT.output.vector[2], '7');
	EXPECT_EQ(BT.output.vector[3], '5');
	EXPECT_EQ(BT.output.vector[4], '.');
	EXPECT_EQ(BT.output.vector[5], '2');
	EXPECT_EQ(BT.output.vector[6], '6');

	//7th: 3-3
	testNumber = 478.348;
	numberOfFrac = 3;
	BT.output.ctr = 0;
	BT.output.vector[BT.output.ctr++] = 0;
	BT.output.vector[BT.output.ctr++] = 'P';
	CalcCharAndFillOutput(testNumber, numberOfFrac);
	EXPECT_EQ(BT.output.vector[2], '4');
	EXPECT_EQ(BT.output.vector[3], '7');
	EXPECT_EQ(BT.output.vector[4], '8');
	EXPECT_EQ(BT.output.vector[5], '.');
	EXPECT_EQ(BT.output.vector[6], '3');
	EXPECT_EQ(BT.output.vector[7], '4');
	EXPECT_EQ(BT.output.vector[8], '8');

	//8th: 4-3 + negative
	testNumber = -1478.348;
	numberOfFrac = 4;
	BT.output.ctr = 0;
	BT.output.vector[BT.output.ctr++] = 0;
	BT.output.vector[BT.output.ctr++] = 'P';
	CalcCharAndFillOutput(testNumber, numberOfFrac);
	EXPECT_EQ(BT.output.vector[2], '-');
	EXPECT_EQ(BT.output.vector[3], '1');
	EXPECT_EQ(BT.output.vector[4], '4');
	EXPECT_EQ(BT.output.vector[5], '7');
	EXPECT_EQ(BT.output.vector[6], '8');
	EXPECT_EQ(BT.output.vector[7], '.');
	EXPECT_EQ(BT.output.vector[8], '3');
	EXPECT_EQ(BT.output.vector[9], '4');
	EXPECT_EQ(BT.output.vector[10], '8');

	//9th: smaller then 1, 1 fracional
	testNumber = 0.5;
	numberOfFrac = 4;
	BT.output.ctr = 0;
	BT.output.vector[BT.output.ctr++] = 0;
	BT.output.vector[BT.output.ctr++] = 'P';
	CalcCharAndFillOutput(testNumber, numberOfFrac);
	EXPECT_EQ(BT.output.vector[2], '0');
	EXPECT_EQ(BT.output.vector[3], '.');
	EXPECT_EQ(BT.output.vector[4], '5');

	//10th: smaller then 1, 2 fracional, negative
	testNumber = -0.475;
	numberOfFrac = 4;
	BT.output.ctr = 0;
	BT.output.vector[BT.output.ctr++] = 0;
	BT.output.vector[BT.output.ctr++] = 'P';
	CalcCharAndFillOutput(testNumber, numberOfFrac);
	EXPECT_EQ(BT.output.vector[2], '-');
	EXPECT_EQ(BT.output.vector[3], '0');
	EXPECT_EQ(BT.output.vector[4], '.');
	EXPECT_EQ(BT.output.vector[5], '4');
	EXPECT_EQ(BT.output.vector[6], '7');

	//11th: smaller then 1, 3 fracional, negative
	testNumber = -0.9781;
	numberOfFrac = 4;
	BT.output.ctr = 0;
	BT.output.vector[BT.output.ctr++] = 0;
	BT.output.vector[BT.output.ctr++] = 'P';
	CalcCharAndFillOutput(testNumber, numberOfFrac);
	EXPECT_EQ(BT.output.vector[2], '-');
	EXPECT_EQ(BT.output.vector[3], '0');
	EXPECT_EQ(BT.output.vector[4], '.');
	EXPECT_EQ(BT.output.vector[5], '9');
	EXPECT_EQ(BT.output.vector[6], '7');
	EXPECT_EQ(BT.output.vector[7], '8');
}

TEST(test_BT, BTTransmit_Test)
{
	controllerIn_st testIn;
	controllerOut_st testOut;
	BT.txFrame.streamDataFlags = 0;
	BT.output.vector[1] = '$';
	BT.output.vector[2] = 0;
	BT.output.vector[3] = 0;
	BT.output.vector[4] = 0;
	BT.output.vector[5] = 0;

    //0st: deltaT small
    sysTimer.loopTime = 0.005;
    BT.txDeltaT = 0;
    BTTransmit(&testIn, &testOut);
    EXPECT_NEAR(BT.txDeltaT, 0.005, 0.0005);
    sysTimer.loopTime = 0.001;
    BTTransmit(&testIn, &testOut);
    EXPECT_NEAR(BT.txDeltaT, 0.006, 0.0005);

	//1st: no send
    sysTimer.loopTime = 0.005;
	BT.txFrame.sendParam = false;
	BT.txFrame.paramData = 0;
	BT.txFrame.numberOfFrac = 0;
	BTTransmit(&testIn, &testOut);
	EXPECT_EQ(BT.output.vector[2], 0);
	EXPECT_EQ(BT.output.vector[3], 0);
    EXPECT_NEAR(BT.txDeltaT, 0.011, 0.0005);

	//2nd: send param, 1 digit
    BT.txDeltaT = 0.012;
	BT.txFrame.sendParam = true;
	BT.txFrame.paramData = 5;
	BT.txFrame.numberOfFrac = 0;
	BTTransmit(&testIn, &testOut);
	EXPECT_EQ(BT.output.vector[0], 5);
	EXPECT_EQ(BT.output.vector[1], '$');
	EXPECT_EQ(BT.output.vector[2], 'P');
	EXPECT_EQ(BT.output.vector[3], '5');
	EXPECT_EQ(BT.output.vector[4], '\r');
	EXPECT_EQ(BT.output.vector[5], '\n');
    EXPECT_NEAR(BT.txDeltaT, 0.0f, 0.0005);

	//3rd: send param, 4 digit
    BT.txDeltaT = 0.012;
	BT.txFrame.sendParam = true;
	BT.txFrame.paramData = 1657;
	BT.txFrame.numberOfFrac = 0;
	BTTransmit(&testIn, &testOut);
	EXPECT_EQ(BT.output.vector[0], 8);
	EXPECT_EQ(BT.output.vector[1], '$');
	EXPECT_EQ(BT.output.vector[2], 'P');
	EXPECT_EQ(BT.output.vector[3], '1');
	EXPECT_EQ(BT.output.vector[4], '6');
	EXPECT_EQ(BT.output.vector[5], '5');
	EXPECT_EQ(BT.output.vector[6], '7');
	EXPECT_EQ(BT.output.vector[7], '\r');
	EXPECT_EQ(BT.output.vector[8], '\n');

	//4th: send param, 1-3, negative
    BT.txDeltaT = 0.012;
	BT.txFrame.sendParam = true;
	BT.txFrame.paramData = -1.4985;
	BT.txFrame.numberOfFrac = 3;
	BTTransmit(&testIn, &testOut);
	EXPECT_EQ(BT.output.vector[0], 10);
	EXPECT_EQ(BT.output.vector[1], '$');
	EXPECT_EQ(BT.output.vector[2], 'P');
	EXPECT_EQ(BT.output.vector[3], '-');
	EXPECT_EQ(BT.output.vector[4], '1');
	EXPECT_EQ(BT.output.vector[5], '.');
	EXPECT_EQ(BT.output.vector[6], '4');
	EXPECT_EQ(BT.output.vector[7], '9');
	EXPECT_EQ(BT.output.vector[8], '8');
	EXPECT_EQ(BT.output.vector[9], '\r');
	EXPECT_EQ(BT.output.vector[10], '\n');

	uint8_t diff = ID_pitch_PT2 - ID_bitshift_substracter;
	uint64_t shift = uint64_t(1) << (ID_pitch_PT2 - ID_bitshift_substracter);
	BT.txFrame.streamDataFlags = shift;
	uint64_t flag = (BT.txFrame.streamDataFlags & (uint64_t(1) << (ID_pitch_PT2 - ID_bitshift_substracter))) > 0;


	//5th: send 1 stream data
    BT.txDeltaT = 0.012;
	BT.txFrame.sendParam = false;
	BT.txFrame.streamDataFlags = 1;
	BT.output.vector[0] = 0;
	BT.output.vector[1] = '$';
	BT.output.vector[2] = 0;
	BT.output.vector[3] = 0;
	BT.output.vector[4] = 0;
	BT.output.vector[5] = 0;
	pidRate.sensor.signal.x = 1.234f;
	BTTransmit(&testIn, &testOut);
	EXPECT_EQ(BT.output.vector[0], 12);
	EXPECT_EQ(BT.output.vector[1], '$');
	EXPECT_EQ(BT.output.vector[2], 'S');
	EXPECT_EQ(BT.output.vector[3], uint8_t(ID_pidRate_sensor_signal_X & 0xFF));
	EXPECT_EQ(BT.output.vector[4], uint8_t((ID_pidRate_sensor_signal_X & 0xFF00) >> 8));
	EXPECT_EQ(BT.output.vector[5], '1');
	EXPECT_EQ(BT.output.vector[6], '.');
	EXPECT_EQ(BT.output.vector[7], '2');
	EXPECT_EQ(BT.output.vector[8], '3');
	EXPECT_EQ(BT.output.vector[9], '4');
	EXPECT_EQ(BT.output.vector[10], '!');
	EXPECT_EQ(BT.output.vector[11], '\r');
	EXPECT_EQ(BT.output.vector[12], '\n');

	//6th: send 1 stream data again
    BT.txDeltaT = 0.012;
	BT.txFrame.sendParam = false;
	BT.txFrame.streamDataFlags = 1;
	pidRate.sensor.signal.x = 5.678f;
	BTTransmit(&testIn, &testOut);
	EXPECT_EQ(BT.output.vector[0], 12);
	EXPECT_EQ(BT.output.vector[1], '$');
	EXPECT_EQ(BT.output.vector[2], 'S');
	EXPECT_EQ(BT.output.vector[3], uint8_t(ID_pidRate_sensor_signal_X & 0xFF));
	EXPECT_EQ(BT.output.vector[4], uint8_t((ID_pidRate_sensor_signal_X & 0xFF00) >> 8));
	EXPECT_EQ(BT.output.vector[5], '5');
	EXPECT_EQ(BT.output.vector[6], '.');
	EXPECT_EQ(BT.output.vector[7], '6');
	EXPECT_EQ(BT.output.vector[8], '7');
	EXPECT_EQ(BT.output.vector[9], '8');
	EXPECT_EQ(BT.output.vector[10], '!');
	EXPECT_EQ(BT.output.vector[11], '\r');
	EXPECT_EQ(BT.output.vector[12], '\n');

	//7th: send 2 stream data
    BT.txDeltaT = 0.012;
	BT.txFrame.sendParam = false;
	BT.txFrame.streamDataFlags = 0b1001;
	BT.output.vector[0] = 0;
	BT.output.vector[1] = '$';
	BT.output.vector[2] = 0;
	BT.output.vector[3] = 0;
	BT.output.vector[4] = 0;
	BT.output.vector[5] = 0;
	pidRate.sensor.signal.x = 1.234f;
	testIn.gyro.signal.x = 2.345f;
	BTTransmit(&testIn, &testOut);
	EXPECT_EQ(BT.output.vector[0], 20);
	EXPECT_EQ(BT.output.vector[1], '$');
	EXPECT_EQ(BT.output.vector[2], 'S');
	EXPECT_EQ(BT.output.vector[3], uint8_t(ID_pidRate_sensor_signal_X & 0xFF));
	EXPECT_EQ(BT.output.vector[4], uint8_t((ID_pidRate_sensor_signal_X & 0xFF00) >> 8));
	EXPECT_EQ(BT.output.vector[5], '1');
	EXPECT_EQ(BT.output.vector[6], '.');
	EXPECT_EQ(BT.output.vector[7], '2');
	EXPECT_EQ(BT.output.vector[8], '3');
	EXPECT_EQ(BT.output.vector[9], '4');
	EXPECT_EQ(BT.output.vector[10], '!');
	EXPECT_EQ(BT.output.vector[11], uint8_t(ID_gyro_signal_X & 0xFF));
	EXPECT_EQ(BT.output.vector[12], uint8_t((ID_gyro_signal_X & 0xFF00) >> 8));
	EXPECT_EQ(BT.output.vector[13], '2');
	EXPECT_EQ(BT.output.vector[14], '.');
	EXPECT_EQ(BT.output.vector[15], '3');
	EXPECT_EQ(BT.output.vector[16], '4');
	EXPECT_EQ(BT.output.vector[17], '5');
	EXPECT_EQ(BT.output.vector[18], '!');
	EXPECT_EQ(BT.output.vector[19], '\r');
	EXPECT_EQ(BT.output.vector[20], '\n');

	//8th: send 3 stream data + param data
    BT.txDeltaT = 0.012;
    BT.txFrame.sendParam = false;
    BT.txFrame.streamDataFlags = 0b11001;
    BT.output.vector[0] = 0;
    BT.output.vector[1] = '$';
    BT.output.vector[2] = 0;
    BT.output.vector[3] = 0;
    BT.output.vector[4] = 0;
    BT.output.vector[5] = 0;
    pidRate.sensor.signal.x = 1.234f;
    testIn.gyro.signal.x = 2.345f;
    testIn.gyro.signal.y = 987.5f; 
    BT.txFrame.sendParam = true;
    BT.txFrame.paramData = 5;
    BT.txFrame.numberOfFrac = 0;
    BTTransmit(&testIn, &testOut);
    EXPECT_EQ(BT.output.vector[0], 32);
    EXPECT_EQ(BT.output.vector[1], '$');
    EXPECT_EQ(BT.output.vector[2], 'S');
    EXPECT_EQ(BT.output.vector[3], uint8_t(ID_pidRate_sensor_signal_X & 0xFF));
    EXPECT_EQ(BT.output.vector[4], uint8_t((ID_pidRate_sensor_signal_X & 0xFF00) >> 8));
    EXPECT_EQ(BT.output.vector[5], '1');
    EXPECT_EQ(BT.output.vector[6], '.');
    EXPECT_EQ(BT.output.vector[7], '2');
    EXPECT_EQ(BT.output.vector[8], '3');
    EXPECT_EQ(BT.output.vector[9], '4');
    EXPECT_EQ(BT.output.vector[10], '!');
    EXPECT_EQ(BT.output.vector[11], uint8_t(ID_gyro_signal_X & 0xFF));
    EXPECT_EQ(BT.output.vector[12], uint8_t((ID_gyro_signal_X & 0xFF00) >> 8));
    EXPECT_EQ(BT.output.vector[13], '2');
    EXPECT_EQ(BT.output.vector[14], '.');
    EXPECT_EQ(BT.output.vector[15], '3');
    EXPECT_EQ(BT.output.vector[16], '4');
    EXPECT_EQ(BT.output.vector[17], '5');
    EXPECT_EQ(BT.output.vector[18], '!');
    EXPECT_EQ(BT.output.vector[19], uint8_t(ID_gyro_signal_Y & 0xFF));
    EXPECT_EQ(BT.output.vector[20], uint8_t((ID_gyro_signal_Y & 0xFF00) >> 8));
    EXPECT_EQ(BT.output.vector[21], '9');
    EXPECT_EQ(BT.output.vector[22], '8');
    EXPECT_EQ(BT.output.vector[23], '7');
    EXPECT_EQ(BT.output.vector[24], '.');
    EXPECT_EQ(BT.output.vector[25], '5');
    EXPECT_EQ(BT.output.vector[26], '0');
    EXPECT_EQ(BT.output.vector[27], '0');
    EXPECT_EQ(BT.output.vector[28], '!');
    EXPECT_EQ(BT.output.vector[29], 'P');
    EXPECT_EQ(BT.output.vector[30], '5');
    EXPECT_EQ(BT.output.vector[31], '\r');
    EXPECT_EQ(BT.output.vector[32], '\n');
}