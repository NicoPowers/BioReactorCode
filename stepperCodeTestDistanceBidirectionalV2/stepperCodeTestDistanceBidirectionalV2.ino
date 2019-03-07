// Bounce.pde
// -*- mode: C++ -*-
//
// Make a single stepper bounce from one limit to another
//
// Copyright (C) 2012 Mike McCauley
// $Id: Random.pde,v 1.1 2011/01/05 01:51:01 mikem Exp mikem $

#include <AccelStepper.h>

//int steps = 800;
float mm = 10.0;
//float steps = mm * mmToStepsConversion;
float steps = (377.7 * mm) + 275.7;
float frequency = 0.5;
float period = (float) 1.0 / frequency;
float acceleration = (float) (2.0 * steps) / (pow(period / 2, 2));

float stepsToGoBack;
// Define a stepper and the pins it will use
AccelStepper stepper(1, 4, 7); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5

void setup()
{
  // Change these to suit your stepper if you want
  //stepper.setMaxSpeed(20000);
  Serial.begin(9600);
  stepper.setAcceleration(acceleration);
  stepper.setCurrentPosition(0);
  stepper.moveTo(steps);
  Serial.println(acceleration);
  delay(2000);
}

void loop()
{
  // If at the end of travel go to the other end
  if (stepper.distanceToGo() == 0)
  {
    while(1) {
      
    }
    stepsToGoBack = -stepper.currentPosition();
    stepper.setCurrentPosition(0);
    stepper.moveTo(stepsToGoBack);
    //frequency++;
    //period = (float) 1.0 / frequency;
    ////acceleration = (float) (2.0 * steps) / (pow(period / 2, 2));
    //stepper.setAcceleration(acceleration);
  }

  stepper.run();
}
