/*
  Authors: Nicolas Montoya
  Credits: Bryan James
  Status: Prototype
  This code is ran on the Mega and is meant to control the Nano and the UNO; which allows you to control the MasterFlex Pump, stretcher, and hydrostatic pressure vial

  Refer to the README.md to see what commands can be sent to the UNO flow controller and the NANO stretch controller.


*/
#include "hx711.h"
#include <hardwareSerial.h>

int count = 1;

// PendoTech Sensor ratio (calibrated value)
float ratio = 102.0 / 8387415.50;

String input;

char decision;

Hx711 sensor(A1, A0);

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600); // serial connection to UNO
  Serial2.begin(9600); // serial connection to nano
  Serial.println("MEGA: Ready...");
  Serial.flush();
  delay(500);
}

void loop()
{

  // while user has not sent a command to the NANO or UNO, check to see if
  // message received from UNO or NANO
  if (Serial.available() == 0)
  {
    // check to see if received message from UNO
    if ((Serial1.available() > 0) && Serial2.available() == 0)
    {
      Serial.println(Serial1.readString());
      Serial.flush();
      while (Serial1.available())
      {
        Serial1.read();
      }
    }
    // check to see if received message from NANO
    else if ((Serial2.available() > 0) && Serial1.available() == 0)
    {
      Serial.println(Serial2.readString());
      Serial.flush();
      while (Serial2.available())
      {
        Serial2.read();
      }
    }
  }

  // if user is going to send command to either the UNO or NANO
  else if (Serial.available() > 0)
  {
    input = Serial.readString();
    decision = input.charAt(0);
    if (decision == 'n')
    {
      Serial2.println(input.substring(1));
      Serial2.flush();
    }
    else if (decision == 'u')
    {
      Serial1.println(input.substring(1));
      Serial1.flush();
    }
    while (Serial.available())
    {
      Serial.read();
    }
  }
}

// void readSensor()
// {
//   Serial.println(getSensorReading(), 3);
// }

// float getSensorReading()
// {
//   return ((ratio * sensor.getValue()) - 102.0);
// }
