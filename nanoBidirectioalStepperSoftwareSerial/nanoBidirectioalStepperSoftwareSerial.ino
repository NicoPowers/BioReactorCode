#include <AccelStepper.h>
#include <SoftwareSerial.h>

float frequency, period, acceleration, steps, mm = 0.0, stepsToGoBack, newMaxSpeed, distanceFromHome = 0.0, repeatingDistance = 0.0;

long unsigned int t_1, t_2, lastHit = 0;

volatile bool repeating = false, goingIn = false, goingOut = false;

String input;

char decision;
// Define a stepper and the pins it will use
AccelStepper stepper(1, 4, 7);   // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
SoftwareSerial mySerial(11, 10); // RX, TX

void setup()
{
  // Change these to suit your stepper if you want
  Serial.begin(9600);
  mySerial.begin(9600);
  delay(5000);
  Serial.println("ready...");
  //pinMode(8, OUTPUT);
  pinMode(9, INPUT); // pin connected to limit switch
  //digitalWrite(8, HIGH);
  frequency = 0.3;
  period = getPeriod(frequency);
  goToLimitSwitch();
}
// d50 will go down 50 mm, u70 will go up 70 mm, h will go home, s will set current position to 0
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
    //Serial.print("Got data: ");
    input = mySerial.readString();
    //Serial.println(input);
    decision = input.charAt(0);

    if (decision == 'r')
    {
      repeating = true;
      repeatingDistance = input.substring(1).toFloat();
      goingOut = true;
      goToLimitSwitch();
    }
    else if (decision == 'f')
    {
      frequency = input.substring(1).toFloat();
      period = getPeriod(frequency);
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

float mmToSteps(float mm)
{
  return (float)((377.7 * mm) + 275.7); // converts mm to steps, only works for basic lead screw from openbuilds with 1.8 degree per step stepper motor (mySerial # 180815)
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
      //stepsToGoBack = -(stepper.currentPosition() + stepper.distanceToGo() + mmToSteps(10.0));
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

  distanceFromHome = distanceFromHome + mm;
}

void goToLimitSwitch()
{
  travelInwards(100);
}
