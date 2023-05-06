/**
 * @file sdData.cpp
 * @author Alexander Dunn
 * @version 0.1
 * @date 2022-12-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Arduino.h>
#include "sharedData.h"
#include "SD.h"
#include <utility>
#include "sdData.h"


/**
 * @brief A constructor for the SD_Data class
 * 
 * @param pin The pin used for the SD chip select
 * @return SD_Data 
 */
SD_Data :: SD_Data(gpio_num_t pin)
{
    // Update CS pin
    CS = pin;

    pinMode(LED, OUTPUT);

    // Start SD stuff
    pinMode(CS, OUTPUT);

    // uint16_t longTimer = millis();
    uint16_t timer = millis();
    while (!SD.begin(CS))
    {
        // // Restart if more than 10 seconds
        // if ((millis() - longTimer) > 10000)
        // {
        //     assert(false);
        // }

        // Blink LED
        if ((millis() - timer) > 300)
        {
            timer = millis();
            digitalWrite(LED, 1-digitalRead(LED));
        }
    }
    digitalWrite(LED, LOW);
}

/**
 * @brief A method to check and write header files to the SD card
 * 
 */
void SD_Data :: writeHeader()
{
    // Check if file exists and create one if not
    if (!SD.exists("/README.txt"))
    {
        File read_me = SD.open("/README.txt", FILE_WRITE);
        if(!read_me) return;

        // Create header with title, timestamp, and column names
        read_me.println("");
        read_me.printf(
            "Cal Poly Tide Sensor\n"
            "https://github.com/adunn-kal/workSoftware/tree/master/waterSense\n\n"
            "Data File format:\n"
            "UNIX Time (GMT), Distance (mm), External Temp (F), Humidity (%), Battery Voltage (V), Solar Panel Voltage (V)\n");
        read_me.close();

        SD.mkdir("/Data");
    }
}

/**
 * @brief Open a new file
 * 
 * @param hasFix Whether or not the GPS has a fix
 * @param wakeCounter The number of wake cycles
 * @param time The current unix timestamp
 * @return The opened file
 */
File SD_Data :: createFile(bool hasFix, uint32_t wakeCounter, String time)
{
    String fileName = "/Data/";

    // Filenames are at most 8 characters + 6("/Data/") + 4(".txt") + null terminator = 19
    if (hasFix) // If the gps has a fix, use its timestamp
    {
        fileName += String(time.toInt(), HEX);
        fileName += ".txt";
    }
    else if (lastKnownUnix) // If we have switched over to the RTC, use it
    {
        fileName += String(time.toInt(), HEX);
        fileName += ".txt";
    }
    else // If no GPS fix, use wake counter
    {
        fileName += String(wakeCounter, HEX);
        fileName += "_";
        fileName += String(millis(), HEX);
        fileName += ".txt";
    }

    File file = SD.open(fileName, FILE_WRITE, true);
    assert(file);
    return file;
}

/**
 * @brief A method to write a log message to the SD card
 * 
 * @param unixTime The unix timestamp
 * @param wakeCounter The number of times the MCU has woken from deep sleep
 * @param latitude The latitude as measured by the GPS
 * @param longitude The longitude as measured by the GPS
 * @param altitude The altitude as measured by the GPS
 */
void SD_Data :: writeLog(String unixTime, uint32_t wakeCounter, float latitude, float longitude, float altitude)
{
    //Open log file and write to it
    File logFile = SD.open("/logFile.txt", FILE_WRITE);
    if(!logFile) return;

    logFile.printf("Wake Number: %d\n", wakeCounter);
    logFile.println("UNIX Time (GMT), Latitude (decimal degrees), Longitude(decimal degrees), Altitude (meters above MSL)");
    logFile.print(unixTime);
    logFile.printf(", %0.5f, %0.5f, %0.2f\n", latitude, longitude, altitude);
    logFile.close();
}

/**
 * @brief A method to take a write data to the SD card
 * 
 * @param data_file A reference to the data file to be written to
 * @param distance The distance measured by the SONAR sensor
 * @param unixTime The unix timestamp for when the data was recorded
 * @param temperature The current temperature measured by the temperature and humidity sensor
 * @param humidity The current humidity measured by the temperature and humidity sensor
 * @param solarVoltage Voltage of solar panel
 * @return sensorData An object containing all of the data
 */
void SD_Data :: writeData(File &dataFile, int32_t distance, String unixTime, float temperature, float humidity, float batteryVoltage, float solarVoltage)
{
    dataFile.print(unixTime);
    dataFile.printf(", %d, %0.2f, %0.2f, %0.2f, %0.2f\n", distance, temperature, humidity, batteryVoltage, solarVoltage);
}

/**
 * @brief A method to close the current file and put the device to sleep
 * 
 * @param dataFile The file to close
 */
void SD_Data :: sleep(File &dataFile)
{
    dataFile.close();
}