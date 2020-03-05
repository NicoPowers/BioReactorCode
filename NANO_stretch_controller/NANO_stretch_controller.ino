/*
  Authors: Nicolas Montoya
  Credits: Bryan James
  Status: Prototype
  This code is uploaded to the NANO, and controls the bidirectional stepper in a sinusoidal fashion
  The commands that can be given to the nano from the MEGA include:
  "nd%f" to set the initial displacement
  "nf%f" to set the frequency for the oscillation where %f is the value you want to set it to
  "no%d" to move outwards a distance %f
  "ni%f" to move inwards a distance %f
  "nq" to toggle the state of the MasterFlex Pump
  "ne%f" to set the flow rate for the MasterFlex Pump, where %f ranges from 0 to 80
  "nx" to cancel the oscillation and return stretcher to limit switch
  "nr1%f" to start oscillation where %f is the repeating distance (in mm) of oscillation with the sinusoidal pump flow
  "nr0%f" to start oscillation where %f is the repeating distance (in mm) of oscillation without the sinusoidal pump flow

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
*/

#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#define LIMIT_SWITCH 9
#define SS_RX 2
#define SS_TX 3
#define SYNC_START 15 // TODO: CHANE IN HARDWARE
#define SYNC_PROPAGATION_MS 0

// the following distances only work at 1 Hz, and must be entered using the respective array index
float stretchingDistances[10] = {1.0, 1.25, 1.50, 1.75, 2.00, 7.00, 7.25, 7.50, 7.75, 8.00};
float stretchingSpeedConstants[10] = {1.0, 1.00, 1.00, 1.00, 1.00, 500.00, 1.00, 1.00, 1.00, 1.00};
float stretchingAccelerationConstants[10] = {7.3, 7.46, 7.55, 7.52, 7.58, 100.00, 1.00, 1.00, 1.00, 1.00};

int stretchIndex = 0, withPump = 0;
float steps, frequency = 1;

float initialDistance = 2.0; // mm start distance in between plates

long unsigned int t_1, t_2, lastHit = 0;

volatile bool repeating = false, pumpOn = false;

String input;

char decision;
// Define a stepper and the pins it will use
// Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
AccelStepper stepper(1, 4, 7);

SoftwareSerial mySerial(SS_RX, SS_TX); // RX, TX

void setup()
{

  // Begins serial interface for serial monitor and between mega
  Serial.begin(9600);
  mySerial.begin(9600);

  pinMode(LIMIT_SWITCH, INPUT); // pin connected to limit switch
  stepper.setCurrentPosition(0);
  stepper.setMaxSpeed(10000);
  stepper.setAcceleration(1000);

  hitLimitSwitch();
  travelOutwards(initialDistance);

  pinMode(SYNC_START, OUTPUT);   // TODO: CHANGE IN HARDWARE
  digitalWrite(SYNC_START, LOW); // start sync start low so pump does not start with sinusoidal flow rate

  Serial.println("NANO: Stretching ready ...");
}

void loop()
{

  while (mySerial.available() == 0)
  {

    if (repeating)
    {
      sinusoidalStretch(stretchIndex);
    }
  }
  if (mySerial.available() > 0)
  {
    input = mySerial.readString();
    decision = input.charAt(0);

    // **** command to start stretching
    if (decision == 'r')
    {
      hitLimitSwitch();
      travelOutwards(initialDistance);
      repeating = true;
      stretchIndex = input.substring(1, 2).toInt();
      withPump = input.substring(2).toInt();
      mySerial.print("NANO: Starting to stretch distance of ");
      mySerial.print(stretchingDistances[stretchIndex]);
      mySerial.println(" mm");
    }
    // **** command to move outwards
    else if (decision == 'o') // travel outwards a certain distance
    {
      float outDistance = input.substring(1).toFloat();
      travelOutwards(outDistance);
      mySerial.print("NANO: Travelled outwards distance of ");
      mySerial.print(outDistance);
      mySerial.println(" mm");
    }
    // **** command to change the initial displacment between the plates
    else if (decision == 'd') // change the initial displacement between plates
    {
      initialDistance = input.substring(1).toFloat();
      mySerial.print("NANO: New initial distance = ");
      mySerial.print(initialDistance);
      mySerial.println(" mm");
    }
    // **** command to move inwards
    else if (decision == 'i')
    {
      float inDistance = input.substring(1).toFloat();
      travelInwards(inDistance);
      mySerial.print("NANO: Travelled inwards distance of ");
      mySerial.print(inDistance);
      mySerial.println(" mm");
    }
    // **** command to stop the stretching, must also stop the pump if it is on
    else if (decision == 'x')
    {
      hitLimitSwitch();
      travelOutwards(initialDistance);
      repeating = false;
      digitalWrite(SYNC_START, LOW); // Tell UNO to stop the pump
      mySerial.println("NANO: Stretching cancelled");
      mySerial.print("NANO: Current Displacement = ");
      mySerial.print(initialDistance);
      mySerial.println(" mm");
    }
    else
    {
      mySerial.println("NANO: Please enter a valid command");
    }
  }

  mySerial.flush();
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

void hitLimitSwitch()
{

  steps = getSteps(1000);
  stepper.setCurrentPosition(0);
  stepper.moveTo(steps);

  while (stepper.distanceToGo() > 0)
  {
    // if stepper motor hit limit switch and the last time it was hit is greater than 1 second
    if ((digitalRead(LIMIT_SWITCH) == LOW && (millis() - lastHit >= 1000)))
    {
      stepper.setCurrentPosition(0);
      lastHit = millis();
      return;
    }
    else
    {
      stepper.run();
    }
  }
  lastHit = 0;
}

void travelInwards(float mm)
{

  steps = getSteps(mm);
  stepper.setCurrentPosition(0);
  stepper.moveTo(steps);

  while (stepper.distanceToGo() > 0)
  {

    if ((digitalRead(LIMIT_SWITCH) == LOW && (millis() - lastHit >= 1000)))
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
  lastHit = 0;
}

void sinusoidalStretch(int stretchIndex)
{

  float period = 1; // can only do 1 Hz properly

  float steps = getSteps(stretchingDistances[stretchIndex]);
  float distanceToStretch = stretchingDistances[stretchIndex]
  float averageSpeed = 2 * (steps / period);
  float averageAcceleration = averageSpeed / period;

  long unsigned int t0 = 0, t1 = 0;

  stepper.setAcceleration(stretchingAccelerationConstants[stretchIndex] * averageAcceleration);
  stepper.setSpeed(stretchingSpeedConstants[stretchIndex] * averageSpeed);

  
  if (withPump == 1)
  {
    digitalWrite(SYNC_START, HIGH); // Tell UNO to start the pump in sync with stretching
    delay(SYNC_PROPAGATION_MS);     // delay in case stepper starts too soon before pump
  }
  /*
  t0 = millis();
  stepper.move(-steps); // move outwards (first half of period)
  while (stepper.distanceToGo() < -1)
  {
    stepper.run();
  }
  */
  t0 = millis();
  travelOutwards(distanceToStretch); // move outwards (first half of period)
  travelInwards(distanceToStretch); // move back inwards (final half of period)
  t1 = millis();
  /*
  stepper.move(steps); // move back inwards (final half of period)
  while (stepper.distanceToGo() > 1)
  {
    stepper.run();
  }
  t1 = millis();
  */
  if (withPump == 1)
  {
    digitalWrite(SYNC_START, LOW); // Tell UNO to start the pump in sync with stretching
    delay(SYNC_PROPAGATION_MS);    // delay in case stepper starts too soon before pump
  }

  mySerial.print("NANO: Finished stretching with a period of ");
  mySerial.print(t1 - t0);
  mySerial.println(" ms");
}
