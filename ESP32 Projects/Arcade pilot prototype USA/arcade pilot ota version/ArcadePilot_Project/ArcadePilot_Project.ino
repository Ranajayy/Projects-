#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <FastLED.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <OneButton.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


#define buzz 18
#define reset_btn 19
#define cp_pin 15
#define tick1_out 21
#define tick2_out 23

OneButton btn = OneButton(reset_btn, false, false);

Preferences preferences;

#define NUM_LEDS 15       // Number of LEDs in your strip
#define DATA_PIN 5        // Pin connected to the data input of the LED strip
#define BRIGHTNESS 200    // Maximum brightness (0-255)

CRGB leds[NUM_LEDS];
#define COLOR_LIGHT_BLUE CRGB(0, 100, 255)  // Light blue color

unsigned long lastUpdateTime = 0;
unsigned int pulseDuration = 2000;  // Pulsate every 2 seconds
float pulseFactor = 0.0;  // Pulsating factor (0.0 to 1.0)
bool increasing = true;


//---- MQTT Broker settings
const char* mqtt_server = "db2be04733cd410ba6554e0a21bd9c1c.s2.eu.hivemq.cloud"; // replace with your broker url
const char* mqtt_username = "ranaj"; // replace with your Credential
const char* mqtt_password = "pIoneer090";
const int mqtt_port = 8883;
//===========================

WiFiClientSecure espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

String topic;
String messageTemp;
String message;
String isl, cpt, cpd, tpt, tpl, tph, rfid_tag, tp_thresh = "";
String d_id = "";
String game_id = "";
String play_cost = "";
String url = "";
String balance_avail = "";
String ssid, pass = "";

bool configured = false;
bool first_time = true;
bool reset_flag = false;
bool live_screen = true;
bool isPulseHigh = false;
bool url_screen = false;
bool game_won = false;
bool sendPassword = false; // Flag to indicate whether to send the password command
bool sendSSID = false;    // Flag to indicate whether to send the SSID command
bool s_p = false;
bool sendURL = false;

unsigned long wifiConnectStartTime = 0;
unsigned long pulseStartTime = 0;
unsigned long pulsetime = 0;
unsigned long pulseCount = 0;
unsigned long  ping_time = 0;
const unsigned long wifiConnectTimeout = 10000;  // 10 seconds timeout


int reader_mode = 0;
int a = 0;
int rs = 0;
int tick1, tick2 = 0;
int tickets = 0;
int tick1_thresh, tick2_thresh = 0;
const int voltagePin = 33;  // Pin connected to the voltage divider output

const float R1 = 10000.0;   // Resistor R1 value in ohms (10 kΩ)
const float R2 = 2700.0;    // Resistor R2 value in ohms (2.7 kΩ)
float voltage = 0;
unsigned long last_ping = 0;

void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600);

  nextion_startup();
  pins();
  check_startup();

  if (first_time == false)
  {

    String s_ssid, s_pass = "";

    preferences.begin("data", false);
    tpt = preferences.getString("tpt", "");
    tph = preferences.getString("tph", "");
    tpl = preferences.getString("tpl", "");
    cpt = preferences.getString("cpt", "");
    cpd = preferences.getString("tpd", "");
    s_ssid = preferences.getString("ssid", "");
    s_pass = preferences.getString("password", "");
    game_id = preferences.getString("game_id", "");
    url = preferences.getString("url", "");
    tp_thresh = preferences.getString("tp_thresh", "");
    tick1_thresh = tp_thresh.toInt();
    tick2_thresh = tp_thresh.toInt();

    //Serial.println(ssid);
    //Serial.println(pass);

    if (s_ssid == "" || s_pass == "")
    {
      //Serial.println("No values saved for ssid or password");
    }
    else
    {
      wifi_init(s_ssid, s_pass);
    }
  }

    else
    {
      rfid_init();
    }


  last_ping = millis();

  
}


void loop()
{

  btn.tick();
  battery();

  if (reset_flag == true)
  {
    reset_flag = false;
    reset_mode();
  }

  if (first_time == true)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      unsigned long currentTime = millis();
      if (currentTime - lastUpdateTime >= pulseDuration)
      {
        lastUpdateTime = currentTime;
        increasing = !increasing; // Toggle the direction
      }
      updatePulsateEffect();
      rfid_setup();

    }

    if (WiFi.status() == WL_CONNECTED)
    {
      if (url == "")
      {
        if (url_screen)
        {
          url_screen = false;
          Serial.print("page page4");
          Serial.write(0xff);
          Serial.write(0xff);
          Serial.write(0xff);
          Serial.print("page page4");
          Serial.write(0xff);
          Serial.write(0xff);
          Serial.write(0xff);
        }
        url_setup();
      }
      else
      {
        if (String(d_id) == "")
        {
          post_function_d_id();
          delay(5000);// try to get device id every 5 seconds untill u get it

        }

        if (String(d_id) != "")
        {
          if (!client.connected()) reconnect();
          client.loop();
        }

      }
    }
  }

  else
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      
      if (!client.connected()) reconnect();
      client.loop();

      if (reader_mode == 0)
      {
        if (live_screen)
        {
          live_screen = false;
          page3();
        }
        
        live_mode();
      }

      else if (reader_mode == 1) //test mode
      {
        test_mode();
      }
    }
  }
  
  if(WiFi.status() == WL_CONNECTED)
  {
     ArduinoOTA.handle();
  }
}


//MODES//

void reset_mode()
{
  Serial.print("page page0");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.print("page page0");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.print("page0.t0.txt=");
  Serial.print("\"");
  Serial.print("Present Setup Card");
  Serial.print("\"");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.print("page0.p0.pic=");
  Serial.print("0");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  preferences.begin("data", false);
  preferences.putString("ssid", "");
  preferences.putString("password", "");
  preferences.putString("startup", "");
  preferences.putString("game_id", "");
  preferences.putString("cpt", "");
  preferences.putString("cpd", "");
  preferences.putString("tpt", "");
  preferences.putString("tph", "");
  preferences.putString("tpl", "");
  //Serial.println("RESET!");
  preferences.end();
  GET_function_reset();
  ESP.restart();
}

void live_mode()
{
  //Serial.println("in lvie mode");
  circularPattern();
  rfid_live();
  ticket_1();
  ticket_2();

  if (tpt == "N.O")
  {
    count_tp_pulse_no();
  }

  else if (tpt == "N.C")
  {
    count_tp_pulse_nc();
  }

  if (game_won == true)
  {
    game_won = false;
    Serial.print("page3.t5.txt=");
    Serial.print("\"");
    Serial.print(String(tickets) + " tickets won");
    Serial.print("\"");
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
    post_function_tickets(game_id, String(tickets));
  }

  if (millis() - last_ping > 15000)
  {
    ping_server();
    last_ping = millis();
  }
}

void test_mode()
{
  rfid_test();
}

// PIN INIT //

void pins()
{
  pinMode(buzz, OUTPUT);
  pinMode(cp_pin, OUTPUT);
  pinMode(tick1_out, OUTPUT);
  pinMode(tick2_out, OUTPUT);


  btn.attachClick(btn1_click);

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}

// NEXTION FUCNTION //

void nextion_startup()
{
  Serial.write(255);
  Serial.write(255);
  Serial.write(255);
  Serial.print("bkcmd=0"); // 0 disables errors, 2 enables errors
  Serial.write(255);
  Serial.write(255);
  Serial.write(255);

  Serial.write(255);
  Serial.write(255);
  Serial.write(255);
  Serial.print("bkcmd=0"); // 0 disables errors, 2 enables errors
  Serial.write(255);
  Serial.write(255);
  Serial.write(255);

  Serial.print("page page0");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.print("page page0");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.print("page0.t0.txt=");
  Serial.print("\"");
  Serial.print("Present Setup Card");
  Serial.print("\"");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.print("page0.t1.txt=");
  Serial.print("\"");
  Serial.print(" ");
  Serial.print("\"");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.print("page0.p0.pic=");
  Serial.print("0");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

}

void test_nextion_screen()
{
  Serial.print("page page2");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.print("page page2");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.print("page2.t0.txt=");
  Serial.print("\"");
  Serial.print(String(voltage)); // enter voltage here
  Serial.print("\"");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.print("page2.t5.txt=");
  Serial.print("\"");
  Serial.print(d_id); // enter id here
  Serial.print("\"");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.print("page2.t8.txt=");
  Serial.print("\"");
  Serial.print(String(millis()-last_ping / 1000));
  Serial.print("\"");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}

void page1()
{
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}

void page3()
{
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}

// URL SETUP //
void url_setup()
{
  if (Serial2.available())
  {
    read_url();
  }
  if (sendURL)
  {
    send_url();
    sendURL = false;
  }
}

// WIFI INIT //
void wifi_init(String ssid, String password)
{
  Serial.println(ssid + "  " + password);

  if (WiFi.status() != WL_CONNECTED)
  {
    //Serial.println("in wifi");
    if (wifiConnectStartTime == 0)
    {
      //Serial.print("\nConnecting to ");
      //Serial.println(ssid);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid.c_str(), password.c_str());
      client.setCallback(callback);
      wifiConnectStartTime = millis();
    }

    while (WiFi.status() != WL_CONNECTED && millis() - wifiConnectStartTime < wifiConnectTimeout)
    {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      // change to serial 2 after debugging

      Serial.print("page0.t0.txt=");
      Serial.print("\"");
      Serial.print("Fetching Device ID");
      Serial.print("\"");
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);

      Serial.print("page0.p0.pic=");
      Serial.print("1");
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);

      //Serial.println("\nWiFi connected\nIP address: ");
      //Serial.println(WiFi.localIP());

      espClient.setCACert(root_ca);
      client.setServer(mqtt_server, mqtt_port);
      client.setCallback(callback);
      if (!client.connected()) reconnect();
      preferences.begin("data", false);
      preferences.putString("ssid", ssid);
      preferences.putString("password", password);
      //Serial.println("Network Credentials Saved using Preferences");
      preferences.end();
      OTA_INIT();
      blinkGreen(3000);
      url_screen = true;

    }

    else
    {
      ssid = "";
      pass = "";
      //Serial.println("\nWiFi connection failed within timeout. Returning to main loop.");
      blinkRed(3000);
    }

    // Reset wifiConnectStartTime for the next connection attempt
    wifiConnectStartTime = 0;
  }
}


// CHECK STARTUP //
void check_startup()
{
  String first_start = "";
  preferences.begin("data", false);
  first_start = preferences.getString("startup", "");

  if (first_start == "")
  {
    Serial.println("No values saved for startup");
  }
  else
  {
    Serial.println("device_configured!");
    first_time = false;
  }
}


//POST FUCNTIONS//

void post_function_d_id()
{
  HTTPClient http;
  String server = "https://" + url + ".data-pilot.cloud/api/device/register";
  http.begin(server.c_str());
  http.addHeader("Content-Type", "Text");

  int httpResponseCode = http.POST("");   //Send the actual POST request

  if (httpResponseCode > 0)
  {

    String response = http.getString();

    // Serial.println(httpResponseCode);
    if (httpResponseCode == 200)
    {
      //Serial.println(response);
      get_device_id(response.c_str());

      Serial.print("page4.t1.txt=");
      Serial.print("\"");
      Serial.print(d_id);
      Serial.print("\"");
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.print("page4.t0.txt=");
      Serial.print("\"");
      Serial.print("WAITING FOR CONFIGURATION");
      Serial.print("\"");
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
    }

    else
    {
      //Serial.print("Error on sending POST: ");
      //Serial.println(httpResponseCode);
      blinkRed(3000);

    }

    http.end();

  }
}

void post_function_check()
{
  HTTPClient http;
  String server = "https://" + url + ".data-pilot.cloud/api/device/" + d_id + "/check";
  Serial.println(server);
  http.begin(server.c_str());
  http.addHeader("Content-Type", "Text");

  int httpResponseCode = http.POST("");   //Send the actual POST request

  if (httpResponseCode > 0)
  {

    String response = http.getString();

    // Serial.println(httpResponseCode);
    if (httpResponseCode == 200)
    {
      //Serial.println(response);
      check_device(response.c_str());
    }

    else
    {
      //Serial.print("Error on sending POST: ");
      //Serial.println(httpResponseCode);
      blinkRed(3000);
    }
    http.end();
  }
}

void GET_function_reset()
{
  HTTPClient http;
  String server = "https://" + url + ".data-pilot.cloud/api/device/" + d_id + "/register";
  http.begin(server.c_str());
  int httpResponseCode = http.GET(); //Send the actual POST request

  if (httpResponseCode > 0)
  {

    String response = http.getString();

    //Serial.println(httpResponseCode);
    if (httpResponseCode == 200)
    {
      blinkWhite(2000);
      //Serial.println("device Reset!");
    }

    else
    {
      //Serial.print("Error on sending POST: ");
      //Serial.println(httpResponseCode);
    }
    http.end();
  }
}

void post_function_game_start(String deviceId, String cardNumber)
{
  game_id = "";
  balance_avail = "";
  tickets = 0;

  String formData = "deviceId=" + String(deviceId) + "&cardNumber=" + String(cardNumber);

  HTTPClient http;
  String server = "https://" + url + ".data-pilot.cloud/api/game/start";

  http.begin(server.c_str());
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(formData); // Send the JSON data as the request body

  if (httpResponseCode > 0)
  {
    String response = http.getString();

    if (httpResponseCode == 200)
    {
      //Serial.println(response);
      parseJsonResponse(response);

      if (balance_avail == "success")
      {

        Serial.print("page3.t5.txt=");
        Serial.print("\"");
        Serial.print(tickets);
        Serial.print("\"");
        Serial.write(0xff);
        Serial.write(0xff);
        Serial.write(0xff);

        Serial.print("page3.t4.txt=");
        Serial.print("\"");
        Serial.print(play_cost);
        Serial.print("\"");
        Serial.write(0xff);
        Serial.write(0xff);
        Serial.write(0xff);
        blinkGreen(3000);
        //        Serial.print("page3.t1.txt=");
        //        Serial.print("\"");
        //        Serial.print(game_id);
        //        Serial.print("\"");
        //        Serial.write(0xff);
        //        Serial.write(0xff);
        //        Serial.write(0xff);

        if (cpt == "N.O")
        {
          digitalWrite(cp_pin, HIGH);
          delay(cpd.toInt());
          digitalWrite(cp_pin, LOW);
        }

        else if (cpt == "N.C")
        {
          digitalWrite(cp_pin, LOW);
          delay(cpd.toInt());
          digitalWrite(cp_pin, HIGH);
        }
      }

      else
      {
        //Serial.println(httpResponseCode);
        //Serial.println(response);
        Serial.print("page3.t1.txt=");
        Serial.print("\"");
        Serial.print("No Balance");
        Serial.print("\"");
        Serial.write(0xff);
        Serial.write(0xff);
        Serial.write(0xff);

        blinkRed(3000);
      }
    }

    else
    {
      //Serial.print("Error on sending POST: ");
      //Serial.println(httpResponseCode);
      Serial.print("page3.t1.txt=");
      Serial.print("\"");
      Serial.print("No Balance");
      Serial.print("\"");
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      blinkRed(3000);
    }

    http.end();
  }

}


void post_function_tickets(String p_id, String tickets)
{

  String server = "https://" + url + ".data-pilot.cloud/api/game/win";
  String formData = "&playId=" + String(p_id) + "&tickets=" + String(tickets);

  HTTPClient http;
  http.begin(server.c_str());
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(formData); // Send the JSON data as the request body

  if (httpResponseCode > 0)
  {

    String response = http.getString();

    //Serial.println(httpResponseCode);
    if (httpResponseCode == 200)
    {
      //Serial.println(response);
    }

    else
    {
      //Serial.println(response);
      //Serial.print("Error on sending POST: ");
      //Serial.println(httpResponseCode);
    }

    http.end();

  }
}


// RESET BUTTON//
static void btn1_click()
{
  reset_flag = true;
}




//PATTERENS//
void circularPattern()
{
  static int startIndex = 0;
  static unsigned long lastUpdate = 0;
  const unsigned long delayTime = 100; // Time between LED updates in milliseconds

  // Check if it's time to update the LEDs
  if (millis() - lastUpdate >= delayTime) {
    lastUpdate = millis();

    // Clear all LEDs
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 50));

    // Set the LED at the current index to the desired color
    leds[startIndex] = CRGB(0, 0, 255);;

    // Increment the index for the next LED
    startIndex = (startIndex + 1) % NUM_LEDS;

    // Show the updated LEDs
    FastLED.show();
  }
}

void updatePulsateEffect()
{
  // Calculate the pulsating factor using cosine function
  float elapsedTime = millis() - lastUpdateTime;
  pulseFactor = 0.5 * (1 + cos(elapsedTime * 2 * PI / pulseDuration));

  CRGB adjustedColor = CRGB::Yellow;
  adjustedColor.nscale8_video(static_cast<uint8_t>(255 * pulseFactor));

  fill_solid(leds, NUM_LEDS, adjustedColor);
  FastLED.show();
}


// TICKET COUNTERS //
void count_tp_pulse_nc()
{
  while (tick1 > tick1_thresh || tick2 > tick2_thresh)
  {
    game_won = true;
    ticket_1();
    ticket_2();
    if (tick1 > tick1_thresh)
    {
      digitalWrite(tick1_out, HIGH);
      delay(tph.toInt());
      digitalWrite(tick1_out, LOW);
      delay(tph.toInt());
      tickets++;
    }

    if (tick2 > tick2_thresh)
    {
      digitalWrite(tick2_out, HIGH);
      delay(tph.toInt());
      digitalWrite(tick2_out, LOW);
      delay(tph.toInt());
      tickets++;
    }
  }

  // Serial.println(tickets);

}


void count_tp_pulse_no()
{

  while (tick1 > tick1_thresh || tick2 > tick2_thresh)
  {
    game_won = true;
    ticket_1();
    ticket_2();
    if (tick1 > tick1_thresh)
    {
      digitalWrite(tick1_out, LOW);
      delay(tpl.toInt());
      digitalWrite(tick1_out, HIGH);
      delay(tpl.toInt());
      tickets++;
    }

    if (tick2 > tick2_thresh)
    {
      digitalWrite(tick2_out, LOW);
      delay(tpl.toInt());
      digitalWrite(tick2_out, HIGH);
      delay(tpl.toInt());
      tickets++;
    }
  }

  //Serial.println(tickets);
}


//RFID RESPONSE FUNTIONS//


void rfid_init()
{
  // Give the Serial2 module some time to power up
  // Send the initial command
  byte command[] = {0xAA, 0x00, 0x01, 0x83, 0x82, 0xBB};
  sendCommand(command, sizeof(command));

}

void rfid_setup()
{
  if (Serial2.available())
  {
    read_wifi_credentials();
  }
  if (sendSSID)
  {
    send_ssid();
    sendSSID = false;
  }

  // Check if you have received a response and need to send the password command
  if (sendPassword)
  {
    send_pass();
    sendPassword = false;
  }

  if (ssid != "" && pass != "")
  {
    //Serial.println(ssid+" "+pass);
    wifi_init(ssid, pass);
  }
  else
  {
    wifiConnectStartTime = 0;
  }
}

void rfid_test()
{
  rfid_tag = "";


  if (Serial2.available())
  {
    rfid_tag  = read_card();
    test_nextion_screen();
    blinkGreentest(3000);
  }

}

void rfid_live()
{
  String uid = "";

  if (Serial2.available())
  {
    uid = read_card();
    // Serial.println(uid);
    Serial.print("page0.t1.txt=");
    Serial.print("\"");
    Serial.print("Fetching Game ID!");
    Serial.print("\"");
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
    post_function_game_start("644b52885b5fc", "2483404489");

  }

}

void sendCommand(byte command[], int length)
{
  for (int i = 0; i < length; i++)
  {
    Serial2.write(command[i]);
  }
}

void read_url()
{

  delay(100);  // Give the module some time to respond (adjust as needed)
  String response = "";
  while (Serial2.available())
  {
    byte responseByte = Serial2.read();
    response += (responseByte < 0x10 ? "0" : ""); // Add leading "0" if necessary
    response += String(responseByte, HEX); // Append the byte in hexadecimal format
    response += " "; // Add a space character to separate values
    //Serial.print(responseByte < 0x10 ? "0" : "");
    //Serial.print(responseByte, HEX);
    //Serial.print(" ");
  }
  //Serial.println();
  // Check if the response is "AA 00 05 00" (card scanned)

  if (response.indexOf("aa 00 05 00") != -1)
  {
    sendURL = true;
  }

  extract_url(response);
  rs++;
}

void read_wifi_credentials()
{

  delay(100);  // Give the module some time to respond (adjust as needed)
  String response = "";
  while (Serial2.available())
  {
    byte responseByte = Serial2.read();
    response += (responseByte < 0x10 ? "0" : ""); // Add leading "0" if necessary
    response += String(responseByte, HEX); // Append the byte in hexadecimal format
    response += " "; // Add a space character to separate values
    // Serial.print(responseByte < 0x10 ? "0" : "");
    //Serial.print(responseByte, HEX);
    //Serial.print(" ");
  }
  //Serial.println();
  // Check if the response is "AA 00 05 00" (card scanned)

  if (response.indexOf("aa 00 05 00") != -1)
  {
    sendSSID = true;
    s_p = true;
  }

  else if (s_p == true)
  {
    sendPassword = true;
    s_p = false;
  }

  extract_string(response);
  rs++;
}

String read_card()
{
  delay(100);  // Give the module some time to respond (adjust as needed)
  String response = "";
  while (Serial2.available())
  {
    byte responseByte = Serial2.read();
    response += (responseByte < 0x10 ? "0" : ""); // Add leading "0" if necessary
    response += String(responseByte, HEX); // Append the byte in hexadecimal format
    response += " "; // Add a space character to separate values
    // Serial.print(responseByte < 0x10 ? "0" : "");
    //Serial.print(responseByte, HEX);
    //Serial.print(" ");
  }
  //Serial.println();
  // Check if the response is "AA 00 05 00" (card scanned)

  if (response.indexOf("aa 00 05 00") != -1)
  {
    return String(response);
  }
}

void extract_url(String hexString)
{
  int startIndex = 21;
  int endIndex;

  if (startIndex != -1)
  {
    if (hexString.indexOf("00") != -1)
    {
      endIndex = hexString.indexOf("00", startIndex);
    }

    else
    {
      endIndex = hexString.indexOf("bb", startIndex) - 2;
    }

    if (endIndex != -1)
    {
      String subString = hexString.substring(startIndex + 2, endIndex);
      String asciiString = hexToString(subString);
      //Serial.println(asciiString);
      //Serial.println(rs);
      if (rs == 2)
      {
        rs = 0;
        url = asciiString;
        //Serial.println(url);
        preferences.begin("data", false);
        preferences.putString("url", url);
        //Serial.println("url Saved using Preferences");
      }
    }
  }
}

void extract_string( String hexString)
{

  int startIndex = 21;
  int endIndex;

  if (startIndex != -1)
  {
    if (hexString.indexOf("00") != -1)
    {
      endIndex = hexString.indexOf("00", startIndex);
    }

    else
    {
      endIndex = hexString.indexOf("bb", startIndex) - 2;
    }

    if (endIndex != -1)
    {
      String subString = hexString.substring(startIndex + 2, endIndex);
      String asciiString = hexToString(subString);
      //Serial.println(asciiString);
      //Serial.println(rs);
      if (rs == 2)
      {
        ssid = asciiString;
        //Serial.println(ssid);
      }

      if (rs == 3)
      {
        rs = 0;
        pass = asciiString;
        //Serial.println(pass);

        if (pass == "")
        {
          blinkRed(3000);
        }
      }
    }
  }

}

String hexToString(String hexString)
{
  String asciiString = "";
  hexString.replace(" ", ""); // Remove spaces
  for (int i = 0; i < hexString.length(); i += 2)
  {
    String hexPair = hexString.substring(i, i + 2);
    char asciiChar = char(strtoul(hexPair.c_str(), NULL, 16));
    asciiString += asciiChar;
  }
  return asciiString;
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


//RFID COMMANDS//
void send_pass()
{
  // Serial.println("sending pass command");
  byte command[] = {0xAA, 0x00, 0x0A, 0x20, 0x01, 0x01, 0x05, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2F, 0xBB}; //password get command
  sendCommand(command, sizeof(command));
}

void send_ssid()
{
  // Serial.println("sending ssid command");
  byte command[] = {0xAA, 0x00, 0x0A, 0x20, 0x01, 0x01, 0x04, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2E, 0xBB}; //ssid get command
  sendCommand(command, sizeof(command));
}

void send_url()
{
  byte command[] = {0xAA, 0x00, 0x0A, 0x20, 0x01, 0x01, 0x04, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2E, 0xBB}; //url get command
  sendCommand(command, sizeof(command));
}



//VOLTAGE CALCULATION//
void ticket_1()
{
  int rawValue = analogRead(34);

  float  voltage = (float)rawValue * (3.3 / 4095.0) * (1000 + 2700) / 2700;
  float surce = (12700 / 2700) * voltage;
  tick1 = surce;
  // Serial.print("Raw ADC Value: ");
  //Serial.print(rawValue);
  //Serial.print("\tVoltage: ");
  //Serial.print(surce, 2);  // Print voltage with 2 decimal places
  //Serial.println(" V");
}

void ticket_2()
{
  int rawValue = analogRead(32);

  // Convert the raw value to voltage using the voltage divider formula
  float  voltage = (float)rawValue * (3.3 / 4095.0) * (1000 + 2700) / 2700;
  float surce = (12700 / 2700) * voltage;
  tick2 = surce;
  // Serial.print("Raw ADC Value: ");
  //Serial.print(rawValue);
  //Serial.print("\tVoltage: ");
  // Serial.print(surce, 2);  // Print voltage with 2 decimal places
  // Serial.println(" V");
}

void battery()
{
  int rawValue = analogRead(voltagePin);

  // Convert the raw value to voltage using the voltage divider formula
  voltage = (float)rawValue * (3.3 / 4095.0) * (R1 + R2) / R2;

  // Serial.print("Raw ADC Value: ");
  //Serial.print(rawValue);
  //Serial.print("\tVoltage: ");
  //Serial.print(voltage, 2);  // Print voltage with 2 decimal places
  // Serial.println(" V");
}


//JSON RESPSONSE DECODE//

void parseJsonResponse(const String& jsonResponse)
{
  DynamicJsonDocument doc(1024);

  DeserializationError error = deserializeJson(doc, jsonResponse);

  if (error)
  {
    //Serial.print("JSON parsing failed: ");
    //Serial.println(error.c_str());
    return;
  }

  const char* status = doc["status"];
  JsonObject game = doc["game"];
  String g_id = String(game["id"].as<String>());          // Convert to String explicitly
  String game_costOfPlay = String(game["costOfPlay"].as<String>()); // Convert to String explicitly

  //Serial.print("Status: ");
  //Serial.println(status);
  balance_avail = status;
  // Serial.print("Game ID: ");
  // Serial.println(game_id);
  game_id = g_id;
  play_cost = game_costOfPlay;

  preferences.begin("data", false);
  preferences.putString("game_id", game_id);
  preferences.end();

  //   Serial.print("Cost of Play: ");
  //  Serial.println(game_costOfPlay);
}

void get_device_id(const char* json)
{
  const size_t bufferSize = JSON_OBJECT_SIZE(100); // Adjust the size according to your JSON structure
  DynamicJsonDocument doc(bufferSize);

  // Deserialize the JSON data
  DeserializationError error = deserializeJson(doc, json);

  // Check for parsing errors
  if (error)
  {
    //Serial.print("JSON parsing failed: ");
    //Serial.println(error.c_str());
    return;
  }

  // Extract the "deviceId" field
  const char* deviceId = doc["data"]["deviceId"];
  // d_id = String(deviceId);
  d_id = "644b52885b5fc";
  //Serial.print("Device ID: ");
  //Serial.println(deviceId);
}

void check_device(String json)
{
  const size_t bufferSize = JSON_OBJECT_SIZE(100); // Adjust the size according to your JSON structure
  DynamicJsonDocument doc(bufferSize);

  // Deserialize the JSON data
  DeserializationError error = deserializeJson(doc, json);

  // Check for parsing errors
  if (error)
  {
    //Serial.print("JSON parsing failed: ");
    //Serial.println(error.c_str());
    return;
  }

  // Extract the "deviceId" field
  const char* stat = doc["status"];
  //Serial.print("Status: ");
  //Serial.println(stat);
  if (String(stat) == "success")
  {
    first_time = false;
  }
  else
  {
    first_time = true;
    blinkRed(3000);

  }

}


//BLINK CODES//

void blinkRed(unsigned int duration)
{
  unsigned long startTime = millis();

  while (millis() - startTime < duration)
  {
    // Turn on all LEDs to red
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show();
    delay(500);  // LED on time

    // Turn off all LEDs
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(500);  // LED off time
  }
}

void blinkGreen(unsigned int duration)
{
  unsigned long startTime = millis();

  while (millis() - startTime < duration) {
    // Turn on all LEDs to green
    fill_solid(leds, NUM_LEDS, CRGB::Green);
    FastLED.show();
    delay(500);  // LED on time

    // Turn off all LEDs
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(500);  // LED off time

  }
}

void blinkWhite(unsigned int duration)
{
  unsigned long startTime = millis();

  while (millis() - startTime < duration) {
    // Turn on all LEDs to green
    fill_solid(leds, NUM_LEDS, CRGB::White);
    FastLED.show();

    digitalWrite(buzz, HIGH);
    delay(500);  // LED on time

    // Turn off all LEDs
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    digitalWrite(buzz, LOW);
    delay(500);  // LED off time

  }
}

void blinkGreentest(unsigned int duration)
{
  unsigned long startTime = millis();
  int cnt = 3;
  while (cnt != 0)
  {
    cnt--;
    Serial.print("page2.t3.txt=");
    Serial.print("\"");
    Serial.print(String(cnt));
    Serial.print("\"");
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
    // Turn on all LEDs to green
    fill_solid(leds, NUM_LEDS, CRGB::Green);
    FastLED.show();
    delay(500);

    // Turn off all LEDs
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(500);

  }

  digitalWrite(buzz, HIGH);
  delay(2000);
  digitalWrite(buzz, LOW);


  page1();
}

// MQTT FUCNTIONS //

void callback(char* topic, byte* payload, unsigned int length)
{
  String s_topic = topic;

  if (s_topic == "configure")
  {
    String incommingMessage = "";
    for (int i = 0; i < length; i++) incommingMessage += (char)payload[i];
    //    Serial.println(incommingMessage);
    isl = getValue(incommingMessage, ',', 0);
    cpt = getValue(incommingMessage, ',', 1);
    cpd = getValue(incommingMessage, ',', 2);
    tpt = getValue(incommingMessage, ',', 3);
    tph = getValue(incommingMessage, ',', 4);
    tpl = getValue(incommingMessage, ',', 5);
    tp_thresh = getValue(incommingMessage, ',', 6);

    if (cpt == "N.O")
    {
      digitalWrite(cp_pin, LOW);
    }
    else if (cpt == "N.C")
    {
      digitalWrite(cp_pin, HIGH);
    }

    //    Serial.println(isl);
    //    Serial.println(cpt);
    //    Serial.println(cpd);
    //    Serial.println(tpt);
    //    Serial.println(tph); Serial.println(tpl);
    //    Serial.println(tp_thresh);

    if (isl != "" && cpt != "" && cpd != "" && tpt != "" && tph != "" && tpl != "" && tp_thresh != "")
    {
      tick1_thresh = tp_thresh.toInt();
      tick2_thresh = tp_thresh.toInt();
      preferences.begin("data", false);
      preferences.putString("startup", "1");
      preferences.putString("cpt", cpt );
      preferences.putString("cpd", cpd );
      preferences.putString("tpt", tpt );
      preferences.putString("tph", tph );
      preferences.putString("tpl", tpl );
      preferences.putString("tp_thresh", tp_thresh );
      //Serial.println("startup variables updated!");
      preferences.end();
      post_function_check();

    }

    else
    {
      //Serial.println("not configured!");
    }
  }

  else if (s_topic == "mode")
  {

    // Serial.println(s_topic);
    String incommingMessage = "";
    for (int i = 0; i < length; i++) incommingMessage += (char)payload[i];
    // Serial.println(incommingMessage);
    reader_mode = incommingMessage.toInt();
  }

  else if (s_topic == "backend")
  {
    String incommingMessage = "";
    for (int i = 0; i < length; i++) incommingMessage += (char)payload[i];
    //Serial.println(incommingMessage);

    if (incommingMessage == "locate")
    {
      blinkWhite(10000);
    }

    else if (incommingMessage == "reboot")
    {
      ESP.restart();
    }
    else if (incommingMessage == "reset")
    {
      reset_mode();
    }
  }

  if (reader_mode == 1 && first_time == false)
  {
    page1();
    
  }

  else if (reader_mode == 0 && first_time == false)
  {
    live_screen = true;
  }

}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    //Serial.print("Attempting MQTT connection...");
    String clientId = "clientId-4h73XJgYro";   // Create a random client ID
    // clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password))
    {
      //Serial.println("connected");

      //SUBSCRIBE TO TOPIC HERE

      client.subscribe("configure");
      client.subscribe("mode");
      client.subscribe("backend");

    }
    else
    {
      //      Serial.print("failed, rc=");
      //      Serial.print(client.state());
      //      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void ping_server()
{
  // Convert topic and payload to C-style strings
//  const char* topic = ("/device/" + d_id + "/ping").c_str();
  const char* topic = ("/device/644b52885b5fc/ping");
  const char* payload = String(last_ping / 1000).c_str();

  // Publish the message
  client.publish(topic, payload);
}

void OTA_INIT()
{
  ArduinoOTA
  .onStart([]()
  {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
}
