#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include<WiFi.h>



#define red_led 26
#define green_led 25
#define door_pin 33
#define open_door_pin 17

const char* ssid = "OCTATHORN"; // change wtih your  WIFI ssid here
const char* pass = "62543800";  // change with you WIFI pass here
const char * host = "192.168.18.9"; // change with you ip here

const uint16_t port = 4260;
String str_res = "";
int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
#define SS_PIN  5  // ESP32 pin GIOP5 
#define RST_PIN 27 // ESP32 pin GIOP27 

MFRC522 rfid(SS_PIN, RST_PIN);
String rfid_tag = "";
unsigned long door_millis = 0;
unsigned long door_open_time = 5000;
unsigned long get_time = 0;
unsigned long time_to_get_data = 3000;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("...");
  }

  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());
  SPI.begin();
  rfid.PCD_Init();
  lcd.begin();
  lcd.backlight();
  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DOOR IS LOCKED");
  pinMode(red_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(open_door_pin, INPUT);
  pinMode(door_pin, OUTPUT);
  digitalWrite(red_led, HIGH);
  digitalWrite(green_led, LOW);
  get_time = millis();
}

void loop()
{
  update_time();
  if (rfid.PICC_IsNewCardPresent())
  {
    if (rfid.PICC_ReadCardSerial())
    {
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(rfid.PICC_GetTypeName(piccType));

      // print UID in Serial Monitor in the hex format
      Serial.print("UID:");
      for (int i = 0; i < rfid.uid.size; i++)
      {
        //  Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        //  Serial.print(rfid.uid.uidByte[i], HEX);
        rfid_tag += String(rfid.uid.uidByte[i], HEX) + " ";
      }
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
  }

  if (rfid_tag != "")
  {
    send_rfid();
    rfid_tag = "";
  }

  if (str_res != "")
  {

    if (str_res == "yes")
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("DOOR IS UNLOCKED");
      door_millis = millis();
      digitalWrite(door_pin, HIGH);
      digitalWrite(green_led, HIGH);
      digitalWrite(red_led, LOW);

    }
    else if (str_res == "no")
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("NO CREDITS");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("DOOR IS LOCKED");
    }
    str_res = "";
  }

  if (millis() - door_millis > door_open_time)
  {
    digitalWrite(door_pin, LOW);
    digitalWrite(red_led, HIGH);
    digitalWrite(green_led, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DOOR IS LOCKED");
  }
  if (digitalRead(open_door_pin) == 1)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DOOR IS UNLOCKED");
    door_millis = millis();
    digitalWrite(door_pin, HIGH);
    digitalWrite(green_led, HIGH);
    digitalWrite(red_led, LOW);
    send_door_state();
  }

}

void send_rfid()
{
  WiFiClient client;

  String send_data = "check," + rfid_tag + "\n\r";
  Serial.println(send_data);
  if (client.connect(host, port))
  {

    if (client.connected())
    {
      client.write(send_data.c_str());                      // send data
    }

    while (!client.available());                // wait for response

    str_res = client.readStringUntil('\n');  // read entire response
    Serial.print("[Rx] ");
    Serial.println(str_res);
    client.stop();
  }

}

void send_door_state()
{
  WiFiClient client;

  String send_data = "door,open\n\r";
  Serial.println(send_data);
  if (client.connect(host, port))
  {

    if (client.connected())
    {
      client.write(send_data.c_str());                      // send data
    }
    client.stop();
  }
}

void update_time()
{
  if (millis() - get_time > time_to_get_data)
  {
    get_time = millis();
    WiFiClient client;

    String send_data = "time,\n\r";
    Serial.println(send_data);
    if (client.connect(host, port))
    {
      client.write(send_data.c_str());

      while (!client.available());                // wait for response
      str_res = client.readStringUntil('\n');  // read entire response
      Serial.print("[Rx] ");
      Serial.println(str_res);
      if (str_res != "yes" && str_res != "no")
      {
        int time_app = str_res.toInt();
        door_open_time = time_app;
      }
      client.stop();
      str_res = "";
    }
  }
}
