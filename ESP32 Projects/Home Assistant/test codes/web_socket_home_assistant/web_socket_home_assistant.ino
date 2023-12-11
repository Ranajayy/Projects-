#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>

const char* authToken = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJiNjM5OTU2NjhkOTk0NjcyODZkNzg1MDE2Zjc5NTI5YSIsImlhdCI6MTY5MzQ3NTc4NCwiZXhwIjoyMDA4ODM1Nzg0fQ.OH8vPHyPPPFD6YdEdBWiL3u01qfGtLmBVViviSXYDCE";
int count = 1;

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
bool isconnected = false;
#define USE_SERIAL Serial

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

      // send message to server when Connected
      // webSocket.sendTXT("Connected");
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
        authJson["access_token"] = authToken;

        String authPayload;
        serializeJson(authJson, authPayload);

        // Send the authentication payload via WebSocket
        webSocket.sendTXT(authPayload);
        Serial.println("auth token sent");
      }
      if (jsonDoc.containsKey("type") && jsonDoc["type"].as<String>() == "auth_ok")
      {
        isconnected = true;
        sendSwitchTurnOn();
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



void setup()
{
  USE_SERIAL.begin(115200);

  USE_SERIAL.setDebugOutput(true);
  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFiMulti.addAP("OCTATHORN", "62543800");

  //WiFi.disconnect();
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  // server address, port and URL
  webSocket.begin("192.168.18.79", 8123, "/api/websocket");

  // event handler
  webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);

}

void loop()
{
  webSocket.loop();
  //  if (isconnected)
  //  {
  //    sendSwitchTurnOn();
  //  }
  //  count++;

}

void sendSwitchTurnOn()
{
  if (isconnected)
  {
    // Prepare the command JSON to turn on the switch
    DynamicJsonDocument commandJson(256);
    commandJson["id"] = count;
    commandJson["type"] = "call_service";
    commandJson["domain"] = "switch";
    commandJson["service"] = "turn_on";
    JsonObject target = commandJson.createNestedObject("target");
    target["entity_id"] = "switch.1";

    String commandPayload;
    serializeJson(commandJson, commandPayload);
    Serial.println(commandPayload);
    // Send the command payload via WebSocket
    webSocket.sendTXT(commandPayload);
  }
}
