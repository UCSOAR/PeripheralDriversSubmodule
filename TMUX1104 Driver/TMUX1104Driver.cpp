#include "Inc\TMUX1104Driver.hpp"

TMUX1104Driver::TMUX1104Driver(GPIO_TypeDef* a0_gpio, uint16_t a0_pin,
		GPIO_TypeDef* a1_gpio, uint16_t a1_pin,
		GPIO_TypeDef *enable_gpio, uint16_t enable_pin)
		:a0_gpio(a0_gpio), a0_pin(a0_pin), a1_gpio(a1_pin), a1_pin(a1_pin)
{
}

TMUX1104Driver::InitSwitch(){
	HAL_GPIO_WritePin(enable_gpio, enable_pin, GPIO_PIN_SET);

}
