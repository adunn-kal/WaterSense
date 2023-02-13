/**
 * @file taskVoltage.cpp
 * @author Alexander Dunn
 * @brief Main file for the voltage measurement task
 * @version 0.1
 * @date 2023-02-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <Arduino.h>
#include "taskVoltage.h"
#include "setup.h"
#include "sharedData.h"

#include <driver/adc.h>


/**
 * @brief The voltage task
 * @details Checks solar panel voltage and sets duty cycle
 * 
 * @param params A pointer to task parameters
 */
void taskVoltage(void* params)
{
  adc1_config_width(ADC_WIDTH_12Bit);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_0db); //set reference voltage to internal

  // Task Setup
  uint8_t state = 0;

  // Task Loop
  while (true)
  {
    // Measure voltage
    if (state == 0)
    {
      // Measure voltage
      float solarVoltage = 0.0;
      float batteryVoltage = 0.0;
      
      for (uint8_t i = 0; i < 50; i++)
      {
        solarVoltage += analogRead(ADC_PIN)*(3.3/4095) * ((R1s + R2s)/ R2s);
        batteryVoltage += adc1_get_raw(ADC1_CHANNEL_0)*(1.1/4095) * ((R1b + R2b)/ R2b);
      }

      solarVoltage /= 50.0;
      batteryVoltage /= 50.0;
  
      solar.put(solarVoltage);
      battery.put(batteryVoltage);
      
      #ifdef VARIABLE_DUTY
        if (batteryVoltage < 3.0)
        {
          READ_TIME.put(LOW_READ);
          MINUTE_ALLIGN.put(LOW_ALLIGN);
        }

        else if (batteryVoltage < 3.2)
        {
          READ_TIME.put(MID_READ);
          MINUTE_ALLIGN.put(MID_ALLIGN);
        }

        else
        {
          READ_TIME.put(HI_READ);
          MINUTE_ALLIGN.put(HI_ALLIGN);
        }
      #endif
    }
    
    voltageCheck.put(true);
    vTaskDelay(VOLTAGE_PERIOD);
  }
}