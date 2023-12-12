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
#include <avr/wdt.h>



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
//char ssid[] = "MAS iPhone";            // your network SSID (name)
//char pass[] = "vr46lh44lm10";        // your network password
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


String buff;
int alertCount = 0;
bool req = false;
bool GET = true;
String deforst, pumpdown = "";

//device ID//
String d_id = "17";
int cc = 1;
String uid, mail = "";
int a = 0;

void setup()
{

  Serial.begin(115200);
 // while (!Serial);

  Serial2.begin(115200);
  //  wdt_enable(WDTO_8S);
  pinMode(relay1_pin, OUTPUT);
  pinMode(relay2_pin, OUTPUT);
  digitalWrite(relay1_pin, HIGH);
  digitalWrite(relay2_pin, HIGH);


  sensors.begin();


  ads.setGain(GAIN_TWO);      // +/- 1.024V 1bit = 0.5mV
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
  ads2.setGain(GAIN_TWO);      // +/- 1.024V 1bit = 0.5mV
  if (!ads2.begin(0X49)) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
  dht.begin();


  WiFi.init(Serial2);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println();
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  wdt_reset();




  WiFi.disconnect(); // to clear the way. not persistent

  WiFi.setPersistent(); // set the following WiFi connection as persistent

  WiFi.endAP(); // to disable default automatic start of persistent AP at startup

  //  uncomment this lines for persistent static IP. set addresses valid for your network
  //  IPAddress ip(192, 168, 1, 9);
  //  IPAddress gw(192, 168, 1, 1);
  //  IPAddress nm(255, 255, 255, 0);
  //  WiFi.config(ip, gw, gw, nm);

  Serial.println();
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  //  use following lines if you want to connect with bssid
  //  const byte bssid[] = {0x8A, 0x2F, 0xC3, 0xE9, 0x25, 0xC0};
  //  int status = WiFi.begin(ssid, pass, bssid);

  int status = WiFi.begin(ssid, pass);

  if (status == WL_CONNECTED) {
    Serial.println();
    Serial.println("Connected to WiFi network.");
    printWifiStatus();
  } else {
    WiFi.disconnect(); // remove the WiFi connection
    Serial.println();
    Serial.println("Connection to WiFi network failed.");
  }

  Serial.println("Reading from sensors for initial readings!");
  sensors_code();
  Serial.println("Done!!");
  wdt_reset();



}


void loop()
{




  if (GET == true)
  {
    GET = false;
    req = false;
    httpRequest();
    wdt_reset();

    simple_packet = 1;
  }



  if (req == true)
  {
    if (client.connect(server, 80))
    {
      if (relay1 == 1 || relay2 == 1)
      {
        alertCount = 0;
      }


      if ( volt_sig1 > 2005 )
      {
        deforst = "true";
        Serial.println("Connecting for post request for Defrost!");
        String content = "{\"deviceId\":\"" + String(d_id) + "\",\"sensorOne\":\"" + String(currentRMS) + "\",\"sensorTwo\":\"" + String(currentRMS2) + "\",\"sensorThree\":\"" + String(currentRMS3) + "\",\"sensorFive\":\"" + String(U1) + "\",\"sensorSix\":\"" + String(U2) + "\",\"sensorSeven\":\"" + String(U3) + "\",\"sensorThrteen\":\"" + String(pressureValue1) + "\",\"sensorFourteen\":\"" + String(pressureValue2) + "\",\"sensorTwelve\":\"" + String(h) + "\",\"sensorFifteen\":\"" + String(s1_t) + "\",\"sensorSixteen\":\"" + String(s2_t) + "\",\"sensorSeventeen\":\"" + String(s3_t) +  "\",\"sensorEighteen\":\"" + String(s4_t) +  "\",\"alertCount\":\"" + String(alertCount) + "\",\"deforst\":\"" + String(deforst) + "\"}";
        // String content = "{\"deviceId\":\"" + String(d_id) + "\",\"sensorOne\":\"" + String(currentRMS) + "\",\"sensorFive\":\"" + String(U1) + "\",\"sensorTwelve\":\"" + String(h) + "\",\"sensorThrteen\":\"" + String(pressureValue1) + "\",\"sensorFourteen\":\"" + String(pressureValue2) + "\",\"sensorFifteen\":\"" + String(s1_t) + "\",\"sensorSixteen\":\"" + String(s2_t) + "\",\"sensorSeventeen\":\"" + String(s3_t) + "\",\"sensorEighteen\":\"" + String(s4_t) + "\",\"sensorNine\":\"" + String(d1) + "\",\"sensorTen\":\"" + String(d2) +  "\",\"alertCount\":\"" + String(alertCount) + "\",\"deforst\":\"" + String(deforst) + "\",\"pumpDown\":\"" + String(pumpdown) + "\"}";
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

      if (volt_sig2 > 1000)
      {
        pumpdown = "true";
        Serial.println("Connecting for post request for pumpDown!");
        String content = "{\"deviceId\":\"" + String(d_id) + "\",\"sensorOne\":\"" + String(currentRMS) + "\",\"sensorTwo\":\"" + String(currentRMS2) + "\",\"sensorThree\":\"" + String(currentRMS3) + "\",\"sensorFive\":\"" + String(U1) + "\",\"sensorSix\":\"" + String(U2) + "\",\"sensorSeven\":\"" + String(U3) + "\",\"sensorThrteen\":\"" + String(pressureValue1) + "\",\"sensorFourteen\":\"" + String(pressureValue2) + "\",\"sensorTwelve\":\"" + String(h) + "\",\"sensorFifteen\":\"" + String(s1_t) + "\",\"sensorSixteen\":\"" + String(s2_t) + "\",\"sensorSeventeen\":\"" + String(s3_t) +  "\",\"sensorEighteen\":\"" + String(s4_t) +  "\",\"alertCount\":\"" + String(alertCount) + "\",\"deforst\":\"" + String(deforst) + "\"}";
        // String content = "{\"deviceId\":\"" + String(d_id) + "\",\"sensorOne\":\"" + String(currentRMS) + "\",\"sensorFive\":\"" + String(U1) + "\",\"sensorEight\":\"" + String(pressureValue1) + "\",\"sensorNine\":\"" + String(pressureValue2) + "\",\"sensorTen\":\"" + String(s1_t) + "\",\"sensorElven\":\"" + String(s2_t) + "\",\"sensorTwelve\":\"" + String(h) + "\",\"sensorThrteen\":\"" + String(pressureValue1) + "\",\"sensorFourteen\":\"" + String(pressureValue2) + "\",\"sensorFifteen\":\"" + String(s1_t) + "\",\"sensorSixteen\":\"" + String(s2_t) + "\",\"sensorSeventeen\":\"" + String(s3_t) +  "\",\"sensorEighteen\":\"" + String(s4_t) +  "\",\"alertCount\":\"" + String(alertCount) + "\",\"pumpDown\":\"" + String(pumpdown) + "\",\"deforst\":\"" + String(deforst) + "\"}";
        client.println("POST /device/update/" + String(d_id) + " HTTP/1.1");
        client.println("Host: www.integravac.co.za");
        client.println("Accept: */*");
        client.println("Content-Length: " + String(content.length()));
        client.println("Content-Type: application/json");
        client.println();
        client.println(content);
        Serial.println("data sent");
        wdt_reset();

        req = false;
        GET = true;
        simple_packet = 0;
      }

      if (simple_packet == 1)
      {
        cc++;
        pumpdown = "false";
        deforst = "false";
        // Serial.println("Connecting for post request simple");
        String content = "{\"deviceId\":\"" + String(d_id) + "\",\"sensorOne\":\"" + String(currentRMS) + "\",\"sensorTwo\":\"" + String(currentRMS2) + "\",\"sensorThree\":\"" + String(currentRMS3) + "\",\"sensorFive\":\"" + String(U1) + "\",\"sensorSix\":\"" + String(U2) + "\",\"sensorSeven\":\"" + String(U3) + "\",\"sensorThrteen\":\"" + String(pressureValue1) + "\",\"sensorFourteen\":\"" + String(pressureValue2) + "\",\"sensorTwelve\":\"" + String(h) + "\",\"sensorFifteen\":\"" + String(s1_t) + "\",\"sensorSixteen\":\"" + String(s2_t) + "\",\"sensorSeventeen\":\"" + String(s3_t) +  "\",\"sensorEighteen\":\"" + String(s4_t) +  "\",\"alertCount\":\"" + String(alertCount) + "\",\"deforst\":\"" + String(deforst) + "\"}";
        //  String content = "{\"deviceId\":\"" + String(d_id) + "\",\"sensorOne\":\"" + String(cc) + "\",\"sensorTwo\":\"" + String(cc) + "\",\"sensorThree\":\"" + String(cc) + "\",\"sensorFive\":\"" + String(cc) + "\",\"sensorSix\":\"" + String(cc) + "\",\"sensorSeven\":\"" + String(cc) + "\",\"sensorTwelve\":\"" + String(cc) + "\",\"sensorThrteen\":\"" + String(cc) + "\",\"sensorFourteen\":\"" + String(cc) + "\",\"sensorFifteen\":\"" + String(cc) + "\",\"sensorSixteen\":\"" + String(cc) + "\",\"sensorSeventeen\":\"" + String(cc) + "\",\"sensorEighteen\":\"" + String(cc) + "\",\"alertCount\":\"" + String(cc) + "\",\"mail\":\"" + String(mail) + "\"}";
        client.println("POST /device/update/" + String(d_id) + " HTTP/1.1");
        client.println("Host: www.integravac.co.za");
        client.println("Accept: */*");
        client.println("Content-Length: " + String(content.length()));
        client.println("Content-Type: application/json");
        client.println();
        client.println(content);
        Serial.println("data sent");
        wdt_reset();
        req = false;
        GET = true;
        simple_packet = 0;
        wdt_reset();

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

      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
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

      uid = doc["userid"].as<String>();
      mail = doc["email"].as<String>();


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
    wdt_reset();

    req = true;
  }



}



void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void httpRequest()
{


  if (client.connect("www.integravac.co.za", 80))
  {
    Serial.println("Connecting...");
    client.println("GET /settings/single/" + String(d_id) + " HTTP/1.1");
    client.println("Host: www.integravac.co.za");
    client.println("Connection: close");
    client.println();
    client.flush();
  }
  else
  {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}


void sensors_code()
{


  ac_v1();
  //  Serial.println("U1: " + String(U1));
  //  Serial.println("U2: " + String(U2));
  currentRMS = getcurrent1();

  h = dht.readHumidity();

  if (isnan(h))
  {
    //  Serial.println(F("Failed to read from DHT sensor!"));
    h = 0;
    //    return;
  }
  //Serial.print(F("Humidity: "));
  //Serial.print(h);
  //Serial.println("");


  pressureValue1 = analogRead(pressureInput1);
  pressureValue1 = ((pressureValue1 - pressureZero1) * pressuretransducermaxPSI1) / (pressureMax1 - pressureZero1);
  //Serial.print(pressureValue1, 1);
  //Serial.println("psi");
  //delay(sensorreadDelay);

  pressureValue2 = analogRead(pressureInput2);
  pressureValue2 = ((pressureValue2 - pressureZero2) * pressuretransducermaxPSI2) / (pressureMax2 - pressureZero2);
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


  // Serial.println("out of sensor");
}


void printMeasure(String prefix, float value, String postfix)
{
  Serial.print(prefix);
  Serial.print(value, 3);
  Serial.println(postfix);
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
    voltage2 = ads.readADC_Differential_2_3() * multiplier;
    voltage3 = ads2.readADC_Differential_0_1() * multiplier;
    current = voltage * FACTOR;
    current2 = voltage2 * FACTOR;
    current3 = voltage3 * FACTOR;
    sum += sq(current);
    sum2 += sq(current2);
    sum3 += sq(current3);
    counter = counter + 1;
  }

  current = sqrt(sum / counter);
  current2 = sqrt(sum2 / counter);
  current3 = sqrt(sum3 / counter);
  currentRMS = current;
  currentRMS2 = current2;
  currentRMS3 = current3;
}

void ac_v1()
{
  float testFrequency = 50;                     // test signal frequency (Hz)
  float windowLength = 40.0 / testFrequency;   // how long to average the signal, for statistist

  int Sensor, Sensor2, Sensor3, Sensor4, Sensor5 = 0; //Sensor analog input, here it's A0
  float intercept = -0.04; // to be adjusted based on calibration testing
  float slope = 0.0405; // to be adjusted based on calibration testing
  float current_Volts, current_Volts2, current_Volts3, current_Volts4, current_Volts5; // Voltage
  unsigned long printPeriod = 1000; //Refresh rate
  unsigned long previousMillis = 0;

  RunningStatistics inputStats;       //Easy life lines, actual calculation of the RMS requires a load of coding
  RunningStatistics inputStats2;
  RunningStatistics inputStats3;
  RunningStatistics inputStats4;
  RunningStatistics inputStats5;
  inputStats.setWindowSecs( windowLength );
  inputStats2.setWindowSecs( windowLength );
  inputStats3.setWindowSecs( windowLength );
  inputStats4.setWindowSecs( windowLength );
  inputStats5.setWindowSecs( windowLength );

  while ( true )
  {

    Sensor = analogRead(A0);  // read the analog in value:
    Sensor2 = analogRead(A1);
    Sensor3 = analogRead(A2);
    Sensor4 = analogRead(A3);
    Sensor5 = analogRead(A4);
    inputStats.input(Sensor);  // log to Stats function
    inputStats2.input(Sensor2);  // log to Stats function
    inputStats3.input(Sensor3);  // log to Stats function
    inputStats4.input(Sensor4);  // log to Stats function
    inputStats5.input(Sensor5);  // log to Stats function
    if ((unsigned long)(millis() - previousMillis) > printPeriod)
    {
      previousMillis = millis();   // update time every second
      a++;
      current_Volts = intercept + slope * inputStats.sigma(); //Calibartions for offset and amplitude
      current_Volts = current_Volts * (40.3231);             //Further calibrations for the amplitude

      current_Volts2 = intercept + slope * inputStats2.sigma(); //Calibartions for offset and amplitude
      current_Volts2 = current_Volts2 * (40.3231);             //Further calibrations for the amplitude

      current_Volts3 = intercept + slope * inputStats3.sigma(); //Calibartions for offset and amplitude
      current_Volts3 = current_Volts3 * (40.3231);             //Further calibrations for the amplitude

      current_Volts4 = intercept + slope * inputStats4.sigma(); //Calibartions for offset and amplitude
      current_Volts4 = current_Volts4 * (40.3231);             //Further calibrations for the amplitude

      current_Volts5 = intercept + slope * inputStats5.sigma(); //Calibartions for offset and amplitude
      current_Volts5 = current_Volts5 * (40.3231);             //Further calibrations for the amplitude
      // Serial.println(a);

    }
    if (a == 3)
    {
      U1 = current_Volts;
      U2 = current_Volts2;
      U3 = current_Volts3;
      volt_sig1 = current_Volts4;
      volt_sig2 = current_Volts5;
      a = 0;
      break;
    }


  }
}
