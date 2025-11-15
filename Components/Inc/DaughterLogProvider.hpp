/*
 * DaughterLogProvider.hpp
 *
 *  Created on: Sep 3, 2025
 *      Author: goada
 */

#ifndef INC_DAUGHTERLOGPROVIDER_HPP_
#define INC_DAUGHTERLOGPROVIDER_HPP_

#include <stdint.h>
#include <CanAutoNodeDaughter.hpp>
#include <MessageInterface.h>
#include <SensorData.h>
#include <WriteBufferFixedSize.h>
#include <cobs.h>
#include <StructToProto.hpp>

/* A function definition that fulfills this signature must:
 * 	-read log data from flash (or some other buffer) and write it to the array pointed to by dataArrayOut
 * 	-start reading data beginning at offsetInBytes in whatever source it is from
 * 	-output no more than outLen bytes into the buffer
 * 	-return the number of bytes actually successfully read
 */
typedef uint16_t (*GetData)(uint16_t offsetInBytes, uint8_t* dataArrayOut, uint16_t outLen);

#define TEMPLATE_PROTOBUF_MSG_ONLY 	template<typename T, typename = std::enable_if_t<std::is_base_of<EmbeddedProto::MessageInterface, T>::value>>

class WriteBufferRuntimeSize : public EmbeddedProto::WriteBufferInterface {
public:
	WriteBufferRuntimeSize(const uint16_t max_size);
	~WriteBufferRuntimeSize();
	void clear() override;
	uint32_t get_size() const override;
	uint32_t get_max_size() const override;
	uint32_t get_available_size() const override;
	bool push(const uint8_t byte) override;
	bool push(const uint8_t* bytes, const uint32_t length) override;
	const uint8_t* get_data() const;
private:
	const uint16_t max_size;
	uint16_t num_data = 0;
	uint8_t* data = nullptr;
};

class DaughterLogProvider {
public:
	DaughterLogProvider(GetData dataGetter, CanAutoNodeDaughter* autonode);
	~DaughterLogProvider();
	DaughterLogProvider(const DaughterLogProvider &other) = delete;
	DaughterLogProvider(DaughterLogProvider &&other) = delete;
	DaughterLogProvider& operator=(const DaughterLogProvider &other) = delete;
	DaughterLogProvider& operator=(DaughterLogProvider &&other) = delete;


	TEMPLATE_PROTOBUF_MSG_ONLY
	static uint16_t GetMaxSizeOfProtoWrappedMsg(const T& msg) {
		EmbeddedProto::MessageSizeCalculator calc;
		msg.serialize(calc);
		return msg.serialized_size();
	}

	static uint16_t GetMaxSizeOfCobsWrapped(uint16_t len) {
		return len+(len+253)/254;
	}

	TEMPLATE_PROTOBUF_MSG_ONLY
	static T RawToProtoMsg(const void* e) {

	}


	/* Send a number of logs to the motherboard over CAN by repeatedly calling the data getter function specified on instantiation.
	 * @param numToSend Number of logs to send. Set to 0 to send until the first time the getter function fails.
	 * @param logIndex The index of the log to send.
	 * @param delayMs The number of milliseconds to delay between each log send.
	 * @return true if all logs sent successfully, or at least one if numToSend is zero.
	 */
	TEMPLATE_PROTOBUF_MSG_ONLY
	bool SendLogs(uint32_t numToSend, uint16_t delayMs) {

		uint16_t logIndex = 1; // TODO: GET FROM PROTO MSG ENUM WAITING ON JAD!!!!!!!!!!!!!!!!!!!
		uint16_t numSent = 0;
		uint16_t partialBytesRead = 0;
		uint16_t rawsize = autonode->GetSizeOfLog(logIndex);

		while(numSent < numToSend || numToSend == 0) {
			uint8_t rawdata[rawsize];
			memset(rawdata,0,sizeof(rawdata));

			partialBytesRead += getter(numSent*rawsize+partialBytesRead,rawdata+partialBytesRead,rawsize-partialBytesRead);																																																												//
			if(partialBytesRead == 0 && numToSend == 0) {
				return numSent > 0;
			}

			if(partialBytesRead > rawsize) {
				// ????????????????????
				return false;
			}
			if(partialBytesRead == rawsize) {
				partialBytesRead = 0;

				T msg = StructToProtoMsg<T>(rawdata);

				//start protoing yay
				const uint16_t size = GetMaxSizeOfProtoWrappedMsg<T>(msg);
				WriteBufferRuntimeSize protodata = {size};

				msg.serialize(protodata);

				uint8_t cobs_out[autonode->GetSizeOfLog(logIndex)];

				cobs_encode_result cobsres = cobs_encode(cobs_out, sizeof(cobs_out), protodata.get_data(), protodata.get_size());

				if(cobsres.status != COBS_ENCODE_OK) {
					return false;
				}
				memset(cobs_out+cobsres.out_len,0x00,sizeof(cobs_out)-cobsres.out_len);

				bool sent = autonode->SendMessageToMotherboardByLogID(logIndex, cobs_out);
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

private:

	CanAutoNodeDaughter* autonode;

	GetData getter;

};

#endif /* INC_DAUGHTERLOGPROVIDER_HPP_ */
