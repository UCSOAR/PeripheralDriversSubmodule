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

#define AIUDJAISDFJAEIOAOISDHLAFHA

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
	if(logIndex >= numRegisteredLogs) {
		return false;
	}
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
bool FDCanController::SendByMsgID(const uint8_t *msg, size_t len, uint16_t ID, uint16_t timeout) {
  size_t framesToSend = (len - 1) / 64 + 1;

  if(framesToSend > 2048) {
	  return false;
  }
  if (ID >= 2048 - framesToSend) {
    // ID too large
    return false;
  }

  FDCAN_TxHeaderTypeDef txheader = {0};
  txheader.IdType = FDCAN_STANDARD_ID;
  txheader.TxFrameType = FDCAN_DATA_FRAME;
  txheader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;

  if(fdcan->Init.FrameFormat == FDCAN_FRAME_CLASSIC) {
	  txheader.BitRateSwitch = FDCAN_BRS_OFF;
	  txheader.FDFormat = FDCAN_CLASSIC_CAN;
  } else {
	  txheader.BitRateSwitch = FDCAN_BRS_ON;
	  txheader.FDFormat = FDCAN_FD_CAN;
  }

  txheader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  txheader.MessageMarker = 0;
  txheader.DataLength = FDCAN_DLC_BYTES_64;

  uint8_t data[64];
  size_t remaining = len;
  for (size_t frame = 0; frame < framesToSend; frame++) {
	  size_t frameLen = remaining > 64 ? 64 : remaining;
    txheader.Identifier = ID + frame;

    // Length will be 64 bytes, except for the final frame,
    // which will be minimum required for the remaining data
    if (frame == framesToSend - 1) txheader.DataLength = FDGetModDLC(frameLen);



    memcpy(data, msg + 64 * frame, frameLen);

    // If data won't fill the whole buffer, fill buf with zeros.
    if (frameLen < 64) {
      memset(data + frameLen, 0x00, 64 - frameLen);
    }
    remaining -= frameLen;

    uint32_t starting = HAL_GetTick();
    while (HAL_FDCAN_GetTxFifoFreeLevel(fdcan) < 1) {
    	if(HAL_GetTick() - starting > timeout) {
    		return false;
    	}
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
FDCanController::RXBuffer* FDCanController::GetRXBuf(uint16_t index, FDCanController::SelectedBuffer side) {
	if(index >= MAX_FDCAN_RX_BUFFERS) {
		return nullptr;
	}

	if(side == FDCanController::SelectedBuffer::Buf_A) {
		return &buffersA[index];
	}

	return &buffersB[index];
}

/* @brief Gets the pointer to the RX buffer that contains the given CAN ID in the log registered to it.
 * @param canid The CAN ID to look for
 * @return Pointer to the RX buffer that a registered log maps the ID to. nullptr if none found.
 */
FDCanController::RXBuffer* FDCanController::GetFrontBufferFromCanID(uint16_t canid) {
//	for (uint16_t i = 0; i < numRegisteredLogs; i++) {
//		const LogRegister& thisReg = registeredLogs[i];
//
//		if(thisReg.startingMsgID <= canid && thisReg.startingMsgID + ((thisReg.byteLength-1)/64) >= canid) {
//
//			return GetRXBuf(thisReg.startingRXBuf+(canid-thisReg.startingMsgID),selectedBufsForLog[i]);
//		}
//	}
//	return nullptr;
	if(buffersByCanID[canid].selected == Buf_A) {
		return buffersByCanID[canid].A;
	}
	return buffersByCanID[canid].B;
}

/* @brief Gets the pointer to the RX buffer that contains the given CAN ID in the log registered to it.
 * @param canid The CAN ID to look for
 * @return Pointer to the RX buffer that a registered log maps the ID to. nullptr if none found.
 */
FDCanController::RXBuffer* FDCanController::GetBackBufferFromCanID(uint16_t canid) {
//	for (uint16_t i = 0; i < numRegisteredLogs; i++) {
//		const LogRegister& thisReg = registeredLogs[i];
//
//		if(thisReg.startingMsgID <= canid && thisReg.startingMsgID + ((thisReg.byteLength-1)/64) >= canid) {
//
//			return GetRXBuf(thisReg.startingRXBuf+(canid-thisReg.startingMsgID),
//					(selectedBufsForLog[i] == FDCanController::SelectedBuffer::Buf_A) ? FDCanController::SelectedBuffer::Buf_B : FDCanController::SelectedBuffer::Buf_A);
//		}
//	}
//	return nullptr;
	if(buffersByCanID[canid].selected == Buf_A) {
		return buffersByCanID[canid].B;
	}
	return buffersByCanID[canid].A;
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


		while(HAL_FDCAN_GetRxMessage(hfdcan,
				FDCAN_RX_FIFO0,
				&rxHeader,
				buf) == HAL_OK){
			if(callbackcontroller) {
#ifdef AIUDJAISDFJAEIOAOISDHLAFHA
//printf("got can msg id %lu\n",rxHeader.Identifier);
#endif

				FDCanController::RXBuffer* rxbuf = callbackcontroller->GetBackBufferFromCanID(rxHeader.Identifier);
				if(rxbuf) {
#ifdef AIUDJAISDFJAEIOAOISDHLAFHA
					//printf("putting in back rxbuf %p\n",((void*)rxbuf));
#endif
					memcpy(rxbuf->data,buf,FDCanController::FDGetByteLengthOfDLC(rxHeader.DataLength));
					rxbuf->available = true;


					callbackcontroller->RaiseFXFlag();
				}
			}
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

  for(uint8_t i = 0; i < MAX_FDCAN_LOGS; i++) {
	  selectedBufsForLog[i] = Buf_A;
  }
  RegisterLogs(logs, numLogs);

  callbackcontroller = this;
}

/* @brief Registers a filter that directs an FDCAN message ID to the FIFO, which
 * will later be directed in the callback to a certain RX
 * buffer. Filter will be placed in next available filter slot. Must ensure
 * there are enough Std Filters configured in the FDCAN parameter settings.
 * @param msgID FDCAN message ID to direct.
 * @return HAL_OK on success.
 */
HAL_StatusTypeDef FDCanController::RegisterFilterRXBuf(uint16_t startingID, uint16_t endingID) {
  if (nextUnregisteredFilterID >= numFDFilters) {
    return HAL_ERROR;
  }
  FDCAN_FilterTypeDef filter;

  filter.IdType = FDCAN_STANDARD_ID;
  filter.FilterType = FDCAN_FILTER_RANGE;
  filter.FilterIndex = nextUnregisteredFilterID;
  filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  filter.FilterID1 = startingID;
  filter.FilterID2 = endingID;

  nextUnregisteredFilterID++;
  return HAL_FDCAN_ConfigFilter(fdcan, &filter);
}


/* @brief FDCAN frames can only be certain size values between 0 and 64.
 * 		  Returns next-highest valid FDCAN size for a given byte length.
 * @param unroundedlen Byte length to be rounded.
 * @return Total byte size representable by series of valid FDCAN frame sizes.
 */
uint16_t FDCanController::FDRoundDataSize(uint16_t unroundedLen) {
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
uint32_t FDCanController::FDGetModDLC(uint16_t unroundedLen) {
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

/* @brief Returns the length in bytes corresponding to a given FDCAN DLC value.
 * @param DLC One of FDCAN_DLC_BYTES_...
 * @return The number of bytes corresponding to this DLC value.
 */
uint16_t FDCanController::FDGetByteLengthOfDLC(uint16_t DLC) {
	if(DLC <= FDCAN_DLC_BYTES_8) {
		return DLC;
	}
	switch(DLC) {
	case FDCAN_DLC_BYTES_12:
		return 12;
	case FDCAN_DLC_BYTES_16:
		return 16;
	case FDCAN_DLC_BYTES_20:
		return 20;
	case FDCAN_DLC_BYTES_24:
		return 24;
	case FDCAN_DLC_BYTES_32:
		return 32;
	case FDCAN_DLC_BYTES_48:
		return 48;
	case FDCAN_DLC_BYTES_64:
		return 64;
	}
	return 0;
}

/* @brief Raises the RX flag for this controller letting it know that there is at least
 * one incoming message.
 */
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
    uint16_t len = ReceiveLogIndexFromRXBuf(out, i);
    if (len > 0) {
      // data now contains entire multi-frame log, potentially padded with
      // some 0s
      // Received full log
      *logID = i;
      return len;
    }
  }
  // Found nothing
  RXFlag = false;
  return 0;
}


/* Retrieves a log of a given index.
 * @param out Output array the data will be put into.
 * Must be big enough for the log type, rounded up to the nearest 64 bytes! (use FDRoundDataSize)
 * Excess space in the FDCAN frame the log doesn't fill will be padded with zeros.
 * @param logIndexFilter The log index to read.
 * @return The size of the data returned in bytes, or zero if no log was read.
 */
uint16_t FDCanController::ReceiveLogIndexFromRXBuf(uint8_t *out, uint16_t logIndex) {
  if (!RXFlag || logIndex >= numRegisteredLogs)  {
#ifdef AIUDJAISDFJAEIOAOISDHLAFHA

	  printf("didn't receive. rxflag: %d, logindex: %d\n",RXFlag,logIndex);

#endif
	  return 0;
  }

  const LogRegister &thisRegisteredLog = registeredLogs[logIndex];
  if (thisRegisteredLog.byteLength == 0)  {
#ifdef AIUDJAISDFJAEIOAOISDHLAFHA
	  printf("didn't receive. bytelength was zero\n");
#endif
	  return 0;
  }


  SelectedBuffer currentFront = selectedBufsForLog[logIndex];
  SelectedBuffer currentBack = (currentFront == Buf_A ? Buf_B : Buf_A);
  RXBuffer* backbuf = (currentBack == Buf_A) ? buffersA : buffersB;

  // check if all ready
  __disable_irq();
  bool allReady = true;
  for(uint8_t b = thisRegisteredLog.startingRXBuf; b <= thisRegisteredLog.endingRXBuf; b++) {
    if(!backbuf[b].available) {
#ifdef AIUDJAISDFJAEIOAOISDHLAFHA
    	printf("buffer %d of %d/%d wasnt ready, waiting for canid %d\n",b,thisRegisteredLog.startingRXBuf,thisRegisteredLog.endingRXBuf, thisRegisteredLog.startingMsgID+b-thisRegisteredLog.startingRXBuf);
#endif
      allReady = false;
      break;
    }
  }

  if(!allReady) {
    __enable_irq();
#ifdef AIUDJAISDFJAEIOAOISDHLAFHA

    //printf("didn't receive, wasnt all ready in log %d. frontbuf: %d, backbuf: %d\n",logIndex, currentFront, currentBack);

#endif
    return 0;
  }

  // swap!!!!!!!
  selectedBufsForLog[logIndex] = currentBack;
  for(uint16_t i = thisRegisteredLog.startingRXBuf; i <= thisRegisteredLog.endingRXBuf; i++) {
	  buffersByCanID[thisRegisteredLog.startingMsgID+i-thisRegisteredLog.startingRXBuf].selected = currentBack;
  }
  __enable_irq();

  uint8_t *d = out;
  for (uint8_t b = thisRegisteredLog.startingRXBuf; b <= thisRegisteredLog.endingRXBuf; b++) {
    uint8_t copyLen = (b == thisRegisteredLog.endingRXBuf)
                      ? ((thisRegisteredLog.byteLength - 1) % 64 + 1)
                      : 64;
    memcpy(d, backbuf[b].data, copyLen);

    // Mark as consumed so it can be used again when the ISR eventually swaps back
    backbuf[b].available = false;
    d += 64;
  }

  return thisRegisteredLog.byteLength;
}

/* @brief Sends a log message to a registered buffer.
 * @param msg Pointer to data to send. Must be large enough to hold the target
 * log's size.
 * @param logIndex Index of the log to send to.
 */
bool FDCanController::SendByLogIndex(const uint8_t *msg, uint16_t logIndex) {
	if(logIndex >= numRegisteredLogs) {
		return false;
	}
  return SendByMsgID(msg, registeredLogs[logIndex].byteLength,
                     registeredLogs[logIndex].startingMsgID);
}

/* @brief Registers log types to the driver so that they can be sent and received.
 * @param logs Pointer to an array of log initialization structs.
 * @param numLogs Number of elements in logs.
 * @return HAL_OK if successful
 */
HAL_StatusTypeDef FDCanController::RegisterLogs(LogInitStruct *logs, uint16_t numLogs) {

	  for(uint16_t i = 0; i < numLogs; i++) {
		  if(!AddLogType(logs[i])) {
			  return HAL_ERROR;
		  }
	  }
	  return HAL_OK;
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

/* @brief Registers a log index and rebuilds filters.
 * @param log The log to register.
 * @return true on success.
 *
 */
bool FDCanController::AddLogType(LogInitStruct log) {
	if(numRegisteredLogs >= MAX_FDCAN_LOGS) {
		return false;
	}

	registeredLogs[numRegisteredLogs].startingMsgID = log.startingMsgID;
	registeredLogs[numRegisteredLogs].byteLength = log.byteLength;

	numRegisteredLogs++;

	return RebuildFilters();
}

/* @brief Rebuilds the FDCAN peripheral filters and restarts it.
 * @return true on success.
 */
bool FDCanController::RebuildFilters() {
	numFDFilters = numRegisteredLogs;
	memset(buffersByCanID,0x00,sizeof(buffersByCanID));


	fdcan->Init.StdFiltersNbr = numFDFilters;

	if(HAL_FDCAN_DeInit(fdcan) != HAL_OK) {
		CANError();
	}
	if(HAL_FDCAN_Init(fdcan) != HAL_OK) {
		CANError();
	}
	nextUnregisteredFilterID = 0;
	uint8_t nextRXBuf = 0;

	for(uint8_t i = 0; i < numRegisteredLogs; i++) {
		uint16_t msgID = registeredLogs[i].startingMsgID;
		uint16_t len = registeredLogs[i].byteLength;

		uint8_t frames = (len-1)/64+1;

		registeredLogs[i].startingRXBuf = nextRXBuf;
		registeredLogs[i].endingRXBuf = nextRXBuf+frames-1;

		for(uint16_t canid = msgID; canid <= msgID+frames-1; canid++) {
			buffersByCanID[canid].A = &buffersA[registeredLogs[i].startingRXBuf + canid - msgID];
			buffersByCanID[canid].B = &buffersB[registeredLogs[i].startingRXBuf + canid - msgID];
		}


		nextRXBuf += frames;
		if(nextRXBuf > MAX_FDCAN_RX_BUFFERS) {
			return false;
		}

		if(RegisterFilterRXBuf(msgID, msgID+frames-1) != HAL_OK) {
			return false;
		}

	}
	 HAL_FDCAN_ConfigGlobalFilter(fdcan, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);
	 HAL_FDCAN_ActivateNotification(fdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
	 HAL_FDCAN_ActivateNotification(fdcan, FDCAN_IT_TX_COMPLETE, 0);

	 return HAL_FDCAN_Start(fdcan) == HAL_OK;

}
