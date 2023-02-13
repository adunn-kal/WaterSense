/**
 * @file gpsClock.cpp
 * @author Alexander Dunn
 * @version 0.1
 * @date 2022-12-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Arduino.h>
#include "Adafruit_GPS.h"
#include "UnixTime.h"
#include "gpsClock.h"

/**
 * @brief A constructor for an object of the gpsClock class
 * 
 * @param ser A pointer to the serial port to be used by the GPS module
 * @param rxPin The receive pin of the GPS module
 * @param txPin The transmit pin of the GPS module
 * @param enPin The enable pin of the GPS module
 * @return GpsClock A gpsClock object
 */
GpsClock :: GpsClock(HardwareSerial *ser, gpio_num_t rxPin, gpio_num_t txPin, gpio_num_t enPin)
{
    serialPort = ser;
    RX = rxPin;
    TX = txPin;
    EN = enPin;
}

/**
 * @brief A method to start the GPS module
 * @details Enables only the RMC NMEA data for speed purposes
 * 
 * @return Adafruit_GPS A GPS object to be used for data collection
 */
Adafruit_GPS GpsClock :: begin()
{
    // Start serial port
    serialPort->begin(MONITOR_SPEED, SERIAL_8N1, RX, TX);
    Adafruit_GPS GPS(serialPort);

    gpio_hold_dis(EN);
    pinMode(EN, OUTPUT);
    digitalWrite(EN, HIGH);
    // GPS.wakeup(); // Testing wakeup vs standby

    // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
    GPS.begin(9600);

    // Uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    //gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);

    // Set the update rate
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);

    return GPS;
}

/**
 * @brief A method to wait until the GPS has a fix
 * 
 * @param gps 
 * @param waitTime 
 * @return bool A true or false indicator that the method is done
 */
bool GpsClock :: getFix(Adafruit_GPS &gps, uint32_t waitTime)
{
  Serial.printf("Waiting %d seconds for GPS fix\n", waitTime);
  
  uint32_t start = millis();
  uint8_t fixType = 0;

  // Wait a minute to try and get a fix
  while ((millis() - start) < waitTime*1000)
  {
    gps.read();
    if(gps.newNMEAreceived())
    {
      gps.parse(gps.lastNMEA());  // this also sets the newNMEAreceived() flag to false
      fixType = gps.fixquality;
    }
    
    if (fixType)
    {
      Serial.printf("GPS fix found after %d seconds\n", (millis()-start)/1000);
      return true;
    }
  }

  return true;
}

/**
 * @brief A method to run and check the GPS module for new data
 * 
 * @param GPS A reference to a GPS object to update
 */
void GpsClock :: update(Adafruit_GPS &GPS)
{
    // Check the GPS for new messages
    for (uint8_t i = 0; i < 100; i++)
    {
        GPS.read();
    }

    // If a new message has arrived
    if(GPS.newNMEAreceived())
    {
        // Parse the message
        GPS.parse(GPS.lastNMEA());
        newData = true;

        millisOffset = (millis() - GPS.milliseconds) % 1000; // Update the millisecond offset

        fixType = GPS.fixquality; // Update the fix type

        // if (GPS.lat == 'N') latitude = GPS.latitudeDegrees; // Update the latitude
        // else latitude = -1 * GPS.latitudeDegrees;
        latitude = GPS.latitudeDegrees; // Update the latitude

        // if (GPS.lon == 'E') longitude = GPS.longitudeDegrees; // Update the longitude
        // else longitude = -1 * GPS.longitudeDegrees;
        longitude = GPS.longitudeDegrees; // Update the longitude

        altitude = GPS.altitude; // Update the altitude (meters above MSL)
    }
}

/**
 * @brief A method to read data from the GPS
 * @details only runs if new data is available
 * 
 * @param GPS A reference to a GPS object to update
 */
void GpsClock :: read(Adafruit_GPS &GPS)
{
    if (newData)
    {
        newData = false;

        millisOffset = millis() - GPS.milliseconds; // Update the millisecond offset

        fixType = GPS.fixquality; // Update the fix type

        if (GPS.lat == 'N') latitude = GPS.latitude; // Update the latitude
        else latitude = -1 * GPS.latitude;

        if (GPS.lon == 'E') longitude = GPS.longitude; // Update the longitude
        else longitude = -1 * GPS.longitude;

        altitude = GPS.altitude; // Update the latitude
    }
}

/**
 * @brief Get a current Unix timestamp
 * 
 * @param GPS A reference to the GPS object from which to pull time data
 * @return String A Unix timestamp
 */
String GpsClock :: getUnixTime(Adafruit_GPS &GPS)
{
    UnixTime stamp(0);
    stamp.setDateTime(2000 + GPS.year, GPS.month, GPS.day, GPS.hour, GPS.minute, GPS.seconds);

    unix = stamp.getUnix() + UNIX_OFFSET;

    uint32_t myMillis = (millis() - millisOffset)%1000;
    String unixString = String(unix) + ".";
    if (myMillis < 100) unixString += "0";
    if (myMillis < 10) unixString += "0";
    unixString += String(myMillis);


    return unixString;
}

/**
 * @brief Get a current displayable timestamp
 * 
 * @param GPS A reference to the GPS object from which to pull time data
 * @return String A displayable timestamp
 */
String GpsClock :: getDisplayTime(Adafruit_GPS &GPS)
{
    UnixTime stamp(0);
    stamp.setDateTime(2000 + GPS.year, GPS.month, GPS.day, GPS.hour, GPS.minute, GPS.seconds);

    uint32_t unix = stamp.getUnix() + UNIX_OFFSET;

    
    String displayString = String(GPS.hour) + ":";

    if (GPS.minute < 10) displayString += "0";
    displayString += String(GPS.minute) + ":";

    if (GPS.seconds < 10) displayString += "0";
    displayString += String(GPS.seconds) + ".";

    uint32_t myMillis = (millis() - millisOffset)%1000;
    if (myMillis < 100) displayString += "0";
    if (myMillis < 10) displayString += "0";
    displayString += String(myMillis);

    return displayString;
}

/**
 * @brief Calculate the time to sleep
 * 
 * @param GPS The GPS object used to determine the time
 * @param MINUTE_ALLIGN The minute allignment for data readings to be centered about
 * @param READ_TIME The number of seconds to read data
 * @return uint64_t The number of microseconds to sleep
 */
uint64_t GpsClock :: getSleepTime(Adafruit_GPS &GPS, uint8_t MINUTE_ALLIGN, uint8_t READ_TIME)
{
    // If the gps has a fix, use it to calculate the next read time
    if (fixType)
    {
        // Calculate next time in uS until next measurement interval
        uint64_t next_measurement = (MINUTE_ALLIGN - (GPS.minute % MINUTE_ALLIGN)) * (60 * 1000000);
        next_measurement -= GPS.seconds * 1000000;
        // next_measurement -= ((millis() - millisOffset) % 1000) * 1000;

        // Subtract off half of the read time
        if (next_measurement > (READ_TIME*1000000/2))
        {
            next_measurement -= READ_TIME*1000000/2;
            return next_measurement;
        }

        else return 1;
    }

    // Otherwise, just sleep for the set time
    else
    {
        return (MINUTE_ALLIGN*60*1000000);
    }
    
}

/**
 * @brief A method to put the sensor to sleep
 * 
 * @param gps The GPS object to be put to sleep
 */
void GpsClock :: sleep(Adafruit_GPS &gps)
{
    digitalWrite(EN, LOW);
    gpio_hold_en(EN);
    // gps.standby();
}


void GpsClock :: updateInternal(Adafruit_GPS &GPS, ESP32Time &RTC)
{
    uint32_t myMillis = (millis() - millisOffset)%1000;
    uint8_t mySecond = GPS.seconds;
    uint8_t myMinute = GPS.minute;
    uint8_t myHour = GPS.hour;
    uint8_t myDay = GPS.day;
    uint8_t myMonth = GPS.month;
    uint16_t myYear = 2000 + GPS.year;

    // Serial.printf("%d %d, %d\n", myMonth, myDay, myYear);
    // Serial.printf("%d:%d:%d.%3d\n", myHour, myMinute, mySecond, myMillis);

    // Does not work for date, but will get the time correct for display time
    RTC.setTime(mySecond, myMinute, myHour, myDay, myMonth, myYear, myMillis);

    lastGpsUnix = unix;
    internalStart = RTC.getEpoch();
}


String GpsClock :: getUnixInternal(ESP32Time &RTC)
{
    // uint32_t unix = RTC.getEpoch();
    // String unixString = String(unix);

    // Update unix counter
    uint32_t diff = RTC.getEpoch() - internalStart;
    unix = lastGpsUnix + diff;

    uint32_t myMillis = RTC.getMillis();

    String unixString = String(unix) + ".";

    if (myMillis < 100) unixString += "0";
    if (myMillis < 10) unixString += "0";
    unixString += String(myMillis);


    return unixString;
}

String GpsClock :: getDisplayInternal(ESP32Time &RTC)
{
    String displayString = RTC.getTime() + ".";

    uint32_t myMillis = RTC.getMillis();
    if (myMillis < 100) displayString += "0";
    if (myMillis < 10) displayString += "0";
    displayString += String(myMillis);

    return displayString;
}

uint64_t GpsClock :: getSleepInternal(ESP32Time &RTC, uint8_t MINUTE_ALLIGN, uint8_t READ_TIME)
{
    // Calculate next time in uS until next measurement interval
    uint64_t next_measurement = (MINUTE_ALLIGN - (RTC.getMinute() % MINUTE_ALLIGN)) * (60 * 1000000);
    next_measurement -= RTC.getSecond() * 1000000;
    next_measurement -= (RTC.getMillis() % 1000) * 1000;

    // Subtract off half of the read time
    if (next_measurement > (READ_TIME*1000000/2))
    {
        next_measurement -= READ_TIME*1000000/2;
        return next_measurement;
    }

    else return 1;
}