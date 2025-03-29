/**
 ********************************************************************************
 * @file    MX66L1G45GMI.cpp
 * @author  shiva
 * @date    Mar 26, 2025
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/
//-------------------------------------------------------------------------------------------------
// STM32 SPI Driver
//-------------------------------------------------------------------------------------------------
void W25Q_Delay(uint32_t time)
{
	HAL_Delay(time);
}

void csLOW (void)
{
	HAL_GPIO_WritePin(SPI_FLASH_CS_GPIO_Port, SPI_FLASH_CS_Pin, GPIO_PIN_RESET);
}

void csHIGH (void)
{
	HAL_GPIO_WritePin(SPI_FLASH_CS_GPIO_Port, SPI_FLASH_CS_Pin, GPIO_PIN_SET);
}

void SPI_Write (uint8_t *data, uint16_t len)
{
	HAL_SPI_Transmit(&W25Q_SPI, data, len, 2000);
}

void SPI_Read (uint8_t *data, uint16_t len)
{
	HAL_SPI_Receive(&W25Q_SPI, data, len, 5000);
}

/************************************
 * FUNCTION DEFINITIONS
 ************************************/
uint32_t MX66_ReadID (void)
{
	uint8_t tData = 0x9F;
	uint8_t rData[3];
	csLOW();
	SPI_Write(&tData, 1);
	SPI_Read(rData, 3);
	csHIGH();
	uint32 ret = ((rData[0]<<16)|(rData[1]<<8)| rData[2]);
	return ret;
}


uint8_t MX66_ReadStatus(int reg) // WIP Bit???
{
	uint8_t tData,rData;
	switch(reg){
		case 1: tData = 0x05; break;
		case 2: tData = 0x35; break;
		case 3: tData = 0x15; break;
		default:
			printf("Invalid status register 0\n");
			return 0;
	}
	csLOW();
	SPI_Write(&tData, 1);
	SPI_Read(&rData, 1);
	csHIGH();
	return (rData);
}

void MX66_WriteStatus(int reg, uint8_t newstatus)
{
	uint8_t tData[2];
	switch(reg){
		case 1: tData[0] = 0x01; break;
		case 2: tData[0] = 0x31; break;  // Cant understand cases
		case 3: tData[0] = 0x11; break;
		default: return;
	}

	tData[1]=newstatus;
	write_enable();
	csLOW();
	SPI_Write(tData, 2);
	csHIGH();
	write_disable();
}

void W25Q_ReadSFDP(uint8_t *rData)
{
	uint8_t tData[5]={0x5A,0,0,0,0};
	csLOW();
	SPI_Write(tData, 5);
	SPI_Read(rData, 256);  			//	Why 256???
	csHIGH();
}


void MX66_Read (uint32_t block, uint16_t offset, uint32_t size, uint8_t *rData)
{
	uint8_t tData[6];
	uint32_t memAddr = (block*FS_SECTOR_SIZE) + offset;

	tData[0] = 0x03;
	tData[1] = (memAddr>>16)&0xFF;
	tData[2] = (memAddr>>8)&0xFF;
	tData[3] = (memAddr)&0xFF;

	csLOW();
	SPI_Write(tData, 4);
	SPI_Read(rData, size);
	csHIGH();
}


void MX66_FastRead (uint32_t block, uint16_t offset, uint32_t size, uint8_t *rData)
{
	uint8_t tData[6];
	uint32_t memAddr = (block*FS_SECTOR_SIZE) + offset;

	tData[0] = 0x0B;
	tData[1] = (memAddr>>16)&0xFF;
	tData[2] = (memAddr>>8)&0xFF;
	tData[3] = (memAddr)&0xFF;
	tData[4] = 0;

	csLOW();
	SPI_Write(tData, 5);
	SPI_Read(rData, size);
	csHIGH();
}


void write_enable(void)
{
	uint8_t tData = 0x06;
	csLOW();
	SPI_Write(&tData, 1);
	csHIGH();
	HAL_Delay(5);
}

void write_disable(void)
{
	uint8_t tData = 0x04;
	csLOW();
	SPI_Write(&tData, 1);
	csHIGH();
	HAL_Delay(5);
}

void MX66_Erase_Chip(void)
{
	uint8_t tData = 0x60;

	write_enable();
	csLOW();
	SPI_Write(&tData, 1);
	csHIGH();
	write_disable();

	while(W25Q_ReadStatus(1)&0x01);		// ????
}

void MX66_Erase_Sector(uint16_t numsector)
{
	uint8_t tData[6];
	uint32_t memAddr = numsector*FS_SECTOR_SIZE;					// Each sector contains 16 pages * 256 bytes

	write_enable();

	tData[0] = 0x21;
	tData[1] = (memAddr>>16)&0xFF;
	tData[2] = (memAddr>>8)&0xFF;
	tData[3] = (memAddr)&0xFF;

	csLOW();
	SPI_Write(tData, 4);
	csHIGH();

	while(W25Q_ReadStatus(1)&0x01);

	write_disable();
}

void MX66_Erase_Block(uint32_t block)
{
	uint8_t tData[6];
	uint32_t memAddr = block*FS_BLOCK_SIZE;					// Each sector contains 16 pages * 256 bytes

	write_enable();

	tData[0] = 0xD8;
	tData[1] = (memAddr>>16)&0xFF;
	tData[2] = (memAddr>>8)&0xFF;
	tData[3] = (memAddr)&0xFF;

	csLOW();
	SPI_Write(tData, 4);
	csHIGH();

	while(W25Q_ReadStatus(1)&0x01);

	write_disable();
}

