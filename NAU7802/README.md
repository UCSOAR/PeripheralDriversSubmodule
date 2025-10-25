
# STM32 C++ Driver for NAU7802

This is a simple, lightweight C++ driver for the Nuvoton NAU7802 24-bit ADC, designed to be used with the STM32 HAL library in a C++ environment (for example, STM32CubeIDE).

### Features

- Handles sensor initialization and power-up sequence
- Software reset
- Sets PGA gain (from 1x to 128x)
- Checks if new conversion data is ready
- Reads the 24-bit signed ADC result

### Project Structure

The driver is split into logical components to make it easy to understand and port:

- `I2C_Wrapper.hpp` / `.cpp`
  - A small C++ class that wraps the C-style STM32 HAL I2C functions (`HAL_I2C_Mem_Write` / `HAL_I2C_Mem_Read`).
  - Acts as the low-level communication handler for register reads/writes.
- `NAU7802_regs.hpp`
  - Register addresses, bit masks, and gain values for the NAU7802 (datasheet-based definitions).
- `NAU7802.hpp` / `.cpp`
  - The main driver with logic for communicating with the sensor.
  - Defines `NAU7802_PARAMS` for configuration and `NAU7802_OUT` for output data.
  - Uses `I2C_Wrapper` to perform all register operations.
- `main_read_test.cpp` / `ExampleUsage.cpp`
  - Example `main()` files demonstrating driver initialization and reading.

### How to use

Follow these steps in your project (assumes HAL and `hi2c1` are available):

1. Include the headers in your `main.cpp` or equivalent:

```c
#include "main.h"
#include "I2C_Wrapper.hpp"
#include "NAU7802.hpp"

// Ensure hi2c1 is declared (defined in main.c)
extern I2C_HandleTypeDef hi2c1;
```

2. Initialize the driver (after `MX_I2C1_Init()`):

```c
// Create the I2C wrapper and config/data structs
I2C_Wrapper i2c_bus_1(&hi2c1);
NAU7802_PARAMS adc_config;
NAU7802_OUT adc_data;

// Configure parameters
adc_config.initialGain = NAU7802_GAIN_1X; // choose desired gain

// Create the driver object (constructor will initialize the sensor)
NAU7802 adc(adc_config, i2c_bus_1);

// Check initialization
if (!adc.get_isInitialized()) {
    Error_Handler();
}
```

3. Read data in your main loop:

```c
while (1) {
    if (adc.isReady()) {
        if (adc.readSensor(&adc_data) == HAL_OK) {
            // Use adc_data.raw_reading (32-bit signed)
            // e.g. printf("Raw ADC: %ld\r\n", adc_data.raw_reading);
        }
    }
    HAL_Delay(100); // 100 ms is reasonable for default sample rate
}
```

### Complete example

See `main_read_test.cpp` or `ExampleUsage.cpp` in this driver folder for a complete example that initializes the sensor at 1x gain and continuously reads raw ADC values.

### Dependencies

- STM32 HAL (driver uses HAL I2C functions)
- C++11 or newer (used for small utilities like `std::array` in `I2C_Wrapper`)

### Datasheet

For full register details and calibration notes, consult the NAU7802 datasheet (e.g. en-us--DS_NAU7802_DataSheet_EN_Rev2.6.pdf).