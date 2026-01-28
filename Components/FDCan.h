/*
 * FDCan.hpp
 *
 *  Created on: Aug 24, 2024
 *      Author: goada
 */

#ifndef FDCAN_H_
#define FDCAN_H_

#include "stm32g4xx.h"

constexpr size_t MAX_FDCAN_RX_BUFFERS = 128;

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

  bool SendByMsgID(const uint8_t *msg, size_t len, uint16_t ID);

  HAL_StatusTypeDef RegisterLogs(LogInitStruct *logs, uint16_t numLogs);

  uint16_t ReceiveFirstLogFromRXBuf(uint8_t *out, uint16_t *msgID);
  uint16_t ReceiveLogTypeFromRXBuf(uint8_t *out, uint16_t logIndexFilter);

  HAL_StatusTypeDef RegisterLogType(uint16_t msgIDStart, uint8_t rxBufStart,
                                    uint16_t length);

  static const uint16_t FDRoundDataSize(uint16_t unroundedLen);
  static const uint32_t FDGetModDLC(uint16_t unroundedLen);

  inline void RaiseFXFlag();

  HAL_StatusTypeDef GetRxFIFO(uint8_t* out, uint32_t* msgIDOut);
  HAL_StatusTypeDef RegisterFilterRXFIFO(uint16_t msgIDMin, uint16_t msgIDMax);

  struct RXBuffer {
	  uint8_t data[64];
	  bool available = false;
  };


  RXBuffer* GetRXBuf(uint16_t index);

  RXBuffer* GetBufferFromCanID(uint16_t canid);

 protected:
  struct LogRegister {
    uint8_t startingRXBuf = 0;
    uint8_t endingRXBuf = 0;
    uint16_t byteLength = 0;
    uint16_t startingMsgID = 0;
  };
  RXBuffer buffers[MAX_FDCAN_RX_BUFFERS];

  FDCAN_HandleTypeDef *fdcan;

  uint8_t numFDFilters = 0;

  uint8_t nextUnregisteredFilterID = 0;
  uint8_t numRegisteredLogs = 0;
  LogRegister registeredLogs[MAX_FDCAN_RX_BUFFERS];

  HAL_StatusTypeDef RegisterFilterRXBuf(uint16_t msgID, uint8_t rxBufferNum);

  HAL_StatusTypeDef InitFDCAN();

  volatile bool RXFlag = false;

  bool readingRXBufSemaphore = false;
};

extern FDCanController *callbackcontroller;
void RXMsgCallback(FDCAN_HandleTypeDef *hfdcan);

void CANError();

#endif /* FDCAN_H_ */
