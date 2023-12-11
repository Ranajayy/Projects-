int sensitivity = 66; // use 100 for 20A Module and 66 for 30A Module
int adcvalue = 0;
float offsetvoltage = 2.5;
double Voltage = 0; //voltage measuring
double ecurrent = 0;// Current measuring
void setup()
{
  //Robojax.com Allegro ACS Current Sensor
  Serial.begin(9600);// initialize serial monitor
  Serial.println("Robojax Tutorial");
  Serial.println("ACS770 Current Sensor");

}

void loop() {
  float a = analogRead(A1);
  a = (a * 3.3) / 1024;
  float  ecurrent = ((a - offsetvoltage) / sensitivity);
  Serial.println(ecurrent,2);

  delay(1000);
}
