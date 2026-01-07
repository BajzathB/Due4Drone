// Header to configure and set USART1 for RC IBUS data receive

//AT settings:
//EN pin to be 5V and button pushed on poweron to enter AT mode
//460800baud 1stop 0pairty

#ifndef BT_HEADER
#define BT_HEADER

#include "stdint.h"
#include "Controller.h"

enum E_rxstate : uint8_t
{
	WAITING_FOR_NUMBEROFBYTES = 0,
	RECEVING_DATA_BYTES = 1,
	FRAME_RECEIVED = 2
};

enum E_command : uint8_t
{
	CMD_NONE   = 0b00000000,
	CMD_SET    = 0b00000011,
	CMD_GET    = 0b00001100,
	CMD_STREAM = 0b00110000
};

struct buffer_
{
	uint8_t vector[150];
	uint8_t ctr{ 0 };
};

struct rxframe_st
{
	E_command cmd{ CMD_NONE };
	uint16_t id{ 0 };
};

struct txframe_st
{
	uint64_t streamDataFlags{ 0 };
	uint64_t streamDataFlags2{ 0 };

	bool sendParam{ false };
	float paramData{ 0.0 };
	uint8_t numberOfFrac{ 0 };
};

struct bt{
	volatile buffer_ output;
	volatile buffer_ input;

	E_rxstate rxDataState{ WAITING_FOR_NUMBEROFBYTES };

	rxframe_st rxFrame;
	txframe_st txFrame;

    float txDeltaT{ 0 };
};

// Method to set initial values for controller variable
void SetupBT(void);

// Method to run controller logic cyclically
void RunBT(const controllerIn_st* ctrlIn, const controllerOut_st* ctrlOut);

// Method to process received message
void BTReceive(void);

// Method to assemble and send message
void BTTransmit(const controllerIn_st* ctrlIn, const controllerOut_st* ctrlOut);

// Method to process received frame
void ProcessRxFrame(void);

// Function to convert input string to value from SET cmd
bool ConvertStrToBool(volatile buffer_* input);
uint8_t ConvertStrToUint8(volatile buffer_* input);
uint16_t ConvertStrToUint16(volatile buffer_* input);
float ConvertStrToFloat(volatile buffer_* input);

// Method to check if frame ending is received
bool isFrameEndReceived(void);

// Method to calculate character from value and fill Tx frame
void CalcCharAndFillOutput(float value, uint8_t numOfFrac);

// Method to set stream data flag bit
void SetSteamFlag(const uint16_t steamDataID);

// Method to set stream data 2 flag bit
void SetSteamFlag2(const uint16_t steamDataID);

// Method to set stream data value into transmit buffer
void SetStreamData(const uint16_t dataID, const float dataValue);

#endif // !BT_HEADER