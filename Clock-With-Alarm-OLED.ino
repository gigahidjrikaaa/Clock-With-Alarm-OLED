/************************************************************************/
/************************************************************************/
/******                                                            ******/
/******            Code made by Giga Hidjrika Aura Adkhy           ******/
/******                      21/479228/TK/52833                    ******/
/******             Alarm Clock with OLED & Matrix Keypad          ******/
/******           Assign an Alarm and able to show the time        ******/
/******                                                            ******/
/******                                                            ******/
/************************************************************************/
/************************************************************************/

#include <WiFi.h>
#include <NTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // Obtained from I2C Scanner. Look for the program in Github.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // OLED class object declaration

// const char* ssid = "Rumah Wisda";
// const char* password = "87654321";

const char* ssid = "Muh Hadjar ORBIT";
const char* password = "12345678Mh.";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

const int numRows = 4; // Number of rows in the keypad
const int numCols = 4; // Number of columns in the keypad

// Define the pins for the keypad
int colPins[numCols] = {27, 14, 12, 13};
int rowPins[numRows] = {32, 33, 25, 26};
int buzzerPin = 34;

// Function to make the OLED displays content centered with X and Y offset
void displayCenter(String text, int X, int Y) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.clearDisplay();
  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  display.setCursor((SCREEN_WIDTH - width) / 2 + X, (SCREEN_HEIGHT - height) / 2 + Y);
  display.println(text);
  display.display();
  delay(300);
}

void splashScreen()
{
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(3);
  displayCenter("ALARM", 0, -16);
  displayCenter("CLOCK", 0, 16);
  delay(2000);
  display.clearDisplay();
}

// function to display the time on the OLED
void displayTime() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  displayCenter(timeClient.getFormattedTime(), 0, 0);
  display.display();
}

// Define the characters for each key
char keyMap[numRows][numCols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Declare a global variable for the current row and current column read.
int currRow, currCol;

char readKeypad() {
  for (int i = 0; i < numRows; i++) {
    digitalWrite(rowPins[i], LOW); // Set the current row low
    for (int j = 0; j < numCols; j++) {
      if (digitalRead(colPins[j]) == LOW) {
        delay(10); // Debounce the button press
        digitalWrite(rowPins[i], HIGH); // Set the row back to high
        currRow = i;
        currCol = j;
        return keyMap[i][j]; // Return the pressed key
      }
    }
    digitalWrite(rowPins[i], HIGH); // Set the row back to high
  }
  return '\0'; // Return null if no key is pressed
}

void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  timeClient.begin();
  timeClient.setTimeOffset(25200); // GMT+7 Offset
  for (int i = 0; i < numRows; i++) {
    pinMode(rowPins[i], OUTPUT); // Set rows as output pins
    digitalWrite(rowPins[i], HIGH); // Set rows high initially
  }
  for (int i = 0; i < numCols; i++) {
    pinMode(colPins[i], INPUT_PULLUP); // Set columns as input pins with pull-up resistors
  }
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  splashScreen();
}

int alarmHour[2] = {0, 0};
int alarmMinute[2] = {0, 0};
bool setHour = true;
bool setAlarmMode = false;
unsigned long updateTimer = 0;

void loop() {
  if(millis() - updateTimer > 1000) {
    timeClient.update();
    Serial.println(timeClient.getFormattedTime());
    updateTimer = millis();
  }

  char keyPressed = readKeypad(); // Read the keypad and get the pressed key
  if (keyPressed != '\0') { 
    if(keyPressed == "A") {
      setAlarmMode = true;
    }
    else if(keyPressed == "B") {
      setHour = true;
    }
    else if(keyPressed == "C") {
      setHour = false;
    }
    else if (keyPressed == 'D')
    {
      setAlarmMode = false;
    }
  }

  if(setAlarmMode)
  {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    displayCenter("Set Alarm", 0, -16);
    displayCenter(String(alarmHour) + ":" + String(alarmMinute), 0, 16);
    display.display();
    // set alarm hour by pressing the number keys
    if(keyPressed >= '0' && keyPressed <= '9' && setHour) {
      alarmHour[0] = alarmHour[1];
      alarmHour[1] = keyPressed - '0';
    }
    if(keyPressed == '*' && setHour) {
      // decrement alarm hour
      alarmHour[1]--;
      if(alarmHour[1] < 0) {
        alarmHour[1] = 9;
        alarmHour[0]--;
        if(alarmHour[0] < 0) {
          alarmHour[0] = 2;
        }
      }
    }
    if(keyPressed == '#' && setHour) {
      // increment alarm hour
      alarmHour[1]++;
      if(alarmHour[1] > 9) {
        alarmHour[1] = 0;
        alarmHour[0]++;
        if(alarmHour[0] > 2) {
          alarmHour[0] = 0;
        }
      }
    }
    // set alarm minute by pressing the number keys
    else if(keyPressed >= '0' && keyPressed <= '9' && !setHour) {
      alarmMinute[0] = alarmMinute[1];
      alarmMinute[1] = keyPressed - '0';
    }
  }
  else
  {
    displayTime();
  }
  
}
