#include "TMUX1104Driver.hpp"

TMUX1104Driver::TMUX1104Driver(GPIO_TypeDef* a0_gpio, uint16_t a0_pin,
		GPIO_TypeDef* a1_gpio, uint16_t a1_pin,
		GPIO_TypeDef *enable_gpio, uint16_t enable_pin)
		:a0_gpio_(a0_gpio), a0_pin_(a0_pin), a1_gpio_(a1_gpio), a1_pin_(a1_pin),
		 enable_gpio_(enable_gpio), enable_pin_(enable_pin)
{
	HAL_GPIO_WritePin(enable_gpio_, enable_pin_, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(_a0_gpio, _a0_pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(_a1_gpio, _a1_pin, GPIO_PIN_RESET);
	current_cam = Camera::NONE;
	enabled = false;


}

void TMUX1104Driver::Enable(){
	//turns on enable for circuit
	HAL_GPIO_WritePin(enable_gpio_, enable_pin_, GPIO_PIN_SET);
	enabled = true;

}

void TMUX1104Driver::Disable(){

	//turns off enable for circuit (a1, a0) -> (X, X)
	HAL_GPIO_WritePin(enable_gpio_, enable_pin_, GPIO_PIN_RESET);
	enabled = false;

}

void TMUX1104Driver::Select(Camera cam){
	switch(cam){
	case CAMERA1:
		//(0, 0) input to mux allows connection to peripheral at s1
		HAL_GPIO_WritePin(_a1_gpio, _a1_pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(_a0_gpio, _a0_pin, GPIO_PIN_RESET);
		current_cam = CAMERA1;
		break;
	case CAMERA2:
		//(0, 1) input to mux allows connection to peripheral at s2
		HAL_GPIO_WritePin(_a1_gpio, _a1_pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(_a0_gpio, _a0_pin, GPIO_PIN_SET);
		current_cam = CAMERA2;
		break;
	case CAMERA3:
		//(1, 0) input to mux allows connection to peripheral at s3
		HAL_GPIO_WritePin(_a1_gpio, _a1_pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(_a0_gpio, _a0_pin, GPIO_PIN_RESET);
		current_cam = CAMERA3;
		break;
	case CAMERA4:
		//(1, 1) input to mux allows connection to peripheral at s4
		HAL_GPIO_WritePin(_a1_gpio, _a1_pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(_a0_gpio, _a0_pin, GPIO_PIN_SET);
		current_cam = CAMERA4;
		break;
	default:
		SOAR_PRINT("No camera has been selected");
		current_cam = NONE;
	}
}
