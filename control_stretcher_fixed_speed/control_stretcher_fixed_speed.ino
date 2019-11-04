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
  stepper.setCurrentPosition(0);
  stepper.setMaxSpeed(10000);
  stepper.setAcceleration(1000);
  delay(2000);

  travelInwards(100.0);
  Serial.println("Ready.");
  while (1) {
    stretch(10, 1.5);
    //    delay(5000);
  }

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

void stretch(float distance, float frequency)
{
  long unsigned int t_1, t_2, t_3;
  float omega = PI * frequency ; // rad/s
  float period = (1.0 / frequency); // s
  float samples = 10;

  float steps = getSteps(distance);
  float stepperSpeed;
  float flowRate;
  //  Serial.println(frequency, 4);
  //  Serial.println(omega, 4);
  //  Serial.println(period, 4);
  //  Serial.println(timeStep, 4);
  //  Serial.println(steps, 4);
  float timePerSample = (float) period / samples; // seconds per sample
  int stepsPerSample = steps / (samples / 2);
  //Serial.println(numSamples);
  int midWay = samples / 2;
  int quarterWay = midWay / 2;
  //Serial.println(numSamples);
  uint16_t velocity;
  float floatSpeed, t;
  int16_t accelerations[midWay] = {};
  uint16_t speeds[midWay] = {};
  float floatSpeeds[midWay] = {};
  float averageSpeed = 2 * (steps / period);
  float averageAcceleration = averageSpeed / period;
  int t0 = millis();
  stepper.setAcceleration(round(averageAcceleration));
  stepper.setSpeed(round(averageSpeed));
  //    stepper.setAcceleration(accelerations[i]);
  stepper.move(-round(steps));
  //    Serial.println(floatSpeeds[i]);
  //    Serial.println(accelerations[i]);
  //    Serial.println(stepper.distanceToGo());
  //  Serial.println("Going to start");
  while (stepper.distanceToGo() < -1) {
    stepper.run();
  }
  //  Serial.println("At Amplitude");
  stepper.move(round(steps));
  //    Serial.println(floatSpeeds[i]);
  //    Serial.println(accelerations[i]);
  //    Serial.println(stepper.distanceToGo());
  //  stepper.setSpeed(round(averageSpeed));
  //  stepper.setAcceleration(round(averageAcceleration));
  while (stepper.distanceToGo() > 1) {
    stepper.run();
  }
  Serial.println(millis() - t0);
  /*
    addr = 0;
    for (uint16_t i = 0; i < midWay ; i ++) { // only need to store half the values because the other way is just the same but negative
    t = ((float)timePerSample * i) ;
    //Serial.print(t);
    floatSpeed = steps * sin(omega * t) * cos(omega * t);
    velocity = round(abs(floatSpeed));
    //    //Serial.println(acceleration);
    //    fram.writeEnable(true);
    //    fram.write8(addr, highByte(velocity));
    //    fram.writeEnable(false);
    //    fram.writeEnable(true);
    //    fram.write8(addr + 1, lowByte(velocity));
    //    fram.writeEnable(false);
    //    addr ++;
    //    speeds[i] = velocity;
    floatSpeeds[i] = floatSpeed;
    accelerations[i] = abs(round(steps * (pow(cos(omega * t), 2) - pow(sin(omega * t), 2))));



    }
    uint16_t currentSpeed;
    uint16_t highValue;
    uint16_t lowValue;

    addr = 0;
    for (uint16_t i = 1; i < midWay ; i ++) {
    //    highValue = fram.read8(addr);
    //    lowValue = fram.read8(addr + 1);
    //    highValue <<= 8;
    //    currentSpeed = highValue | lowValue;
    //    Serial.println(speeds[i]);
    //    Serial.println(accelerations[i]);
    //stepper.setCurrentPosition(0); // outwards
    //    Serial.println(floatSpeeds[i]);
    stepper.setSpeed(floatSpeeds[i]);
    //    stepper.setAcceleration(accelerations[i]);
    stepper.move(-stepsPerSample);
    //    Serial.println(floatSpeeds[i]);
    //    Serial.println(accelerations[i]);
    //    Serial.println(stepper.distanceToGo());
    Serial.println(i);
    while (stepper.distanceToGo() < 0) {
      stepper.run();
    }
    }


    for (uint16_t i = midWay - 1; i > 0; i --) {
    //    highValue = fram.read8(addr - 1);
    //    lowValue = fram.read8(addr);
    //    highValue <<= 8;
    //    currentSpeed = highValue | lowValue;
    //stepper.move(stepsPerSample);
    //    Serial.println(floatSpeeds[i]);
    stepper.setSpeed(floatSpeeds[i]);
    //    stepper.setAcceleration(accelerations[i]);
    stepper.move(stepsPerSample);
    Serial.println(i);
    while (stepper.distanceToGo() > 0) {
      stepper.run();
    }

    }
  */


}
