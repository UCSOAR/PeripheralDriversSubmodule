#include "airbrakes.hpp"
#include <math.h>

// The PWM timer is a handle to whatever timer controls the PWM output pin (e.g. "&htim3")
// The ADC timer is a handle to whatever ADC controls the servo ADC input pin (e.g. "&hadc2");
AirbrakesDriver::AirbrakesDriver(TIM_HandleTypeDef* servoPWMTimerHandle,
		ADC_HandleTypeDef* servoADCHandle, GPIO_TypeDef* servoENPort, uint16_t servoENPin,
		GPIO_TypeDef* servoLatchResetPort, uint16_t servoLatchResetPin,
		GPIO_TypeDef* comparatorPort, uint16_t comparatorPin, float timerHz) : servoPWMTimer(servoPWMTimerHandle), servoADCHandle(servoADCHandle),
		servoENPort(servoENPort),servoENPin(servoENPin),servoLatchResetPort(servoLatchResetPort),servoLatchResetPin(servoLatchResetPin),
		comparatorPort(comparatorPort),comparatorPin(comparatorPin),hz(timerHz){

	HAL_GPIO_WritePin(servoLatchResetPort, servoLatchResetPin, GPIO_PIN_RESET); // put SR latch in known position
	Disable();
}

/* @brief Enables the driver. Starts ADC sampling and PWM output.
 * Resets current latch and monitors for runaway current for 250ms.
 */
void AirbrakesDriver::Enable() {
	if(enabled) {
		return;
	}

	HAL_TIM_PWM_Start(servoPWMTimer, TIM_CHANNEL_1);
	HAL_ADC_Start(servoADCHandle);

	HAL_GPIO_WritePin(servoLatchResetPort, servoLatchResetPin, GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(servoLatchResetPort, servoLatchResetPin, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(servoENPort, servoENPin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(servoLatchResetPort, servoLatchResetPin, GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(servoLatchResetPort, servoLatchResetPin, GPIO_PIN_RESET);

	HAL_Delay(10);

	// Immediately stop if current starts running away
	for(uint16_t i = 0; i < 100; i++) {
		if(!CurrentGood()) {
			Disable();
			return;
		}
		HAL_Delay(1);
	}
	vel = 0;
	enabled = true;
}

/* @brief Disables the driver. Stops ADC sampling and PWM output and disables current.
 */
void AirbrakesDriver::Disable() {
	if(!enabled) {
		return;
	}
	HAL_GPIO_WritePin(servoENPort, servoENPin, GPIO_PIN_RESET);
	HAL_TIM_PWM_Stop(servoPWMTimer, TIM_CHANNEL_1);
	HAL_ADC_Stop(servoADCHandle);
	HAL_GPIO_WritePin(servoLatchResetPort, servoLatchResetPin, GPIO_PIN_RESET);
	vel = 0;
	enabled = false;
}

/* @brief Set the airbrake level from fully off to fully on.
 * @param level The airbrake level from 0 to (AIRBRAKES_NUM_DEPLOYMENT_LEVELS-1) inclusive. For example, if there are 10 levels, accepts 0-9.
 * @return true if successful.
 */
bool AirbrakesDriver::SetTargetLevel(uint8_t level) {
	if(level >= AIRBRAKES_NUM_DEPLOYMENT_LEVELS) {
		return false;
	}

	hit = false;
	vel = 0;
	return SetTargetDutyCycle(static_cast<float>(level)/(AIRBRAKES_NUM_DEPLOYMENT_LEVELS-1)*(0.0025*hz-0.0005*hz)/2.1+0.0005*hz);
}

/* @brief Set the airbrake target duty cycle directly.
 * @param dutyFrac Duty cycle between 0.0 and 1.0.
 * @return true if successful.
 */
bool AirbrakesDriver::SetTargetDutyCycle(float dutyFrac) {

	if(dutyFrac > 1.0f || dutyFrac < 0.0f) {
		return false;
	}
	targetDutyCycle = dutyFrac;
	hit = false;
	vel = 0;
	return true;
}

/* @brief Read the ADC value in raw counts.
 * @return The value in counts from 0 to the max value of the ADC inclusive (4095 for 12-bit resolution).
 */
uint32_t AirbrakesDriver::ReadRawADC() {
	return HAL_ADC_GetValue(servoADCHandle);
}

/* @brief Read the ADC value in voltage assuming 12-bit resolution.
 * @return The ADC value in volts.
 */
float AirbrakesDriver::ReadVoltsADC() {
	return float(ReadRawADC())/((1<<12)-1)*3.3f;
}

/* @brief Run the active current ramping loop. Will smooth PWM changes and check latch status.
 * @return true if successful. false if the current exceeds limits or the driver is disabled.
 */
bool AirbrakesDriver::TickControlLoop() {

	if(!IsEnabled()) {
		return false;
	}

	if(!CurrentGood()) {
		Disable();
		return false;
	}

	if(hit) {
		return true;
	}



    float distance = targetDutyCycle - currentDutyCycle;
    float ideal_vel = sqrtf(2.0 * MAX_ACCEL * fabsf(distance));

    if (distance < 0) {
        ideal_vel = -ideal_vel;
    }

    if (ideal_vel > MAX_VELOCITY) {
        ideal_vel = MAX_VELOCITY;
    }
    else if (ideal_vel < -MAX_VELOCITY) {
        ideal_vel = -MAX_VELOCITY;
    }

    float vel_error = ideal_vel - vel;
    if (vel_error > MAX_ACCEL) {
        vel += MAX_ACCEL;
    }
    else if (vel_error < -MAX_ACCEL) {
        vel -= MAX_ACCEL;
    }
    else  {
        vel = ideal_vel;
    }

    currentDutyCycle += vel;

    if(currentDutyCycle < ABS_MIN*hz) {
    	currentDutyCycle = ABS_MIN*hz;
    }
    if(currentDutyCycle > ABS_MAX*hz) {
    	currentDutyCycle = ABS_MAX*hz;
    }

    SetHardwareCycle();

	if(abs(currentDutyCycle-targetDutyCycle) < MAX_VELOCITY) {
		hit = true;
		vel = 0;
	}

	return true;

}

/* @brief Converts a voltage measured by the ADC into the corresponding current through the servo
 * Assumes 0.132V/A from the ACS725LLCTR-10AB datasheet
 * @param v Voltage to convert (V)
 * @return Calculated servo current (A)
 */
float AirbrakesDriver::ADCVoltsToCurrent(float v) const {
	return (v-1.6)/0.132;
}

/* @brief Converts a current through the servo into the corresponding voltage measured by the ADC
 * Assumes 0.132V/A from the ACS725LLCTR-10AB datasheet
 * @param c Current to convert (A)
 * @return Calculated ADC voltage (V)
 */
float AirbrakesDriver::CurrentToADCVolts(float c) const {
	return c*0.132+1.6;
}

/* @brief Checks that the hardware failsafe has not triggered and the measured current is within limits.
 * @return true is current is safely within range.
 */
bool AirbrakesDriver::CurrentGood() {
	//return CheckComparatorGood();
	return CheckComparatorGood() && (ADCVoltsToCurrent(ReadVoltsADC()) <= AIRBRAKES_MAX_CURRENT_AMPS);
}


/* @brief Checks the state of the hardware failsafe latch.
 * @return true if the latch is good, i.e. current limit has not been triggered
 */
bool AirbrakesDriver::CheckComparatorGood() const {

	return HAL_GPIO_ReadPin(comparatorPort, comparatorPin) == GPIO_PIN_SET;
}


