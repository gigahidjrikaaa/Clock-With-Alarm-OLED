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

const char* ssid = "Rumah Wisda";
const char* password = "87654321";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

const int numRows = 4; // Number of rows in the keypad
const int numCols = 4; // Number of columns in the keypad

// Define the pins for the keypad
int colPins[numCols] = {27, 14, 12, 13};
int rowPins[numRows] = {32, 33, 25, 26};
int buzzerPin = 23;

int alarmHour[2] = {0, 0};
int alarmMinute[2] = {0, 0};
bool setHour = true;
bool setAlarmMode = false;
bool alarmOn = false;
bool silenced = false;
unsigned long updateTimer = 0;
unsigned long refreshTimer = 0;

// Declare a global variable for the current row and current column read.
int currRow, currCol;
int lastKeyPressed;

// Current time
int hour = timeClient.getHours();
int minute = timeClient.getMinutes();
int second = timeClient.getSeconds();

int alarmHourInt = (alarmHour[0] * 10) + alarmHour[1];
int alarmMinuteInt = (alarmMinute[0] * 10) + alarmMinute[1];

// Function to make the OLED displays content centered with X and Y offset
void displayCenter(String text, int X, int Y) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  display.setCursor((SCREEN_WIDTH - width) / 2 + X, (SCREEN_HEIGHT - height) / 2 + Y);
  display.println(text);
  display.display();
}

void splashScreen()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(3);
  displayCenter("ALARM", 0, -16);
  displayCenter("CLOCK", 0, 16);
  // turn on the buzzer
  tone(buzzerPin, 500, 1000);
  tone(buzzerPin, 1000, 1000);
  delay(2000);
  noTone(buzzerPin);
  display.clearDisplay();
}

// function to display the time on the OLED
void displayTime() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  displayCenter(timeClient.getFormattedTime(), 0, 8);
  display.setTextSize(1);
  displayCenter("Alarm set: " + String(alarmHour[0]) + String(alarmHour[1]) + ":" + String(alarmMinute[0]) + String(alarmMinute[1]), 0, -16);
  display.display();
}

// Define the characters for each key
char keyMap[numRows][numCols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

char readKeypad() {
  for (int i = 0; i < numRows; i++) {
    digitalWrite(rowPins[i], LOW); // Set the current row low
    for (int j = 0; j < numCols; j++) {
      if (digitalRead(colPins[j]) == LOW) {
        delay(25); // Debounce the button press
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

void alarmCheck()
{
  if (hour == alarmHourInt && minute == alarmMinuteInt && !silenced) {
    alarmOn = true;
    digitalWrite(buzzerPin, HIGH);
    display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE);
    displayCenter("ALARM!!", 0, 0);
    display.display();
  }
  else if(hour != alarmHourInt || minute != alarmMinuteInt) {
    silenced = false;
    alarmOn = false;
    digitalWrite(buzzerPin, LOW);
  }
  else if(silenced)
  {
    alarmOn = false;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi Not Connected...");
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
  pinMode(buzzerPin, OUTPUT);
  splashScreen();
}

void loop() {
  if(millis() - updateTimer > 1000) {
    timeClient.update();
    Serial.println(timeClient.getFormattedTime());
    hour = timeClient.getHours();
    minute = timeClient.getMinutes();
    second = timeClient.getSeconds();
    updateTimer = millis();
    alarmHourInt = (alarmHour[0] * 10) + alarmHour[1];
    alarmMinuteInt = (alarmMinute[0] * 10) + alarmMinute[1];
  }

  char keyPressed = readKeypad(); // Read the keypad and get the pressed key
  if (keyPressed != '\0' && keyPressed != lastKeyPressed) { 
    if(keyPressed == 'A') {
      setAlarmMode = true;
      silenced = false;
    }
    else if(keyPressed == 'B') {
      setHour = true;
    }
    else if(keyPressed == 'C') {
      setHour = false;
    }
    else if (keyPressed == 'D')
    {
      setAlarmMode = false;
    }
    if(alarmOn && keyPressed == '0')
    {
      silenced = true;
      display.clearDisplay();
      display.setTextSize(2);
      displayCenter("Alarm Off", 0, 0);
      digitalWrite(buzzerPin, LOW);
      delay(1500);
    }
  }
  else if(keyPressed == '\0')
  {
    lastKeyPressed = '\0';
  }

  if(setAlarmMode)
  {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    displayCenter("Set Alarm", 0, -16);
    displayCenter(String(alarmHour[0]) + String(alarmHour[1]) + ":" + String(alarmMinute[0]) + String(alarmMinute[1]), 0, 16);
    display.display();

    
    if(lastKeyPressed != keyPressed)
    {
      if(setHour)
      {
        if(keyPressed >= '0' && keyPressed <= '9') {
          alarmHour[0] = alarmHour[1];
          alarmHour[1] = keyPressed - '0';
        }
        if(keyPressed == '*') {
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
        if(keyPressed == '#') {
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
        if(alarmHour[0] > 2) {
          alarmHour[0] = 2;
        }
        if(alarmHour[0] == 2 && alarmHour[1] > 3) {
          alarmHour[1] = 3;
        }
      }
      // set alarm minute by pressing the number keys
      else
      {
        if(keyPressed >= '0' && keyPressed <= '9') {
          alarmMinute[0] = alarmMinute[1];
          alarmMinute[1] = keyPressed - '0';
        }
        if(keyPressed == '*') {
          // decrement alarm minute
          alarmMinute[1]--;
          if(alarmMinute[1] < 0) {
            alarmMinute[1] = 9;
            alarmMinute[0]--;
            if(alarmMinute[0] < 0) {
              alarmMinute[0] = 5;
            }
          }
        }
        if(keyPressed == '#') {
          // increment alarm minute
          alarmMinute[1]++;
          if(alarmMinute[1] > 9) {
            alarmMinute[1] = 0;
            alarmMinute[0]++;
            if(alarmMinute[0] > 5) {
              alarmMinute[0] = 0;
            }
          }
        }
        if(alarmMinute[0] > 5) {
          alarmMinute[0] = 5;
        }
      }
    }
  }
  else
  {
    if(millis() - refreshTimer > 500)
    {
      if(!alarmOn)
        displayTime();
      alarmCheck();
      refreshTimer = millis();
    }
  }
  
  lastKeyPressed = keyPressed;
}
