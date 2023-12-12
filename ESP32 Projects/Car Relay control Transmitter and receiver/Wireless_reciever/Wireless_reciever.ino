#include <WiFi.h>
#include <esp_now.h>
#include "OneButton.h"


const char* ssid = "MAC_RECIEVER";
const char* password = "123456789";
const char* ip_server = "192.168.1.2";
int count = 0;

const int button1_pin = 21;
const int relay1_pin = 22;
OneButton btn = OneButton(button1_pin, false, false);
int p_b = 0;


typedef struct test_struct
{
  int relay_no;
} test_struct;
test_struct myData;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{
  memcpy(&myData, incomingData, sizeof(myData));
  count++;

  if (count > 1)
  {
    count = 0;
  }

}


String read_mac()
{
  String mac = String(WiFi.macAddress());
  return mac;
}



void setup()
{
  Serial.begin(115200);
  btn.attachClick(b1_click);
  pinMode(relay1_pin, OUTPUT);
  digitalWrite(relay1_pin, HIGH);

  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);

}



void loop()
{

  btn.tick();

  if (count == 1)
  {
    digitalWrite(relay1_pin, LOW);
  }
  else if (count == 0)
  {
    digitalWrite(relay1_pin, HIGH);
  }

}




static void b1_click()
{
  WiFiClient client;
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  if (!client.connect(ip_server, 5556))
  {
    Serial.println("connection failed");
    return;
  }
  Serial.println("conenctted to server!");

  client.write(read_mac().c_str());
  delay(10);
  client.stop();
  WiFi.disconnect();
}
