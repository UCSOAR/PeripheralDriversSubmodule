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
