#include "stm32f4xx_hal.h"

template <typename TIM_TypeDef, uint32_t Channel>
class PWM
{
private:
    TIM_HandleTypeDef htim;
    uint32_t pclk_freq;
    uint32_t duty_cycle;
    uint32_t pwm_freq;

public:
    PWM(TIM_TypeDef *instance, uint32_t pclk_freq)
        : pclk_freq(pclk_freq), duty_cycle(0)
    {
        htim.Instance = instance;
        htim.Init.Prescaler = 84 - 1; // Prescaler value
        htim.Init.CounterMode = TIM_COUNTERMODE_UP;
        htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        htim.Init.Period = 65535; // Fixed ARR value

        HAL_TIM_PWM_Init(&htim);

        // Calculate the PWM frequency
        pwm_freq = pclk_freq / ((htim.Init.Period + 1) * (htim.Init.Prescaler + 1));
    }

    void start()
    {
        HAL_TIM_PWM_Start(&htim, Channel);
    }

    void stop()
    {
        HAL_TIM_PWM_Stop(&htim, Channel);
    }

    void setDutyCycle(uint32_t duty_cycle)
    {
        this->duty_cycle = duty_cycle;

        // Calculate the CCR value for the given duty cycle percentage
        uint32_t ccr_value = (duty_cycle * __HAL_TIM_GET_AUTORELOAD(&htim)) / 100;

        // Set the CCR register value
        __HAL_TIM_SET_COMPARE(&htim, Channel, ccr_value);
    }

    uint32_t getDutyCycle()
    {
        return duty_cycle;
    }

    void setPCLKFreq(uint32_t pclk_freq)
    {
        this->pclk_freq = pclk_freq;
        // Recalculate the PWM frequency with the new PCLK frequency
        pwm_freq = pclk_freq / ((htim.Init.Period + 1) * (htim.Init.Prescaler + 1));
    }

    void rotateToAngle(float angle)
    {
        // Ensure the angle is within the valid range
        if (angle < 0)
            angle = 0;
        if (angle > 270)
            angle = 270;

        // Calculate the pulse width in microseconds
        float pulse_width = 500 + (angle / 270.0) * (2500 - 500);

        // Calculate the period in microseconds
        float period = (1.0 / pwm_freq) * 1000000;

        // Calculate the duty cycle percentage
        float duty_cycle = (pulse_width / period) * 100;

        // Set the duty cycle
        setDutyCycle(static_cast<uint32_t>(duty_cycle));
    }

    void dimLED(float brightness)
    {
        // Ensure the brightness is within the valid range (0 to 100%)
        if (brightness < 0)
            brightness = 0;
        if (brightness > 100)
            brightness = 100;

        // Set the duty cycle based on the brightness percentage
        setDutyCycle(static_cast<uint32_t>(brightness));
    }
};

int main()
{
    HAL_Init();
    SystemClock_Config();

    // Initialize the timer and PWM with a clock frequency of 64 MHz
    PWM<TIM_TypeDef, TIM_CHANNEL_1> pwm(TIM8, 64000000); // 64 MHz clock
    pwm.start();
    pwm.rotateToAngle(135); // Rotate to 135 degrees

    // Dim the LED to 50% brightness
    pwm.dimLED(50);

    while (1)
    {
        // Main loop
    }
}