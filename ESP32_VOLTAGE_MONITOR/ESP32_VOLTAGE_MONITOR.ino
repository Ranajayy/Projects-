#include<WiFi.h>
#include<Preferences.h>
#include <Adafruit_ADS1X15.h>
#include <time.h>

Adafruit_ADS1115 ads;
Preferences preferences;
WiFiClient client;

const int PORT_NO = 4260;
String id = "";
String pass = "";
String ssid = "";
String ip = "";
String sleep_time = "";
String f_sleep_time = "";
String pc_data = "";
String data_packet = "";
String data_server = "";
int16_t adc0;
float volts0;
float battery = 0;
bool sett = false;
int ft = 0;
bool wake = false;

#define uS_TO_S_FACTOR 1000000ULL

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 5 * 3600;
const int daylightOffset_sec = 0;
char currentTimeStr[20];





void setup()
{
  Serial.begin(115200);
  pinMode(2, OUTPUT);


  if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
  get_votls();
  print_wakeup_reason();
  _init();





}


void loop()
{
  get_votls();

  if (volts0 <= 0 || sett == false)
  {
    get_settings_serial();
  }

  else
  {
    send_data_sleep();
  }

}



void _init()
{
  preferences.begin("credentials", false);
  ssid = preferences.getString("ssid", "");
  pass = preferences.getString("password", "");
  ip = preferences.getString("ip", "");
  id = preferences.getString("id", "");
  sleep_time = preferences.getString("time", "");
  f_sleep_time = preferences.getString("ftime", "");

  if (ssid == "" || pass == "" || ip == "" || id == "" || sleep_time == "" || f_sleep_time == "")
  {
    Serial.println("no settings saved!");
    digitalWrite(2, HIGH);
    sett = false;

  }

  else
  {

    sett = true;
    Serial.println("SSID: " + String(ssid));
    Serial.println("Pass: " + String(pass));
    Serial.println("ip: " + String(ip));
    Serial.println(ip.length());

    WiFi.begin(ssid.c_str(), pass.c_str());

    int count = 0;

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      //Serial.println(".");
      count++;
      if (count == 20 || Serial.available() > 0)
      {
        break;
      }
    }

    if (WL_CONNECTED)
    {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

      if (wake == true && volts0 != 0)
      {
        Serial.println("power reset!");

        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
          Serial.println("Failed to obtain time");
          return;
        }

        // Extract the current hour and minute
        long currentMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;
        long total_min = atol(f_sleep_time.c_str());
        Serial.println(currentMinutes);
        Serial.println(total_min);

        if (currentMinutes < total_min)
        {
          long diff = total_min - currentMinutes;
          diff = diff * 60;
          esp_sleep_enable_timer_wakeup(diff * uS_TO_S_FACTOR);
          Serial.println("sleeping until time is reached");
          esp_deep_sleep_start();
        }

        else if (total_min < currentMinutes)
        {
          long diff = 1440 - currentMinutes;
          diff = diff + total_min;
          diff = diff * 60;
          esp_sleep_enable_timer_wakeup(diff * uS_TO_S_FACTOR);
          Serial.println("sleeping until time is reached T");
          esp_deep_sleep_start();
        }


      }
      else
      {
        Serial.println("Sleep time is :" + sleep_time);
        esp_sleep_enable_timer_wakeup(sleep_time.toInt() * uS_TO_S_FACTOR);
      }

    }

    else
    {
      Serial.println("wifi connection failed");
    }

  }
}


void get_settings_serial()
{
  if (Serial.available() > 0)
  {
    pc_data = "";
    pc_data = Serial.readString();
    Serial.println(pc_data);
    Serial.println("PC Data Length: " + String(pc_data.length()));

    if (pc_data[0] == '@')
    {
      //Serial.println("data is: " + data_packet);
      preferences.begin("credentials", false);
      ssid = preferences.getString("ssid", "");
      pass = preferences.getString("password", "");
      id = preferences.getString("id", "");
      ip = preferences.getString("ip", "");
      sleep_time = preferences.getString("time", "");

      if (ssid == "" || pass == "" || id == "" || ip == "" || sleep_time == "")
      {
        //Serial.println("No values saved for ssid or password or id or ip or time");
        data_packet = "@,0,0,0,0,0,\n";
        Serial.println(data_packet);
      }

      else
      {
        data_packet = "@," + id + "," + pass + "," + ssid + "," + ip + "," + sleep_time + "," + f_sleep_time + ",\n";
        Serial.println(data_packet);
      }
    }

    else
    {
      WiFi.disconnect();
      id = getValue(pc_data, ',', 0);
      Serial.println(id);
      pass = getValue(pc_data, ',', 1);
      Serial.println(pass);
      ssid = getValue(pc_data, ',', 2);
      Serial.println(ssid);
      ip = getValue(pc_data, ',', 3);
      Serial.println(ip);
      sleep_time = getValue(pc_data, ',', 4);
      f_sleep_time = getValue(pc_data, ',', 5);

      preferences.begin("credentials", false);
      preferences.putString("ssid", ssid);
      preferences.putString("password", pass);
      preferences.putString("id", id);
      preferences.putString("ip", ip);
      preferences.putString("time", sleep_time);
      preferences.putString("ftime", f_sleep_time);
      Serial.println("Network Credentials Saved using Preferences");
      preferences.end();

      Serial.println(data_packet);
      Serial.println("saved");
      sett = true;

    }
  }
}



void get_votls()
{
  adc0 = ads.readADC_SingleEnded(0);
  volts0 = ads.computeVolts(adc0);
  //Serial.println(volts0);
  battery = volts0 * (4.5795);
  // Serial.println(battery);
}






void send_data_sleep()
{

  if (client.connect(ip.c_str(), PORT_NO, 3000))
  {
    Serial.println("connected to server!");
    String a = "S," + String(id) + "," + String( battery) + '\n';
    const char* snd = a.c_str();
    client.write(snd);
    delay(500);
    client.stop();
    WiFi.disconnect();
    Serial.println("sleeping for sleep time");
    esp_deep_sleep_start();

  }

  else
  {
    Serial.println("server connection failed");
  }

}


String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}



void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); wake = true; Serial.println("Setting 'wake' to true"); break;
  }
}
