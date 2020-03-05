/*
This code was used to establish the relationship between the height of the water vial with the hydrostatic pressure
*/

#include <SD.h>
#include "hx711.h"
#include <hardwareSerial.h>

File myFile;

bool go = false;

float ratio = 102.0 / 8387415.50;

String input;

char decision;

float distanceFromHome = 0.0;

Hx711 sensor(A1, A0);

void setup()
{
  // put your setup code here, to run once:
  delay(10000);

  Serial.begin(9600);
  Serial1.begin(9600); // connection to UNO

  if (!SD.begin(53))
  {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("pumpData.csv", FILE_WRITE);
}

void loop()
{
  while (Serial.available() == 0)
  {
    readSensor();
  }
  if (go)
  {
    int distance = 0;
    while (distance < 150)
    {
      Serial1.print('u');
      Serial1.println(10);
      distance += 10;
      Serial.println("just told stepper to move");
      delay(5000);
      Serial.println("recorded sensor data");
      myFile.print(distanceFromHome);
      myFile.print(',');
      myFile.println(getSensorReading());
    }
    myFile.close();
    Serial.println("DONE");
    while (true)
    {
    }
  }
  if (Serial.available() > 0)
  {
    input = Serial.readString();
    decision = input.charAt(0);
    if (decision == 'g')
    {
      go = true;
    }
  }
}

void readSensor()
{
  Serial.println((ratio * sensor.averageValue()) - 102.0, 3);
}

float getSensorReading()
{
  return ((ratio * sensor.averageValue()) - 102.0);
}
