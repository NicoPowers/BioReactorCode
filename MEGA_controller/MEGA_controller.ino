/* 
Authors: Nicolas Montoya
Credits: Bryan James
Status: Prototype
This code is ran on the Mega and is meant to control the Nano and the UNO; which allows you to control the MasterFlex Pump, stretcher, and hydrostatic pressure vial

To send commands to the Nano, allow commands must start with a lowercase "n" (to signify nano); below are the following commands that you can send to the nano:

  "nd%f" to set the initial displacement
  "nf%f" to set the frequency for the oscillation where %f is the value you want to set it to
  "no%d" to move outwards a distance %f
  "ni%f" to move inwards a distance %f
  "nq" to toggle the state of the MasterFlex Pump
  "ne%f" to set the flow rate for the MasterFlex Pump, where %f ranges from 0 to 80
  "nx" to cancel the oscillation and return stretcher to limit switch
  "nr1%i" to start oscillation where %i is the repeating distance index (integer) of oscillation with the sinusoidal pump flow
  "nr0%i" to start oscillation where %f is the repeating distance index (integer) of oscillation without the sinusoidal pump flow
  REPEATING DISTANCES ALLOWED AT 1 HZ:
  [1.0, 1.25, 1.50, 1.75, 2.00, 7.00, 7.25, 7.50, 7.75, 8.00]
    0     1     2     3     4     5     6     7     8     9

  This does 1 Hz Frequency only at the following distances:
  5% Strain:
  1 mm
  1.25 mm
  1.50 mm
  1.75 mm
  2.0 mm
  10% Strain:
  7 mm
  7.25 mm
  7.5 mm
  7.75 mm
  8.0 mm

Likewise, for the UNO, all commands must start with a lowercase "u"(to signify UNO);
below are the following commands that you can send to the UNO :

"ud%f" to move the vial down a certain distance %f in mm
"uu%f" to move the vial up a certain distance %f in mm
"uh" to move back to the home position
"us" to set the current position as the home position
"uq" to toggle the state of the MasterFlex pump
"uc" to check the current distance from the home position
"ux" to set the current position of the water vial as the home position
"ua%f" to set the phase of the sinusoidal flow rate
"up%f" to set the phase of the sinusoidal flow rate
"us%f" to set the vertical shift of the sinusoidal flow rate
"ua%f" to set the max amplitude (max flow rate) of the sinusoidal flow rate
"ue%f" to set the manual (constant) flow rate (disables sinusoidal flow rate)
"ue%f" to enable sinusoidal flow rate (allows for NANO to tell UNO when to start
        sinusoidal flow rate, disables manual flow rate)


*/
#include "hx711.h"
#include <hardwareSerial.h>

int count = 1;

// PendoTech Sensor ratio (calibrated value)
float ratio = 102.0 / 8387415.50;

String input;

char decision;

// int flowRates[6] = {30, 40, 50, 60, 70, 80};
// float pumpPressures[6] = {0, 0, 0, 0, 0, 0};

Hx711 sensor(A1, A0);

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600); // serial connection to UNO
  Serial2.begin(9600); // serial connection to nano
}

void loop()
{

  // while user has not sent a command to the NANO or UNO, check to see if
  // message received from UNO or NANO
  while (Serial.available() == 0)
  {
    // check to see if received message from UNO
    if (Serial1.available() > 0)
    {
      Serial.println(Serial1.readString());
    }
    // check to see if received message from NANO
    if (Serial2.available() > 0)
    {
      Serial.println(Serial2.readString());
    }
  }

  // if user is going to send command to either the UNO or NANO
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

// void readSensor()
// {
//   Serial.println(getSensorReading(), 3);
// }

// float getSensorReading()
// {
//   return ((ratio * sensor.getValue()) - 102.0);
// }
