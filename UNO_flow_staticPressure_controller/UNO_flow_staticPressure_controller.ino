/* This code controls the vertical displacement of the water vial to vary the hydrostatic pressure

*/
#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include <Adafruit_MCP4725.h>
#define DAC_NOISE_OFFSET 0
#define LIMIT_SWITCH 2
#define PUMP_POWER A1
#define SS_RX 10
#define SS_TX 11
#define SYNC_START 15 // TODO: CHANE IN HARDWARE

float frequency, period, acceleration, steps, mm, stepsToGoBack, newMaxSpeed, distanceFromHome = 0.0;
float maxFlowRate = 20.0, phase = 0.0, shift = 0.0;
long int t_1, t_2, lastHit = 0;

volatile bool pumpOn = false, sinusodialFlow = false;

String input;

char decision;
// Define a stepper and the pins it will use
AccelStepper stepper(1, 4, 7); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5SoftwareSerial mySerial(11, 10); // RX, TX
SoftwareSerial mySerial(SS_RX, SS_TX);

Adafruit_MCP4725 dac;

void setup()
{
  // Sets pin used to turn on/off MasterFlex Pump and turns it off

  pinMode(PUMP_POWER, OUTPUT); // TODO: CHANE IN HARDWARE
  digitalWrite(PUMP_POWER, pumpOn);
  digitalWrite(PUMP_POWER, !pumpOn);

  // Starts DAC to control MasterFlex Pump flow rate
  dac.begin(0x62); // TODO: CHANE IN HARDWARE
  setFlowRate(0);

  pinMode(LIMIT_SWITCH, INPUT); // pin connected to limit switch
  pinMode(SYNC_START, INPUT);   // pin connect to NANO (to know when to start the pump)

  mySerial.begin(9600);

  frequency = 0.5;
  mm = -50.0;
  steps = mmToSteps(mm);
  period = getPeriod(frequency);
  acceleration = getAcceleration(steps, period);
  stepper.setAcceleration(acceleration);
  stepper.setCurrentPosition(0);

  mySerial.println("*UNO FLOW/PRESSURE CONTROL READY*");
}

void loop()
{

  while (mySerial.available() == 0)
  {
    if (digitalRead(SYNC_START) == HIGH)
    {
      sinusodialFlow = true;
    }
  }
  if (mySerial.available() > 0)
  {
    input = mySerial.readString();
    decision = input.charAt(0);
    // **** command to move the water vial upwards
    if (decision == 'u')
    {
      mm = input.substring(1).toFloat();
      travelUp(mm);
    }
    // **** command to move the water vial downwards
    else if (decision == 'd')
    {
      mm = input.substring(1).toFloat();
      travelDown(mm);
    }
    // **** command to return to the home position for the water vial
    else if (decision == 'h')
    {
      goHome();
    }
    // **** command to toggle the pump
    else if (decision == 'q')
    {
      pumpOn = !pumpOn;
      digitalWrite(PUMP_POWER, pumpOn);
      digitalWrite(PUMP_POWER, !pumpOn);
    }
    // **** command to change the phase of the sinusoidal flow rate
    else if (decision == 'p')
    {
      phase = input.substring(1).toFloat();
    }
    // **** command to change the vertical shift of the sinusoidal flow rate
    else if (decision == 's')
    {
      shift = input.substring(1).toFloat();
    }
    // **** command to change the amplitude (max flow rate) of the sinusoidal flow rate
    else if (decision == 'a')
    {
      maxFlowRate = input.substring(1).toFloat();
    }
    // **** command to manually set the flow rate
    else if (decision == 'e')
    {
      setFlowRate(input.substring(1).toFloat());
    }
    // **** command to set current position of water vial to home position
    else if (decision == 'x')
    {
      distanceFromHome = 0.0;
      mySerial.println("UNO: Set this position as 0.");
    }
    // **** command to return the current distance from home position
    else if (decision == 'c')
    {
      mySerial.print("UNO: Distance from home = ");
      mySerial.print(distanceFromHome);
      mySerial.println(" mm");
    }
    else
    {
      mySerial.println("UNO: Please enter a valid decision");
    }
  }
  mySerial.flush();
}

// sets the desired flow rate to the pump
void setFlowRate(float flowRate)
{
  float voltage = ((float)(flowRate / 80.0) * 4095) + DAC_NOISE_OFFSET;
  dac.setVoltage(voltage, false);
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
  while (digitalRead(LIMIT_SWITCH) == HIGH)
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
    if ((digitalRead(LIMIT_SWITCH) == LOW && (millis() - lastHit >= 1000)))
    { // if stepper motor hit limit switch
      stepsToGoBack = -(stepper.currentPosition() + stepper.distanceToGo() + mmToSteps(10.0));
      stepper.setCurrentPosition(0);
      lastHit = millis();
      mySerial.println("UNO: Hit limit switch! Setting this position as 0.");
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

void sinusoidFlowRate()
{
  int numSteps = 100;
  float timePerStep = 1.0 / numSteps; // time per step in seconds
  float timeDelay = timePerStep / 1000.0;
  for (int i = 0; i++; i < numSteps)
  {
    float currentFlowRate = shift + (maxFlowRate * pow(sin(PI * (i * timePerStep) + phase), 2));
    setFlowRate(currentFlowRate);
    delay(timeDelay);
  }
}