
#include <Wire.h>
#include <Adafruit_MCP4725.h>

Adafruit_MCP4725 dac;

/* Note: If flash space is tight a quarter sine wave is enough
   to generate full sine and cos waves, but some additional
   calculation will be required at each step after the first
   quarter wave.                                              */

void setup(void)
{
  Serial.begin(9600);
  Serial.println("Hello!");
  pinMode(40, OUTPUT);
  // For Adafruit MCP4725A1 the address is 0x62 (default) or 0x63 (ADDR pin tied to VCC)
  // For MCP4725A0 the address is 0x60 or 0x61
  // For MCP4725A2 the address is 0x64 or 0x65
  dac.begin(0x62);
  dac.setVoltage(1096, false);
  //delay(1000);
}

void loop(void)
{

  //Serial.println(analogRead(A0));
  //delay(20);
  digitalWrite(40, HIGH);
  delay(5000);
  digitalWrite(40, LOW);
  delay(5000);
}
