#include <Keypad.h>
#include <WiFi.h>
#include <Preferences.h>
#include <WebServer.h>
#include "esp_wifi.h"

#define led 5
#define buzz 4

bool new_entry = false;                           
bool pass_present = false;
bool pass_updated = false;


const char *ssid = "ESP32AP";    /*SSID of ESP32 Access point*/
const char *password = "123456789";  /*password for Access point*/
String macAddresses = "";

// Default password to compare against
const String defaultPassword = "1234";
String keypad_password = ""; // Stores the entered password

IPAddress local_ip(192, 168, 1, 4);
IPAddress gateway(192, 168, 1, 4);
IPAddress subnet(255, 255, 255, 0);
WebServer server(80);

const byte ROW_NUM = 4; // Number of rows in the keypad
const byte COLUMN_NUM = 4; // Number of columns in the keypad

// Define the keypad layout and key mappings
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Pins connected to the keypad rows and columns
byte pin_rows[ROW_NUM] = {21, 18, 17, 16};
byte pin_columns[COLUMN_NUM] = {12, 13, 14, 15};

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_columns, ROW_NUM, COLUMN_NUM);




const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<h2>2 FACTOR AUTHENTICATION<h2>
<h3> HTML Form ESP32</h3>

<form action="/action_page">
  CLIENT MAC:<br>
  <input type="text" name="MAC" value="%MAC%">
  <br>
  PASSWORD:<br>
  <input type="text" name="PASS" value="">
  <br>   
  <br>
  <input type="submit" value="Submit">
</form> 

</body>
</html>
)=====";

const int MAX_ENTRIES = 10; // Maximum number of MAC address and password entries
Preferences preferences;

void savePassword(const char* mac, const char* password) {
  bool macExists = false;
  bool passExists = false;

  preferences.begin("passwords", false); // Open preferences with namespace "passwords"

  // Check if the MAC address already exists
  for (int i = 0; i < MAX_ENTRIES; i++) {
    char key[18];
    sprintf(key, "mac_%d", i); // Generate the key for MAC address
    String storedMac = preferences.getString(key, "");
    if(storedMac!="")
    {
      Serial.println(storedMac);
    }
    if (storedMac == String(mac)) {
      macExists = true;
      sprintf(key, "pass_%d", i);
      preferences.putString(key, password); // Update the password for the existing MAC address
      Serial.println("Password updated successfully!");
       new_entry = false;
       pass_present = false;
       pass_updated = true;
      break; // Exit the loop if MAC address already exists

    }
  }

  if (macExists) 
  {
    Serial.println("MAC address already exists, exit the function!");
    preferences.end(); // Close preferences
    return;
  }

  // Check if the password already exists for a different MAC address
  for (int i = 0; i < MAX_ENTRIES; i++) {
    char macKey[18];
    char passKey[18];
    sprintf(macKey, "mac_%d", i); // Generate the key for MAC address
    sprintf(passKey, "pass_%d", i); // Generate the key for password
    String storedMac = preferences.getString(macKey, "");
    String storedPass = preferences.getString(passKey, "");
    if (storedMac != "" && storedPass == password) {
      passExists = true;
      Serial.println("Password already exists for a different MAC address!");
       new_entry = false;
       pass_present = true;
       pass_updated = false;
      break;
    }
  }

  if (passExists) {
    Serial.println("Password already exists, exit the function!");
    preferences.end(); // Close preferences
    return;
  }

  bool emptySlotFound = false; // Flag to track if an empty slot is found

  // Find an empty slot to save the new MAC address and password
  for (int i = 0; i < MAX_ENTRIES; i++) {
    char key[18];
    sprintf(key, "mac_%d", i); // Generate the key for MAC address
    String storedMac = preferences.getString(key, "");
    if (storedMac == "") {
      preferences.putString(key, mac); // Store the new MAC address
      sprintf(key, "pass_%d", i); // Generate the key for password
      preferences.putString(key, password); // Store the password
      emptySlotFound = true;
       new_entry = true;
       pass_present = false;
       pass_updated = false;
      break; // Exit the loop after saving the entry
    }
  }

  preferences.end(); // Close preferences

  if (emptySlotFound) {
    Serial.println("Entry saved successfully!");
  } else {
    Serial.println("Maximum number of entries reached!");
  }
}


bool checkPassword(const char* mac, const char* password) 
{
  preferences.begin("passwords", true); // Open preferences with namespace "passwords"

  // Check if the MAC address and password match
  for (int i = 0; i < MAX_ENTRIES; i++)
  {
    char key[18];
    sprintf(key, "mac_%d", i); // Generate the key for MAC address
    String storedMac = preferences.getString(key, "");
    sprintf(key, "pass_%d", i); // Generate the key for password
    String storedPassword = preferences.getString(key, "");

    if (storedMac == mac && storedPassword == password)
    {
      preferences.end(); // Close preferences
      return true; // MAC address and password match
    }
  }

  preferences.end(); // Close preferences
  return false; // MAC address and password not found
}

void PrintStations()
{
  macAddresses = "";
  wifi_sta_list_t stationList;    /*Number of connected stations*/
  esp_wifi_ap_get_sta_list(&stationList);
  //Serial.print("N of connected stations: ");
 // Serial.println(stationList.num);
  for (int i = 0; i < stationList.num; i++)
  {
    wifi_sta_info_t station = stationList.sta[i];
    for (int j = 0; j < 6; j++)
    {
      char str[3];
      sprintf(str, "%02x", (int)station.mac[j]);  /*prints MAC address of connected station*/
      macAddresses += str;
      if (j < 5)
      {
        macAddresses += ":";
      }
    }
   // Serial.println();
  }
 // Serial.println(macAddresses);
 // Serial.println("-----------------");
}

String generatePage()
{
  String page = String(MAIN_page);
  page.replace("%MAC%", macAddresses);
  return page;
}


void setup()
{
  Serial.begin(115200);
  pinMode(led,OUTPUT);
  pinMode(buzz,OUTPUT);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println("");
  Serial.print("Connected to WiFi");
  Serial.print("Access IP address: ");
  Serial.println(local_ip);
  Serial.println("Setting up WiFi");

  server.on("/", handleRoot);
  server.on("/action_page", handleForm);
  server.begin();

  Serial.println("HTTP server started");
//  preferences.begin("passwords", true); // Open preferences with namespace "passwords"
//    for (int i = 0; i < MAX_ENTRIES; i++) {
//    char macKey[18];
//    char passKey[18];
//    sprintf(macKey, "mac_%d", i); // Generate the key for MAC address
//    sprintf(passKey, "pass_%d", i); // Generate the key for password
//    Serial.println(macKey);
//    Serial.println(passKey);
//    String storedMac = preferences.getString(macKey, "");
//    String storedPass = preferences.getString(passKey, "");
//    Serial.println(storedMac);
//    Serial.println(storedPass);
//  }
//  preferences.end();

}

void handleRoot()
{
  String page = generatePage();
  server.send(200, "text/html", page);
}

void handleForm()
{
  String mac = server.arg("MAC");
  String pass = server.arg("PASS");
  Serial.print("CLIENT MAC:");
  Serial.println(mac);
  Serial.print("MAC PASS:");
  Serial.println(pass);
  savePassword(mac.c_str(), pass.c_str());

  if(new_entry==true)
  {
    new_entry=false;
    String s = "<h1>New Mac Address Added!</h1><a href='/'> Go Back </a>";
     server.send(200, "text/html", s);
  }
   else if(pass_present==true)
  {
    pass_present=false;
    String s = "<h1>Please Enter a Unique Password for this Mac</h1><a href='/'> Go Back </a>";
     server.send(200, "text/html", s);
  }
   else if(pass_updated==true)
  {
    pass_updated=false;
    String s = "<h1>Mac Address was Already Present Pass Updated!</h1><a href='/'> Go Back </a>";
     server.send(200, "text/html", s);
  }
  

}

void loop()
{
  PrintStations();
  server.handleClient();
  if(macAddresses!="")
  {
    check_pass();
  }
  else
  {
   digitalWrite(led,LOW);
  }
}

void check_pass()
{
  char key = keypad.getKey();

  if (key)
  {
    if (key == '*') 
    {
      keypad_password = ""; // Clear the entered password if "*" is pressed
    } 
    else if (key == '#') 
    {
      bool Authentication=checkPassword(macAddresses.c_str(), keypad_password.c_str());
      if(Authentication)
      {
        Serial.println("2 Factor Authenticated!");
        digitalWrite(led,HIGH);
      }
      else
      {
        Serial.println("2 Factor Authentication Failed!");
        digitalWrite(buzz,HIGH);
        delay(2000);
        digitalWrite(buzz,LOW);
      }
      keypad_password = ""; // Clear the entered password
    }
    else
    {
      keypad_password += key; // Append the entered key to the password
    }
   // Serial.println(keypad_password);
  }
}
