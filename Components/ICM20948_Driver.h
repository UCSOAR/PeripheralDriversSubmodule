/*
 * IMUDriver.h
 *
 *  Created on: Aug 31, 2024
 *      Author: goada
 */

#ifndef ICM20948_DRIVER_H_
#define ICM20948_DRIVER_H_

#include "stm32h7xx.h"

constexpr uint8_t SPI_DUMMY_BYTE = 0x00;
constexpr uint8_t ICM20948_ID = 0xEA;
constexpr uint8_t MAGNETOMETER_ID = 0x0C;

struct ICM20948_REGISTER_t {
	uint8_t bank;
	uint8_t addr;
};

struct ACCEL_t {
	int16_t x;
	int16_t y;
	int16_t z;
};

struct GYRO_t {
	int16_t x;
	int16_t y;
	int16_t z;
};

struct MAG_t {
	int16_t x;
	int16_t y;
	int16_t z;
};

struct ICM20948_DATA_t {
	ACCEL_t accel;
	GYRO_t gyro;
	int16_t temp;
	MAG_t mag;
};

/* @brief Singleton SPI driver for the ICM20948 IMU.
 * Must call Init before using any other functions.
 * Max SPI frequency 7MHz.
 */
class IMUDriver {
public:
	IMUDriver();
	~IMUDriver();

	static IMUDriver& Inst() {
		static IMUDriver inst;
		return inst;
	}

	void Init(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpio_, uint16_t cs_pin_);

	bool SetRegister(ICM20948_REGISTER_t reg, uint8_t val);
	uint8_t GetRegister(ICM20948_REGISTER_t reg);
	void GetMultipleRegisters(ICM20948_REGISTER_t startreg, int numBytes, uint8_t* out);
	void ReadAllSensorRegs(uint8_t* out);
	void ReadFIFO(int numReads, uint8_t* out);

	void SetMagReadLocation(uint8_t addr, uint8_t len);
	void MagRegWrite(uint8_t addr, uint8_t val);

	IMUDriver(const IMUDriver&) = delete;
	IMUDriver& operator=(const IMUDriver&) = delete;

	const ICM20948_DATA_t GetDataFromBuf(const uint8_t *buf, bool accel = true, bool gyro = true, bool temp = true, bool mag = true);

	void SetCSPin(GPIO_TypeDef* gpio, uint16_t pin);

private:
	bool initialized = false;
	SPI_HandleTypeDef* hspi = nullptr;
	GPIO_TypeDef* cs_gpio;
	uint16_t cs_pin;

	bool SwitchBank(uint8_t bank);

	void CSLow();
	void CSHigh();

};



// Constants for registers in the ICM20948 IMU
// First byte is the bank in which it is located
// (0xff means it is located in the same place in every bank. Only used by the bank switch register)
// Second byte is address within bank
namespace ICM20948_REG {

	static const ICM20948_REGISTER_t BANK = {0xff,0x7f};
	static const ICM20948_REGISTER_t WHO_AM_I = {0,0x00};
	static const ICM20948_REGISTER_t USER_CTL = {0,0x03};
	static const ICM20948_REGISTER_t FIFO_EN_2 = {0,0x67};
	static const ICM20948_REGISTER_t PWR_MGMT_1 = {0,0x06};

	static const ICM20948_REGISTER_t ACCEL_XOUT_H = {0,0x2d};
	static const ICM20948_REGISTER_t ACCEL_XOUT_L = {0,0x2e};
	static const ICM20948_REGISTER_t ACCEL_YOUT_H = {0,0x2f};
	static const ICM20948_REGISTER_t ACCEL_YOUT_L = {0,0x30};
	static const ICM20948_REGISTER_t ACCEL_ZOUT_H = {0,0x31};
	static const ICM20948_REGISTER_t ACCEL_ZOUT_L = {0,0x32};

	static const ICM20948_REGISTER_t GYRO_XOUT_H = {0,0x33};
	static const ICM20948_REGISTER_t GYRO_XOUT_L = {0,0x34};
	static const ICM20948_REGISTER_t GYRO_YOUT_H = {0,0x35};
	static const ICM20948_REGISTER_t GYRO_YOUT_L = {0,0x36};
	static const ICM20948_REGISTER_t GYRO_ZOUT_H = {0,0x37};
	static const ICM20948_REGISTER_t GYRO_ZOUT_L = {0,0x38};

	static const ICM20948_REGISTER_t TEMP_OUT_H = {0,0x39};
	static const ICM20948_REGISTER_t TEMP_OUT_L = {0,0x3a};

	static const ICM20948_REGISTER_t FIFO_RW = {0,0x72};

	static const ICM20948_REGISTER_t I2C_MST_CTRL = {3,0x01};
	static const ICM20948_REGISTER_t I2C_SLV0_ADDR = {3,0x03};
	static const ICM20948_REGISTER_t I2C_SLV0_REG = {3,0x04};
	static const ICM20948_REGISTER_t I2C_SLV0_CTRL = {3,0x05};
	static const ICM20948_REGISTER_t I2C_SLV0_DO = {3,0x06};

	static const ICM20948_REGISTER_t EXT_SLV_SENS_DATA_00 = {0,0x3b};
}

// Constants for registers in the AK09916 magnetometer in the IMU
namespace AK09916_REG {
	static const uint8_t CNTL2 = 0x31;
	static const uint8_t MEAS = 0x11;
}

#endif /* ICM20948_DRIVER_H_ */
