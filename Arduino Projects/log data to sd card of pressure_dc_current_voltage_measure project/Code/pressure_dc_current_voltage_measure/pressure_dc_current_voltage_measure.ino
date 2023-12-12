#include <Robojax_AllegroACS_Current_Sensor.h>
#include <LM35.h>
#include <SPI.h>
#include <SD.h>
#include <OneWire.h>

#define button_pin 8
#define solenoid_purge 7
#define dc_motor 9

unsigned long initial_milli = 0;
int bf = 0;
long sec = 0;
int flag = 0;

File myFile;
unsigned long sd_millis = 0;
int secon = 0;
const int VIN1 = A1; // define the Arduino pin A0 as voltage input (V in)
const int VIN2 = A2;
const float VCC   = 5;// supply voltage
const int MODEL = 2;
const int pressureInput1 = A4; //select the analog input pin for the pressure transducer
const float pressureZero1 = 102.4; //analog reading of pressure transducer at 0psi
const float pressureMax1 = 921.6; //analog reading of pressure transducer at 100psi
const int pressuretransducermaxPSI1 = 100; //psi value of transducer being used
#define ANALOG_IN_PIN A0
Robojax_AllegroACS_Current_Sensor c1(MODEL, VIN1);
Robojax_AllegroACS_Current_Sensor c2(MODEL, VIN2);
LM35 sensor(A3);
float current_1, current_2, voltage, temp1 = 0;
float vd_level = 0 ;
float vd_voltage = 0;
float battery_voltage = 0;
float adc_voltage = 0.0;
float in_voltage = 0.0;
float bar_pressure = 0;
// Floats for resistor values in divider (in ohms)
float R1 = 30000.0;
float R2 = 7500.0;

// Float for Reference Voltage
float ref_voltage = 5.0;
float cel = 0;
// Integer for ADC value
int adc_value = 0;
int val;
void setup()
{
  Serial.begin(9600);
  Serial.print("Initializing SD card...");
  pinMode(button_pin, INPUT);
  pinMode(solenoid_purge, OUTPUT);
  pinMode(dc_motor, OUTPUT);
  digitalWrite(solenoid_purge, HIGH);
  digitalWrite(dc_motor, LOW);



  if (!SD.begin(10))
  {
    Serial.println("initialization failed!");
    while (1);

  }
}
void loop()
{
  dc_voltage();
  button_funtion();
  current_1 = c1.getCurrentAverage(300) - 0.1;
  current_2 = c2.getCurrentAverage(300) -0.1;
  Serial.println("Current 1 is: " + String(current_1));
  Serial.println("Current 2 is: " + String(current_2));
  val = analogRead(A3);
  float mv = ( val / 1024.0) * 5000;
  cel = mv / 10;
  Serial.print("TEMPRATURE = ");
  Serial.print(cel);
  Serial.print("*C");
  Serial.println();

  float  pressureValue1 = analogRead(pressureInput1);
  pressureValue1 = ((pressureValue1 - pressureZero1) * pressuretransducermaxPSI1) / (pressureMax1 - pressureZero1);
  pressureValue1 = (pressureValue1 / 14.5038);
  bar_pressure = pressureValue1;
  if (bar_pressure < 1)
  {
    bar_pressure = 1;
  }
  if (bar_pressure > 6)
  {
    bar_pressure = 6;
  }
  Serial.println(bar_pressure);

  if (bar_pressure > 3)
  {
    digitalWrite(dc_motor, HIGH);
  }
  else
  {
    digitalWrite(dc_motor, LOW);
  }

  write_to_sd_card();
  delay(1000);
}

void dc_voltage()
{
  adc_value = analogRead(ANALOG_IN_PIN);

  // Determine voltage at ADC input
  adc_voltage  = (adc_value * ref_voltage) / 1024.0;

  // Calculate voltage at divider input
  in_voltage = adc_voltage / (R2 / (R1 + R2)) ;

  // Print results to Serial Monitor to 2 decimal places
  Serial.print("Input Voltage = ");
  Serial.println(in_voltage, 2);

}

void button_funtion()
{
  if (digitalRead(button_pin) == HIGH)
  {
    while (digitalRead(button_pin) == 1);

    if (bf == 0)
    {
      bf = 1;
    }

  }
  if (bf == 1)
  {
    digitalWrite(solenoid_purge, LOW);
    initial_milli = millis();
    bf = 2;
  }
  if (bf == 2 && (millis() - initial_milli > 1000))
  {
    sec++;
    initial_milli = millis();
  }
  if (sec >= 5)
  {
    digitalWrite(solenoid_purge, HIGH);
    bf = 0;
    sec = 0;
  }
}

void write_to_sd_card()
{

  if (millis() - sd_millis > 1000)
  {
    secon++;
    sd_millis = millis();
  }

  if (secon >= 5)
  {
    secon = 0;
    Serial.println("Writing to sd card");
    myFile = SD.open("data.txt", FILE_WRITE);
    // if the file opened okay, write to it:
    if (myFile)
    {
      Serial.print("Writing to data.txt...");
      myFile.println("data in file is as below :)");
      myFile.println("DC Current 1 is: " + String(current_1));
      myFile.println("DC Current 2 is: " + String(current_2));

      myFile.println("DC Voltage is: " + String(in_voltage));
      myFile.println("pressure is: " + String(bar_pressure) + " BAR");
      myFile.println("Temeperature from thermocouple is: " + String(cel));

      myFile.close();
      Serial.println("done.");
      delay(1000);
    }

    else
    {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }


  }



}
