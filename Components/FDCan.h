/*
 * FDCan.h
 *
 *  Created on: Aug 24, 2024
 *      Author: goada
 */

#ifndef FDCAN_H_
#define FDCAN_H_

#include "stm32g4xx.h"

constexpr size_t MAX_FDCAN_RX_BUFFERS = 128;
constexpr size_t MAX_FDCAN_LOGS = 8;


class FDCanController {
 public:
  struct LogInitStruct {
    uint16_t byteLength = 0;
    uint16_t startingMsgID = 0;
  };

  FDCanController(FDCAN_HandleTypeDef *fdcan,
                  FDCanController::LogInitStruct *logs, uint16_t numLogs);
  ~FDCanController();
  FDCanController(const FDCanController &) = delete;
  FDCanController &operator=(const FDCanController &) = delete;

  bool SendByLogIndex(const uint8_t *msg, uint16_t logIndex);

  bool SendStringByLogIndex(const char *msg, uint16_t logIndex);

  bool SendByMsgID(const uint8_t *msg, size_t len, uint16_t ID, uint16_t timeout = 1000);

  bool AddLogType(LogInitStruct log);

  HAL_StatusTypeDef RegisterLogs(LogInitStruct *logs, uint16_t numLogs);

  uint16_t ReceiveFirstLogFromRXBuf(uint8_t *out, uint16_t *msgID);
  uint16_t ReceiveLogIndexFromRXBuf(uint8_t *out, uint16_t logIndexFilter);

  static uint16_t FDRoundDataSize(uint16_t unroundedLen);
  static uint32_t FDGetModDLC(uint16_t unroundedLen);
  static uint16_t FDGetByteLengthOfDLC(uint16_t DLC);

  inline void RaiseFXFlag();

  HAL_StatusTypeDef GetRxFIFO(uint8_t* out, uint32_t* msgIDOut);

  struct RXBuffer {
	  uint8_t data[64];
	  volatile bool available = false;
  };

  enum SelectedBuffer {
	  Buf_A,
	  Buf_B
  };
  SelectedBuffer selectedBufsForLog[MAX_FDCAN_LOGS];

  RXBuffer* GetRXBuf(uint16_t index,SelectedBuffer side);

  RXBuffer* GetFrontBufferFromCanID(uint16_t canid);
  RXBuffer* GetBackBufferFromCanID(uint16_t canid);
 protected:
  struct LogRegister {
    uint8_t startingRXBuf = 0;
    uint8_t endingRXBuf = 0;
    uint16_t byteLength = 0;
    uint16_t startingMsgID = 0;
  };

  bool RebuildFilters();

  RXBuffer buffersA[MAX_FDCAN_RX_BUFFERS];
  RXBuffer buffersB[MAX_FDCAN_RX_BUFFERS];

  struct BufferTracker {
	  RXBuffer* A;
	  RXBuffer* B;
	  SelectedBuffer selected;
  };

  BufferTracker buffersByCanID[2048];



  FDCAN_HandleTypeDef *fdcan;

  uint8_t numFDFilters = 0;

  uint8_t nextUnregisteredFilterID = 0;
  uint8_t numRegisteredLogs = 0;
  LogRegister registeredLogs[MAX_FDCAN_LOGS];

  HAL_StatusTypeDef RegisterFilterRXBuf(uint16_t startingID, uint16_t endingID);

  // Raised when there is at least 1 rx msg
  volatile bool RXFlag = false;

  volatile bool readingRXBufSemaphore = false;
};

extern FDCanController *callbackcontroller;

void CANError();

#endif /* FDCAN_H_ */
