# STM32 C++ Driver for MMC5983MA

### Description
This is a C++ driver for the MEMSIC MMC5983MA 3-axis magnetometer, designed for STM32 microcontrollers using the HAL library.

This driver communicates with the sensor over the SPI interface and is built to be easily integrated into STM32 projects.

---

### Features
* Read the device Product ID.
* Trigger single-shot magnetic field measurements.
* Perform SET and RESET operations.
* Read 18-bit raw data for X, Y, and Z axes.
* Calculate scaled magnetic field data in Gauss.

---

### Project Structure
* `spi/mmc5983ma.hpp`: The main driver class header. It defines the `MMC5983MA` class, public functions, and private helpers.
* `spi/mmc5983ma.cpp`: The driver implementation file. Contains the logic for all class functions.
* `spi/mmc5983ma_regs.hpp`: A helper header that defines all register addresses and bitmasks for the sensor.
* `spi_wrapper.hpp` / `spi_wrapper.cpp`: An abstraction layer that wraps the STM32 HAL SPI functions (`HAL_SPI_TransmitReceive`, etc.) into a simple C++ class. The `MMC5983MA` driver uses this wrapper for all SPI communication.
* `spi/MMC5983MA_RevA_4-3-19.pdf`: The official sensor datasheet.

---

### Dependencies
* **STM32 HAL Library:** The driver depends on HAL types (`SPI_HandleTypeDef`, `GPIO_TypeDef*`, etc.).
* **`spi_wrapper`:** The `MMC5983MA` class requires a pointer to an initialized `SPI_Wrapper` object.

---

### How to Use
Here is the high-level workflow for integrating the driver:

1.  **Include Files:**
    In your main application file, include the necessary headers:
    ```cpp
    #include "spi_wrapper.hpp"
    #include "mmc5983ma.hpp"
    ```

2.  **Ensure Hardware is Configured:**
    Before using the driver, make sure your STM32's `SPI_HandleTypeDef` (e.g., `hspi1`) and the Chip Select (CS) GPIO pin are configured and initialized by the HAL (e.g., in `main.c` via STM32CubeMX).

3.  **Create Driver Instances:**
    * Create an instance of `SPI_Wrapper`, passing it a pointer to your initialized `SPI_HandleTypeDef`.
    * Create an instance of `MMC5983MA`, passing it a pointer to your `SPI_Wrapper` instance, along with the `GPIO_TypeDef*` and `uint16_t` pin number for your CS pin.

4.  **Initialize the Sensor:**
    * Call the `.begin()` method on your `MMC5983MA` object.
    * Check the return value (`bool`) to confirm that communication was successful and the sensor's Product ID was correctly read.

5.  **Read Data in Your Main Loop:**
    * Call `.triggerMeasurement()` to request a new reading from the sensor.
    * Wait for the measurement to complete (refer to the datasheet for measurement time, e.g., `HAL_Delay(10)` for 100Hz bandwidth).
    * Create a `MagData` struct variable.
    * Call `.readData(your_mag_data_struct)`. If it returns `true`, your struct is now populated with the latest X, Y, and Z data.

---

### Complete Example
Check `main_read_test.cpp` for a complete example on this.

### Datasheet
Initial commit, MMC5983MA magnetometer project Structure.