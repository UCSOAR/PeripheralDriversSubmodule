/*
 * IMUDriver.h
 *
 *  Created on: Aug 31, 2024
 *      Author: goada
 */

#ifndef LSM6DO32DRIVER_H_
#define LSM6DO32DRIVER_H_

#include "stm32h7xx.h"

constexpr uint8_t SPI_DUMMY_BYTE = 0x00;
constexpr uint8_t LSM6DSO32_ID = 0x6C;

typedef uint8_t LSM6DSO32_REGISTER_t;

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

struct LSM6DSO32_DATA_t {
	ACCEL_t accel;
	GYRO_t gyro;
	int16_t temp;
};

/* @brief Singleton SPI driver for the 6-axis LSM6DSO32 IMU.
 * Must call Init before using any other functions besides SetCSPin.
 * SPI max clock speed 10MHz.
 */
class LSM6DO32_Driver {
public:
	LSM6DO32_Driver();
	~LSM6DO32_Driver();

	static LSM6DO32_Driver& Inst() {
		static LSM6DO32_Driver inst;
		return inst;
	}
	enum LSM6D032_SAMPLE_SPEED {
		FREQ_12p5_HZ=0b0001,
		FREQ_26_HZ=0b0010,
		FREQ_52_HZ=0b0011,
		FREQ_104_HZ=0b0100,
		FREQ_208_HZ=0b0101,
		FREQ_416_HZ=0b0110,
		FREQ_833_HZ=0b0111,
		FREQ_1660_HZ=0b1000,
		FREQ_3330_HZ=0b1001,
		FREQ_6660_HZ=0b1010
	};

	enum LSM6D032_ACCEL_SCALE_SELECT {
		SCALE_4g = 0b00,
		SCALE_32g = 0b01,
		SCALE_8g = 0b10,
		SCALE_16g = 0b11
	};

	enum LSM6D032_GYRO_SCALE_SELECT {
		SCALE_250dps = 0b00,
		SCALE_500dps = 0b01,
		SCALE_1000dps = 0b10,
		SCALE_2000dps = 0b11
	};


	void Init(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpio_, uint16_t cs_pin_);

	bool SetRegister(LSM6DSO32_REGISTER_t reg, uint8_t val);
	uint8_t GetRegister(LSM6DSO32_REGISTER_t reg);
	void GetMultipleRegisters(LSM6DSO32_REGISTER_t startreg, int numBytes, uint8_t* out);
	void ReadAllSensorRegs(uint8_t* out);
	void ReadFIFOs(int numReads, uint8_t* out);

	void SetAccelODR(LSM6D032_SAMPLE_SPEED speed);
	void SetGyroODR(LSM6D032_SAMPLE_SPEED speed);
	void SetAccelFullScaleRange(LSM6D032_ACCEL_SCALE_SELECT scale);
	void SetGyroFullScaleRange(LSM6D032_GYRO_SCALE_SELECT scale);

	LSM6DO32_Driver(const LSM6DO32_Driver&) = delete;
	LSM6DO32_Driver& operator=(const LSM6DO32_Driver&) = delete;

	const LSM6DSO32_DATA_t GetDataFromBuf(const uint8_t *buf, bool accel = true, bool gyro = true, bool temp = true);

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



// Constants for registers in the LSM6DSO32 IMU
// First byte is the bank in which it is located
// (0xff means it is located in the same place in every bank. Only used by the bank switch register)
// Second byte is address within bank
namespace LSM6DSO32_REG {

	static const LSM6DSO32_REGISTER_t FIFO_CTRL2 = 0x08;
	static const LSM6DSO32_REGISTER_t FIFO_CTRL3 = 0x09;
	static const LSM6DSO32_REGISTER_t FIFO_CTRL4 = 0x0A;

	static const LSM6DSO32_REGISTER_t WHO_AM_I  = 0x0F;

	static const LSM6DSO32_REGISTER_t CTRL1_XL  = 0x10;
	static const LSM6DSO32_REGISTER_t CTRL2_G   = 0x11;

	static const LSM6DSO32_REGISTER_t STATUS_REG    = 0x1E;

	static const LSM6DSO32_REGISTER_t OUT_TEMP_L    = 0x20;
	static const LSM6DSO32_REGISTER_t OUT_TEMP_H    = 0x21;

	static const LSM6DSO32_REGISTER_t OUTX_L_G    = 0x22;
	static const LSM6DSO32_REGISTER_t OUTX_H_G    = 0x23;
	static const LSM6DSO32_REGISTER_t OUTY_L_G    = 0x24;
	static const LSM6DSO32_REGISTER_t OUTY_H_G    = 0x25;
	static const LSM6DSO32_REGISTER_t OUTZ_L_G    = 0x26;
	static const LSM6DSO32_REGISTER_t OUTZ_H_G    = 0x27;

	static const LSM6DSO32_REGISTER_t OUTX_L_A    = 0x28;
	static const LSM6DSO32_REGISTER_t OUTX_H_A    = 0x29;
	static const LSM6DSO32_REGISTER_t OUTY_L_A    = 0x2A;
	static const LSM6DSO32_REGISTER_t OUTY_H_A    = 0x2B;
	static const LSM6DSO32_REGISTER_t OUTZ_L_A    = 0x2C;
	static const LSM6DSO32_REGISTER_t OUTZ_H_A    = 0x2D;

	static const LSM6DSO32_REGISTER_t FIFO_DATA_OUT_TAG = 0x78;

	static const LSM6DSO32_REGISTER_t FIFO_DATA_OUT_X_L = 0x79;
	static const LSM6DSO32_REGISTER_t FIFO_DATA_OUT_X_H = 0x7A;

	static const LSM6DSO32_REGISTER_t FIFO_DATA_OUT_Y_L = 0x7B;
	static const LSM6DSO32_REGISTER_t FIFO_DATA_OUT_Y_H = 0x7C;

	static const LSM6DSO32_REGISTER_t FIFO_DATA_OUT_Z_L = 0x7D;
	static const LSM6DSO32_REGISTER_t FIFO_DATA_OUT_Z_H = 0x7E;


}



#endif /* LSM6DO32DRIVER_H_ */
