/*
 * main_read_test.cpp
 *
 * A complete example of how to initialize and read data from
 * the MMC5983MA magnetometer using the C++ driver.
 *
 * Assumes HAL initialization for SPI and GPIOs is done in main.c.
 */

// Include the HAL library for SPI_HandleTypeDef, GPIO_TypeDef, etc.
// The exact path might vary based on your project structure.
extern "C" {
    #include "main.h" // Or "stm32f4xx_hal.h" directly
}

// Include the driver files
#include "spi_wrapper.hpp"
#include "spi/mmc5983ma.hpp"

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
extern SPI_HandleTypeDef hspi1;
#define MMC_SPI_HANDLE &hspi1

// 2. Define your Chip Select (CS) Pin Port and Pin
// This pin must be configured as a GPIO_Output in main.c
#define MMC_CS_PORT GPIOA
#define MMC_CS_PIN  GPIO_PIN_4

// ##################################################################

// 1. Create an instance of the SPI_Wrapper
//    Pass it the pointer to your initialized HAL SPI handle.
SPI_Wrapper spiBus(MMC_SPI_HANDLE);

// 2. Create an instance of the MMC5983MA driver
//    Pass it the SPI wrapper, CS port, and CS pin.
MMC5983MA magnetometer(&spiBus, MMC_CS_PORT, MMC_CS_PIN);

// 3. Create a struct to hold the magnetometer data
MagData magData;

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
    
    printf("--- MMC5983MA Read Test ---\r\n");

    // 4. Initialize the sensor
    if (magnetometer.begin()) {
        printf("Sensor initialized successfully. Product ID OK.\r\n");
    } else {
        printf("Sensor initialization failed! Check wiring or SPI config.\r\n");
        while (1) {
            // Hang here on failure
            HAL_Delay(1000);
        }
    }

    // --- Main Application Loop ---
    while (1)
    {
        // 5. Trigger a new measurement
        magnetometer.triggerMeasurement();

        // 6. Wait for the measurement to complete.
        // The README suggests 10ms for 100Hz bandwidth.
        // Refer to the datasheet (spi/MMC5983MA_RevA_4-3-19.pdf) for exact times.
        HAL_Delay(10); // 10ms delay

        // 7. Attempt to read the data
        if (magnetometer.readData(magData)) {
            // If readData() returns true, the 'magData' struct is populated.

            // Print the scaled data in Gauss
            printf("X: %.4f G | Y: %.4f G | Z: %.4f G\r\n",
                   magData.scaledX,
                   magData.scaledY,
                   magData.scaledZ);
            
            // You can also access the raw 18-bit integer data
            // printf("Raw X: %ld | Raw Y: %ld | Raw Z: %ld\r\n",
            //        magData.rawX,
            //        magData.rawY,
            //        magData.rawZ);

        } else {
            // readData() returned false, meaning the "data ready"
            // bit was not set. This might mean the HAL_Delay was too short.
            printf("Failed to read data. Data not ready.\r\n");
        }

        // Wait before the next reading
        HAL_Delay(1000); // Read once per second
    }
}