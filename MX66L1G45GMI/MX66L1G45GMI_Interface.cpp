/**
 ********************************************************************************
 * @file    MX66L1G45GMI_Interface.cpp
 * @author  Javier
 * @date    Jan 28, 2026
 * @brief   Interface Implementation for MX66L1G45GMI Flash Memory Driver.
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "MX66L1G45GMI.hpp"
#include "DMATransfer.hpp"
#include "stm32g4xx_hal.h"

#include "main.h"

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/
extern SPI_HandleTypeDef hspi1; 


/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

 /************************************
 * FUNCTION DEFINITIONS
 ************************************/

 // Chip Select Implementation
void IMPL_SetCS(bool high) {
    HAL_GPIO_WritePin(SPI_FLASH_CS_GPIO_Port, SPI_FLASH_CS_Pin, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// Delay Implementation
void IMPL_Delay(uint32_t ms) {
    HAL_Delay(ms); 
}

void IMPL_Write(const uint8_t* data, uint16_t len) {
    // Use Polling for tiny commands (<=32 bytes) to avoid DMA overhead
    if (len <= 32) {
        HAL_SPI_Transmit(&hspi1, (uint8_t*)data, len, 100);
    } else {
        // Use DMA for larger data transfers (TX-only)
        if (DMAControl::Transfer(&hspi1, 0, (uint8_t*)data, nullptr, len) != HAL_OK)
        {
            SOAR_PRINT("IMPL_Write: DMA Transfer Error\n");
            return;
        }

        // BLOCKING WAIT: Protects the stack buffer 'tData' inside the driver
        while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY)
            ;
    }
}

// Read Implementation
void IMPL_Read(uint8_t* data, uint16_t len) {
    // Use DMA Tool
    DMAControl::Transfer(&hspi1, 0, nullptr, data, len);
    // BLOCKING WAIT
    int tick1 = HAL_GetTick();
 
    {
    while (HAL_GetTick() - tick1 < 500) { // 500ms timeout
        if(HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_READY) break;
        MX66_Delay(1);
    }

    }
    
    //while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);

    /*
    Unbounded busy-spin: while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY); can hang 
    forever if the SPI enters an error state or the DMA never starts. Add a timeout and handle HAL_SPI_STATE_ERROR
     (and consider yielding/delaying in RTOS builds).
    */
}

// Setup Function
void Setup_Flash_Interface() {
    static MX66_Config cfg;
    
    cfg.Write = IMPL_Write;
    cfg.Read  = IMPL_Read;
    cfg.SetCS = IMPL_SetCS;
    cfg.Delay = IMPL_Delay;
    
    // Inject dependencies into the abstract driver
    MX66_Init(&cfg);
}
// End of MX66L1G45GMI_Interface.cpp