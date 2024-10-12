#include "stm32f4xx_hal.h"

class PWM {
private:
    TIM_HandleTypeDef htim;
    uint32_t channel;
    uint32_t pclk_freq;
    uint32_t duty_cycle;

public:
    PWM(TIM_HandleTypeDef htim, uint32_t channel, uint32_t pclk_freq) 
        : htim(htim), channel(channel), pclk_freq(pclk_freq), duty_cycle(0) {}

    void start() {
        HAL_TIM_PWM_Start(&htim, channel);
    }

    void stop() {
        HAL_TIM_PWM_Stop(&htim, channel);
    }

    void setDutyCycle(uint32_t duty_cycle) {
        this->duty_cycle = duty_cycle;
        __HAL_TIM_SET_COMPARE(&htim, channel, duty_cycle);
    }

    uint32_t getDutyCycle() {
        return duty_cycle;
    }

    void setPCLKFreq(uint32_t pclk_freq) {
        this->pclk_freq = pclk_freq;
        // Reconfigure the timer with the new PCLK frequency if needed
    }
};

int main() {
    HAL_Init();
    SystemClock_Config();

    // Initialize the timer and PWM
    TIM_HandleTypeDef htim2;
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 84 - 1;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 1000 - 1;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim2);

    PWM pwm(htim2, TIM_CHANNEL_1, HAL_RCC_GetPCLK1Freq());
    pwm.start();
    pwm.setDutyCycle(500); // Set 50% duty cycle

    while (1) {
        // Main loop
    }
}