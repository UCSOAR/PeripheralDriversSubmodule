/*
 * FDCan.hpp
 *
 *  Created on: Aug 24, 2024
 *      Author: goada
 */

#ifndef FDCAN_H_
#define FDCAN_H_

#include "stm32h7xx.h"

constexpr size_t MAX_FDCAN_RX_BUFFERS = 64;

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

  bool SendByLogIndex(const char *msg, uint32_t logIndex);

  bool SendByMsgID(const uint8_t *msg, size_t len, uint32_t ID);

  uint16_t ReceiveFromRXBuf(uint8_t *out, uint16_t *msgID);

  HAL_StatusTypeDef RegisterLogType(uint16_t msgIDStart, uint8_t rxBufStart,
                                    uint16_t length);

  static const uint16_t FDRoundDataSize(uint16_t unroundedLen);
  static const uint32_t FDGetModDLC(uint16_t unroundedLen);

  inline void RaiseFXFlag();

 protected:
  struct LogRegister {
    uint8_t startingRXBuf = 0;
    uint8_t endingRXBuf = 0;
    uint16_t byteLength = 0;
    uint16_t startingMsgID = 0;
  };

  FDCAN_HandleTypeDef *fdcan;

  uint8_t numFDFilters = 0;

  uint8_t nextUnregisteredFilterID = 0;
  uint8_t numRegisteredLogs = 0;
  LogRegister registeredLogs[MAX_FDCAN_RX_BUFFERS];

  HAL_StatusTypeDef RegisterFilterRXBuf(uint16_t msgID, uint8_t rxBufferNum);
  HAL_StatusTypeDef RegisterFilterRXFIFO(uint16_t msgIDMin, uint16_t msgIDMax);

  volatile bool RXFlag = false;
};

extern FDCanController *callbackcontroller;
void RXMsgCallback(FDCAN_HandleTypeDef *hfdcan);

void CANError();

#endif /* FDCAN_H_ */
