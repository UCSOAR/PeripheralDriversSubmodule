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

FDCanController *callbackcontroller = nullptr;

void CANError() {
  while (1) {
    HAL_Delay(100);  //
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
 * @param logIndex Index of the log type to be sent
 * @return Success
 */
bool FDCanController::SendStringByLogIndex(const char *msg, uint16_t logIndex) {
  return SendByMsgID((const uint8_t *)msg, strlen(msg),
                     registeredLogs[logIndex].startingMsgID);
}

/* @brief Send a message through CANBus, in multiple frames and incrementing the
 * msg ID if needed.
 * @param msg Bytes to send
 * @param len Length of msg
 * @param ID Starting CAN identifier of message, will be incremented if len > 64
 * bytes
 * @return Success
 */
bool FDCanController::SendByMsgID(const uint8_t *msg, size_t len, uint16_t ID) {
  size_t framesToSend = (len - 1) / 64 + 1;

  if (ID >= 2048 - framesToSend) {
    // ID too large
    return false;
  }

  FDCAN_TxHeaderTypeDef txheader;
  txheader.IdType = FDCAN_STANDARD_ID;
  txheader.TxFrameType = FDCAN_DATA_FRAME;
  txheader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  txheader.BitRateSwitch = FDCAN_BRS_ON;
  txheader.FDFormat = FDCAN_FD_CAN;
  txheader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  txheader.MessageMarker = 0;
  txheader.DataLength = FDCAN_DLC_BYTES_64;

  uint8_t data[64];

  for (size_t frame = 0; frame < framesToSend; frame++) {
    txheader.Identifier = ID + frame;

    // Length will be 64 bytes, except for the final frame,
    // which will be minimum required for the remaining data
    if (frame == framesToSend - 1) txheader.DataLength = FDGetModDLC(len);

    // If data won't fill the whole buffer, fill buf with zeros.
    if (len < 64) {
      memset(data + len, 0x00, 64 - len);
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

/* @brief Get a pointer to the RX buffer at an index.
 * @param index Index of the buffer to get
 * @return Pointer to the RX buffer struct at index
 */
FDCanController::RXBuffer* FDCanController::GetRXBuf(uint16_t index) {
	return &buffers[index];
}

/* @brief Gets the pointer to the RX buffer that contains the given CAN ID in the log registered to it.
 * @param canid The CAN ID to look for
 * @return Pointer to the RX buffer that a registered log maps the ID to. nullptr if none found.
 */
FDCanController::RXBuffer* FDCanController::GetBufferFromCanID(uint16_t canid) {
	for (uint16_t i = 0; i < numRegisteredLogs; i++) {
		const LogRegister& thisReg = registeredLogs[i];
		if(thisReg.startingRXBuf <= canid && thisReg.endingRXBuf >= canid) {
			return GetRXBuf(thisReg.startingRXBuf+(canid-thisReg.startingMsgID));
		}
	}
	return nullptr;
}

/* Overridden callback that fires when an RX message is received.
 * Immediately reroutes the message to an RX buffer associated with the CAN ID.
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if (RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE)
    {
        FDCAN_RxHeaderTypeDef rxHeader;
        uint8_t buf[64];


        HAL_FDCAN_GetRxMessage(hfdcan,
                               FDCAN_RX_FIFO0,
                               &rxHeader,
                               buf);
        FDCanController::RXBuffer* rxbuf = callbackcontroller->GetBufferFromCanID(rxHeader.Identifier);
        if(rxbuf) {
			memcpy(rxbuf->data,buf,sizeof(buf));
			rxbuf->available = true;


			callbackcontroller->RaiseFXFlag();
        }
    }
}

/* @brief Constructor. Also initializes FDCAN filters and callback.
 * Calls CANError if error in initialization. Two controllers should not
 * share the same FDCAN peripheral.
 * @param fdcan Handle to FDCAN peripheral
 */
FDCanController::FDCanController(FDCAN_HandleTypeDef *fdcan,
                                 FDCanController::LogInitStruct *logs,
                                 uint16_t numLogs) {
  this->fdcan = fdcan;

  InitFDCAN();
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
  if (nextUnregisteredFilterID >= numFDFilters) {
    return HAL_ERROR;
  }
  FDCAN_FilterTypeDef filter;

  filter.IdType = FDCAN_STANDARD_ID;
  filter.FilterIndex = nextUnregisteredFilterID;
  filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  filter.FilterID1 = msgID;
  //filter.RxBufferIndex = rxBufferNum;
  //filter.IsCalibrationMsg = 0;

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
 */
HAL_StatusTypeDef FDCanController::RegisterLogType(uint16_t msgIDStart,
                                                   uint8_t rxBufStart,
                                                   uint16_t length) {
  registeredLogs[numRegisteredLogs].startingRXBuf = rxBufStart;
  registeredLogs[numRegisteredLogs].endingRXBuf =
      rxBufStart + (length - 1) / 64;
  registeredLogs[numRegisteredLogs].byteLength = length;
  registeredLogs[numRegisteredLogs].startingMsgID = msgIDStart;
  for (uint16_t i = 0; i < ((length - 1) / 64 + 1); i++) {
    HAL_StatusTypeDef stat =
        RegisterFilterRXBuf(msgIDStart + i, rxBufStart + i);
    if (stat != HAL_OK) {
      return stat;
    }
  }
  numRegisteredLogs++;
  return HAL_OK;
}

/* @brief FDCAN frames can only be certain size values between 0 and 64.
 * 		  Returns next-highest valid FDCAN size for a given byte length.
 * @param unroundedlen Byte length to be rounded.
 * @return Total byte size representable by series of valid FDCAN frame sizes.
 */
const uint16_t FDCanController::FDRoundDataSize(uint16_t unroundedLen) {
  uint8_t mod = (unroundedLen - 1) % 64 + 1;

  // Round up remainder
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
 * @param logID Output of the index of the log.
 * @return Size in bytes of received message. If no message, returns 0 and out
 * is unmodified.
 */
uint16_t FDCanController::ReceiveFirstLogFromRXBuf(uint8_t *out,
                                                   uint16_t *logID) {
  if (!RXFlag) {
    return 0;
  }
  for (uint8_t i = 0; i < numRegisteredLogs; i++) {
    uint16_t len = ReceiveLogTypeFromRXBuf(out, i);
    if (len > 0) {
      // data now contains entire multi-frame log, potentially padded with
      // some 0s
      // Received full log, do send or something
      *logID = i;
      return len;
    }
  }
  RXFlag = false;
  return 0;
}

/* Retrieves a log of a given type.
 * @param out Output array the data will be put into.
 * Must be big enough for the log type, rounded up to the nearest 64 bytes!
 * Excess space in the FDCAN frame the log doesn't fill will be padded with zeros.
 * @param logIndexFilter The log index to read.
 * @return The size of the data returned in bytes, or zero if no log was read.
 */
uint16_t FDCanController::ReceiveLogTypeFromRXBuf(uint8_t *out,
                                                  uint16_t logIndexFilter) {
  if (!RXFlag) {
    return 0;
  }

  const LogRegister &thisRegisteredLog = registeredLogs[logIndexFilter];
  if (thisRegisteredLog.byteLength == 0) {
    return 0;
  }

  if (buffers[thisRegisteredLog.endingRXBuf].available) {
    if (readingRXBufSemaphore) {
      return 0;  // something else using the semaphore
    }
    readingRXBufSemaphore = true;

    // Received full log
    uint8_t *d = out;
    for (uint8_t b = thisRegisteredLog.startingRXBuf;
         b <= thisRegisteredLog.endingRXBuf; b++) {
      memcpy(d,buffers[b].data,64);
      buffers[b].available = false;
      d += 64;
    }

    readingRXBufSemaphore = false;
    // data now contains entire multi-frame log, potentially padded with
    // some 0s
    // Received full log, do send or something
    return thisRegisteredLog.byteLength;
  }

  return 0;
}

/* @brief Sends a log message to a registered buffer.
 * @param msg Pointer to data to send. Must be large enough to hold the target
 * log's size.
 * @param logIndex Index of the log to send to.
 */
bool FDCanController::SendByLogIndex(const uint8_t *msg, uint16_t logIndex) {
  return SendByMsgID(msg, registeredLogs[logIndex].byteLength,
                     registeredLogs[logIndex].startingMsgID);
}

/* @brief Registers log types to the driver so that they can be sent and received.
 * @param logs Pointer to an array of log initialization structs.
 * @param numLogs Number of elements in logs.
 * @return HAL_OK if successful
 */
HAL_StatusTypeDef FDCanController::RegisterLogs(LogInitStruct *logs, uint16_t numLogs) {

	  for (uint16_t i = 0; i < numLogs; i++) {
	    numFDFilters += (logs[i].byteLength - 1) / 64 + 1;
	  }

	  fdcan->Init.StdFiltersNbr = numFDFilters;

	  if (HAL_FDCAN_Init(fdcan) != HAL_OK) {
	    CANError();
	  }

	  for (uint16_t i = 0; i < numLogs; i++) {
	    if (RegisterLogType(logs[i].startingMsgID, nextUnregisteredFilterID,
	                        logs[i].byteLength) != HAL_OK) {
	      CANError();
	    }
	  }

	  return HAL_FDCAN_Start(fdcan);
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

/* @brief Gets a message from the RX FIFO.
 * @param out The data output. Must be at least 64 bytes long.
 * @param msgIDOut Where the CAN ID of the message will be stored.
 * @return HAL_OK on success.
 */
HAL_StatusTypeDef FDCanController::GetRxFIFO(uint8_t *out, uint32_t *msgIDOut) {
	if(HAL_FDCAN_GetRxFifoFillLevel(fdcan, 0) == 0) {
		return HAL_ERROR;
	}
	FDCAN_RxHeaderTypeDef header;
	HAL_FDCAN_GetRxMessage(fdcan, FDCAN_RX_FIFO0, &header, out);
	*msgIDOut = header.Identifier;
	return HAL_OK;
}

/* @brief Initializes and starts the FDCAN peripheral.
 * @return HAL_OK on success.
 */
HAL_StatusTypeDef FDCanController::InitFDCAN() {
	HAL_StatusTypeDef stat;
	if (HAL_FDCAN_ConfigGlobalFilter(fdcan, FDCAN_REJECT, FDCAN_REJECT,
	                                   FDCAN_REJECT_REMOTE,
	                                   FDCAN_REJECT_REMOTE) != HAL_OK) {
	    CANError();
	  }

	  // Turn on callback for receiving msg
	  stat =
	      HAL_FDCAN_ActivateNotification(fdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
	  if (stat != HAL_OK) {
	    CANError();
	  }

	  stat =HAL_FDCAN_ActivateNotification(fdcan,
	                                 FDCAN_IT_TX_COMPLETE,
	                                 0);
	  if (stat != HAL_OK) {
	    CANError();
	  }


	  stat = HAL_FDCAN_Start(fdcan);
	  if (stat != HAL_OK) {
	    CANError();
	  }
	  return stat;
}
