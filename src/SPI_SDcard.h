#pragma once

#include "stdint.h"
#include "SPI_common.h"

#define MAX_MEAS_BUFFER_SIZE 150

typedef enum E_SDMainStates
{
	SD_PRE_INIT = 0,
	SD_WAIT_4_MEASUREMENT = 1,
	SD_MEASUREMENT_ONGOING = 2,
    SD_POST_INIT = 3,
    SD_WRITE_DATA = 4,
	SD_WRITE_ROOT = 5,
	SD_WRITE_FAT = 6,

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
	uint16_t numberInName{0};	//number which is in the name
	uint32_t clusters[300];
	uint16_t numberOfClusters;	//number of valid elements cluster array has
	uint32_t size;
	uint8_t  blockCount;	//0-63 value to track the given block inside a cluster
}fileInfo;

typedef struct date
{
	uint8_t year{ 0 };	//last 2 digit of year
	uint8_t month{ 0 };
	uint8_t day{ 0 };
	uint8_t hour{ 0 };
	uint8_t min{ 0 };
	uint8_t sec{ 0 };
}date;

typedef struct data512_st
{
    uint8_t data[512];
}data512_st;


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
	E_SDMainStates MainState{ SD_WAIT_4_MEASUREMENT };

	uint32_t blockPerCluster{ 0 };
	uint32_t bootSectorAddr{ 0 };
	uint32_t FAT1Addr{ 0 };
	uint32_t rootDirAddr{ 0 };

	uint32_t rootDirEmptyBlockNumber{ 0 };  //the block which has empty file slot, where new file can be saved
	uint16_t rootDirEmptySlotNumber{ 0 };    //the slot within the empty block, where new file can be saved
	uint32_t FATBlockOffset{ 0 };    //the block which has empty slots, where new FAT entry can be saved

	fileInfo lastFile;
	fileInfo newFile;

    data512_st measBuffer[MAX_MEAS_BUFFER_SIZE];
    uint16_t measDataCtr{ 0 };  //0..511 count of data
    uint8_t measBufferCtr{ 0 }; //0..149 count of buffer

	//sending buffer, start with 0xFE(start token of CMD24)
	uint32_t sendingBuffer[517] = { 0xFE };
    uint8_t sendBufferIndex{ 0 }; //0..149 count of buffer

    uint32_t writeStartTick{ 0 };
    bool writingMultiFATBlock{ false };
	uint32_t measTickPrev{ 0 };

	date globalDateAndTime{};
    uint64_t sysTickAtGlobalTick{ 0 };

    uint64_t postInitTick{ 0 };
    uint64_t timeoutTick{ 0 };
    uint64_t rootOrFatWriteTick{ 0 };
    uint8_t acmd41TryCtr{ 0 };  //count of how many acmd41 was finished

    uint16_t lastSwitch2Way{1000};

}SpiSDcard_st;

typedef struct Meas2Card
{
    //timestamp
    bool measureSysTick{ true };
    //gyro
    bool measureGyroRawX{ false };
    bool measureGyroRawY{ false };
    bool measureGyroRawZ{ false };
    bool measureGyroPT1X{ false };
    bool measureGyroPT1Y{ false };
    bool measureGyroPT1Z{ false };
    bool measureGyroRealX{ false };
    bool measureGyroRealY{ false };
    bool measureGyroRealZ{ false };
    bool measureGyroRealPT1X{ false };
    bool measureGyroRealPT1Y{ false };
    bool measureGyroRealPT1Z{ false };
    //acc
    bool measureAccRawX{ false };
    bool measureAccRawY{ false };
    bool measureAccRawZ{ false };
    bool measureAccPT1X{ false };
    bool measureAccPT1Y{ false };
    bool measureAccPT1Z{ false };
	bool measureAccRealX{ false };
	bool measureAccRealY{ false };
	bool measureAccRealZ{ false };
	bool measureAccRealPT1X{ false };
	bool measureAccRealPT1Y{ false };
	bool measureAccRealPT1Z{ false };
    //angle
    //bool measureAngleRawRoll{ false };
    //bool measureAngleRawPitch{ false };
    bool measureAnglePT1Roll{ false };
    bool measureAnglePT1Pitch{ false };
 //   bool measureAnglePT2Roll{ false };
 //   bool measureAnglePT2Pitch{ false };
 //   bool measureAngleKFRawRoll{ false };
 //   bool measureAngleKFRawPitch{ false };
    bool measureAngleKFPT11Roll{ false };
    bool measureAngleKFPT11Pitch{ false };
 //   bool measureAngleCFRawRoll{ false };
 //   bool measureAngleCFRawPitch{ false };
	//bool measureAngleCFPT10Roll{ false };
	//bool measureAngleCFPT10Pitch{ false };
	//bool measureAngleCFPT11Roll{ false };
	//bool measureAngleCFPT11Pitch{ false };
	//bool measureAngleCFWeightedRawRoll{ false };
	//bool measureAngleCFWeightedRawPitch{ false };
	//bool measureAngleCFWeightedPT01Roll{ false };
	//bool measureAngleCFWeightedPT01Pitch{ false };
    //PID control
	bool measurePIDRefsigX{ true };
	bool measurePIDRefsigY{ false };
	bool measurePIDRefsigZ{ false };
	bool measurePIDSensorX{ true };
	bool measurePIDSensorY{ false };
	bool measurePIDSensorZ{ false };
	bool measurePIDPoutX{ true };
	bool measurePIDPoutY{ false };
	bool measurePIDPoutZ{ false };
	bool measurePIDIoutX{ false };
	bool measurePIDIoutY{ false };
	bool measurePIDIoutZ{ false };
	bool measurePIDDoutX{ false };
	bool measurePIDDoutY{ false };
	bool measurePIDDoutZ{ false };
	bool measurePIDFFoutX{ true };
	bool measurePIDFFoutY{ false };
	bool measurePIDFFoutZ{ false };
	bool measurePIDUX{ true };
	bool measurePIDUY{ false };
	bool measurePIDUZ{ false };
    //PID internals
    bool measurePIDerrorX{ false };
    bool measurePIDerrorY{ false };
    bool measurePIDerrorZ{ false };
    bool measurePIDerrorSumX{ false };
    bool measurePIDerrorSumY{ false };
    bool measurePIDerrorSumZ{ false };
    bool measurePIDerrorDotPT1X{ false };
    bool measurePIDerrorDotPT1Y{ false };
    bool measurePIDerrorDotPT1Z{ false };
    bool measurePIDrefSigDotPT1X{ false };
    bool measurePIDrefSigDotPT1Y{ false };
    bool measurePIDrefSigDotPT1Z{ false };
    bool measurePIDiRelaxWeightX{ false };
    bool measurePIDiRelaxWeightY{ false };
    bool measurePIDiRelaxWeightZ{ false };

}Meas2Card;

// Method to init SD card logic
void InitSDCard();

// Method to run SD card communication and logic
void RunSdCard();

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

// Method to prep sendingBuffer with the current measBuffer data
void prepSendingBuffer();

// Function to write a block to sd card
E_SDWriteStates writeBlock(uint32_t blockOffset, volatile uint32_t* txBuf);

// Function to extract file info from root data and write to serial
fileInfo getFileInfo(volatile uint8_t* rawFileData);

// Function to extract cluster postions from FAT
bool getAllFileClusters(volatile uint8_t* rawFileData, fileInfo* fileInfo);

// Method to set file time in ROOT
void setFileTime(volatile uint32_t* block, uint64_t sysTick);

// Method to add file into to root block
void addFileInfo2RootDir(volatile uint32_t* block, fileInfo* file, uint64_t sysTick);

// Method to add file cluster info into FAT
bool addFileFATInfo(volatile uint32_t* block, fileInfo* file, E_SDFATWriteStates FATState);

// Method to print out file info to serialusb, DEBUG only
void printFileInfo(fileInfo* fileInfo);

// Method to append chip select flag for write
void appendCsSdCard(volatile uint32_t* block, uint16_t blockSize);

// Method to convert data into characters
void convert2CharStream(uint8_t* buffer, uint8_t* startPos, float value, uint8_t numberOfFractions, bool explicitPlusSign);

// Method to store 1 measured data into loading buffer
void measureData(bool isMeasured, bool isCommaed, float data, uint8_t numberOfFrac, bool isExplicitPlus, char* debugName);

// Method to save data for measurement snapshots
void saveMeasData();	

// Method to load data chararacters into loading buffer
void loadData2Buffer(uint8_t* chars2Add, uint8_t numberOfChar);

// Function to check buffer and data counter if they need incrementing
int8_t checkCtrs(void);

// Method to append comma to loadingBuffer
void appendChar(const char c);

// Method to add measured signals names to header
void addMeasNameHeader(bool isMeasured, bool isCommaed, char* name, uint8_t numberOfChar);

// Method to add header to the beginning of the file
void addMeasHeader(void);

// Method to write data to sd card paralel to saving
void writeData(uint64_t sysTick);

// Method to write data to sd card paralel to saving
void writeRoot(uint64_t sysTick);

// Method to write data to sd card paralel to saving
void writeFAT(uint64_t sysTick);

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

// Method to set global time value
// data is copied due to temp value and only done a few times per power cycle
void setGlobalTime(const date newTime, const uint64_t currentSysTick);
void setGlobalDate(const date newTime);

// Method to detect retrigger post-init and write to sdcard
void DetectReInitAndWrite(const uint16_t switch2way);