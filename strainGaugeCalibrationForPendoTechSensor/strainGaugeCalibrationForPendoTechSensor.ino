/* calibration for Pendo Tech sensor using Hx711 module

   hardware design: syyyd
   available at http://syyyd.taobao.com

   library design: Weihong Guan (@aguegu)
   http://aguegu.net

   library host on
   https://github.com/aguegu/Arduino
*/

// Hx711.DOUT - pin #A1
// Hx711.SCK - pin #A0

#include "hx711.h"
int count = 1;

long unsigned int averageValues[100];
float averageValue = 0;
float ratio = 102.0 / 8387415.50; // at atmospheric pressure (102 kPa) the value was 8387415.50

Hx711 sensor(A1, A0);

void setup()
{

  Serial.begin(9600);
  // The code below is to calibrate the sensor and find the average value at atmospheric conditions; which happened to be 8387415.50
  /*
  for (int i = 0; i < 100; i ++) {
    averageValues[i] = 0;
  }
  for (int i = 0; i < 100; i ++) {
    averageValues[i] = sensor.averageValue();
    Serial.print("Got ");
    Serial.print(i + 1);
    Serial.println(" average value");
    Serial.println(averageValues[i]);
    delay(200);
  }
  for (int i = 0; i < 100; i ++ ) {
    averageValue += averageValues[i];
  }
  Serial.print("Final average value = ");
  Serial.println(averageValue / 100); // 8387415.50
  */
  //sensor.setScale(102.0 / 8387415.50);
  //sensor.setOffset(0);
}

void loop()
{

  Serial.println((ratio * (sensor.averageValue())));
}
