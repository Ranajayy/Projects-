#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>


Preferences prefrences; // saving mac adresses


#define LED_PIN 3
#define NUM_LEDS 1
Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const int oneWireBus = 0;
OneWire oneWire(oneWireBus);

DallasTemperature sensors(&oneWire);
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;
float prev_temp;
float prev_humidity = 0;
float battery_voltage = 0;
float vd_level = 0;
float vd_voltage = 0;
float percentage = 0;
int perc = 0;
bool led_stat = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks1: public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
      std::string rcv = pCharacteristic->getValue();
      if (rcv.length() > 0)
      {
        Serial.print("recived:\n");
        for (int i = 0; i < rcv.length(); i++)
        {
          Serial.print(rcv[i]);
        }
      }

      if (rcv == "Led on")
      {
        pixels.setPixelColor(0, pixels.Color(255, 0, 0));  // Red color
        pixels.show();
        prefrences.begin("data", false);
        prefrences.putString("led", "1");
        prefrences.end();
        led_stat = true;
      }
      else if (rcv == "Led off")
      {
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));  // black color
        pixels.show();
        prefrences.begin("data", false);
        prefrences.putString("led", "0");
        prefrences.end();
        led_stat = false;
      }

      else if (rcv.find("S") != std::string::npos)
      {
        // Substring "S" found in rcv
        size_t colonPos = rcv.find(":");
        if (colonPos != std::string::npos)
        {
          std::string bright = rcv.substr(colonPos + 1);
          int brightnessValue = std::stoi(bright);
          pixels.setBrightness(brightnessValue);
          pixels.show();
        }
      }

    }
};

void updateTemp(float temp)
{
  if (prev_temp != temp)
  {

    String tempString = "";
    tempString += (int)temp;
    tempString += "C";
    prev_temp = temp;
  }

  if (led_stat == true)
  {
    if (temp < 32.2)
    {
      pixels.setPixelColor(0, pixels.Color(0, 0, 255));  // blue color
      pixels.show();
    }
    else if (temp > 32.2 && temp < 37.2)
    {
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));  // green color
      pixels.show();
    }
    else if (temp > 37.2)
    {
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));  // red color
      pixels.show();
    }
  }
  Serial.println(temp);
}

void updateTemp_test()
{
  prev_temp++;
  if (prev_temp == 60)
  {
    prev_temp = 0;
  }

}

void updateHumidity()
{

  vd_level = analogRead(4);
  // Serial.println(vd_level);
  vd_voltage = vd_level / 4095.0;
  // Serial.println(vd_voltage);
  vd_voltage = vd_voltage * 2.83 * 2;
  Serial.println(vd_voltage);
  percentage = (vd_voltage - 3) * 83.33;

  if (percentage > 100)
  {
    perc = 100;
  }

  else if (percentage < 0)
  {
    perc = 0;
  }
  else
  {
    perc = int(percentage);
  }
  Serial.println(perc);

}


void setup()
{
  Serial.begin(115200);
  pixels.begin();
  sensors.begin();

  prefrences.begin("data", false);
  String ld = prefrences.getString("led", "");
  Serial.println(ld);
  if (ld == "1")
  {
    led_stat = true;
  }

  else
  {
    led_stat = false;
  }

  // Create the BLE Device
  BLEDevice::init("Temperature Monitor");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->setCallbacks(new MyCallbacks1());
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

}

void loop()
{
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  //float temperatureF = sensors.getTempFByIndex(0);
  updateTemp(temperatureC);
  updateHumidity();

  //  updateTemp_test();
  // notify changed value

  if (deviceConnected)
  {

    delay(10);
    String str = "";
    str += prev_temp;
    str += ",";
    str += perc;
    pCharacteristic->setValue((char*)str.c_str());
    pCharacteristic->notify();

    // Serial.println(percentage);
    // Serial.println(F("%  Temperature: "));

    // Serial.println(prev_temp);
    // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
  }
  // disconnecting Serial.println("start advertising");
  oldDeviceConnected = deviceConnected;
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500); // give the bluetooth stack the chance to get things ready

  }

  // connecting
  if (deviceConnected && !oldDeviceConnected)
  {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
  delay (500);
}


String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
