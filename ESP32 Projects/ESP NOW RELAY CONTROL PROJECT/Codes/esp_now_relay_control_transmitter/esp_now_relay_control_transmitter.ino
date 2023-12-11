#include <esp_now.h>
#include <WiFi.h>

#define SWITCH_PIN1 19
#define SWITCH_PIN2 5
#define SWITCH_PIN3 14
#define SWITCH_PIN4 15
#define SWITCH_PIN5 16
#define SWITCH_PIN6 17
#define SWITCH_PIN7 18

uint8_t receiverMac[] = {0xC8, 0xC9, 0xA3, 0xC6, 0x2B, 0x3C}; // add your receiver mac here

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  if (status == ESP_NOW_SEND_SUCCESS)
  {
    Serial.println("Message sent successfully");
  }
  else
  {
    Serial.println("Message failed to send");
  }
}



void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  esp_now();

  pinMode(SWITCH_PIN1, INPUT_PULLUP);
  pinMode(SWITCH_PIN2, INPUT_PULLUP);
  pinMode(SWITCH_PIN3, INPUT_PULLUP);
  pinMode(SWITCH_PIN4, INPUT_PULLUP);
  pinMode(SWITCH_PIN5, INPUT_PULLUP);
  pinMode(SWITCH_PIN6, INPUT_PULLUP);
  pinMode(SWITCH_PIN7, INPUT_PULLUP);

}

void loop()
{
  if (digitalRead(SWITCH_PIN1) == LOW)
  {
    send_data("1");
    delay(500);
  }

  else if (digitalRead(SWITCH_PIN2) == LOW)
  {
    send_data("2");
    delay(500);
  }

  else if (digitalRead(SWITCH_PIN3) == LOW)
  {
    send_data("3");
    delay(500);
  }

  else if (digitalRead(SWITCH_PIN4) == LOW)
  {
    send_data("4");
    delay(500);
  }

  else if (digitalRead(SWITCH_PIN5) == LOW)
  {
    send_data("5");
    delay(500);
  }

  else  if (digitalRead(SWITCH_PIN6) == LOW)
  {
    send_data("6");
    delay(500);
  }

  else if (digitalRead(SWITCH_PIN7) == LOW)
  {
    send_data("7");
    delay(500);
  }

}

void esp_now()
{
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void send_data(String message)
{
  esp_err_t result = esp_now_send(receiverMac, (uint8_t*)message.c_str(), message.length());

  if (result == ESP_OK)
  {
    Serial.println("Sending: " + message);
  }
  else
  {
    Serial.println("Failed to send data");
  }
}
