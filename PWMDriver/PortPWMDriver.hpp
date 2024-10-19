#ifndef PWM_DRIVER_HPP
#define PWM_DRIVER_HPP

#include "stm32f4xx_hal.h"

#define PCLK1_CLK_FREQ 64000000

/*
 * Calculations:
 *
 * Fclk = 64 MHz = 64,000,000 Hz
 * Freq = X
 * Fclk / ((PSC-1) * (ARR)) = Freq
 * PSC = (Fclk / (Freq * ARR)) + 1
 */

class PWM
{
private:
    TIM_HandleTypeDef htim;
    uint32_t pclk_freq;
    uint32_t duty_cycle;
    uint32_t pwm_freq;
    uint32_t channel;

public:
    PWM(TIM_TypeDef *instance, uint32_t pclk_freq, uint32_t channel)
        : pclk_freq(pclk_freq), duty_cycle(0), channel(channel)
    {
        htim.Instance = instance;
        htim.Init.Prescaler = 64 - 1; // Prescaler value
        htim.Init.CounterMode = TIM_COUNTERMODE_UP;
        htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        htim.Init.Period = 10000; // Fixed ARR value

        HAL_TIM_PWM_Init(&htim);

        // Calculate the PWM frequency
        pwm_freq = pclk_freq / ((htim.Init.Period + 1) * (htim.Init.Prescaler + 1));
    }

    void start()
    {
        HAL_TIM_PWM_Start(&htim, channel);
    }

    void stop()
    {
        HAL_TIM_PWM_Stop(&htim, channel);
    }

    void setCCR(uint32_t ccr)
    {
        __HAL_TIM_SET_COMPARE(&htim, channel, ccr);
    }

    uint32_t getCCR()
    {
        return __HAL_TIM_GET_COMPARE(&htim, channel);
    }

    void setARR(uint32_t arr)
    {
        __HAL_TIM_SET_AUTORELOAD(&htim, arr);
    }

    uint32_t getARR()
    {
        return __HAL_TIM_GET_AUTORELOAD(&htim);
    }

    void setPSC(uint32_t psc)
    {
        __HAL_TIM_SET_PRESCALER(&htim, psc);
    }

    uint32_t getPSC()
    {
        return htim.Init.Prescaler;
    }

    float getDutyCycle()
    {
        return (getCCR() * 100.0f) / getARR();
    }

    void setFreq(uint32_t freq)
    {
        uint32_t arr = getARR();
        uint32_t prescaler_value = (pclk_freq / (freq * arr)) - 1;
        setPSC(prescaler_value);
    }

    bool setDutyCycle(float duty)
    {
        if (duty > 100)
        {
            return false;
        }

        uint32_t calCCR = getARR() * (duty / 100.0f);
        setCCR(calCCR);

        return true;
    }
};

#endif // PWM_DRIVER_HPP