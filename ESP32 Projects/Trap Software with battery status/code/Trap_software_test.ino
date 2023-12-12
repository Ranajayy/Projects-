#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <FastLED.h>

Adafruit_MPU6050 mpu;
HardwareSerial gsmserial(1);
String smsStatus, senderNumber, receivedDate, msg, gsm = "";

//pins for led ring,
#define NUM_LEDS 16 // number of leds change according to your led ring
#define LED_RING_PIN 3
CRGB leds[NUM_LEDS];
//pins for sim800l
int rx = 13; // esp rx
int tx = 12; // esp tx
// pins for SDA and SCL of MPU are 21 and 22 respectively on ESP32-WROOM just connect them there
int vd_level = 0 ;
float vd_voltage = 0;
float battery_voltage = 0;
float x_previous = 0;
float y_previous = 0;
float z_previous = 0;
float x_current = 0;
float y_current = 0;
float z_current = 0;
int pos_flag;
int battery_flag1;
int battery_flag2;
int battery_flag3;
int battery_flag4;
int battery_flag5;
String phone_number = "+923325032468"; //enter phone number here with the country code



void setup()
{
  Serial.begin(115200);
  gsmserial.begin(9600, SERIAL_8N1, rx, tx);// tx of sim module in pin 13 of esp and rx of sim module to pin 12 of esp
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");


  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
    case MPU6050_RANGE_250_DEG:
      Serial.println("+- 250 deg/s");
      break;
    case MPU6050_RANGE_500_DEG:
      Serial.println("+- 500 deg/s");
      break;
    case MPU6050_RANGE_1000_DEG:
      Serial.println("+- 1000 deg/s");
      break;
    case MPU6050_RANGE_2000_DEG:
      Serial.println("+- 2000 deg/s");
      break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
    case MPU6050_BAND_260_HZ:
      Serial.println("260 Hz");
      break;
    case MPU6050_BAND_184_HZ:
      Serial.println("184 Hz");
      break;
    case MPU6050_BAND_94_HZ:
      Serial.println("94 Hz");
      break;
    case MPU6050_BAND_44_HZ:
      Serial.println("44 Hz");
      break;
    case MPU6050_BAND_21_HZ:
      Serial.println("21 Hz");
      break;
    case MPU6050_BAND_10_HZ:
      Serial.println("10 Hz");
      break;
    case MPU6050_BAND_5_HZ:
      Serial.println("5 Hz");
      break;
  }

  Serial.print("led ring being tested!");
  FastLED.addLeds<WS2812, LED_RING_PIN, GRB>(leds, NUM_LEDS);
  for (int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red
    leds[i] = CHSV(1, 255, 255);
    // Show the leds
    FastLED.show();
    delay(300);
    // now that we've shown the leds, reset the i'th led to black
    leds[i] = CRGB::Black;
    FastLED.show();

    // Wait a little bit before we loop around and do it again
  }

  Serial.println("");
  delay(100);
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  Serial.print("GYROSCOPE's Current Position is: ");
  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");
  x_previous = g.gyro.x;
  y_previous = g.gyro.y;
  z_previous = g.gyro.z;
  Serial.println("");
  gsmserial.print("AT+CMGF=1\r");
  Serial.print("sms Mode Ready!");
  delay(2000);


}

void loop()
{
  vd_level = analogRead(4);
  vd_voltage = float(3.3 / 4096) * vd_level;
  battery_voltage = float((vd_voltage * (2700 + 10000)) / 10000);
  Serial.print("Battery voltage is: ");
  Serial.println(battery_voltage, 2);

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  x_current = g.gyro.x;
  y_current = g.gyro.y;
  z_current = g.gyro.z;


  if ((abs(x_current - x_previous) >= 0.01 || abs(y_current - y_previous ) >= 0.01) || abs(z_current - z_previous) >= 0.01  )
  {

    Serial.println("GYROSCOPES Angle has changed!");
    delay(3000);
  }

  if (battery_voltage >= 4.0 && battery_voltage <= 4.2)
  {

    leds[0] = CRGB::Green;
    leds[1] = CRGB::Green;
    leds[2] = CRGB::Green;
    leds[3] = CRGB::Green;
    leds[4] = CRGB::Green;
    leds[5] = CRGB::Green;
    leds[6] = CRGB::Green;
    leds[7] = CRGB::Green;
    leds[8] = CRGB::Green;
    leds[9] = CRGB::Green;
    leds[10] = CRGB::Green;
    leds[11] = CRGB::Green;
    leds[12] = CRGB::Green;
    leds[13] = CRGB::Green;
    leds[14] = CRGB::Green;
    leds[15] = CRGB::Green;
    FastLED.show();
    if ( battery_flag1 == 0)
    {
      battery_flag1 = 1;
      String msg_1 = String(battery_voltage);
      Send_message("Battery voltage is: " + msg_1, phone_number);
      Serial.print("msg 1 sent");
    }

  }


  if (battery_voltage <= 3.9 && battery_voltage >= 3.8)
  {

    leds[0] = CRGB::Green;
    leds[1] = CRGB::Green;
    leds[2] = CRGB::Green;
    leds[3] = CRGB::Green;
    leds[4] = CRGB::Green;
    leds[5] = CRGB::Green;
    leds[6] = CRGB::Green;
    leds[7] = CRGB::Green;
    leds[8] = CRGB::Yellow;
    leds[9] = CRGB::Yellow;
    leds[10] = CRGB::Yellow;
    leds[11] = CRGB::Yellow;
    leds[12] = CRGB::Yellow;
    leds[13] = CRGB::Yellow;
    leds[14] = CRGB::Yellow;
    leds[15] = CRGB::Yellow;
    FastLED.show();
    if (battery_flag2 == 0)
    {
      battery_flag2 = 1;
      String msg_2 = String(battery_voltage);
      Send_message("Battery voltage is: " + msg_2, phone_number);
    }
  }


  if (battery_voltage >= 3.4 && battery_voltage <= 3.7)
  {

    leds[0] = CRGB::Yellow;
    leds[1] = CRGB::Yellow;
    leds[2] = CRGB::Yellow;
    leds[3] = CRGB::Yellow;
    leds[4] = CRGB::Yellow;
    leds[5] = CRGB::Yellow;
    leds[6] = CRGB::Yellow;
    leds[7] = CRGB::Yellow;
    leds[8] = CRGB::Yellow;
    leds[9] = CRGB::Yellow;
    leds[10] = CRGB::Yellow;
    leds[11] = CRGB::Yellow;
    leds[12] = CRGB::Yellow;
    leds[13] = CRGB::Yellow;
    leds[14] = CRGB::Yellow;
    leds[15] = CRGB::Yellow;
    FastLED.show();
    if (battery_flag3 == 0)
    {
      battery_flag3 = 1;
      String msg_3 = String(battery_voltage);
      Send_message("Battery voltage is: " + msg_3, phone_number);
    }
  }
  if (battery_voltage >= 3.2 && battery_voltage <= 3.3)
  {

    leds[0] = CRGB::Yellow;
    leds[1] = CRGB::Yellow;
    leds[2] = CRGB::Yellow;
    leds[3] = CRGB::Yellow;
    leds[4] = CRGB::Yellow;
    leds[5] = CRGB::Yellow;
    leds[6] = CRGB::Yellow;
    leds[7] = CRGB::Yellow;
    leds[8] = CRGB::Red;
    leds[9] = CRGB::Red;
    leds[10] = CRGB::Red;
    leds[11] = CRGB::Red;
    leds[12] = CRGB::Red;
    leds[13] = CRGB::Red;
    leds[14] = CRGB::Red;
    leds[15] = CRGB::Red;
    FastLED.show();
    if ( battery_flag4 == 0)
    {
      battery_flag4 = 1;
      String msg_4 = String(battery_voltage);
      Send_message("Battery voltage is: " + msg_4, phone_number);
    }
  }


  if (battery_voltage <= 3.1)
  {

    leds[0] = CRGB::Red;
    leds[1] = CRGB::Red;
    leds[2] = CRGB::Red;
    leds[3] = CRGB::Red;
    leds[4] = CRGB::Red;
    leds[5] = CRGB::Red;
    leds[6] = CRGB::Red;
    leds[7] = CRGB::Red;
    leds[8] = CRGB::Red;
    leds[9] = CRGB::Red;
    leds[10] = CRGB::Red;
    leds[11] = CRGB::Red;
    leds[12] = CRGB::Red;
    leds[13] = CRGB::Red;
    leds[14] = CRGB::Red;
    leds[15] = CRGB::Red;
    FastLED.show();
    if (battery_flag5 == 0)
    {
      battery_flag5 = 1;
      String msg_5 = String(battery_voltage);
      Send_message("Battery voltage is: " + msg_5, phone_number);
    }
  }
}


void Send_message(String text, String number)
{
  gsmserial.print("AT+CMGF=1\r");
  delay(1000);
  gsmserial.print("AT+CMGS=\"" + number + "\"\r");
  delay(1000);
  gsmserial.print(text);
  delay(100);
  gsmserial.write(0x1A); //ascii code for ctrl-26 //gsmserial.println((char)26); //ascii code for ctrl-26
  delay(1000);
  Serial.println("SMS Sent Successfully.");
}
