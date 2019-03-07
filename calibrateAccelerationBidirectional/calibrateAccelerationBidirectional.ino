#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include <SD.h>

File myFile;

float frequency, period, acceleration, steps, mm = 0.0, stepsToGoBack, newMaxSpeed, distanceFromHome = 0.0, repeatingDistance = 0.0;

long unsigned int t_1, t_2, lastHit = 0;

float frequencies[6] = {0.25, 0.50, 1.0, 1.5, 2.0};
float distances[10] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};


volatile bool repeating = false, goingIn = false, goingOut = false;

String input;

char decision;
// Define a stepper and the pins it will use
AccelStepper stepper(1, 4, 7); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5

void setup()
{
  // Change these to suit your stepper if you want
  Serial.begin(9600);
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the SD library functions will not work.
  pinMode(53, OUTPUT);

  if (!SD.begin(53)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("calData.txt", FILE_WRITE);





}
// d50 will go down 50 mm, u70 will go up 70 mm, h will go home, s will set current position to 0
void loop()
{
  Serial.println("Starting...");
  for (int i = 0; i < 6; i ++ ) {
    frequency = frequencies[i];
    period = getPeriod(frequency);
    Serial.print("Starting frequency = ");
    Serial.println(frequency);
    for (int j = 0; j < 10; j ++ ) {
      repeatingDistance = distances[j];
      Serial.print("Starting distance = ");
      Serial.println(repeatingDistance);
      for (int k = 0; k < 11; k ++) {
        t_1 = millis();
        travelOutwards(repeatingDistance);
        t_2 = millis();
        myFile.print(frequency);
        myFile.print(',');
        myFile.print(repeatingDistance);
        myFile.print(',');
        myFile.println(t_2 - t_1);
        
        delay(50);
        
        t_1 = millis();
        travelInwards(repeatingDistance);
        t_2 = millis();
        myFile.print(frequency);
        myFile.print(',');
        myFile.print(repeatingDistance);
        myFile.print(',');
        myFile.println(t_2 - t_1);
        
        Serial.print("Finished trial ");
        Serial.print(k);
        Serial.print(" of distance = ");
        Serial.println(repeatingDistance);
      }
    }
  }
  myFile.close();
  Serial.println("Done!");

}



float mmToSteps ( float mm ) {
  return (float) ((377.7 * mm) + 275.7); // converts mm to steps, only works for basic lead screw from openbuilds with 1.8 degree per step stepper motor (mySerial # 180815)
}

float getAcceleration ( float steps, float period ) {
  return (float) (2.0 * steps) / (pow(period, 2)); // uses 2D kinetmatic formula to caluclate acceleration such that stepper stops at final position
}

float getPeriod ( float frequency ) {
  return (float) 1.0 / frequency; // takes inverse of frequency to return period of oscillation
}


void travelOutwards ( float mm ) {
  steps = -mmToSteps(mm);
  acceleration = getAcceleration(steps, period);
  stepper.setAcceleration(acceleration);
  stepper.setCurrentPosition(0);
  stepper.moveTo(steps);
  while (stepper.distanceToGo() < 0) {
    stepper.run();
  }
  distanceFromHome = distanceFromHome - mm;
}

void travelInwards ( float mm ) {

  steps = mmToSteps(mm);
  acceleration = getAcceleration(steps, period);
  stepper.setAcceleration(acceleration);
  stepper.setCurrentPosition(0);
  stepper.moveTo(steps);

  while (stepper.distanceToGo() > 0) {

    stepper.run();

  }

  distanceFromHome = distanceFromHome + mm;

}

void goToLimitSwitch () {
  travelInwards(100);
}
