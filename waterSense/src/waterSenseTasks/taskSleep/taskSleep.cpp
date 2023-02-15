/**
 * @file taskSleep.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-02-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <Arduino.h>
#include "taskSleep.h"
#include "setup.h"
#include "sharedData.h"

/**
 * @brief The sleep task
 * @details Sets the sleep time and triggers sleep
 * 
 * @param params A pointer to task parameters
 */
void taskSleep(void* params)
{
  // Task Setup
  uint8_t state = 0;
  uint64_t runTimer = millis();

  // Task Loop
  while (true)
  {
    // Begin
    if (state == 0)
    {
      if (wakeReady.get())
      {
        // Start run timer
        runTimer = millis();

        // Increment wake counter
        wakeCounter++;

        // Make sure sleep flag is not set
        sleepFlag.put(false);

        Serial.printf("Wakeup number %d\n", wakeCounter);

        Serial.println("Sleep state 0 -> 1");
        state = 1;
      }
    }

    // Wait
    else if (state == 1)
    {
      // If runTimer, go to state 2
      uint16_t myReadTime = READ_TIME.get();
      if ((millis() - runTimer) > myReadTime*1000)
      {
        Serial.println("Sleep state 1 -> 2");

        // Set sleep flag
        sleepFlag.put(true);
        state = 2;
      }
    }

    // Initiate Sleep
    else if (state == 2)
    {
      // If all tasks are ready to sleep, go to state 3
      if (sonarSleepReady.get() && tempSleepReady.get() && clockSleepReady.get() && sdSleepReady.get())
      {
        Serial.println("Sleep state 2 -> 3");
        state = 3;
      }
    }

    // Sleep
    else if (state == 3)
    {
      // Get sleep time
      uint64_t mySleep = sleepTime.get();
      // mySleep /= 1000000;

      // Go to sleep
      Serial.printf("Read time: %d minutes\nMinute Allign: %d\n", READ_TIME.get()/60, MINUTE_ALLIGN.get());
      Serial.printf("%s: Going to sleep for ", displayTime.get());
      Serial.print(mySleep/1000000);
      Serial.println(" seconds");
      
      gpio_deep_sleep_hold_en();
      esp_sleep_enable_timer_wakeup(mySleep);
      esp_deep_sleep_start();

      state = 0;
    }

    sleepCheck.put(true);
    vTaskDelay(SLEEP_PERIOD);
  }
}