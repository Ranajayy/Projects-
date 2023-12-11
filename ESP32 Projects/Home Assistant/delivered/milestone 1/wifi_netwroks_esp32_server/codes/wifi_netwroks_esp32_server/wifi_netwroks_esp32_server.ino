#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

Preferences preferences;


const char* apSSID = "CALERO";
const char* apIP = "192.168.99.1";
String s_ssid = "";
String s_pass = "";

AsyncWebServer server(80);

bool scanButtonPressed = false;
bool wifi_flag = false;

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200);

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

  wifi_not_connected_screen();

  String ssid, pass = "";

  preferences.begin("data", false);
  ssid = preferences.getString("ssid", "");
  pass = preferences.getString("password", "");

  if (ssid == "" || pass == "")
  {
    Serial.println("No Prevoiusly stored Wifi Network");
    ap_init();
    run_server();
  }

  else
  {
    s_ssid = ssid;
    s_pass = pass;
    wifi_init(ssid, pass);
    wifi_connected_Screen(ssid);
  }

}

void loop()
{
  if (WiFi.status() == WL_CONNECTED && wifi_flag == false)
  {
    wifi_connected_Screen(s_ssid);
    check_wifi();
  }
}



void check_wifi()
{
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.softAPdisconnect(true);
  preferences.begin("data", false);
  preferences.putString("ssid", s_ssid);
  preferences.putString("password", s_pass);
  Serial.println("Network Credentials Saved using Preferences");
  preferences.end();
  wifi_flag = true;
}


void wifi_init(String ssid, String password)
{
  unsigned long wifi_m = millis();
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED && millis() - wifi_m < 10000)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    preferences.begin("data", false);
    preferences.putString("ssid", "");
    preferences.putString("password", "");
    Serial.println("Network Credentials Saved using Preferences");
    preferences.end();
    ESP.restart();
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
    String html = "<html><head><style>";
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
    String response = "Connecting to WiFi network: " + connectSSID;
    request->send(200, "text/plain", response);
    wifi_init(s_ssid, s_pass);

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
  Serial2.print("page page0");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);

  Serial2.print("page0.p1.pic=");
  Serial2.print("0");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}
