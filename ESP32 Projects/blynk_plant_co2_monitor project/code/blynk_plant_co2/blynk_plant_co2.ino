#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPLY7YvjaHD"
#define BLYNK_TEMPLATE_NAME "plant co2"
#define BLYNK_AUTH_TOKEN "GRyBojdMVw65bLIjAwaS5-_AABjUpn78"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>

#define analogPin 34
#define carbon_valve 26

const char ssid[] = "EXECUTIVE";  // change with your wifi ssid
const char pass[] = "pakistan@786"; // change with your wifi pass


long timer_start_set[2] = {0xFFFF, 0xFFFF};
long timer_stop_set[2] = {0xFFFF, 0xFFFF};
int sh, sm, eh, em;
int rtc_h, rtc_m;
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);


RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String Time = "";

bool e_f, r_f, h_f = false;
float p_c = 0;
int x, z = 0;

unsigned long ppm_millis = 0;
unsigned long rtc_minutes = 0;
unsigned long from_minutes = 0;
unsigned long to_minutes = 0;
unsigned long reading_miilis = 0;
float concentration = 0;
float set_concentration = 0;

BLYNK_WRITE(V0)
{
  set_concentration = param.asFloat();
  Serial.println(set_concentration);
}

BLYNK_WRITE(V1)
{
  TimeInputParam t(param);

  if (t.hasStartTime())
  {
    timer_start_set[0] = (t.getStartHour() * 60 * 60) + (t.getStartMinute() * 60) ;
    sh = int(t.getStartHour());
    sm = int(t.getStartMinute());
    from_minutes = (sh * 60) + sm;
    Serial.println(from_minutes);
    //    Serial.println(sm);
  }
}

BLYNK_WRITE(V2)
{
  TimeInputParam T(param);

  if (T.hasStartTime())
  {
    timer_stop_set[0] = (T.getStartHour() * 60 * 60) + (T.getStartMinute() * 60) ;
    eh = int(T.getStartHour());
    em = int(T.getStartMinute());
    to_minutes = (eh * 60) + em;
    Serial.println(to_minutes);
    //    Serial.println(em);
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(carbon_valve, OUTPUT);
//  lcd.begin();
//  lcd.backlight();
//  if (! rtc.begin())
//  {
//    Serial.println("Couldn't find RTC");
//    while (1);
//  }
//  rtc.adjust(DateTime(__DATE__, __TIME__));

  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  ppm_millis = millis();
  reading_miilis = millis();
  delay(1000);
}

void loop()
{
  Blynk.run();
  time_rtc();
  CO2_code();
  
  if (rtc_minutes >= from_minutes && rtc_minutes <= to_minutes)
  {
    // Serial.println("in operating time zone");
    if (millis() - reading_miilis > 60000) //enter 60000
    {
      reading_miilis = millis();
      CO2_code();

      if (e_f == false && h_f == false)
      {
        if (concentration < set_concentration)
        {
          float diff = set_concentration - concentration;
          int open_time = map(diff, 0, set_concentration, 0, 30000); // enter 30000
          Serial.println(open_time);

          digitalWrite(carbon_valve, HIGH);
          delay(open_time);
          digitalWrite(carbon_valve, LOW);
        }

      }

    }
  }
}

void time_rtc()
{
  DateTime now = rtc.now();
  rtc_h = now.hour();
  rtc_m = now.minute();
  rtc_minutes = (rtc_h * 60) + rtc_m;
  // Time = String(now.hour()) + ":" + String(now.minute());
  // Serial.println(rtc_minutes);

}

void CO2_code()
{
  int adcVal = analogRead(analogPin);
  float voltage = adcVal * (3.3 / 4095.0);

  if (voltage == 0)
  {
    e_f = true;
    h_f = false;
    r_f = false;
    z = 0;

    if (x == 0)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Error in Sensor");
      x++;
    }
    // Serial.println("A problem has occurred with the sensor.");

  }
  else if (voltage < 0.4)
  {
    h_f = true;
    e_f = false;
    r_f = false;
    x = 0;
    //  Serial.println("Pre-heating the sensor...");

    if (z == 0)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pre-Heating Mode");
      z++;
    }
  }

  else
  {

    float voltageDiference = voltage - 0.4;
    concentration = (voltageDiference * 5000.0) / 1.6;

    //    Serial.print("voltage:");
    //    Serial.print(voltage);
    //    Serial.println("V");
    //  Serial.print(concentration);
    //    Serial.println("ppm");

    e_f = false;
    h_f = false;
    r_f = true;
    x = 0;
    z = 0;

    if (millis() - ppm_millis > 2000)
    {
      ppm_millis = millis();
      p_c = concentration;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(String(concentration) + " ppm");
    }
  }
}
