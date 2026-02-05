
# MX66L1G45GMI Flash Memory Driver

**Authors:** Shiva (Core Logic), Javier (Interface & DMA)  
**Date:** 31 Jan 2026
**Target:** STM32G4 / STM32H7

---

## 1. Overview

This driver provides a full-featured interface for the Macronix MX66L1G45GMI SPI Flash memory. It utilizes a layered architecture to decouple high-level flash commands from hardware-specific SPI/DMA implementations.

---

## 2. Architecture

The driver is split into three distinct layers:

### A. Abstract Driver (`MX66L1G45GMI.cpp/.hpp`)

- Hardware-agnostic logic for the flash memory
- Handles command opcode sequencing (Read, Write, Erase)
- Address calculation
- Status register polling (Checking the BUSY bit)
- Multi-page write logic (`MX66_Write_Block`)

### B. Interface Implementation (`MX66L1G45GMI_Interface.cpp`)

- Connects the Abstract Driver to the actual STM32 hardware
- Implements `IMPL_Write`, `IMPL_Read`, `IMPL_SetCS`, and `IMPL_Delay`
- Uses a hybrid strategy: Polling for small commands (≤ 32 bytes) and DMA for large data payloads
- note: Delay implementation on `MX66L1G45GMI_Interface.cpp` is subject to change via editing `IMPL_Delay` below.

```cpp
// ... code ...

// Delay Implementation
void IMPL_Delay(uint32_t ms) {
    // Change this line to switch between HAL_Delay, osDelay, vTaskDelay, etc
    HAL_Delay(ms); 
}

// ... code ...
```

### C. DMA Tool (`DMATransfer.hpp`)

- Template-based utility
- Automated Cache Coherency (D-Cache cleaning/invalidating for G4/H7)
- Protocol-specific DMA calls (e.g., `HAL_SPI_TransmitReceive_DMA` for SPI)  

---

## 3. Hardware Specifications

| Feature         | Value           | Details                                 |
|-----------------|-----------------|-----------------------------------------|
| Total Capacity  | 128 MB          | 1 Gbit                                  |
| Page Size       | 256 Bytes       | Basic programming unit                  |
| Sector Size     | 4 KB            | 16 pages × 256 bytes                    |
| Block Size      | 64 KB           | 16 sectors × 4 KB                       |

---

## 4. Usage & Initialization

> **Critical Setup Note:**
> DMA must be initialized before SPI in your `main.c` or equivalent startup file. This is required for correct operation of the flash driver.

### Initialization Example

```c
// In your main.c or main.cpp:

// 1. Initialize DMA and SPI peripherals in this order:
MX_DMA_Init(); // DMA *MUST* be initialized BEFORE SPI
MX_SPI1_Init();

// 2. Initialize the Flash Driver:
extern bool MX66L1G45GMI_INIT();

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_SPI1_Init();
    
    if (MX66L1G45GMI_INIT() != HAL_OK) {
        // Handle error: Flash not detected
    }
    while (1)
    {
        // main loop
    }
}
```

**What does `MX66L1G45GMI_INIT()` do?**

- Injects hardware interface functions (SPI, CS, Delay, DMA) into the driver
- Verifies the flash chip is present (checks for ID 0xC2201B)
- Returns `HAL_OK` if successful, `HAL_ERROR` if not

**Note:**

- You do NOT need to call `Setup_Flash_Interface()` directly; it is called inside `MX66L1G45GMI_INIT()`.

---

## 5. API Reference

### 5.1. Identification

```c
bool MX66L1G45GMI_INIT(void);
```

Initializes the driver and checks for the correct flash chip. Returns `HAL_OK` or `HAL_ERROR`.

```c
uint32_t MX66_ReadID(void);
```

Returns the 3-byte Manufacturer and Device ID.

### 5.2. Read Operations

```c
void MX66_Read(uint32_t block, uint16_t offset, uint32_t size, uint8_t *rData);
```

Standard read starting at a specific block and offset.

```c
void MX66_FastRead(uint32_t block, uint16_t offset, uint32_t size, uint8_t *rData);
```

Uses the 0x0B command with dummy cycles for higher clock frequencies.

### 5.3. Write Operations

```c
void MX66_Write_Page(uint32_t page, uint16_t offset, uint32_t size, const uint8_t *data);
```

Writes data within a single 256-byte page. **Note:** Does not handle page roll-over.

```c
void MX66_Write_Block(uint32_t block, uint16_t offset, uint32_t size, const uint8_t *data);
```

Writes data across multiple pages, handling page boundaries automatically.

---

## 6. Notes on Implementation

- The interface layer (`MX66L1G45GMI_Interface.cpp`) uses polling for small SPI commands (≤32 bytes) and DMA for larger transfers.
- All hardware-specific details (SPI handle, CS pin, delay) are abstracted and injected at runtime.
- Timeout and error handling are implemented for DMA reads.

### 5.4. Erase Operations

```c
void MX66_Erase_Sector(uint16_t numsector); // Erases a 4KB sector
void MX66_Erase_Block(uint32_t block);      // Erases a 64KB block
void MX66_Erase_Chip(void);                 // Erases the entire chip (blocking, can take several minutes)
```

---

## 6. Implementation Details

### Hybrid Transmission Strategy

The Interface (`IMPL_Write`) intelligently switches methods based on data length:

- **Length ≤ 32 Bytes:** Uses Polling (`HAL_SPI_Transmit`). This avoids the setup overhead of DMA for command headers and small register writes.
- **Length > 32 Bytes:** Uses DMA (`DMAControl::Transfer`). This frees the CPU during bulk data transfer, though the current implementation blocks to ensure buffer safety.

### DMA & Cache Safety

The `DMATransfer` tool automatically manages cache coherency for G4/H7 processors:

- **TX:** Cleans D-Cache (flushes cache to RAM) before transmission.
- **RX:** Invalidates D-Cache (forces read from RAM) after reception.
