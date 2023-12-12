#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPLv7o0c7rF"
#define BLYNK_TEMPLATE_NAME "Water Heater"
#define BLYNK_AUTH_TOKEN "ZusInk3TdUIRo6HJypot3Uuo7RaUK2XA"



#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

const char ssid[] = "OCTATHORN";  // change with your wifi ssid
const char pass[] = "62543800"; // change with your wifi pass





// Data wire is plugged into port 25 on the esp32
#define ONE_WIRE_BUS 25
#define WATER_PUMP_PIN 23
#define HEATER_PIN 22


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
BlynkTimer timer;

int SWITCH_STATE = 0;
float WATER_TEMP = 0;
float OUTSIDE_TEMP = 0;

unsigned long time_run_macro1, pump_millis, heater_millis, low_temp_p_millis, low_temp_h_millis, hp_millis = 0;
bool low_temp_hf, low_temp_pf, disable_macro_1_2, wt_temp, mil = false;
bool hp_once = true;
bool macro7_8 = false;


BLYNK_WRITE(V0)
{
  SWITCH_STATE = param.asInt();
  Serial.println(SWITCH_STATE);
}


void setup(void)
{
  Serial.begin(115200);
  pinMode(WATER_PUMP_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);

  sensors.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, temperature_code);

  if (SWITCH_STATE == 1)
  {

    if (WATER_TEMP > 103)
    {
      pump_millis = millis();
      heater_millis = millis();
      macro7_8 = false;
      hp_millis = millis();
    }

    if (WATER_TEMP < 101)
    {

      Serial.println(F("water temp < 101"));
      digitalWrite(WATER_PUMP_PIN, HIGH);
      digitalWrite(HEATER_PIN, HIGH);
      pump_millis = millis();
      heater_millis = millis();
    }





    if ( macro7_8 == false) //every 5 mintues check
    {
      if (OUTSIDE_TEMP < 34)
      {
        pump_millis = millis();
        digitalWrite(WATER_PUMP_PIN, HIGH);
      }
      time_run_macro1 = millis();
    }

  }


  else
  {
    Serial.println(F("swtich 0"));
    if (WATER_TEMP > 40)
    {
      Serial.println(F("water temp > 40"));
      disable_macro_1_2 = false;
      digitalWrite(HEATER_PIN, LOW);
      digitalWrite(WATER_PUMP_PIN, LOW);
      wt_temp = false;
      pump_millis = millis();
      heater_millis = millis();
    }

    if (WATER_TEMP < 40)
    {
      wt_temp = true;
      time_run_macro1 = millis();
      disable_macro_1_2 = true;
      Serial.println(F("water temp < 40"));
      digitalWrite(WATER_PUMP_PIN, HIGH);
      digitalWrite(HEATER_PIN, HIGH);
      pump_millis = millis();
      heater_millis = millis();
    }

    if  (disable_macro_1_2 == false) //every 5 mintues check
    {
      if (OUTSIDE_TEMP < 34)
      {
        Serial.println(F("outside temp less than 34 so relay on !"));
        pump_millis = millis();
        digitalWrite(WATER_PUMP_PIN, HIGH);
      }
      time_run_macro1 = millis();
    }
  }

}

void loop()
{
//  if (Serial.available())
//  {
//    WATER_TEMP  = Serial.parseFloat();
//    Serial.println(WATER_TEMP);
//  }

  Blynk.run();
  timer.run();
 

  if (SWITCH_STATE == 1)
  {
    Heater_program();
  }
  else
  {
    Maintain_program();
  }



}


void Maintain_program()
{
  if (WATER_TEMP > 40)
  {
    Serial.println(F("water temp > 40"));
    disable_macro_1_2 = false;
    digitalWrite(HEATER_PIN, HIGH);
    digitalWrite(WATER_PUMP_PIN, HIGH);
    wt_temp = false;
    pump_millis = millis();
    heater_millis = millis();
    mil = true;
  }
  else
  {
    disable_macro_1_2 = true;
    if (mil == true)
    {
      Serial.println(F("started cto check heater"));
      mil = false;
      hp_millis = millis();
    }
  }

  if ((millis() - time_run_macro1 > 300000 && disable_macro_1_2 == false)) //every 5 mintues check // for now 30 seconds check
  {
    Serial.println("checking if ot temp greater less than 34");
    delay(2000);
    if (OUTSIDE_TEMP < 34)
    {
      pump_millis = millis();
      digitalWrite(WATER_PUMP_PIN, LOW);
    }
    time_run_macro1 = millis();
  }


  if (millis() - pump_millis > 30000 && disable_macro_1_2 == false ) //30000=30 seconds for now 10 sec for pump
  {
    pump_millis = millis();
    digitalWrite(WATER_PUMP_PIN, HIGH);
  }


  if (millis() - hp_millis > 150000) // 25 min 40sec
  {
    Serial.println(F("checking after 25 min if water less than 40"));
    hp_millis = millis();
    wt_temp = true;

    if (WATER_TEMP < 40)
    {
      Serial.println(F("water less than 40 in 25 min cehck"));
      time_run_macro1 = millis();
      disable_macro_1_2 = true;
      Serial.println(F("water temp < 40"));
      digitalWrite(WATER_PUMP_PIN, LOW);
      digitalWrite(HEATER_PIN, LOW);
      pump_millis = millis();
      heater_millis = millis();
    }
  }


  if (millis() - pump_millis > 1140000 && wt_temp == true) //1140000=19 minutes
  {

    Serial.println(F("pump on as water temp less than 40"));
    pump_millis = millis();

    if (WATER_TEMP < 40)
    {
      digitalWrite(WATER_PUMP_PIN, HIGH);
      low_temp_pf = true;
      low_temp_p_millis = millis();
    }
    else
    {
      wt_temp = false;
      digitalWrite(WATER_PUMP_PIN, HIGH);
      low_temp_pf = false;
      hp_millis = millis();
    }
  }


  if (millis() - heater_millis > 1200000 && wt_temp == true) //1200000=20 minutes
  {
    Serial.println(F("heater on as water temp less than 40"));
    heater_millis = millis();

    if (WATER_TEMP < 40)
    {
      digitalWrite(HEATER_PIN, HIGH);
      low_temp_hf = true;
      low_temp_h_millis = millis();
    }
    else
    {
      wt_temp = false;
      digitalWrite(HEATER_PIN, HIGH);
      low_temp_hf = false;
      hp_millis = millis();
    }
  }


  if (low_temp_pf == true && millis() - low_temp_p_millis > 60000)
  {
    Serial.println(F("low_temp_pf true"));
    Serial.println(F("turn pump of for 1 min and trun on bakc agian!"));
    low_temp_pf = false;
    low_temp_p_millis = millis();
    pump_millis = millis();
    digitalWrite(WATER_PUMP_PIN, LOW);
  }

  if (low_temp_hf == true && millis() - low_temp_h_millis > 60000)
  {
    Serial.println(F("low_temp_hf true"));
    Serial.println(F("turn pump of for 1 min and trun on bakc agian!"));
    low_temp_hf = false;
    low_temp_h_millis = millis();
    heater_millis = millis();
    digitalWrite(HEATER_PIN, LOW);
  }

}



void Heater_program()
{

  if (WATER_TEMP > 101)
  {
    Serial.println(F("water temp > 101"));
    disable_macro_1_2 = false;
    digitalWrite(HEATER_PIN, LOW);
    digitalWrite(WATER_PUMP_PIN, LOW);
    wt_temp = false;
    pump_millis = millis();
    heater_millis = millis();
    mil = true;
  }
  else
  {
    disable_macro_1_2 = true;
    if (mil == true)
    {
      Serial.println(F("started cto check heater"));
      mil = false;
      hp_millis = millis();
    }
  }

  if ((millis() - time_run_macro1 > 300000 && disable_macro_1_2 == false)) //every 5 mintues check // for now 30 seconds check
  {
    Serial.println("checking if ot temp greater less than 34");
    delay(2000);
    if (OUTSIDE_TEMP < 34)
    {
      pump_millis = millis();
      digitalWrite(WATER_PUMP_PIN, LOW);
    }
    time_run_macro1 = millis();
  }


  if (millis() - pump_millis > 30000 && disable_macro_1_2 == false ) //30000=30 seconds for now 10 sec for pump
  {
    pump_millis = millis();
    digitalWrite(WATER_PUMP_PIN, HIGH);
  }


  if (millis() - hp_millis > 40000) // 25 min= 150000
  {
    Serial.println(F("checking after 25 min if water less than 101"));
    hp_millis = millis();
    wt_temp = true;

    if (WATER_TEMP < 101)
    {
      Serial.println(F("water less than 101 in 25 min cehck"));
      time_run_macro1 = millis();
      disable_macro_1_2 = true;
      Serial.println(F("water temp < 101"));
      digitalWrite(WATER_PUMP_PIN, HIGH);
      digitalWrite(HEATER_PIN, HIGH);
      pump_millis = millis();
      heater_millis = millis();
    }
  }


  if (millis() - pump_millis > 15000 && wt_temp == true) //1140000=19 minutes
  {

    Serial.println(F("pump on as water temp less than 101"));
    pump_millis = millis();

    if (WATER_TEMP < 101)
    {
      digitalWrite(WATER_PUMP_PIN, LOW);
      low_temp_pf = true;
      low_temp_p_millis = millis();
    }
    else
    {
      wt_temp = false;
      digitalWrite(WATER_PUMP_PIN, LOW);
      low_temp_pf = false;
      hp_millis = millis();
    }
  }


  if (millis() - heater_millis > 15000 && wt_temp == true) //1200000=20 minutes
  {
    Serial.println(F("heater on as water temp less than 101"));
    heater_millis = millis();

    if (WATER_TEMP < 101)
    {
      digitalWrite(HEATER_PIN, LOW);
      low_temp_hf = true;
      low_temp_h_millis = millis();
    }
    else
    {
      wt_temp = false;
      digitalWrite(HEATER_PIN, LOW);
      low_temp_hf = false;
      hp_millis = millis();
    }
  }


  if (low_temp_pf == true && millis() - low_temp_p_millis > 2000)
  {
    Serial.println(F("low_temp_pf true"));
    Serial.println(F("turn pump of for 1 min and trun on bakc agian!"));
    low_temp_pf = false;
    low_temp_p_millis = millis();
    pump_millis = millis();
    digitalWrite(WATER_PUMP_PIN, HIGH);
  }

  if (low_temp_hf == true && millis() - low_temp_h_millis > 2000)
  {
    Serial.println(F("low_temp_hf true"));
    Serial.println(F("turn pump of for 1 min and trun on bakc agian!"));
    low_temp_hf = false;
    low_temp_h_millis = millis();
    heater_millis = millis();
    digitalWrite(HEATER_PIN, HIGH);
  }

}


void temperature_code()
{
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  float WATER_TEMP = sensors.getTempFByIndex(0);
  float OUTSIDE_TEMP = sensors.getTempFByIndex(1);
  // Check if reading was successful
  if (WATER_TEMP != DEVICE_DISCONNECTED_F)
  {
    Serial.print("Temperature for water: ");
    Serial.println(WATER_TEMP);
    Blynk.virtualWrite(V1, String(WATER_TEMP) + " F");

  }
  else
  {
    Serial.println("Error: Could not read temperature data");
  }

  if (OUTSIDE_TEMP != DEVICE_DISCONNECTED_F)
  {
    Serial.print("Temperature for outside: ");
    Serial.println(OUTSIDE_TEMP);
    Blynk.virtualWrite(V2, String(OUTSIDE_TEMP) + " F");
  }
  else
  {
    Serial.println("Error: Could not read temperature data");
  }

}
