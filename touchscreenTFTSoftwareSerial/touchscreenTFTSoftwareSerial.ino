#include <stdint.h>
#include "TouchScreen.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <hardwareSerial.h>

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A9 // Chip Select goes to Analog 3
#define LCD_CD A8 // Command/Data goes to Analog 2
#define LCD_WR A7 // LCD Write goes to Analog 1
#define LCD_RD A6 // LCD Read goes to Analog 0
#define LCD_RESET 33

#define YP A11 // must be an analog pin, use "An" notation!
#define XM A10 // must be an analog pin, use "An" notation!
#define YM 31  // can be a digital pin
#define XP 30  // can be a digital pin

// Assign human-readable names to some common 16-bit color values:
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 263);

int cx = tft.width() / 2 - 1;
int cy = tft.height() / 2 - 1;
bool white = true;

int pressX, pressY;

int prevPressX = 0, prevPressY = 0;

String tempFrequency = "", tempDistance = "";
float frequency = 0.0, distance = 0.0;

long lastPressed;

bool holding = false, homeSection = true, frequencySection = false, distanceSection = false, nextSection = false, alreadyHasDecimal = false;

void homeMenu(float frequencyValue, float distanceValue)
{
  tft.fillScreen(BLACK);
  // Draw Frequency rectangle
  drawRoundRectangle(cx + 75, cy - 60, 90, 320, BLUE);  // Frequency rectangle blue border
  drawRoundRectangle(cx + 75, cy - 60, 80, 310, WHITE); // Frequency rectangle text holder box
  // Draw Step/distance rectange
  drawRoundRectangle(cx - 75, cy - 60, 90, 320, BLUE);  // Step rectangle blue border
  drawRoundRectangle(cx - 75, cy - 60, 80, 310, WHITE); // Step rectangle text holder box
  // Draw next screen triangle
  // Draw triangle border
  tft.fillTriangle(
      cx, cy + 210,      // peak
      cx + 50, cy + 135, // bottom left
      cx - 50, cy + 135, // bottom right
      BLUE);
  tft.fillTriangle(
      cx, cy + 200,      // peak
      cx + 50, cy + 125, // bottom left
      cx - 50, cy + 125, // bottom right
      WHITE);

  writeText(cx - 110, cy - 160, "Frequency:", 3, RED); // cx goes left to right, cy goes from top to bottom
  writeText(cx - 110, cy - 10, "Distance:", 3, RED);   // cx goes left to right, cy goes from top to bottom
  setFrequency(frequencyValue);
  setDistance(distanceValue);
}

void writeText(int x, int y, String text, int fontSize, uint16_t color)
{
  tft.setRotation(1);
  tft.setCursor(x, y); // cx goes left to right, cy goes from top to bottom
  tft.setTextColor(color);
  tft.setTextSize(fontSize);
  tft.println(text);
  tft.setRotation(0);
}

void writeValue(int x, int y, float value, int fontSize, uint16_t color)
{
  tft.setRotation(1);
  tft.setCursor(x, y); // cx goes left to right, cy goes from top to bottom
  tft.setTextColor(color);
  tft.setTextSize(fontSize);
  tft.println(value);
  tft.setRotation(0);
}
void setFrequency(float value)
{
  drawRoundRectangle(cx + 70, cy + 35, 50, 100, WHITE);
  tft.setRotation(1);
  tft.setCursor(cx + 80, cy - 160); // cx goes left to right, cy goes from top to bottom
  tft.setTextColor(BLUE);
  tft.setTextSize(3);
  tft.println(value);
  tft.setRotation(0);
}

void setDistance(float value)
{
  drawRoundRectangle(cx - 75, cy + 25, 50, 100, WHITE);
  tft.setRotation(1);
  tft.setCursor(cx + 55, cy - 10); // cx goes left to right, cy goes from top to bottom
  tft.setTextColor(BLUE);
  tft.setTextSize(3);
  tft.println(value);
  tft.setRotation(0);
}

void nextSectionMenu()
{
  tft.fillScreen(BLACK);
  // Draw Frequency rectangle
  drawRoundRectangle(cx + 75, cy + 50, 90, 250, BLUE);  // Frequency rectangle blue border
  drawRoundRectangle(cx + 75, cy + 50, 80, 240, WHITE); // Frequency rectangle text holder box
  // Draw Step/distance rectange
  drawRoundRectangle(cx - 75, cy + 50, 90, 250, BLUE);  // Step rectangle blue border
  drawRoundRectangle(cx - 75, cy + 50, 80, 240, WHITE); // Step rectangle text holder box
  // Draw next screen triangle
  // Draw triangle border
  tft.fillTriangle(
      cx, cy - 210,      // peak
      cx + 50, cy - 135, // bottom left
      cx - 50, cy - 135, // bottom right
      BLUE);
  tft.fillTriangle(
      cx, cy - 200,      // peak
      cx + 50, cy - 125, // bottom left
      cx - 50, cy - 125, // bottom right
      WHITE);
}

void drawRectangle(int x, int y, int height, int width, uint16_t color)
{
  tft.setRotation(0);
  tft.fillRect(x - (height / 2), y - (width / 2), height, width, color);
}

void drawRoundRectangle(int x, int y, int height, int width, uint16_t color)
{
  tft.setRotation(0);
  tft.fillRoundRect(x - (height / 2), y - (width / 2), height, width, max(height, width) / 8, color);
}
void numPadFrequency()
{
  tft.fillScreen(BLACK);
  // Number display bar
  drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
  drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
  writeText((cy / 2) - 50, (cx / 4) - 5, tempFrequency, 4, BLACK);
  // 0
  drawRectangle(cx * 7 / 4, cy * 5 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 7 / 4, cy * 5 / 4, 75, cy / 2, WHITE);
  writeText((cy * 5 / 4) - 15, (cx / 4) - 10, "0", 5, BLACK);
  // .
  drawRectangle(cx * 7 / 4, cy * 7 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 7 / 4, cy * 7 / 4, 75, cy / 2, WHITE);
  writeText((cy * 7 / 4) - 15, (cx / 4) - 10, ".", 5, BLACK);
  // 1
  drawRectangle(cx * 5 / 4, cy / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 5 / 4, cy / 4, 75, cy / 2, WHITE);
  writeText((cy / 4) - 15, (cx * 3 / 4) - 10, "1", 5, BLACK);
  // 2
  drawRectangle(cx * 5 / 4, cy * 3 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 5 / 4, cy * 3 / 4, 75, cy / 2, WHITE);
  writeText((cy * 3 / 4) - 15, (cx * 3 / 4) - 10, "2", 5, BLACK);
  // 3
  drawRectangle(cx * 5 / 4, cy * 5 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 5 / 4, cy * 5 / 4, 75, cy / 2, WHITE);
  writeText((cy * 5 / 4) - 15, (cx * 3 / 4) - 10, "3", 5, BLACK);
  // <--
  drawRectangle(cx * 5 / 4, cy * 7 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 5 / 4, cy * 7 / 4, 75, cy / 2, WHITE);
  writeText((cy * 7 / 4) - 35, (cx * 3 / 4) - 10, "<--", 4, RED);
  // 4
  drawRectangle(cx * 3 / 4, cy / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 3 / 4, cy / 4, 75, cy / 2, WHITE);
  writeText((cy / 4) - 15, (cx * 5 / 4) - 10, "4", 5, BLACK);
  // 5
  drawRectangle(cx * 3 / 4, cy * 3 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 3 / 4, cy * 3 / 4, 75, cy / 2, WHITE);
  writeText((cy * 3 / 4) - 15, (cx * 5 / 4) - 10, "5", 5, BLACK);
  // 6
  drawRectangle(cx * 3 / 4, cy * 5 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 3 / 4, cy * 5 / 4, 75, cy / 2, WHITE);
  writeText((cy * 5 / 4) - 15, (cx * 5 / 4) - 10, "6", 5, BLACK);
  // Cancel
  drawRectangle(cx * 3 / 4, cy * 7 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 3 / 4, cy * 7 / 4, 75, cy / 2, WHITE);
  writeText((cy * 7 / 4) - 15, (cx * 5 / 4) - 10, "X", 5, RED);
  // 7
  drawRectangle(cx / 4, cy / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx / 4, cy / 4, 75, cy / 2, WHITE);
  writeText((cy / 4) - 15, (cx * 7 / 4) - 10, "7", 5, BLACK);
  // 8
  drawRectangle(cx / 4, cy * 3 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx / 4, cy * 3 / 4, 75, cy / 2, WHITE);
  writeText((cy * 3 / 4) - 15, (cx * 7 / 4) - 10, "8", 5, BLACK);
  // 9
  drawRectangle(cx / 4, cy * 5 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx / 4, cy * 5 / 4, 75, cy / 2, WHITE);
  writeText((cy * 5 / 4) - 15, (cx * 7 / 4) - 10, "9", 5, BLACK);
  // SET
  drawRectangle(cx / 4, cy * 7 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx / 4, cy * 7 / 4, 75, cy / 2, WHITE);
  writeText((cy * 7 / 4) - 35, (cx * 7 / 4) - 10, "SET", 4, GREEN);
}

void numPadDistance()
{
  tft.fillScreen(BLACK);
  // Number display bar
  drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
  drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
  writeText((cy / 2) - 50, (cx / 4) - 5, tempDistance, 4, BLACK);
  // .
  drawRectangle(cx * 7 / 4, cy * 7 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 7 / 4, cy * 7 / 4, 75, cy / 2, WHITE);
  writeText((cy * 7 / 4) - 15, (cx / 4) - 10, ".", 5, BLACK);
  // 0
  drawRectangle(cx * 7 / 4, cy * 5 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 7 / 4, cy * 5 / 4, 75, cy / 2, WHITE);
  writeText((cy * 5 / 4) - 15, (cx / 4) - 10, "0", 5, BLACK);
  // 1
  drawRectangle(cx * 5 / 4, cy / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 5 / 4, cy / 4, 75, cy / 2, WHITE);
  writeText((cy / 4) - 15, (cx * 3 / 4) - 10, "1", 5, BLACK);
  // 2
  drawRectangle(cx * 5 / 4, cy * 3 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 5 / 4, cy * 3 / 4, 75, cy / 2, WHITE);
  writeText((cy * 3 / 4) - 15, (cx * 3 / 4) - 10, "2", 5, BLACK);
  // 3
  drawRectangle(cx * 5 / 4, cy * 5 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 5 / 4, cy * 5 / 4, 75, cy / 2, WHITE);
  writeText((cy * 5 / 4) - 15, (cx * 3 / 4) - 10, "3", 5, BLACK);
  // <--
  drawRectangle(cx * 5 / 4, cy * 7 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 5 / 4, cy * 7 / 4, 75, cy / 2, WHITE);
  writeText((cy * 7 / 4) - 35, (cx * 3 / 4) - 10, "<--", 4, RED);
  // 4
  drawRectangle(cx * 3 / 4, cy / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 3 / 4, cy / 4, 75, cy / 2, WHITE);
  writeText((cy / 4) - 15, (cx * 5 / 4) - 10, "4", 5, BLACK);
  // 5
  drawRectangle(cx * 3 / 4, cy * 3 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 3 / 4, cy * 3 / 4, 75, cy / 2, WHITE);
  writeText((cy * 3 / 4) - 15, (cx * 5 / 4) - 10, "5", 5, BLACK);
  // 6
  drawRectangle(cx * 3 / 4, cy * 5 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 3 / 4, cy * 5 / 4, 75, cy / 2, WHITE);
  writeText((cy * 5 / 4) - 15, (cx * 5 / 4) - 10, "6", 5, BLACK);
  // Cancel
  drawRectangle(cx * 3 / 4, cy * 7 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx * 3 / 4, cy * 7 / 4, 75, cy / 2, WHITE);
  writeText((cy * 7 / 4) - 15, (cx * 5 / 4) - 10, "X", 5, RED);
  // 7
  drawRectangle(cx / 4, cy / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx / 4, cy / 4, 75, cy / 2, WHITE);
  writeText((cy / 4) - 15, (cx * 7 / 4) - 10, "7", 5, BLACK);
  // 8
  drawRectangle(cx / 4, cy * 3 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx / 4, cy * 3 / 4, 75, cy / 2, WHITE);
  writeText((cy * 3 / 4) - 15, (cx * 7 / 4) - 10, "8", 5, BLACK);
  // 9
  drawRectangle(cx / 4, cy * 5 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx / 4, cy * 5 / 4, 75, cy / 2, WHITE);
  writeText((cy * 5 / 4) - 15, (cx * 7 / 4) - 10, "9", 5, BLACK);
  // SET
  drawRectangle(cx / 4, cy * 7 / 4, 85, cy / 2 + 10, BLUE);
  drawRectangle(cx / 4, cy * 7 / 4, 75, cy / 2, WHITE);
  writeText((cy * 7 / 4) - 35, (cx * 7 / 4) - 10, "SET", 4, GREEN);
}

void setup(void)
{
  //Serial.begin(9600);
  //Serial1.begin(9600);
  Serial2.begin(9600);

#ifdef USE_ADAFRUIT_SHIELD_PINOUT
  //Serial.println(F("Using Adafruit 2.8\" TFT Arduino Shield Pinout"));
#else
  //Serial.println(F("Using Adafruit 2.8\" TFT Breakout Board Pinout"));
#endif

  //Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());

  tft.reset();

  uint16_t identifier = tft.readID();

  tft.begin(identifier);
  homeMenu(frequency, distance);
}

void loop(void)
{
  // a point object holds x y and z coordinates
  TSPoint p = ts.getPoint();
  // if no presses detected after half a second
  // reset previous presses and turn holding off
  evalPress(p);
  delay(100);
}

void evalPress(TSPoint p)
{
  if ((millis() - lastPressed) >= 200)
  {
    prevPressX = 0;
    prevPressY = 0;
    holding = false;
  }
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (p.z > ts.pressureThreshhold)
  {
    lastPressed = millis();
    Serial.print("X = ");
    Serial.print(p.x);
    Serial.print("\tY = ");
    Serial.println(p.y);
    //Serial.print("PrevX = "); Serial.print(prevPressX);
    //Serial.print("\tPrevY = "); Serial.println(prevPressY);
    // check to see if current press is within same region of previous press
    if ((p.x > (prevPressX - 250) && p.x < (prevPressX + 250)) && (p.y > (prevPressY - 200) && p.y < (prevPressY + 200)))
    {
      holding = true;
    }
    else
    {
      holding = false;
    }

    if (!holding)
    {
      if (homeSection) // if at home section
      {
        // check to see if pressed frequency hit-box
        if ((p.x > 600 && p.x < 800) && (p.y > 315 && p.y < 711))
        {
          frequencySection = true;
          homeSection = false;
          numPadFrequency();
        }
        // check to see if pressed distance hit-box
        else if ((p.x > 252 && p.x < 380) && (p.y > 144 && p.y < 664))
        {

          distanceSection = true;
          homeSection = false;
          numPadDistance();
        }
        // check to see if hit next section play button
        else if ((p.x > 430 && p.x < 700) && (p.y > 720 && p.y < 830))
        {

          tft.fillTriangle(
              cx, cy + 210,      // peak
              cx + 50, cy + 135, // bottom left
              cx - 50, cy + 135, // bottom right
              WHITE);
          tft.fillTriangle(
              cx, cy + 200,      // peak
              cx + 50, cy + 125, // bottom left
              cx - 50, cy + 125, // bottom right
              BLUE);

          delay(500);
          // Display new section with reservoir tube
          nextSectionMenu();
          white = false;
          homeSection = false;
          nextSection = true;
        }
      }
      else if (nextSection) // if at next section
      {
        // check to see if pressed frequency hit-box
        if ((p.x > 600 && p.x < 800) && (p.y > 400 && p.y < 800))
        {
          if (white)
          {
            drawRoundRectangle(cx + 75, cy + 50, 80, 240, BLUE);
            white = false;
          }
          else
          {
            drawRoundRectangle(cx + 75, cy + 50, 80, 240, WHITE);
            white = true;
          }
        }
        // check to see if home section play button
        else if ((p.x > 430 && p.x < 700) && (p.y > 130 && p.y < 260))
        {

          tft.fillTriangle(
              cx, cy - 210,      // peak
              cx + 50, cy - 135, // bottom left
              cx - 50, cy - 135, // bottom right
              WHITE);
          tft.fillTriangle(
              cx, cy - 200,      // peak
              cx + 50, cy - 125, // bottom left
              cx - 50, cy - 125, // bottom right
              BLUE);
          ;

          delay(500);
          // Display new section with reservoir tube
          homeMenu(frequency, distance);
          white = false;
          homeSection = true;
          nextSection = false;
        }
      }
      else if (frequencySection)
      {
        if ((p.x > 730 && p.x < 880) && (p.y > 511 && p.y < 700)) // pressed 0
        {
          //Serial.println(0);
          tempFrequency += "0";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempFrequency, 4, BLACK);
        }
        else if ((p.x > 722 && p.x < 872) && (p.y > 721 && p.y < 915)) // pressed .
        {
          //Serial.println(".");
          if (!alreadyHasDecimal)
          {
            tempFrequency += ".";
            drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
            drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
            writeText((cy / 2) - 50, (cx / 4) - 5, tempFrequency, 4, BLACK);
            alreadyHasDecimal = true;
          }
        }
        else if ((p.x > 533 && p.x < 696) && (p.y > 96 && p.y < 266)) // pressed 1
        {
          //Serial.println(1);
          tempFrequency += "1";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempFrequency, 4, BLACK);
        }
        else if ((p.x > 537 && p.x < 697) && (p.y > 284 && p.y < 480)) // pressed 2
        {
          //Serial.println(2);
          tempFrequency += "2";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempFrequency, 4, BLACK);
        }
        else if ((p.x > 526 && p.x < 697) && (p.y > 506 && p.y < 710)) // pressed 3
        {
          //Serial.println(3);
          tempFrequency += "3";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempFrequency, 4, BLACK);
        }
        else if ((p.x > 518 && p.x < 700) && (p.y > 725 && p.y < 920)) // pressed backspace
        {
          Serial.println(tempFrequency[tempFrequency.length() - 1]);
          if (tempFrequency.charAt(tempFrequency.length() - 1) == '.')
          {
            Serial.println("Registerd as Decimal");
            alreadyHasDecimal = false;
          }
          tempFrequency.remove(tempFrequency.length() - 1);
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempFrequency, 4, BLACK);
        }
        else if ((p.x > 319 && p.x < 487) && (p.y > 100 && p.y < 270)) // pressed 4
        {
          //Serial.println(4);
          tempFrequency += "4";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempFrequency, 4, BLACK);
        }
        else if ((p.x > 305 && p.x < 500) && (p.y > 288 && p.y < 488)) // pressed 5
        {
          //Serial.println(5);
          tempFrequency += "5";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempFrequency, 4, BLACK);
        }
        else if ((p.x > 315 && p.x < 491) && (p.y > 500 && p.y < 700)) // pressed 6
        {
          //Serial.println(6);
          tempFrequency += "6";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempFrequency, 4, BLACK);
        }
        else if ((p.x > 332 && p.x < 488) && (p.y > 721 && p.y < 915)) // pressed cancel
        {
          //Serial.println("exiting");
          homeMenu(frequency, distance);
          homeSection = true;
          frequencySection = false;
          alreadyHasDecimal = false;
          tempFrequency = "";
        }
        else if ((p.x > 142 && p.x < 297) && (p.y > 98 && p.y < 273)) // pressed 7
        {
          //Serial.println(7);
          tempFrequency += "7";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempFrequency, 4, BLACK);
        }
        else if ((p.x > 145 && p.x < 296) && (p.y > 292 && p.y < 478)) // pressed 8
        {
          //Serial.println(8);
          tempFrequency += "8";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempFrequency, 4, BLACK);
        }
        else if ((p.x > 153 && p.x < 281) && (p.y > 500 && p.y < 700)) // pressed 9
        {
          //Serial.println(9);
          tempFrequency += "9";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempFrequency, 4, BLACK);
        }
        else if ((p.x > 148 && p.x < 280) && (p.y > 720 && p.y < 922)) // pressed SET
        {
          //Serial.println("Setting");
          frequency = tempFrequency.toFloat();
          Serial2.println("f" + tempFrequency);
          homeMenu(frequency, distance);
          homeSection = true;
          frequencySection = false;
          alreadyHasDecimal = false;
          tempFrequency = "";
        }
      }

      else if (distanceSection)
      {
        if ((p.x > 730 && p.x < 880) && (p.y > 511 && p.y < 700)) // pressed 0
        {
          //Serial.println(0);
          tempDistance += "0";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempDistance, 4, BLACK);
        }

        else if ((p.x > 722 && p.x < 872) && (p.y > 721 && p.y < 915)) // pressed .
        {
          //Serial.println(".");
          if (!alreadyHasDecimal)
          {
            tempDistance += ".";
            drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
            drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
            writeText((cy / 2) - 50, (cx / 4) - 5, tempDistance, 4, BLACK);
            alreadyHasDecimal = true;
          }
        }

        else if ((p.x > 533 && p.x < 696) && (p.y > 96 && p.y < 266)) // pressed 1
        {
          //Serial.println(1);
          tempDistance += "1";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempDistance, 4, BLACK);
        }
        else if ((p.x > 537 && p.x < 697) && (p.y > 284 && p.y < 480)) // pressed 2
        {
          //Serial.println(2);
          tempDistance += "2";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempDistance, 4, BLACK);
        }
        else if ((p.x > 526 && p.x < 697) && (p.y > 506 && p.y < 710)) // pressed 3
        {
          //Serial.println(3);
          tempDistance += "3";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempDistance, 4, BLACK);
        }
        else if ((p.x > 518 && p.x < 700) && (p.y > 725 && p.y < 920)) // pressed backspace
        {

          tempDistance.remove(tempFrequency.length() - 1);
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempDistance, 4, BLACK);
        }
        else if ((p.x > 319 && p.x < 487) && (p.y > 100 && p.y < 270)) // pressed 4
        {
          //Serial.println(4);
          tempDistance += "4";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempDistance, 4, BLACK);
        }
        else if ((p.x > 305 && p.x < 500) && (p.y > 288 && p.y < 488)) // pressed 5
        {
          //Serial.println(5);
          tempDistance += "5";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempDistance, 4, BLACK);
        }
        else if ((p.x > 315 && p.x < 491) && (p.y > 500 && p.y < 700)) // pressed 6
        {
          //Serial.println(6);
          tempDistance += "6";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempDistance, 4, BLACK);
        }
        else if ((p.x > 332 && p.x < 488) && (p.y > 721 && p.y < 915)) // pressed cancel
        {
          //Serial.println("exiting");
          homeMenu(frequency, distance);
          homeSection = true;
          distanceSection = false;
          tempDistance = "";
        }
        else if ((p.x > 142 && p.x < 297) && (p.y > 98 && p.y < 273)) // pressed 7
        {
          //Serial.println(7);
          tempDistance += "7";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempDistance, 4, BLACK);
        }
        else if ((p.x > 145 && p.x < 296) && (p.y > 292 && p.y < 478)) // pressed 8
        {
          //Serial.println(8);
          tempDistance += "8";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempDistance, 4, BLACK);
        }
        else if ((p.x > 153 && p.x < 281) && (p.y > 500 && p.y < 700)) // pressed 9
        {
          //Serial.println(9);
          tempDistance += "9";
          drawRectangle(cx * 7 / 4, cy / 2, 85, cy + 10, BLUE);
          drawRectangle(cx * 7 / 4, cy / 2, 75, cy, WHITE);
          writeText((cy / 2) - 50, (cx / 4) - 5, tempDistance, 4, BLACK);
        }
        else if ((p.x > 148 && p.x < 280) && (p.y > 720 && p.y < 922)) // pressed SET
        {
          //Serial.println("Setting");
          distance = tempDistance.toFloat();
          if ((distance > 0.1) && (distance < 29.0))
          {
            Serial2.println("r" + tempDistance);
            //Serial.println("just sent data");
            //Serial.println(distance);
          }
          else
          {
            Serial2.println("x");
            //Serial.println("just sent x");
          }
          homeMenu(frequency, distance);
          homeSection = true;
          distanceSection = false;
          tempDistance = "";
        }
      }

      //Serial.print("\tPressure = "); Serial.println(p.z);
      //Serial.print("boolean"); Serial.println(holding);
      prevPressX = p.x;
      prevPressY = p.y;
    }
    //Serial.print(millis()); Serial.print("\t"); Serial.println(lastPressed);
  }
}
