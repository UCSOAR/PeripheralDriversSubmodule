/*
 * DaughterLogProvider.cpp
 *
 *  Created on: Sep 3, 2025
 *      Author: Adam Godin
 */

#include <DaughterLogProvider.hpp>

DaughterLogProvider::DaughterLogProvider(GetData getter, CanAutoNodeDaughter* autonode) {
	this->getter = getter;
	this->autonode = autonode;

}

DaughterLogProvider::~DaughterLogProvider() {

}

/* Send a number of logs to the motherboard over CAN by repeatedly calling the data getter function specified on instantiation.
 * @param numToSend Number of logs to send. Set to 0 to send until the first time the getter function fails.
 * @param logIndex The index of the log to send.
 * @param delayMs The number of milliseconds to delay between each log send.
 * @return true if all logs sent successfully, or at least one if numToSend is zero.
 */
bool DaughterLogProvider::SendLogs(uint32_t numToSend, uint8_t logIndex, uint16_t delayMs) {

	uint16_t numSent = 0;
	uint16_t partialBytesRead = 0;
	const uint16_t size = autonode->GetSizeOfLog(logIndex);
	while(numSent < numToSend || numToSend == 0) {
		uint8_t data[size];

		partialBytesRead += getter(numSent*size+partialBytesRead,data+partialBytesRead,size-partialBytesRead);
		if(partialBytesRead == 0 && numToSend == 0) {
			return numSent > 0;
		}

		if(partialBytesRead > size) {
			// ????????????????????
			return false;
		}
		if(partialBytesRead == size) {
			partialBytesRead = 0;
			bool sent = autonode->SendMessageToMotherboardByLogID(logIndex, data);
			if(sent) {
				numSent++;
				if(delayMs > 0) {
					HAL_Delay(delayMs);
				}
			} else {
				return false;
			}
		}

	}

	return true;

}
