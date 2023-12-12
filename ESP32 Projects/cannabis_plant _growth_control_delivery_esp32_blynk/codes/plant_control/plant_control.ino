#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPLYHIie24n"
#define BLYNK_TEMPLATE_NAME "Valve Control"
#define BLYNK_AUTH_TOKEN "mreqrlt7k0Z0ucZ-sG0L0JD4F_nh2lbL"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Preferences.h>
#include "time.h"


const char ssid[] = "EXECUTIVE";  // change with your wifi ssid
const char pass[] = "pakistan@786"; // change with your wifi pass

const int AirValue = 570; //you need to change this value that you had recorded in t
const int WaterValue = 0; //you need to change this value that you had recorded in th
int intervals = (AirValue - WaterValue) / 3;
int soilMoistureValue = 0;

const int soil_moisture_pin = 39;
const int solenoid_pin = 23;
unsigned long time_to_read_val = 0;
int average_val = 0;
int average = 0;

unsigned long solenoid_open_time = 5000; //5000 means 5 seconds
//unsigned long change_in_time = 2000; //2000 means 2 seconds

bool dry, very_wet, wet = false;
int wet_counter, dry_counter = 0;
bool open_valve = false;
unsigned long solenoid_time = 0;
int hour_count = 0;
unsigned long saved_time = 0;

Preferences preferences;

const char* ntpServer = "asia.pool.ntp.org";
const long  gmtOffset_sec = 18000;
const int   daylightOffset_sec = 3600;
String log_details = "";
bool log_event = false;
unsigned long rtc_minutes = 0;
unsigned long from_minutes = 0;
unsigned long to_minutes = 0;
long timer_start_set[2] = {0xFFFF, 0xFFFF};
long timer_stop_set[2] = {0xFFFF, 0xFFFF};
int sh, sm, eh, em;
int rtc_h, rtc_m;

BLYNK_WRITE(V0)
{
  solenoid_open_time = param.asLong() * 1000;
  Serial.println(solenoid_open_time);
  preferences.begin("time", false);
  preferences.putULong("s_time", solenoid_open_time);
  Serial.println("Config Saved using Preferences");
  preferences.end();
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
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);


  pinMode(solenoid_pin, OUTPUT);
  pinMode(soil_moisture_pin, INPUT);
  preferences.begin("time", false);
  saved_time = preferences.getULong("s_time", 0);
  Serial.println(saved_time);

  if (saved_time == 0)
  {
    Serial.println(F("no value stored in prefrences so setting default value for solenoid opening!"));
    solenoid_open_time = 5000;
    preferences.begin("time", false);
    preferences.putULong("s_time", solenoid_open_time);
    Serial.println("Config Saved using Preferences");
    preferences.end();
  }

  else
  {
    solenoid_open_time = saved_time;
    Serial.println(solenoid_open_time);
  }



  Serial.println("reading vlaues");
  for (int i = 0; i < 10; i++)
  {
    soilMoistureValue = analogRead(soil_moisture_pin);
    // Serial.println(soilMoistureValue);
    int new_val = map(soilMoistureValue, 0, 4093, 0, 570);
    average_val += new_val;
  }
  average = (average_val / 10);
  Serial.println(average);
  average_val = 0;
  sensor_code();
  if (very_wet == true)
  {
    very_wet = false;
  }

  if (dry == true)
  {
    if (rtc_minutes >= from_minutes && rtc_minutes <= to_minutes)
    {
      Serial.println("in operating time zone");
      dry = false;
      digitalWrite(solenoid_pin, HIGH);
      printLocalTime();
      Blynk.logEvent("time", "Solenoid turned on for " + String(solenoid_open_time / 1000) + " Seconds");
      delay(solenoid_open_time);
      printLocalTime();
      Blynk.logEvent("time", "Solenoid turned off ");// + log_details);
      digitalWrite(solenoid_pin, LOW);
    }
  }
  time_to_read_val = millis();
}

void loop()
{
  Blynk.run();

  if (millis() - time_to_read_val > 10000)//thirty seconds for testing 600000
  {
    hour_count++;
    Serial.println("reading vlaues");
    time_to_read_val = millis();
    for (int i = 0; i < 10; i++)
    {
      soilMoistureValue = analogRead(soil_moisture_pin);
      int new_val = map(soilMoistureValue, 0, 4093, 0, 570);
      average_val += new_val;
    }
    average = (average_val / 10);
    Serial.println(average);
    average_val = 0;
    sensor_code();

    if (very_wet == true)
    {
      very_wet = false;
      wet_counter++;
    }

    if (dry == true)
    {
      open_valve = true;// open water valve
      dry = false;
      dry_counter++;
    }
  }

  if (rtc_minutes >= from_minutes && rtc_minutes <= to_minutes)
  {
    Serial.println("in operating time zone");
    if (open_valve == true)
    {
      open_valve = false;
      digitalWrite(solenoid_pin, HIGH);
      log_event = true;
      printLocalTime();
      Blynk.logEvent("time", "Solenoid turned on for " + String(solenoid_open_time / 1000) + " Seconds"); //+ log_details);
      solenoid_time = millis();
    }
  }

  if (millis() - solenoid_time > solenoid_open_time)
  {
    solenoid_time = millis();
    digitalWrite(solenoid_pin, LOW);
    if (log_event == true)
    {
      log_event = false;
      printLocalTime();
      Blynk.logEvent("time", "Solenoid turned off "); //+log_details);
    }
  }
}

void sensor_code()
{
  if (average > WaterValue && average < (WaterValue + intervals))
  {
    Serial.println("Very Wet");
    very_wet = true;
  }
  if (average > (WaterValue + intervals) && average < (AirValue - intervals))
  {
    Serial.println("Wet");
    wet = true;
  }
  if (average < AirValue && average > (AirValue - intervals))
  {
    Serial.println("Dry");
    dry = true;
  }

}

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  char timeHour[3];
  char timemin[3];
  char timesec[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  strftime(timemin, 3, "%M", &timeinfo);
  strftime(timesec, 3, "%S", &timeinfo);
  char Day[10];
  char Month[10];
  char Date[10];
  char Year[10];
  strftime(Day, 10, "%A", &timeinfo);
  strftime(Month, 10, "%B", &timeinfo);
  strftime(Date, 10, "%D", &timeinfo);
  strftime(Year, 10, "%Y", &timeinfo);
  log_details = String(Day) + "-" + String(Month) + "-" +  String(Date) + "-" + String(Year) + "  " + String(timeHour) + ":" + String(timemin) + ":" + String(timesec);
  String hours_ = String(timeHour);
  String min_ = String(timemin);
  rtc_h = hours_.toInt();
  rtc_m = min_.toInt();
  rtc_minutes = (rtc_h * 60) + rtc_m;
  // Serial.println(log_details);

}
