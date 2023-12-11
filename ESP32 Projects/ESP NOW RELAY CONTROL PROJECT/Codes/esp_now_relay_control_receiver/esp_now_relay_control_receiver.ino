#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>


String  receivedData = "";

void OnDataReceived(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  Serial.print("Received data from: ");
  for (int i = 0; i < 6; i++)
  {
    Serial.print(mac_addr[i], HEX);
    if (i < 5) Serial.print(':');
  }
  Serial.println();
  Serial.print("Data: ");
  Serial.write(data, data_len);
  Serial.println();
  receivedData = String((char*)data);
}



// Define the GPIO pin that the relay module's IN pin is connected to.
const int RELAY_PIN1 = 2;
const int RELAY_PIN2 = 4;
const int RELAY_PIN3 = 21;
const int RELAY_PIN4 = 13;

// Define the GPIO pin that the momentary switch is connected to.
const int SWITCH_PIN1 = 19;
const int SWITCH_PIN2 = 5;
const int SWITCH_PIN3 = 14;
const int SWITCH_PIN4 = 15;
const int SWITCH_PIN5 = 16;
const int SWITCH_PIN6 = 17;
const int SWITCH_PIN7 = 18;

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  esp_now();

  pinMode(RELAY_PIN1, OUTPUT);
  pinMode(RELAY_PIN2, OUTPUT);
  pinMode(RELAY_PIN3, OUTPUT);
  pinMode(RELAY_PIN4, OUTPUT);

  pinMode(SWITCH_PIN1, INPUT_PULLUP);
  pinMode(SWITCH_PIN2, INPUT_PULLUP);
  pinMode(SWITCH_PIN3, INPUT_PULLUP);
  pinMode(SWITCH_PIN4, INPUT_PULLUP);
  pinMode(SWITCH_PIN5, INPUT_PULLUP);
  pinMode(SWITCH_PIN6, INPUT_PULLUP);
  pinMode(SWITCH_PIN7, INPUT_PULLUP);

  digitalWrite(RELAY_PIN1, HIGH);
  digitalWrite(RELAY_PIN2, HIGH);
  digitalWrite(RELAY_PIN3, HIGH);
  digitalWrite(RELAY_PIN4, HIGH);

}

void loop()
{

  if (digitalRead(SWITCH_PIN1) == LOW || receivedData == "1")
  {
    digitalWrite(RELAY_PIN1, LOW);
    digitalWrite(RELAY_PIN2, HIGH);
    digitalWrite(RELAY_PIN3, HIGH);
    digitalWrite(RELAY_PIN4, HIGH);
    receivedData = "";
  }


  if (digitalRead(SWITCH_PIN2) == LOW || receivedData == "2")
  {
    digitalWrite(RELAY_PIN1, HIGH);
    digitalWrite(RELAY_PIN2, LOW);
    digitalWrite(RELAY_PIN3, HIGH);
    digitalWrite(RELAY_PIN4, HIGH);
    receivedData = "";
  }

  if (digitalRead(SWITCH_PIN3) == LOW || receivedData == "3")
  {
    digitalWrite(RELAY_PIN1, HIGH);
    digitalWrite(RELAY_PIN2, HIGH);
    digitalWrite(RELAY_PIN3, LOW);
    digitalWrite(RELAY_PIN4, HIGH);
    digitalWrite(RELAY_PIN4, LOW);
    delay(2000);
    digitalWrite(RELAY_PIN4, HIGH);
    receivedData = "";
  }

  if (digitalRead(SWITCH_PIN4) == LOW || receivedData == "4")
  {
    digitalWrite(RELAY_PIN4, LOW);
    delay(2000);
    digitalWrite(RELAY_PIN4, HIGH);
    receivedData = "";
  }


  if (digitalRead(SWITCH_PIN5) == LOW || receivedData == "5")
  {
    digitalWrite(RELAY_PIN1, HIGH);
    digitalWrite(RELAY_PIN2, LOW);
    digitalWrite(RELAY_PIN3, HIGH);
    digitalWrite(RELAY_PIN4, HIGH);
    delay(60000);
    digitalWrite(RELAY_PIN2, HIGH);
    digitalWrite(RELAY_PIN3, LOW);
    digitalWrite(RELAY_PIN4, LOW);
    delay(2000);
    digitalWrite(RELAY_PIN4, HIGH);
    receivedData = "";
  }

  if (digitalRead(SWITCH_PIN6) == LOW || receivedData == "6")
  {
    digitalWrite(RELAY_PIN1, HIGH);
    digitalWrite(RELAY_PIN2, LOW);
    digitalWrite(RELAY_PIN3, HIGH);
    digitalWrite(RELAY_PIN4, HIGH);
    delay(30000);
    digitalWrite(RELAY_PIN2, HIGH);
    digitalWrite(RELAY_PIN3, LOW);
    digitalWrite(RELAY_PIN4, LOW);
    delay(2000);
    digitalWrite(RELAY_PIN4, HIGH);
    receivedData = "";
  }


  if (digitalRead(SWITCH_PIN7) == LOW || receivedData == "7")
  {
    digitalWrite(RELAY_PIN1, HIGH);
    digitalWrite(RELAY_PIN2, HIGH);
    digitalWrite(RELAY_PIN3, HIGH);
    digitalWrite(RELAY_PIN4, HIGH);
    receivedData = "";
  }
}

void esp_now()
{
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  // Register callback function to handle received data
  esp_now_register_recv_cb(OnDataReceived);
}
