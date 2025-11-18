/*
 * main_read_test.cpp
 *
 * Test application for MMC5983MA I2C Driver.
 */

extern "C" {
    #include "main.h" // Contains I2C handle declaration (hi2c1)
}

#include "i2c_wrapper.hpp"
#include "mmc5983ma_i2c.hpp" // Ensure this matches your actual header filename

#include <stdio.h>

// ##################################################################
// ##                 Hardware Configuration                     ##
// ##################################################################

// 1. Define your I2C Handle (e.g., hi2c1, hi2c2)
extern I2C_HandleTypeDef hi2c1;
#define MMC_I2C_HANDLE &hi2c1

// 2. Define Sensor Address
// Datasheet (Pg 9) specifies 7-bit address: 0110000 (0x30)
#define MMC_I2C_ADDR 0x30

// ##################################################################

// 1. Create the I2C Wrapper instance
I2C_Wrapper i2cBus(MMC_I2C_HANDLE);

// 2. Create the MMC5983MA Driver instance
//    Pass pointer to wrapper and the I2C address
MMC5983MA magnetometer(&i2cBus, MMC_I2C_ADDR);

// 3. Data container
MagData magData;

/**
 * @brief  The application entry point.
 */
int main(void)
{
    // --- HAL Init (Usually done in main.c) ---
    // HAL_Init();
    // SystemClock_Config();
    // MX_I2C1_Init();
    
    printf("--- MMC5983MA I2C Test ---\r\n");

    // 4. Initialize the sensor
    if (magnetometer.begin()) {
        printf("Sensor Initialized. Product ID Verified.\r\n");
    } else {
        printf("Sensor Init Failed! Check wiring and pull-ups.\r\n");
        while (1) {
            HAL_Delay(1000); // Trap on failure
        }
    }

    // --- Main Loop ---
    while (1)
    {
        // 5. Trigger Measurement
        magnetometer.triggerMeasurement();

        // 6. Wait for measurement to complete
        // Default bandwidth (100Hz) takes ~8ms
        HAL_Delay(10); 

        // 7. Read Data
        if (magnetometer.readData(magData)) {
            // Print scaled values
            printf("X: %.4f G | Y: %.4f G | Z: %.4f G\r\n",
                   magData.scaledX,
                   magData.scaledY,
                   magData.scaledZ);
        } else {
            printf("Data not ready yet.\r\n");
        }

        HAL_Delay(500); // Read every 500ms
    }
}