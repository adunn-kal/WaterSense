/**
 * @file taskClock.cpp
 * @author Alexander Dunn
 * @brief Main file for the clock task
 * @version 0.1
 * @date 2023-02-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <Arduino.h>
#include "taskClock.h"
#include "setup.h"
#include "sharedData.h"
#include "waterSenseLibs/gpsClock/gpsClock.h"

/**
 * @brief The clock task
 * @details Runs the GPS clock in the background to maintain timestamps
 * 
 * @param params A pointer to task parameters
 */
void taskClock(void* params)
{
  GpsClock myGPS(&Serial2, GPS_RX, GPS_TX, GPS_EN);
  Adafruit_GPS myClock = myGPS.begin();
  
  uint8_t state = 0;

  while (true)
  {
    // Begin
    if (state == 0)
    {
      if ((wakeCounter % 1000) == 0)
      {
        wakeReady.put(false);
        state = 4;
      }

      else
      {
        wakeReady.put(true);
        state = 1;
      }
    }

    // Update
    else if (state == 1)
    {
      // Update the GPS
      myGPS.update(myClock);

      // If new data is available, go to state 2
      if (myGPS.newData)
      {
        state = 2;
      }

      // If sleepFlag is tripped, go to state 3
      if (sleepFlag.get())
      {
        state = 3;
      }
    }

    // Read
    else if (state == 2)
    {
      myGPS.update(myClock);

      latitude.put(myGPS.latitude);
      longitude.put(myGPS.longitude);
      altitude.put(myGPS.altitude);
      fixType.put(myGPS.fixType);

      // If the GPS has a fix, use the clock to set the time
      if (myGPS.fixType)
      {
        unixTime.put(myGPS.getUnixTime(myClock));
        displayTime.put(myGPS.getDisplayTime(myClock));
      }

      // Otherwise show zero
      else
      {
        unixTime.put(String(0));
        displayTime.put("NaT");
      }
      
      state = 1;
    }

    // Sleep
    else if (state == 3)
    {
      // Calculate sleep time
      sleepTime.put(myGPS.getSleepTime(myClock, MINUTE_ALLIGN.get(), READ_TIME.get()));

      // Disable GPS
      myGPS.sleep(myClock);
      clockSleepReady.put(true);
    }

    // Get fix
    else if (state == 4)
    {
      wakeCounter = 0;
      wakeReady.put(myGPS.getFix(myClock, FIX_DELAY));

      state = 1;
    }

    clockCheck.put(true);
    vTaskDelay(CLOCK_PERIOD);
  }
}