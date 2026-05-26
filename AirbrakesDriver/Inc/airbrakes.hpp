#ifndef _AIRBRAKES_HPP_
#define _AIRBRAKES_HPP_

#include "stm32g4xx.h"
#define AIRBRAKES_NUM_DEPLOYMENT_LEVELS 9
#define AIRBRAKES_MAX_CURRENT_AMPS 10.0

class AirbrakesDriver {
public:
	AirbrakesDriver(TIM_HandleTypeDef* servoPWMTimer, ADC_HandleTypeDef* servoADCHandle,
			GPIO_TypeDef* servoENPort, uint16_t servoENPin,
					GPIO_TypeDef* servoLatchResetPort, uint16_t servoLatchResetPin,
					GPIO_TypeDef* comparatorPort, uint16_t comparatorPin, float timerHz);

	void Enable();
	void Disable();
	bool SetTargetLevel(uint8_t level);
	bool SetTargetDutyCycle(float current);
	uint32_t ReadRawADC();
	float ReadVoltsADC();
	bool TickControlLoop();

	inline bool IsEnabled() const {
		return enabled;
	}


private:
	TIM_HandleTypeDef* servoPWMTimer;
	ADC_HandleTypeDef* servoADCHandle;
	GPIO_TypeDef* servoENPort;
	uint16_t servoENPin;
	GPIO_TypeDef* servoLatchResetPort;
	uint16_t servoLatchResetPin;
	GPIO_TypeDef* comparatorPort;
	uint16_t comparatorPin;

	float targetDutyCycle = 0;

	float ADCVoltsToCurrent(float v) const;
	float CurrentToADCVolts(float c) const;

	bool CurrentGood();
	bool CheckComparatorGood() const;

	float currentDutyCycle = 0;
	float vel = 0.0;

	static inline uint32_t round(float v) {
		if(v > 0)
			return static_cast<uint32_t>(v+0.5f);
		else
			return static_cast<uint32_t>(v-0.5f);
	}

	bool enabled = false;

	const float hz;

	bool hit = false;


};

#endif
