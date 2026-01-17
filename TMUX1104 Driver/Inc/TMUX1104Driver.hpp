#ifndef TMUX1104_HPP
#define TMUX1104_HPP

#include "stm32g4xx.h"

enum Camera{
	CAMERA1,
	CAMERA2,
	CAMERA3,
	CAMERA4
};

class TMUX1104Driver{
	public:
		TMUX1104Driver(GPIO_TypeDef* a0_gpio, uint16_t a0_pin,
						GPIO_TypeDef* a1_gpio, uint16_t a1_pin,
						GPIO_TypeDef *enable_gpio, uint16_t enable_pin);
		void InitSwitch();

	private:

		//a0 LSB for decoder
		GPIO_TypeDef *a0_gpio;
		uint16_t a0_pin;
		//a1 MSB for decoder
		GPIO_TypeDef *a1_gpio;
		uint16_t a1_pin;
		//Enable for decoder
		GPIO_TypeDef *enable_gpio;
		uint16_t enable_pin;






};
