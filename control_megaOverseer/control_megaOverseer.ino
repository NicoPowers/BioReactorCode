/* This code is ran on the Mega and is meant to control the Nano and the UNO; which allows you to control the MasterFlex Pump, stretcher, and hydrostatic pressure vial

To send commands to the Nano, allow commands must start with a lowercase "n" (to signify nano); below are the following commands that you can send to the nano:

"ns" to set the current position as the Home position
"nh" to return to the home position
"nf%f" to set the frequency for the oscillation where %f is the value you want to set it to
"no%f" to move outwards a distance %f
"ni%f" to move inwards a distance %f
"nq" to toggle the state of the MasterFlex Pump
"ne%f" to set the flow rate for the MasterFlex Pump, where %f ranges from 0 to 80
"nx" to cancel the oscillation and return stretcher to limit switch
"nr%f" to start oscillation where %f is the repeating distance (in mm) of oscillation

Likewise, to the UNO, allow commands must start with a lowercase "u" (to signify UNO); below are the following commands that you can send to the UNO:

"ud%f" to move the vial down a certain distance %f in mm
"uu%f" to move the vial up a certain distance %f in mm
"uh" to move back to the home position
"us" to set the current position as the home position
"uc" to check the current distance from the home position

*/
#include "hx711.h"
#include <hardwareSerial.h>

int count = 1;

// PendoTech Sensor ratio
float ratio = 102.0 / 8387415.50;

String input;

char decision;

int flowRates[6] = {30, 40, 50, 60, 70, 80};
float pumpPressures[6] = {0, 0, 0, 0, 0, 0};

Hx711 sensor(A1, A0);

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600); // serial connection to UNO
  Serial2.begin(9600); // serial connection to nano
}

void loop()
{

  while (Serial.available() == 0)
  {
    readSensor();
    delay(100);
  }

  if (Serial.available() > 0)
  {
    input = Serial.readString();
    decision = input.charAt(0);
    if (decision == 'n')
    {
      Serial2.println(input.substring(1));
    }
    else if (decision == 'u')
    {
      Serial1.println(input.substring(1));
    }
  }
  Serial.flush();
  Serial1.flush();
  Serial2.flush();
}

void readSensor()
{
  Serial.println(getSensorReading(), 3);
}

float getSensorReading()
{
  return ((ratio * sensor.getValue()) - 102.0);
}
