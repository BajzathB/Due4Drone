#include "pch.h"

#include "../../src/SPI.h"
#include "../../src/SPI_common.h"
#include "../../src/SPI_SDcard.h"
#include "../../src/sysTime.h"
#include "../../src/Controller.h"
#include "../helper/support4Testing.h"

extern Usart* USART1;
extern spi_st SPI;
extern SpiSDcard_st SDcard;
extern sysTime sysTimer;
extern pid_st pidRate;
extern Meas2Card meas2Card;

uint8_t rootDirBase[515] = { 0x42,0x20,0x00,0x49,0x00,0x6E,0x00,0x66,  0x00,0x6F,0x00,0x0F,0x00,0x72,0x72,0x00,
							 0x6D,0x00,0x61,0x00,0x74,0x00,0x69,0x00,  0x6F,0x00,0x00,0x00,0x6E,0x00,0x00,0x00,
							 0x01,0x53,0x00,0x79,0x00,0x73,0x00,0x74,  0x00,0x65,0x00,0x0F,0x00,0x72,0x6D,0x00,
							 0x20,0x00,0x56,0x00,0x6F,0x00,0x6C,0x00,  0x75,0x00,0x00,0x00,0x6D,0x00,0x65,0x00,
							 0x53,0x59,0x53,0x54,0x45,0x4D,0x7E,0x31,  0x20,0x20,0x20,0x16,0x00,0xC3,0xE3,0x52,
							 0x9B,0x51,0x9B,0x51,0x00,0x00,0xE4,0x52,  0x9B,0x51,0x03,0x00,0x00,0x00,0x00,0x00,
							 'P' ,'A' ,'R' ,'A' ,'M' ,0x20,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x06,0x00,0x05,0x00,0x00,0x00,
							 'M' ,'E' ,'A' ,'S' ,'1' ,0x20,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x07,0x00,0x10,0x11,0x12,0x13,
							 'M' ,'E' ,'A' ,'S' ,'2' ,0x20,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x08,0x00,0xBA,0xDC,0xFE,0x00,
							 'M','E' ,'A' ,'S' ,'3' ,0x20,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x09,0x00,0x00,0x10,0x00,0x00,
							 'M' ,'E' ,'A' ,'S' ,'4' ,0x20,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x0A,0x00,0x00,0x10,0x00,0x00,
							 'M' ,'E' ,'A' ,'S' ,'5' ,0x20,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x0B,0x00,0x00,0x10,0x00,0x00,
							 'M' ,'E' ,'A' ,'S' ,'6' ,0x20,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x0C,0x00,0x00,0x10,0x00,0x00,
							 'M' ,'E' ,'A' ,'S' ,'7' ,0x20,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x0D,0x00,0x00,0x10,0x00,0x00,
							 'M' ,'E' ,'A' ,'S' ,'8' ,0x20,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x0E,0x00,0x00,0x10,0x00,0x00,
							 'M' ,'E' ,'A' ,'S' ,'9' ,0x20,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x0F,0x00,0x00,0x10,0x00,0x00,
							 'M' ,'E' ,'A' ,'S' ,'1' ,'0' ,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x10,0x00,0x00,0x10,0x00,0x00,
							 'M' ,'E' ,'A' ,'S' ,'1' ,'1' ,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x11,0x00,0x00,0x10,0x00,0x00,
							 'M' ,'E' ,'A' ,'S' ,'1' ,'2' ,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x12,0x00,0x00,0x10,0x00,0x00 };

uint8_t FAT1Base[515] = { 0xF8,0xFF,0xFF,0x0F, 0xFF,0xFF,0xFF,0x7F,  0xFF,0xFF,0xFF,0x0F, 0xFF,0xFF,0xFF,0x0F,
						  0xFF,0xFF,0xFF,0x0F, 0xFF,0xFF,0xFF,0x0F,  0xFF,0xFF,0xFF,0x0F, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, //10
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0xC5,0x00,0x00,0x00, 0xC6,0x00,0x00,0x00,  0xFF,0xFF,0xFF,0x0F, 0xC8,0x00,0x00,0x00,
						  0xC9,0x00,0x00,0x00, 0xCA,0x00,0x00,0x00,  0xCB,0x00,0x00,0x00, 0xCC,0x00,0x00,0x00,	//72. slot
						  0xCD,0x00,0x00,0x00, 0xCE,0x00,0x00,0x00,  0xCF,0x00,0x00,0x00, 0xD0,0x00,0x00,0x00,
						  0xD1,0x00,0x00,0x00, 0xD2,0x00,0x00,0x00,  0xD3,0x00,0x00,0x00, 0xD4,0x00,0x00,0x00, //20
						  0xD5,0x00,0x00,0x00, 0xD6,0x00,0x00,0x00,  0xD7,0x00,0x00,0x00, 0xD8,0x00,0x00,0x00,
						  0xD9,0x00,0x00,0x00, 0xDA,0x00,0x00,0x00,  0xDB,0x00,0x00,0x00, 0xDC,0x00,0x00,0x00,
						  0xDD,0x00,0x00,0x00, 0xDE,0x00,0x00,0x00,  0xDF,0x00,0x00,0x00, 0xE0,0x00,0x00,0x00,
						  0xE1,0x00,0x00,0x00, 0xE2,0x00,0x00,0x00,  0xE3,0x00,0x00,0x00, 0xE4,0x00,0x00,0x00,
						  0xE5,0x00,0x00,0x00, 0xE6,0x00,0x00,0x00,  0xE7,0x00,0x00,0x00, 0xE8,0x00,0x00,0x00,
						  0xE9,0x00,0x00,0x00, 0xEA,0x00,0x00,0x00,  0xEB,0x00,0x00,0x00, 0xEC,0x00,0x00,0x00,
						  0xED,0x00,0x00,0x00, 0xEE,0x00,0x00,0x00,  0xEF,0x00,0x00,0x00, 0xF0,0x00,0x00,0x00,
						  0xF1,0x00,0x00,0x00, 0xF2,0x00,0x00,0x00,  0xF3,0x00,0x00,0x00, 0xF4,0x00,0x00,0x00,
						  0xF5,0x00,0x00,0x00, 0xF6,0x00,0x00,0x00,  0xF7,0x00,0x00,0x00, 0xF8,0x00,0x00,0x00,
						  0xF9,0x00,0x00,0x00, 0xFA,0x00,0x00,0x00,  0xFB,0x00,0x00,0x00, 0xFC,0x00,0x00,0x00, //30
						  0xFD,0x00,0x00,0x00, 0xFE,0x00,0x00,0x00,  0xFF,0x00,0x00,0x00, 0x00,0x01,0x00,0x00 };

uint8_t FAT1Base2[514] = { 0xF8,0xFF,0xFF,0x0F, 0xFF,0xFF,0xFF,0x7F,  0xFF,0xFF,0xFF,0x0F, 0xFF,0xFF,0xFF,0x0F,
						  0xFF,0xFF,0xFF,0x0F, 0xFF,0xFF,0xFF,0x0F,  0xFF,0xFF,0xFF,0x0F, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, //10
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00, 0xC8,0x00,0x00,0x00,
						  0xC9,0x00,0x00,0x00, 0xCA,0x00,0x00,0x00,  0xCB,0x00,0x00,0x00, 0xCC,0x00,0x00,0x00,	//72. slot
						  0xCD,0x00,0x00,0x00, 0xCE,0x00,0x00,0x00,  0xCF,0x00,0x00,0x00, 0xD0,0x00,0x00,0x00,
						  0xD1,0x00,0x00,0x00, 0xD2,0x00,0x00,0x00,  0xD3,0x00,0x00,0x00, 0xD4,0x00,0x00,0x00, //20
						  0xD5,0x00,0x00,0x00, 0xD6,0x00,0x00,0x00,  0xD7,0x00,0x00,0x00, 0xD8,0x00,0x00,0x00,
						  0xD9,0x00,0x00,0x00, 0xDA,0x00,0x00,0x00,  0xDB,0x00,0x00,0x00, 0xDC,0x00,0x00,0x00,
						  0xDD,0x00,0x00,0x00, 0xDE,0x00,0x00,0x00,  0xDF,0x00,0x00,0x00, 0xE0,0x00,0x00,0x00,
						  0xE1,0x00,0x00,0x00, 0xE2,0x00,0x00,0x00,  0xE3,0x00,0x00,0x00, 0xE4,0x00,0x00,0x00,
						  0xE5,0x00,0x00,0x00, 0xE6,0x00,0x00,0x00,  0xFF,0xFF,0xFF,0x0F, 0x68,0x00,0x00,0x00,
						  0x69,0x00,0x00,0x00, 0x6A,0x00,0x00,0x00,  0x6B,0x00,0x00,0x00, 0x6C,0x00,0x00,0x00,
						  0x6D,0x00,0x00,0x00, 0x6E,0x00,0x00,0x00,  0x6F,0x00,0x00,0x00, 0x70,0x00,0x00,0x00,
						  0x71,0x00,0x00,0x00, 0x72,0x00,0x00,0x00,  0x73,0x00,0x00,0x00, 0x74,0x00,0x00,0x00,
						  0x75,0x00,0x00,0x00, 0x76,0x00,0x00,0x00,  0xFF,0xFF,0xFF,0x0F, 0x78,0x00,0x00,0x00,
						  0x79,0x00,0x00,0x00, 0x7A,0x00,0x00,0x00,  0x7B,0x00,0x00,0x00, 0x7C,0x00,0x00,0x00, //30
						  0x7D,0x00,0x00,0x00, 0x7E,0x00,0x00,0x00,  0x7F,0x00,0x00,0x00, 0xFF,0xFF,0xFF,0x0F };

TEST(test_BT_SDcard, SDcard_Call)
{
	volatile uint32_t testtxbuff[600];
	volatile uint8_t testrxbuff[600];
	uint32_t testctr{0};
	triggerSDRxTx(testtxbuff, testrxbuff, testctr);
	intSafeTriggerSDRxTx(testtxbuff, testrxbuff, testctr);
	CMD0();
	CMD8();
	CMD58();
	CMD55();
	ACMD41();
	uint32_t testblockaddr{ 0 };
	readBlock(testblockaddr, testrxbuff);
	writeBlock(testblockaddr, testtxbuff);
	readBoot();
	readRoot();
	readFAT();
	fileInfo testfileInfo;
	testfileInfo.numberOfClusters = 1;
	uint8_t testrawFileData{ 0 };
	getFileInfo(&testrawFileData);
	getAllFileClusters(&testrawFileData, &testfileInfo);
	float testTime{ 0 };
	setFileTime(testtxbuff, testTime);
	addFileInfo2RootDir(testtxbuff, &testfileInfo, testTime);
	E_SDFATWriteStates testFATState{ E_SDFATWRITE_FIRST_CALL };
	addFileFATInfo(testtxbuff, &testfileInfo, testFATState);
	printFileInfo(&testfileInfo);
	uint16_t testblockSize{ 0 };
	volatile uint32_t testblock[514];
	appendCsSdCard(testblock, testblockSize);
	swapDataBufferPointers();
	uint32_t testbuffer[514];
	uint8_t testNumber{ 1 };
	convert2String(testbuffer, &testNumber, 1, 0, false);
    measureData(false, false, 1.000f, 0, false, "test");
	SPIInput testinput;
	SPIOutput testoutput;
	saveMeasData(&testinput, &testoutput);
	loadData2Buffer(testbuffer, 1);
	appendComma();
	appendNewLine();
    addMeasNameHeader(false, false, "test", 4);
	addMeasHeader();
	writeData(1000, testTime);
	writeRoot();
	writeFAT();
	RunSdCard(&testinput, &testoutput);
	InitSDCard();
	SetupSdCard();
	TriggerNextSdCardTxRx();
	SDWriteWaitResponse();
	date testDate{};
	setGlobalTime(testDate, testTime);
}

TEST(test_BT_SDcard, CMD0_Test)
{
	//1st: trigger CMD0, no active read
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_SEND;
    SDcard.sdCardInitFinished = true;
	EXPECT_EQ(CMD0(), SDINIT_CMD0);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);
	EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
	EXPECT_EQ(SDcard.sdCardInitFinished, false);

	//2nd: trigger CMD0, gyro active read
	SPI.spiActivityGyro = ACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(CMD0(), SDINIT_CMD0);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);

	//3rd: wait for CMD0 finish, 
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = ACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	EXPECT_EQ(CMD0(), SDINIT_CMD0);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);

	//4th: CMD0 finished correctly
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	SDcard.SdRx[0] = 1;
	SDcard.SdCtr = 1;
	EXPECT_EQ(CMD0(), SDINIT_CMD8);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_SEND);

	//5th: CMD0 finished incorrectly
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	SDcard.SdRx[0] = 0;
	EXPECT_EQ(CMD0(), SDINIT_FAILURE);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);
    EXPECT_EQ(SDcard.sdCardInitFinished, true);
}

TEST(test_BT_SDcard, CMD8_Test)
{
	//1st: trigger CMD8, no active read
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(CMD8(), SDINIT_CMD8);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);

	//2nd: trigger CMD8, gyro active read
	SPI.spiActivityGyro = ACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(CMD8(), SDINIT_CMD8);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);

	//3rd: wait for CMD8 finish, 
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = ACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	EXPECT_EQ(CMD8(), SDINIT_CMD8);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);

	//4th: CMD8 finished correctly
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	SDcard.SdRx[0] = 1;
	SDcard.SdCtr = 5;
	EXPECT_EQ(CMD8(), SDINIT_CMD58);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_SEND);

	//5th: CMD8 finished incorrectly
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	SDcard.SdRx[0] = 0;
	SDcard.SdCtr = 5;
	EXPECT_EQ(CMD8(), SDINIT_FAILURE);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);
}

TEST(test_BT_SDcard, CMD58_Test)
{
	//1st: trigger CMD58, no active read
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(CMD58(), SDINIT_CMD58);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);

	//2nd: trigger CMD58, gyro active read
	SPI.spiActivityGyro = ACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(CMD58(), SDINIT_CMD58);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);

	//3rd: wait for CMD58 finish, 
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = ACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	EXPECT_EQ(CMD58(), SDINIT_CMD58);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);

	//4th: CMD58 finished correctly
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	SDcard.SdRx[0] = 1;
	SDcard.SdCtr = 5;
	EXPECT_EQ(CMD58(), SDINIT_CMD55);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_SEND);

	//5th: CMD58 finished incorrectly
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	SDcard.SdRx[0] = 0;
	SDcard.SdCtr = 5;
	EXPECT_EQ(CMD58(), SDINIT_FAILURE);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);
}

TEST(test_BT_SDcard, CMD55_Test)
{
	//1st: trigger CMD55, no active read
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(CMD55(), SDINIT_CMD55);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);

	//2nd: trigger CMD55, gyro active read
	SPI.spiActivityGyro = ACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(CMD55(), SDINIT_CMD55);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);

	//3rd: wait for CMD55 finish, 
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = ACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	EXPECT_EQ(CMD55(), SDINIT_CMD55);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);

	//4th: CMD55 finished correctly
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	SDcard.SdRx[0] = 1;
	SDcard.SdCtr = 1;
	EXPECT_EQ(CMD55(), SDINIT_ACMD41);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_SEND);

	//5th: CMD55 finished incorrectly
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	SDcard.SdRx[0] = 0;
	SDcard.SdCtr = 1;
	EXPECT_EQ(CMD55(), SDINIT_FAILURE);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);
}

TEST(test_BT_SDcard, ACMD41_Test)
{
	//1st: trigger ACMD41, no active read
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(ACMD41(), SDINIT_ACMD41);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);

	//2nd: trigger ACMD41, gyro active read
	SPI.spiActivityGyro = ACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(ACMD41(), SDINIT_ACMD41);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);

	//3rd: wait for ACMD41 finish, 
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = ACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	EXPECT_EQ(ACMD41(), SDINIT_ACMD41);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_WAIT4RX);

	//4th: ACMD41 finished correctly
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	SDcard.SdRx[0] = 0;
	SDcard.SdCtr = 1;
	EXPECT_EQ(ACMD41(), SDINIT_READ_00);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_SEND);

	//5th: ACMD41 finished incorrectly
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDCommandState = SDCOMMAND_WAIT4RX;
	SDcard.SdRx[0] = 1;
	SDcard.SdCtr = 1;
	EXPECT_EQ(ACMD41(), SDINIT_CMD55);
	EXPECT_EQ(SDcard.SDCommandState, SDCOMMAND_SEND);
}

TEST(test_BT_SDcard, TriggerSDRxTx_Test)
{
	uint32_t testTx[10];
	uint8_t testRx[10];
	uint32_t ctr = 5;

	//1st
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	triggerSDRxTx(testTx, testRx, ctr);
	EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);

	//2nd
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = ACTIVE;
	triggerSDRxTx(testTx, testRx, ctr);
	EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);

	//3rd
	SPI.spiActivityGyro = ACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	triggerSDRxTx(testTx, testRx, ctr);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);

	//4th
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = ACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	triggerSDRxTx(testTx, testRx, ctr);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);

	//5th
	SPI.spiActivityGyro = PENDING;
	SPI.spiActivityAcc = ACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	triggerSDRxTx(testTx, testRx, ctr);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);

	//7th
	SPI.spiActivityGyro = PENDING;
	SPI.spiActivityAcc = ACTIVE;
	SDcard.spiActivitySDCard = PENDING;
	triggerSDRxTx(testTx, testRx, ctr);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);

	//7th
	SPI.spiActivityGyro = PENDING;
	SPI.spiActivityAcc = ACTIVE;
	SDcard.spiActivitySDCard = PENDING;
	intSafeTriggerSDRxTx(testTx, testRx, ctr);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);

}

TEST(test_BT_SDcard, readBlock_Test)
{
	uint32_t testblockaddr{ 0x1234 };
	volatile uint8_t testrxbuff[600];

	//trigger, no active read
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDReadState = SDREAD_START;
	EXPECT_EQ(readBlock(testblockaddr, testrxbuff), SDREAD_WAIT_RESPONSE);
	EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);

	//trigger, active read
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = ACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDReadState = SDREAD_START;
	EXPECT_EQ(readBlock(testblockaddr, testrxbuff), SDREAD_WAIT_RESPONSE);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);

	//wait response
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = ACTIVE;
	SDcard.spiActivitySDCard = PENDING;
	SDcard.SDReadState = SDREAD_WAIT_RESPONSE;
	EXPECT_EQ(readBlock(testblockaddr, testrxbuff), SDREAD_WAIT_RESPONSE);

	//wait response, finished correctly active
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDReadState = SDREAD_WAIT_RESPONSE;
	SDcard.SdRx[0] = 0;
	SDcard.SdCtr = 1;
	EXPECT_EQ(readBlock(testblockaddr, testrxbuff), SDREAD_WAIT_FE);
	EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);

	//wait response, finished correctly pending
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = ACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDReadState = SDREAD_WAIT_RESPONSE;
	SDcard.SdRx[0] = 0;
	SDcard.SdCtr = 1;
	EXPECT_EQ(readBlock(testblockaddr, testrxbuff), SDREAD_WAIT_FE);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);

	//wait response, finishedin correctly
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = ACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDReadState = SDREAD_WAIT_RESPONSE;
	SDcard.SdRx[0] = 1;
	SDcard.SdCtr = 1;
	EXPECT_EQ(readBlock(testblockaddr, testrxbuff), SDREAD_FAILED);

	//wait FE, not inactive yet
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = ACTIVE;
	SDcard.SDReadState = SDREAD_WAIT_FE;
	SDcard.SdRx[0] = 1;
	SDcard.SdCtr = 1;
	EXPECT_EQ(readBlock(testblockaddr, testrxbuff), SDREAD_WAIT_FE);

	//wait FE, finished, incorrect response
	SPI.spiActivityGyro = ACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDReadState = SDREAD_WAIT_FE;
	SDcard.SdRx[0] = 0xFF;
	EXPECT_EQ(readBlock(testblockaddr, testrxbuff), SDREAD_WAIT_FE);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);

	//wait FE, finished correctly
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDReadState = SDREAD_WAIT_FE;
	SDcard.SdRx[0] = 0xFE;
	EXPECT_EQ(readBlock(testblockaddr, testrxbuff), SDREAD_WAIT_DATA);
	EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);

	//wait data
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = ACTIVE;
	SDcard.SDReadState = SDREAD_WAIT_DATA;
	SDcard.SdRx[0] = 0xFE;
	EXPECT_EQ(readBlock(testblockaddr, testrxbuff), SDREAD_WAIT_DATA);

	//wait data
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDReadState = SDREAD_WAIT_DATA;
	SDcard.SdRx[0] = 0xFE;
	EXPECT_EQ(readBlock(testblockaddr, testrxbuff), SDREAD_FINISHED);
}

TEST(test_BT_SDcard, readBoot_Test)
{
	//wait
	SDcard.SDReadState = SDREAD_START;
	EXPECT_EQ(readBoot(), SDINIT_READ_BOOT);
	SDcard.SDReadState = SDREAD_WAIT_RESPONSE;
	EXPECT_EQ(readBoot(), SDINIT_READ_BOOT);
	SDcard.SDReadState = SDREAD_WAIT_FE;
	EXPECT_EQ(readBoot(), SDINIT_READ_BOOT);
	SDcard.SDReadState = SDREAD_WAIT_DATA;
	EXPECT_EQ(readBoot(), SDINIT_READ_BOOT);

	//failed
	SDcard.SDReadState = SDREAD_FAILED;
	EXPECT_EQ(readBoot(), SDINIT_FAILURE);

	//finished
	SDcard.SDReadState = SDREAD_FINISHED;
	SDcard.bootSectorAddr = 0x00;
	SDcard.SdRx[14] = 0x5A;
	SDcard.SdRx[15] = 0x02;
	SDcard.SdRx[16] = 0x02;
	SDcard.SdRx[36] = 0xD3;
	SDcard.SdRx[37] = 0x0E;
	SDcard.SdRx[38] = 0x00;
	SDcard.SdRx[39] = 0x00;
	EXPECT_EQ(readBoot(), SDINIT_READ_ROOTDIR);
	EXPECT_EQ(SDcard.SDReadState, SDREAD_START);
}

TEST(test_BT_SDcard, getFileInfo_Test)
{
	uint8_t testFileInfo[32] = { 'T','E','S','T',0x20,0x20,0x20,0x20,'t','x','t',0x00,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x02,0x01,0x00,0x00,0x00,0x00,0x04,0x03,0x05,0x06,0x07,0x08 };
	//1
	fileInfo testFile = getFileInfo(testFileInfo);
	EXPECT_STREQ(testFile.name, "TEST");
	EXPECT_EQ(testFile.numberInName, 0);
	EXPECT_EQ(testFile.clusters[0], 0x01020304);
	EXPECT_EQ(testFile.size, 0x08070605);
	//2
	testFileInfo[4] = '_';
	testFileInfo[5] = '3';
	testFile = getFileInfo(testFileInfo);
	EXPECT_STREQ(testFile.name, "TEST");
	EXPECT_EQ(testFile.numberInName, 3);
	//3
	testFileInfo[4] = '_';
	testFileInfo[5] = '_';
	testFileInfo[6] = '5';
	testFile = getFileInfo(testFileInfo);
	EXPECT_STREQ(testFile.name, "TEST");
	EXPECT_EQ(testFile.numberInName, 5);
	//4
	testFileInfo[4] = '1';
	testFileInfo[5] = '3';
	testFileInfo[6] = '5';
	testFile = getFileInfo(testFileInfo);
	EXPECT_STREQ(testFile.name, "TEST");
	EXPECT_EQ(testFile.numberInName, 135);
	//5
	uint8_t testFileInfo2[] = { 0x42,0x20,0x00,0x49,0x00,0x6E,0x00,0x66,  0x00,0x6F,0x00,0x0F,0x00,0x72,0x72,0x00,
								0x6D,0x00,0x61,0x00,0x74,0x00,0x69,0x00,  0x6F,0x00,0x00,0x00,0x6E,0x00,0x00,0x00,
								0x01,0x53,0x00,0x79,0x00,0x73,0x00,0x74,  0x00,0x65,0x00,0x0F,0x00,0x72,0x6D,0x00,
								0x20,0x00,0x56,0x00,0x6F,0x00,0x6C,0x00,  0x75,0x00,0x00,0x00,0x6D,0x00,0x65,0x00,
								0x53,0x59,0x53,0x54,0x45,0x4D,0x7E,0x31,  0x20,0x20,0x20,0x16,0x00,0xC3,0xE3,0x52,
								0x9B,0x51,0x9B,0x51,0x00,0x00,0xE4,0x52,  0x9B,0x51,0x03,0x00,0x00,0x00,0x00,0x00,
								'P' ,'A' ,'R' ,'A' ,'M' ,0x20,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
								0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x06,0x00,0x05,0x00,0x00,0x00,
								'T' ,'E' ,'S' ,'T' ,'3' ,0x20,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
								0x00,0x00,0x00,0x00,0x01,0x02,0x00,0x00,  0x00,0x00,0x03,0x04,0x10,0x11,0x12,0x13 };
	testFile = getFileInfo(&testFileInfo2[0]);
	EXPECT_STREQ(testFile.name, "B");
	testFile = getFileInfo(&testFileInfo2[32]);
	EXPECT_STREQ(testFile.name, "");
	testFile = getFileInfo(&testFileInfo2[64]);
	EXPECT_STREQ(testFile.name, "SYSTEM");
	testFile = getFileInfo(&testFileInfo2[96]);
	EXPECT_STREQ(testFile.name, "PARAM");
	EXPECT_EQ(testFile.clusters[0], 0x06);
	EXPECT_EQ(testFile.size, 0x05);
	testFile = getFileInfo(&testFileInfo2[128]);
	EXPECT_STREQ(testFile.name, "TEST");
	EXPECT_EQ(testFile.numberInName, 3);
	EXPECT_EQ(testFile.clusters[0], 0x02010403);
	EXPECT_EQ(testFile.size, 0x13121110);
	//6
	testFileInfo[0] = 0xE5;
	testFile = getFileInfo(testFileInfo);
	EXPECT_STREQ(testFile.name, "DELETED");
	EXPECT_EQ(testFile.clusters[0], 0);
	EXPECT_EQ(testFile.size, 0);
}

TEST(test_BT_SDcard, readRoot_Test)
{
	//wait
	SDcard.SDReadState = SDREAD_START;
	EXPECT_EQ(readRoot(), SDINIT_READ_ROOTDIR);
	SDcard.SDReadState = SDREAD_WAIT_RESPONSE;
	EXPECT_EQ(readRoot(), SDINIT_READ_ROOTDIR);
	SDcard.SDReadState = SDREAD_WAIT_FE;
	EXPECT_EQ(readRoot(), SDINIT_READ_ROOTDIR);
	SDcard.SDReadState = SDREAD_WAIT_DATA;
	EXPECT_EQ(readRoot(), SDINIT_READ_ROOTDIR);

	//failed
	SDcard.SDReadState = SDREAD_FAILED;
	EXPECT_EQ(readRoot(), SDINIT_FAILURE);

	//finished, slot not found yet
	for (uint16_t i = 0; i < 512; i++)
	{
		SDcard.rootDirInfo_8b[i] = rootDirBase[i];
	}
	SDcard.SDReadState = SDREAD_FINISHED;
	EXPECT_EQ(readRoot(), SDINIT_READ_ROOTDIR);
	EXPECT_EQ(SDcard.SDReadState, SDREAD_START);
	EXPECT_EQ(SDcard.rootDirEmptySlotNumber, 0);
	EXPECT_EQ(SDcard.rootDirEmptyBlockNumber, 1);
	EXPECT_STREQ(SDcard.lastFile.name, "MEAS");
	EXPECT_EQ(SDcard.lastFile.numberInName, 12);

	//finished, slot found
	SDcard.rootDirInfo_8b[192] = 0x00;
    SDcard.SDReadState = SDREAD_FINISHED;
    EXPECT_EQ(readRoot(), SDINIT_READ_FAT);
    EXPECT_EQ(SDcard.SDReadState, SDREAD_START);
	EXPECT_STREQ(SDcard.lastFile.name, "MEAS");
	EXPECT_EQ(SDcard.lastFile.numberInName, 2);
	EXPECT_EQ(SDcard.rootDirInfo[0], 0xFE | SPI_TDR_PCS(CS_SDCARD));
	EXPECT_EQ(SDcard.rootDirInfo[1], uint32_t(rootDirBase[0]) | SPI_TDR_PCS(CS_SDCARD));
	EXPECT_EQ(SDcard.rootDirInfo[11], uint32_t(rootDirBase[10]) | SPI_TDR_PCS(CS_SDCARD));
	EXPECT_EQ(SDcard.rootDirInfo[101], uint32_t(rootDirBase[100]) | SPI_TDR_PCS(CS_SDCARD));
}

TEST(test_BT_SDcard, getAllFileClusters_Test)
{
	uint8_t l_testFAT[514] = { 0 };
	fileInfo l_testFile;

	//1
	l_testFile.numberOfClusters = 1;
	l_testFile.clusters[0] = 3;
	l_testFAT[0] = 0xF8; l_testFAT[1] = 0xFF; l_testFAT[2] = 0xFF; l_testFAT[3] = 0x0F;
	l_testFAT[4] = 0xFF; l_testFAT[5] = 0xFF; l_testFAT[6] = 0xFF; l_testFAT[7] = 0xFF;
	l_testFAT[8] = 0xFF; l_testFAT[9] = 0xFF; l_testFAT[10] = 0xFF; l_testFAT[11] = 0x0F;
	l_testFAT[12] = 0x04; l_testFAT[13] = 0x00; l_testFAT[14] = 0x00; l_testFAT[15] = 0x00;
	l_testFAT[16] = 0x05; l_testFAT[17] = 0x00; l_testFAT[18] = 0x00; l_testFAT[19] = 0x00;
	l_testFAT[20] = 0xFF; l_testFAT[21] = 0xFF; l_testFAT[22] = 0xFF; l_testFAT[23] = 0x0F;
	EXPECT_EQ(getAllFileClusters(l_testFAT, &l_testFile), true);
	EXPECT_EQ(l_testFile.numberOfClusters, 3);
	EXPECT_EQ(l_testFile.clusters[1], 4);
	EXPECT_EQ(l_testFile.clusters[2], 5);
	//2
	l_testFile.numberOfClusters = 1;
	l_testFile.clusters[0] = 6;
	l_testFAT[24] = 0x07; l_testFAT[25] = 0x00; l_testFAT[26] = 0x00; l_testFAT[27] = 0x00;
	l_testFAT[28] = 0x08; l_testFAT[29] = 0x00; l_testFAT[30] = 0x00; l_testFAT[31] = 0x00;
	l_testFAT[32] = 0x09; l_testFAT[33] = 0x00; l_testFAT[34] = 0x00; l_testFAT[35] = 0x00;
	l_testFAT[36] = 0x0A; l_testFAT[37] = 0x00; l_testFAT[38] = 0x00; l_testFAT[39] = 0x00;
	l_testFAT[40] = 0x0B; l_testFAT[41] = 0x00; l_testFAT[42] = 0x00; l_testFAT[43] = 0x00;
	l_testFAT[44] = 0x0C; l_testFAT[45] = 0x00; l_testFAT[46] = 0x00; l_testFAT[47] = 0x00;
	l_testFAT[48] = 0xFF; l_testFAT[49] = 0xFF; l_testFAT[50] = 0xFF; l_testFAT[51] = 0x0F;
	EXPECT_EQ(getAllFileClusters(l_testFAT, &l_testFile), true);
	EXPECT_EQ(l_testFile.numberOfClusters, 7);
	EXPECT_EQ(l_testFile.clusters[1], 7);
	EXPECT_EQ(l_testFile.clusters[2], 8);
	EXPECT_EQ(l_testFile.clusters[3], 9);
	EXPECT_EQ(l_testFile.clusters[4], 10);
	EXPECT_EQ(l_testFile.clusters[5], 11);
	EXPECT_EQ(l_testFile.clusters[6], 12);
	//3
	SDcard.FATBlockOffset = 0;
	l_testFile.numberOfClusters = 1;
	l_testFile.clusters[0] = 125;
	l_testFAT[500] = 0x7E; l_testFAT[501] = 0x00; l_testFAT[502] = 0x00; l_testFAT[503] = 0x00;
	l_testFAT[504] = 0x7F; l_testFAT[505] = 0x00; l_testFAT[506] = 0x00; l_testFAT[507] = 0x00;
	l_testFAT[508] = 0x80; l_testFAT[509] = 0x00; l_testFAT[510] = 0x00; l_testFAT[511] = 0x00;
	EXPECT_EQ(getAllFileClusters(l_testFAT, &l_testFile), false);
	EXPECT_EQ(l_testFile.numberOfClusters, 4);
	EXPECT_EQ(l_testFile.clusters[1], 126);
	EXPECT_EQ(l_testFile.clusters[2], 127);
	EXPECT_EQ(l_testFile.clusters[3], 128);
	EXPECT_EQ(SDcard.FATBlockOffset, 1);
	l_testFAT[0] = 0x81; l_testFAT[1] = 0x00; l_testFAT[2] = 0x00; l_testFAT[3] = 0x00;
	l_testFAT[4] = 0xFF; l_testFAT[5] = 0xFF; l_testFAT[6] = 0xFF; l_testFAT[7] = 0x0F;
	EXPECT_EQ(getAllFileClusters(l_testFAT, &l_testFile), true);
	EXPECT_EQ(l_testFile.numberOfClusters, 5);
	EXPECT_EQ(l_testFile.clusters[4], 129);
	EXPECT_EQ(SDcard.FATBlockOffset, 1);
	//4
	l_testFile.numberOfClusters = 1;
	l_testFile.clusters[0] = 254;
	l_testFAT[504] = 0xFF; l_testFAT[505] = 0x00; l_testFAT[506] = 0x00; l_testFAT[507] = 0x00;
	l_testFAT[508] = 0x00; l_testFAT[509] = 0x01; l_testFAT[510] = 0x00; l_testFAT[511] = 0x00;
	EXPECT_EQ(getAllFileClusters(l_testFAT, &l_testFile), false);
	EXPECT_EQ(l_testFile.numberOfClusters, 3);
	EXPECT_EQ(l_testFile.clusters[1], 255);
	EXPECT_EQ(l_testFile.clusters[2], 256);
	EXPECT_EQ(SDcard.FATBlockOffset, 2);
	l_testFAT[0] = 0x01; l_testFAT[1] = 0x01; l_testFAT[2] = 0x00; l_testFAT[3] = 0x00;
	l_testFAT[4] = 0xFF; l_testFAT[5] = 0xFF; l_testFAT[6] = 0xFF; l_testFAT[7] = 0x0F;
	EXPECT_EQ(getAllFileClusters(l_testFAT, &l_testFile), true);
	EXPECT_EQ(l_testFile.numberOfClusters, 4);
	EXPECT_EQ(l_testFile.clusters[3], 257);
	EXPECT_EQ(SDcard.FATBlockOffset, 2);
	//5
	l_testFile.numberOfClusters = 1;
	l_testFile.clusters[0] = 0;
	l_testFAT[0] = 0x01; l_testFAT[1] = 0x01; l_testFAT[2] = 0x01; l_testFAT[3] = 0x01;
	l_testFAT[4] = 0xFF; l_testFAT[5] = 0xFF; l_testFAT[6] = 0xFF; l_testFAT[7] = 0x0F;
	EXPECT_EQ(getAllFileClusters(l_testFAT, &l_testFile), true);
	EXPECT_EQ(l_testFile.numberOfClusters, 2);
	EXPECT_EQ(l_testFile.clusters[1], 16843009);
	//6
	l_testFile.numberOfClusters = 1;
	l_testFile.clusters[0] = 119;
	EXPECT_EQ(getAllFileClusters(FAT1Base2, &l_testFile), true);
	EXPECT_EQ(l_testFile.numberOfClusters, 9);
	EXPECT_EQ(l_testFile.clusters[1], 120);
	//7
	SDcard.FATBlockOffset = 0;
	l_testFile.numberOfClusters = 1;
	l_testFile.clusters[0] = 119;
	EXPECT_EQ(getAllFileClusters(FAT1Base2, &l_testFile), true);
	EXPECT_EQ(l_testFile.numberOfClusters, 9);
	EXPECT_EQ(l_testFile.clusters[1], 120);
	EXPECT_EQ(SDcard.FATBlockOffset, 0);
}

TEST(test_BT_SDcard, readFAT_Test)
{
	//wait
	SDcard.SDReadState = SDREAD_START;
	EXPECT_EQ(readFAT(), SDINIT_READ_FAT);
	SDcard.SDReadState = SDREAD_WAIT_RESPONSE;
	EXPECT_EQ(readFAT(), SDINIT_READ_FAT);
	SDcard.SDReadState = SDREAD_WAIT_FE;
	EXPECT_EQ(readFAT(), SDINIT_READ_FAT);
	SDcard.SDReadState = SDREAD_WAIT_DATA;
	EXPECT_EQ(readFAT(), SDINIT_READ_FAT);

	//failed
	SDcard.SDReadState = SDREAD_FAILED;
	EXPECT_EQ(readFAT(), SDINIT_FAILURE);

	//finished, slot not found yet
	for (uint16_t i = 0; i < 512; i++)
	{
		SDcard.FAT1Info_8b[i] = FAT1Base[i];
	}
	SDcard.lastFile.numberOfClusters = 1;
	SDcard.lastFile.clusters[0] = 119;
	SDcard.SDReadState = SDREAD_FINISHED;
	EXPECT_EQ(readFAT(), SDINIT_READ_FAT);
	EXPECT_EQ(SDcard.SDReadState, SDREAD_START);

	//finished, slot found, on PARAM file
	SDcard.lastFile.name[0] = 'P';
	SDcard.lastFile.name[1] = 'A';
	SDcard.lastFile.name[2] = 'R';
	SDcard.lastFile.name[3] = 'A';
	SDcard.lastFile.name[4] = 'M';
	SDcard.lastFile.numberInName = 0;
	SDcard.lastFile.numberOfClusters = 1;
	SDcard.lastFile.clusters[0] = 196;
	SDcard.lastFile.clusters[1] = 0;
	SDcard.lastFile.clusters[2] = 0;
	SDcard.SDReadState = SDREAD_FINISHED;
	EXPECT_EQ(readFAT(), SDINIT_SUCCESS);
	EXPECT_EQ(SDcard.SDReadState, SDREAD_FINISHED);
	EXPECT_STREQ(SDcard.newFile.name, "MEAS");
	EXPECT_EQ(SDcard.newFile.numberInName, 1);

	//finished, slot found, on MEAS4 file
	SDcard.lastFile.name[0] = 'M';
	SDcard.lastFile.name[1] = 'E';
	SDcard.lastFile.name[2] = 'A';
	SDcard.lastFile.name[3] = 'S';
	SDcard.lastFile.numberInName = 4;
	SDcard.lastFile.numberOfClusters = 1;
	SDcard.lastFile.clusters[0] = 196;
	SDcard.lastFile.clusters[1] = 0;
	SDcard.lastFile.clusters[2] = 0;
	SDcard.SDReadState = SDREAD_FINISHED;
	EXPECT_EQ(readFAT(), SDINIT_SUCCESS);
	EXPECT_EQ(SDcard.SDReadState, SDREAD_FINISHED);
	EXPECT_STREQ(SDcard.newFile.name, "MEAS");
	EXPECT_EQ(SDcard.newFile.numberInName, 5);
}

TEST(test_BT_SDcard, setupSdCard_Test)
{
	//CMD0 not ok
	SDcard.SDInitStatus = SDINIT_CMD0;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD0);
	SDcard.spiActivitySDCard = ACTIVE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD0);
	SDcard.SdRx[88] = 0;
	SDcard.spiActivitySDCard = INACTIVE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_FAILURE);
	//CMD0 ok
	SDcard.SDInitStatus = SDINIT_CMD0;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD0);
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SdRx[87] = 1;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD8);

	//CMD8 not ok
	SDcard.SDInitStatus = SDINIT_CMD8;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD8);
	SDcard.spiActivitySDCard = ACTIVE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD8);
	SDcard.SdRx[7] = 0;
	SDcard.spiActivitySDCard = INACTIVE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_FAILURE);
	//CMD8 ok
	SDcard.SDInitStatus = SDINIT_CMD8;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD8);
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SdRx[7] = 1;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD58);

	//CMD58 not ok
	SDcard.SDInitStatus = SDINIT_CMD58;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD58);
	SDcard.spiActivitySDCard = ACTIVE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD58);
	SDcard.SdRx[7] = 0;
	SDcard.spiActivitySDCard = INACTIVE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_FAILURE);
	//CMD58 ok
	SDcard.SDInitStatus = SDINIT_CMD58;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD58);
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SdRx[7] = 1;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD55);

	//CMD55 not ok
	SDcard.SDInitStatus = SDINIT_CMD55;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD55);
	SDcard.spiActivitySDCard = ACTIVE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD55);
	SDcard.SdRx[7] = 0;
	SDcard.spiActivitySDCard = INACTIVE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_FAILURE);
	//CMD55 ok
	SDcard.SDInitStatus = SDINIT_CMD55;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD55);
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SdRx[7] = 1;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_ACMD41);

	//ACMD41 not ok
	SDcard.SDInitStatus = SDINIT_ACMD41;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_ACMD41);
	SDcard.spiActivitySDCard = ACTIVE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_ACMD41);
	SDcard.SdRx[7] = 1;
	SDcard.spiActivitySDCard = INACTIVE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_CMD55);
	//ACMD41 ok
	SDcard.SDInitStatus = SDINIT_ACMD41;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_ACMD41);
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SdRx[7] = 0;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_00);

	//read 00 ok
	SDcard.SDInitStatus = SDINIT_READ_00;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_BOOT);

	//read boot
	SDcard.SDInitStatus = SDINIT_READ_BOOT;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_START;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_BOOT);
	SDcard.SDInitStatus = SDINIT_READ_BOOT;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_WAIT_RESPONSE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_BOOT);
	SDcard.SDInitStatus = SDINIT_READ_BOOT;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_WAIT_FE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_BOOT);
	SDcard.SDInitStatus = SDINIT_READ_BOOT;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_WAIT_DATA;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_BOOT);
	SDcard.SDInitStatus = SDINIT_READ_BOOT;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_FAILED;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_FAILURE);
	SDcard.SDInitStatus = SDINIT_READ_BOOT;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_FINISHED;
	SDcard.bootSectorAddr = 0x00;
	SDcard.SdRx[14] = 0x00;
	SDcard.SdRx[15] = 0x00;
	SDcard.SdRx[16] = 0x00;
	SDcard.SdRx[36] = 0x00;
	SDcard.SdRx[37] = 0x00;
	SDcard.SdRx[38] = 0x00;
	SDcard.SdRx[39] = 0x00;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_FAILURE);
	//read boot ok
	SDcard.SDInitStatus = SDINIT_READ_BOOT;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_FINISHED;
	SDcard.bootSectorAddr = 0x00;
	SDcard.SdRx[14] = 0x5A;
	SDcard.SdRx[15] = 0x02;
	SDcard.SdRx[16] = 0x02;
	SDcard.SdRx[36] = 0xD3;
	SDcard.SdRx[37] = 0x0E;
	SDcard.SdRx[38] = 0x00;
	SDcard.SdRx[39] = 0x00;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_ROOTDIR);

	//read root
	SDcard.SDInitStatus = SDINIT_READ_ROOTDIR;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_START;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_ROOTDIR);
	SDcard.SDInitStatus = SDINIT_READ_ROOTDIR;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_WAIT_RESPONSE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_ROOTDIR);
	SDcard.SDInitStatus = SDINIT_READ_ROOTDIR;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_WAIT_FE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_ROOTDIR);
	SDcard.SDInitStatus = SDINIT_READ_ROOTDIR;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_WAIT_DATA;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_ROOTDIR);
	SDcard.SDInitStatus = SDINIT_READ_ROOTDIR;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_FAILED;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_FAILURE);
	//read root need next cluster
	SDcard.SDInitStatus = SDINIT_READ_ROOTDIR;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	for (uint16_t i = 0; i < 512; i++)
	{
		SDcard.rootDirInfo_8b[i] = rootDirBase[i];
	}
	SDcard.SDReadState = SDREAD_FINISHED;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_ROOTDIR);
	//read root ok
	SDcard.SDInitStatus = SDINIT_READ_ROOTDIR;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.rootDirInfo_8b[192] = 0x00;
	SDcard.SDReadState = SDREAD_FINISHED;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_FAT);

	//read FAT
	SDcard.SDInitStatus = SDINIT_READ_FAT;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_START;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_FAT);
	SDcard.SDInitStatus = SDINIT_READ_FAT;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_WAIT_RESPONSE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_FAT);
	SDcard.SDInitStatus = SDINIT_READ_FAT;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_WAIT_FE;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_FAT);
	SDcard.SDInitStatus = SDINIT_READ_FAT;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_WAIT_DATA;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_FAT);
	SDcard.SDInitStatus = SDINIT_READ_FAT;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_FAILED;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_FAILURE);
	//read FAT need next cluster
	SDcard.SDInitStatus = SDINIT_READ_FAT;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_FINISHED;
	for (uint16_t i = 0; i < 512; i++)
	{
		SDcard.FAT1Info_8b[i] = FAT1Base[i];
	}
	SDcard.lastFile.numberOfClusters = 1;
	SDcard.lastFile.clusters[0] = 119;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_READ_FAT);
	EXPECT_EQ(SDcard.SDReadState, SDREAD_START);
	//reed FAT ok
	SDcard.SDInitStatus = SDINIT_READ_FAT;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	SDcard.SDReadState = SDREAD_FINISHED;
	SDcard.lastFile.name[0] = 'M';
	SDcard.lastFile.name[1] = 'E';
	SDcard.lastFile.name[2] = 'A';
	SDcard.lastFile.name[3] = 'S';
	SDcard.lastFile.numberInName = 4;
	SDcard.lastFile.numberOfClusters = 1;
	SDcard.lastFile.clusters[0] = 196;
	SDcard.lastFile.clusters[1] = 0;
	SDcard.lastFile.clusters[2] = 0;
	EXPECT_EQ(SetupSdCard(), SD_INIT);
	EXPECT_EQ(SDcard.SDInitStatus, SDINIT_SUCCESS);
	EXPECT_EQ(SetupSdCard(), SD_WAIT_4_MEASUREMENT);

	//failed
	SDcard.SDInitStatus = SDINIT_FAILURE;
	EXPECT_EQ(SetupSdCard(), SD_DO_NOTHING);

}

TEST(test_BT_SDcard, swapDataBufferPointers_Test)
{
	SDcard.newFile.size = 0;
	SDcard.newFile.blockCount = 0;
	SDcard.loadingDataPointer = SDcard.dataBuffer1;
	SDcard.sendingDataPointer = SDcard.dataBuffer2;
	EXPECT_EQ(SDcard.loadingDataPointer[0] & 0xFF, 0xFE);
	EXPECT_EQ(SDcard.sendingDataPointer[0] & 0xFF, 0xFE);
	//1
	SDcard.writeMeasData = false;
	swapDataBufferPointers();
	EXPECT_EQ(SDcard.loadingDataPointer, SDcard.dataBuffer2);
	EXPECT_EQ(SDcard.sendingDataPointer, SDcard.dataBuffer1);
	EXPECT_EQ(SDcard.newFile.size, 512);
	EXPECT_EQ(SDcard.newFile.blockCount, 1);
	EXPECT_EQ(SDcard.writeMeasData, true);
	//2
	SDcard.newFile.size = 512 * 20;
	SDcard.newFile.blockCount = 50;
	swapDataBufferPointers();
	EXPECT_EQ(SDcard.loadingDataPointer, SDcard.dataBuffer1);
	EXPECT_EQ(SDcard.sendingDataPointer, SDcard.dataBuffer2);
	EXPECT_EQ(SDcard.newFile.size, 512 * 21);
	EXPECT_EQ(SDcard.newFile.blockCount, 51);
	//3
	SDcard.newFile.size = 512 * 127;
	SDcard.newFile.blockCount = 63;
	swapDataBufferPointers();
	EXPECT_EQ(SDcard.loadingDataPointer, SDcard.dataBuffer2);
	EXPECT_EQ(SDcard.sendingDataPointer, SDcard.dataBuffer1);
	EXPECT_EQ(SDcard.newFile.size, 512 * 128);
	EXPECT_EQ(SDcard.newFile.blockCount, 64);
	//4
	swapDataBufferPointers();
	EXPECT_EQ(SDcard.loadingDataPointer, SDcard.dataBuffer1);
	EXPECT_EQ(SDcard.sendingDataPointer, SDcard.dataBuffer2);
	EXPECT_EQ(SDcard.newFile.size, 512 * 129);
	EXPECT_EQ(SDcard.newFile.blockCount, 1);

}

TEST(test_BT_SDcard, appendCsSdCard_Test)
{
	volatile uint32_t testblock[10];

	//append 0
	testblock[0] = 0;
	testblock[1] = 1;
	testblock[2] = 2;
	appendCsSdCard(testblock, 0);
	EXPECT_EQ(testblock[0], 0);
	EXPECT_EQ(testblock[1], 1);
	EXPECT_EQ(testblock[2], 2);
	//append 1
	testblock[0] = 0;
	testblock[1] = 1;
	testblock[2] = 2;
	appendCsSdCard(testblock, 1);
	EXPECT_NE(testblock[0], 0);
	EXPECT_EQ(testblock[1], 1);
	EXPECT_EQ(testblock[2], 2);
	//append 2
	testblock[0] = 0;
	testblock[1] = 1;
	testblock[2] = 2;
	appendCsSdCard(testblock, 2);
	EXPECT_NE(testblock[0], 0);
	EXPECT_NE(testblock[1], 1);
	EXPECT_EQ(testblock[2], 2);
	//append 2
	testblock[0] = 0;
	testblock[1] = 1;
	testblock[2] = 2;
	testblock[3] = 3;
	testblock[4] = 4;
	testblock[5] = 5;
	testblock[6] = 6;
	testblock[7] = 7;
	testblock[8] = 8;
	testblock[9] = 9;
	appendCsSdCard(testblock, 10);
	EXPECT_NE(testblock[0], 0);
	EXPECT_NE(testblock[1], 1);
	EXPECT_NE(testblock[2], 2);
	EXPECT_NE(testblock[3], 3);
	EXPECT_NE(testblock[4], 4);
	EXPECT_NE(testblock[5], 5);
	EXPECT_NE(testblock[6], 6);
	EXPECT_NE(testblock[7], 7);
	EXPECT_NE(testblock[8], 8);
	EXPECT_NE(testblock[9], 9);
}

TEST(test_BT_SDcard, convert2String_Test)
{
	uint32_t testBuffer[30];
	uint8_t testNumberOfChar{ 0 };

	//int, 1digit
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, 5, 0, false);
	EXPECT_EQ(testBuffer[0], '5');
	EXPECT_EQ(testNumberOfChar, 1);
	//int, 2digit
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, 23, 0, false);
	EXPECT_EQ(testBuffer[0], '2');
	EXPECT_EQ(testBuffer[1], '3');
	EXPECT_EQ(testNumberOfChar, 2);
	//int, 3digit
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, 346, 0, false);
	EXPECT_EQ(testBuffer[0], '3');
	EXPECT_EQ(testBuffer[1], '4');
	EXPECT_EQ(testBuffer[2], '6');
	EXPECT_EQ(testNumberOfChar, 3);
	//int, negative
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, -89, 0, false);
	EXPECT_EQ(testBuffer[0], '-');
	EXPECT_EQ(testBuffer[1], '8');
	EXPECT_EQ(testBuffer[2], '9');
	EXPECT_EQ(testNumberOfChar, 3);
	//int, explicit + sign
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, 43, 0, true);
	EXPECT_EQ(testBuffer[0], '+');
	EXPECT_EQ(testBuffer[1], '4');
	EXPECT_EQ(testBuffer[2], '3');
	EXPECT_EQ(testNumberOfChar, 3);
	//int, explicit + sign, negative
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, -145, 0, true);
	EXPECT_EQ(testBuffer[0], '-');
	EXPECT_EQ(testBuffer[1], '1');
	EXPECT_EQ(testBuffer[2], '4');
	EXPECT_EQ(testBuffer[3], '5');
	EXPECT_EQ(testNumberOfChar, 4);
	//float, just int part
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, 1.5, 0, false);
	EXPECT_EQ(testBuffer[0], '1');
	EXPECT_EQ(testNumberOfChar, 1);
	//float, 1 fractional
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, 1.5, 1, false);
	EXPECT_EQ(testBuffer[0], '1');
	EXPECT_EQ(testBuffer[1], '.');
	EXPECT_EQ(testBuffer[2], '5');
	EXPECT_EQ(testNumberOfChar, 3);
	//float, 3 fractional
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, 791.546, 3, false);
	EXPECT_EQ(testBuffer[0], '7');
	EXPECT_EQ(testBuffer[1], '9');
	EXPECT_EQ(testBuffer[2], '1');
	EXPECT_EQ(testBuffer[3], '.');
	EXPECT_EQ(testBuffer[4], '5');
	EXPECT_EQ(testBuffer[5], '4');
	EXPECT_EQ(testBuffer[6], '6');
	EXPECT_EQ(testNumberOfChar, 7);
	//float, negative, 5 fractional
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, -18.46875, 5, false);
	EXPECT_EQ(testBuffer[0], '-');
	EXPECT_EQ(testBuffer[1], '1');
	EXPECT_EQ(testBuffer[2], '8');
	EXPECT_EQ(testBuffer[3], '.');
	EXPECT_EQ(testBuffer[4], '4');
	EXPECT_EQ(testBuffer[5], '6');
	EXPECT_EQ(testBuffer[6], '8');
	EXPECT_EQ(testBuffer[7], '7');
	EXPECT_EQ(testBuffer[8], '5');
	EXPECT_EQ(testNumberOfChar, 9);
	//float, explict +sign, 3 fractional
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, 6.731, 3, true);
	EXPECT_EQ(testBuffer[0], '+');
	EXPECT_EQ(testBuffer[1], '6');
	EXPECT_EQ(testBuffer[2], '.');
	EXPECT_EQ(testBuffer[3], '7');
	EXPECT_EQ(testBuffer[4], '3');
	EXPECT_EQ(testBuffer[5], '1');
	EXPECT_EQ(testNumberOfChar, 6);
	//float, smaller than 1
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, 0.942, 3, false);
	EXPECT_EQ(testBuffer[0], '0');
	EXPECT_EQ(testBuffer[1], '.');
	EXPECT_EQ(testBuffer[2], '9');
	EXPECT_EQ(testBuffer[3], '4');
	EXPECT_EQ(testBuffer[4], '2');
	EXPECT_EQ(testNumberOfChar, 5);
	//float, smaller than 1, explicit sign
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, 0.681, 3, true);
	EXPECT_EQ(testBuffer[0], '+');
	EXPECT_EQ(testBuffer[1], '0');
	EXPECT_EQ(testBuffer[2], '.');
	EXPECT_EQ(testBuffer[3], '6');
	EXPECT_EQ(testBuffer[4], '8');
	EXPECT_EQ(testBuffer[5], '1');
	EXPECT_EQ(testNumberOfChar, 6);
	//float, smaller than 1, 
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, -0.123, 3, false);
	EXPECT_EQ(testBuffer[0], '-');
	EXPECT_EQ(testBuffer[1], '0');
	EXPECT_EQ(testBuffer[2], '.');
	EXPECT_EQ(testBuffer[3], '1');
	EXPECT_EQ(testBuffer[4], '2');
	EXPECT_EQ(testBuffer[5], '3');
	EXPECT_EQ(testNumberOfChar, 6);
	//float, smaller than 1, 0 fractional, negative
	testNumberOfChar = 0;
	convert2String(testBuffer, &testNumberOfChar, -0.123, 0, false);
	EXPECT_EQ(testBuffer[0], '-');
	EXPECT_EQ(testBuffer[1], '0');
	EXPECT_EQ(testNumberOfChar, 2);
}

TEST(test_BT_SDcard, appendComma_Test)
{
	//
	SDcard.loadingDataPointer[1] = '1';
	SDcard.loadingDataCounter = 2;
	appendComma();
	EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], '1');
	EXPECT_EQ(SDcard.loadingDataPointer[2], ',');
	EXPECT_EQ(SDcard.loadingDataCounter, 3);
}

TEST(test_BT_SDcard, appendNewLine_Test)
{
	//
	SDcard.loadingDataPointer[1] = '5';
	SDcard.loadingDataCounter = 2;
	appendNewLine();
	EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], '5');
	EXPECT_EQ(SDcard.loadingDataPointer[2], '\n');
	EXPECT_EQ(SDcard.loadingDataCounter, 3);
}

TEST(test_BT_SDcard, loadData2Buffer_Test)
{
	uint32_t testBuffer[30];
	uint8_t testNumber{ 0 };

	//int 1 char
	SDcard.loadingDataCounter = 1;
	testNumber = 0;
	testBuffer[testNumber++] = '6';
	loadData2Buffer(testBuffer, testNumber);
	EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], '6');
	EXPECT_EQ(SDcard.loadingDataCounter, 2);
	//int additional 2 char
	testNumber = 0;
	testBuffer[testNumber++] = '2';
	testBuffer[testNumber++] = '7';
	loadData2Buffer(testBuffer, testNumber);
	EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], '6');
	EXPECT_EQ(SDcard.loadingDataPointer[2], '2');
	EXPECT_EQ(SDcard.loadingDataPointer[3], '7');
	EXPECT_EQ(SDcard.loadingDataCounter, 4);
	//int 4 char
	SDcard.loadingDataCounter = 1;
	testNumber = 0;
	testBuffer[testNumber++] = '1';
	testBuffer[testNumber++] = '2';
	testBuffer[testNumber++] = '3';
	testBuffer[testNumber++] = '4';
	loadData2Buffer(testBuffer, testNumber);
	EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], '1');
	EXPECT_EQ(SDcard.loadingDataPointer[2], '2');
	EXPECT_EQ(SDcard.loadingDataPointer[3], '3');
	EXPECT_EQ(SDcard.loadingDataPointer[4], '4'); 
	EXPECT_EQ(SDcard.loadingDataCounter, 5);
	//float 3 fractional
	SDcard.loadingDataCounter = 1;
	testNumber = 0;
	testBuffer[testNumber++] = '5';
	testBuffer[testNumber++] = '.';
	testBuffer[testNumber++] = '4';
	testBuffer[testNumber++] = '3';
	testBuffer[testNumber++] = '2';
	loadData2Buffer(testBuffer, testNumber);
	EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], '5');
	EXPECT_EQ(SDcard.loadingDataPointer[2], '.');
	EXPECT_EQ(SDcard.loadingDataPointer[3], '4');
	EXPECT_EQ(SDcard.loadingDataPointer[4], '3');
	EXPECT_EQ(SDcard.loadingDataPointer[5], '2');
	EXPECT_EQ(SDcard.loadingDataCounter, 6);
	//float 3 fractional, negative, less than 1
	SDcard.loadingDataCounter = 1;
	testNumber = 0;
	testBuffer[testNumber++] = '-';
	testBuffer[testNumber++] = '0';
	testBuffer[testNumber++] = '.';
	testBuffer[testNumber++] = '8';
	testBuffer[testNumber++] = '5';
	testBuffer[testNumber++] = '4';
	loadData2Buffer(testBuffer, testNumber);
	EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], '-');
	EXPECT_EQ(SDcard.loadingDataPointer[2], '0');
	EXPECT_EQ(SDcard.loadingDataPointer[3], '.');
	EXPECT_EQ(SDcard.loadingDataPointer[4], '8');
	EXPECT_EQ(SDcard.loadingDataPointer[5], '5');
	EXPECT_EQ(SDcard.loadingDataPointer[6], '4');
	EXPECT_EQ(SDcard.loadingDataCounter, 7);
	// filling the buffer
	SDcard.sendingDataPointer[510] = 0;
	SDcard.sendingDataPointer[511] = 0;
	SDcard.loadingDataCounter = 510;
	testNumber = 0;
	testBuffer[testNumber++] = '1';
	testBuffer[testNumber++] = '5';
	testBuffer[testNumber++] = '7';
	loadData2Buffer(testBuffer, testNumber);
	EXPECT_EQ(SDcard.sendingDataPointer[510], '1');
	EXPECT_EQ(SDcard.sendingDataPointer[511], '5');
	EXPECT_EQ(SDcard.sendingDataPointer[512], '7');
	EXPECT_EQ(SDcard.loadingDataCounter, 1);
	// split data
	SDcard.sendingDataPointer[510] = 0;
	SDcard.sendingDataPointer[511] = 0;
	SDcard.sendingDataPointer[1] = 0;
	SDcard.sendingDataPointer[2] = 0;
	SDcard.loadingDataCounter = 510;
	testNumber = 0;
	testBuffer[testNumber++] = '4';
	testBuffer[testNumber++] = '.';
	testBuffer[testNumber++] = '8';
	testBuffer[testNumber++] = '3';
	testBuffer[testNumber++] = '1';
	loadData2Buffer(testBuffer, testNumber);
	EXPECT_EQ(SDcard.sendingDataPointer[510], '4');
	EXPECT_EQ(SDcard.sendingDataPointer[511], '.');
	EXPECT_EQ(SDcard.sendingDataPointer[512], '8');
	EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], '3');
	EXPECT_EQ(SDcard.loadingDataPointer[2], '1');
	EXPECT_EQ(SDcard.loadingDataCounter, 3);
}

TEST(test_BT_SDcard, measureData_Test)
{
    //not measured
    SDcard.loadingDataCounter = 1;
    measureData(false, false, 1.000f, 0, false, "test");
    EXPECT_EQ(SDcard.loadingDataCounter, 1);
    //integer
    SDcard.loadingDataCounter = 1;
    measureData(true, false, 1.234f, 0, false, "test");
    EXPECT_EQ(SDcard.loadingDataCounter, 2);
    EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
    EXPECT_EQ(SDcard.loadingDataPointer[1], '1');
    //float 3 fractional
    SDcard.loadingDataCounter = 1;
    measureData(true, false, 1.234f, 3, false, "test");
    EXPECT_EQ(SDcard.loadingDataCounter, 6);
    EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
    EXPECT_EQ(SDcard.loadingDataPointer[1], '1');
    EXPECT_EQ(SDcard.loadingDataPointer[2], '.');
    EXPECT_EQ(SDcard.loadingDataPointer[3], '2');
    EXPECT_EQ(SDcard.loadingDataPointer[4], '3');
    EXPECT_EQ(SDcard.loadingDataPointer[5], '4');
    //float 3 fractional negative
    SDcard.loadingDataCounter = 1;
    measureData(true, false, -9.876f, 3, false, "test");
    EXPECT_EQ(SDcard.loadingDataCounter, 7);
    EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
    EXPECT_EQ(SDcard.loadingDataPointer[1], '-');
    EXPECT_EQ(SDcard.loadingDataPointer[2], '9');
    EXPECT_EQ(SDcard.loadingDataPointer[3], '.');
    EXPECT_EQ(SDcard.loadingDataPointer[4], '8');
    EXPECT_EQ(SDcard.loadingDataPointer[5], '7');
    EXPECT_EQ(SDcard.loadingDataPointer[6], '6');
    //commaed float 3 fractional explicit sign
    measureData(true, true, 6.543f, 3, true, "test");
    EXPECT_EQ(SDcard.loadingDataCounter, 14);
    EXPECT_EQ(SDcard.loadingDataPointer[7], ',');
    EXPECT_EQ(SDcard.loadingDataPointer[8], '+');
    EXPECT_EQ(SDcard.loadingDataPointer[9], '6');
    EXPECT_EQ(SDcard.loadingDataPointer[10], '.');
    EXPECT_EQ(SDcard.loadingDataPointer[11], '5');
    EXPECT_EQ(SDcard.loadingDataPointer[12], '4');
    EXPECT_EQ(SDcard.loadingDataPointer[13], '3');
}

TEST(test_BT_SDcard, saveMeasData_Test)
{
	SpiInput testIn;
	SPIOutput testOut;

	// systime measured
    meas2Card.measureSysTime = true;
    meas2Card.measureGyroRawX = false;
    meas2Card.measureGyroRawY = false;
    meas2Card.measureGyroRawZ = false;
	testIn.sysTime = 1.234;
	testIn.gyro.signal.x = 2.345;
	testIn.gyro.signal.y = 0.456;
	testIn.gyro.signal.z = -7.896;
	SDcard.loadingDataCounter = 1;
	saveMeasData(&testIn, &testOut);
	EXPECT_EQ(SDcard.loadingDataPointer[0],0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1],'1');
	EXPECT_EQ(SDcard.loadingDataPointer[2],'.');
	EXPECT_EQ(SDcard.loadingDataPointer[3],'2');
	EXPECT_EQ(SDcard.loadingDataPointer[4],'3');
	EXPECT_EQ(SDcard.loadingDataPointer[5],'4');
	EXPECT_EQ(SDcard.loadingDataPointer[6],'\n');
	EXPECT_EQ(SDcard.loadingDataCounter,7);
	// systime+gyroX measured
	meas2Card.measureSysTime = true;
	meas2Card.measureGyroRawX = true;
	meas2Card.measureGyroRawY = false;
	meas2Card.measureGyroRawZ = false;
	testIn.sysTime = 5.678;
	testIn.gyro.signal.x = 2.345;
	testIn.gyro.signal.y = 0.456;
	testIn.gyro.signal.z = -7.896;
	SDcard.loadingDataCounter = 1;
	saveMeasData(&testIn, &testOut);
	EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], '5');
	EXPECT_EQ(SDcard.loadingDataPointer[2], '.');
	EXPECT_EQ(SDcard.loadingDataPointer[3], '6');
	EXPECT_EQ(SDcard.loadingDataPointer[4], '7');
	EXPECT_EQ(SDcard.loadingDataPointer[5], '8');
	EXPECT_EQ(SDcard.loadingDataPointer[6], ',');
	EXPECT_EQ(SDcard.loadingDataPointer[7], '2');
	EXPECT_EQ(SDcard.loadingDataPointer[8], '.');
	EXPECT_EQ(SDcard.loadingDataPointer[9], '3');
	EXPECT_EQ(SDcard.loadingDataPointer[10], '4');
	EXPECT_EQ(SDcard.loadingDataPointer[11], '5');
	EXPECT_EQ(SDcard.loadingDataPointer[12], '\n');
	EXPECT_EQ(SDcard.loadingDataCounter, 13);
	// systime+gyroY+gyroZ measured
	meas2Card.measureSysTime = true;
	meas2Card.measureGyroRawX = false;
	meas2Card.measureGyroRawY = true;
	meas2Card.measureGyroRawZ = true;
	testIn.sysTime = 8.271;
	testIn.gyro.signal.x = 2.345;
	testIn.gyro.signal.y = 0.456;
	testIn.gyro.signal.z = -7.896;
	saveMeasData(&testIn, &testOut);
	EXPECT_EQ(SDcard.loadingDataPointer[13], '8');
	EXPECT_EQ(SDcard.loadingDataPointer[14], '.');
	EXPECT_EQ(SDcard.loadingDataPointer[15], '2');
	EXPECT_EQ(SDcard.loadingDataPointer[16], '7');
	EXPECT_EQ(SDcard.loadingDataPointer[17], '1');
	EXPECT_EQ(SDcard.loadingDataPointer[18], ',');
	EXPECT_EQ(SDcard.loadingDataPointer[19], '0');
	EXPECT_EQ(SDcard.loadingDataPointer[20], '.');
	EXPECT_EQ(SDcard.loadingDataPointer[21], '4');
	EXPECT_EQ(SDcard.loadingDataPointer[22], '5');
	EXPECT_EQ(SDcard.loadingDataPointer[23], '6');
	EXPECT_EQ(SDcard.loadingDataPointer[24], ',');
	EXPECT_EQ(SDcard.loadingDataPointer[25], '-');
	EXPECT_EQ(SDcard.loadingDataPointer[26], '7');
	EXPECT_EQ(SDcard.loadingDataPointer[27], '.');
	EXPECT_EQ(SDcard.loadingDataPointer[28], '8');
	EXPECT_EQ(SDcard.loadingDataPointer[29], '9');
	EXPECT_EQ(SDcard.loadingDataPointer[30], '6');
	EXPECT_EQ(SDcard.loadingDataPointer[31], '\n');
	EXPECT_EQ(SDcard.loadingDataCounter, 32);
}

TEST(test_BT_SDcard, writeBlock_Test)
{
	uint32_t testblockaddr{ 0x1234 };
	volatile uint32_t testrxbuff[600];
	SDcard.FAT1Addr = 500;

	//trigger, no active read
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDWriteState = SDWRITE_START;
	EXPECT_EQ(writeBlock(testblockaddr, testrxbuff), SDWRITE_WAIT_RESPONSE);
	EXPECT_EQ(SDcard.spiActivitySDCard, ACTIVE);
	//trigger, active read
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = ACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDWriteState = SDWRITE_START;
	EXPECT_EQ(writeBlock(testblockaddr, testrxbuff), SDWRITE_WAIT_RESPONSE);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);

	//wait response, finished, failed
	SDcard.SDWriteState = SDWRITE_WAIT_RESPONSE;
	SDcard.SdRx[7] = 1;
	SDWriteWaitResponse();
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_FAILED);
	//wait response, finished, ok
	SDcard.SDWriteState = SDWRITE_WAIT_RESPONSE;
	SDcard.SdRx[7] = 0;
    sysTimer.sysTime = 1.234;
	SDWriteWaitResponse();
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_DATA);
	EXPECT_EQ(SDcard.SdCtr, 516);
	EXPECT_NEAR(SDcard.writeStartTime, 1.234, 0.01);

    //wait data write, send 1 byte
    SDcard.spiActivitySDCard = ACTIVE;
    SDcard.SDWriteState = SDWRITE_WAIT_DATA;
    SDcard.SdRx[515] = 0;
    SDcard.SdCtr = 516;
    SDcard.writeStartTime = 1.500;
    sysTimer.sysTime = 1.543;
    SDWriteWaitData();
    EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_DATA);
    EXPECT_EQ(SDcard.SdCtr, 1);
	//wait data write, failed
	SDcard.spiActivitySDCard = ACTIVE;
	SDcard.SDWriteState = SDWRITE_WAIT_DATA;
	SDcard.SdRx[515] = 0;
    SDcard.SdCtr = 516;
    SDcard.writeStartTime = 1.500;
    sysTimer.sysTime = 1.801;
    SDWriteWaitData();
    EXPECT_EQ(SDcard.SDWriteState, SDWRITE_FAILED);
	//wait data write, ok
	SDcard.spiActivitySDCard = ACTIVE;
	SDcard.SDWriteState = SDWRITE_WAIT_DATA;
	SDcard.SdRx[0] = 0b0100;
    SDcard.SdCtr = 1;
    SDWriteWaitData();
    EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_WRITE_FINISH);

	//wait write finish, pending
	SDcard.spiActivitySDCard = PENDING;
	SDcard.SDWriteState = SDWRITE_WAIT_WRITE_FINISH;
	EXPECT_EQ(writeBlock(testblockaddr, testrxbuff), SDWRITE_WAIT_WRITE_FINISH);
	//wait write finish, inactive, wait more
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDWriteState = SDWRITE_WAIT_WRITE_FINISH;
	SDcard.SdRx[0] = 0;
	SDcard.writeStartTime = 1.500;
	sysTimer.sysTime = 1.510; 
	EXPECT_EQ(writeBlock(testblockaddr, testrxbuff), SDWRITE_WAIT_WRITE_FINISH);
	EXPECT_EQ(SDcard.spiActivitySDCard, PENDING);
	//wait write finish, inactive, timeed out
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDWriteState = SDWRITE_WAIT_WRITE_FINISH;
	SDcard.SdRx[0] = 0;
	SDcard.writeStartTime = 1.500;
	sysTimer.sysTime = 1.801;
	EXPECT_EQ(writeBlock(testblockaddr, testrxbuff), SDWRITE_FAILED);
	EXPECT_EQ(writeBlock(testblockaddr, testrxbuff), SDWRITE_FAILED);
	//wait write finish, inactive, timeed out
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SDWriteState = SDWRITE_WAIT_WRITE_FINISH;
	SDcard.SdRx[0] = 1;
	EXPECT_EQ(writeBlock(testblockaddr, testrxbuff), SDWRITE_FINISHED);
	EXPECT_EQ(writeBlock(testblockaddr, testrxbuff), SDWRITE_FINISHED);
}

TEST(test_BT_SDcard, setFileTime_Test)
{
	volatile uint32_t testBuffer[516];
	float testTime{0};

	// time delay is between 0-60 sec
	testTime = 30;
	SDcard.globalTime.sec = 0;
	SDcard.globalTime.min = 0;
	SDcard.globalTime.hour = 0;
	SDcard.globalTime.day = 0;
	SDcard.globalTime.month = 0;
	SDcard.globalTime.year = 0;
	SDcard.rootDirEmptySlotNumber = 0;
	setFileTime(testBuffer, testTime);
	EXPECT_EQ(testBuffer[14], 0b1111);

	// time delay is between 0-59 min
	testTime = 15 + 60*7;
	SDcard.globalTime.sec = 0;
	SDcard.globalTime.min = 0;
	SDcard.globalTime.hour = 0;
	SDcard.globalTime.day = 0;
	SDcard.globalTime.month = 0;
	SDcard.globalTime.year = 0;
	SDcard.rootDirEmptySlotNumber = 0;
	setFileTime(testBuffer, testTime);
	EXPECT_EQ(testBuffer[14], 0b11100111);
	EXPECT_EQ(testBuffer[15], 0b0);

	// time delay is between 1-24 hour
	testTime = 45 + 60 * 9 + 3600 * 3;
	SDcard.globalTime.sec = 0;
	SDcard.globalTime.min = 0;
	SDcard.globalTime.hour = 0;
	SDcard.globalTime.day = 0;
	SDcard.globalTime.month = 0;
	SDcard.globalTime.year = 0;
	SDcard.rootDirEmptySlotNumber = 0;
	setFileTime(testBuffer, testTime);
	EXPECT_EQ(testBuffer[14], 0b00110110);
	EXPECT_EQ(testBuffer[15], 0b00011001);

	// time delay is between 1-24 hour
	testTime = 45 + 60 * 9 + 3600 * 3;
	SDcard.globalTime.sec = 0;
	SDcard.globalTime.min = 0;
	SDcard.globalTime.hour = 0;
	SDcard.globalTime.day = 0;
	SDcard.globalTime.month = 0;
	SDcard.globalTime.year = 0;
	SDcard.rootDirEmptySlotNumber = 0;
	SDcard.sysTimeAtGlobalTime = 40;
	setFileTime(testBuffer, testTime);
	EXPECT_EQ(testBuffer[14], 0b00100010);
	EXPECT_EQ(testBuffer[15], 0b00011001);

	// time with year
	testTime = 21 + 60 * 49 + 3600 * 15;
	SDcard.globalTime.sec = 0;
	SDcard.globalTime.min = 0;
	SDcard.globalTime.hour = 0;
	SDcard.globalTime.day = 11;
	SDcard.globalTime.month = 5;
	SDcard.globalTime.year = 25;
	SDcard.rootDirEmptySlotNumber = 0;
	SDcard.sysTimeAtGlobalTime = 0;
	setFileTime(testBuffer, testTime);
	EXPECT_EQ(testBuffer[14], 0b00101010);
	EXPECT_EQ(testBuffer[15], 0b01111110);
	EXPECT_EQ(testBuffer[16], 0b10101011);
	EXPECT_EQ(testBuffer[17], 0b01011010);

	//overflow sec
	testTime = 15;
	SDcard.globalTime.sec = 51;
	SDcard.globalTime.min = 0;
	SDcard.globalTime.hour = 0;
	SDcard.globalTime.day = 0;
	SDcard.globalTime.month = 0;
	SDcard.globalTime.year = 0;
	SDcard.rootDirEmptySlotNumber = 0;
	setFileTime(testBuffer, testTime);
	EXPECT_EQ(testBuffer[14], 0b00100011);
	EXPECT_EQ(testBuffer[15], 0b00000000);
	EXPECT_EQ(testBuffer[16], 0b00000000);
	EXPECT_EQ(testBuffer[17], 0b00101000);

	//overflow min
	testTime = 15 + 60 * 8;
	SDcard.globalTime.sec = 51;
	SDcard.globalTime.min = 54;
	SDcard.globalTime.hour = 0;
	SDcard.globalTime.day = 0;
	SDcard.globalTime.month = 0;
	SDcard.globalTime.year = 0;
	SDcard.rootDirEmptySlotNumber = 0;
	setFileTime(testBuffer, testTime);
	EXPECT_EQ(testBuffer[14], 0b01100011);
	EXPECT_EQ(testBuffer[15], 0b00001000);
	EXPECT_EQ(testBuffer[16], 0b00000000);
	EXPECT_EQ(testBuffer[17], 0b00101000);
}

TEST(test_BT_SDcard, addFileInfo2RootDir_Test)
{
	volatile uint32_t testBuffer[516];
	fileInfo testFile;
	float testTime{30};

	//
	SDcard.rootDirEmptySlotNumber = 0;
	SDcard.rootDirEmptyBlockNumber = 0;
	testFile.name[0] = 'M';
	testFile.name[1] = 'E';
	testFile.name[2] = 'A';
	testFile.name[3] = 'S';
	testFile.numberInName = 1;
	testFile.clusters[0] = 6;
	testFile.numberOfClusters = 1;
	testFile.size = 10;
	SDcard.globalTime.sec = 30;
	SDcard.globalTime.min = 59;
	SDcard.globalTime.hour = 11;
	SDcard.globalTime.day = 15;
	SDcard.globalTime.month = 5;
	SDcard.globalTime.year = 21;
	addFileInfo2RootDir(testBuffer, &testFile, testTime);
	EXPECT_EQ(testBuffer[0], 'M'); 
	EXPECT_EQ(testBuffer[1], 'E');
	EXPECT_EQ(testBuffer[2], 'A');
	EXPECT_EQ(testBuffer[3], 'S');
	EXPECT_EQ(testBuffer[4], '1');
	EXPECT_EQ(testBuffer[5], ' ');
	EXPECT_EQ(testBuffer[6], ' ');
	EXPECT_EQ(testBuffer[7], ' ');
	EXPECT_EQ(testBuffer[8], 'T');
	EXPECT_EQ(testBuffer[9], 'X');
	EXPECT_EQ(testBuffer[10], 'T');
	EXPECT_EQ(testBuffer[13], 0x4E);
	EXPECT_EQ(testBuffer[14], 0x00);
	EXPECT_EQ(testBuffer[15], 0x60);
	EXPECT_EQ(testBuffer[16], 0xAF);
	EXPECT_EQ(testBuffer[17], 0x52);
	EXPECT_EQ(testBuffer[18], 0xAF);
	EXPECT_EQ(testBuffer[19], 0x52);
	EXPECT_EQ(testBuffer[22], 0x00);
	EXPECT_EQ(testBuffer[23], 0x60);
	EXPECT_EQ(testBuffer[24], 0xAF);
	EXPECT_EQ(testBuffer[25], 0x52);
	EXPECT_EQ(testBuffer[20], 0x00);
	EXPECT_EQ(testBuffer[21], 0x00);
	EXPECT_EQ(testBuffer[26], 0x06);
	EXPECT_EQ(testBuffer[27], 0x00);
	EXPECT_EQ(testBuffer[28], 0x0A);
	EXPECT_EQ(testBuffer[29], 0x00);
	EXPECT_EQ(testBuffer[30], 0x00);
	EXPECT_EQ(testBuffer[31], 0x00);
	//2
	SDcard.rootDirEmptyBlockNumber = 0;
	SDcard.rootDirEmptySlotNumber = 0;
	testFile.name[0] = 'M';
	testFile.name[1] = 'E';
	testFile.name[2] = 'A';
	testFile.name[3] = 'S';
	testFile.numberInName = 15;
	testFile.clusters[0] = 6;
	testFile.clusters[1] = 7;
	testFile.numberOfClusters = 2;
	testFile.size = 30;
	testBuffer[0] = 0xFE;
	addFileInfo2RootDir(&testBuffer[1], &testFile, testTime);
	EXPECT_EQ(testBuffer[0], 0xFE);
	EXPECT_EQ(testBuffer[1], 'M');
	EXPECT_EQ(testBuffer[2], 'E');
	EXPECT_EQ(testBuffer[3], 'A');
	EXPECT_EQ(testBuffer[4], 'S');
	EXPECT_EQ(testBuffer[5], '1');
	EXPECT_EQ(testBuffer[6], '5');
	EXPECT_EQ(testBuffer[7], ' ');
	EXPECT_EQ(testBuffer[8], ' ');
	EXPECT_EQ(testBuffer[21], 0x00);
	EXPECT_EQ(testBuffer[22], 0x00);
	EXPECT_EQ(testBuffer[27], 0x06);
	EXPECT_EQ(testBuffer[28], 0x00);
	EXPECT_EQ(testBuffer[29], 0x1E);
	EXPECT_EQ(testBuffer[30], 0x00);
	EXPECT_EQ(testBuffer[31], 0x00);
	EXPECT_EQ(testBuffer[32], 0x00);
	//3
	SDcard.rootDirEmptyBlockNumber = 0;
	SDcard.rootDirEmptySlotNumber = 0;
	testFile.name[0] = 'M';
	testFile.name[1] = 'E';
	testFile.name[2] = 'A';
	testFile.name[3] = 'S';
	testFile.numberInName = 157;
	testFile.clusters[0] = 600;
	testFile.size = 300;
	addFileInfo2RootDir(testBuffer, &testFile, testTime);
	EXPECT_EQ(testBuffer[0], 'M');
	EXPECT_EQ(testBuffer[1], 'E');
	EXPECT_EQ(testBuffer[2], 'A');
	EXPECT_EQ(testBuffer[3], 'S');
	EXPECT_EQ(testBuffer[4], '1');
	EXPECT_EQ(testBuffer[5], '5');
	EXPECT_EQ(testBuffer[6], '7');
	EXPECT_EQ(testBuffer[7], ' ');
	EXPECT_EQ(testBuffer[20], 0x00);
	EXPECT_EQ(testBuffer[21], 0x00);
	EXPECT_EQ(testBuffer[26], 0x58);
	EXPECT_EQ(testBuffer[27], 0x02);
	EXPECT_EQ(testBuffer[28], 0x2C);
	EXPECT_EQ(testBuffer[29], 0x01);
	EXPECT_EQ(testBuffer[30], 0x00);
	EXPECT_EQ(testBuffer[31], 0x00);
	//4
	SDcard.rootDirEmptyBlockNumber = 0;
	SDcard.rootDirEmptySlotNumber = 1;
	testFile.name[0] = 'M';
	testFile.name[1] = 'E';
	testFile.name[2] = 'A';
	testFile.name[3] = 'S';
	testFile.numberInName = 1;
	testFile.clusters[0] = 0x02;
	testFile.size = 0x03;
	addFileInfo2RootDir(testBuffer, &testFile, testTime);
	EXPECT_EQ(testBuffer[32 + 0], 'M');
	EXPECT_EQ(testBuffer[32 + 1], 'E');
	EXPECT_EQ(testBuffer[32 + 2], 'A');
	EXPECT_EQ(testBuffer[32 + 3], 'S');
	EXPECT_EQ(testBuffer[32 + 4], '1');
	EXPECT_EQ(testBuffer[32 + 5], ' ');
	EXPECT_EQ(testBuffer[32 + 6], ' ');
	EXPECT_EQ(testBuffer[32 + 7], ' ');
	EXPECT_EQ(testBuffer[32 + 20], 0x00);
	EXPECT_EQ(testBuffer[32 + 21], 0x00);
	EXPECT_EQ(testBuffer[32 + 26], 0x02);
	EXPECT_EQ(testBuffer[32 + 27], 0x00);
	EXPECT_EQ(testBuffer[32 + 28], 0x03);
	EXPECT_EQ(testBuffer[32 + 29], 0x00);
	EXPECT_EQ(testBuffer[32 + 30], 0x00);
	EXPECT_EQ(testBuffer[32 + 31], 0x00);
	//5
	SDcard.rootDirEmptyBlockNumber = 0;
	SDcard.rootDirEmptySlotNumber = 15;
	testFile.name[0] = 'M';
	testFile.name[1] = 'E';
	testFile.name[2] = 'A';
	testFile.name[3] = 'S';
	testFile.numberInName = 1;
	testFile.clusters[0] = 0x02;
	testFile.size = 0x03;
	addFileInfo2RootDir(testBuffer, &testFile, testTime);
	EXPECT_EQ(testBuffer[15 * 32 + 0], 'M');
	EXPECT_EQ(testBuffer[15 * 32 + 1], 'E');
	EXPECT_EQ(testBuffer[15 * 32 + 2], 'A');
	EXPECT_EQ(testBuffer[15 * 32 + 3], 'S');
	EXPECT_EQ(testBuffer[15 * 32 + 4], '1');
	EXPECT_EQ(testBuffer[15 * 32 + 5], ' ');
	EXPECT_EQ(testBuffer[15 * 32 + 6], ' ');
	EXPECT_EQ(testBuffer[15 * 32 + 7], ' ');
	EXPECT_EQ(testBuffer[15 * 32 + 20], 0x00);
	EXPECT_EQ(testBuffer[15 * 32 + 21], 0x00);
	EXPECT_EQ(testBuffer[15 * 32 + 26], 0x02);
	EXPECT_EQ(testBuffer[15 * 32 + 27], 0x00);
	EXPECT_EQ(testBuffer[15 * 32 + 28], 0x03);
	EXPECT_EQ(testBuffer[15 * 32 + 29], 0x00);
	EXPECT_EQ(testBuffer[15 * 32 + 30], 0x00);
	EXPECT_EQ(testBuffer[15 * 32 + 31], 0x00);
	//6
	testFile.name[0] = 'M';
	testFile.name[1] = 'E';
	testFile.name[2] = 'A';
	testFile.name[3] = 'S';
	testFile.numberInName = 4;
	testFile.clusters[0] = 0x05;
	testFile.size = 0x06;
	addFileInfo2RootDir(testBuffer, &testFile, testTime);
	EXPECT_EQ(testBuffer[0 * 32 + 0], 'M');
	EXPECT_EQ(testBuffer[0 * 32 + 1], 'E');
	EXPECT_EQ(testBuffer[0 * 32 + 2], 'A');
	EXPECT_EQ(testBuffer[0 * 32 + 3], 'S');
	EXPECT_EQ(testBuffer[0 * 32 + 4], '4');
	EXPECT_EQ(testBuffer[0 * 32 + 5], ' ');
	EXPECT_EQ(testBuffer[0 * 32 + 6], ' ');
	EXPECT_EQ(testBuffer[0 * 32 + 7], ' ');
	EXPECT_EQ(testBuffer[0 * 32 + 20], 0x00);
	EXPECT_EQ(testBuffer[0 * 32 + 21], 0x00);
	EXPECT_EQ(testBuffer[0 * 32 + 26], 0x05);
	EXPECT_EQ(testBuffer[0 * 32 + 27], 0x00);
	EXPECT_EQ(testBuffer[0 * 32 + 28], 0x06);
	EXPECT_EQ(testBuffer[0 * 32 + 29], 0x00);
	EXPECT_EQ(testBuffer[0 * 32 + 30], 0x00);
	EXPECT_EQ(testBuffer[0 * 32 + 31], 0x00);
	EXPECT_EQ(SDcard.rootDirEmptyBlockNumber, 1);
	EXPECT_EQ(SDcard.rootDirEmptySlotNumber, 1);
	EXPECT_EQ(testBuffer[1 * 32 + 0], 0);
	EXPECT_EQ(testBuffer[1 * 32 + 1], 0);
	EXPECT_EQ(testBuffer[1 * 32 + 2], 0);
	EXPECT_EQ(testBuffer[1 * 32 + 3], 0);
	EXPECT_EQ(testBuffer[1 * 32 + 4], 0);
	EXPECT_EQ(testBuffer[1 * 32 + 5], 0);
	EXPECT_EQ(testBuffer[1 * 32 + 6], 0);
	EXPECT_EQ(testBuffer[1 * 32 + 7], 0);
}

TEST(test_BT_SDcard, addFileFATInfo_Test)
{
	uint32_t testBufferRaw[513] = { 0xFE };
	uint32_t* testBuffer = &testBufferRaw[1];
	fileInfo testFile;
	uint16_t pos = 0;
	uint64_t offset = 0;
	SDcard.FATBlockOffset = 0;
	// 0-3 slots
	testFile.name[0] = 'M';
	testFile.name[1] = 'E';
	testFile.name[2] = 'A';
	testFile.name[3] = 'S';
	testFile.numberInName = 1;
	testFile.numberOfClusters = 4;
	testFile.clusters[0] = 0;	//to rootDir
	testFile.clusters[1] = 1;
	testFile.clusters[2] = 2;
	testFile.clusters[3] = 3;
	EXPECT_EQ(addFileFATInfo(&testBufferRaw[1], &testFile, E_SDFATWRITE_FIRST_CALL), false);
	pos = 0; EXPECT_EQ(testBuffer[4 * pos + 0], 0x01); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 1; EXPECT_EQ(testBuffer[4 * pos + 0], 0x02); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 2; EXPECT_EQ(testBuffer[4 * pos + 0], 0x03); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 3; EXPECT_EQ(testBuffer[4 * pos + 0], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 1], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0x0F);
	// 4 slot
	testFile.numberOfClusters = 1;
	testFile.clusters[0] = 4;	//to rootDir
	EXPECT_EQ(addFileFATInfo(&testBufferRaw[1], &testFile, E_SDFATWRITE_FIRST_CALL), false);
	pos = 4; EXPECT_EQ(testBuffer[4 * pos + 0], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 1], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0x0F);
	// additional 5-14 slots
	testFile.numberOfClusters = 10;
	testFile.clusters[0] = 5;	//to rootDir
	testFile.clusters[1] = 6;
	testFile.clusters[2] = 7;
	testFile.clusters[3] = 8;
	testFile.clusters[4] = 9;
	testFile.clusters[5] = 10;
	testFile.clusters[6] = 11;
	testFile.clusters[7] = 12;
	testFile.clusters[8] = 13;
	testFile.clusters[9] = 14;
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_FIRST_CALL), false);
	pos = 5;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x06); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 6;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x07); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 7;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x08); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 8;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x09); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 9;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x0A); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 10; EXPECT_EQ(testBuffer[4 * pos + 0], 0x0B); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 11; EXPECT_EQ(testBuffer[4 * pos + 0], 0x0C); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 12; EXPECT_EQ(testBuffer[4 * pos + 0], 0x0D); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 13; EXPECT_EQ(testBuffer[4 * pos + 0], 0x0E); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 14; EXPECT_EQ(testBuffer[4 * pos + 0], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 1], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0x0F);
	// high value clusters
	testFile.numberOfClusters = 3;
	testFile.clusters[0] = 0x11223344;	//to rootDir
	testFile.clusters[1] = 0x11223345;
	testFile.clusters[2] = 0x11223346;
	testBuffer[268] = 0x00;
	testBuffer[269] = 0x00;
	testBuffer[270] = 0x00;
	testBuffer[271] = 0x00;
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_FIRST_CALL), false);
	testBuffer[268] = 0xFF;
	testBuffer[269] = 0xFF;
	testBuffer[270] = 0xFF;
	testBuffer[271] = 0x0F;
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_FIRST_CALL), false);
	pos = 68; EXPECT_EQ(testBuffer[4 * pos + 0], 0x45); EXPECT_EQ(testBuffer[4 * pos + 1], 0x33); EXPECT_EQ(testBuffer[4 * pos + 2], 0x22); EXPECT_EQ(testBuffer[4 * pos + 3], 0x11);
	pos = 69; EXPECT_EQ(testBuffer[4 * pos + 0], 0x46); EXPECT_EQ(testBuffer[4 * pos + 1], 0x33); EXPECT_EQ(testBuffer[4 * pos + 2], 0x22); EXPECT_EQ(testBuffer[4 * pos + 3], 0x11);
	pos = 70; EXPECT_EQ(testBuffer[4 * pos + 0], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 1], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0x0F);
	// start from 2nd slot, end in 2nd(number 1) block
	offset = 1;
	testBuffer[0] = 0xFF;
	testBuffer[1] = 0xFF;
	testBuffer[2] = 0xFF;
	testBuffer[3] = 0x0F;
	testFile.numberOfClusters = 135;
	for (uint16_t i = 0; i < testFile.numberOfClusters; i++)
	{
		testFile.clusters[i] = i + offset;
	}
	SDcard.FATBlockOffset = 0;
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_FIRST_CALL), true);
	pos = 1; EXPECT_EQ(testBuffer[4 * pos + 0], 0x02); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 2; EXPECT_EQ(testBuffer[4 * pos + 0], 0x03); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 127; EXPECT_EQ(testBuffer[4 * pos + 0], 0x80); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_CONSECUTIVE_CALL), false);
	pos = 0; EXPECT_EQ(testBuffer[4 * pos + 0], 0x81); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 1; EXPECT_EQ(testBuffer[4 * pos + 0], 0x82); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 2; EXPECT_EQ(testBuffer[4 * pos + 0], 0x83); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 3; EXPECT_EQ(testBuffer[4 * pos + 0], 0x84); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 4; EXPECT_EQ(testBuffer[4 * pos + 0], 0x85); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 5; EXPECT_EQ(testBuffer[4 * pos + 0], 0x86); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 6; EXPECT_EQ(testBuffer[4 * pos + 0], 0x87); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 7; EXPECT_EQ(testBuffer[4 * pos + 0], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 1], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0x0F);
	EXPECT_EQ(SDcard.FATBlockOffset, 1);
	// add extra 150 slots, end in 3rd(number 2) block
	for (uint16_t i = 0; i < 512; i++)
	{
		testBuffer[i] = 0;
	}
	offset = 10;
	testBuffer[4 * (offset - 1) + 0] = 0xFF;
	testBuffer[4 * (offset - 1) + 1] = 0xFF;
	testBuffer[4 * (offset - 1) + 2] = 0xFF;
	testBuffer[4 * (offset - 1) + 3] = 0x0F;
	testFile.numberOfClusters = 150;
	for (uint16_t i = 0; i < testFile.numberOfClusters; i++)
	{
		testFile.clusters[i] = i + offset;
	}
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_FIRST_CALL), true);
	pos = 10;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x0B); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 11;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x0C); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 127; EXPECT_EQ(testBuffer[4 * pos + 0], 0x80); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_CONSECUTIVE_CALL), false);
	pos = 0;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x81); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 1;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x82); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 30; EXPECT_EQ(testBuffer[4 * pos + 0], 0x9F); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 31; EXPECT_EQ(testBuffer[4 * pos + 0], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 1], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0x0F);
	EXPECT_EQ(SDcard.FATBlockOffset, 2);
	// add 2+ block amount of slots, end in 5th(number 4) block
	offset = 75;
	testBuffer[4 * (offset - 1) + 0] = 0xFF;
	testBuffer[4 * (offset - 1) + 1] = 0xFF;
	testBuffer[4 * (offset - 1) + 2] = 0xFF;
	testBuffer[4 * (offset - 1) + 3] = 0x0F;
	testFile.numberOfClusters = 250;
	for (uint16_t i = 0; i < testFile.numberOfClusters; i++)
	{
		testFile.clusters[i] = i + offset;
	}
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_FIRST_CALL), true);
	pos = 75;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x4C); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 76;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x4D); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 127; EXPECT_EQ(testBuffer[4 * pos + 0], 0x80); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_CONSECUTIVE_CALL), true);
	pos = 0;   EXPECT_EQ(testBuffer[4 * pos + 0], 0x81); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 1;   EXPECT_EQ(testBuffer[4 * pos + 0], 0x82); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 127; EXPECT_EQ(testBuffer[4 * pos + 0], 0x00); EXPECT_EQ(testBuffer[4 * pos + 1], 0x01); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	EXPECT_EQ(SDcard.FATBlockOffset, 3);
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_CONSECUTIVE_CALL), false);
	pos = 0;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x01); EXPECT_EQ(testBuffer[4 * pos + 1], 0x01); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 1;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x02); EXPECT_EQ(testBuffer[4 * pos + 1], 0x01); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 67; EXPECT_EQ(testBuffer[4 * pos + 0], 0x44); EXPECT_EQ(testBuffer[4 * pos + 1], 0x01); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 68; EXPECT_EQ(testBuffer[4 * pos + 0], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 1], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0x0F);
	EXPECT_EQ(SDcard.FATBlockOffset, 4);
	// start at last block, add 2+ blocks
	offset = 1091071;  //128 * 8524 - 1;
	testBuffer[4 * (offset % 128 - 1) + 0] = 0xFF;
	testBuffer[4 * (offset % 128 - 1) + 1] = 0xFF;
	testBuffer[4 * (offset % 128 - 1) + 2] = 0xFF;
	testBuffer[4 * (offset % 128 - 1) + 3] = 0x0F;
	testFile.numberOfClusters = 300;
	for (uint16_t i = 0; i < testFile.numberOfClusters; i++)
	{
		testFile.clusters[i] = i + offset;
	}
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_FIRST_CALL), true);
	pos = 127; EXPECT_EQ(testBuffer[4 * pos + 0], 0x00); EXPECT_EQ(testBuffer[4 * pos + 1], 0xA6); EXPECT_EQ(testBuffer[4 * pos + 2], 0x10); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_CONSECUTIVE_CALL), true);
	pos = 0;   EXPECT_EQ(testBuffer[4 * pos + 0], 0x01); EXPECT_EQ(testBuffer[4 * pos + 1], 0xA6); EXPECT_EQ(testBuffer[4 * pos + 2], 0x10); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 1;   EXPECT_EQ(testBuffer[4 * pos + 0], 0x02); EXPECT_EQ(testBuffer[4 * pos + 1], 0xA6); EXPECT_EQ(testBuffer[4 * pos + 2], 0x10); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 127; EXPECT_EQ(testBuffer[4 * pos + 0], 0x80); EXPECT_EQ(testBuffer[4 * pos + 1], 0xA6); EXPECT_EQ(testBuffer[4 * pos + 2], 0x10); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_CONSECUTIVE_CALL), true);
	pos = 0;   EXPECT_EQ(testBuffer[4 * pos + 0], 0x81); EXPECT_EQ(testBuffer[4 * pos + 1], 0xA6); EXPECT_EQ(testBuffer[4 * pos + 2], 0x10); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 1;   EXPECT_EQ(testBuffer[4 * pos + 0], 0x82); EXPECT_EQ(testBuffer[4 * pos + 1], 0xA6); EXPECT_EQ(testBuffer[4 * pos + 2], 0x10); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 127; EXPECT_EQ(testBuffer[4 * pos + 0], 0x00); EXPECT_EQ(testBuffer[4 * pos + 1], 0xA7); EXPECT_EQ(testBuffer[4 * pos + 2], 0x10); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_CONSECUTIVE_CALL), false);
	pos = 0;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x01); EXPECT_EQ(testBuffer[4 * pos + 1], 0xA7); EXPECT_EQ(testBuffer[4 * pos + 2], 0x10); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 1;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x02); EXPECT_EQ(testBuffer[4 * pos + 1], 0xA7); EXPECT_EQ(testBuffer[4 * pos + 2], 0x10); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 41; EXPECT_EQ(testBuffer[4 * pos + 0], 0x2A); EXPECT_EQ(testBuffer[4 * pos + 1], 0xA7); EXPECT_EQ(testBuffer[4 * pos + 2], 0x10); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 42; EXPECT_EQ(testBuffer[4 * pos + 0], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 1], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0x0F);
	// add 2+ block of slots
	offset = 2868838416;  //0xAAFF0010;
	testBuffer[4 * (offset % 128 - 1) + 0] = 0xFF;
	testBuffer[4 * (offset % 128 - 1) + 1] = 0xFF;
	testBuffer[4 * (offset % 128 - 1) + 2] = 0xFF;
	testBuffer[4 * (offset % 128 - 1) + 3] = 0x0F;
	testFile.numberOfClusters = 273;
	for (uint16_t i = 0; i < testFile.numberOfClusters; i++)
	{
		testFile.clusters[i] = i + offset;
	}
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_FIRST_CALL), true);
	pos = 16;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x11); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0xAA);
	pos = 17;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x12); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0xAA);
	pos = 127; EXPECT_EQ(testBuffer[4 * pos + 0], 0x80); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0xAA);
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_CONSECUTIVE_CALL), true);
	pos = 0;   EXPECT_EQ(testBuffer[4 * pos + 0], 0x81); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0xAA);
	pos = 1;   EXPECT_EQ(testBuffer[4 * pos + 0], 0x82); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0xAA);
	pos = 127; EXPECT_EQ(testBuffer[4 * pos + 0], 0x00); EXPECT_EQ(testBuffer[4 * pos + 1], 0x01); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0xAA);
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_CONSECUTIVE_CALL), false);
	pos = 0;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x01); EXPECT_EQ(testBuffer[4 * pos + 1], 0x01); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0xAA);
	pos = 1;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x02); EXPECT_EQ(testBuffer[4 * pos + 1], 0x01); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0xAA);
	pos = 31; EXPECT_EQ(testBuffer[4 * pos + 0], 0x20); EXPECT_EQ(testBuffer[4 * pos + 1], 0x01); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0xAA);
	pos = 32; EXPECT_EQ(testBuffer[4 * pos + 0], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 1], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0x0F);
	// last slot in next block
	pos = 112;
	testBuffer[4 * pos + 0] = 0xFF;
	testBuffer[4 * pos + 1] = 0xFF;
	testBuffer[4 * pos + 2] = 0xFF;
	testBuffer[4 * pos + 3] = 0x0F;
	testFile.numberOfClusters = 16;
	testFile.clusters[0] = 113;	//to rootDir
	testFile.clusters[1] = 114;
	testFile.clusters[2] = 115;
	testFile.clusters[3] = 116;
	testFile.clusters[4] = 117;
	testFile.clusters[5] = 118;
	testFile.clusters[6] = 119;
	testFile.clusters[7] = 120;
	testFile.clusters[8] = 121;
	testFile.clusters[9] = 122;
	testFile.clusters[10] = 123;
	testFile.clusters[11] = 124;
	testFile.clusters[12] = 125;
	testFile.clusters[13] = 126;
	testFile.clusters[14] = 127;
	testFile.clusters[15] = 128;	//in new cluster
	SDcard.FATBlockOffset = 0;
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_FIRST_CALL), true);
	pos = 113;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x72); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 114;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x73); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 115;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x74); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 116;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x75); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 117;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x76); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 118;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x77); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 119;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x78); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 120;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x79); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 121;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x7A); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 122;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x7B); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 123;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x7C); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 124;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x7D); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 125;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x7E); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 126;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x7F); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 127;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x80); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_CONSECUTIVE_CALL), false);
	pos = 0;  EXPECT_EQ(testBuffer[4 * pos + 0], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 1], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0x0F);
	EXPECT_EQ(SDcard.FATBlockOffset, 1);
	// 1st slot is random value end slot, overwrite it
	pos = 0;
	testBuffer[4 * pos + 0] = 0x0A;	//random values
	testBuffer[4 * pos + 1] = 0x0B;
	testBuffer[4 * pos + 2] = 0x0C;
	testBuffer[4 * pos + 3] = 0x0D;
	testFile.numberOfClusters = 2;
	testFile.clusters[0] = 128;	//to rootDir
	testFile.clusters[1] = 129;
	SDcard.FATBlockOffset = 0;
	EXPECT_EQ(addFileFATInfo(testBuffer, &testFile, E_SDFATWRITE_FIRST_CALL), false);
	pos = 0;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x81); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
	pos = 1;  EXPECT_EQ(testBuffer[4 * pos + 0], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 1], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 2], 0xFF); EXPECT_EQ(testBuffer[4 * pos + 3], 0x0F);
	pos = 2;  EXPECT_EQ(testBuffer[4 * pos + 0], 0x00); EXPECT_EQ(testBuffer[4 * pos + 1], 0x00); EXPECT_EQ(testBuffer[4 * pos + 2], 0x00); EXPECT_EQ(testBuffer[4 * pos + 3], 0x00);
}

TEST(test_BT_SDcard, writeData_Test)
{
	uint16_t testSwitch{ 0 };
	float testTime{ 0 };
	//wait
	SDcard.writeMeasData = true;
	SDcard.SDWriteState = SDWRITE_START;
	writeData(testSwitch, testTime);
	EXPECT_EQ(SDcard.writeMeasData, true);
	SDcard.SDWriteState = SDWRITE_WAIT_RESPONSE;
	writeData(testSwitch, testTime);
	EXPECT_EQ(SDcard.writeMeasData, true);
	SDcard.SDWriteState = SDWRITE_WAIT_DATA;
	writeData(testSwitch, testTime);
	EXPECT_EQ(SDcard.writeMeasData, true);
	SDcard.SDWriteState = SDWRITE_WAIT_WRITE_FINISH;
	writeData(testSwitch, testTime);
	EXPECT_EQ(SDcard.writeMeasData, true);

	//failed
	SDcard.SDWriteState = SDWRITE_FAILED;
	writeData(testSwitch, testTime);
	EXPECT_EQ(SDcard.writeMeasData, false);

	//finished
	SDcard.SDWriteState = SDWRITE_FINISHED;
	testSwitch = 2000;
	writeData(testSwitch, testTime);
	EXPECT_EQ(SDcard.writeMeasData, false);
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_START);
	//
	SDcard.SDWriteState = SDWRITE_FINISHED;
	SDcard.MainState = SD_MEASUREMENT_ONGOING;
	testSwitch = 1000;
	writeData(testSwitch, testTime);
	EXPECT_EQ(SDcard.writeMeasData, false);
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_START);
	EXPECT_EQ(SDcard.MainState, SD_WRITE_ROOT);

}

TEST(test_BT_SDcard, addMeasNameHeader_Test)
{
    //not measured
    SDcard.loadingDataCounter = 1;
    addMeasNameHeader(false, false, "test", 4);
    EXPECT_EQ(SDcard.loadingDataCounter, 1);
    //measured
    SDcard.loadingDataCounter = 1;
    addMeasNameHeader(true, false, "test", 4);
    EXPECT_EQ(SDcard.loadingDataCounter, 5);
    EXPECT_EQ(SDcard.loadingDataPointer[1], 't');
    EXPECT_EQ(SDcard.loadingDataPointer[2], 'e');
    EXPECT_EQ(SDcard.loadingDataPointer[3], 's');
    EXPECT_EQ(SDcard.loadingDataPointer[4], 't');
    //measured + comma
    addMeasNameHeader(true, true, "sysTime", 7);
    EXPECT_EQ(SDcard.loadingDataCounter, 13);
    EXPECT_EQ(SDcard.loadingDataPointer[5], ',');
    EXPECT_EQ(SDcard.loadingDataPointer[6], 's');
    EXPECT_EQ(SDcard.loadingDataPointer[7], 'y');
    EXPECT_EQ(SDcard.loadingDataPointer[8], 's');
    EXPECT_EQ(SDcard.loadingDataPointer[9], 'T');
    EXPECT_EQ(SDcard.loadingDataPointer[10], 'i');
    EXPECT_EQ(SDcard.loadingDataPointer[11], 'm');
    EXPECT_EQ(SDcard.loadingDataPointer[12], 'e');
    //
    addMeasNameHeader(true, true, "°RawR", 5);
    EXPECT_EQ(SDcard.loadingDataCounter, 19);
    EXPECT_EQ(SDcard.loadingDataPointer[13], ',');
    EXPECT_EQ(SDcard.loadingDataPointer[14], '°');
    EXPECT_EQ(SDcard.loadingDataPointer[15], 'R');
    EXPECT_EQ(SDcard.loadingDataPointer[16], 'a');
    EXPECT_EQ(SDcard.loadingDataPointer[17], 'w');
    EXPECT_EQ(SDcard.loadingDataPointer[18], 'R');
}

TEST(test_BT_SDcard, addMeasHeader_Test)
{
	pidRate.P.x = 150;
	pidRate.I.x = 40;
	pidRate.D.x = 1600;
	pidRate.FFr.x = 100;
	pidRate.DTermC = 12;
	meas2Card.measureSysTime = true;
	meas2Card.measureGyroRawX = true;
	meas2Card.measureGyroRawZ = true;
	addMeasHeader();
	//1st line
	EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], 'R');
	EXPECT_EQ(SDcard.loadingDataPointer[2], '\n');
	//2nd line
	EXPECT_EQ(SDcard.loadingDataPointer[3], 'P');
	EXPECT_EQ(SDcard.loadingDataPointer[4], 'x');
	EXPECT_EQ(SDcard.loadingDataPointer[5], ',');
	EXPECT_EQ(SDcard.loadingDataPointer[6], 'I');
	EXPECT_EQ(SDcard.loadingDataPointer[7], 'x');
	EXPECT_EQ(SDcard.loadingDataPointer[8], ',');
	EXPECT_EQ(SDcard.loadingDataPointer[9], 'D');
	EXPECT_EQ(SDcard.loadingDataPointer[10], 'x');
	EXPECT_EQ(SDcard.loadingDataPointer[11], ',');
	//...
	EXPECT_EQ(SDcard.loadingDataPointer[27], 'F');
	EXPECT_EQ(SDcard.loadingDataPointer[28], 'F');
	EXPECT_EQ(SDcard.loadingDataPointer[29], 'r');
	EXPECT_EQ(SDcard.loadingDataPointer[30], 'x');
	EXPECT_EQ(SDcard.loadingDataPointer[31], ',');
	EXPECT_EQ(SDcard.loadingDataPointer[32], 'F');
	EXPECT_EQ(SDcard.loadingDataPointer[33], 'F');
	EXPECT_EQ(SDcard.loadingDataPointer[34], 'r');
	EXPECT_EQ(SDcard.loadingDataPointer[35], 'y');
	EXPECT_EQ(SDcard.loadingDataPointer[36], ',');
	//...
	EXPECT_EQ(SDcard.loadingDataPointer[54], 's');
	EXPECT_EQ(SDcard.loadingDataPointer[55], 'a');
	EXPECT_EQ(SDcard.loadingDataPointer[56], 't');
	EXPECT_EQ(SDcard.loadingDataPointer[57], 'P');
	EXPECT_EQ(SDcard.loadingDataPointer[58], 'I');
	EXPECT_EQ(SDcard.loadingDataPointer[59], 'D');
	EXPECT_EQ(SDcard.loadingDataPointer[60], ',');
	//...
	EXPECT_EQ(SDcard.loadingDataPointer[76], 'A');
	EXPECT_EQ(SDcard.loadingDataPointer[77], 'p');
	EXPECT_EQ(SDcard.loadingDataPointer[78], 'a');
	EXPECT_EQ(SDcard.loadingDataPointer[79], 'r');
	EXPECT_EQ(SDcard.loadingDataPointer[80], 'a');
	EXPECT_EQ(SDcard.loadingDataPointer[81], 'm');
	EXPECT_EQ(SDcard.loadingDataPointer[82], 'C');
	EXPECT_EQ(SDcard.loadingDataPointer[83], ',');
	//...
	EXPECT_EQ(SDcard.loadingDataPointer[107], 'C');
	EXPECT_EQ(SDcard.loadingDataPointer[108], 'P');
	EXPECT_EQ(SDcard.loadingDataPointer[109], 'x');
	EXPECT_EQ(SDcard.loadingDataPointer[110], ',');
	//...
	EXPECT_EQ(SDcard.loadingDataPointer[144], 'C');
	EXPECT_EQ(SDcard.loadingDataPointer[145], 'F');
	EXPECT_EQ(SDcard.loadingDataPointer[146], 'F');
	EXPECT_EQ(SDcard.loadingDataPointer[147], 'd');
	EXPECT_EQ(SDcard.loadingDataPointer[148], 'r');
	EXPECT_EQ(SDcard.loadingDataPointer[149], 'y');
	//...
	EXPECT_EQ(SDcard.loadingDataPointer[150], '\n');
	//3rd line
	EXPECT_EQ(SDcard.loadingDataPointer[151], '1');
	EXPECT_EQ(SDcard.loadingDataPointer[152], '5');
	EXPECT_EQ(SDcard.loadingDataPointer[153], '0');
	EXPECT_EQ(SDcard.loadingDataPointer[154], ',');
	EXPECT_EQ(SDcard.loadingDataPointer[155], '4');
	EXPECT_EQ(SDcard.loadingDataPointer[156], '0');
	EXPECT_EQ(SDcard.loadingDataPointer[157], ',');
	EXPECT_EQ(SDcard.loadingDataPointer[158], '1');
	EXPECT_EQ(SDcard.loadingDataPointer[159], '6');
	EXPECT_EQ(SDcard.loadingDataPointer[160], '0');
	EXPECT_EQ(SDcard.loadingDataPointer[161], '0');
	EXPECT_EQ(SDcard.loadingDataPointer[162], ',');
	//...
	EXPECT_EQ(SDcard.loadingDataPointer[173], '1');
	EXPECT_EQ(SDcard.loadingDataPointer[174], '0');
	EXPECT_EQ(SDcard.loadingDataPointer[175], '0');
	EXPECT_EQ(SDcard.loadingDataPointer[176], ',');
	//...
	EXPECT_EQ(SDcard.loadingDataPointer[235], '0');
	EXPECT_EQ(SDcard.loadingDataPointer[236], '\n');
	//4th line
	EXPECT_EQ(SDcard.loadingDataPointer[237], 's');
	EXPECT_EQ(SDcard.loadingDataPointer[238], 'y');
	EXPECT_EQ(SDcard.loadingDataPointer[239], 's');
	EXPECT_EQ(SDcard.loadingDataPointer[240], 'T');
	EXPECT_EQ(SDcard.loadingDataPointer[241], 'i');
	EXPECT_EQ(SDcard.loadingDataPointer[242], 'm');
	EXPECT_EQ(SDcard.loadingDataPointer[243], 'e');
	EXPECT_EQ(SDcard.loadingDataPointer[244], ',');
	EXPECT_EQ(SDcard.loadingDataPointer[245], 'G');
	EXPECT_EQ(SDcard.loadingDataPointer[246], 'R');
	EXPECT_EQ(SDcard.loadingDataPointer[247], 'a');
	EXPECT_EQ(SDcard.loadingDataPointer[248], 'w');
	EXPECT_EQ(SDcard.loadingDataPointer[249], 'X');
	EXPECT_EQ(SDcard.loadingDataPointer[250], ',');
	//...
	EXPECT_EQ(SDcard.loadingDataPointer[257], 'G');
	EXPECT_EQ(SDcard.loadingDataPointer[258], 'R');
	EXPECT_EQ(SDcard.loadingDataPointer[259], 'a');
	EXPECT_EQ(SDcard.loadingDataPointer[260], 'w');
	EXPECT_EQ(SDcard.loadingDataPointer[261], 'Z'); 
	EXPECT_EQ(SDcard.loadingDataPointer[262], '\n');
}

TEST(test_BT_SDcard, writeRoot_Test)
{
	//wait
	SDcard.SDWriteState = SDWRITE_START;
	SDcard.MainState = SD_WRITE_ROOT;
	writeRoot();
	EXPECT_EQ(SDcard.MainState, SD_WRITE_ROOT);
	SDcard.SDWriteState = SDWRITE_WAIT_RESPONSE;
	SDcard.MainState = SD_WRITE_ROOT;
	writeRoot();
	EXPECT_EQ(SDcard.MainState, SD_WRITE_ROOT);
	SDcard.SDWriteState = SDWRITE_WAIT_DATA;
	SDcard.MainState = SD_WRITE_ROOT;
	writeRoot();
	EXPECT_EQ(SDcard.MainState, SD_WRITE_ROOT);
	SDcard.SDWriteState = SDWRITE_WAIT_WRITE_FINISH;
	SDcard.MainState = SD_WRITE_ROOT;
	writeRoot();
	EXPECT_EQ(SDcard.MainState, SD_WRITE_ROOT);

	//failed
	SDcard.SDWriteState = SDWRITE_FAILED;
	SDcard.MainState = SD_WRITE_ROOT;
	writeRoot();
	EXPECT_EQ(SDcard.MainState, SD_DO_NOTHING);

	//finished
	SDcard.SDWriteState = SDWRITE_FINISHED;
	SDcard.MainState = SD_WRITE_ROOT;
	writeRoot();
	EXPECT_EQ(SDcard.MainState, SD_WRITE_FAT);
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_START);
}

TEST(test_BT_SDcard, writeFAT_Test)
{
	//wait
	SDcard.SDWriteState = SDWRITE_START;
	SDcard.MainState = SD_WRITE_FAT;
	writeFAT();
	EXPECT_EQ(SDcard.MainState, SD_WRITE_FAT);
	SDcard.SDWriteState = SDWRITE_WAIT_RESPONSE;
	SDcard.MainState = SD_WRITE_FAT;
	writeFAT();
	EXPECT_EQ(SDcard.MainState, SD_WRITE_FAT);
	SDcard.SDWriteState = SDWRITE_WAIT_DATA;
	SDcard.MainState = SD_WRITE_FAT;
	writeFAT();
	EXPECT_EQ(SDcard.MainState, SD_WRITE_FAT);
	SDcard.SDWriteState = SDWRITE_WAIT_WRITE_FINISH;
	SDcard.MainState = SD_WRITE_FAT;
	writeFAT();
	EXPECT_EQ(SDcard.MainState, SD_WRITE_FAT);

	//failed
	SDcard.SDWriteState = SDWRITE_FAILED;
	SDcard.MainState = SD_WRITE_FAT;
	writeFAT();
	EXPECT_EQ(SDcard.MainState, SD_DO_NOTHING);

	//finished
	//new block write trigger
	SDcard.SDWriteState = SDWRITE_FINISHED;
	SDcard.MainState = SD_WRITE_FAT;
	SDcard.writingMultiFATBlock = true;
	writeFAT();
	EXPECT_EQ(SDcard.MainState, SD_WRITE_FAT);
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_START);
	//no new block
	SDcard.SDWriteState = SDWRITE_FINISHED;
	SDcard.MainState = SD_WRITE_FAT;
	SDcard.writingMultiFATBlock = false;
	SDcard.newFile.numberInName = 3;
	SDcard.newFile.numberOfClusters = 2;
	SDcard.newFile.clusters[1] = 5;
	SDcard.newFile.blockCount = 1;
	SDcard.newFile.size = 235;
	SDcard.loadingDataCounter = 3;
	writeFAT();
	EXPECT_EQ(SDcard.MainState, SD_WAIT_4_MEASUREMENT);
	EXPECT_EQ(SDcard.newFile.numberInName, 4);
	EXPECT_EQ(SDcard.newFile.numberOfClusters, 1);
	EXPECT_EQ(SDcard.newFile.clusters[0], 6);
	EXPECT_EQ(SDcard.newFile.blockCount, 0);
	EXPECT_EQ(SDcard.newFile.size, 0);
	EXPECT_EQ(SDcard.loadingDataCounter, 1);
	EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], 0xFF);
	EXPECT_EQ(SDcard.loadingDataPointer[513], 0xFF);
	EXPECT_EQ(SDcard.loadingDataPointer[516], 0xFF);
	EXPECT_EQ(SDcard.sendingDataPointer[0] & 0xFF, 0xFE);
	EXPECT_EQ(SDcard.sendingDataPointer[1], 0xFF);
	EXPECT_EQ(SDcard.sendingDataPointer[513], 0xFF);
	EXPECT_EQ(SDcard.sendingDataPointer[516], 0xFF);
}

TEST(test_BT_SDcard, RunSdCard_Test)
{
	SpiInput testIn;
	SPIOutput testOut;
	//setup
	SDcard.MainState = SD_INIT;
	SDcard.SDInitStatus = SDINIT_CMD0;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_INIT);
	SDcard.MainState = SD_INIT;
	SDcard.SDInitStatus = SDINIT_FAILURE;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_DO_NOTHING);
	SDcard.MainState = SD_INIT;
	SDcard.SDInitStatus = SDINIT_SUCCESS;
	SDcard.SDCommandState = SDCOMMAND_SEND;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_WAIT_4_MEASUREMENT);
	//wait 4 meas
	SDcard.loadingDataCounter = 1;
	SDcard.loadingDataPointer[1] = ' ';
	SDcard.loadingDataPointer[2] = ' ';
	testIn.rcSignals.measurementSwitch = 1000;
	SDcard.MainState = SD_WAIT_4_MEASUREMENT;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_WAIT_4_MEASUREMENT);
	EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], ' ');
	EXPECT_EQ(SDcard.loadingDataPointer[2], ' ');
	testIn.rcSignals.measurementSwitch = 2000;
	SDcard.MainState = SD_WAIT_4_MEASUREMENT;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_MEASUREMENT_ONGOING);
	EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], 'R');
	EXPECT_EQ(SDcard.loadingDataPointer[2], '\n');
	//meas ongoing
	SDcard.measTimePrev = 1.500;
	testIn.sysTime = 1.501;
	SDcard.MainState = SD_MEASUREMENT_ONGOING;
	SDcard.writeMeasData = false;
	SDcard.loadingDataCounter = 1;
	SDcard.loadingDataPointer[1] = ' ';
	SDcard.loadingDataPointer[2] = ' ';
	SDcard.loadingDataPointer[3] = ' ';
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_MEASUREMENT_ONGOING);
	EXPECT_NEAR(SDcard.measTimePrev, 1.500, 0.01);
	//
	SDcard.measTimePrev = 1.500;
	testIn.sysTime = 1.511;
	SDcard.MainState = SD_MEASUREMENT_ONGOING;
	SDcard.writeMeasData = false;
	SDcard.loadingDataCounter = 1;
	SDcard.loadingDataPointer[1] = ' ';
	SDcard.loadingDataPointer[2] = ' ';
	SDcard.loadingDataPointer[3] = ' ';
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_MEASUREMENT_ONGOING);
	EXPECT_NEAR(SDcard.measTimePrev, 1.510, 0.01);
	EXPECT_EQ(SDcard.loadingDataPointer[0], 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], '1');
	EXPECT_EQ(SDcard.loadingDataPointer[2], '.');
	EXPECT_EQ(SDcard.loadingDataPointer[3], '5');
	EXPECT_EQ(SDcard.writeMeasData, false);
	//
	SDcard.measTimePrev = 1.510;
	testIn.sysTime = 1.521;
	SDcard.MainState = SD_MEASUREMENT_ONGOING;
	SDcard.writeMeasData = false;
	SDcard.loadingDataCounter = 512;
	SDcard.loadingDataPointer[1] = ' ';
	SDcard.loadingDataPointer[2] = ' ';
	SDcard.loadingDataPointer[3] = ' ';
	testIn.rcSignals.measurementSwitch = 2000;
	SDcard.SDWriteState = SDWRITE_START;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_MEASUREMENT_ONGOING);
	EXPECT_NEAR(SDcard.measTimePrev, 1.520, 0.01);
	EXPECT_EQ(SDcard.sendingDataPointer[512] & 0xFF, '1');
	EXPECT_EQ(SDcard.loadingDataPointer[0] & 0xFF, 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], '.');
	EXPECT_EQ(SDcard.loadingDataPointer[2], '5');
	EXPECT_EQ(SDcard.writeMeasData, true);
	//
	SDcard.measTimePrev = 1.520;
	testIn.sysTime = 1.531;
	SDcard.MainState = SD_MEASUREMENT_ONGOING;
	SDcard.writeMeasData = false;
	SDcard.loadingDataCounter = 512;
	SDcard.loadingDataPointer[1] = ' ';
	SDcard.loadingDataPointer[2] = ' ';
	SDcard.loadingDataPointer[3] = ' ';
	testIn.rcSignals.measurementSwitch = 1000;
	RunSdCard(&testIn, &testOut);
	SDcard.SDWriteState = SDWRITE_FINISHED;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_WRITE_ROOT);
	EXPECT_NEAR(SDcard.measTimePrev, 1.530, 0.01);
	EXPECT_EQ(SDcard.sendingDataPointer[512] & 0xFF, '1');
	EXPECT_EQ(SDcard.loadingDataPointer[0] & 0xFF, 0xFE);
	EXPECT_EQ(SDcard.loadingDataPointer[1], '.');
	EXPECT_EQ(SDcard.loadingDataPointer[2], '5');
	EXPECT_EQ(SDcard.writeMeasData, false);
	//write root
	SDcard.MainState = SD_WRITE_ROOT;
	SDcard.SDWriteState = SDWRITE_START;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_WRITE_ROOT);
	//
	SDcard.MainState = SD_WRITE_ROOT;
	SDcard.SDWriteState = SDWRITE_FINISHED;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_WRITE_FAT);
	//write fat
	SDcard.MainState = SD_WRITE_FAT;
	SDcard.SDWriteState = SDWRITE_START;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_WRITE_FAT);
	//
	SDcard.MainState = SD_WRITE_FAT;
	SDcard.SDWriteState = SDWRITE_FINISHED;
	SDcard.writingMultiFATBlock = true;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_WRITE_FAT);
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_START);
	//
	SDcard.MainState = SD_WRITE_FAT;
	SDcard.SDWriteState = SDWRITE_FINISHED;
	SDcard.writingMultiFATBlock = false;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_WAIT_4_MEASUREMENT);
}

TEST(test_BT_SDcard, WriteFlow_Test)
{
	SpiInput testIn;
	SPIOutput testOut;

	uint8_t rootDir[512] = { 0x42,0x20,0x00,0x49,0x00,0x6E,0x00,0x66,  0x00,0x6F,0x00,0x0F,0x00,0x72,0x72,0x00,
							 0x6D,0x00,0x61,0x00,0x74,0x00,0x69,0x00,  0x6F,0x00,0x00,0x00,0x6E,0x00,0x00,0x00,
							 0x01,0x53,0x00,0x79,0x00,0x73,0x00,0x74,  0x00,0x65,0x00,0x0F,0x00,0x72,0x6D,0x00,
							 0x20,0x00,0x56,0x00,0x6F,0x00,0x6C,0x00,  0x75,0x00,0x00,0x00,0x6D,0x00,0x65,0x00,
							 0x53,0x59,0x53,0x54,0x45,0x4D,0x7E,0x31,  0x20,0x20,0x20,0x16,0x00,0xC3,0xE3,0x52,
							 0x9B,0x51,0x9B,0x51,0x00,0x00,0xE4,0x52,  0x9B,0x51,0x03,0x00,0x00,0x00,0x00,0x00,
							 'P' ,'A' ,'R' ,'A' ,'M' ,0x20,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x06,0x00,0x05,0x00,0x00,0x00,
							 'M' ,'E' ,'A' ,'S' ,'1' ,0x20,0x20,0x20,  0x20,'t' ,'x' ,'t' ,0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x80,0x00,0x10,0x11,0x12,0x13 };
	uint8_t FAT[512] = { 0x81,0x00,0x00,0x00, 0x82,0x00,0x00,0x00,  0x83,0x00,0x00,0x00, 0x84,0x00,0x00,0x00,
						 0x85,0x00,0x00,0x00, 0xFF,0xFF,0xFF,0x0F,  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00 };
	for (uint16_t i = 0; i < 512; i++)
	{
		SDcard.rootDirInfo[i+1] = rootDir[i];
		SDcard.FAT1Info[i+1] = FAT[i];
	}
	SDcard.newFile.name[0] = 'M';
	SDcard.newFile.name[1] = 'E';
	SDcard.newFile.name[2] = 'A';
	SDcard.newFile.name[3] = 'S';
	SDcard.newFile.numberInName = 2;
	SDcard.newFile.clusters[0] = 134;
	SDcard.newFile.numberOfClusters = 1;
	SDcard.newFile.size = 0;
	SDcard.rootDirEmptySlotNumber = 5;

	meas2Card.measureGyroRawX = true;
	meas2Card.measureGyroRawY = true;
	meas2Card.measureGyroRawZ = true;

	//wait 4 meas
	SPI.spiActivityGyro = INACTIVE;
	SPI.spiActivityAcc = INACTIVE;
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.MainState = SD_WAIT_4_MEASUREMENT;
	testIn.rcSignals.measurementSwitch = 2000;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.MainState, SD_MEASUREMENT_ONGOING);
	//measuring
	SDcard.measTimePrev = 2.000f;
	testIn.sysTime = 2.101;
	testIn.rcSignals.measurementSwitch = 1000;
	do
	{
		testIn.sysTime += 0.101;
		RunSdCard(&testIn, &testOut);

	} while (false == SDcard.writeMeasData);
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_RESPONSE);
	//write measured data
	SDcard.SdRx[7] = 0;
    testIn.sysTime = 4.000;
	DMAC_Handler();
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_DATA);
	SDcard.SdRx[515] = 0b0100;
    testIn.sysTime = 4.020;
    SPI.spiActivityGyro = PENDING;
    DMAC_Handler();
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_WRITE_FINISH);
    SPI.spiActivityGyro = INACTIVE;
	DMAC_Handler();
	SDcard.SdRx[0] = 0;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_WRITE_FINISH);
	DMAC_Handler();
	SDcard.SdRx[0] = 0xFE;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.writeMeasData, false);
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_START);
	EXPECT_EQ(SDcard.MainState, SD_WRITE_ROOT);
	EXPECT_EQ(SDcard.rootDirInfo[161], 'M');
	EXPECT_EQ(SDcard.rootDirInfo[162], 'E');
	EXPECT_EQ(SDcard.rootDirInfo[163], 'A');
	EXPECT_EQ(SDcard.rootDirInfo[164], 'S');
	EXPECT_EQ(SDcard.rootDirInfo[165], '2');
	//write ROOT
	SDcard.MainState = SD_WRITE_ROOT;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_RESPONSE);
	SDcard.SdRx[7] = 0;
    testIn.sysTime = 4.200;
	SDWriteWaitResponse();
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_DATA);
	SDcard.SdRx[515] = 0b0100;
    testIn.sysTime = 4.250;
    DMAC_Handler();
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_WRITE_FINISH);
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SdRx[0] = 0;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_WRITE_FINISH);
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SdRx[0] = 0xFE;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_START);
	EXPECT_EQ(SDcard.MainState, SD_WRITE_FAT);
	EXPECT_EQ(SDcard.FAT1Info[25], 0xFF);
	EXPECT_EQ(SDcard.FAT1Info[26], 0xFF);
	EXPECT_EQ(SDcard.FAT1Info[27], 0xFF);
	EXPECT_EQ(SDcard.FAT1Info[28], 0x0F);
	//write FAT
	SDcard.MainState = SD_WRITE_FAT;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_RESPONSE);
	SDcard.SdRx[7] = 0;
    testIn.sysTime = 4.300;
	SDWriteWaitResponse();
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_DATA);
	SDcard.SdRx[515] = 0b0100;
    testIn.sysTime = 4.350;
    DMAC_Handler();
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_WRITE_FINISH);
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SdRx[0] = 0;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_WAIT_WRITE_FINISH);
	SDcard.spiActivitySDCard = INACTIVE;
	SDcard.SdRx[0] = 0xFE;
	RunSdCard(&testIn, &testOut);
	EXPECT_EQ(SDcard.SDWriteState, SDWRITE_FINISHED);
	EXPECT_EQ(SDcard.MainState, SD_WAIT_4_MEASUREMENT);
	EXPECT_EQ(SDcard.newFile.numberInName, 3);
	EXPECT_EQ(SDcard.newFile.clusters[0], 135);

}

TEST(test_BT_SDcard, setGlobalTime_Test)
{
	date testDate{};
	float testTime{ 0 };

	//default
	SDcard.globalTime.year = 0;
	SDcard.globalTime.month = 0;
	SDcard.globalTime.day = 0;
	SDcard.globalTime.hour = 0;
	SDcard.globalTime.min = 0;
	SDcard.globalTime.sec = 0;
	testDate.year = 0;
	testDate.month = 0;
	testDate.day = 0;
	testDate.hour = 0;
	testDate.min = 0;
	testDate.sec = 0;
	testTime = 0;
	setGlobalTime(testDate, testTime);
	EXPECT_EQ(SDcard.globalTime.year, 0);
	EXPECT_EQ(SDcard.globalTime.month, 0);
	EXPECT_EQ(SDcard.globalTime.day, 0);
	EXPECT_EQ(SDcard.globalTime.hour, 0);
	EXPECT_EQ(SDcard.globalTime.min, 0);
	EXPECT_EQ(SDcard.globalTime.sec, 0);
	EXPECT_EQ(SDcard.sysTimeAtGlobalTime, 0);

	//
	testDate.year = 25;
	testDate.month = 5;
	testDate.day = 15;
	testDate.hour = 12;
	testDate.min = 34;
	testDate.sec = 26;
	testTime = 1235;
	setGlobalTime(testDate, testTime);
	EXPECT_EQ(SDcard.globalTime.year, 25);
	EXPECT_EQ(SDcard.globalTime.month, 5);
	EXPECT_EQ(SDcard.globalTime.day, 15);
	EXPECT_EQ(SDcard.globalTime.hour, 12);
	EXPECT_EQ(SDcard.globalTime.min, 34);
	EXPECT_EQ(SDcard.globalTime.sec, 26);
	EXPECT_EQ(SDcard.sysTimeAtGlobalTime, 1235);
}