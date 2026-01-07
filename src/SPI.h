// Header to configure and set SPI for gyro acc and SD card communication



#ifndef SPI_HPP
#define SPI_HPP

#include "stdint.h"
#include "SPI_common.h"


typedef enum E_SPIReadWrite : uint32_t
{
	SPI_MEMS_WRITE = 0x00,
	SPI_MEMS_READ  = 0x80
};

// Method to configure all SPI related settings
void SetupSPI(void);

// Method to configuring SPI pins and registers
void SetupSPIPins(void);

// Method to configuring DMAC registers
void SetupDMAC(void);

// Method to configuring gyro mems registers
void SetupGyro(void);

// Method to configure gyro interrupt pin
void SetupGyroIntPin(void);

// Method to configuring acc mems registers
void SetupAcc(void);

// Method to configure acc interrupt pin
void SetupAccIntPin(void);

// Method to enable interrupt handling of SPI
void EnableIntHandling(void);

// Method to run SPI related functions cyclically
void RunSPI(SPIInput* spiInput, SPIOutput* spiOutput);

// Function to check if SPI RX finished
bool IsRxDone(void);

// Method to wait for SPI RX end
void WaitRxDone(void);

// Method to read 1 register from MEMS
void ReadFromMEMS(E_ChipSelect CS, uint32_t adr2read);

// Method to write 1 register from MEMS
void WriteToMEMS(E_ChipSelect CS, uint32_t adr2write, uint32_t data2write);

// Method to read data(6 register) from MEMS
void ReadMEMSMeas(E_ChipSelect CS, uint32_t adr2read);

// Method to calculate signal from received buffer
void calcSignalGyro(volatile signal* gyroSig, volatile uint8_t* buffer);

// Method to calculate signal from received buffer
void calcSignalAcc(volatile signal* accSig, volatile uint8_t* buffer);

// Method to calculate gyro offset in the very early startup
void calcOffsetGyro(volatile signal* gyroSig);

// Method to compensate gyro data with offset
void compensateData(volatile signal* Sig);

// Method to get gyro data consistently
void getGyroAndAcc(sigOut* axisGyro, sigOut* axisAcc);

// Method to get SPI struct
spi_st* getSPI();

#endif // !SPI_HPP
