#include "NewPing.h" // ultrasonic sensor
#include "DHT.h"   // dht11

///////////////////////////////////// blynk settings //////////////////////
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <SPI.h>
char auth[] = "   nHbW4mMzh6KQRPpkN-HwLZaf15g1sFp2";
char ssid[] = "Octathorn";
char pass[] = "62543800";
//BlynkTimer timer;

///////////////////////////////// light sensor /////////////////////
int temt6000Pin = 39;
float light;
int light_value;

///////////////////////////////// moisture sensor /////////////////////
const int sensor_pin = 36;
float moisture_percentage;
int sensor_analog;
int a;

///////////////////////////////// CO2 sensor /////////////////////
int gas_concentration;
// PWM interface
const int pwmPin = 4;


///////////////////////////////// Ultrasonic sensor for waterlevel/////////////////////
#define TRIGGER_PIN 32
#define ECHO_PIN 33
#define MAX_DISTANCE 80
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
float distance1;

///////////////////////////////// Ultrasonic sensor for plant distance /////////////////////
#define TRIGGER_PIN2 25
#define ECHO_PIN2 26
NewPing sonar2(TRIGGER_PIN2, ECHO_PIN2, MAX_DISTANCE);
float  distance2;

///////////////////////////////// limit Switch /////////////////////////////////////////
int Lswitch = 27;
int SW1 = 0;

#define DHTPIN 13     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22
DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  pinMode(temt6000Pin, INPUT); //data pin for ambientlight sensor
  pinMode(pwmPin, INPUT);
  pinMode(Lswitch, INPUT);  // limit switch
  dht.begin();  // dht11 begin
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  int wifi_ctr = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Blynk.begin(auth, ssid, pass);
}

void loop()
{
  Blynk.run();

  ///////////////////////////////// moisture sensor /////////////////////
  sensor_analog = analogRead(sensor_pin);  // moisture increases Analog value decreases
  moisture_percentage = ( 100 - ( (sensor_analog / 4096.00) * 100 ) );
  a = moisture_percentage;
  Serial.print("Moisture Percentage = ");
  Serial.print(moisture_percentage);
  Serial.println("%\n\n");
  Serial.println("");
  Blynk.virtualWrite(V4, a); //blynk

  ///////////////////////////////// light sensor /////////////////////
  Serial.println("");
  int light_value = analogRead(temt6000Pin);
  light = light_value * 0.02441; // percentage calculation
  Serial.print("light Percentage = ");
  Serial.print(light);
  Serial.println(" %");
  Serial.println("");
  Blynk.virtualWrite(V5, light); //blynk

  ///////////////////////////////// CO2 sensor /////////////////////
  int ppm_PWM = gas_concentration_PWM();
  Serial.print("CO2 in AIR in (PPM) : ");
  Serial.println(ppm_PWM);
  Serial.println("");
  Blynk.virtualWrite(V3, ppm_PWM); //blynk

  ///////////////////////////////// Ultrasonic sensor for waterlevel/////////////////////
  distance1 = sonar.ping_cm();

  if (distance1 >= 400)
  {
    Serial.println("tank empty ");
    Serial.println("");

  }
  if (distance1 <= 2)
  {
    Serial.println("tank full ");
    Serial.println("");

  }
  else
  {
    // Send results to Serial Monitor
    Serial.print("Distance from water level = ");
    Serial.print(distance1);
    Serial.println(" cm");
    Serial.println("");

  }

  ///////////////////////////////// Ultrasonic sensor for plant distance /////////////////////
  distance2 = sonar2.ping_cm();

  Serial.print("Distance from plant = ");
  Serial.print(distance2);
  Serial.println(" cm");
  Serial.println("");
  Blynk.virtualWrite(V6, distance2); //blynk

  ///////////////////////////////// limit Switch /////////////////////////////////////////
  SW1 = digitalRead(Lswitch);
  if ( SW1 == 0 )
  {
    Serial.print("limit switch : ");
    Serial.println("Switch open");
    Serial.println("");
  }
  if ( SW1 == 1 )
  {
    Serial.print("limit switch : ");
    Serial.println("Switch pressed");
    Serial.println("");
  }

  ///////////////////////////////// DHT 11  /////////////////////////////////////////
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Blynk.virtualWrite(V2, h); // blynk
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("C "));
  Blynk.virtualWrite(V1, t); // blynk
  Serial.print(f);
  Serial.print(F("F "));
  Serial.println("");
  delay(1000); // sleep for 1 minute
}


///////////////////////////////// CO2 sensor /////////////////////
int gas_concentration_PWM()
{
  while (digitalRead(pwmPin) == LOW) {};
  long t0 = millis();
  while (digitalRead(pwmPin) == HIGH) {};
  long t1 = millis();
  while (digitalRead(pwmPin) == LOW) {};
  long t2 = millis();
  long tH = t1 - t0;
  long tL = t2 - t1;
  long ppm = 5000L * (tH - 2) / (tH + tL - 4);
  while (digitalRead(pwmPin) == HIGH) {};
  return int(ppm);
}
