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
  "na%f" to set the amplitude of the sinusoidal flow rate


*/

#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_MCP4725.h>

float frequency, steps, mm = 0.0, repeatingDistance = 0.0, maxFlowRate = 20.0, phase = 0.0, shift = 0.0;

long unsigned int t_1, t_2, lastHit = 0;

volatile bool repeating = false, pumpOn = false;

String input;

char decision;
// Define a stepper and the pins it will use
// Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
AccelStepper stepper(1, 4, 7);

SoftwareSerial mySerial(10, 11); // RX, TX

Adafruit_MCP4725 dac;

void setup()
{
  // Sets pin used to turn on/off MasterFlex Pump and turns it off
  pinMode(A1, OUTPUT);
  digitalWrite(A1, pumpOn);
  digitalWrite(A1, !pumpOn);

  // Begins serial interface for serial monitor and between mega
  Serial.begin(9600);
  mySerial.begin(9600);

  // Starts DAC to control MasterFlex Pump flow rate
  dac.begin(0x62);
  setFlowRate(0);
  pinMode(9, INPUT); // pin connected to limit switch
  stepper.setMaxSpeed(10000);
  stepper.setAcceleration(1000);
  delay(2000);

  travelInwards(100.0);
  mySerial.println("Ready.");
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
      travelOutwards(1);
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
  return (float)(390.55 * mm);
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
  long unsigned int t_1, t_2;
  frequency = frequency / 1000.0;
  float continuosFrequency = 2.0 * PI * frequency;
  float period = 1.0 / frequency;
  float timePerSegment = period / 1000.0;
  float amplitude = getSteps(distance);
  float stepperSpeed;
  float flowRate;

  t_1 = millis();
  while (millis() - t_1 < period)
  {

    t_2 = millis();

    stepperSpeed = amplitude * PI * frequency * sin(continuosFrequency * (t_2 - t_1));
    flowRate = (maxFlowRate * pow(sin((continuosFrequency / 2.0 * (t_2 - t_1)) + phase), 2)) + shift;

    stepper.setSpeed(-stepperSpeed * 1000.0);
    setFlowRate(flowRate);

    while (millis() - t_2 < timePerSegment)
    {
      stepper.runSpeed();
    }
  }
}
