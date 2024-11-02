/**
 ******************************************************************************
 * File Name          : PWMController.hpp
 * Description        : PWM Controller
 * Useful References  : https://deepbluembedded.com/stm32-pwm-example-timer-pwm-mode-tutorial/
 ******************************************************************************
 */

#ifndef PortPWMDriver_HPP
#define PortPWMDriver_HPP
#include "stm32f4xx_hal_tim.h"

// PCLK1_CLOCK_FREQ is specified by user in clock configuration by APB2 or ABP1 Timer Clocks in MHz (Dependant on Board Specifications)

/*
 * Calculations (example):
 *
 * PCLK1_CLOCK_FREQ = 24 MHz = 24,000,000 Hz
 * Freq = X (Ideally 50Hz to obtain period of 20ms for PWM)
 * Fclk / ((PSC) * (ARR)) = Freq (Therefore set PSC as 480 and ARR as 1000)
 * 24000000 / (480 * 1000) = 50
 */

class PWMController
{
public:
    // Constructor accepts a timer in the form htimx, and an integer for channel. e.g (htim1, 2)
    PWMController(TIM_HandleTypeDef &htim, uint8_t ch) : htim_(htim), ch_(ch)
    {

        // Set the timer channel variable (bitwise)
        // 
        switch (ch_)
        {
        case 1:
            timCh_ = TIM_CHANNEL_1;
            break;
        case 2:
            timCh_ = TIM_CHANNEL_2;
            break;
        case 3:
            timCh_ = TIM_CHANNEL_3;
            break;
        case 4:
            timCh_ = TIM_CHANNEL_4;
            break;
        default:
            timCh_ = 0;
            break;
        }
    }

    // --------------------------------------
    // Start the PWM
    void start() { HAL_TIM_PWM_Start(&htim_, timCh_); }
    // End the PWM 
    void stop() { HAL_TIM_PWM_Stop(&htim_, timCh_); }

    void setCCR(uint32_t ccr){
    	__HAL_TIM_SET_COMPARE(&htim_, timCh_, ccr);
    }

    uint32_t getCCR(){
    __HAL_TIM_GET_COMPARE(&htim_, timCh_);
    return 0;
    }

    void setARR(uint32_t arr)
    {
        __HAL_TIM_SET_AUTORELOAD(&htim_, arr);
    }

    uint32_t getARR()
    {
        return __HAL_TIM_GET_AUTORELOAD(&htim_);
    }

    void setPSC(uint32_t psc)
    {
        __HAL_TIM_SET_PRESCALER(&htim_, psc);
    }

    int32_t getPSC()
    {
        return htim_.Init.Prescaler;
    }

    // returns a value between 0-100 corresponding to the duty cycle %, e.g. 25 is 25%
    float getDutyCycle() { return (getCCR() * 100) / getARR(); }

    bool setDutyCycle(float duty)
    {
        if (duty > 100)
        {
            return false;
        }

        uint32_t calCCR = getARR() * (duty / 100.0);

        setCCR(calCCR);

        return true;
    }

    bool rotate(uint16_t degrees)
    {
        if (degrees > 270)
        {
            return false;
        }

        // Calculate duty cycle based on degrees (0-270)
        float duty = (degrees / 270.0) * 20;
        return setDutyCycle(duty);

    }

    private:
        TIM_HandleTypeDef & htim_;
        uint8_t ch_;
        uint8_t timCh_;
    };

#endif // PPortPWMDriver_HPP
