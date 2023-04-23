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
#include <EEPROM.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // Obtained from I2C Scanner. Look for the program in Github.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // OLED class object declaration

const char* ssid = "Rumah Wisda";
const char* password = "87654321";

// NTP objects
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Keypad rows and columns
const int numRows = 4; // Number of rows in the keypad
const int numCols = 4; // Number of columns in the keypad

// Define the pins for the keypad
int rowPins[numCols] = {13, 12, 14, 27};
int colPins[numRows] = {26, 25, 33, 32};
int buzzerPin = 23;

// Store hour and minutes per digit
int alarmHour[2] = {0, 0};
int alarmMinute[2] = {0, 0};
// Flag to mark setting the hour
bool setHour = true;
// Flag to mark alarm setting mode
bool setAlarmMode = false;
// Flag to mark help mode
bool helpMode = false;
// Flag to mark alarm is on
bool alarmOn = false;
// Flag to mark silence button has been pressed
bool silenced = false;
// NTP Update timer
unsigned long updateTimer = 0;

// Get current hour, minute, and second using NTP
int hour = timeClient.getHours();
int minute = timeClient.getMinutes();

// Current alarm hour as integer
int alarmHourInt = (alarmHour[0] * 10) + alarmHour[1];
int alarmMinuteInt = (alarmMinute[0] * 10) + alarmMinute[1];

// Declare a global variable for the current row and current column read. Also for the last key pressed and the current key pressed.
int currRow, currCol;
char lastKeyPressed;
char keyPressed;

// Define the characters for each key
char keyMap[numRows][numCols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Function to read the keypad
char readKeypad() {
  for (int i = 0; i < numRows; i++) {
    digitalWrite(rowPins[i], LOW);          // Set the current row low
    for (int j = 0; j < numCols; j++) {
      if (digitalRead(colPins[j]) == LOW) {
        delay(25);                          // Debounce the button press
        digitalWrite(rowPins[i], HIGH);     // Set the row back to high
        currRow = i;
        currCol = j;
        return keyMap[i][j];                // Return the pressed key
      }
    }
    digitalWrite(rowPins[i], HIGH);         // Set the row back to high
  }
  return '\0';                              // Return null if no key is pressed
}

// Function to check if current time is the same as the alarm time
void alarmCheck()
{
  if (hour == alarmHourInt && minute == alarmMinuteInt && !silenced) {
    alarmOn = true;
    digitalWrite(buzzerPin, HIGH);
    display.clearDisplay();
    display.setTextSize(3);
    displayCenter("ALARM!!", 0, 0);
    display.display();
  }
  else if(hour != alarmHourInt || minute != alarmMinuteInt) {
    silenced = false;
    alarmOn = false;
    digitalWrite(buzzerPin, LOW);
    display.clearDisplay();
  }
  else if(silenced)
  {
    alarmOn = false;
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize the EEPROM
  EEPROM.begin(4);

  // Read the alarm time from EEPROM
  alarmHour[0] = EEPROM.read(0);
  alarmHour[1] = EEPROM.read(1);
  alarmMinute[0] = EEPROM.read(2);
  alarmMinute[1] = EEPROM.read(3);

  // Initialize the OLED display1
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  splashScreen();

  WiFi.begin(ssid, password);       // Connect to the network
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi Not Connected...");
    display.setTextSize(2);
    displayCenter("Wifi off", 0, 0);
  }
  display.clearDisplay();

  timeClient.begin();               // Start the NTP client
  timeClient.setTimeOffset(25200);  // GMT+7 Offset

  for (int i = 0; i < numRows; i++) {
    pinMode(rowPins[i], OUTPUT);    // Set rows as output pins
    digitalWrite(rowPins[i], HIGH); // Set rows high initially
  }
  for (int i = 0; i < numCols; i++) {
    pinMode(colPins[i], INPUT_PULLUP); // Set columns as input pins with pull-up resistors
  }
  pinMode(buzzerPin, OUTPUT);       // Set buzzer pin as output
}

void loop() {

  keyPressed = readKeypad();        // Read the keypad and get the pressed key
  if(setAlarmMode)                  // If in alarm setting mode
  { 
    modifyAlarm();                  // Run the modifyAlarm function
  }
  else if(helpMode)
  {
    helpMenu();
  }
  else
  {
    if(millis() - updateTimer > 1000) {
      timeClient.update();              // Update the time
      Serial.println(timeClient.getFormattedTime());
      hour = timeClient.getHours();
      minute = timeClient.getMinutes();

      alarmHourInt = (alarmHour[0] * 10) + alarmHour[1];
      alarmMinuteInt = (alarmMinute[0] * 10) + alarmMinute[1];

      if(!alarmOn)
      {
        displayAlarm();
        displayTime();
        displayDate();
        display.display();
        display.clearDisplay();
      }
      alarmCheck();
      updateTimer = millis();
    }
  }

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
    else if (keyPressed == 'D' && setAlarmMode)
    {
      setAlarmMode = false;
      setHour = true;
      correctAlarm();
      display.clearDisplay();
    }
    else if(keyPressed == '0')
    {
      if(alarmOn)
        silenceAlarm();
      else {
        if(!setAlarmMode){
        helpMode = !helpMode;
        display.clearDisplay();
        }
      }
    }
    else{};                           // Do nothing if number key is pressed
  }

  lastKeyPressed = keyPressed;
}

// function to display triangle on the OLED with X and Y offset and a b size
void displayTriangle(int X, int Y, int a, int b) {
  display.drawLine(X, Y, X + a, Y, SSD1306_WHITE);
  display.drawLine(X, Y, X + (a / 2), Y + b, SSD1306_WHITE);
  display.drawLine(X + a, Y, X + (a / 2), Y + b, SSD1306_WHITE);
}

// Function to make the OLED displays content centered with X and Y offset
void displayCenter(String text, int X, int Y) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
  displayBorder();

  display.setCursor((SCREEN_WIDTH - width) / 2 + X, (SCREEN_HEIGHT - height) / 2 + Y);
  display.println(text);
}

// Display Splash Screen to OLED
void splashScreen()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(3);
  displayCenter("ALARM", 0, -16);
  displayCenter("CLOCK", 0, 16);
  display.display();
  tone(buzzerPin, 2000, 1000);      // turn on the buzzer for 1 second
  delay(2000);
  noTone(buzzerPin);                // turn off the buzzer
  display.clearDisplay();
}

// function to display a border around the OLED
void displayBorder() {
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
}

// function to display the time on the OLED
void displayTime() {
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  displayCenter(timeClient.getFormattedTime(), 0, 0);
}

// function to get the date from the NTP server and display it on the OLED
void displayDate() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  String year = timeClient.getFormattedDate().substring(0, 4);
  String month = timeClient.getFormattedDate().substring(5, 7);
  String day = timeClient.getFormattedDate().substring(8, 10);
  displayCenter("---- " + day + "/" + month + "/" + year + " ----", 0, 24);
}

// function to display the alarm time on the OLED
void displayAlarm() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  displayCenter("Alarm set: " + String(alarmHour[0]) + String(alarmHour[1]) + ":" + String(alarmMinute[0]) + String(alarmMinute[1]), 0, -24);
}

// function to display the alarm setting mode on the OLED
void displayAlarmMode() {
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  displayCenter("Set Alarm", 0, -18);
  displayCenter(String(alarmHour[0]) + String(alarmHour[1]) + ":" + String(alarmMinute[0]) + String(alarmMinute[1]), 0, 18);
  if(setHour) {
    displayTriangle(40, 27, 10, 8);
  } else {
    displayTriangle(75, 27, 10, 8);
  }
  display.display();
}

// function to modify the alarm time
void modifyAlarm()
{
  display.clearDisplay();
  if(lastKeyPressed != keyPressed)        // prevent multiple input
  {
    if(setHour)
    {
      if(keyPressed >= '0' && keyPressed <= '9') {    // set alarm hour if number key is pressed 
        alarmHour[0] = alarmHour[1];
        alarmHour[1] = keyPressed - '0';
      }
      if(keyPressed == '*') {
        alarmHour[1]--;               // decrement alarm hour
        if(alarmHour[1] < 0) {
          alarmHour[1] = 9;
          alarmHour[0]--;
          if(alarmHour[0] < 0) {
            alarmHour[0] = 2;
          }
        }
      }
      if(keyPressed == '#') {
        alarmHour[1]++;              // increment alarm hour
        if(alarmHour[1] > 9) {
          alarmHour[1] = 0;
          alarmHour[0]++;
        }
        else if(alarmHour[1] > 3 && alarmHour[0] == 2) {
          alarmHour[1] = 0;
          alarmHour[0] = 0;
        }
      }
    }
    else                            
    {
      if(keyPressed >= '0' && keyPressed <= '9') {  // set alarm minute if key pressed is a number
        alarmMinute[0] = alarmMinute[1];
        alarmMinute[1] = keyPressed - '0';
      }
      if(keyPressed == '*') {
        alarmMinute[1]--;           // decrement alarm minute
        if(alarmMinute[1] < 0) {
          alarmMinute[1] = 9;
          alarmMinute[0]--;
          if(alarmMinute[0] < 0) {
            alarmMinute[0] = 5;
          }
        }
      }
      if(keyPressed == '#') {
        alarmMinute[1]++;           // increment alarm minute
        if(alarmMinute[1] > 9) {
          alarmMinute[1] = 0;
          alarmMinute[0]++;
          if(alarmMinute[0] > 5) {
            alarmMinute[0] = 0;
          }
        }
      }
    }
  }
  displayAlarmMode();
}

void saveProcess()
{
  // if alarm hour and minute is not the same as before, save it to EEPROM
  if(alarmHour[0] != EEPROM.read(0) || alarmHour[1] != EEPROM.read(1) || alarmMinute[0] != EEPROM.read(2) || alarmMinute[1] != EEPROM.read(3)) {
    display.clearDisplay();
    display.setTextSize(2);
    displayCenter("Saved!", 0, 0);
    saveAlarm();
    display.display();
    delay(1500);
    display.clearDisplay();
  }
}

// function to save the alarm time to EEPROM
void saveAlarm()
{
  EEPROM.write(0, alarmHour[0]);
  EEPROM.write(1, alarmHour[1]);
  EEPROM.write(2, alarmMinute[0]);
  EEPROM.write(3, alarmMinute[1]);
  EEPROM.commit();
}

// correct alarm time if it is not valid
void correctAlarm()               
{
  if(alarmHour[0] > 2) {
    alarmHour[0] = 2;
  }
  else if(alarmHour[0] == 2 && alarmHour[1] > 3) {
    alarmHour[1] = 3;
  }
  else if(alarmMinute[0] > 5) {
    alarmMinute[0] = 5;
  }
  else{
    saveProcess();
  }
}

// silence the alarm
void silenceAlarm()               
{
  silenced = true;
  display.clearDisplay();
  display.setTextSize(2);
  displayCenter("Alarm Off", 0, 0);
  display.display();
  digitalWrite(buzzerPin, LOW);
  delay(1500);
  display.clearDisplay();
}

// display help menu
void helpMenu()                   
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  displayCenter("HELP MENU", 0, -24);
  displayCenter("0 - Stop alarm/exit", 0, -10);
  displayCenter("A/D - Set alarm", 0, 0);
  displayCenter("B/C - Chg time unit", 0, 10);
  displayCenter("*/# - Decr./Incr.", 0, 20);
  display.display();
}
