#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <VL53L0X.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

VL53L0X d_sensor;

#define ONE_WIRE_BUS 5 // pin no 5 on arduino for temperature sensor!

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature t_sensors(&oneWire);

float Celcius = 0;
const int solenoid_pin = 9;
const int dsub_pin1 = 1;
const int dsub_pin2 = 8;
const int water_pump = 6;
const int ink_pump = 7;
const int ldr_pin = A0;


void setup(void)
{
  Serial.begin(9600);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  pinMode(12, INPUT_PULLUP);// for distance sensor!
  pinMode(ldr_pin, INPUT);
  pinMode(solenoid_pin, OUTPUT);
  pinMode(dsub_pin1, OUTPUT);
  pinMode(dsub_pin2, OUTPUT);
  pinMode(water_pump, OUTPUT);
  pinMode(ink_pump, OUTPUT);

  digitalWrite(12, HIGH);
  digitalWrite(water_pump,HIGH);



  d_sensor.init();
  d_sensor.setTimeout(500);
  d_sensor.startContinuous();

  t_sensors.begin();
}

void loop(void)
{
  int ldr_val = analogRead(ldr_pin);
  int distance = d_sensor.readRangeContinuousMillimeters();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print("mm");
  if (d_sensor.timeoutOccurred())
  {
    Serial.print(" TIMEOUT");
  }

  Serial.println();
  delay(100);

  t_sensors.requestTemperatures();
  Celcius = t_sensors.getTempCByIndex(0);
  Serial.print(" C  ");
  Serial.print(Celcius);

  delay(100);

  /////// lcd display/////
  lcd.setCursor(0, 0);
  lcd.print("Temeperatue: ");
  lcd.print(Celcius);
  lcd.setCursor(0, 1);
  lcd.print("Distance: ");
  lcd.print(distance);
  lcd.setCursor(0, 2);
  lcd.print("Intensity: ");
  lcd.print(ldr_val);



  //////now code to control the heating element and pumps etc////////////////
  if (distance < 10)
  {
    digitalWrite(solenoid_pin, HIGH);// turn on solenoid
  }
  else
  {
    digitalWrite(solenoid_pin, LOW); //
  }


  if (Celcius < 60)
  {
    digitalWrite(dsub_pin1, LOW); // change LOW and HIGH of pins respectiveley to turn on/off heating element!
    digitalWrite(dsub_pin2, HIGH);
  }
  else
  {
    digitalWrite(dsub_pin1, LOW); // change LOW and HIGH of pins respectiveley to turn on/off heating element!
    digitalWrite(dsub_pin2, LOW);
  }


  if (ldr_val < 500)
  {
    digitalWrite(ink_pump, HIGH);
  }
  else
  {
    digitalWrite(ink_pump, LOW);
  }


}
