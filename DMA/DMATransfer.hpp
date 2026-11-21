/**
 ********************************************************************************
 * @file    DMATransfer.hpp
 * @author  Javier
 * @date    2026-01-10
 * @brief   Cross-family (STM32G4 / STM32H7) DMA transfer wrapper with FreeRTOS
 *          task-notification synchronization and automatic H7 D-Cache handling.
 *
 * Family auto-detection uses the STM32Cube HAL configuration header included
 * by main.h (STM32G4xx_HAL_CONF_H / STM32H7xx_HAL_CONF_H) — no manual user
 * defines required.
 *
 * Note for STM32H7:
 *  - DMA1/DMA2 cannot access DTCMRAM (0x20000000); place DMA buffers in AXI SRAM
 *    (D1) or SRAM1/2/3 (D2) and align RX buffers to 32 bytes (alignas(32)).
 *
 * Note for DMA completion callbacks:
 *  - Forward HAL_*CpltCallback / HAL_*ErrorCallback to
 *    DMAControl::NotifyCompleteFromISR(handle, isError).
 ********************************************************************************
 */

#ifndef DMATRANSFER_HPP
#define DMATRANSFER_HPP

/************************************
 * INCLUDES
 ************************************/
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include <cstdint>
#include <type_traits>

/************************************
 * AUTO MCU FAMILY DETECTION
 ************************************/
#if defined(STM32H7xx_HAL_CONF_H) || defined(__STM32H7xx_HAL_H)
  #define SOAR_DMA_MCU_H7 1
#elif defined(STM32G4xx_HAL_CONF_H) || defined(__STM32G4xx_HAL_H)
  #define SOAR_DMA_MCU_G4 1
#endif

/************************************
 * CLASS DEFINITIONS
 ************************************/
class DMAControl {
public:
    static constexpr uint32_t DEFAULT_TIMEOUT_MS = 100;

    /**
     * @brief RTOS-friendly millisecond delay (replaces HAL_Delay inside tasks).
     *        Yields CPU via vTaskDelay when FreeRTOS scheduler is running.
     */
    static inline void DelayMs(uint32_t ms) {
        if (ms == 0) {
            return;
        }
        if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
            const TickType_t ticks = pdMS_TO_TICKS(ms);
            vTaskDelay(ticks > 0 ? ticks : 1);
        } else {
            // Pre-scheduler fallback without HAL_Delay
            const uint32_t start = HAL_GetTick();
            while ((HAL_GetTick() - start) < ms) {
                __NOP();
            }
        }
    }

    /**
     * @brief Call from HAL_*CpltCallback (isError=false) or HAL_*ErrorCallback (isError=true).
     */
    static inline void NotifyCompleteFromISR(void* handle, bool isError = false) {
        if (handle == nullptr) {
            return;
        }
        for (uint8_t i = 0; i < MAX_SLOTS; ++i) {
            if (slots_[i].handle == handle) {
                TaskHandle_t task = slots_[i].task;
                slots_[i].handle = nullptr;
                slots_[i].task = nullptr;
                if (task != nullptr) {
                    BaseType_t woken = pdFALSE;
                    const uint32_t result = isError ? NOTIFY_ERR : NOTIFY_OK;
                    xTaskNotifyFromISR(task, result, eSetValueWithOverwrite, &woken);
                    portYIELD_FROM_ISR(woken);
                }
                return;
            }
        }
    }

    /**
     * @brief Generic SPI / I2C / UART transfer using DMA + FreeRTOS notification.
     *        Falls back to HAL blocking if the handle has no DMA channel linked in CubeMX.
     */
    template <typename HandleType>
    static HAL_StatusTypeDef Transfer(HandleType* handle,
                                      uint16_t devAddr,
                                      uint8_t* txData,
                                      uint8_t* rxData,
                                      uint16_t size,
                                      uint32_t timeoutMs = DEFAULT_TIMEOUT_MS) {
        if (handle == nullptr || size == 0) {
            return HAL_ERROR;
        }

        // === SPI ===
#ifdef HAL_SPI_MODULE_ENABLED
        if constexpr (std::is_same_v<HandleType, SPI_HandleTypeDef>) {
            (void)devAddr;
            const bool canDmaTx = (txData == nullptr) || (handle->hdmatx != nullptr);
            const bool canDmaRx = (rxData == nullptr) || (handle->hdmarx != nullptr);

            if (canDmaTx && canDmaRx && IsRtosRunning()) {
                CleanDCache(txData, size);
                int8_t slot = RegisterWaiter(handle);
                if (slot < 0) {
                    return HAL_BUSY;
                }

                HAL_StatusTypeDef status = HAL_ERROR;
                if (txData != nullptr && rxData != nullptr) {
                    status = HAL_SPI_TransmitReceive_DMA(handle, txData, rxData, size);
                } else if (txData != nullptr) {
                    status = HAL_SPI_Transmit_DMA(handle, txData, size);
                } else if (rxData != nullptr) {
                    status = HAL_SPI_Receive_DMA(handle, rxData, size);
                }

                if (status != HAL_OK) {
                    ReleaseWaiter(slot);
                    return status;
                }

                status = WaitForCompletion(slot, timeoutMs);
                if (status == HAL_OK) {
                    InvalidateDCache(rxData, size);
                }
                return status;
            }

            // Fallback when DMA is not configured on this SPI handle or scheduler not started
            if (txData != nullptr && rxData != nullptr) {
                return HAL_SPI_TransmitReceive(handle, txData, rxData, size, timeoutMs);
            } else if (txData != nullptr) {
                return HAL_SPI_Transmit(handle, txData, size, timeoutMs);
            } else if (rxData != nullptr) {
                return HAL_SPI_Receive(handle, rxData, size, timeoutMs);
            }
            return HAL_ERROR;
        }
#endif

        // === I2C Master ===
#ifdef HAL_I2C_MODULE_ENABLED
        if constexpr (std::is_same_v<HandleType, I2C_HandleTypeDef>) {
            if (txData != nullptr && rxData == nullptr) {
                if (handle->hdmatx != nullptr && IsRtosRunning()) {
                    CleanDCache(txData, size);
                    int8_t slot = RegisterWaiter(handle);
                    if (slot < 0) {
                        return HAL_BUSY;
                    }
                    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit_DMA(handle, devAddr, txData, size);
                    if (status != HAL_OK) {
                        ReleaseWaiter(slot);
                        return status;
                    }
                    return WaitForCompletion(slot, timeoutMs);
                }
                return HAL_I2C_Master_Transmit(handle, devAddr, txData, size, timeoutMs);
            } else if (rxData != nullptr && txData == nullptr) {
                if (handle->hdmarx != nullptr && IsRtosRunning()) {
                    int8_t slot = RegisterWaiter(handle);
                    if (slot < 0) {
                        return HAL_BUSY;
                    }
                    HAL_StatusTypeDef status = HAL_I2C_Master_Receive_DMA(handle, devAddr, rxData, size);
                    if (status != HAL_OK) {
                        ReleaseWaiter(slot);
                        return status;
                    }
                    status = WaitForCompletion(slot, timeoutMs);
                    if (status == HAL_OK) {
                        InvalidateDCache(rxData, size);
                    }
                    return status;
                }
                return HAL_I2C_Master_Receive(handle, devAddr, rxData, size, timeoutMs);
            }
            return HAL_ERROR;
        }
#endif

        // === UART ===
#ifdef HAL_UART_MODULE_ENABLED
        if constexpr (std::is_same_v<HandleType, UART_HandleTypeDef>) {
            (void)devAddr;
            if (txData != nullptr && rxData == nullptr) {
                if (handle->hdmatx != nullptr && IsRtosRunning()) {
                    CleanDCache(txData, size);
                    int8_t slot = RegisterWaiter(handle);
                    if (slot < 0) {
                        return HAL_BUSY;
                    }
                    HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(handle, txData, size);
                    if (status != HAL_OK) {
                        ReleaseWaiter(slot);
                        return status;
                    }
                    return WaitForCompletion(slot, timeoutMs);
                }
                return HAL_UART_Transmit(handle, txData, size, timeoutMs);
            } else if (rxData != nullptr && txData == nullptr) {
                if (handle->hdmarx != nullptr && IsRtosRunning()) {
                    int8_t slot = RegisterWaiter(handle);
                    if (slot < 0) {
                        return HAL_BUSY;
                    }
                    HAL_StatusTypeDef status = HAL_UART_Receive_DMA(handle, rxData, size);
                    if (status != HAL_OK) {
                        ReleaseWaiter(slot);
                        return status;
                    }
                    status = WaitForCompletion(slot, timeoutMs);
                    if (status == HAL_OK) {
                        InvalidateDCache(rxData, size);
                    }
                    return status;
                }
                return HAL_UART_Receive(handle, rxData, size, timeoutMs);
            }
            return HAL_ERROR;
        }
#endif

        return HAL_ERROR;
    }

#ifdef HAL_I2C_MODULE_ENABLED
    /**
     * @brief Register-addressed I2C Mem Read/Write using DMA + FreeRTOS notification.
     *        Automatically falls back to HAL_I2C_Mem_* if DMA is not linked on hi2c yet.
     */
    static HAL_StatusTypeDef MemTransfer(I2C_HandleTypeDef* hi2c,
                                         uint16_t devAddr,
                                         uint16_t memAddr,
                                         uint16_t memAddSize,
                                         uint8_t* txData,
                                         uint8_t* rxData,
                                         uint16_t size,
                                         uint32_t timeoutMs = DEFAULT_TIMEOUT_MS) {
        if (hi2c == nullptr || size == 0) {
            return HAL_ERROR;
        }

        if (txData != nullptr && rxData == nullptr) {
            if (hi2c->hdmatx != nullptr && IsRtosRunning()) {
                CleanDCache(txData, size);
                int8_t slot = RegisterWaiter(hi2c);
                if (slot < 0) {
                    return HAL_BUSY;
                }
                HAL_StatusTypeDef status = HAL_I2C_Mem_Write_DMA(hi2c, devAddr, memAddr, memAddSize, txData, size);
                if (status != HAL_OK) {
                    ReleaseWaiter(slot);
                    return status;
                }
                return WaitForCompletion(slot, timeoutMs);
            }
            return HAL_I2C_Mem_Write(hi2c, devAddr, memAddr, memAddSize, txData, size, timeoutMs);
        }

        if (rxData != nullptr && txData == nullptr) {
            if (hi2c->hdmarx != nullptr && IsRtosRunning()) {
                int8_t slot = RegisterWaiter(hi2c);
                if (slot < 0) {
                    return HAL_BUSY;
                }
                HAL_StatusTypeDef status = HAL_I2C_Mem_Read_DMA(hi2c, devAddr, memAddr, memAddSize, rxData, size);
                if (status != HAL_OK) {
                    ReleaseWaiter(slot);
                    return status;
                }
                status = WaitForCompletion(slot, timeoutMs);
                if (status == HAL_OK) {
                    InvalidateDCache(rxData, size);
                }
                return status;
            }
            return HAL_I2C_Mem_Read(hi2c, devAddr, memAddr, memAddSize, rxData, size, timeoutMs);
        }

        return HAL_ERROR;
    }
#endif

private:
    struct WaitSlot {
        void* handle;
        TaskHandle_t task;
    };

    static constexpr uint8_t  MAX_SLOTS  = 8;
    static constexpr uint32_t NOTIFY_OK  = 1U;
    static constexpr uint32_t NOTIFY_ERR = 2U;
    inline static WaitSlot slots_[MAX_SLOTS]{};

    static inline bool IsRtosRunning() {
        return xTaskGetSchedulerState() == taskSCHEDULER_RUNNING;
    }

    static inline int8_t RegisterWaiter(void* handle) {
        // Clear any stale notification value before arming DMA
        uint32_t dummy = 0;
        (void)xTaskNotifyWait(0, 0xFFFFFFFFU, &dummy, 0);

        taskENTER_CRITICAL();
        for (uint8_t i = 0; i < MAX_SLOTS; ++i) {
            if (slots_[i].handle == nullptr) {
                slots_[i].handle = handle;
                slots_[i].task = xTaskGetCurrentTaskHandle();
                taskEXIT_CRITICAL();
                return static_cast<int8_t>(i);
            }
        }
        taskEXIT_CRITICAL();
        return -1;
    }

    static inline void ReleaseWaiter(int8_t slot) {
        if (slot < 0 || slot >= static_cast<int8_t>(MAX_SLOTS)) {
            return;
        }
        taskENTER_CRITICAL();
        slots_[slot].handle = nullptr;
        slots_[slot].task = nullptr;
        taskEXIT_CRITICAL();
    }

    static inline HAL_StatusTypeDef WaitForCompletion(int8_t slot, uint32_t timeoutMs) {
        uint32_t notifyVal = 0;
        if (xTaskNotifyWait(0, 0xFFFFFFFFU, &notifyVal, pdMS_TO_TICKS(timeoutMs)) != pdPASS) {
            ReleaseWaiter(slot);
            return HAL_TIMEOUT;
        }
        return (notifyVal == NOTIFY_OK) ? HAL_OK : HAL_ERROR;
    }

    static inline void CleanDCache(const void* addr, uint32_t size) {
#if defined(SOAR_DMA_MCU_H7) && defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
        if (addr != nullptr && size > 0 && (SCB->CCR & SCB_CCR_DC_Msk)) {
            const uintptr_t start = reinterpret_cast<uintptr_t>(addr) & ~static_cast<uintptr_t>(31U);
            const uintptr_t end   = (reinterpret_cast<uintptr_t>(addr) + size + 31U) & ~static_cast<uintptr_t>(31U);
            SCB_CleanDCache_by_Addr(reinterpret_cast<uint32_t*>(start), static_cast<int32_t>(end - start));
        }
#else
        (void)addr;
        (void)size;
#endif
    }

    static inline void InvalidateDCache(void* addr, uint32_t size) {
#if defined(SOAR_DMA_MCU_H7) && defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
        if (addr != nullptr && size > 0 && (SCB->CCR & SCB_CCR_DC_Msk)) {
            const uintptr_t start = reinterpret_cast<uintptr_t>(addr) & ~static_cast<uintptr_t>(31U);
            const uintptr_t end   = (reinterpret_cast<uintptr_t>(addr) + size + 31U) & ~static_cast<uintptr_t>(31U);
            SCB_InvalidateDCache_by_Addr(reinterpret_cast<uint32_t*>(start), static_cast<int32_t>(end - start));
        }
#else
        (void)addr;
        (void)size;
#endif
    }
};

#endif /* DMATRANSFER_HPP */
