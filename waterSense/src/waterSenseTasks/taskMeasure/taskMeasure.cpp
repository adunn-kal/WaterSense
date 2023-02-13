/**
 * @file taskMeasure.cpp
 * @author Alexander Dunn
 * @brief Main file for the measurement task
 * @version 0.1
 * @date 2023-02-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <Arduino.h>
#include "taskMeasure.h"
#include "setup.h"
#include "sharedData.h"
#include "waterSenseLibs/maxbotixSonar/maxbotixSonar.h"
#include "waterSenseLibs/adafruitTempHumidity/adafruitTempHumidity.h"

/**
 * @brief The measurement task
 * @details Takes measurements from the sonar sensor, temperature and humidity, and GPS data
 * 
 * @param params A pointer to task parameters
 */
void taskMeasure(void* params)
{
  MaxbotixSonar mySonar(&Serial1, SONAR_RX, SONAR_TX, SONAR_EN);
  AdafruitTempHumidity myTemp(TEMP_EN, TEMP_SENSOR_ADDRESS);

  // Task Setup
  uint8_t state = 0;

  // Task Loop
  while (true)
  {
    // Begin
    if (state == 0)
    {
      if (wakeReady.get())
      {
        // Setup sonar
        mySonar.begin();
        sonarSleepReady.put(false);

        // Setup temp/humidity
        myTemp.begin();
        tempSleepReady.put(false);

        state = 1;
      }
    }

    // Check for data
    else if (state == 1)
    {
      // If data is available, go to state 2
      if (mySonar.available())
      {
        state = 2;
      }

      // If sleepFlag is tripped, go to state 3
      if (sleepFlag.get())
      {
        state = 3;
      }
    }

    // Get measurements
    else if (state == 2)
    {
      // Get sonar measurement
      distance.put(mySonar.measure());
      temperature.put(myTemp.getTemp());
      humidity.put(myTemp.getHum());

      // Trip dataFlag
      dataReady.put(true);

      state = 1;
    }

    // Sleep
    else if (state == 3)
    {
      // Disable sonar
      mySonar.sleep();
      sonarSleepReady.put(true);

      // Disable temp/humidity
      myTemp.sleep();
      tempSleepReady.put(true);
    }

    measureCheck.put(true);
    vTaskDelay(MEASUREMENT_PERIOD);
  }
}