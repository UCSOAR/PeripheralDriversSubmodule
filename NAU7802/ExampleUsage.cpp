/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "I2C_Wrapper.hpp"
#include "NAU7802.hpp"

/* Private variables ---------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;

/* Function Prototypes -------------------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);

int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();

  /* --- SETUP STAGE --- */
    // 1. Create wrapper and config structs
    I2C_Wrapper i2c_bus_1(&hi2c1);
    NAU7802_PARAMS adc_config;
    NAU7802_OUT adc_data;

  // 3. Configure parameters
  adc_config.initialGain = NAU7802_GAIN_128X;
  
  // 4. Create the driver object
  NAU7802 adc(adc_config, i2c_bus_1);
  // 5. Check if initialization was successful
  if (!adc.get_isInitialized()) 
  {
    // ADC not found or failed to configure
    Error_Handler();
  }

  /* --- MAIN LOOP --- */
  while (1)
  {
    // 7. Check if data is ready
    if (adc.isReady()) 
    {
      // 8. Read data
      if (adc.readSensor(&adc_data) == HAL_OK)
      {
        // Successfully read data
        //Example Debug Print:
        // printf("Raw ADC Value: %ld\r\n", adc_data.raw_reading);
      }
    }
    HAL_Delay(100);
  }
}