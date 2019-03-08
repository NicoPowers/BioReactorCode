#include <SD.h>
#include "hx711.h"
#include <hardwareSerial.h>

File myFile;

int count = 1;

float averageValue = 0;

float tempPressureValues[30];
float averagePressureValue;
int tempFlowRate;

float ratio = 102.0 / 8387415.50;

String input;

char decision;

int flowRates[6] = {30, 40, 50, 60, 70, 80};
float pumpPressures[6] = {0, 0, 0, 0, 0, 0};

Hx711 sensor(A1, A0);

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial2.begin(9600);

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
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0)
  {
    input = Serial.readString();
    decision = input.charAt(0);
    if (decision == 'r')
    {
      for (int i = 0; i < 30; i++)
      {
        tempPressureValues[i] = sensor.averageValue();
        Serial.print(i + 1);
        Serial.print(" pressure sensor reading = ");
        Serial.println(tempPressureValues[i]);
      }
      for (int i = 0; i < 30; i++)
      {
        averagePressureValue += tempPressureValues[i];
      }
      averagePressureValue /= 30.0;
      Serial.print("Got an average pressure of: ");
      Serial.println(averagePressureValue);
      Serial.println("What flowrate does this correspond to?");
      while (Serial.available() == 0)
      {
      }
      tempFlowRate = Serial.parseInt();
      switch (tempFlowRate)
      {
      case 30:
        pumpPressures[0] = averagePressureValue;
        break;
      case 40:
        pumpPressures[1] = averagePressureValue;
        break;
      case 50:
        pumpPressures[2] = averagePressureValue;
        break;
      case 60:
        pumpPressures[3] = averagePressureValue;
        break;
      case 70:
        pumpPressures[4] = averagePressureValue;
        break;
      case 80:
        pumpPressures[5] = averagePressureValue;
      }
    }
    else if (decision == 'y')
    {
      for (int i = 0; i < 6; i++)
      {
        myFile.print(flowRates[i]);
        myFile.print(',');
        myFile.println(pumpPressures[i]);
      }
      myFile.close();
      Serial.println("DONE!");
    }
    else
    {
      Serial2.println(input);
    }
    Serial.flush();
  }
  else if (Serial2.available() > 0)
  {
    input = Serial2.readString();
    decision = input.charAt(0);
    if (decision == 'r')
    {
      readSensor();
    }
    else
    {
      Serial.println(input);
    }
    Serial2.flush();
  }
}

void readSensor()
{
  Serial.println((ratio * sensor.averageValue()) - 102.0, 3);
}
