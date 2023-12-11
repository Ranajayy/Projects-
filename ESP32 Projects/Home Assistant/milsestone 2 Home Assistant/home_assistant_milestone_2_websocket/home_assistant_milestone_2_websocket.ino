#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WebSocketsClient.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "soc/rtc_wdt.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"


WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
bool isconnected = false;
#define USE_SERIAL Serial

Preferences preferences;

const char* apSSID = "CALERO";
const char* apIP = "192.168.99.1";
String s_ssid = "";
String s_pass = "";
String s_name = "";
String s_ip = "";
String s_token = "";

AsyncWebServer server(80);

bool scanButtonPressed = false;
bool wifi_flag = false;
String f_ip = "";
String default_h_name = "homeassistant";

void setup()
{


  Serial.begin(115200);
  Serial2.begin(115200);

  TIMERG1.wdt_wprotect = TIMG_WDT_WKEY_VALUE; // Unlock timer config.
  TIMERG1.wdt_feed = 1; // Reset feed count.
  TIMERG1.wdt_config0.en = 0; // Disable timer.
  TIMERG1.wdt_wprotect = 0; // Lock timer config.

  TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed = 1;
  TIMERG0.wdt_config0.en = 0;
  TIMERG0.wdt_wprotect = 0;

  Serial2.write(255);
  Serial2.write(255);
  Serial2.write(255);
  Serial2.print("bkcmd=0"); // 0 disables errors, 2 enables errors
  Serial2.write(255);
  Serial2.write(255);
  Serial2.write(255);

  Serial2.write(255);
  Serial2.write(255);
  Serial2.write(255);
  Serial2.print("bkcmd=0"); // 0 disables errors, 2 enables errors
  Serial2.write(255);
  Serial2.write(255);
  Serial2.write(255);

  loading_screen();

  delay(3000);
  WiFi.mode(WIFI_AP_STA);



  String ssid, pass, ip, token, names = "";

  preferences.begin("data", false);
  ssid = preferences.getString("ssid", "");
  pass = preferences.getString("password", "");
  ip = preferences.getString("ip", "");
  token = preferences.getString("token", "");
  names = preferences.getString("name", "");

  if (ssid == "" || pass == "")
  {
    Serial.println("No Prevoiusly stored Wifi Network");
    wifi_not_connected_screen();
    ap_init();
    run_server();
  }

  else
  {
    s_ssid = ssid;
    s_pass = pass;
    s_ip = ip;
    s_token = token;
    s_name = names;
    ap_init();
    run_server();
    wifi_init(ssid, pass);
    wifi_connected_Screen(ssid);

  }

}

void loop()
{

  webSocket.loop();

  if (WiFi.status() == WL_CONNECTED && wifi_flag == false)
  {
    wifi_connected_Screen(s_ssid);
    check_wifi();
    connect_web_sock(s_ip, s_token, s_name);
  }
}



void check_wifi()
{
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());
  // WiFi.softAPdisconnect(true); dont dissconnect ap as we need to scan for home assistant
  preferences.begin("data", false);
  preferences.putString("ssid", s_ssid);
  preferences.putString("password", s_pass);
  Serial.println("Network Credentials Saved using Preferences");
  preferences.end();
  wifi_flag = true;
}


void wifi_init(String ssid, String password)
{
  int c = 0;
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
    c++;
    if (c > 9)
    {
      break;
    }
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    preferences.begin("data", false);
    preferences.putString("ssid", "");
    preferences.putString("password", "");
    Serial.println("Network Credentials Saved using Preferences");
    preferences.end();
  }
  else
  {
    Serial.println("Conencted!");
  }

}


void ap_init()
{
  // Connect to Wi-Fi as an access point
  WiFi.softAPConfig(IPAddress(192, 168, 99, 1), IPAddress(192, 168, 99, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);

  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ip);
}

void run_server()
{
  unsigned long wifi_m = millis();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f2f2f2; }";
    html += "h1 { text-align: center; margin-bottom: 20px; }";
    html += "form { background-color: #fff; padding: 20px; border-radius: 5px; box-shadow: 0 0 10px rgba(0,0,0,0.2); }";
    html += "input[type='text'], input[type='password'], select { width: 100%; padding: 10px; margin-bottom: 10px; border: 1px solid #ccc; border-radius: 3px; }";
    html += "input[type='submit'] { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 3px; cursor: pointer; }";
    html += "</style></head><body>";
    html += "<h1>ESP32 Web Server</h1>";
    html += "<form action='/submit' method='POST'>";
    html += "<label for='wifiNetwork'>WiFi Network:</label><br>";
    html += "<select name='wifiNetwork'>";
    html += "<option value=''>-- Select SSID --</option>";
    if (scanButtonPressed)
    {
      html += getWiFiNetworkOptions();
    }
    html += "</select><br>";
    html += "<label for='manualSSID'>Manual SSID:</label><br>";
    html += "<input type='text' name='manualSSID'><br>";
    html += "<label for='wifiPassword'>Password:</label><br>";
    html += "<input type='password' name='wifiPassword'><br>";
    html += "<input type='submit' value='Connect'>";
    html += "</form>";
    html += "<form action='/scan' method='GET'>";
    html += "<input type='submit' name='scanButton' value='Scan'>";
    html += "</form>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  // Route to handle form submission
  server.on("/submit", HTTP_POST, [](AsyncWebServerRequest * request) {
    String selectedWiFi = request->arg("wifiNetwork");
    String manualSSID = request->arg("manualSSID");
    String wifiPassword = request->arg("wifiPassword");
    String connectSSID;

    if (!manualSSID.isEmpty())
    {
      connectSSID = manualSSID;
    }
    else if (!selectedWiFi.isEmpty())
    {
      connectSSID = selectedWiFi;
    }

    s_ssid = connectSSID;
    s_pass = wifiPassword;

    wifi_init(s_ssid, s_pass);

    if (WiFi.status() == WL_CONNECTED)
    {

      home_assis_setup();
      String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
      html += "<title>Home Assistant</title></head><body>";
      html += "<h1>Authentication Page</h1>";
      html += "<form action='/set-auth-token' method='POST'>";
      html += "<label for='HomeAssistant'>ASSISTANT INSTANCE:</label><br>";
      html += "<select name='homeASSISTANT'>";
      if (check_home_assistant(default_h_name))
      {
        "<option value=''>" + f_ip + "</option>";
        html += "<option value='" + f_ip + "'>" + f_ip + "</option>";

      }
      else
      {
        html += "<option value='' disabled>No Instance Found</option>";
      }
      html += "</select><br>";
      html += "HOME ASSISTANT NAME: <input type='text' name='name'><br>";
      html += "HOME ASSISTANT IP: <input type='text' name='ip'><br>";
      html += "HOME ASSISTANT TOKEN: <input type='text' name='token'><br>";
      html += "<input type='submit' value='Submit'>";
      html += "</form>";
      html += "</body></html>";
      request->send(200, "text/html", html);
    }

    else
    {
      String html = "NOT CONNECTED TO WIFI PLEASE GO BACK AND TRY AGAIN!";
      request->send(200, "text/plain", html);
    }
  });

  server.on("/set-auth-token", HTTP_POST, [](AsyncWebServerRequest * request) {
    String assistant_name = request->arg("name");
    String assitant_ip = request->arg("ip");
    String token_assis = request->arg("token");
    s_token = token_assis;

    Serial.println(assistant_name);
    Serial.println(assitant_ip);
    Serial.println(token_assis);

    preferences.begin("data", false);
    preferences.putString("name", assistant_name);
    preferences.putString("token", token_assis);
    preferences.putString("ip", assitant_ip);
    Serial.println("assistant Credentials Saved using Preferences");
    preferences.end();

    connect_web_sock(assitant_ip, token_assis, assistant_name);
  });

  // Route to handle scan button press
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest * request) {
    scanButtonPressed = true;
    request->redirect("/");
  });

  server.begin();
}


String getWiFiNetworkOptions()
{

  String options = "";
  int numNetworks = WiFi.scanNetworks();
  for (int i = 0; i < numNetworks; i++)
  {
    options += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";

  }
  return options;
}

void wifi_connected_Screen(String ssid)
{
  Serial2.print("page page1");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);

  Serial2.print("page page1");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);

  Serial2.print("page1.t0.txt=");
  Serial2.print("\"");
  Serial2.print("Conencted to " + String(ssid));
  Serial2.print("\"");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);

  Serial2.print("page1.p0.pic=");
  Serial2.print("0");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);

}

void wifi_not_connected_screen()
{
  Serial2.print("page page0");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
  //  Serial2.print("page page0");
  //  Serial2.write(0xff);
  //  Serial2.write(0xff);
  //  Serial2.write(0xff);

  Serial2.print("page0.p1.pic=");
  Serial2.print("0");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}

bool check_home_assistant(String serverHostname)
{
  String h_ip = "";
  f_ip = "";
  int cn = 0;
  if (!MDNS.begin("esp32"))
  {
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
      cn++;
      if (cn > 5)
      {
        break;
      }
    }
  }

  Serial.println("mDNS responder started");
  Serial.println("name to check is: " + serverHostname);
  IPAddress serverIp = MDNS.queryHost(serverHostname.c_str());
  Serial.print("IP address of server: ");
  Serial.println(serverIp.toString());
  h_ip = serverIp.toString();

  if (h_ip != "" && h_ip != "0.0.0.0")
  {
    f_ip = h_ip;
    return true;
  }

  else
  {
    return false;
  }
}

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
  const uint8_t* src = (const uint8_t*) mem;
  USE_SERIAL.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for (uint32_t i = 0; i < len; i++) {
    if (i % cols == 0) {
      USE_SERIAL.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    USE_SERIAL.printf("%02X ", *src);
    src++;
  }
  USE_SERIAL.printf("\n");
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  DeserializationError jsonError;
  DynamicJsonDocument jsonDoc(512); // Adjust buffer size as needed

  switch (type) {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[WSc] Disconnected!\n");
      isconnected = false;
      break;
    case WStype_CONNECTED:
      USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
      break;
    case WStype_TEXT:
      USE_SERIAL.printf("[WSc] get text: %s\n", payload);

      // Parse the incoming JSON payload
      jsonError = deserializeJson(jsonDoc, payload);

      if (jsonError)
      {
        USE_SERIAL.println("JSON parsing error!");
        return;
      }

      // Check if the server is requesting authentication
      if (jsonDoc.containsKey("type") && jsonDoc["type"].as<String>() == "auth_required")
      {

        // Prepare the authentication payload in JSON format
        DynamicJsonDocument authJson(256);
        authJson["type"] = "auth";
        authJson["access_token"] = s_token;

        String authPayload;
        serializeJson(authJson, authPayload);

        // Send the authentication payload via WebSocket
        webSocket.sendTXT(authPayload);
        Serial.println("auth token sent");
      }

      if (jsonDoc.containsKey("type") && jsonDoc["type"].as<String>() == "auth_ok")
      {
        home_assis();
      }

      break;
    case WStype_BIN:
      // ...
      break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      // ...
      break;
  }
}

void loading_screen()
{
  Serial2.print("page page2");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.print("page page2");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}

void home_assis()
{
  Serial2.print("page page4");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.print("page page4");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}


void home_assis_setup()
{
  Serial2.print("page page3");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.print("page page3");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}

void connect_web_sock(String ip, String token, String namez)
{
  if (!ip.isEmpty() && !token.isEmpty())
  {

    Serial.println("connecting to home assistant web socket");

    webSocket.begin(ip, 8123, "/api/websocket");
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
  }

  else if (!namez.isEmpty() && !token.isEmpty())
  {
    bool avail = check_home_assistant(namez);
    if (avail)
    {
      Serial.println("home assistant found!");

      webSocket.begin(f_ip, 8123, "/api/websocket");
      webSocket.onEvent(webSocketEvent);
      webSocket.setReconnectInterval(5000);
    }
    else
    {
      Serial.println("failed");
      home_assis_setup();
    }
  }
}
