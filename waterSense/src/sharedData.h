/**
 * @file sharedData.h
 * @author Alexander Dunn
 * @brief A file to contain all shared variables
 * @version 0.1
 * @date 2023-02-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "waterSenseLibs/shares/taskshare.h"


//-----------------------------------------------------------------------------------------------------||
//---------- Shares & Queues --------------------------------------------------------------------------||

// Flags
extern Share<bool> dataReady;
extern Share<bool> sleepFlag;
extern Share<bool> clockSleepReady;
extern Share<bool> sonarSleepReady;
extern Share<bool> tempSleepReady;
extern Share<bool> sdSleepReady;

// Shares from GPS Clock
extern Share<float> latitude;
extern Share<float> longitude;
extern Share<float> altitude;
extern Share<uint8_t> fixType;
extern Share<String> unixTime;
extern Share<String> displayTime;
extern Share<bool> wakeReady;
extern Share<uint64_t> sleepTime;

// Shares from sensors
extern Share<int16_t> distance;
extern Share<float> temperature;
extern Share<float> humidity;

// Duty Cycle
extern Share<float> solar;
extern Share<float> battery;
extern Share<uint16_t> READ_TIME;
extern Share<uint16_t> MINUTE_ALLIGN;


//-----------------------------------------------------------------------------------------------------||
//-----------------------------------------------------------------------------------------------------||