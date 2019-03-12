/* This code controls the bidirectional stepper and the MasterFlex Pump at a constant flow rate 

The commands that can be given to the nano from the mega (control_megaOverseer.ino) include:

"ns" to set the current position as the Home position
"nh" to return to the home position
"nf%f" to set the frequency for the oscillation where %f is the value you want to set it to
"no%d" to move outwards a distance %f
"ni%f" to move inwards a distance %f
"nq" to toggle the state of the MasterFlex Pump
"ne%f" to set the flow rate for the MasterFlex Pump, where %f ranges from 0 to 80
"nx" to cancel the oscillation and return stretcher to limit switch
"nr%f" to start oscillation where %f is the repeating distance (in mm) of oscillation

*/

#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_MCP4725.h>

float frequency, period, acceleration, steps, mm = 0.0, stepsToGoBack, newMaxSpeed, distanceFromHome = 0.0, repeatingDistance = 0.0, amplitude = 0.0;

long unsigned int t_1, t_2, lastHit = 0;

volatile bool repeating = false, goingIn = false, goingOut = false, pumpOn = false;

String input;

char decision;
// Define a stepper and the pins it will use
AccelStepper stepper(1, 4, 7);   // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
SoftwareSerial mySerial(11, 10); // RX, TX

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
  delay(5000);

  // Starts DAC to control MasterFlex Pump flow rate
  dac.begin(0x62);
  dac.setVoltage(setFlowRate(20.0), false);

  mySerial.println("NANO: ready...");
  pinMode(9, INPUT); // pin connected to limit switch
  frequency = 0.5;
  period = getPeriod(frequency);
  goToLimitSwitch();
}

void loop()
{

  while (mySerial.available() == 0)
  {
    if (repeating)
    {

      if (goingOut)
      {
        t_1 = millis();
        travelOutwards(repeatingDistance);
        t_2 = millis();
        // Check how long oscillation took
        Serial.print("Took: ");
        Serial.print(t_2 - t_1);
        Serial.println(" ms long");
        Serial.print("With acceleration = ");
        Serial.print(acceleration);
        Serial.print(" and frequency = ");
        Serial.println(frequency);
        Serial.print("steps = ");
        Serial.println(mmToSteps(repeatingDistance));
        goingOut = false;
        goingIn = true;
      }
      else if (goingIn)
      {
        t_1 = millis();
        travelInwards(repeatingDistance);
        t_2 = millis();
        // Check how long oscillation took
        Serial.print("Took: ");
        Serial.print(t_2 - t_1);
        Serial.println(" ms long");
        Serial.print("With acceleration = ");
        Serial.print(acceleration);
        Serial.print(" and frequency = ");
        Serial.println(frequency);
        Serial.print("steps = ");
        Serial.println(mmToSteps(repeatingDistance));
        goingIn = false;
        goingOut = true;
      }
    }
  }
  if (mySerial.available() > 0)
  {
    input = mySerial.readString();
    decision = input.charAt(0);

    if (decision == 'r')
    {
      repeating = true;
      repeatingDistance = input.substring(1).toFloat();
      goingOut = true;
    }
    else if (decision == 's')
    {
      distanceFromHome = 0.0;
      mySerial.println("NANO: Set this position as home.");
    }
    else if (decision == 'h')
    {
      goHome();
    }
    else if (decision == 'f')
    {
      frequency = input.substring(1).toFloat();
      period = getPeriod(frequency);
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
      dac.setVoltage(setFlowRate(input.substring(1).toFloat()), false);
    }
    else if (decision == 'x')
    {
      repeating = false;
      if (goingIn)
      {
        goToLimitSwitch();
      }
      goingOut = false;
      goingIn = false;
    }
  }

  mySerial.flush();
}

float setFlowRate(float flowRate)
{
  return (float)(flowRate / 80.0) * 4095;
}

float mmToSteps(float mm)
{
  return (float)(390.55 * mm); // converts mm to steps, only works for bidirectional lead screw
}

float getAcceleration(float steps, float period)
{
  return (float)(2.0 * steps) / (pow(period, 2)); // uses 2D kinetmatic formula to caluclate acceleration such that stepper stops at final position
}

float getPeriod(float frequency)
{
  return (float)1.0 / frequency; // takes inverse of frequency to return period of oscillation
}

void travelOutwards(float mm)
{
  steps = -mmToSteps(mm);
  acceleration = getAcceleration(steps, period);
  stepper.setAcceleration(acceleration);
  stepper.setCurrentPosition(0);
  stepper.moveTo(steps);
  while (stepper.distanceToGo() < 0)
  {
    stepper.run();
  }
  // update distanceFromHome
  distanceFromHome = distanceFromHome - mm;
}

void travelInwards(float mm)
{

  steps = mmToSteps(mm);
  acceleration = getAcceleration(steps, period);
  stepper.setAcceleration(acceleration);
  stepper.setCurrentPosition(0);
  stepper.moveTo(steps);

  while (stepper.distanceToGo() > 0)
  {

    if ((digitalRead(9) == LOW && (millis() - lastHit >= 3000)))
    { // if stepper motor hit limit switch
      stepper.setCurrentPosition(0);
      lastHit = millis();
      distanceFromHome = 0.0;
      return;
    }
    else
    {
      stepper.run();
    }
  }
  // update distanceFromHome
  distanceFromHome = distanceFromHome + mm;
}

void goHome()
{
  if (distanceFromHome > 0.0)
  {
    travelOutwards(distanceFromHome);
  }
  else if (distanceFromHome < 0.0)
  {
    travelInwards(distanceFromHome);
  }
}
void goToLimitSwitch()
{
  travelInwards(100);
}
