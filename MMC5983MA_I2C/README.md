# STM32 C++ Driver for MMC5983MA (I2C)

### Description
This is a high-performance C++ driver for the MEMSIC MMC5983MA 3-axis magnetometer, designed for STM32 microcontrollers using the HAL library.

**Key Feature:** This driver uses a **Stateless I2C Wrapper**, making it thread-safe.

---

### Features
* **Stateless Architecture:** Wrapper stores no state, preventing race conditions in preemptive environments.
* **Standard I2C Support:** Communicates via the standard 2-wire I2C interface.
* **Core Functionality:**
    * Read Product ID (Validation).
    * Trigger Magnetic Field Measurements.
    * Perform SET and RESET operations (De-gaussing).
    * Read and scale 18-bit X, Y, Z magnetic data (in Gauss).

---

### Project Structure
* `mmc5983ma_i2c.hpp` / `.cpp`: The main driver class. Handles register logic and data conversion.
* `i2c_wrapper.hpp` / `.cpp`: A lightweight abstraction layer for STM32 HAL I2C functions.
* `mmc5983ma_regs.hpp`: Register map and bit definitions.
* `main_read_test.cpp`: Example implementation.

---

### Dependencies
* **STM32 HAL Library:** Requires `stm32f4xx_hal.h` (or equivalent for your F1/H7/L4 series).
* **I2C Handle:** You must initialize an `I2C_HandleTypeDef` (e.g., `hi2c1`) in your `main.c`.

---

### How to Use

1.  **Include Headers:**
    ```cpp
    #include "i2c_wrapper.hpp"
    #include "mmc5983ma_i2c.hpp"
    ```

2.  **Instantiate Objects:**
    Since the driver is stateless, you create the wrapper and pass it by pointer to the driver, along with the device address.
    ```cpp
    // 1. Create Wrapper (Pass the HAL Handle)
    I2C_Wrapper i2cBus(&hi2c1);

    // 2. Create Driver (Pass Wrapper Pointer + I2C Address)
    // Standard Address: 0x30 (0110000 shifted left by 1 is 0x60, handled internally)
    MMC5983MA mag(&i2cBus, 0x30);
    ```

3.  **Initialization & Reading:**
    ```cpp
    if (mag.begin()) {
        // Init success
    }

    // In your loop:
    mag.triggerMeasurement();
    HAL_Delay(10); // Wait for conversion (~8ms)
    
    MagData data;
    if (mag.readData(data)) {
        // Use data.scaledX, data.scaledY, data.scaledZ
    }
    ```

### Address Note
The MMC5983MA 7-bit address is `0x30`. The driver automatically handles the left-shift required by the HAL library.