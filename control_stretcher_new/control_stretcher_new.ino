/* This code controls the bidirectional stepper and the MasterFlex Pump at a constant flow rate
  The commands that can be given to the nano from the mega (control_megaOverseer.ino) include:
  "nd%f" to set the initial displacement
  "nf%f" to set the frequency for the oscillation where %f is the value you want to set it to
  "no%d" to move outwards a distance %f
  "ni%f" to move inwards a distance %f
  "nq" to toggle the state of the MasterFlex Pump
  "ne%f" to set the flow rate for the MasterFlex Pump, where %f ranges from 0 to 80
  "nx" to cancel the oscillation and return stretcher to limit switch
  "nr%f" to start oscillation where %f is the repeating distance (in mm) of oscillation
  "np%f" to set the phase of the sinusoidal flow rate
  "ns%f" to set the vertical shift of the sinusoidal flow rate
  "na%f" to set the steps of the sinusoidal flow rate
  This does 1 Hz Frequency only at the following distances:
  5% Strain:
  1 mm
  1.25 mm
  1.50 mm
  1.75 mm
  2.0 mm
  10% Strain: (enter as half distance)
  7 mm
  7.25 mm
  7.5 mm
  7.75 mm
  8.0 mm
*/

#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <SPI.h>
#include "Adafruit_FRAM_SPI.h"

/* Example code for the Adafruit SPI FRAM breakout */
uint8_t FRAM_CS = 10;

uint8_t FRAM_SCK = 13;
uint8_t FRAM_MISO = 12;
uint8_t FRAM_MOSI = 11;

uint16_t addr = 0;
// 1, 1.25, 1.5, 1.75
float stretchingDistances[10] = {1.0, 1.25, 1.50, 1.75, 2.00, 7.00, 7.25, 7.50, 7.75, 8.00};
float stretchingSpeedConstants[10] = {1.0, 1.00, 1.00, 1.00, 1.00, 500.00, 1.00, 1.00, 1.00, 1.00};
float stretchingAccelerationConstants[10] = {7.3, 7.46, 7.55, 7.52, 7.58, 100.00, 1.00, 1.00, 1.00, 1.00};

float steps, mm = 0.0, repeatingDistance = 0.0, maxFlowRate = 20.0, phase = 0.0, shift = 0.0;
float frequency = 0.5;

float initialDistance = 2.0; // mm start distance

long unsigned int t_1, t_2, lastHit = 0;

volatile bool repeating = false, pumpOn = false;

String input;

char decision;
// Define a stepper and the pins it will use
// Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
AccelStepper stepper(1, 4, 7);

SoftwareSerial mySerial(2, 3); // RX, TX

Adafruit_MCP4725 dac;
Adafruit_FRAM_SPI fram = Adafruit_FRAM_SPI(FRAM_SCK, FRAM_MISO, FRAM_MOSI, FRAM_CS);

void setup()
{
  // Sets pin used to turn on/off MasterFlex Pump and turns it off
  pinMode(A1, OUTPUT);
  digitalWrite(A1, pumpOn);
  digitalWrite(A1, !pumpOn);

  // Begins serial interface for serial monitor and between mega
  Serial.begin(9600);
  mySerial.begin(9600);
  fram.begin();

  // Starts DAC to control MasterFlex Pump flow rate
  dac.begin(0x62);
  setFlowRate(0);
  pinMode(9, INPUT); // pin connected to limit switch
  stepper.setCurrentPosition(0);
  stepper.setMaxSpeed(10000);
  stepper.setAcceleration(1000);
  delay(2000);

  travelInwards(100.0);
  travelOutwards(initialDistance);
  Serial.println("Ready.");
}

void loop()
{

  while (mySerial.available() == 0)
  {

    if (repeating)
    {
      stretch(repeatingDistance, frequency);
    }
  }
  if (mySerial.available() > 0)
  {
    input = mySerial.readString();
    decision = input.charAt(0);

    if (decision == 'r')
    {
      travelInwards(100);
      travelOutwards(initialDistance);
      repeating = true;
      repeatingDistance = input.substring(1).toFloat();
    }
    else if (decision == 'p')
    {
      phase = input.substring(1).toFloat();
    }
    else if (decision == 's')
    {
      shift = input.substring(1).toFloat();
    }
    else if (decision == 'a')
    {
      maxFlowRate = input.substring(1).toFloat();
    }
    else if (decision == 'f')
    {
      frequency = input.substring(1).toFloat();
    }
    else if (decision == 'o')
    {
      travelOutwards(input.substring(1).toFloat());
    }
    else if (decision == 'd')
    {
      repeating = false;
      initialDistance = input.substring(1).toFloat();
      travelInwards(100.0);
      travelOutwards(initialDistance);
    }
    else if (decision == 'i')
    {
      travelInwards(input.substring(1).toFloat());
    }
    else if (decision == 'q')
    {
      pumpOn = !pumpOn;
      digitalWrite(A1, pumpOn);
      digitalWrite(A1, !pumpOn);
    }
    else if (decision == 'e')
    {
      setFlowRate(input.substring(1).toFloat());
    }
    else if (decision == 'x')
    {
      travelInwards(100);
      travelOutwards(initialDistance);
      repeating = false;
    }
  }

  mySerial.flush();
}

// sets the desired flow rate to the pump
void setFlowRate(float flowRate)
{
  float voltage = (float)(flowRate / 80.0) * 4095;
  dac.setVoltage(voltage, false);
}

// converts mm to steps, only works for bidirectional lead screw
float getSteps(float mm)
{
  return (float)(390.55 * mm); // 390.55 steps per mm
}

void travelOutwards(float mm)
{
  steps = getSteps(mm);
  stepper.setCurrentPosition(0);
  stepper.moveTo(-steps);
  while (stepper.distanceToGo() < 0)
  {
    stepper.run();
  }
}

void travelInwards(float mm)
{

  steps = getSteps(mm);
  stepper.setCurrentPosition(0);
  stepper.moveTo(steps);

  while (stepper.distanceToGo() > 0)
  {

    if ((digitalRead(9) == LOW && (millis() - lastHit >= 1000)))
    { // if stepper motor hit limit switch
      stepper.setCurrentPosition(0);
      lastHit = millis();
      return;
    }
    else
    {
      stepper.run();
    }
  }
}

void stretch(int stretchIndex, float frequency)
{

  float period = (1.0 / frequency); // s

  float steps = getSteps(stretchingDistances[stretchIndex]);

  float averageSpeed = 2 * (steps / period);
  float averageAcceleration = averageSpeed / period;

  long unsigned int t0 = 0, t1 = 0;

  // find the perfect acceleration constant until the correct oscillation period is found
  bool foundaccelMultiplier = false;
  bool firstRun = true;
  float accelMultiplier = 1.0;
  float maxResolutionDiff = 3.0;
  float prevError = 0.0;
  float error = 0.0;
  float maxError = 0.0;
  float k = 0.0;

  t0 = millis();
  stepper.setAcceleration(stretchingAccelerationConstants[stretchIndex] * averageAcceleration);
  stepper.setSpeed(stretchingSpeedConstants[stretchIndex] * averageSpeed);
  stepper.move(-round(steps)); // move outwards

  while (stepper.distanceToGo() < -1)
  {
    stepper.run();
  }

  stepper.move(round(steps)); // move back inwards (1 period of oscillation)

  while (stepper.distanceToGo() > 1)
  {
    stepper.run();
  }
  t1 = millis();

  Serial.println(t1 - t0);
}
