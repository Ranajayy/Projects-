#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <SPI.h>
#include <RH_RF95.h>

#if defined(ADAFRUIT_FEATHER_M0) || defined(ADAFRUIT_FEATHER_M0_EXPRESS) || defined(ARDUINO_SAMD_FEATHER_M0)  // Feather M0 w/Radio
#define RFM95_CS    8
#define RFM95_INT   3
#define RFM95_RST   4

#elif defined(ARDUINO_ADAFRUIT_FEATHER_RP2040_RFM)  // Feather RP2040 w/Radio
#define RFM95_CS   16
#define RFM95_INT  21
#define RFM95_RST  17

#elif defined (__AVR_ATmega328P__)  // Feather 328P w/wing
#define RFM95_CS    4  //
#define RFM95_INT   3  //
#define RFM95_RST   2  // "A"

#elif defined(ESP8266)  // ESP8266 feather w/wing
#define RFM95_CS    2  // "E"
#define RFM95_INT  15  // "B"
#define RFM95_RST  16  // "D"

#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) || defined(ARDUINO_NRF52840_FEATHER) || defined(ARDUINO_NRF52840_FEATHER_SENSE)
#define RFM95_CS   10  // "B"
#define RFM95_INT   9  // "A"
#define RFM95_RST  11  // "C"

#elif defined(ESP32)  // ESP32 feather w/wing
#define RFM95_CS   33  // "B"
#define RFM95_INT  27  // "A"
#define RFM95_RST  13

#elif defined(ARDUINO_NRF52832_FEATHER)  // nRF52832 feather w/wing
#define RFM95_CS   11  // "B"
#define RFM95_INT  31  // "C"
#define RFM95_RST   7  // "A"

#endif

#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
Adafruit_8x16minimatrix matrix = Adafruit_8x16minimatrix();

const int potPin = A0;
int16_t packetnum = 0;

void setup()
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  Serial.begin(115200);
//  while (!Serial) delay(1);
//  delay(100);
  matrix.begin(0x70);
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init())
  {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }

  Serial.println("LoRa radio init OK!");
  if (!rf95.setFrequency(RF95_FREQ))
  {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  rf95.setTxPower(23, false);
}

void loop()
{
  int sensorValue = analogRead(potPin);  // Read the analog value from the potentiometer
  int mappedValue = map(sensorValue, 0, 1023, 20, 400);  // Map the analog value to a range between 0 and 500
  displayValue(mappedValue);
  byte bytesToSend[sizeof(int)];
  memcpy(bytesToSend, &mappedValue, sizeof(int));
  Serial.println("Transmitting...");
  Serial.println("Sending...");
  delay(10);
  rf95.send(bytesToSend, sizeof(int));

  Serial.println("Waiting for packet to complete...");
  delay(10);
  rf95.waitPacketSent();
  delay(200);
}

void displayValue(int value)
{
  matrix.clear();
  matrix.setTextSize(0);
  matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix.setTextColor(LED_ON);
  matrix.setRotation(1);
  matrix.clear();  // Clear the LED matrix display

  int hundreds = value / 100;
  int tens = (value / 10) % 10;
  int ones = value % 10;

  String t = String(hundreds) + String(tens) + String(ones);
  Serial.println(t);
  matrix.setCursor(-1, 0);
  matrix.print(t);
  matrix.writeDisplay();  // Update the LED matrix display
}
