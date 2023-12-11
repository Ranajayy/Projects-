#include "Wire.h"
#include <MPU6050_light.h>
#include <Adafruit_BMP085.h>
#include <SPI.h>
#include <SD.h>
#include <TinyGPS++.h>

TinyGPSPlus gps;

#define record_button 9
#define led_pin A2
#define hall_1 7
#define hall_2 6
#define hall_3 5
#define hall_4 4


MPU6050 mpu(Wire);
Adafruit_BMP085 bmp;
File myFile;

// dc voltage //
#define dc_voltage_pin A0
float adc_voltage = 0.0;
float in_voltage = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
float ref_voltage = 3;
int adc_value = 0;


//dc current//
#define acs_pin A1
int sensitivity = 66; // use 100 for 20A Module and 66 for 30A Module
int adcvalue = 0;
float offsetvoltage = 2.5;
double Voltage = 0; //voltage measuring
double ecurrent = 0;// Current measuring

long timer = 0;
int bf = 0;


float revolutions, revolutions2, revolutions3, revolutions4 = 0;
unsigned long rpm, rpm2, rpm3, rpm4 = 0; 


int prev_state , state, prev_state2 , state2, prev_state3 , state3, prev_state4 , state4 = 0;
unsigned long prev, f_millis, d_millis = 0;
unsigned long I_time, E_time, Time = 0;

//air speed//
byte fetch_pressure(unsigned int *p_Pressure);
double VV;

String lati, longi, alti, speed_km , no_satelites  = "";

void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600);
  Wire.begin();

  pinMode(record_button, INPUT);
  pinMode(led_pin, OUTPUT);
  pinMode(hall_1, INPUT);
  pinMode(hall_2, INPUT);
  pinMode(hall_3, INPUT);
  pinMode(hall_4, INPUT);
  

  digitalWrite(led_pin, HIGH);
  delay(2000);
  digitalWrite(led_pin, LOW);
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while (status != 0)
  {

  } 

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }

  Serial.print("Initializing SD card...");
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    while (1);

  }
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets(true, true); // gyro and accelero
  Serial.println("Done!\n");

  digitalWrite(led_pin, HIGH);

  prev_state = digitalRead(hall_1);
  prev_state2 = digitalRead(hall_2);
  prev_state3 = digitalRead(hall_3);
  prev_state4 = digitalRead(hall_4);
  prev = millis();

}

void loop()
{
  if (digitalRead(record_button) == HIGH)
  {
    while (digitalRead(record_button) == 1);

    if (bf == 0)
    {
      bf = 1;
      tone(led_pin, 1);
    }
    else
    {
      bf = 0;
      noTone(led_pin);
      digitalWrite(led_pin, HIGH);
    }

  }

  RPM_measure();


  adc_value = analogRead(dc_voltage_pin);
  //Serial.println(adc_value);
  adc_voltage  = (adc_value * ref_voltage) / 1024.0;
  in_voltage = adc_voltage / (R2 / (R1 + R2)) ;


  float current_dc = analogRead(A1);
  current_dc = (current_dc * 3.3) / 1024;
  float  ecurrent = ((current_dc - offsetvoltage) / sensitivity);
  //Serial.println(ecurrent, 2);

  if (millis() - f_millis > 1000)
  {
    air_sensor();
    GPS();
    mpu.update();
    f_millis = millis();
    revolutions = 0;
    revolutions2 = 0;
    revolutions3 = 0;
    revolutions4 = 0;
    prev = millis();
  }

  if (bf == 1)
  {

    I_time = millis();


    float Ax, Ay, Az, Gz, Gy, Gx, roll, pitch, yaw, temperature, pressure, alti = 0;

    Ax = mpu.getAccX();
    Ay = mpu.getAccY();
    Az = mpu.getAccZ();
    Gx = mpu.getGyroX();
    Gy = mpu.getGyroY();
    Gz = mpu.getGyroZ();
    roll = mpu.getAngleX();
    pitch = mpu.getAngleY();
    yaw = mpu.getAngleZ();
    temperature = bmp.readTemperature();
    pressure = bmp.readPressure();
    alti = bmp.readAltitude();
    E_time = millis();
    Time = E_time - I_time;

    if (millis() - d_millis > 1000)
    {
      myFile = SD.open("DATA.txt", FILE_WRITE);
      if (myFile)
      {

        myFile.println(String(Time) + ";" + String(ecurrent) + ";" + String(in_voltage) + ";" + String(VV) + ";" + String(lati) + ";" + String(longi) + ";" + String(alti) + ";" + String(no_satelites) + ";" + String(speed_km) + ";" + String(Ax) + ";" + String(Ay) + ";" + String(Az) + ";" + String(Gx) + ";" + String(Gy) + ";" + String(Gz) + ";" + String(roll) + ";" + String(pitch) + ";" + String(yaw) + ";" + String(temperature) + ";" + String(pressure) + ";" + String(rpm) + ";" + String(rpm2) + ";" + String(rpm3) + ";" + String(rpm4));
        myFile.close();
        Serial.println("done.");
      }

      else
      {

        Serial.println("error opening test.txt");
      }
      d_millis = millis();

    }
    revolutions = 0;
    revolutions2 = 0;
    revolutions3 = 0;
    revolutions4 = 0;
    prev = millis();
  }



}

void RPM_measure()
{
  state = digitalRead(hall_1);


  if (state != prev_state)
  {
    if (state == 0)
    {
      revolutions++;
    }
    prev_state = state;
  }


  state2 = digitalRead(hall_2);

  if (state2 != prev_state2)
  {
    if (state2 == 0)
    {
      revolutions2++;
    }
    prev_state2 = state2;
  }

  state3 = digitalRead(hall_3);
  if (state3 != prev_state3)
  {
    if (state3 == 0)
    {
      revolutions3++;
    }
    prev_state3 = state3;
  }

  state4 = digitalRead(hall_4);

  if (state4 != prev_state4)
  {
    if (state4 == 0)
    {
      revolutions4++;
    }
    prev_state4 = state4;
  }


  if (millis() - prev >= 500)
  {
    rpm = revolutions * 120;
    rpm2 = revolutions2 * 120;
    rpm3 = revolutions3 * 120;
    rpm4 = revolutions4 * 120;
    Serial.println(rpm);
    revolutions = 0;
    revolutions2 = 0;
    revolutions3 = 0;
    revolutions4 = 0;
    prev = millis();
  }

}


void air_sensor()
{
  byte _status;
  unsigned int P_dat;
  unsigned int T_dat;
  double PR;
  double TR;
  double V;


  _status = fetch_pressure(&P_dat, &T_dat);



  PR = (double)((P_dat - 819.15) / (14744.7)) ;
  PR = (PR - 0.49060678) ;
  PR = abs(PR);
  V = ((PR * 13789.5144) / 1.225);
  VV = (sqrt((V)));


  TR = (double)((T_dat * 0.09770395701));
  TR = TR - 50;

//  Serial.print("speed m/s :");
//  Serial.println(VV, 5);

}

byte fetch_pressure(unsigned int *p_P_dat, unsigned int *p_T_dat)
{
  byte address, Press_H, Press_L, _status;
  unsigned int P_dat;
  unsigned int T_dat;

  address = 0x28;
  Wire.beginTransmission(address);
  Wire.endTransmission();
  delay(100);

  Wire.requestFrom((int)address, (int) 4);
  Press_H = Wire.read();
  Press_L = Wire.read();
  byte Temp_H = Wire.read();
  byte  Temp_L = Wire.read();
  Wire.endTransmission();


  _status = (Press_H >> 6) & 0x03;
  Press_H = Press_H & 0x3f;
  P_dat = (((unsigned int)Press_H) << 8) | Press_L;
  *p_P_dat = P_dat;

  Temp_L = (Temp_L >> 5);
  T_dat = (((unsigned int)Temp_H) << 3) | Temp_L;
  *p_T_dat = T_dat;
  return (_status);

}

void GPS()
{


  if (Serial1.available() > 0)
  {
    if (gps.encode(Serial1.read()))
    {
      if (gps.location.isValid())
      {
        lati = String(gps.location.lat());
        longi = String( gps.location.lng());

      }
      else
      {
        lati = "NAN";
        longi = "NAN";

      }
      if (gps.altitude.isValid())
      {
        alti = String(gps.altitude.meters());
      }
      else
      {
        alti = "NAN";

      }
      if (gps.satellites.isValid())
      {
        no_satelites = String( gps.satellites.value());
      }
      else
      {
        no_satelites = "NAN";

      }
      if (gps.speed.isValid())
      {
        speed_km = String( gps.speed.kmph());
      }

      else
      {
        speed_km = "NAN";

      }
    }
  }
}
