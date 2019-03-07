// Bounce.pde
// -*- mode: C++ -*-
//
// Make a single stepper bounce from one limit to another
//
// Copyright (C) 2012 Mike McCauley
// $Id: Random.pde,v 1.1 2011/01/05 01:51:01 mikem Exp mikem $

#include <AccelStepper.h>

//int steps = 800;
// float mm = 100.0;
//float steps = mm * mmToStepsConversion;
// float steps = (199.99 * mm) - 2374.5;
// float period = (float) 1.0 / frequency;
// float acceleration = (float) (2.0 * steps) / (pow(period / 2, 2));

float frequency, period, acceleration, steps, mm, stepsToGoBack;

bool hitSwitch = false;

unsigned long lastHit = 0;
// Define a stepper and the pins it will use
AccelStepper stepper(1, 4, 7); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5

void setup()
{
  // Change these to suit your stepper if you want

  Serial.begin(9600);
  pinMode(2, INPUT);
  frequency = 0.125;
  mm = -50.0;
  steps = mmToSteps(mm);
  period = getPeriod(frequency);
  acceleration = getAcceleration(steps, period);

  stepper.setAcceleration(acceleration);
  stepper.setCurrentPosition(0);
  stepper.moveTo(steps); // positive is up, negative is down
  Serial.print("Distance to go = ");
  Serial.println(stepper.distanceToGo());
  delay(5000);
}

void loop()
{
  // If at the end of travel go to the other end
  if (stepper.distanceToGo() > 0 && (digitalRead(2) == LOW && (millis() - lastHit >= 3000))) {
    stepsToGoBack = -(stepper.currentPosition() + stepper.distanceToGo() + mmToSteps(10.0));
    stepper.setCurrentPosition(0);
    //stepper.stop();
    //stepper.moveTo(-(stepper.currentPosition() + stepper.distanceToGo()));
    //stepper.setCurrentPosition(0);
    lastHit = millis();
    hitSwitch = true;
  }
  else
  {
    if (stepper.distanceToGo() == 0)
    {
      if (hitSwitch) {
        stepper.moveTo(stepsToGoBack);
        hitSwitch = false;
      }
      else
      {
        goHome();
        while (1) {

        }
        stepsToGoBack = -stepper.currentPosition();
        stepper.setCurrentPosition(0);
        stepper.moveTo(stepsToGoBack); // positive is up, negative is down
      }

    }

    stepper.run();
  }

}


float mmToSteps ( float mm ) {
  return (float) (199.99 * mm);
}

float getAcceleration ( float steps, float period ) {
  return (float) (2.0 * steps) / (pow(period / 2, 2));
}

float getPeriod ( float frequency ) {
  return (float) 1.0 / frequency;
}

void goHome () {

  stepper.moveTo(mmToSteps(10000.0));
  //stepper.setMaxSpeed(20);
  while (digitalRead(2) == HIGH) {
    stepper.run();
  }
  stepper.setCurrentPosition(0);

}
