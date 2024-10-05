/*
 * IMUDriver.cpp
 *
 *  Created on: Aug 31, 2024
 *      Author: goada
 */

#include <ICM20948_Driver.h>

/* @brief Initialize the driver. Must be called before any other functions can be used.
 * @param hspi_ Pointer to the SPI handle
 * @param cs_gpio_ GPIO port for the chip select pin (GPIOA, GPIOB ...)
 * @param cs_pin_ Pin number for the chip select
 */
void IMUDriver::Init(SPI_HandleTypeDef* hspi_, GPIO_TypeDef* cs_gpio_, uint16_t cs_pin_) {
	hspi = hspi_;
	initialized = true;
	SetCSPin(cs_gpio_, cs_pin_);
	CSHigh();

	uint8_t ID = GetRegister(ICM20948_REG::WHO_AM_I);
	if(ID != ICM20948_ID) {
		// couldn't get chip ID
		initialized = false;
		return;
	}

	// The ICM20948 contains an AK09916 magnetometer, which is on the same chip but is separate and
	// communicates internally with the rest of the module via I2C. The module itself communicates
	// with the microcontroller via SPI

	SetRegister(ICM20948_REG::USER_CTL, 0b00110000); // lock to SPI mode, and enable I2C master for the magnetometer

	SetRegister(ICM20948_REG::I2C_MST_CTRL, 0b00000011); // set I2C clock frequency

	SetRegister(ICM20948_REG::PWR_MGMT_1, 0b00000001); // wake from sleep

	SetRegister(ICM20948_REG::FIFO_EN_2, 0b00011111); // enable accel, gyro and temp FIFO.

	// can either use FIFO to get accel/gyro/(temp) or read most recent from registers directly

	MagRegWrite(AK09916_REG::CNTL2, 0b00010); // set mag to continuous mode

	SetMagReadLocation(0x01, 1);
	uint8_t magID = GetRegister(ICM20948_REG::EXT_SLV_SENS_DATA_00);
	if(magID != 0b00001001) {
		// couldn't get magnetometer ID
		initialized = false;
		return;
	}

	SetMagReadLocation(AK09916_REG::MEAS, 8); // prepare mag for reading


}

/* @brief Sets a single 8-bit register.
 * @param reg The register to set. Constants contained in ICM20948_REG namespace.
 * @param val Value to set the register to.
 * @return Success
 */
bool IMUDriver::SetRegister(ICM20948_REGISTER_t reg, uint8_t val) {
	assert(initialized);
	if(reg.bank <= 0b11) {
		SwitchBank(reg.bank);
	}
	uint8_t data[2] = {(uint8_t)(0b00000000 | (reg.addr&0x7F)),val};
	CSLow();
	HAL_StatusTypeDef r = HAL_SPI_Transmit(hspi, data, 2, 1000);
	CSHigh();
	return r == HAL_OK;

}

/* @brief Gets a single 8-bit register.
 * @param reg The register to get. Constants contained in ICM20948_REG namespace.
 * @return Value read from the register.
 */
uint8_t IMUDriver::GetRegister(ICM20948_REGISTER_t reg) {
	assert(initialized);
	if(reg.bank <= 0b11) {
		SwitchBank(reg.bank);
	}
	uint8_t data[2] = {(uint8_t)(0b10000000 | reg.addr),SPI_DUMMY_BYTE};
	uint8_t incoming[2] = {0,0};
	CSLow();
	HAL_SPI_TransmitReceive(hspi, data, incoming, 2, 1000);
	CSHigh();
	return incoming[1];
}

/* @brief Reads multiple successive registers in a row.
 * @param startreg The register that the readings should start at.
 * @param numBytes Number of bytes to read.
 * @param out Address of buffer to receive data. Must be numBytes long.
 */
void IMUDriver::GetMultipleRegisters(ICM20948_REGISTER_t startreg, int numBytes,
		uint8_t *out) {
	assert(initialized);
	if(startreg.bank <= 0b11) {
		SwitchBank(startreg.bank);
	}
	uint8_t transmit[numBytes+1] = {SPI_DUMMY_BYTE};
	transmit[0] = (uint8_t)(0b10000000 | startreg.addr);

	CSLow();
	HAL_SPI_TransmitReceive(hspi, transmit, out, numBytes+1, 1000);
	CSHigh();
}

/* @brief Repeatedly reads the top byte of the FIFO.
 * @param numReads Number of repetitive reads to perform.
 * @param out Buffer to receive data in. Must be numReads long.
 */
void IMUDriver::ReadFIFO(int numReads, uint8_t *out) {
	assert(initialized);
	for(int i = 0; i < numReads; i++) {
		out[i] = GetRegister(ICM20948_REG::FIFO_RW);
	}
}

/* @brief Sets the mem location that the magnetometer will read from\
 * @param addr The memorhy location
 * @param len Number of bytes to read
 */
void IMUDriver::SetMagReadLocation(uint8_t addr, uint8_t len) {
	assert(initialized);
	SetRegister(ICM20948_REG::I2C_SLV0_ADDR, MAGNETOMETER_ID | 0b10000000);
	SetRegister(ICM20948_REG::I2C_SLV0_REG, addr);
	SetRegister(ICM20948_REG::I2C_SLV0_CTRL, 0b10000000 | (len & 0x7f));
}

/* @brief Writes an 8-bit register in the magnetometer
 * @param addr Address of register
 * @param val Value to write
 */
void IMUDriver::MagRegWrite(uint8_t addr, uint8_t val) {
	assert(initialized);
	SetRegister(ICM20948_REG::I2C_SLV0_ADDR, MAGNETOMETER_ID);
	SetRegister(ICM20948_REG::I2C_SLV0_REG, addr);
	SetRegister(ICM20948_REG::I2C_SLV0_DO, val);
}

/* @brief Extract IMU data from a raw byte buffer (such as from ReadAllSensorRegs) into a struct.
 * @param buf Input buffer containing raw data.
 * @param accel Buffer includes accel data
 * @param gyro Buffer includes gyroscope data
 * @param temp Buffer includes temperature data
 * @param mag Buffer includes magnetometer data
 * @return Struct containing extracted data
 */
const ICM20948_DATA_t IMUDriver::GetDataFromBuf(const uint8_t *buf, bool accel, bool gyro, bool temp, bool mag) {
	ICM20948_DATA_t out;
	size_t i = 0;

	// Accel gyro and temp are big-endian
	if(accel) {
		out.accel.x = (buf[i  ] << 8) | buf[i+1];
		out.accel.y = (buf[i+2] << 8) | buf[i+3];
		out.accel.z = (buf[i+4] << 8) | buf[i+5];
		i += 6;
	}

	if(gyro) {

		out.gyro.x = (buf[i  ] << 8) | buf[i+1];
		out.gyro.y = (buf[i+2] << 8) | buf[i+3];
		out.gyro.z = (buf[i+4] << 8) | buf[i+5];
		i += 6;
	}

	if(temp) {
		out.temp = (buf[i] << 8) | buf[i+1];
		i += 2;
	}

	// Magnetometer is little-endian
	if(mag) {

		out.mag.x = (buf[i+1] << 8) | buf[i];
		out.mag.y = (buf[i+3] << 8) | buf[i+2];
		out.mag.z = (buf[i+5] << 8) | buf[i+4];
	}

	return out;

}

/* @brief Reads all 20 sensor registers in order.
 * @param out Raw bytes read from registers. Must be 20 bytes long
 */
void IMUDriver::ReadAllSensorRegs(uint8_t* out) {
	GetMultipleRegisters(ICM20948_REG::ACCEL_XOUT_H, 20, out);
}

/* @brief Switches between register banks 0-3.
 * @param bank Bank to switch to. Should be from 0-3.
 * @return Success
 */
bool IMUDriver::SwitchBank(uint8_t bank) {
	return SetRegister(ICM20948_REG::BANK, (bank & 0b11) << 4);
}

IMUDriver::IMUDriver() {
}

IMUDriver::~IMUDriver() {
}

void IMUDriver::CSLow() {
	assert(initialized);
	HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_RESET);
}

void IMUDriver::SetCSPin(GPIO_TypeDef* gpio, uint16_t pin) {
	cs_gpio = gpio;
	cs_pin = pin;
}

void IMUDriver::CSHigh() {
	assert(initialized);
	HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_SET);
}
