#ifndef _AIRBRAKES_HPP_
#define _AIRBRAKES_HPP_

#include "stm32g4xx.h"
#define AIRBRAKES_NUM_DEPLOYMENT_LEVELS 10
#define AIRBRAKES_MAX_CURRENT_AMPS 1.0

class AirbrakesDriver {
public:
	AirbrakesDriver(TIM_HandleTypeDef* servoPWMTimer, ADC_HandleTypeDef* servoADCHandle,
			GPIO_TypeDef* servoENPort, uint16_t servoENPin,
					GPIO_TypeDef* servoLatchResetPort, uint16_t servoLatchResetPin,
					GPIO_TypeDef* comparatorPort, uint16_t comparatorPin);

	void Enable();
	void Disable();
	bool SetTargetLevel(uint8_t level);
	bool SetTargetCurrent(float current);
	uint32_t ReadRawADC();
	float ReadVoltsADC();
	bool Adjust();


private:
	TIM_HandleTypeDef* servoPWMTimer;
	ADC_HandleTypeDef* servoADCHandle;
	GPIO_TypeDef* servoENPort;
	uint16_t servoENPin;
	GPIO_TypeDef* servoLatchResetPort;
	uint16_t servoLatchResetPin;
	GPIO_TypeDef* comparatorPort;
	uint16_t comparatorPin;

	float targetVoltage = 0;

	float ADCVoltsToCurrent(float v) const;
	float CurrentToADCVolts(float c) const;

	bool CheckComparatorGood() const;

	float currentDutyCycle = 0;

};

#endif
