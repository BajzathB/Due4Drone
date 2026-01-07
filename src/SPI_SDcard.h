#pragma once

#include "stdint.h"
#include "SPI_common.h"

typedef enum E_SDMainStates
{
	SD_INIT = 0,
	SD_WAIT_4_MEASUREMENT = 1,
	SD_MEASUREMENT_ONGOING = 2,
	SD_WRITE_ROOT = 3,
	SD_WRITE_FAT = 4,

	SD_DO_NOTHING = 10
}E_SDMainStates;

typedef enum E_SDInitStates : uint8_t
{
	SDINIT_CMD0 = 0,
	SDINIT_CMD8 = 1,
	SDINIT_CMD58 = 2,
	SDINIT_CMD55 = 3,
	SDINIT_ACMD41 = 4,
	SDINIT_READ_00 = 5,
	SDINIT_READ_BOOT = 6,
	SDINIT_READ_ROOTDIR = 7,
	SDINIT_READ_FAT = 8,

	SDINIT_SUCCESS = 15,	//final state if every sd initalization step was successful
	SDINIT_FAILURE = 16	//final state if any sd initilazation step failed
}E_SDInitStates;

typedef enum E_SDCommandStates : uint8_t
{
	SDCOMMAND_SEND = 0,
	SDCOMMAND_WAIT4RX = 1
}E_SDCommandStates;

typedef enum E_SDReadStates : uint8_t
{
	SDREAD_START = 0,
	SDREAD_WAIT_RESPONSE = 1,
	SDREAD_WAIT_FE = 2,
	SDREAD_WAIT_DATA = 3,
	SDREAD_FINISHED = 4,

	SDREAD_FAILED = 10,
}E_SDReadStates;

typedef enum E_SDWriteStates : uint8_t
{
	SDWRITE_START = 0,
	SDWRITE_WAIT_RESPONSE = 1,
	SDWRITE_WAIT_DATA = 2,
	SDWRITE_WAIT_WRITE_FINISH = 3,
	SDWRITE_FINISHED = 4,

	SDWRITE_FAILED = 10,
}E_SDWriteStates;

typedef enum E_SDFATWriteStates : uint8_t
{
	E_SDFATWRITE_FIRST_CALL = 0,
	E_SDFATWRITE_CONSECUTIVE_CALL = 1
}E_SDFATWriteStates;

//typedef enum E_MeasurementState : uint8_t
//{
//	MEASUREMENT_OFF = 0,
//	MEASUREMENT_ON = 1
//}E_MeasurementState;

typedef struct fileInfo
{
	char name[11];			//Name of the file, expect number
	uint16_t numberInName;	//number which is in the name
	uint32_t clusters[300];
	uint16_t numberOfClusters;	//number of valid elements cluster array has
	uint32_t size;
	uint8_t  blockCount;	//0-63 value to track the given block inside a cluster
}fileInfo;

typedef struct SpiSDcard_st
{
	uint32_t SdCtr{ 0 };
	volatile uint32_t SdTx[100];
	volatile uint8_t SdRx[600];
	volatile uint8_t rootDirInfo_8b[516] = { 0 };
	volatile uint32_t rootDirInfo[517] = { 0xFE };	//start with 0xFE(start token of CMD24)
	volatile uint8_t FAT1Info_8b[516] = { 0 };
	volatile uint32_t FAT1Info[517] = { 0xFE };		//start with 0xFE(start token of CMD24)

	volatile E_SPIActivity spiActivitySDCard{ INACTIVE };
	volatile uint32_t* nextTxBuffer;
	volatile uint32_t* nextTxBuffer4Data;
	volatile uint8_t* nextRxBuffer;
	uint32_t nextCtr;

	E_SDCommandStates SDCommandState{ SDCOMMAND_SEND };
	E_SDReadStates SDReadState{ SDREAD_START };
	volatile E_SDWriteStates SDWriteState{ SDWRITE_START };
	E_SDInitStates SDInitStatus{ SDINIT_CMD0 };
	E_SDMainStates MainState{ SD_INIT };

	bool  sdCardInitFinished{ false };	// flags init state be done successfully or not

	uint32_t blockPerCluster{ 0 };
	uint32_t bootSectorAddr{ 0 };
	uint32_t FAT1Addr{ 0 };
	uint32_t rootDirAddr{ 0 };

	uint32_t rootDirEmptyBlockNumber{ 0 };  //the block which has empty file slot, where new file can be saved
	uint16_t rootDirEmptySlotNumber{ 0 };    //the slot within the empty block, where new file can be saved
	uint32_t FATBlockOffset{ 0 };    //the block which has empty slots, where new FAT entry can be saved

	fileInfo lastFile;
	fileInfo newFile;

	//1 data collection buffer, 1 sending buffer, both start with 0xFE(start token of CMD24)
	uint32_t dataBuffer1[517] = { 0xFE };
	uint32_t dataBuffer2[517] = { 0xFE };
	uint32_t* sendingDataPointer = dataBuffer1;
	uint32_t* loadingDataPointer = dataBuffer2;
	uint16_t loadingDataCounter{ 1 };	//0th element is always 0xFE(start token)

    float writeStartTime{ 0 };
    bool writingMultiFATBlock{ false };
	float measTimePrev{ 0 };
    bool writeMeasData{ false };

}SpiSDcard_st;

typedef struct Meas2Card
{
    //timestamp
    bool measureSysTime{ true };
    //gyro
    bool measureGyroRawX{ false };
    bool measureGyroRawY{ false };
    bool measureGyroRawZ{ false };
    bool measureGyroPT1X{ false };
    bool measureGyroPT1Y{ false };
    bool measureGyroPT1Z{ false };
    bool measureGyroPT2X{ false };
    bool measureGyroPT2Y{ false };
    bool measureGyroPT2Z{ false };
    //acc
    bool measureAccRawX{ false };
    bool measureAccRawY{ false };
    bool measureAccRawZ{ false };
    bool measureAccPT1X{ false };
    bool measureAccPT1Y{ false };
    bool measureAccPT1Z{ false };
    bool measureAccPT2X{ false };
    bool measureAccPT2Y{ false };
    bool measureAccPT2Z{ false };
    //angle
    bool measureAngleRawRoll{ false };
    bool measureAngleRawPitch{ false };
    bool measureAnglePT1Roll{ false };
    bool measureAnglePT1Pitch{ false };
    bool measureAnglePT2Roll{ false };
    bool measureAnglePT2Pitch{ false };
    bool measureAngleKFRawRoll{ false };
    bool measureAngleKFRawPitch{ false };
    bool measureAngleKFPT10Roll{ false };
    bool measureAngleKFPT10Pitch{ false };
    bool measureAngleKFPT20Roll{ false };
    bool measureAngleKFPT20Pitch{ false };
    bool measureAngleKFPT11Roll{ false };
    bool measureAngleKFPT11Pitch{ false };
    bool measureAngleKFPT21Roll{ false };
    bool measureAngleKFPT21Pitch{ false };
    bool measureAngleKFPT22Roll{ false };
    bool measureAngleKFPT22Pitch{ false };
    //PID control
    bool measurePIDRefsigX{ false };
    bool measurePIDRefsigY{ false };
    bool measurePIDRefsigZ{ false };
    bool measurePIDSensorX{ false };
    bool measurePIDSensorY{ false };
    bool measurePIDSensorZ{ false };
    bool measurePIDPoutX{ false };
    bool measurePIDPoutY{ false };
    bool measurePIDPoutZ{ false };
    bool measurePIDIoutX{ false };
    bool measurePIDIoutY{ false };
    bool measurePIDIoutZ{ false };
    bool measurePIDDoutX{ false };
    bool measurePIDDoutY{ false };
    bool measurePIDDoutZ{ false };
    bool measurePIDFFoutX{ false };
    bool measurePIDFFoutY{ false };
    bool measurePIDFFoutZ{ false };
    bool measurePIDUX{ false };
    bool measurePIDUY{ false };
    bool measurePIDUZ{ false };

}Meas2Card;


// Method to run SD card communication and logic
void RunSdCard(SpiInput* spiInput, SPIOutput* spiOutput);

// Function to execute initial SD card configuration
E_SDMainStates SetupSdCard(void);

// Method to trigger SpiDmaTxRx for SD card consistantly
void triggerSDRxTx(volatile uint32_t* txBuff, volatile uint8_t* rxBuff, uint32_t ctr);

// Method to call triggerSDRxTx in interrupt safe way
void intSafeTriggerSDRxTx(volatile uint32_t* txBuff, volatile uint8_t* rxBuff, uint32_t ctr);

// Function to execute CMD0 command for SD card
E_SDInitStates CMD0(void);

// Function to execute CMD8 command for SD card
E_SDInitStates CMD8(void);

// Function to execute CMD58 command for SD card
E_SDInitStates CMD58(void);

// Function to execute CMD55 command for SD card
E_SDInitStates CMD55(void);

// Function to execute ACMD41 command for SD card
E_SDInitStates ACMD41(void);

// Function to read boot sector of SD card
E_SDInitStates readBoot();

// Function to read root sector of SD card and find last file
E_SDInitStates readRoot(void);

// Function to read FAT of SD card and find last file's last sector
E_SDInitStates readFAT(void);

// Function to read a block from sd card
E_SDReadStates readBlock(uint32_t blockaddr, volatile uint8_t* rxBuf);

// Method to trigger writeblock 2nd step(wait reponse) in DMAC_Handler
void SDWriteWaitResponse(void);

// Method to trigger and check data received flag
void SDWriteWaitData(void);

// Function to write a block to sd card
E_SDWriteStates writeBlock(uint32_t blockOffset, volatile uint32_t* txBuf);

// Function to extract file info from root data and write to serial
fileInfo getFileInfo(volatile uint8_t* rawFileData);

// Function to extract cluster postions from FAT
bool getAllFileClusters(volatile uint8_t* rawFileData, fileInfo* fileInfo);

// Method to add file into to root block
void addFileInfo2RootDir(volatile uint32_t* block, fileInfo* file);

// Method to add file cluster info into FAT
bool addFileFATInfo(volatile uint32_t* block, fileInfo* file, E_SDFATWriteStates FATState);

// Method to print out file info to serialusb, DEBUG only
void printFileInfo(fileInfo* fileInfo);

// Method to append chip select flag for write
void appendCsSdCard(volatile uint32_t* block, uint16_t blockSize);

// Method to swap loading and sending pointer in between the databuffers
void swapDataBufferPointers(void);

// Method to convert data into characters
void convert2String(uint32_t* buffer, uint8_t* startPos, float value2Convert, uint8_t numberOfFractions, bool explicitPlusSign);

// Method to store 1 measured data into loading buffer
void measureData(bool isMeasured, bool isCommaed, float data, uint8_t numberOfFrac, bool isExplicitPlus, char* debugName);

// Method to save data for measurement snapshots
void saveMeasData(SpiInput* spiInput, SPIOutput* spiOutput);	

// Method to load data chararacters into loading buffer
void loadData2Buffer(uint32_t* chars2Add, uint8_t numberOfChar);

// Method to append comma to loadingBuffer
void appendComma(void);

// Method to append '\n' to loadingBuffer
void appendNewLine(void);

// Method to add measured signals names to header
void addMeasNameHeader(bool isMeasured, bool isCommaed, char* name, uint8_t numberOfChar);

// Method to add header to the beginning of the file
void addMeasHeader(void);

// Method to write data to sd card paralel to saving
void writeData(uint16_t measSwitch);

// Method to write data to sd card paralel to saving
void writeRoot(void);

// Method to write data to sd card paralel to saving
void writeFAT(void);

// Method to get SPI SDcard struct
SpiSDcard_st* getSPISdCard(void);

// Method to trigger spi dma tx-rx call with the prepaired next values;
void TriggerNextSdCardTxRx(void);

// Method to get Measurement flag struct
Meas2Card* getMeas2Card(void);

// Reset states to wait4meas
E_SDMainStates ResetMeasurement(void);

// Re-init sdcard, starting from cmd0
E_SDMainStates ReinitSDCard(void);