# DMA Transfer Driver (`DMATransfer.hpp`)

**Author:** Javier  
**Supported Families:** STM32G4 (e.g., `STM32G491ME`) & STM32H7 (e.g., `STM32H743VIT6`)  
**RTOS:** FreeRTOS (CMSIS_V1 / CMSIS_V2)

---

## Overview

`DMATransfer.hpp` is a header-only C++17/20 wrapper (`DMAControl`) that replaces blocking HAL calls (`HAL_SPI_*`, `HAL_I2C_*`, `HAL_UART_*`) with **DMA + FreeRTOS direct-to-task notifications**. While a DMA transfer is in flight, the calling task sleeps at **0% CPU** instead of busy-polling.

### Key Features
- **Zero-Config MCU Auto-Detection:** Automatically detects STM32G4 vs. STM32H7 from the HAL config header included by `main.h` (`STM32G4xx_HAL_CONF_H` / `STM32H7xx_HAL_CONF_H`).
- **Automatic H7 D-Cache Coherence:** Cleans TX buffers before DMA and invalidates RX buffers *after* DMA completes on Cortex-M7 (`__DCACHE_PRESENT == 1`), aligned to 32-byte cache lines. Compiles to a no-op on Cortex-M4 (G4).
- **Graceful Fallback:** If a peripheral handle does not have a DMA channel linked in CubeMX yet (`hdmatx == nullptr` or `hdmarx == nullptr`), `DMAControl` automatically falls back to the matching blocking HAL call so drivers still work during bring-up.
- **Race-Free FreeRTOS Sync:** Uses `xTaskNotifyWait` / `xTaskNotifyFromISR` (passing OK/ERR status in the notification value) across up to 8 concurrent DMA handles.

---

## Setup Instructions

### 1. CubeMX (`.ioc`) DMA Configuration
Open your `.ioc` file → **System Core → DMA** (or inside each peripheral's **DMA Settings** tab) and add channels for every direction you use:

| Peripheral | Required DMA Requests | Mode | Increment Address | Data Width |
| :--- | :--- | :--- | :--- | :--- |
| **I2C** (e.g., `I2C3`) | `I2C3_RX` (Periph→Mem) & `I2C3_TX` (Mem→Periph) | Normal | Periph: OFF, **Mem: ON** | Byte / Byte |
| **SPI** (e.g., `SPI1`) | `SPI1_RX` (Periph→Mem) & `SPI1_TX` (Mem→Periph) | Normal | Periph: OFF, **Mem: ON** | Byte / Byte |
| **UART** (HAL-managed) | `USARTx_RX` & `USARTx_TX` | Normal | Periph: OFF, **Mem: ON** | Byte / Byte |

> **Note:** Full-duplex SPI (`HAL_SPI_TransmitReceive_DMA`) requires **both** `SPIx_TX` and `SPIx_RX` channels.

### 2. CubeMX (`.ioc`) NVIC Interrupts (Critical!)
Go to **System Core → NVIC** and enable the following interrupts with **Preemption Priority ≥ `5`** (never `0` — FreeRTOS forbids calling `*FromISR` APIs from priority `< configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY`):

- **DMA channel global interrupts** (`DMA1 channel X global interrupt`, etc.) — verify Priority ≥ `5` and **Uses FreeRTOS functions** is checked.
- **I2C Event & Error interrupts** (`I2Cx event interrupt`, `I2Cx error interrupt`) — **Required!** STM32 HAL I2C DMA fires `HAL_I2C_MemRxCpltCallback` / `HAL_I2C_MasterRxCpltCallback` from the I2C `STOPF` event ISR, *not* the DMA ISR. If `I2Cx event interrupt` is disabled, DMA transfers will time out.
- **SPI global interrupt** (`SPIx global interrupt`) — required for SPI DMA completion/error callbacks.
- **USART global interrupt** (`USARTx global interrupt`) — required if using HAL UART DMA.

### 3. Forward HAL Callbacks in `RunInterface.cpp`
In your C-to-C++ bridge file (e.g., `Components/SysCore/RunInterface.cpp`), include `DMATransfer.hpp` and forward the HAL completion/error callbacks inside `extern "C"`:

```cpp
#include "../../SoarDrivers/DMA/DMATransfer.hpp"

extern "C" {
#ifdef HAL_I2C_MODULE_ENABLED
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef* hi2c) { DMAControl::NotifyCompleteFromISR(hi2c, false); }
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* hi2c) { DMAControl::NotifyCompleteFromISR(hi2c, false); }
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef* hi2c)    { DMAControl::NotifyCompleteFromISR(hi2c, false); }
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef* hi2c)    { DMAControl::NotifyCompleteFromISR(hi2c, false); }
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef* hi2c)        { DMAControl::NotifyCompleteFromISR(hi2c, true);  }
#endif

#ifdef HAL_SPI_MODULE_ENABLED
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi)       { DMAControl::NotifyCompleteFromISR(hspi, false); }
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef* hspi)       { DMAControl::NotifyCompleteFromISR(hspi, false); }
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef* hspi)     { DMAControl::NotifyCompleteFromISR(hspi, false); }
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef* hspi)        { DMAControl::NotifyCompleteFromISR(hspi, true);  }
#endif

#ifdef HAL_UART_MODULE_ENABLED
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)    { DMAControl::NotifyCompleteFromISR(huart, false); }
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)    { DMAControl::NotifyCompleteFromISR(huart, false); }
void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart)     { DMAControl::NotifyCompleteFromISR(huart, true);  }
#endif
}
```

### 4. STM32H7-Only Buffer Rules
On **STM32H743VIT6** (Cortex-M7 with D-Cache):
- **RAM Domain:** DMA1/DMA2 **cannot** access `DTCMRAM` (`0x20000000`). Place DMA buffers in AXI SRAM (`0x24000000`) or D2 SRAM (`0x30000000`) using linker section attributes (or ensure the task stack/buffer is not in DTCM).
- **32-Byte Alignment:** Align RX DMA buffers to 32 bytes (`alignas(32) uint8_t rxBuf[32];`) so cache invalidation does not disturb adjacent variables sharing a cache line.

*(On **STM32G491ME**, normal SRAM1/SRAM2 works directly with no alignment or cache restrictions.)*

---

## API Usage Examples

```cpp
#include "DMATransfer.hpp"

// 1. Register-addressed I2C (e.g., NAU7802)
uint8_t val = 0x01;
DMAControl::MemTransfer(&hi2c3, devAddr << 1, REG_ADDR, I2C_MEMADD_SIZE_8BIT, &val, nullptr, 1);
DMAControl::MemTransfer(&hi2c3, devAddr << 1, REG_ADDR, I2C_MEMADD_SIZE_8BIT, nullptr, rxBuf, 3);

// 2. SPI (Full-Duplex, TX-only, or RX-only)
DMAControl::Transfer(&hspi1, 0, txBuf, rxBuf, len);    // Full-duplex
DMAControl::Transfer(&hspi1, 0, txBuf, nullptr, len);  // TX-only
DMAControl::Transfer(&hspi1, 0, nullptr, rxBuf, len);  // RX-only

// 3. I2C Master (raw read/write)
DMAControl::Transfer(&hi2c3, devAddr << 1, txBuf, nullptr, len);
DMAControl::Transfer(&hi2c3, devAddr << 1, nullptr, rxBuf, len);

// 4. UART (HAL UART_HandleTypeDef only; not for LL UARTDriver)
DMAControl::Transfer(&huart3, 0, txBuf, nullptr, len);
```

---

## Which Sleep / Delay to Use in FreeRTOS

**Never call `HAL_Delay()` inside a running FreeRTOS task.** `HAL_Delay()` busy-spins in a loop on `uwTick`, burning 100% CPU and starving equal- or lower-priority tasks.

| Situation | What to Use | Notes |
| :--- | :--- | :--- |
| **Waiting for DMA to finish** | `DMAControl::Transfer(...)` / `MemTransfer(...)` *(uses `xTaskNotifyWait` internally)* | Sleeps at **0% CPU** and wakes immediately from the DMA/peripheral ISR. |
| **Simple delay inside a driver** (may be called before or after `osKernelStart`) | `DMAControl::DelayMs(ms)` | Calls `vTaskDelay(pdMS_TO_TICKS(ms))` when RTOS is running; safe tick fallback before scheduler starts. |
| **Simple sleep inside a FreeRTOS task** | `vTaskDelay(pdMS_TO_TICKS(ms))` or `osDelay(ms)` | Yields CPU immediately; good for retry loops and non-critical waits. |
| **Exact periodic sensor sampling (no drift)** | `vTaskDelayUntil(&lastWakeTick, pdMS_TO_TICKS(periodMs))` | Maintains an exact cadence (e.g., 10 Hz / 100 Hz) regardless of processing time. |
| **Waiting on a command / event queue** | `qEvtQueue->ReceiveWait(cm)` or `qEvtQueue->Receive(cm, timeoutMs)` | Blocks at 0% CPU until an event or ISR message arrives. |
| **Sub-millisecond (µs) hardware timing** (e.g., CS setup, RS485 turnaround) | Hardware timer counter (`__HAL_TIM_GET_COUNTER`) or `DWT->CYCCNT` | FreeRTOS tick is 1 ms (`vTaskDelay(1)` ≥ 1 ms); use a hardware counter for µs waits. |
| **Low-rate periodic callbacks without a task** | FreeRTOS Software Timer (`xTimerCreate` / `SoarOS` `Timer`) | Avoids allocating a full task stack for simple periodic triggers. |
| **Before `osKernelStart()` only** (early `main()` hardware init) | `DMAControl::DelayMs(ms)` or `HAL_Delay(ms)` | Only safe before the scheduler starts. |
