/*
This piece of code was used to generate the relationship between the flow rate of the pump to the pump pressure
*/

#include <SD.h>
#include "hx711.h"
#include <hardwareSerial.h>

File myFile;

int count = 1;

float averageValue = 0;

float tempPressureValues[10];
float averagePressureValue;
int tempFlowRate;

int trial1[6] = {0, 0, 0, 0, 0, 0};
int trial2[6] = {0, 0, 0, 0, 0, 0};
int trial3[6] = {0, 0, 0, 0, 0, 0};

int currentTrial = 1;
int flowRateIndex;

bool startingTrial = true;
bool performingFlowRate = true;

float ratio = 102.0 / 8387415.50;

String input;

char decision;

int flowRates[6] = {30, 40, 50, 60, 70, 80};
float pumpPressures[6] = {0, 0, 0, 0, 0, 0};

Hx711 sensor(A1, A0);

void setup()
{
  // put your setup code here, to run once:
  randomSeed(analogRead(A3));
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
  if (!determineTrialFlowRate())
  {
    Serial.println("All trials are done!");
    myFile.close();
    while (true)
    {
    }
  }
  else
  {
    performingFlowRate = true;
  }

  while (performingFlowRate)
  {
    while ((Serial.available() == 0) && (Serial1.available() == 0))
    {
      readSensor();
    }
    // put your main code here, to run repeatedly:
    if (Serial.available() > 0)
    {
      input = Serial.readString();
      decision = input.charAt(0);
      if (decision == 'r')
      {

        for (int i = 0; i < 10; i++)
        {
          tempPressureValues[i] = getSensorReading();
          Serial.print(i + 1);
          Serial.print(" pressure sensor reading = ");
          Serial.println(tempPressureValues[i]);
        }
        for (int i = 0; i < 10; i++)
        {
          averagePressureValue += tempPressureValues[i];
        }
        averagePressureValue /= 10.0;

        Serial.print("Got an average pressure of: ");
        Serial.print(averagePressureValue);
        Serial.print(" for a flow rate of: ");
        Serial.print(tempFlowRate);
        Serial.print(" for trial: ");
        Serial.println(currentTrial);

        switch (currentTrial)
        {
        case 1:
          trial1[flowRateIndex] = 1;
          break;
        case 2:
          trial2[flowRateIndex] = 1;
          break;
        case 3:
          trial3[flowRateIndex] = 1;
          break;
        }
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
        averagePressureValue = 0.0;
        performingFlowRate = false;
      }
      else
      {
        Serial1.println(input);
      }
      Serial.flush();
    }
    else if (Serial1.available() > 0) //  if received data from UNO
    {
      input = Serial1.readString();
      decision = input.charAt(0);
      if (decision == 'r')
      {
        readSensor();
      }
      else
      {
        Serial.println(input);
      }
      Serial1.flush();
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

bool determineTrialFlowRate()
{

  flowRateIndex = random(6);
  bool trial1Done = true;
  bool trial2Done = true;
  bool trial3Done = true;
  bool allTrialsDone = true;

  for (int i = 0; i < 6; i++)
  {
    if (trial1[i] == 0)
    {
      trial1Done = false;
    }
  }
  for (int i = 0; i < 6; i++)
  {
    if (trial2[i] == 0)
    {
      trial2Done = false;
    }
  }
  for (int i = 0; i < 6; i++)
  {
    if (trial3[i] == 0)
    {
      trial3Done = false;
    }
  }
  if ((trial1Done && trial2Done) && trial3Done)
  {
    for (int i = 0; i < 6; i++)
    {
      myFile.print(flowRates[i]);
      myFile.print(',');
      myFile.print(pumpPressures[i]);
      myFile.print(',');
      myFile.println(currentTrial);
    }
    Serial.print("Done inserting data for flow rate: ");
    Serial.print(tempFlowRate);
    Serial.print(" trial: ");
    Serial.println(currentTrial);
    return false;
  }
  else if (trial1Done && currentTrial < 2)
  {
    Serial.println("Trial 1 done");
    for (int i = 0; i < 6; i++)
    {
      myFile.print(flowRates[i]);
      myFile.print(',');
      myFile.print(pumpPressures[i]);
      myFile.print(',');
      myFile.println(currentTrial);
    }
    Serial.print("Done inserting data for flow rate: ");
    Serial.print(tempFlowRate);
    Serial.print(" trial: ");
    Serial.println(currentTrial);
    currentTrial += 1;
  }
  else if (trial2Done && currentTrial < 3)
  {
    Serial.println("Trial 2 done");
    for (int i = 0; i < 6; i++)
    {
      myFile.print(flowRates[i]);
      myFile.print(',');
      myFile.print(pumpPressures[i]);
      myFile.print(',');
      myFile.println(currentTrial);
    }
    Serial.print("Done inserting data for flow rate: ");
    Serial.print(tempFlowRate);
    Serial.print(" trial: ");
    Serial.println(currentTrial);
    currentTrial += 1;
  }

  while (true)
  {
    tempFlowRate = flowRates[flowRateIndex];
    if (currentTrial == 1 && trial1[flowRateIndex] == 0)
    {
      Serial.print("Set pump to ");
      Serial.print(tempFlowRate);
      Serial.print(" for trial ");
      Serial.println(currentTrial);
      return true;
    }
    else if (currentTrial == 2 && trial2[flowRateIndex] == 0)
    {
      Serial.print("Set pump to ");
      Serial.print(tempFlowRate);
      Serial.print(" for trial ");
      Serial.println(currentTrial);
      return true;
    }
    else if (currentTrial == 3 && trial3[flowRateIndex] == 0)
    {
      Serial.print("Set pump to ");
      Serial.print(tempFlowRate);
      Serial.print(" for trial ");
      Serial.println(currentTrial);
      return true;
    }
    else
    {
      flowRateIndex = random(6);
    }
  }
}
