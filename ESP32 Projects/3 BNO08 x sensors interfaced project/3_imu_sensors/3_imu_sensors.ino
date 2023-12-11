#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BNO08x.h>


Adafruit_BNO08x bno08x(-1);
sh2_SensorValue_t sensorValue;

#define I2C_SDA_PIN 21   // SDA pin for esp32 wroom
#define I2C_SCL_PIN 22   //SCL pin for esp32 wroom
#define sensor1_pin 16
#define sensor2_pin 17
#define sensor3_pin 18


String Date = "12/13/2021";
String Test = "Test 1";


void setup(void)
{
  Serial.begin(115200);
  Serial.println("Adafruit BNO08x test!");
  Wire.begin( I2C_SDA_PIN, I2C_SCL_PIN );

  if (!bno08x.begin_I2C(0X4A))
  {
    Serial.println("Failed to find BNO08x chip");
    while (1)
    {
      delay(10);
    }
  }
  Serial.println("BNO08x Found!");
  pinMode(sensor1_pin, OUTPUT);
  pinMode(sensor2_pin, OUTPUT);
  pinMode(sensor3_pin, OUTPUT);
  setReports();

  Serial.println("Reading events");
  delay(100);
  Serial.println(Test);
  Serial.println(Date);
}


void loop()
{
  delay(10);

  if (bno08x.wasReset())
  {
    //Serial.print("sensor was reset ");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue))
  {
    return;
  }

  Serial.println("Sensor 1");
  switch (sensorValue.sensorId)
  {

    case SH2_ACCELEROMETER:
      Serial.print("Accelerometer - X11: ");
      Serial.print(sensorValue.un.accelerometer.x);
      Serial.print(" Y11: ");
      Serial.print(sensorValue.un.accelerometer.y);
      Serial.print(" Z11: ");
      Serial.println(sensorValue.un.accelerometer.z);
      break;
    case SH2_GYROSCOPE_CALIBRATED:
      Serial.print("Gyro - X12: ");
      Serial.print(sensorValue.un.gyroscope.x);
      Serial.print(" Y12: ");
      Serial.print(sensorValue.un.gyroscope.y);
      Serial.print(" Z12: ");
      Serial.println(sensorValue.un.gyroscope.z);
      break;
    case SH2_MAGNETIC_FIELD_CALIBRATED:
      Serial.print("Magnetic Field - X13: ");
      Serial.print(sensorValue.un.magneticField.x);
      Serial.print(" Y13: ");
      Serial.print(sensorValue.un.magneticField.y);
      Serial.print(" Z13: ");
      Serial.println(sensorValue.un.magneticField.z);
      break;
    case SH2_LINEAR_ACCELERATION:
      Serial.print("Linear Acceration - X14: ");
      Serial.print(sensorValue.un.linearAcceleration.x);
      Serial.print(" Y14: ");
      Serial.print(sensorValue.un.linearAcceleration.y);
      Serial.print(" Z14: ");
      Serial.println(sensorValue.un.linearAcceleration.z);
      break;
    case SH2_GRAVITY:
      Serial.print("Gravity - X15: ");
      Serial.print(sensorValue.un.gravity.x);
      Serial.print(" Y15: ");
      Serial.print(sensorValue.un.gravity.y);
      Serial.print(" Z15: ");
      Serial.println(sensorValue.un.gravity.z);
      break;
    case SH2_ROTATION_VECTOR:
      Serial.print("Rotation Vector - X16: ");
      Serial.print(sensorValue.un.rotationVector.real);
      Serial.print(" Y16: ");
      Serial.print(sensorValue.un.rotationVector.i);
      Serial.print(" Z16: ");
      Serial.print(sensorValue.un.rotationVector.j);
      Serial.print(" W16: ");
      Serial.println(sensorValue.un.rotationVector.k);
      break;
  }

  digitalWrite(sensor1_pin, HIGH);
  digitalWrite(sensor2_pin, LOW);
  digitalWrite(sensor3_pin, HIGH);
  bno08x.begin_I2C(0X4A);
  if (bno08x.wasReset())
  {
    //Serial.print("sensor was reset ");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue))
  {
    return;
  }
  Serial.println("Sensor 2");
  switch (sensorValue.sensorId)
  {

    case SH2_ACCELEROMETER:
      Serial.print("Accelerometer - X21: ");
      Serial.print(sensorValue.un.accelerometer.x);
      Serial.print(" Y21: ");
      Serial.print(sensorValue.un.accelerometer.y);
      Serial.print(" Z21: ");
      Serial.println(sensorValue.un.accelerometer.z);
      break;
    case SH2_GYROSCOPE_CALIBRATED:
      Serial.print("Gyro - X22: ");
      Serial.print(sensorValue.un.gyroscope.x);
      Serial.print(" Y22: ");
      Serial.print(sensorValue.un.gyroscope.y);
      Serial.print(" Z22: ");
      Serial.println(sensorValue.un.gyroscope.z);
      break;
    case SH2_MAGNETIC_FIELD_CALIBRATED:
      Serial.print("Magnetic Field - X23: ");
      Serial.print(sensorValue.un.magneticField.x);
      Serial.print(" Y23: ");
      Serial.print(sensorValue.un.magneticField.y);
      Serial.print(" Z23: ");
      Serial.println(sensorValue.un.magneticField.z);
      break;
    case SH2_LINEAR_ACCELERATION:
      Serial.print("Linear Acceration - X24: ");
      Serial.print(sensorValue.un.linearAcceleration.x);
      Serial.print(" Y24: ");
      Serial.print(sensorValue.un.linearAcceleration.y);
      Serial.print(" Z24: ");
      Serial.println(sensorValue.un.linearAcceleration.z);
      break;
    case SH2_GRAVITY:
      Serial.print("Gravity - X25: ");
      Serial.print(sensorValue.un.gravity.x);
      Serial.print(" Y25: ");
      Serial.print(sensorValue.un.gravity.y);
      Serial.print(" Z25: ");
      Serial.println(sensorValue.un.gravity.z);
      break;
    case SH2_ROTATION_VECTOR:
      Serial.print("Rotation Vector - X26: ");
      Serial.print(sensorValue.un.rotationVector.real);
      Serial.print(" Y26: ");
      Serial.print(sensorValue.un.rotationVector.i);
      Serial.print(" Z26: ");
      Serial.print(sensorValue.un.rotationVector.j);
      Serial.print(" W26: ");
      Serial.println(sensorValue.un.rotationVector.k);
      break;

  }
  digitalWrite(sensor1_pin, HIGH);
  digitalWrite(sensor2_pin, HIGH);
  digitalWrite(sensor3_pin, LOW);
  bno08x.begin_I2C(0X4A);
  if (bno08x.wasReset())
  {
    //Serial.print("sensor was reset ");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue))
  {
    return;
  }
  Serial.println("Sensor 3");
  switch (sensorValue.sensorId)
  {

    case SH2_ACCELEROMETER:
      Serial.print("Accelerometer - X31: ");
      Serial.print(sensorValue.un.accelerometer.x);
      Serial.print(" Y31: ");
      Serial.print(sensorValue.un.accelerometer.y);
      Serial.print(" Z31: ");
      Serial.println(sensorValue.un.accelerometer.z);
      break;
    case SH2_GYROSCOPE_CALIBRATED:
      Serial.print("Gyro - X32: ");
      Serial.print(sensorValue.un.gyroscope.x);
      Serial.print(" Y32: ");
      Serial.print(sensorValue.un.gyroscope.y);
      Serial.print(" Z32: ");
      Serial.println(sensorValue.un.gyroscope.z);
      break;
    case SH2_MAGNETIC_FIELD_CALIBRATED:
      Serial.print("Magnetic Field - X33: ");
      Serial.print(sensorValue.un.magneticField.x);
      Serial.print(" Y33: ");
      Serial.print(sensorValue.un.magneticField.y);
      Serial.print(" Z33: ");
      Serial.println(sensorValue.un.magneticField.z);
      break;
    case SH2_LINEAR_ACCELERATION:
      Serial.print("Linear Acceration - X34: ");
      Serial.print(sensorValue.un.linearAcceleration.x);
      Serial.print(" Y34: ");
      Serial.print(sensorValue.un.linearAcceleration.y);
      Serial.print(" Z34: ");
      Serial.println(sensorValue.un.linearAcceleration.z);
      break;
    case SH2_GRAVITY:
      Serial.print("Gravity - X35: ");
      Serial.print(sensorValue.un.gravity.x);
      Serial.print(" Y35: ");
      Serial.print(sensorValue.un.gravity.y);
      Serial.print(" Z35: ");
      Serial.println(sensorValue.un.gravity.z);
      break;
    case SH2_ROTATION_VECTOR:
      Serial.print("Rotation Vector - X36: ");
      Serial.print(sensorValue.un.rotationVector.real);
      Serial.print(" Y36: ");
      Serial.print(sensorValue.un.rotationVector.i);
      Serial.print(" Z36: ");
      Serial.print(sensorValue.un.rotationVector.j);
      Serial.print(" W36: ");
      Serial.println(sensorValue.un.rotationVector.k);
      break;
  }
  digitalWrite(sensor1_pin, LOW);
  digitalWrite(sensor2_pin, HIGH);
  digitalWrite(sensor3_pin, HIGH);
  bno08x.begin_I2C(0X4A);

}
void setReports(void)
{
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER))
  {
    Serial.println("Could not enable accelerometer");
  }
  if (!bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED))
  {
    Serial.println("Could not enable gyroscope");
  }
  if (!bno08x.enableReport(SH2_MAGNETIC_FIELD_CALIBRATED))
  {
    Serial.println("Could not enable magnetic field calibrated");
  }
  if (!bno08x.enableReport(SH2_LINEAR_ACCELERATION))
  {
    Serial.println("Could not enable linear acceleration");
  }
  if (!bno08x.enableReport(SH2_GRAVITY))
  {
    Serial.println("Could not enable gravity vector");
  }
  if (!bno08x.enableReport(SH2_ROTATION_VECTOR))
  {
    Serial.println("Could not enable rotation vector");
  }
  if (!bno08x.enableReport(SH2_GEOMAGNETIC_ROTATION_VECTOR))
  {
    Serial.println("Could not enable geomagnetic rotation vector");
  }
  if (!bno08x.enableReport(SH2_GAME_ROTATION_VECTOR))
  {
    Serial.println("Could not enable game rotation vector");
  }

  if (!bno08x.enableReport(SH2_RAW_ACCELEROMETER))
  {
    Serial.println("Could not enable raw accelerometer");
  }
  if (!bno08x.enableReport(SH2_RAW_GYROSCOPE))
  {
    Serial.println("Could not enable raw gyroscope");
  }
  if (!bno08x.enableReport(SH2_RAW_MAGNETOMETER))
  {
    Serial.println("Could not enable raw magnetometer");
  }
}
