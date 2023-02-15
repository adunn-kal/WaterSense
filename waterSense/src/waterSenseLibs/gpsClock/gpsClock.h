/**
 * @file gpsClock.h
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
#include "ESP32Time.h"

class GpsClock
{
    protected:
        uint16_t MONITOR_SPEED = 9600; ///< Serial port baud rate
        HardwareSerial* serialPort; ///< A pointer to a serial object
        gpio_num_t RX;
        gpio_num_t TX;
        gpio_num_t EN;

        uint32_t UNIX_OFFSET = 28800; // 8 hours

    public:
        uint32_t unix;
        uint32_t lastGpsUnix;
        uint32_t internalStart;

        bool newData = false; ///< A flag to represent when new data has arrived
        uint32_t millisOffset = 0; ///< The millisecond offset between the the mcu and the GPS
        float latitude; ///< Signed latitude in degrees.minutes
        float longitude; ///< Signed longitude in degrees.minutes
        float altitude; ///< Altitude in meters above MSL
        
        /**
         * @brief The fix type of the reciever
         * @details 0: No fix, 1: GPS fix, 2: DGPS Fix
         * 
         */
        uint8_t fixType;

        GpsClock(HardwareSerial *ser, gpio_num_t rxPin, gpio_num_t txPin, gpio_num_t enPin); ///< Constructor

        Adafruit_GPS begin(); ///< A method to start the GPS module

        bool getFix(Adafruit_GPS &GPS, uint32_t waitTime); ///< A method to wait for a GPS fix
        void update(Adafruit_GPS &GPS); ///< A method to run and check the GPS module for new data
        void read(Adafruit_GPS &GPS); ///< A method to read data from the GPS module
        String getUnixTime(Adafruit_GPS &GPS); ///< A method to get a current Unix timestamp from the GPS
        String getDisplayTime(Adafruit_GPS &GPS); ///< A method to get a current human readable timestamp from the GPS
        uint64_t getSleepTime(Adafruit_GPS &GPS, uint16_t MINUTE_ALLLIGN, uint16_t READ_TIME); ///< A method to get the sleep time
        void sleep(Adafruit_GPS &GPS); ///< A method to put the GPS module to sleep

        void updateInternal(Adafruit_GPS &GPS, ESP32Time &RTC);
        String getUnixInternal(ESP32Time &RTC);
        String getDisplayInternal(ESP32Time &RTC);
        uint64_t getSleepInternal(ESP32Time &RTC, uint16_t MINUTE_ALLLIGN, uint16_t READ_TIME); ///< A method to get the sleep time
};