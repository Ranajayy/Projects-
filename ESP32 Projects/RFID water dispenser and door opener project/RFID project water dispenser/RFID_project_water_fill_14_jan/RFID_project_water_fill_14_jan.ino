#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include<WiFi.h>
#include<Servo.h>


#define red_led 16
#define green_led 17
#define emergency_button 4

Servo myservo;

const char* ssid = "OCTATHORN"; // change wtih your  WIFI ssid here
const char* pass = "62543800";  // change with you WIFI pass here
const char * host = "192.168.18.9"; // change with you ip here

const uint16_t port = 4260;
int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
#define SS_PIN  5  // ESP32 pin GIOP5 
#define RST_PIN 27 // ESP32 pin GIOP27 

MFRC522 rfid(SS_PIN, RST_PIN);
#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns

char keys[ROW_NUM][COLUMN_NUM] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM]      = {32, 33, 25, 26};
byte pin_column[COLUMN_NUM] = {14, 12, 13, 15};
int pos = 0;
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
String rfid_tag = "";
String str_res = "";
String check_var = "";
String sel_gallons = "";
int scan_flag = 0;
String no_gallons = "";
unsigned long open_duration_n_gallons = 8250; // time to pour one gallon*
unsigned long servo_millis = 0;
int close_angle = 0;
int open_angle = 180;
unsigned long open_millis = 0;
bool open_flag = false;
unsigned long count_gallons_millis = 0;
unsigned long settings_fetch_time = 0;
bool rfid_scan_flag = true;
int cursorColumn = 0;
bool cf = false;
bool cf1 = false;

void setup()
{
  Serial.begin(115200);
  myservo.attach(2);
  SPI.begin();
  rfid.PCD_Init();
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan Your Card");
  lcd.setCursor(0, 1);
  lcd.print("to pour water");
  pinMode(red_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(emergency_button, INPUT);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("...");
  }

  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());
  digitalWrite(red_led, HIGH);
  digitalWrite(green_led, LOW);
  settings_fetch_time = millis();
}

void loop()
{
  char key = keypad.getKey();
  if (rfid_scan_flag == true)
  {

    read_rfid();

  }

    if (millis() - settings_fetch_time > 3000)
    {
      settings_fetch_time = millis();
      fetch_settings();
    }

  if (scan_flag == 1)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fetching info...");
    send_rfid();
    scan_flag = 0;
  }

  if (check_var == "a")
  {
    String customer_name = getValue(str_res, ';', 1);
    no_gallons = getValue(str_res, ';', 2);
    Serial.println(customer_name);
    Serial.println(no_gallons);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Name:" + customer_name);
    lcd.setCursor(0, 1);
    lcd.print("Gallons:" + no_gallons);
    check_var = "";
    str_res = "";
    cf1 = true;
  }
  if (check_var == "c")
  {
    String time_app = getValue(str_res, ';', 1);
    open_duration_n_gallons = atol(time_app.c_str());
    open_angle = getValue(str_res, ';', 2).toInt();
    Serial.print(F("got configuration!"));
    Serial.println(open_duration_n_gallons);
    check_var = "";
    str_res = "";
  }

  if (key && no_gallons != "")
  {

    Serial.println(key);
    if (key == '*')
    {
      Serial.println("no_of_gallons" + sel_gallons);
      int gallons_multiple = sel_gallons.toInt();
      int chk_gal = no_gallons.toInt();
      if (gallons_multiple > chk_gal || gallons_multiple <= 0)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Invalid Amount");
        lcd.setCursor(0, 1);
        lcd.print("Selected");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Scan Your Card");
        lcd.setCursor(0, 1);
        lcd.print("to pour water");
        rfid_tag = "";
        sel_gallons = "";
        no_gallons = "";
        rfid_scan_flag = true;
        digitalWrite(red_led, HIGH);
        digitalWrite(green_led, LOW);
      }
      else
      {
        delay(1000);
        count_gallons_millis = millis();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Pouring:" + sel_gallons + " GALS");
        open_millis = open_duration_n_gallons * gallons_multiple;
        Serial.println(open_millis);
        servo_millis = millis();
        open_flag = true;
        myservo.write(open_angle);
        digitalWrite(red_led, LOW);
        digitalWrite(green_led, HIGH );

      }
    }

    if (key != 'A' && key != 'B' && key != 'D' && key != '#' && key != '*')
    {
      if (cf1 == true)
      {
        cursorColumn = 0;
        lcd.clear();
        cf1 = false;
      }
      if (cf == true)
      {
        cursorColumn = 0;
        cf = false;
        lcd.clear();
      }
      lcd.setCursor(cursorColumn, 0);
      sel_gallons += key;
      lcd.print(key);
      cursorColumn++;
      if (cursorColumn == 16)
      {
        cursorColumn = 0;
      }
    }

    if ( key == 'C')
    {
      cf = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("0");
      sel_gallons = "";
    }
  }

  if (millis() - servo_millis > open_millis && open_flag == true)
  {
    int var = no_gallons.toInt();
    int var2 = sel_gallons.toInt();
    int remaining_gal = var - var2;
    open_flag = false;
    open_millis = 0;
    servo_millis = millis();
    myservo.write(close_angle);
    WiFiClient client;
    String send_data = "gallons," + String(sel_gallons) + "," + String(rfid_tag) + "," + String(remaining_gal) + "\n\r";
    Serial.println(send_data);
    if (client.connect(host, port))
    {
      if (client.connected())
      {
        client.write(send_data.c_str());                      // send data
      }
    }
    client.stop();
    sel_gallons = "";
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan Your Card");
    lcd.setCursor(0, 1);
    lcd.print("to pour water");
    rfid_scan_flag = true;
    rfid_tag = "";
    no_gallons = "";
    digitalWrite(red_led, HIGH);
    digitalWrite(green_led, LOW);
  }

  if (digitalRead(emergency_button) == 1)
  {
    delay(500);
    int var = no_gallons.toInt();

    unsigned long time_emergency = millis() - count_gallons_millis;
    Serial.println(time_emergency);
    int no_gallons_emergency = (time_emergency / open_duration_n_gallons);
    Serial.println(no_gallons_emergency);
    int remaining_gal = var - no_gallons_emergency;
    WiFiClient client;
    String send_data = "emergency," + String(no_gallons_emergency) + "," + String(rfid_tag) + "," + String(remaining_gal) + "\n\r";
    Serial.println(send_data);
    if (client.connect(host, port))
    {
      if (client.connected())
      {
        client.write(send_data.c_str());                      // send data
      }
    }
    client.stop();
    count_gallons_millis = 0;
    open_flag = false;
    open_millis = 0;
    servo_millis = millis();
    myservo.write(close_angle);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Emergency Stop");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan Your Card");
    lcd.setCursor(0, 1);
    lcd.print("to pour water");
    rfid_scan_flag = true;
    rfid_tag = "";
    no_gallons = "";
    sel_gallons = "";
    digitalWrite(red_led, HIGH);
    digitalWrite(green_led, LOW);
  }


}

void read_rfid()
{
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
      // Serial.println(rfid_tag);
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
    scan_flag = 1;
    rfid_scan_flag = false;
  }
}
void send_rfid()
{
  WiFiClient client;

  String send_data = "scan," + rfid_tag + "\n\r";
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
    check_var = getValue(str_res, ';', 0);
    client.stop();
  }

}

void fetch_settings()
{
  WiFiClient client;

  String send_data = "configure,\n\r";
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
    check_var = getValue(str_res, ';', 0);
    client.stop();
  }

}
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
