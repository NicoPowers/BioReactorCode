/* calibration of the pendo tech pressure sensor
    using the basic lead screw from openbuilds
*/

#include <AccelStepper.h>
#include <SoftwareSerial.h>



float frequency, period, acceleration, steps, mm, stepsToGoBack, newMaxSpeed, distanceFromHome = 0.0;

long int t_1, t_2, lastHit = 0;

String input;

char decision;
// Define a stepper and the pins it will use
AccelStepper stepper(1, 4, 7); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5SoftwareSerial mySerial(11, 10); // RX, TX
SoftwareSerial mySerial(10, 11);


void setup()
{
  // Change these to suit your stepper if you want

  mySerial.begin(9600);
  pinMode(2, INPUT); // pin connected to limit switch
  frequency = 0.5;
  mm = -50.0;
  steps = mmToSteps(mm);
  period = getPeriod(frequency);
  acceleration = getAcceleration(steps, period);
  stepper.setAcceleration(acceleration);
  stepper.setCurrentPosition(0);
  delay(3000);
}
// d50 will go down 50 mm, u70 will go up 70 mm, h will go home, s will set current position to 0
void loop()
{
  mySerial.println("Ready...");
  while (mySerial.available() == 0)
  {
    
  }
  if (mySerial.available() > 0)
  {
    input = mySerial.readString();
    decision = input.charAt(0);
    if (decision == 'u')
    {
      mm = input.substring(1).toFloat();
      travelUp(mm);
      mySerial.println('r');
    }
    else if (decision == 'd')
    {
      mm = input.substring(1).toFloat();
      travelDown(mm);
      mySerial.println('r');
    }
    else if (decision == 'h')
    {
      goHome();
      mySerial.println('r');
      
    }
    else if (decision == 's')
    {
      distanceFromHome = 0.0;
      mySerial.println("Set this position as 0.");
    }
    else if (decision == 'c')
    {
      mySerial.print("Distance from home = ");
      mySerial.print(distanceFromHome);
      mySerial.println(" mm");
    }
    else
    {
      mySerial.println("Please enter a valid decision");
    }
  }
  mySerial.flush();
}

float mmToSteps(float mm)
{
  return (float)(199.99 * mm); // converts mm to steps, only works for basic lead screw from openbuilds with 1.8 degree per step stepper motor (mySerial # 180815)
}

float getAcceleration(float steps, float period)
{
  return (float)(2.0 * steps) / (pow(period, 2)); // uses 2D kinetmatic formula to caluclate acceleration such that stepper stops at final position
}

float getPeriod(float frequency)
{
  return (float)1.0 / frequency; // takes inverse of frequency to return period of oscillation
}

void goHome()
{

  stepper.setCurrentPosition(0);
  stepper.moveTo(mmToSteps(-distanceFromHome)); // if stepper motor hit limit switch
  while (digitalRead(2) == HIGH)
  { // if hit stepper motor...
    if (stepper.distanceToGo() == 0)
    {
      break;
    }
    stepper.run();
  }

  distanceFromHome = 0.0;
}

void travelDown(float mm)
{
  steps = -mmToSteps(mm);
  stepper.setCurrentPosition(0);
  stepper.moveTo(steps);
  while (stepper.distanceToGo() < 0)
  {
    stepper.run();
  }
  distanceFromHome = distanceFromHome - mm;
}

void travelUp(float mm)
{

  steps = mmToSteps(mm);
  stepper.setCurrentPosition(0);
  stepper.moveTo(steps);

  while (stepper.distanceToGo() > 0)
  {
    if ((digitalRead(2) == LOW && (millis() - lastHit >= 3000)))
    { // if stepper motor hit limit switch
      stepsToGoBack = -(stepper.currentPosition() + stepper.distanceToGo() + mmToSteps(10.0));
      stepper.setCurrentPosition(0);
      lastHit = millis();
      Serial.println("Hit limit switch! Setting this position as 0.");
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
