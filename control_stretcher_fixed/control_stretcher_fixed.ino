/* This code controls the bidirectional stepper and the MasterFlex Pump at a constant flow rate

  The commands that can be given to the nano from the mega (control_megaOverseer.ino) include:

  "ns" to set the current position as the Home position
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

uint16_t          addr = 0;

float frequency, steps, mm = 0.0, repeatingDistance = 0.0, maxFlowRate = 20.0, phase = 0.0, shift = 0.0;

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
  stepper.setMaxSpeed(10000000);
  stepper.setAcceleration(1000);
  delay(2000);

  travelInwards(100.0);
  Serial.println("Ready.");
  //  stretch(10, 1);
  //  while (1) {
  //
  //  }


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
      //travelInwards(100);
      //travelOutwards(1);
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
  steps = -getSteps(mm);
  stepper.setCurrentPosition(0);
  stepper.moveTo(steps);
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

void stretch(float distance, float frequency)
{
  long unsigned int t_1, t_2, t_3;
  float omega = PI * frequency / 1000.0; // rad/ms
  float period = (1.0 / frequency) * 1000.0; // ms
  float timeStep = 1; // ms

  float steps = getSteps(distance);
  float stepperSpeed;
  float flowRate;
  //  Serial.println(frequency, 4);
  //  Serial.println(omega, 4);
  //  Serial.println(period, 4);
  //  Serial.println(timeStep, 4);
  //  Serial.println(steps, 4);
  int numSamples = period / timeStep;
  //Serial.println(numSamples);
  float stepsPerSample  = (float) steps / numSamples;
  int midWay = numSamples / 2;
  int quarterWay = midWay / 2;
  //Serial.println(numSamples);
  uint16_t acceleration;
  float floatAccel, t;
  addr = 0;
  for (uint16_t i = 0; i < quarterWay + 1 ; i ++) { // only need to store half the values because the other way is just the same but negative
    t = (((float)i / (float)numSamples) * period);
    //Serial.print(t);
    floatAccel = steps * ((pow(cos((float)omega * t), 2.0)) - pow(sin((float)omega * t), 2.0)); // takes 10 ms to compute
    acceleration = round(abs(floatAccel));
    //Serial.println(acceleration);
    fram.writeEnable(true);
    fram.write8(addr, highByte(acceleration));
    fram.writeEnable(false);
    fram.writeEnable(true);
    fram.write8(addr + 1, lowByte(acceleration));
    fram.writeEnable(false);
    addr ++;



  }
  uint16_t currentAcceleration;
  uint16_t highValue;
  uint16_t lowValue;
  addr = 0;
  for (uint16_t i = 0; i < quarterWay ; i ++) {
    highValue = fram.read8(addr);
    lowValue = fram.read8(addr + 1);
    highValue <<= 8;
    currentAcceleration = highValue | lowValue;
    stepper.setAcceleration(currentAcceleration * 1000);
    stepper.setCurrentPosition(0);
    stepper.moveTo(-stepsPerSample);
    while (stepper.distanceToGo() < 0)
    {
      stepper.run();
    }
    addr ++;
  }

  for (uint16_t i = 0; i < quarterWay ; i ++) {
    highValue = fram.read8(addr - 1);
    lowValue = fram.read8(addr);
    highValue <<= 8;
    currentAcceleration = highValue | lowValue;
    stepper.setAcceleration(-currentAcceleration * 1000);
    stepper.setCurrentPosition(0);
    stepper.moveTo(-stepsPerSample);
    while (stepper.distanceToGo() < 0)
    {
      stepper.run();
    }
    addr --;
  }

  for (uint16_t i = 0; i < quarterWay ; i ++) {
    highValue = fram.read8(addr);
    lowValue = fram.read8(addr + 1);
    highValue <<= 8;
    currentAcceleration = highValue | lowValue;
    stepper.setAcceleration(currentAcceleration * 1000);
    stepper.setCurrentPosition(0);
    stepper.moveTo(stepsPerSample);
    while (stepper.distanceToGo() > 0)
    {
      stepper.run();
    }
    addr ++;
  }

  for (uint16_t i = 0; i < quarterWay ; i ++) {
    highValue = fram.read8(addr - 1);
    lowValue = fram.read8(addr);
    highValue <<= 8;
    currentAcceleration = highValue | lowValue;
    stepper.setAcceleration(-currentAcceleration * 1000);
    stepper.setCurrentPosition(0);
    stepper.moveTo(stepsPerSample);
    while (stepper.distanceToGo() > 0)
    {
      stepper.run();
    }
    addr --;
  }





}
