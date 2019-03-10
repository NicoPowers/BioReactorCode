
#include "hx711.h"
#include <hardwareSerial.h>

int count = 1;

float ratio = 102.0 / 8387415.50;

String input;

char decision;

int flowRates[6] = {30, 40, 50, 60, 70, 80};
float pumpPressures[6] = {0, 0, 0, 0, 0, 0};

Hx711 sensor(A1, A0);

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600); // connection to UNO
}

void loop()
{

  while (Serial.available() == 0)
  {
    readSensor();
  }
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0)
  {
    input = Serial.readString();
    decision = input.charAt(0);
    Serial1.println(input);
  }
  Serial.flush();
  Serial1.flush();
}

void readSensor()
{
  Serial.println((ratio * sensor.averageValue()) - 102.0, 3);
}

float getSensorReading()
{
  return ((ratio * sensor.averageValue()) - 102.0);
}
