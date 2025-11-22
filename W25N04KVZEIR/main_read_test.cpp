/*
 * main_read_test.cpp
 *
 * A complete example of how to initialize and read data from
 * the x using the C++ driver.
 *
 * Assumes HAL initialization for SPI and GPIOs is done in main.c.
 */

// Include the HAL library for SPI_HandleTypeDef, GPIO_TypeDef, etc.
// The exact path might vary based on your project structure.
extern "C" {
    #include "main.h" // Or "stm32f4xx_hal.h" directly
}

// Include the driver files

// For printf debugging (e.g., via SWV/ITM)
#include <stdio.h>

// ##################################################################
// ##                 Hardware Configuration                     ##
// ##################################################################

// --- IMPORTANT ---
// You must update these definitions to match your hardware configuration
// set up in your .ioc (STM32CubeMX) file.

// 1. Define your SPI Handle (e.g., hspi1, hspi2)
// This handle must be initialized in main.c

// 2. Define your Chip Select (CS) Pin Port and Pin
// This pin must be configured as a GPIO_Output in main.c

// ##################################################################

// 1. Create an instance of the SPI_Wrapper
//    Pass it the pointer to your initialized HAL SPI handle.

// 2. Create an instance of the MMC5983MA driver
//    Pass it the SPI wrapper, CS port, and CS pin.

// 3. Create a struct to hold the magnetometer data

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    // --- HAL and System Init ---
    // (This is typically called before main() or at the start)
    // HAL_Init();
    // SystemClock_Config();
    // MX_GPIO_Init();
    // MX_SPI1_Init();
    // ...
    

    // --- Driver Initialization ---
    