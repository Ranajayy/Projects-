#include<Wire.h>
#define STANDARD_BAROMETRIC_PRESSURE 102500
byte PX_ADDRESS_SLAVE = 0X28;
byte PX_REGISTER_XY = 0X00;
byte PX_READ_LENGTH = 4;
const int16_t MS4525FullScaleRange = 1;
const int16_t MS4525MinScaleCounts = 1638;
const int16_t MS4525FullScaleCounts = 14746;
const int16_t MS4525Span = MS4525FullScaleCounts - MS4525MinScaleCounts;
const int16_t MS4525ZeroCounts = (MS4525MinScaleCounts + MS4525FullScaleCounts) / 2;

uint16_t px_pressure;
float px_psi;
float px_velocity;
float px_airDensity = 1.225;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  // put your setup code here, to run once:

}

void loop()
{
  Wire.beginTransmission(PX_ADDRESS_SLAVE);
  Wire.write(PX_REGISTER_XY);
  Wire.endTransmission(false);

  Wire.requestFrom(PX_ADDRESS_SLAVE, PX_READ_LENGTH);
  byte buff[PX_READ_LENGTH];
  Wire.readBytes(buff, PX_READ_LENGTH);

  byte pressure_h = buff[0];
  byte pressure_l = buff[1];
  px_pressure = (((uint16_t)pressure_h) << 8) | pressure_l;

  px_psi = (static_cast<float>(static_cast<int16_t>(px_pressure) - MS4525ZeroCounts)) / static_cast<float>(MS4525Span) * static_cast<float>(MS4525FullScaleRange);

  if (px_psi < 0) {
    px_velocity = -sqrt(-(2 * px_psi) / px_airDensity);
  } else {
    px_velocity = sqrt((2 * px_psi) / px_airDensity);
  }
  px_velocity = px_velocity * 10;
  Serial.println(px_velocity);
  // put your main code here, to run repeatedly:

}
