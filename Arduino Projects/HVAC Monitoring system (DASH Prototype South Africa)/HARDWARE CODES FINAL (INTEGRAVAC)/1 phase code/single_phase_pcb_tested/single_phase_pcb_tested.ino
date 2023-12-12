#include <WiFiEspAT.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Filters.h>
#include<avr/wdt.h>

// Emulate Serial1 on pins 6/7 if not present for esp01
#if defined(ARDUINO_ARCH_AVR) && !defined(HAVE_HWSERIAL2)
#include <SoftwareSerial.h>
SoftwareSerial Serial1(6, 7); // RX, TX
#define AT_BAUD_RATE 9600
#else
#define AT_BAUD_RATE 115200
#endif

// ads setups
Adafruit_ADS1115 ads;
Adafruit_ADS1115 ads2;



//dht 11 pins
#define DHTPIN 6
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Current sensor variables//
float currentRMS, currentRMS2, currentRMS3 = 0;
const float FACTOR = 50;
//const float multiplier = 0.00008F;
const float multiplier = 0.0005;

//Voltage sensor variables//

float U1, U2, U3, volt_sig1, volt_sig2 = 0;


//Pressure sensor variables//
const int pressureInput1 = A5; //select the analog input pin for the pressure transducer
const float pressureZero1 = 102.4; //analog reading of pressure transducer at 0psi
const float pressureMax1 = 921.6; //analog reading of pressure transducer at 100psi
const int pressuretransducermaxPSI1 = 500; //psi value of transducer being used
const int sensorreadDelay = 250; //constant integer to set the sensor read delay in milliseconds
float pressureValue1 = 0;

//Pressure sensor 2 variables//
const int pressureInput2 = A6; //select the analog input pin for the pressure transducer
const float pressureZero2 = 102.4; //analog reading of pressure transducer at 0psi
const float pressureMax2 = 921.6; //analog reading of pressure transducer at 100psi
const int pressuretransducermaxPSI2 = 500; //psi value of transducer being used
const int sensorreadDelay2 = 250; //constant integer to set the sensor read delay in milliseconds
float pressureValue2 = 0;

//temp sensors variables//
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);


float  s1_t = 0;
float  s2_t = 0;
float  s3_t = 0;
float  s4_t = 0;



//Relay pins//
const int relay1_pin = 4;
const int relay2_pin = 5;

//Status Led//

const int wifi_led = 9;
const int status_led = 10;
const int led_3 = 11;
const int led_4 = 12;


//Wifi credentials//

char ssid[] = "OCTATHORN";            // your network SSID (name)
char pass[] = "62543800";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

//Server to conncet to//
char server[] = "www.integravac.co.za";

//Initialize the wifi client object
WiFiClient client;


// sensors up and low limits deifned here
float currentDraw1_Ulimit, currentDraw1_Llimit, currentDraw2_Ulimit, currentDraw2_Llimit,
      currentDraw3_Ulimit, currentDraw3_Llimit, ac_Upvol_lim1, ac_Lovol_lim1, ac_Upvol_lim2, ac_Lovol_lim2,
      ac_Upvol_lim3, ac_Lovol_lim3, hum_up_lim, hum_low_lim, p_val_up_lim, p_val_low_lim, p_val_up_lim2, p_val_low_lim2
      , t1_up_lim, t1_low_lim, t2_up_lim, t2_low_lim, t3_up_lim, t3_low_lim, t4_up_lim, t4_low_lim, h = 0;

int relay1, relay2, simple_packet = 0;
int a = 0;

String buff;
int alertCount = 0;
bool req = false;
bool GET = true;
String deforst, pumpdown = "false";

//device ID//
String d_id = "18"; // you can change here only
int cc = 1;
String uid, mail = "";
int http = 1;

unsigned long p_time, get_time, sensor_time = 0;





void setup()
{

  Serial.begin(115200);
  Serial2.begin(115200);

  pinMode(relay1_pin, OUTPUT);
  pinMode(relay2_pin, OUTPUT);
  digitalWrite(relay1_pin, HIGH);
  digitalWrite(relay2_pin, HIGH);

  sensors.begin();
  ads.setGain(GAIN_TWO);      // +/- 1.024V 1bit = 0.5mV
  if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }

  dht.begin();
  wifi_init();

  sensors_code();

  Serial.println("Done!!");


}


void loop()
{



  if (GET == true)
  {
    GET = false;
    req = false;
    get_time = millis();
    httpRequest();

    simple_packet = 1;
  }



  if (req == true)
  {
    p_time = millis();
    if (client.connect(server, 80))
    {

      if (relay1 == 1 || relay2 == 1)
      {
        alertCount = 0;
      }

      if (volt_sig2 < 100)
      {
        pumpdown = "true";
        Serial.println("Connecting for post request for sig2!");
        String content = "{\"deviceId\":\"" + String(d_id) + "\",\"sensorOne\":\"" + String(currentRMS) + "\",\"sensorFive\":\"" + String(U1) + "\",\"sensorEight\":\"" + String(pressureValue1) + "\",\"sensorNine\":\"" + String(pressureValue2) + "\",\"sensorTen\":\"" + String(s1_t) + "\",\"sensorElven\":\"" + String(s2_t) + "\",\"sensorTwelve\":\"" + String(h) + "\",\"sensorThrteen\":\"" + String(pressureValue1) + "\",\"sensorFourteen\":\"" + String(pressureValue2) + "\",\"sensorFifteen\":\"" + String(s1_t) + "\",\"sensorSixteen\":\"" + String(s2_t) + "\",\"sensorSeventeen\":\"" + String(s3_t) +  "\",\"sensorEighteen\":\"" + String(s4_t) +  "\",\"alertCount\":\"" + String(alertCount) + "\",\"pumpDown\":\"" + String(pumpdown) + "\"}";
        client.println("POST /device/update/" + String(d_id) + " HTTP/1.1");
        client.println("Host: www.integravac.co.za");
        client.println("Accept: */*");
        client.println("Content-Length: " + String(content.length()));
        client.println("Content-Type: application/json");
        client.println();
        client.println(content);
        Serial.println("data sent");

        req = false;
        GET = true;
        simple_packet = 0;

      }

      else if (simple_packet == 1)
      {
        cc++;
        pumpdown = "false";
        deforst = "false";
        String content = "{\"deviceId\":\"" + String(d_id) + "\",\"sensorOne\":\"" + String(currentRMS) + "\",\"sensorFive\":\"" + String(U1) + "\",\"sensorEight\":\"" + String(pressureValue1) + "\",\"sensorNine\":\"" + String(pressureValue2) + "\",\"sensorTen\":\"" + String(s1_t) + "\",\"sensorElven\":\"" + String(s2_t) + "\",\"sensorTwelve\":\"" + String(h) + "\",\"sensorThrteen\":\"" + String(pressureValue1) + "\",\"sensorFourteen\":\"" + String(pressureValue2) + "\",\"sensorFifteen\":\"" + String(s1_t) + "\",\"sensorSixteen\":\"" + String(s2_t) + "\",\"sensorSeventeen\":\"" + String(s3_t) +  "\",\"sensorEighteen\":\"" + String(s4_t) +  "\",\"alertCount\":\"" + String(alertCount) + "\",\"pumpDown\":\"" + String(pumpdown) + "\",\"deforst\":\"" + String(deforst) + "\"}";
        client.println("POST /device/update/" + String(d_id) + " HTTP/1.1");
        client.println("Host: www.integravac.co.za");
        client.println("Accept: */*");
        client.println("Content-Length: " + String(content.length()));
        client.println("Content-Type: application/json");
        client.println();
        client.println(content);
        Serial.println("data sent");
        req = false;
        GET = true;
        simple_packet = 0;
        //
      }

    }

    else
    {
      Serial.println("failed");
      req = false;
      GET = true;
    }

  }





  buff = "";
  int flag = 0;
  while (client.available())
  {

    char rcv = client.read();
    // Serial.write(rcv);
    if (rcv == '{')
    {
      flag = 1;
    }

    if (rcv == '}')
    {
      flag = 2;
    }

    if (flag >= 1 && flag <= 2)
    {
      buff += rcv;
      if (flag == 2)
        flag = 3;
    }
  }

  if (flag == 3)
  {
    flag = 0;
    StaticJsonDocument <2048> doc;
    // Serial.print("data passed to deserializer:  ");

    DeserializationError error = deserializeJson(doc, buff);
    if (error)
    {

      // Serial.print(F("deserializeJson() failed: "));
      //Serial.println(error.f_str());
      return;
    }
    else
    {
      // Serial.println(buff);
      //  int device_id = doc["acCurrentSensorOneMin"];
      currentDraw1_Ulimit = float( doc["acCurrentSensorOneMax"]);
      //Serial.println( currentDraw1_Ulimit);
      currentDraw1_Llimit =  float(doc["acCurrentSensorOneMin"]);
      // Serial.println( currentDraw1_Llimit);
      currentDraw2_Ulimit =  float(doc["acCurrentSensorTwoMax"]);
      // Serial.println( currentDraw2_Ulimit);
      currentDraw2_Llimit =  float(doc["acCurrentSensorTwoMin"]);
      // Serial.println( currentDraw2_Llimit);
      currentDraw3_Ulimit =  float(doc["acCurrentSensorThreeMax"]);
      // Serial.println( currentDraw3_Ulimit);
      currentDraw3_Llimit =  float(doc["acCurrentSensorThreeMin"]);
      // Serial.println( currentDraw3_Llimit);
      ac_Upvol_lim1 =  float(doc["acVoltageSensorOneMax"]);
      // Serial.println(ac_Upvol_lim1);
      ac_Lovol_lim1 =  float(doc["acVoltageSensorOneMin"]);
      // Serial.println(ac_Lovol_lim1);
      ac_Upvol_lim2 =  float(doc["acVoltageSensorTwoMax"]);
      // Serial.println(ac_Upvol_lim2);
      ac_Lovol_lim2 =  float(doc["acVoltageSensorTwoMin"]);
      // Serial.println(ac_Lovol_lim2);
      ac_Upvol_lim3 =  float(doc["acVoltageSensorThreeMax"]);
      // Serial.println(ac_Upvol_lim3);
      ac_Lovol_lim3 =  float(doc["acVoltageSensorThreeMin"]);
      // Serial.println(ac_Lovol_lim3);
      hum_up_lim = float(doc["humiditySensorMax"]);
      //  Serial.println(hum_up_lim);
      hum_low_lim = float(doc["humiditySensorMin"]);
      //  Serial.println(hum_low_lim);
      p_val_up_lim = float(doc["pressureTransducerOneMax"]);
      // Serial.println(p_val_up_lim);
      p_val_low_lim = float(doc["pressureTransducerOneMin"]);
      // Serial.println(p_val_low_lim);
      p_val_up_lim2 = float (doc["pressureTransducerTwoMax"]);
      //  Serial.println(p_val_up_lim2);
      p_val_low_lim2 = float(doc["pressureTransducerTwoMin"]);
      // Serial.println(p_val_low_lim2);
      t1_up_lim = float(doc["tempSensorOneMax"]);
      // Serial.println(t1_up_lim);
      t1_low_lim = float(doc["tempSensorOneMin"]);
      // Serial.println(t1_low_lim);
      t2_up_lim = float(doc["tempSensorTwoMax"]);
      //Serial.println(t2_up_lim);
      t2_low_lim = float(doc["tempSensorTwoMin"]);
      //Serial.println(t2_low_lim);
      t3_up_lim = float(doc["tempSensorThreeMax"]);
      // Serial.println(t3_up_lim);
      t3_low_lim = float(doc["tempSensorThreeMin"]);
      // Serial.println(t3_low_lim);
      t4_up_lim = float(doc["tempSensorFourMax"]);
      // Serial.println(t4_up_lim);
      t4_low_lim = float(doc["tempSensorFourMin"]);
      // Serial.println(t4_low_lim);

      relay1 = int(doc["deforst"]);
      relay2 = int(doc["pumpDown"]);

      if (currentRMS >= currentDraw1_Ulimit || currentRMS <= currentDraw1_Llimit )
      {
        alertCount ++;
        // Serial.println("c1");
      }

      if (currentRMS2 >= currentDraw2_Ulimit || currentRMS2 <= currentDraw2_Llimit )
      {
        alertCount ++;
        // Serial.println("c2");
      }

      if (currentRMS3 >= currentDraw3_Ulimit || currentRMS3 <= currentDraw3_Llimit )
      {
        alertCount ++;
      }   // Serial.println("c3");

      if (U1 >= ac_Upvol_lim1 || U1 <= ac_Lovol_lim1)
      {
        alertCount ++;
        //Serial.println("v1");
      }

      if (U2 >= ac_Upvol_lim2 || U2 <= ac_Lovol_lim2)
      {
        alertCount ++;
        //Serial.println("v2");
      }

      if (U3 >= ac_Upvol_lim3 || U3 <= ac_Lovol_lim3)
      {
        alertCount ++;
        //Serial.println("v3");
      }

      if (h >= hum_up_lim || h <= hum_low_lim )
      {
        alertCount ++;
        // Serial.println("H");
      }

      if (pressureValue1 >= p_val_up_lim || pressureValue1 <= p_val_low_lim)
      {
        alertCount ++;
        // Serial.println("p1");
      }

      if (pressureValue2 >= p_val_up_lim2 || pressureValue2 <= p_val_low_lim2)
      {
        alertCount ++;
        // Serial.println("p2");

      }

      if (s1_t >= t1_up_lim || s1_t <= t1_low_lim)
      {
        alertCount ++;
        // Serial.println("t1");
      }

      if (s2_t >= t2_up_lim || s2_t <= t2_low_lim)
      {
        alertCount ++;
        // Serial.println("t2");
      }

      if (s3_t >= t3_up_lim || s3_t <= t3_low_lim)
      {
        alertCount ++;
        //Serial.println("t3");
      }

      if (s4_t >= t4_up_lim || s4_t <= t4_low_lim)
      {
        alertCount ++;
        //  Serial.println("t4");
      }

    }

  }

  if (relay1 == 0)
  {
    digitalWrite(relay1_pin, HIGH);
  }
  
  if (relay1 == 1)
  {
    digitalWrite(relay1_pin, LOW);
  }
  
  if (relay2 == 0)
  {
    digitalWrite(relay2_pin, HIGH);
  }
  
  if (relay2 == 1)
  {
    digitalWrite(relay2_pin, LOW);
  }



  if (buff != "")
  {

    sensors_code();


    sensor_time = 0;
    req = true;
  }

}



void printWifiStatus()
{
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void httpRequest()
{
  //Serial.println();

  client.stop();

  if (client.connect("www.integravac.co.za", 80))
  {
    // Serial.println("Connecting...");
    client.println("GET /settings/single/" + String(d_id) + " HTTP/1.1");
    client.println("Host: www.integravac.co.za");
    client.println("Connection: close");
    client.println();
  }

  else
  {
    // if you couldn't make a connection
    //Serial.println("Connection failed");
  }

}


void sensors_code()
{

  ac_v1();


  currentRMS = getcurrent1();

  h = dht.readHumidity();

  if (isnan(h))
  {
    h = 0;

  }
  //Serial.print(F("Humidity: "));
  //Serial.print(h);
  //Serial.println("");


  pressureValue1 = analogRead(pressureInput1);
  pressureValue1 = ((pressureValue1 - pressureZero1) * pressuretransducermaxPSI1) / (pressureMax1 - pressureZero1) - 10;
  //Serial.print(pressureValue1, 1);
  //Serial.println("psi");
  //delay(sensorreadDelay);

  pressureValue2 = analogRead(pressureInput2);
  pressureValue2 = ((pressureValue2 - pressureZero2) * pressuretransducermaxPSI2) / (pressureMax2 - pressureZero2) - 10;
  //Serial.print(pressureValue2, 1);
  // Serial.println("psi for second sensor");
  // delay(sensorreadDelay);



  //find the temperature here


  sensors.requestTemperatures();
  s1_t = sensors.getTempCByIndex(0);
  s2_t = sensors.getTempCByIndex(1);
  s3_t = sensors.getTempCByIndex(2);
  s4_t = sensors.getTempCByIndex(3);

  //  Serial.println(String(s1_t) + " C");
  //  Serial.println(String(s2_t) + " C");
  //  Serial.println(String(s3_t) + " C");
  //  Serial.println(String(s4_t) + " C");



}


void printMeasure(String prefix, float value, String postfix)
{
  //Serial.print(prefix);
  //Serial.print(value, 3);
  // Serial.println(postfix);
}


float getcurrent1()
{
  float voltage, voltage2, voltage3;
  float current, current2, current3;
  float sum, sum2, sum3 = 0;
  long time_check = millis();
  int counter = 0;

  while (millis() - time_check < 1000)
  {
    voltage = ads.readADC_Differential_0_1() * multiplier;
    current = voltage * FACTOR;
    sum += sq(current);
    counter = counter + 1;
  }

  current = sqrt(sum / counter);
  printMeasure("Irms: ", current, "A");


}


float ac_v1()
{
  int a = 0;
  float testFrequency = 50;
  float windowLength = 40.0 / testFrequency;

  int Sensor, Sensor2 = 0;

  float intercept = -4.2;
  float slope = 11.728123167155425;

  float intercept2 = 0;
  float slope2 = 0;

  float current_Volts, current_Volts2;
  unsigned long printPeriod = 1000;
  unsigned long previousMillis = 0;


  RunningStatistics inputStats;
  RunningStatistics inputStats2;
  RunningStatistics inputStats3;

  inputStats.setWindowSecs( windowLength );
  inputStats2.setWindowSecs( windowLength );
  inputStats3.setWindowSecs( windowLength );

  while ( true )
  {

    Sensor = analogRead(A1);
    Sensor2 = analogRead(A0);

    inputStats.input(Sensor);
    inputStats2.input(Sensor2);

    if ((unsigned long)(millis() - previousMillis) >= printPeriod)
    {
      previousMillis = millis();

      current_Volts = (intercept + slope) * inputStats.sigma();
      current_Volts2 = (intercept + slope) * inputStats2.sigma();

      if (a == 4)
      {
        Serial.println(current_Volts);
        Serial.println(current_Volts2);

        a = 0;
        break;
      }
      a++;
    }
  }
}


void wifi_init()
{
  WiFi.init(&Serial2);

  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  WiFi.disconnect();
  WiFi.setPersistent();
  WiFi.endAP();
  Serial.println();
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  int status = WiFi.begin(ssid, pass);

  if (status == WL_CONNECTED)
  {
    Serial.println();
    Serial.println("Connected to WiFi network.");
    printWifiStatus();
  }

  else
  {
    WiFi.disconnect();
    Serial.println();
    Serial.println("Connection to WiFi network failed.");
  }
}
