/*
 * DaughterLogProvider.cpp
 *
 *  Created on: Sep 3, 2025
 *      Author: Adam Godin
 */

#include <DaughterLogProvider.hpp>
#include <SoarProto/cobs.h>
#include <SoarProto/EmbeddedProto/src/MessageSizeCalculator.h>
#include <SensorData.h>

DaughterLogProvider::DaughterLogProvider(GetData getter, CanAutoNodeDaughter* autonode) {
	this->getter = getter;
	this->autonode = autonode;

}

DaughterLogProvider::~DaughterLogProvider() {

}

void WriteBufferRuntimeSize::clear() {
	num_data = 0;

}

uint32_t WriteBufferRuntimeSize::get_size() const {
	return num_data;
}

uint32_t WriteBufferRuntimeSize::get_max_size() const {
	return max_size;
}

uint32_t WriteBufferRuntimeSize::get_available_size() const {
	if(num_data > max_size) {
		return 0;
	}
	return max_size - num_data;
}

bool WriteBufferRuntimeSize::push(const uint8_t byte) {
	if(num_data <= max_size) {
		data[num_data++] = byte;
		return true;
	}
	return false;
}

bool WriteBufferRuntimeSize::push(const uint8_t *bytes, const uint32_t length) {
	if(num_data+length <= max_size) {
		memcpy(data+num_data,bytes,length);
		num_data += length;
		return true;
	}
	return false;

}

WriteBufferRuntimeSize::WriteBufferRuntimeSize(const uint16_t max_size) : max_size(max_size) {
	this->data = new uint8_t[max_size];
}

WriteBufferRuntimeSize::~WriteBufferRuntimeSize() {
	free(data);
}

const uint8_t* WriteBufferRuntimeSize::get_data() const {
	return data;
}
