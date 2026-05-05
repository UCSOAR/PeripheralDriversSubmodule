#include "airbrakes.hpp"

// The PWM timer is a handle to whatever timer controls the PWM output pin (e.g. "&htim3")
// The ADC timer is a handle to whatever ADC controls the servo ADC input pin (e.g. "&hadc2");
AirbrakesDriver::AirbrakesDriver(TIM_HandleTypeDef* servoPWMTimerHandle,
		ADC_HandleTypeDef* servoADCHandle, GPIO_TypeDef* servoENPort, uint16_t servoENPin,
		GPIO_TypeDef* servoLatchResetPort, uint16_t servoLatchResetPin,
		GPIO_TypeDef* comparatorPort, uint16_t comparatorPin) : servoPWMTimer(servoPWMTimerHandle), servoADCHandle(servoADCHandle),
		servoENPort(servoENPort),servoENPin(servoENPin),servoLatchResetPort(servoLatchResetPort),servoLatchResetPin(servoLatchResetPin),
		comparatorPort(comparatorPort),comparatorPin(comparatorPin){

	HAL_GPIO_WritePin(servoLatchResetPort, servoLatchResetPin, GPIO_PIN_RESET); // put SR latch in known position
	Disable();
}

/* @brief Enables the driver. Starts ADC sampling and PWM output.
 * Resets current latch and monitors for runaway current for 250ms.
 */
void AirbrakesDriver::Enable() {
	HAL_TIM_PWM_Start(servoPWMTimer, TIM_CHANNEL_1);
	HAL_ADC_Start(servoADCHandle);
	HAL_GPIO_WritePin(servoENPort, servoENPin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(servoLatchResetPort, servoLatchResetPin, GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(servoLatchResetPort, servoLatchResetPin, GPIO_PIN_RESET);
	// Immediately stop if current starts running away
	for(uint16_t i = 0; i < 250; i++) {
		if(!CheckComparatorGood()) {
			Disable();
			return;
		}
		HAL_Delay(1);
	}
}

/* @brief Disables the driver. Stops ADC sampling and PWM output and disables current.
 */
void AirbrakesDriver::Disable() {
	HAL_GPIO_WritePin(servoENPort, servoENPin, GPIO_PIN_RESET);
	HAL_TIM_PWM_Stop(servoPWMTimer, TIM_CHANNEL_1);
	HAL_ADC_Stop(servoADCHandle);
	HAL_GPIO_WritePin(servoLatchResetPort, servoLatchResetPin, GPIO_PIN_RESET);
}

/* @brief Set the airbrake PWM level from fully off to fully on.
 * @param level The airbrake level from 0 to (AIRBRAKES_NUM_DEPLOYMENT_LEVELS-1) inclusive. For example, if there are 10 levels, accepts 0-9.
 * @return true if successful.
 */
bool AirbrakesDriver::SetTargetLevel(uint8_t level) {
	if(level >= AIRBRAKES_NUM_DEPLOYMENT_LEVELS) {
		return false;
	}
	return SetTargetCurrent(((float)AIRBRAKES_MAX_CURRENT_AMPS)*level/(AIRBRAKES_NUM_DEPLOYMENT_LEVELS-1));
}

/* @brief Set the airbrake target current directly.
 * @param current The current in amps to request.
 * @return true if successful.
 */
bool AirbrakesDriver::SetTargetCurrent(float current) {

	float targetvoltcalc = CurrentToADCVolts(current);
	if(targetvoltcalc > 3.3 || targetvoltcalc < 0) {
		return false;
	}
	targetVoltage = targetvoltcalc;
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
 * @return true if successful. false if PWM either hits the rails or the current comparator trips.
 */
bool AirbrakesDriver::Adjust() {

	if(HAL_GPIO_ReadPin(comparatorPort, comparatorPin) == GPIO_PIN_SET) {
		Disable();
		return false;
	}
	float currentV = ReadVoltsADC();
	float diff = (targetVoltage-currentV);

	// if we're trying to force the pwm beyond 100% or 0% even though it's already there return false
	if((currentDutyCycle >= 1.0 && diff > 0) || (currentDutyCycle <= 0.0 && diff < 0)) {
		return false;
	}

	currentDutyCycle += diff * 0.1;
	if(currentDutyCycle > 1.0) {
		currentDutyCycle = 1.0;
	}
	else if(currentDutyCycle < 0.0) {
		currentDutyCycle = 0.0;
	}

	servoPWMTimer->Instance->CCR1 = servoPWMTimer->Instance->ARR * currentDutyCycle;

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

/* @brief Checks the state of the hardware failsafe latch.
 * @return true if the latch is good, i.e. current limit has not been triggered
 */
bool AirbrakesDriver::CheckComparatorGood() const {

	return HAL_GPIO_ReadPin(comparatorPort, comparatorPin) == GPIO_PIN_RESET;
}
