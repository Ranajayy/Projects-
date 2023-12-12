#include "RTClib.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "DFRobotDFPlayerMini.h"

DFRobotDFPlayerMini player;

bool alarm_flag = false;

#define DHTPIN 4    // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT11     // DHT 11
DHT_Unified dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);
int tempC;
double tempK;
int tempReading;
unsigned long starttime;
float temperature;
float humidity;

byte clocksymbol[8] = {
  0b00000,
  0b00100,
  0b01110,
  0b01110,
  0b11111,
  0b00000,
  0b00100,
  0b00000,
};

byte temp[8] = 
{
  0b10000,
  0b00111,
  0b01000,
  0b01000,
  0b01000,


  0b01000,
  0b00111,
  0b00000,
};

static const int DHT_SENSOR_PIN = 2;
const int buzzer = 9;
const int busy_pin = 8;







RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const int btSet = 7;
const int btAdj = 6;
const int btAlarm = 5;

long interval = 300;
int melody[] = { 600, 800, 1000, 1200 };

//Variables
int DD, MM, YY, H, M, S, set_state, adjust_state, alarm_state, AH, AM, shake_state;
int shakeTimes = 0;
int i = 0;
int btnCount = 0;
String sDD;
String sMM;
String sYY;
String sH;
String sM;
String sS;
String aH = "12";
String aM = "00";
String alarm = "     ";
long previousMillis = 0;

//Boolean flags
boolean setupScreen = false;
boolean alarmON = false;
boolean turnItOn = false;
boolean backlightON = true;

void setup()
{
  Serial.begin(9600);

  lcd.init();                      // initialize the lcd
  lcd.backlight();
  dht.begin();
  if (! rtc.begin())
  {
    //Serial.println("Couldn't find RTC");
    while (1);
  }
  if (!player.begin(Serial))
  { //Use softwareSerial to communicate with mp3.

    while (true);
  }
  // Serial.println(F("DFPlayer Mini online."));


  //Set outputs/inputs
  pinMode(btSet, INPUT_PULLUP);
  pinMode(btAdj, INPUT_PULLUP);
  pinMode(btAlarm, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);

  lcd.createChar(1, clocksymbol);
  lcd.createChar(2, temp);

  pinMode(busy_pin, INPUT);


  rtc.adjust(DateTime(__DATE__, __TIME__));
  delay(100);
  //Read alarm time from EEPROM memmory
  AH = EEPROM.read(0);
  AM = EEPROM.read(1);
  //Check if the numbers that you read are valid. (Hours:0-23 and Minutes: 0-59)
  if (AH > 23) {
    AH = 0;
  }
  if (AM > 59) {
    AM = 0;
  }


  //starttime=millis();



}

void loop()
{
  readBtns();       //Read buttons
  getTimeDate();    //Read time and date from RTC

  if (!setupScreen)
  {
    lcdPrint();     // print the current time/date/alarm to the LCD
    if (alarmON)
    {
      callAlarm();   // and check the alarm if set on
    }
    else
    {
      lcd.setCursor(10, 0); // when no alarm is set we remove the bell icon that states wether alarm is on or off
      lcd.write(" ");
    }
  }
  else
  {
    timeSetup();    //If button set is pressed then call the time setup function

  }

  // here we get the current temperature from dht sensor and save in a variable
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    //Serial.println(F("Error reading temperature!"));
  }
  else {
    // Serial.print(F("Temperature: "));
    // Serial.print(event.temperature);
    // Serial.println(F("°C"));
    tempC = event.temperature;
  }

}



//Read buttons state and perform different actions on them

void readBtns() // we read button states here and use them to set time and set the alamr and date and the buttons are pulled up internally so when ever a button
// pressed it will give low to the micro controller and the remaining logic is implemented on it
{
  set_state = digitalRead(btSet); // read the set state button used to go to the set time and date and alarm screen
  adjust_state = digitalRead(btAdj); // used to increment numbers of time and date
  alarm_state = digitalRead(btAlarm); // used to decerement numbers of time and date  when in set alarm screen or date or time screen and also used to  turn on alarm and off
  if (!setupScreen)
  {
    if (alarm_state == LOW)
    {
      if (alarmON)
      {
        alarm = "     ";
        alarmON = false;
        player.stop();
      }
      else
      {
        alarmON = true;
      }
      delay(500);
    }


  }
  if (set_state == LOW)  //here we go to the set time and date screen usign the set state button
  {
    if (btnCount < 7)
    {
      btnCount++;
      setupScreen = true;
      if (btnCount == 1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("------SET------");
        lcd.setCursor(0, 1);
        lcd.print("-TIME and DATE-");
        delay(2000);
        lcd.clear();
      }
    }
    else {
      lcd.clear();
      rtc.adjust(DateTime(YY, MM, DD, H, M, 0)); //Save time and date to RTC IC
      EEPROM.write(0, AH);  //Save the alarm hours to EEPROM 0
      EEPROM.write(1, AM);  //Save the alarm minuted to EEPROM 1
      lcd.print("Saving....");
      delay(2000);
      lcd.clear();
      setupScreen = false;
      btnCount = 0;
    }
    delay(500);
  }
}

//Read time and date from rtc module and update on screen
void getTimeDate()
{
  if (!setupScreen)
  {
    DateTime now = rtc.now();
    DD = now.day();
    MM = now.month();
    YY = now.year();
    H = now.hour();
    M = now.minute();
    S = now.second();
  }
  //Make some fixes to display on lcd screen
  if (DD < 10) {
    sDD = '0' + String(DD);
  } else {
    sDD = DD;
  }
  if (MM < 10) {
    sMM = '0' + String(MM);
  } else {
    sMM = MM;
  }
  sYY = YY - 2000;
  if (H < 10) {
    sH = '0' + String(H);
  } else {
    sH = H;
  }
  if (M < 10) {
    sM = '0' + String(M);
  } else {
    sM = M;
  }
  if (S < 10) {
    sS = '0' + String(S);
  } else {
    sS = S;
  }
  if (AH < 10) {
    aH = '0' + String(AH);
  } else {
    aH = AH;
  }
  if (AM < 10) {
    aM = '0' + String(AM);
  }  else {
    aM = AM;
  }
}

//this function prints the data on lcd
void lcdPrint() {
  String line1 = sH + ":" + sM + ":" + sS + " |";
  lcd.setCursor(0, 0); //First row
  lcd.print(line1);
  if (alarmON) {
    lcd.setCursor(10, 0);
    lcd.write(1);
  }
  String line2 = aH + ":" + aM;
  lcd.setCursor(11, 0);
  lcd.print(line2);

  String line3 = sDD + "-" + sMM + "-" + sYY + " | " + tempC ;
  //  Serial.println(tempC);
  lcd.setCursor(0, 1); //Second row
  lcd.print(line3);
  if (setupScreen) {
    lcd.setCursor(13, 1);
    lcd.print("");
  }
  else {
    lcd.setCursor(13, 1);
    lcd.write(2);
  }

}

// in this fucntion we set the time and date and alarm
void timeSetup()
{
  int up_state = adjust_state;
  int down_state = alarm_state;
  if (btnCount <= 5) {
    if (btnCount == 1)
    { //Set Hour
      lcd.setCursor(4, 0); // set cursor on screen  to display the marker which shows which number we are changing on the lcd
      lcd.print(">");
      if (up_state == LOW)
      { //Up button +
        if (H < 23) // hours of time go to max 23 and than to 0
        {
          H++;
        }
        else
        {
          H = 0;
        }
        delay(350);
      }
      if (down_state == LOW)
      { //Down button - to decrement the numbers
        if (H > 0)
        {
          H--;
        }
        else {
          H = 23;
        }
        delay(350);
      }
    }
    else if (btnCount == 2)  // this is the set state button and when its pressed btncount is incremented to go to  a next screen
    { //Set  Minutes
      lcd.setCursor(4, 0);
      lcd.print(" ");
      lcd.setCursor(9, 0);
      lcd.print(">");
      if (up_state == LOW)
      {
        if (M < 59) {
          M++;
        }
        else {
          M = 0;
        }
        delay(350);
      }
      if (down_state == LOW)
      {
        if (M > 0) {
          M--;
        }
        else {
          M = 59;
        }
        delay(350);
      }
    }
    else if (btnCount == 3)
    { //Set Day
      lcd.setCursor(9, 0);
      lcd.print(" ");
      lcd.setCursor(0, 1);
      lcd.print(">");

      //increment in day
      if (up_state == LOW) {
        if (DD < 31) {
          DD++;
        }
        else {
          DD = 1;
        }
        delay(350);
      }
      //decrement in day
      if (down_state == LOW)
      {
        if (DD > 1) {
          DD--;
        }
        else {
          DD = 31;
        }
        delay(350);
      }
    }
    else if (btnCount == 4)
    { //Set Month
      lcd.setCursor(0, 1);
      lcd.print(" ");
      lcd.setCursor(5, 1);
      lcd.print(">");
      if (up_state == LOW) //incrmenet
      {
        if (MM < 12) {
          MM++;
        }
        else {
          MM = 1;
        }
        delay(350);
      }
      if (down_state == LOW) { //decrement in day
        if (MM > 1) {
          MM--;
        }
        else {
          MM = 12;
        }
        delay(350);
      }
    }
    else if (btnCount == 5) {   //Set Year
      lcd.setCursor(5, 1);
      lcd.print(" ");
      lcd.setCursor(10, 1);
      lcd.print(">");
      if (up_state == LOW) {
        if (YY < 2999) {
          YY++;
        }
        else {
          YY = 2000;
        }
        delay(350);
      }
      if (down_state == LOW) {
        if (YY > 2018) {
          YY--;
        }
        else {
          YY = 2999;
        }
        delay(350);
      }
    }
    lcd.setCursor(5, 0);
    lcd.print(sH);
    lcd.setCursor(8, 0);
    lcd.print(":");
    lcd.setCursor(10, 0);
    lcd.print(sM);
    lcd.setCursor(1, 1);
    lcd.print(sDD);
    lcd.setCursor(4, 1);
    lcd.print("-");
    lcd.setCursor(6, 1);
    lcd.print(sMM);
    lcd.setCursor(9, 1);
    lcd.print("-");
    lcd.setCursor(11, 1);
    lcd.print(sYY);
  }
  //after we set the time and date we go to the set alarm screen where we set the alarm time
  else
  {
    setAlarmTime();
  }
}

//This function is used to set the alarm time
void setAlarmTime()
{
  int up_state = adjust_state;  //
  int down_state = alarm_state; //
  String line2;
  lcd.setCursor(0, 0);
  lcd.print("SET  ALARM TIME");
  if (btnCount == 6)
  { //Set alarm Hour
    if (up_state == LOW)
    {
      if (AH < 23) {
        AH++;  // alarm hours
      }
      else {
        AH = 0;
      }
      delay(350);
    }
    if (down_state == LOW)
    {
      if (AH > 0)
      {
        AH--;
      }
      else {
        AH = 23;
      }
      delay(350);
    }
    line2 = "    >" + aH + " : " + aM + "    ";
  }
  else if (btnCount == 7)
  { //Set alarm Minutes
    if (up_state == LOW)
    {
      if (AM < 59)
      {
        AM++; //alarm minutes
      }
      else
      {
        AM = 0;
      }
      delay(350);
    }
    if (down_state == LOW) {
      if (AM > 0) {
        AM--;
      }
      else {
        AM = 59;
      }
      delay(350);
    }
    line2 = "     " + aH + " :>" + aM + "    ";
  }
  lcd.setCursor(0, 1);
  lcd.print(line2); // line 2 will be displayed ons creen
}


//this fucntion is used to play sound when set alarm time is met
void callAlarm()

{
  if (aM == sM && aH == sH && S < 3) // if alarm minutes == current minutes and alarm hours and current hours are equal  a flag is made true which turns alarm on and plays the sound using df player
  {
    turnItOn = true;
    // Serial.println("turn on truee");

  }
  else if (alarm_state == LOW || (S >= 59))
  {
    //  Serial.println("turnon false");
    turnItOn = false;
    alarmON = true;

    delay(50);
  }

  if (turnItOn)
  {
    //Serial.println("alarm on");
    if (alarm_flag == false)
    {
      alarm_flag = true;

      player.volume(30); // used to set the volume of df player
      player.play(1); // plays the current file in sd card

    }
    else
    {
      //Serial.println("Connecting to DFPlayer Mini failed!");
    }

    if (digitalRead(busy_pin) == HIGH) // this logic is used to make the alarm sound loop on untill stop button is pressed
    {
      alarm_flag = false;
    }
  }


  else
  {
    // Serial.println("alarm off");
    //player.stop();
  }
}
