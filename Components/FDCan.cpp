/*
 * FDCan.cpp
 *
 *  Created on: Aug 24, 2024
 *      Author: goada
 */

#include "FDCan.h"

#include <cstdio>
#include <cstring>

#include "main.h"

/* @brief For debug purposes, called in main loop
 *
 */

/* @brief Infinite loop?
 *
 */
void CANError() {
  while (1) {
    HAL_Delay(100);
  }
}

FDCanController::~FDCanController() {
  if (callbackcontroller == this) {
    callbackcontroller = nullptr;
  }
}

#define MIN(a, b) (((a) > (b)) ? (b) : (a))

/* @brief Send a string through CANBus, not including the null terminator
 * @param msg Null-terminated string
 * @param ID Starting CAN message ID, will be incremented if len > 64 bytes
 * @return Success
 */
bool FDCanController::Send(const char *msg, uint32_t ID) {
  return Send((const uint8_t *)msg, strlen(msg), ID);
}

/* @brief Send a message through CANBus, in multiple frames and incrementing the
 * msg ID if needed.
 * @param msg Bytes to send
 * @param len Length of msg
 * @param ID Starting CAN identifier of message, will be incremented if len > 64
 * bytes
 * @return Success
 */
bool FDCanController::Send(const uint8_t *msg, size_t len, uint32_t ID) {
  size_t framesToSend = (len - 1) / 64 + 1;

  if (ID >= 2048 - framesToSend) {
    // ID too large
    return false;
  }

  for (size_t frame = 0; frame < framesToSend; frame++) {
    FDCAN_TxHeaderTypeDef txheader;
    txheader.Identifier = ID + frame;
    txheader.IdType = FDCAN_STANDARD_ID;
    txheader.TxFrameType = FDCAN_DATA_FRAME;
    // Length will be 64 bytes, except for the final frame,
    // which will be minimum required for the remaining data
    if (frame == framesToSend - 1)
      txheader.DataLength = FDGetModDLC(len);
    else
      txheader.DataLength = FDCAN_DLC_BYTES_64;

    txheader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    txheader.BitRateSwitch = FDCAN_BRS_ON;
    txheader.FDFormat = FDCAN_FD_CAN;
    txheader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    txheader.MessageMarker = 0;

    uint8_t data[64];
    // If data won't fill the whole buffer, fill buf with zeros.
    if (len < 64) {
      memset(data, 0x00, sizeof(data));
    }

    memcpy(data, msg + 64 * frame, MIN(len, 64));
    len -= 64;

    while (HAL_FDCAN_GetTxFifoFreeLevel(fdcan) < 1) {
    }

    HAL_StatusTypeDef stat =
        HAL_FDCAN_AddMessageToTxFifoQ(fdcan, &txheader, &data[0]);

    if (stat != HAL_OK) {
      return false;
    }
  }

  return true;
}

/* @brief Callback for receiving a CAN message.
 * @param CAN handle
 */
void RXMsgCallback(FDCAN_HandleTypeDef *fdcan) {
  if (callbackcontroller) callbackcontroller->RaiseFXFlag();
}

/* @brief Constructor. Also initializes FDCAN filters and callback.
 * Calls CANError if error in initialization. Two controllers should not
 * share the same FDCAN peripheral.
 * @param fdcan Handle to FDCAN peripheral
 */
FDCanController::FDCanController(FDCAN_HandleTypeDef *fdcan) {
  this->fdcan = fdcan;

  HAL_StatusTypeDef stat;

  if (InitFilters() != HAL_OK) {
    CANError();
  }

  // Turn on callback for receiving msg
  stat =
      HAL_FDCAN_ActivateNotification(fdcan, FDCAN_IT_RX_BUFFER_NEW_MESSAGE, 0);
  if (stat != HAL_OK) {
    CANError();
  }

  // Set callback
  stat = HAL_FDCAN_RegisterCallback(fdcan, HAL_FDCAN_RX_BUFFER_NEW_MSG_CB_ID,
                                    RXMsgCallback);
  if (stat != HAL_OK) {
    CANError();
  }

  stat = HAL_FDCAN_Start(fdcan);
  if (stat != HAL_OK) {
    CANError();
  }
}

/* @brief Registers a filter that directs an FDCAN message ID to a certain RX
 * buffer. Filter will be placed in next available filter slot. Must ensure
 * there are enough Std Filters configured in the FDCAN parameter settings.
 * @param msgID FDCAN message ID to direct.
 * @param rxBufferNum FDCAN RX buffer index to direct to.
 * @return HAL_OK on success.
 */
HAL_StatusTypeDef FDCanController::RegisterFilterRXBuf(uint16_t msgID,
                                                       uint8_t rxBufferNum) {
  if (nextUnregisteredFilterID >= NUM_FD_FILTERS) {
    return HAL_ERROR;
  }
  FDCAN_FilterTypeDef filter;

  filter.IdType = FDCAN_STANDARD_ID;
  filter.FilterIndex = nextUnregisteredFilterID;
  filter.FilterConfig = FDCAN_FILTER_TO_RXBUFFER;
  filter.FilterID1 = msgID;
  filter.RxBufferIndex = rxBufferNum;
  filter.IsCalibrationMsg = 0;

  nextUnregisteredFilterID++;
  return HAL_FDCAN_ConfigFilter(fdcan, &filter);
}

/* @brief Registers a series of RX buffers where a (multi-frame) log data
 * matching a certain range of message IDs will be directed. Once registered,
 * receiving a frame into the latest of these buffers will trigger the callback.
 * The log will require one frame per 64 bytes, rounded up.
 * @param msgIDStart The lowest message ID that the (multi-frame) log will
 * contain. Will use one successive ID per required frame. Different log types
 * must not register overlapping ID ranges. Must be between 0 and 2048-[required
 * frames].
 * @param rxBufStart The lowest RX buffer index the log will be directed to.
 * 					 Must be between 0 and [registered
 * buffer num]-[required frames].
 * @param length Length in bytes that the log contains.
 *
 *
 */
HAL_StatusTypeDef FDCanController::RegisterLogType(uint16_t msgIDStart,
                                                   uint8_t rxBufStart,
                                                   uint16_t length) {
  registeredLogs[numRegisteredLogs].startingRXBuf = rxBufStart;
  registeredLogs[numRegisteredLogs].endingRXBuf =
      rxBufStart + (length - 1) / 64;
  registeredLogs[numRegisteredLogs].byteLength = length;
  registeredLogs[numRegisteredLogs].startingMsgID = msgIDStart;
  for (uint16_t i = 0; i < (length / 64 + 1); i++) {
    HAL_StatusTypeDef stat =
        RegisterFilterRXBuf(msgIDStart + i, rxBufStart + i);
    if (stat != HAL_OK) {
      return stat;
    }
  }
  numRegisteredLogs++;

  return HAL_OK;
}

/* @brief Registers all log types and the backup FIFO filter.
 * Filters are registered in order, and will match in order. Earlier filters
 * should be higher priority. Ensure the backup FIFO is registered last.
 * @return HAL_OK on success
 */
HAL_StatusTypeDef FDCanController::InitFilters() {
  if (RegisterLogType(0x000, 0, 64) != HAL_OK) {
    return HAL_ERROR;
  }
  if (RegisterFilterRXFIFO(0x000, 0x7FF) != HAL_OK) {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/* @brief FDCAN frames can only be certain size values between 0 and 64.
 * 		  Returns next-highest valid FDCAN size for a given byte length.
 * @param unroundedlen Byte length to be rounded.
 * @return Total byte size representable by series of valid FDCAN frame sizes.
 */
const uint16_t FDCanController::FDRoundDataSize(uint16_t unroundedLen) {
  uint8_t mod = (unroundedLen - 1) % 64 + 1;

  if (mod <= 8) {
  } else if (mod <= 12)
    mod = 12;
  else if (mod <= 16)
    mod = 16;
  else if (mod <= 20)
    mod = 20;
  else if (mod <= 24)
    mod = 24;
  else if (mod <= 32)
    mod = 32;
  else if (mod <= 48)
    mod = 48;
  else
    mod = 64;

  return (unroundedLen - 1) / 64 * 64 + mod;
}

/* @brief Returns the FDCAN DLC code of the final frame required to send a msg
 * of given size.
 * @param unroundedlen Total message size in bytes
 * @return Valid FDCAN_DLC_BYTES value capable of containing final frame size
 */
const uint32_t FDCanController::FDGetModDLC(uint16_t unroundedLen) {
  uint8_t mod = (unroundedLen - 1) % 64 + 1;

  if (mod <= 8) return mod;
  if (mod <= 12) return FDCAN_DLC_BYTES_12;
  if (mod <= 16) return FDCAN_DLC_BYTES_16;
  if (mod <= 20) return FDCAN_DLC_BYTES_20;
  if (mod <= 24) return FDCAN_DLC_BYTES_24;
  if (mod <= 32) return FDCAN_DLC_BYTES_32;
  if (mod <= 48) return FDCAN_DLC_BYTES_48;
  return FDCAN_DLC_BYTES_64;
}

inline void FDCanController::RaiseFXFlag() { RXFlag = true; }

/* @brief Receives the first full message collected in the dedicated RX buffers
 * into the out buffer.
 * @param out Output data. Must be the size of the maximum expected message
 * size, rounded with FDRoundDataSize().
 * @param msgID Output of the FDCAN message ID of the log.
 * @return Size in bytes of received message. If no message, returns 0 and out
 * is unmodified.
 */
uint16_t FDCanController::ReceiveFromRXBuf(uint8_t *out, uint16_t *msgID) {
  if (!RXFlag) {
    return 0;
  }
  for (uint8_t i = 0; i < numRegisteredLogs; i++) {
    const LogRegister &thisRegisteredLog = registeredLogs[i];
    if (HAL_FDCAN_IsRxBufferMessageAvailable(fdcan,
                                             thisRegisteredLog.endingRXBuf)) {
      // Received full log
      // uint8_t data[FDRoundDataSize(thisRegisteredLog.byteLength)];
      // memset(data,0x00,sizeof(data));
      uint8_t *d = out;
      for (uint8_t b = thisRegisteredLog.startingRXBuf;
           b <= thisRegisteredLog.endingRXBuf; b++) {
        FDCAN_RxHeaderTypeDef rxh;
        HAL_FDCAN_GetRxMessage(fdcan, b, &rxh, d);
        d += 64;
      }

      // data now contains entire multi-frame log, potentially padded with
      // some 0s
      // Received full log, do send or something
      *msgID = thisRegisteredLog.startingMsgID;
      return thisRegisteredLog.byteLength;
    }
  }
  RXFlag = false;
  return 0;
}

/* @brief Registers a filter that directs a range of msg IDs to RX FIFO 0.
 * @param msgIDMin Minimum message ID, inclusive, 0-2047
 * @param msgIDMax Maximum message ID, inclusive, 0-2047
 * @return HAL_OK on success
 */
HAL_StatusTypeDef FDCanController::RegisterFilterRXFIFO(uint16_t msgIDMin,
                                                        uint16_t msgIDMax) {
  FDCAN_FilterTypeDef filter;
  filter.IdType = FDCAN_STANDARD_ID;
  filter.FilterIndex = nextUnregisteredFilterID;
  filter.FilterType = FDCAN_FILTER_RANGE;
  filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  filter.FilterID1 = msgIDMin;
  filter.FilterID2 = msgIDMax;

  nextUnregisteredFilterID++;
  return HAL_FDCAN_ConfigFilter(fdcan, &filter);
}
