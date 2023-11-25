#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <EthernetENC.h>
#include <Arduino.h>
#include <OneButton.h>
#include <WebServer.h>
#include <Preferences.h>
#include <esp_task_wdt.h>


// Function prototypes
void handleRoot();
void handleForm();
void streamCallback(StreamData data);
void streamTimeoutCallback(bool timeout);
void setup();
void loop();
void fire_base_init_eth();
void fire_base_init_wifi();
static void handleClick();
void eth_init();
void ultra_sensor();
void wifi_connect();
void reinitalize_connect();

Preferences settings;
IPAddress local_ip(192, 168, 1, 4);
IPAddress gateway(192, 168, 1, 4);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80); // Server on port 80
bool server_on = false;

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<h2>QR DOOR MAN<h2>
<h3>WIFI SETUP PAGE</h3>

<form action="/action_page">
  WIFI SSID:<br>
  <input type="text" name="SSID" value="">
  <br>
  WIFI PASS:<br>
  <input type="text" name="PASS" value="">
  <br>
  <br>
  <input type="submit" value="Submit">
</form>

</body>
</html>
)=====";

void handleRoot()
{
  String s = MAIN_page;             // Read HTML contents
  server.send(200, "text/html", s); // Send web page
}

void handleForm()
{

  String ssid = server.arg("SSID");
  String pass = server.arg("PASS");

  Serial.print("WIFI SSID:");
  Serial.println(ssid);

  Serial.print("WIFI PASS:");
  Serial.println(pass);


  settings.begin("credentials", false);
  settings.putString("ssid", ssid);
  settings.putString("password", pass);
  Serial.println("Network Credentials Saved using Preferences");
  settings.end();
  ESP.restart();
  wifi_connect();
  String s = "<a href='/'> Go Back </a>";
  server.send(200, "text/html", s);
}

#define server_btn 15
#define light_pin 21
#define aux 22
#define door_pin 26
#define eth_wifi_switch 27


unsigned long open_millis = 0;
bool o_f = false;
String WIFI_SSID, WIFI_PASS;


#define API_KEY "AIzaSyBAQmbCuQMsbg58_MU7eOPJVc2xlO-e348"
#define DATABASE_URL "https://qrdoorman-ebf21-default-rtdb.firebaseio.com"
#define USER_EMAIL "uzair123@gmail.com"
#define USER_PASSWORD "12345678"

#define WIZNET_RESET_PIN 26 // Connect W5500 Reset pin to GPIO 26 of ESP32
#define WIZNET_CS_PIN 5     // Connect W5500 CS pin to GPIO 5 of ESP32
#define WIZNET_MISO_PIN 19  // Connect W5500 MISO pin to GPIO 19 of ESP32
#define WIZNET_MOSI_PIN 23  // Connect W5500 MOSI pin to GPIO 23 of ESP32
#define WIZNET_SCLK_PIN 18  // Connect W5500 SCLK pin to GPIO 18 of ESP32

// Define Firebase Data object
FirebaseData stream;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

int count = 0;

String path = "";

uint8_t Eth_MAC[] = {0x02, 0xF0, 0x0D, 0xBE, 0xEF, 0x01};

EthernetClient client;
EthernetClient client2;

unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;

OneButton btn = OneButton(server_btn, false, false );

unsigned char data[4] = {};
float distance;
bool SF = false;
bool object_pf=true;
bool object_rf=true;
float d_thresh = 100;
int et_wi_state = 0, prev_state = 0;
unsigned long previousMillis = millis();  // Store the last execution time
const long interval = 500;

void streamCallback(StreamData data)
{
  Serial.printf("stream path: %s\n", data.streamPath().c_str());
  Serial.printf("event path: %s\n", data.dataPath().c_str());
  Serial.printf("payload: %s\n\n", data.stringData().c_str());
  path = String(data.streamPath());
  String path2 = String(data.dataPath());
  String dat = data.stringData();
  
  if(path2=="/LightValue")
  {
    if(dat=="1")
    {
      digitalWrite(light_pin,LOW);
    }
    else if(dat=="0")
    {
      digitalWrite(light_pin,HIGH);
    }
  }
  if(path2=="/door")
  {
    if(dat=="1")
    {
      digitalWrite(door_pin,LOW);
      o_f=true;
      open_millis=millis();
    }
  }


  Serial.println();
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void setup()
{

  Serial.begin(115200);
  Serial2.begin(9600);

  esp_task_wdt_init(20, true);
  esp_task_wdt_add(NULL);  // Add the current task to the watchdog

  pinMode(door_pin, OUTPUT);
  pinMode(aux, OUTPUT);
  pinMode(light_pin, OUTPUT);

  digitalWrite(light_pin,HIGH);
  digitalWrite(door_pin, HIGH);
  digitalWrite(aux, HIGH);

  pinMode(WIZNET_RESET_PIN, OUTPUT);
  pinMode(eth_wifi_switch, INPUT);
  btn.attachClick(handleClick);
  Serial_Printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  et_wi_state = digitalRead(eth_wifi_switch);

  if (digitalRead(eth_wifi_switch) == 1)
  {
    eth_init();
    fire_base_init_eth();
    esp_task_wdt_reset();
    previousMillis = millis();
  }
  else
  {
    settings.begin("credentials", false);
    WIFI_SSID = settings.getString("ssid", "");
    WIFI_PASS = settings.getString("password", "");

    if (WIFI_SSID == "" || WIFI_PASS == "")
    {
      SF = true;
    }
    else
    {
      wifi_connect();
      esp_task_wdt_reset();
      previousMillis = millis();
    }
  }


}

void loop()
{
  esp_task_wdt_reset();
  ultra_sensor();
  server.handleClient();
  btn.tick();


  if (digitalRead(eth_wifi_switch) != et_wi_state)
  {
    Serial.println("RESTARTING");
    delay(2000);
    ESP.restart();
  }

  if (SF)
  {
    server_on = true;
    SF = false;

    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAP("esp32", "123456789");
    WiFi.softAPConfig(local_ip, gateway, subnet);
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println("WiFi");
    Serial.print("Access IP address: ");
    Serial.println(local_ip);
    Serial.print(" to set WIFI");
    server.on("/", handleRoot);
    server.on("/action_page", handleForm);
    server.begin();
    Serial.println("HTTP server started");
  }


  if (server_on == false)
  {
      if (Firebase.isTokenExpired()&& Firebase.ready())
    {
     
      Firebase.refreshToken(&config);
      Serial.println("Refresh token");
    }
    
    if (o_f == true)
    {
      if (millis() - open_millis > 5000)
      {

        path = path + "/door";
        open_millis = millis();
        digitalWrite(door_pin, HIGH);
        FirebaseJson json;
      if (Firebase.set(fbdo, String(path), "0"))
    {
      Serial.println("Set operation was successful (ok)");

    }
    else
    {
      Serial.print("Set operation failed. Error: ");
      Serial.println(fbdo.errorReason().c_str());
    }
        o_f = false;
      }
    }

  }
  if (int(distance) < 30 && distance != 0 && object_pf==true)
  {
    String obj_path = "/commercial/commercialmHJ6ZFYmFPWo2G389KV02i05nEf1/isObject";
    digitalWrite(aux, HIGH);
    Serial.println("in distance code!");
    FirebaseJson json;
    if (Firebase.set(fbdo, String(obj_path), "1"))
    {
      Serial.println("Set operation was successful (ok)");
      object_pf=false;
      object_rf=true;
    }
    else
    {
      Serial.print("Set operation failed. Error: ");
      Serial.println(fbdo.errorReason().c_str());
      // WiFi.disconnect();
      // reinitalize_connect();
    }

    delay(2000);
  }

  else
  {
    digitalWrite(aux, LOW);
    String obj_path = "/commercial/commercialmHJ6ZFYmFPWo2G389KV02i05nEf1/isObject";
    if(object_rf==true && int(distance)>30)
    {
      FirebaseJson json;
    if (Firebase.set(fbdo, String(obj_path), "0"))
    {
      Serial.println("Set operation was successful (ok)");
      object_rf=false;
      object_pf=true;
      delay(2000);
    }
    else
    {
      Serial.print("Set operation failed. Error: ");
      Serial.println(fbdo.errorReason().c_str());
      // WiFi.disconnect();
      // reinitalize_connect();
    }

  }
  }

}



void fire_base_init_eth()
{
  config.api_key = API_KEY;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;
  fbdo.setEthernetClient(&client, Eth_MAC, WIZNET_CS_PIN, WIZNET_RESET_PIN);   // The staIP can be assigned to the fifth param
  stream.setEthernetClient(&client2, Eth_MAC, WIZNET_CS_PIN, WIZNET_RESET_PIN); // The staIP can be assigned to the fifth param
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
  stream.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
  Firebase.setDoubleDigits(5);
  //config.signer.preRefreshSeconds = 3600 - 60; this will make firebase refresh token after 1 min
  Firebase.begin(&config, &auth);

  if (!Firebase.beginStream(stream, "/commercial/commercialmHJ6ZFYmFPWo2G389KV02i05nEf1"))
    Serial_Printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.setStreamCallback(stream, streamCallback, streamTimeoutCallback);

}


void fire_base_init_wifi()
{
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
 // config.signer.preRefreshSeconds = 3600 - 60; make firebase refresh token after 1 min
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  if (!Firebase.beginStream(stream, "/commercial/commercialmHJ6ZFYmFPWo2G389KV02i05nEf1")) // url to get data from
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.setStreamCallback(stream, streamCallback, streamTimeoutCallback);
}




static void handleClick()
{
  SF = true;
  Serial.println("Server on!");
}


void eth_init()
{
  Serial.println("Begin Ethernet");

  Ethernet.init(5);   // MKR ETH Shield

  if (Ethernet.begin(Eth_MAC)) { // Dynamic IP setup
    Serial.println("DHCP OK!");
  } else {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
  }
  delay(3000);


  Serial.print("Local IP : ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet Mask : ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Gateway IP : ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS Server : ");
  Serial.println(Ethernet.dnsServerIP());

  Serial.println("Ethernet Successfully Initialized");
}

// void ultra_sensor()
// {
//   // Read data from Serial2
//   do
//   {
//     for (int i = 0; i < 4; i++)
//     {
//       while (Serial2.available() == 0)
//       {
//         // Wait for data to be available in the buffer
//       }
//       data[i] = Serial2.read();
//     }
//   } while (data[0] != 0xff);

//   Serial2.flush();

//   if (data[0] == 0xff)
//   {
//     int sum;
//     sum = (data[0] + data[1] + data[2]) & 0x00FF;
//     if (sum == data[3])
//     {
//       distance = (data[1] << 8) + data[2];
//       distance = distance / 10.0; // Use float division for accuracy
//       Serial.print("distance=");
//       Serial.print(distance);
//       Serial.println("cm");
//     }
//     else
//     {
//       Serial.println("ERROR: Checksum mismatch");
//       ESP.restart();
//     }
//   }
// }




void ultra_sensor()
{
 unsigned long currentMillis = millis();  // Get the current time

  // Check if it's time to execute the loop
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;  // Save the current time

    do {
      for (int i = 0; i < 4; i++)
      {
        data[i] = Serial2.read();
      }
    } while (Serial2.read() == 0xff);

    Serial2.flush();

    if (data[0] == 0xff)
    {
      int sum;
      sum = (data[0] + data[1] + data[2]) & 0x00FF;
      if (sum == data[3])
      {
        distance = (data[1] << 8) + data[2];
        distance=distance/10;
        Serial.print("distance=");
        Serial.print(distance);
        Serial.println("cm");

      }
      else
      {
        Serial.println("ERROR");
      }
    }

  }
}


void wifi_connect()
{
  int a = 0;
  settings.begin("credentials", false);

  WIFI_SSID = settings.getString("ssid", "");
  WIFI_PASS = settings.getString("password", "");
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
    a++;
    if (a > 10)
    {
      break;
    }
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("no  wifi try again!");
    SF = true;
    server_on = true;
  }

  else
  {
    SF = false;
    server_on = false;
    Serial.println("WIFI connected!");
    fire_base_init_wifi();
  }

}

void reinitalize_connect()
{
  wifi_connect();
}