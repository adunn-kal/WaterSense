#include <Arduino.h>
#include "sdData.h"

#define FORMAT_BUF_SIZE 200

SD_Data myCard(GPIO_NUM_5);
File myFile;

char format_buf[FORMAT_BUF_SIZE];
int startTime;

int32_t distance = 500;
float temp = 32.2;
float hum = 45.5;



void setup()
{
  Serial.begin(115200);
  while(!Serial) {}
  delay(2000);
  startTime = snprintf(format_buf, FORMAT_BUF_SIZE, "123.45");

  myCard.writeHeader();
  Serial.println("Done writing header file");
  delay(2000);

  String uTime = "123";
  myFile = myCard.createFile(1, 30, uTime);
  Serial.println("Done creating file");

}

void loop()
{
  
  for (int i = 0; i < 10; i++)
  {
    delay(1000);
    String time = "123";
    myCard.writeData(myFile, distance, time, temp, hum, 1);
    Serial.printf("Done writing file %d\n", i+1);
  }
  myFile.close();
  Serial.println("Done writing");
  delay(10000);

}