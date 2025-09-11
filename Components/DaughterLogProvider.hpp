/*
 * DaughterLogProvider.hpp
 *
 *  Created on: Sep 3, 2025
 *      Author: Local user
 */

#ifndef INC_DAUGHTERLOGPROVIDER_HPP_
#define INC_DAUGHTERLOGPROVIDER_HPP_

#include <stdint.h>
#include <CanAutoNodeDaughter.hpp>

/* A function definition that fulfills this signature must:
 * 	-read log data from flash (or some other buffer) and write it to the array pointed to by dataArrayOut
 * 	-start reading data beginning at offsetInBytes in whatever source it is from
 * 	-output no more than outLen bytes into the buffer
 * 	-return the number of bytes actually successfully read
 */
typedef uint16_t (*GetData)(uint16_t offsetInBytes, uint8_t* dataArrayOut, uint16_t outLen);

class DaughterLogProvider {
public:
	DaughterLogProvider(GetData dataGetter, CanAutoNodeDaughter* autonode);
	~DaughterLogProvider();
	DaughterLogProvider(const DaughterLogProvider &other) = delete;
	DaughterLogProvider(DaughterLogProvider &&other) = delete;
	DaughterLogProvider& operator=(const DaughterLogProvider &other) = delete;
	DaughterLogProvider& operator=(DaughterLogProvider &&other) = delete;
	DaughterLogProvider() = delete;

	bool SendLogs(uint32_t numToSend, uint8_t logIndex, uint16_t delayMs);



private:


	CanAutoNodeDaughter* autonode;

	GetData getter;

};

#endif /* INC_DAUGHTERLOGPROVIDER_HPP_ */
