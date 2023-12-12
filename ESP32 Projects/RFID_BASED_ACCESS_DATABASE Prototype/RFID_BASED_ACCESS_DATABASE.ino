#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "FS.h"
#include "SD.h"
#include<SPI.h>
#include <MFRC522.h>


const char* ssid     =  "Octathorn";     // your network SSID (name of wifi network)
const char* password = "62543800"; // your network password

#define SD_CLK_PIN       14
#define SD_MOSI_PIN      13
#define SD_MISO_PIN     27
#define SD_CS_PIN        15    //15  // or 5
#define SS_PIN  21
#define RST_PIN 22
#define buzz_pin  26
#define led_pin 25
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;


SPIClass SPISD(HSPI);
File uploadFile;
unsigned long a = 0;
String url = "http://192.168.18.57:5000/download";  // change your url here
MFRC522 rfid(SS_PIN, RST_PIN);

String uid = "";
int b = 1;
int card = 0;
sqlite3 *db1;
int rc;
int count = 0;
unsigned long sync_millis, wifi_millis = 0;
int rfid_count = 0;




const char* data = "Callback function called";
static int callback(void *data, int argc, char **argv, char **azColName)
{
  int i;
  Serial.printf("%s: ", (const char*)data);

  if (*argv[0] == '0')
  {
    digitalWrite(buzz_pin, HIGH);
    delay(200);
    digitalWrite(buzz_pin, LOW);
    delay(200);
    digitalWrite(buzz_pin, HIGH);
    delay(200);
    digitalWrite(buzz_pin, LOW);
    // Serial.println("no tag found");

  }
  else
  {
    digitalWrite(buzz_pin, HIGH);
    delay(300);
    digitalWrite(buzz_pin, LOW);
    //Serial.println("tag found");
    rfid_count++;

  }
  for (i = 0; i < argc; i++)
  {
    Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

  }
  Serial.printf("\n");
  return 0;
}


int openDb(const char *filename, sqlite3 **db) {
  int rc = sqlite3_open(filename, db);
  if (rc) {
    Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
    return rc;
  } else {
    Serial.printf("Opened database successfully\n");
  }
  return rc;
}

char *zErrMsg = 0;
int db_exec(sqlite3 *db, const char *sql) {
  Serial.println(sql);
  long start = micros();
  int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
  if (rc != SQLITE_OK) {
    Serial.printf("SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    Serial.printf("Operation done successfully\n");
  }
  Serial.print(F("Time taken:"));
  Serial.println(micros() - start);
  return rc;
}
void setup()
{
  Serial.begin(115200);
  pinMode(buzz_pin, OUTPUT);
  pinMode(led_pin, OUTPUT);


  SPI.begin();
  rfid.PCD_Init();


  SPISD.begin(SD_CLK_PIN, SD_MISO_PIN, SD_MOSI_PIN);

  if (!SD.begin(SD_CS_PIN, SPISD))
  {
    Serial.println("Card Mount Failed");
    while (1);
  }
  Serial.println("initialization done.");
  sqlite3_initialize();


  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    count++;
    Serial.println("Connecting to WiFi..");
    if (count >= 5)
    {
      break;
    }
  }
  Serial.println("Connected to the WiFi network");

  sync_rfid();
  sync_millis = millis();

}



void loop()
{

  if (millis() - sync_millis >= 60000)
  {

    sync_rfid();
    b = 0;
    sync_millis = millis();
  }

  if (rfid.PICC_IsNewCardPresent())
  {

    if (rfid.PICC_ReadCardSerial())
    {
      card = 1;
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      char str[32] = "";
      array_to_string(rfid.uid.uidByte, 4, str); //Insert (byte array, length, char array for output)
      Serial.println(str); //Print the output uid string

      uid = str;

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
  }

  if (card == 1)
  {
    if (openDb("/sd/data.db", &db1))
      return;
    String q = "SELECT EXISTS(SELECT 1 FROM total_data_csv WHERE rfid_tag ='" + uid + "')";
    // String q = "INSERT INTO rfid(RFID_TAG) VALUES('" + uid + "')";
    const char *cmp_query = q.c_str();
    rc = db_exec(db1, cmp_query );
    sqlite3_close(db1);
    card = 0;
  }

  uid = "";

  //  if ((WiFi.status() != WL_CONNECTED) && millis()- wifi_millis >= 30000)
  //  {
  //    count = 0;
  //    WiFi.disconnect(true);
  //    WiFi.mode(WIFI_STA);
  //    WiFi.begin(ssid, password);
  //
  //    while (WiFi.status() != WL_CONNECTED) {
  //      delay(1000);
  //      count++;
  //      Serial.println("Connecting to WiFi..");
  //      if (count == 5)
  //      {
  //        wifi_millis = millis();
  //        break;
  //      }
  //    }
  //
  //    Serial.println("Connected to the WiFi network");
  //  }

  send_counter();


}


void sync_rfid()
{

  Serial.print("[HTTP] begin...\n");
  if ((WiFi.status() == WL_CONNECTED))
  {
    HTTPClient Http;
    Http.begin(url);
    Serial.print("[HTTP] GET...\n");
    int httpCode = Http.GET();
    if (httpCode > 0)
    {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK)
      {
        digitalWrite(led_pin, HIGH);
        uploadFile = SD.open("/data.db", FILE_WRITE);

        int len = Http.getSize();
        Serial.printf("Payload size [%d] bytes.\r\n", len);
        uint8_t buff[2048] = {0};
        WiFiClient *stream = Http.getStreamPtr();
        a = millis();
        while (Http.connected() && (len > 0 || len == -1))
        {

          size_t size = stream->available();
          if (size)
          { // read up to 128 byte
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
            // write it to Serial
            Serial.printf("%d bytes read[c].\r\n", c);
            Serial.printf("%d bytes available for read \r\n", size);
            // open file in append mode.
            uploadFile.write(buff, c);
            // close file.
            if (len > 0)
            {
              len -= c;
            }
          }
          //delayMicroseconds(1);
        }
        Serial.println();
        Serial.print("[HTTP] connection closed or file end.\n");
        uploadFile.close();
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", Http.errorToString(httpCode).c_str());
    }
    digitalWrite(led_pin, LOW);

    Http.end();
    Serial.println(millis() - a);
  }
}


void array_to_string(byte array[], unsigned int len, char buffer[])
{
  for (unsigned int i = 0; i < len; i++)
  {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i * 2 + 1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len * 2] = '\0';
}

void send_counter()
{
  const char* serverName = "http://192.168.18.88:8080/user/add";
  if ((millis() - lastTime) > timerDelay)
  {

    if (WiFi.status() == WL_CONNECTED)
    {
      WiFiClient client;
      HTTPClient http;

      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
     String httpRequestData = String(rfid_count);
      // http.addHeader("Content-Type", "text/plain");
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode =  http.POST("{\"rfid\":\""+String(rfid_count)+"\"}");

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
