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
bool MX66L1G45GMI_INIT();
 /************************************
 * FUNCTION DEFINITIONS
 ************************************/

 // Chip Select Implementation
void IMPL_SetCS(bool high) {
    HAL_GPIO_WritePin(SPI_FLASH_CS_GPIO_Port, SPI_FLASH_CS_Pin, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// Delay Implementation
/* 
 * NOTE:
 * This function is provided as a hook for the driver to perform (ms) delays.
 * By default, it uses HAL_Delay, which is a blocking delay (busy-waits for the specified time).
 * Due to RTOS environments, you may want to replace this with osDelay, vTaskDelay, or a non-blocking delay to avoid blocking the scheduler.
 * The function is named generically so you can swap the implementation as needed for your project.
*/
void IMPL_Delay(uint32_t ms) {
    HAL_Delay(ms); // Blocking delay; replace with osDelay/vTaskDelay for RTOS if needed
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
    // BLOCKING WAIT with timeout
    // We use MX66_Delay(1) inside the loop to yield or delay for 1ms between checks.
    // MX66_Delay is a wrapper that calls the delay function provided in the config (IMPL_Delay by default).
    // This allows the delay to be swapped for an RTOS-friendlier version if needed, without changing this code.
    int tick1 = HAL_GetTick();
    while (HAL_GetTick() - tick1 < 500) { // 500ms timeout
        if(HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_READY) break;
        MX66_Delay(1); // Uses the injected delay function (IMPL_Delay by default)
    }
    if (HAL_GetTick() - tick1 >= 500) {
        SOAR_PRINT("IMPL_Read: DMA Transfer Timeout\n");
        return;
    }
}
// Setup Function
// This function sets up the MX66_Config struct with the hardware-specific implementations.
// The Delay function pointer is set to IMPL_Delay, so all driver delays (via MX66_Delay) will use this implementation.
// This allows the delay mechanism to be swapped project-wide by changing only IMPL_Delay.
void Setup_Flash_Interface() {
    static MX66_Config cfg;
    cfg.Write = IMPL_Write;
    cfg.Read  = IMPL_Read;
    cfg.SetCS = IMPL_SetCS;
    cfg.Delay = IMPL_Delay;
    // Inject dependencies into the abstract driver
    MX66_Init(&cfg);
}


bool MX66L1G45GMI_INIT(){
    
    // CRITICAL: ON MAIN.C Initialize DMA before SPI
    // MX_DMA_Init(); // <--- DMA MUST be initialized BEFORE SPI
    // MX_SPI1_Init();
    
    
    // Inject the hardware dependencies into the driver
    Setup_Flash_Interface();
    
    // Verify Connection
    
    uint32_t flashID = MX66_ReadID();
    
    // Expected ID for MX66L1G45GMI is 0xC2201B
    // Manufacturer: 0xC2 (Macronix) | Type: 0x20 | Density: 0x1B (1Gb)
    if (flashID != 0xC2201B) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

// End of MX66L1G45GMI_Interface.cpp