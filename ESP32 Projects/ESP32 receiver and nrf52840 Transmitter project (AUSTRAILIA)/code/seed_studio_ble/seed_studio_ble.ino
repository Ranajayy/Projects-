#include <ArduinoBLE.h>
#include <Wire.h>
#include <ArduinoNunchuk.h>

#define wake_up_pin 2
#define dip1 7
#define dip2 8
#define dip3 9
#define dip4 10

String UID = "";
const char* uid0 = "934974df-ae5e-432c-b64c-aa48664655b7";
const char* uid1 = "gfads";
const char* uid2 = "dsgasd";
const char* uid3 = "aaadgsa";
const char* uid4 = "hhhahh";
const char* uid5 = "hadshas12";
const char* uid6 = "3243245";
const char* uid7 = "5436457";
const char* uid8 = "ht45h242";
const char* uid9 = "h2jhu666";
const char* uid10 = "f3234565";
const char* uid11 = "ff34456526";
const char* uid12 = "f2634621";
const char* uid13 = "1643643ff";
const char* uid14 = "hjhjj42j";
const char* uid15 = "8798kkj";

ArduinoNunchuk nunchuk = ArduinoNunchuk();

void setup()
{
  Serial.begin(115200);
  pinMode(wake_up_pin, INPUT);
  pinMode(dip1, INPUT_PULLUP);
  pinMode(dip2, INPUT_PULLUP);
  pinMode(dip3, INPUT_PULLUP);
  pinMode(dip4, INPUT_PULLUP);
  nrf_gpio_cfg_sense_input(wake_up_pin, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);
  nunchuk.init();

  // initialize the Bluetooth® Low Energy hardware
  BLE.begin();

  Serial.println("Bluetooth® Low Energy Central - Relay control");
  change_uid();
  Serial.println(UID);
  BLE.scanForUuid(UID);


}

void loop()
{


  BLEDevice peripheral = BLE.available();
  if (peripheral)
  {
    BLE.scanForUuid(UID);
    if (peripheral.advertisedServiceUuid() != UID)
    {
      return;
    }

    BLE.stopScan();

    controlLed(peripheral);
    change_uid();
    BLE.scanForUuid(UID);

  }

  if (digitalRead(wake_up_pin) == 1)
  {
    Serial.println("going to sleep!");
    NRF_POWER->SYSTEMOFF = 1;
  }
  change_uid();
  BLE.scanForUuid(UID);

}

void controlLed(BLEDevice peripheral)
{
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // retrieve the LED characteristic
  BLECharacteristic relayCharacteristic = peripheral.characteristic("944974df-ae5e-432c-b64c-aa48664655b7");

  if (!relayCharacteristic)
  {
    Serial.println("Peripheral does not have relay control characteristic!");
    peripheral.disconnect();
    return;
  } else if (!relayCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable relay characteristic!");
    peripheral.disconnect();
    return;
  }

  while (peripheral.connected())
  {
    nunchuk.update();
    if (nunchuk.analogY >= 180)
    {
      relayCharacteristic.writeValue("up");
      // Serial.println("UP");
    }
    if (nunchuk.analogY < 50 )
    {
      relayCharacteristic.writeValue("down");
      // Serial.println("DOWN");
    }
    if (nunchuk.analogX < 50)
    {
      relayCharacteristic.writeValue("left");
      // Serial.println("LEFT");
    }

    if (nunchuk.analogX >= 200)
    {
      relayCharacteristic.writeValue("right");
      // Serial.println("RIGHT");
    }
    if (nunchuk.cButton == 1)
    {
      delay(1000);
      relayCharacteristic.writeValue("c_1");
    }
    if (nunchuk.zButton == 1)
    {
      delay(1000);
      relayCharacteristic.writeValue("z_1");
    }
    if (nunchuk.analogX >= 100 && nunchuk.analogX <= 130 && nunchuk.analogY >= 100 && nunchuk.analogY <= 130 )
    {
      relayCharacteristic.writeValue("0");
    }
    if (digitalRead(wake_up_pin) == 1)
    {
      Serial.println("going to sleep!");
      NRF_POWER->SYSTEMOFF = 1;
    }
  }

  Serial.println("Peripheral disconnected");
}

void change_uid()
{
  if (digitalRead(dip1) == 0 && digitalRead(dip2) == 0 && digitalRead(dip3) == 0 && digitalRead(dip4) == 0)
  {
    UID = uid0;
  }

  if (digitalRead(dip1) == 0 && digitalRead(dip2) == 0 && digitalRead(dip3) == 0 && digitalRead(dip4) == 1)
  {
    UID = uid1;
  }
  if (digitalRead(dip1) == 0 && digitalRead(dip2) == 0 && digitalRead(dip3) == 1 && digitalRead(dip4) == 0)
  {
    UID = uid2;
  }
  if (digitalRead(dip1) == 0 && digitalRead(dip2) == 0 && digitalRead(dip3) == 1 && digitalRead(dip4) == 1)
  {
    UID = uid3;
  }
  if (digitalRead(dip1) == 0 && digitalRead(dip2) == 1 && digitalRead(dip3) == 0 && digitalRead(dip4) == 0)
  {
    UID = uid4;
  }
  if (digitalRead(dip1) == 0 && digitalRead(dip2) == 1 && digitalRead(dip3) == 0 && digitalRead(dip4) == 1)
  {
    UID = uid5;
  }
  if (digitalRead(dip1) == 0 && digitalRead(dip2) == 1 && digitalRead(dip3) == 1 && digitalRead(dip4) == 0)
  {
    UID = uid6;
  }
  if (digitalRead(dip1) == 0 && digitalRead(dip2) == 1 && digitalRead(dip3) == 1 && digitalRead(dip4) == 1)
  {
    UID = uid7;
  }
  if (digitalRead(dip1) == 1 && digitalRead(dip2) == 0 && digitalRead(dip3) == 0 && digitalRead(dip4) == 0)
  {
    UID = uid8;
  }
  if (digitalRead(dip1) == 1 && digitalRead(dip2) == 0 && digitalRead(dip3) == 0 && digitalRead(dip4) == 1)
  {
    UID = uid9;
  }
  if (digitalRead(dip1) == 1 && digitalRead(dip2) == 0 && digitalRead(dip3) == 1 && digitalRead(dip4) == 0)
  {
    UID = uid10;
  }
  if (digitalRead(dip1) == 1 && digitalRead(dip2) == 0 && digitalRead(dip3) == 1 && digitalRead(dip4) == 1)
  {
    UID = uid11;
  }
  if (digitalRead(dip1) == 1 && digitalRead(dip2) == 1 && digitalRead(dip3) == 0 && digitalRead(dip4) == 0)
  {
    UID = uid12;
  }
  if (digitalRead(dip1) == 1 && digitalRead(dip2) == 1 && digitalRead(dip3) == 0 && digitalRead(dip4) == 1)
  {
    UID = uid13;
  }
  if (digitalRead(dip1) == 1 && digitalRead(dip2) == 1 && digitalRead(dip3) == 1 && digitalRead(dip4) == 0)
  {
    UID = uid14;
  }
  if (digitalRead(dip1) == 1 && digitalRead(dip2) == 1 && digitalRead(dip3) == 1 && digitalRead(dip4) == 1)
  {
    UID = uid15;
  }

}
