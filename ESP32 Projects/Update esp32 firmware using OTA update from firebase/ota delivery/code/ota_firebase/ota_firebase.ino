#include <WiFi.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <Update.h>
#include <Firebase_ESP_Client.h>  // Firebase
#include "addons/TokenHelper.h"   // Firebase Provide the token generation process info.
#include <Preferences.h>
Preferences preferences;


// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

const char* ssid = "RSK";
const char* password = "12345678";


#define API_KEY "AIzaSyAPLQ5p0Q-F4kqTWm6mqZy5tK1swRtUfkU"  // Firebase: Define the API Key
#define USER_EMAIL "esp32@gmail.com"
#define USER_PASSWORD "pIoneer090"
#define STORAGE_BUCKET_ID "esp32ota-d002d.appspot.com"  // Firebase: Define the Firebase storage bucket ID e.g bucket-name.appspot.com
#define FIRMWARE_PATH "firmware.bin"                    // Firebase: Define the firmware path on Firebase

String gen = "";

unsigned long previousMillis;



void fcsDownloadCallback(FCS_DownloadStatusInfo info) {
  if (info.status == fb_esp_fcs_download_status_init) {
    Serial.printf("New update found\n");
    Serial.printf("Downloading firmware %s (%d bytes)\n", info.remoteFileName.c_str(), info.fileSize);
  }

  else if (info.status == fb_esp_fcs_download_status_download) {
    Serial.printf("Downloaded %d%s\n", (int)info.progress, "%");
  }

  else if (info.status == fb_esp_fcs_download_status_complete) {
    Serial.println("Donwload firmware completed.");
    Serial.println();
  }

  else if (info.status == fb_esp_fcs_download_status_error) {
    Serial.printf("New firmware update not available or download failed, %s\n", info.errorMsg.c_str());
  }
}


void setup() {

  Serial.begin(115200);

  preferences.begin("firmware", false);
  gen = preferences.getString("version", "");

  if (gen == "") {
    Serial.println("no previous version found!");

  } else {
    Serial.println(gen);
  }

  Serial.println("version 1.0 without built in led blink");

  // Start WiFi connection
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  config.fcs.download_buffer_size = 2048;

  Firebase.reconnectWiFi(true);
}


void loop() {


  unsigned long currentMillis = millis(); /* capture the latest value of millis() */


  if (currentMillis - previousMillis >= 10000) {  // Execute this routine periodically

    update_firmware();

    previousMillis = currentMillis;
  }
}



void update_firmware() {
  if (Firebase.ready()) {

    Serial.println("\nChecking for new firmware update\n");
    Serial.printf("Get file Metadata... %s\n", Firebase.Storage.getMetadata(&fbdo, STORAGE_BUCKET_ID, "firmware.bin" /* remote file */) ? "ok" : fbdo.errorReason().c_str());

    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK) {
      FileMetaInfo meta = fbdo.metaData();
      if (gen != String(meta.generation)) {
        gen = String(meta.generation);
        Serial.println(gen);
        preferences.begin("firmware", false);
        preferences.putString("version", gen);
        Serial.println("Network Credentials Saved using Preferences");
        preferences.end();

        if (!Firebase.Storage.downloadOTA(
              &fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */,
              FIRMWARE_PATH /* path of firmware file stored in the bucket */,
              fcsDownloadCallback /* callback function */
              )) {
          Serial.println(fbdo.errorReason());
        }
        delay(2000);
        ESP.restart();
      } else {
        Serial.println("no new firmware!");
      }
    }
  }
}
