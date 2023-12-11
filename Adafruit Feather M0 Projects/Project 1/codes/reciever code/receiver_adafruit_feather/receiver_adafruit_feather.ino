#include <SPI.h>
#include <RH_RF95.h>
#include <Adafruit_DS3502.h>


Adafruit_DS3502 ds3502 = Adafruit_DS3502();

// First 3 here are boards w/radio BUILT-IN. Boards using FeatherWing follow.
#if defined (__AVR_ATmega32U4__)  // Feather 32u4 w/Radio
#define RFM95_CS    8
#define RFM95_INT   7
#define RFM95_RST   4

#elif defined(ADAFRUIT_FEATHER_M0) || defined(ADAFRUIT_FEATHER_M0_EXPRESS) || defined(ARDUINO_SAMD_FEATHER_M0)  // Feather M0 w/Radio
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


// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
//  while (!Serial)
//    delay(1);
//  delay(100);

  Serial.println("Feather LoRa RX Test!");

  // manual reset
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

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ))
  {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  rf95.setTxPower(23, false);

  if (!ds3502.begin())
  {
    Serial.println("Couldn't find DS3502 chip");
    while (1);
  }
  Serial.println("Found DS3502 chip");
}

void loop()
{
  if (rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {
      digitalWrite(LED_BUILTIN, HIGH);
      int rcvn;
      memcpy(&rcvn, buf, sizeof(int));
      Serial.print("Got: ");
      Serial.println(rcvn);
      int pot_val = map(rcvn, 0, 400, 0, 127);
      Serial.println(pot_val);
      ds3502.setWiper(pot_val);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
}
