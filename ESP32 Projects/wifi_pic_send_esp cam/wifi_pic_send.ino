#include "esp_camera.h"
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include "OneButton.h"
#include <Preferences.h>
#include <WebServer.h>


String ssid     = "";
String password = "";
const char* host = "192.168.18.9";//4260
const int iport = 4260;
const int wport = 4261;
const int bport = 4262;
const int rport = 4263;
const int gport = 4264;
String id = "";
bool   butn_flag = false;
#define PIN_RED    12
#define PIN_GREEN  13
#define PIN_BLUE   15

bool sendata = false;
camera_fb_t *fb = NULL;
bool s_flag = false;

class Button
{
  private:
    OneButton button;
    int value;
  public:
    explicit Button(uint8_t pin): button(pin)
    {
      button.setPressTicks(3000);
      button.attachClick([](void *scope)
      {
        ((Button *) scope)->Clicked();
      }, this);
      button.attachLongPressStart([](void *scope) {
        ((Button *) scope)->LongPressed();
      }, this);
    }

    void Clicked()
    {
      butn_flag = true;
    }

    void LongPressed()
    {
      s_flag = true;
    }

    void handle()
    {
      button.tick();
    }






};

Button button(14);


Preferences settings;
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
WebServer server(80);

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<h2>Octathorn<h2>
<h3> HTML Form ESP32</h3>

<form action="/action_page">
  WIFI SSID:<br>
  <input type="text" name="SSID" value="">
  <br>
  WIFI PASS:<br>
  <input type="text" name="PASS" value="">
  <br>  
    ID:<br>
  <input type="text" name="ID" value="">
  <br>  
  <br>
  <input type="submit" value="Submit">
</form> 

</body>
</html>
)=====";

void handleRoot() 
{
  String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}

void handleForm()
{
  ssid = server.arg("SSID");
 password = server.arg("PASS");
  id=server.arg("ID");

  Serial.print("WIFI SSID:");
  Serial.println(ssid);

  Serial.print("WIFI PASS:");
  Serial.println(password);

Serial.println("ID= "+String(id));
  settings.begin("credentials", false);
  settings.putString("ssid", ssid);
  settings.putString("password", password);
  settings.putString("id", id);
  Serial.println("Network Credentials Saved using Preferences");
  settings.end();
  
  String s = "<a href='/'> Go Back </a>";
  server.send(200, "text/html", s);
  WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  noTone(PIN_RED);
  noTone(PIN_GREEN);
  noTone(PIN_BLUE);
}




void setup()
{
  Serial.begin(115200);
  pinMode(PIN_RED,   OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE,  OUTPUT);



  settings.begin("credentials", false);
    ssid = settings.getString("ssid", "");
    password = settings.getString("password", "");
    id=settings.getString("id", "");

if(ssid=="" ||password==""||id=="")
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP("esp32", "123456789");
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println("WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/action_page", handleForm);
  server.begin();
  Serial.println("HTTP server started");
  tone(PIN_RED,1);
  tone(PIN_GREEN,1);
  tone(PIN_BLUE,1);
}

else
{

  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


}
  initCamera();
}

WiFiClient client;

void loop()
{
  server.handleClient();
  button.handle();
  if(butn_flag==true)
  {
    butn_flag=false;
      Serial.println("sending pictures!");
      digitalWrite(PIN_RED,HIGH);
      digitalWrite(PIN_GREEN,HIGH);
      digitalWrite(PIN_BLUE,HIGH);

      send_id();
      send_w();
      digitalWrite(PIN_RED,LOW);
      digitalWrite(PIN_GREEN,LOW);
      digitalWrite(PIN_BLUE,HIGH);
      send_b();
      digitalWrite(PIN_RED,LOW);
      digitalWrite(PIN_GREEN,HIGH);
      digitalWrite(PIN_BLUE,LOW);
      send_g();
      digitalWrite(PIN_RED,HIGH);
      digitalWrite(PIN_GREEN,LOW);
      digitalWrite(PIN_BLUE,LOW);
      send_r();
      digitalWrite(PIN_RED,LOW);
      digitalWrite(PIN_GREEN,LOW);
      digitalWrite(PIN_BLUE,LOW);
  }

  if(s_flag==true)
{
  s_flag=false;
  WiFi.mode(WIFI_AP);
  WiFi.softAP("esp32", "123456789");
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println("WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/action_page", handleForm);
  server.begin();
  Serial.println("HTTP server started");
  tone(PIN_RED,1);
  tone(PIN_GREEN,1);
  tone(PIN_BLUE,1);
}

}

void initCamera()
{camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (psramFound()) 
  {
    config.frame_size = FRAMESIZE_SXGA;
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    Serial.println("sram found!");
  } else {
     Serial.println("no sram found!");
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 20;
    config.fb_count = 2;
  }


  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }
   sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID)
  {
    s->set_vflip(s, 1); 
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }

  if(config.pixel_format == PIXFORMAT_JPEG)
  {
    s->set_framesize(s, FRAMESIZE_SXGA);
  }



}

void set_camera_frames()
{
  int i = 0;
 camera_fb_t *fb = NULL;
  // Take Picture with Camera
  while (i != 15)
  {
    fb = esp_camera_fb_get();

    if (!fb)
    {
      Serial.println("Camera capture failed");
      return;
    }
    esp_camera_fb_return(fb);
    i++;
    if (i == 15)
    {

      fb = esp_camera_fb_get();
    

      if (!fb)
      {
        Serial.println("Camera capture failed");
        return;
      }
      break;
    }
  }
  client.write(fb->buf, fb->len);
  Serial.println("picture sent");
  esp_camera_fb_return(fb);
}
void send_id()
{
  if (!client.connect(host, iport))
  {
    Serial.println("connection failed");
    return;
  }
  client.write(id.c_str());
  client.stop();
}

void send_w()
{
  if (!client.connect(host, wport))
  {
    Serial.println("connection failed");
    return;
  }
set_camera_frames();
  client.stop();
}

void send_b()
{
  if (!client.connect(host, bport))
  {
    Serial.println("connection failed");
    return;
  }
  set_camera_frames();
  client.stop();
}
void send_g()
{
  if (!client.connect(host, gport))
  {
    Serial.println("connection failed");
    return;
  }
  set_camera_frames();
  client.stop();
}
void send_r()
{
  if (!client.connect(host, rport))
  {
    Serial.println("connection failed");
    return;
  }
  set_camera_frames();
  client.stop();
}
