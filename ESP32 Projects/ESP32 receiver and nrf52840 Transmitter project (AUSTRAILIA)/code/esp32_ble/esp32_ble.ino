#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>



#define relay_pin1 23
#define relay_pin2 22
#define relay_pin3 21
#define relay_pin4 19
#define relay_pin5 18
#define relay_pin6 5

int c1 = 0;
int c2 = 0;
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
std::string rcv = "";

#define SERVICE_UUID        "934974df-ae5e-432c-b64c-aa48664655b7"
#define CHARACTERISTIC_UUID "944974df-ae5e-432c-b64c-aa48664655b7"


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
      rcv = pCharacteristic->getValue();
      if (rcv.length() > 0)
      {
        Serial.print("\nrecived:");
        for (int i = 0; i < rcv.length(); i++)
        {
          Serial.print(rcv[i]);
        }
      }
    }
};


void setup()
{
  Serial.begin(115200);

  pinMode(relay_pin1, OUTPUT);
  pinMode(relay_pin2, OUTPUT);
  pinMode(relay_pin3, OUTPUT);
  pinMode(relay_pin4, OUTPUT);
  pinMode(relay_pin5, OUTPUT);
  pinMode(relay_pin6, OUTPUT);

  digitalWrite(relay_pin1, HIGH);
  digitalWrite(relay_pin2, HIGH);
  digitalWrite(relay_pin3, HIGH);
  digitalWrite(relay_pin4, HIGH);
  digitalWrite(relay_pin5, HIGH);
  digitalWrite(relay_pin6, HIGH);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );
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
  if (deviceConnected)
  {
    if (rcv == "up")
    {
      digitalWrite(relay_pin1, LOW);
      digitalWrite(relay_pin2, LOW);
      digitalWrite(relay_pin4, HIGH);
      digitalWrite(relay_pin5, HIGH);
      digitalWrite(relay_pin3, HIGH);
      rcv = "";
    }
    if (rcv == "down")
    {
      digitalWrite(relay_pin4, HIGH);
      digitalWrite(relay_pin5, HIGH);
      digitalWrite(relay_pin2, HIGH);
      digitalWrite(relay_pin1, LOW);
      digitalWrite(relay_pin3, LOW);
      c2 = 0;
      rcv = "";

    }
    if (rcv == "left")
    {
      digitalWrite(relay_pin1, LOW);
      digitalWrite(relay_pin4, LOW);
      digitalWrite(relay_pin3, HIGH);
      digitalWrite(relay_pin2, HIGH);
      digitalWrite(relay_pin5, HIGH);
      rcv = "";
    }
    if (rcv == "right")
    {
      digitalWrite(relay_pin1, LOW);
      digitalWrite(relay_pin5, LOW);
      digitalWrite(relay_pin4, HIGH);
      digitalWrite(relay_pin3, HIGH);
      digitalWrite(relay_pin2, HIGH);
      rcv = "";
    }
    if (rcv[0] == 'c')
    {
      Serial.println("in button c");

      if (c1 == 0)
      {
        digitalWrite(relay_pin6, LOW);
        c1 = 1;

      }
      else if (c1 == 1)
      {
        digitalWrite(relay_pin6, HIGH);
        c1 = 0;

      }

      rcv = "";
    }

    if (rcv[0] == 'z' )
    {
      Serial.println("in button Z");
      if (c2 == 0)
      {
        digitalWrite(relay_pin3, LOW);
        c2 = 1;
      }
      else  if (c2 == 1)
      {
        digitalWrite(relay_pin3, HIGH);
        c2 = 0;

      }

      rcv = "";
    }

    if (  rcv == "up" && c2 == 1)
    {
      digitalWrite(relay_pin3, HIGH);
      c2 = 0;
      rcv = "";
    }

    if (rcv == "0")
    {
      digitalWrite(relay_pin1, HIGH);
      digitalWrite(relay_pin2, HIGH);
      digitalWrite(relay_pin4, HIGH);
      digitalWrite(relay_pin5, HIGH);
      if (c2 == 0)
      {
        digitalWrite(relay_pin3, HIGH);
      }
      rcv = "";
    }

  }

  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}
