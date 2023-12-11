#include <WiFi.h>
#include <HTTPClient.h>
#include "OneButton.h"

#define button1_pin 23
OneButton btn = OneButton(button1_pin, true, true);
int count = 0;
const char* ssid = "OCTATHORN";
const char* password = "62543800";
const char* token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJlMmY2NzVhMGU1Mjk0MTZlYWRhYWU4YTY4OTk1N2VmYyIsImlhdCI6MTY5MTc1ODY3MCwiZXhwIjoyMDA3MTE4NjcwfQ.7gbduuMCPcB-kYzhS0K5esqJBD6X8jZ3n-8AedKD_2w";
const char* ipAddress = "192.168.18.79";

void setup()
{

  Serial.begin(115200);
  btn.attachClick(b1_click);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void loop()
{
  btn.tick();
}

static void b1_click()
{
  Serial.println("press!");
  count++;
  if (count > 1)
  {
    count = 0;
  }
  if (count == 1)
  {
    http_on();
  }
  if (count == 0)
  {
    http_off();
  }
}



void http_on()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    // Construct the URL for turning on a switch entity
    String url = "http://" + String(ipAddress) + ":8123/api/services/switch/turn_on";

    http.begin(url);
    http.addHeader("Authorization", "Bearer " + String(token));
    http.addHeader("Content-Type", "application/json");

    // Optional: Prepare a JSON payload if required by the service
    String payload = "{\"entity_id\": \"switch.esphome_web_006410_switch_1\"}";

    int httpCode = http.POST(payload);
    String response = http.getString();

    Serial.println("HTTP Code: " + String(httpCode));
    Serial.println("Response: " + response);

    http.end();
  }
}

void http_off()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    // Construct the URL for turning on a switch entity
    String url = "http://" + String(ipAddress) + ":8123/api/services/switch/turn_off";

    http.begin(url);
    http.addHeader("Authorization", "Bearer " + String(token));
    http.addHeader("Content-Type", "application/json");

    // Optional: Prepare a JSON payload if required by the service
    String payload = "{\"entity_id\": \"switch.esphome_web_006410_switch_1\"}";

    int httpCode = http.POST(payload);
    String response = http.getString();

    Serial.println("HTTP Code: " + String(httpCode));
    Serial.println("Response: " + response);

    http.end();
  }
}
