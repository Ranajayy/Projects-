// NOTE : Below is a String I use to add to the data sent to the Nextion Display
//        And to verify the end of the string for incoming data.
//        Replace 3 x Serial2.write(0xff);
String  endChar = String(char(0xff)) + String(char(0xff)) + String(char(0xff));
String  dfd  = ""; // data from display
// NOTE : General Async Delay
unsigned	long asyncDelay = 0;// NOTE : 4,294,967,295
int delayLength = 10000;
// NOTE : Comment out the next two lines if using an Arduino with more than 1 serial port(MEGA)
//#include <SoftwareSerial.h>
//SoftwareSerial Serial2(2, 3); // RX, TX

#include <WiFi.h>
#include <Preferences.h>

Preferences preferences;
String SSIDname = "NoName";//CheapControls -- Engineering
String SSIDpassword = "NoPass";//controls  -- testtest

void setup()
{

  Serial.begin(9600);
  Serial2.begin(9600);
  pinMode(2, OUTPUT);
  String ssid, pass = "";
  int cnt = 0;
  preferences.begin("wifi", false);
  ssid = preferences.getString("ssid", "");
  pass = preferences.getString("password", "");

  if (ssid == "" || pass == "")
  {
    Serial.println("No values saved for ssid or password");
  }
  else
  {
    WiFi.begin(ssid.c_str(), pass.c_str());
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      if (cnt == 20)
      {
        break;
      }
      delay(500);
      cnt++;
    }
  }



}

void loop() {
  // NOTE : COLLECT CHARACTERS FROM NEXTION DISPLAY
  if (Serial2.available()) {
    inputFunction();
  }
  // NOTE : ASYNC DELAY
  if (millis() > asyncDelay) {
    asyncDelay += delayLength;
    delayFunction();
  }
  //NOTE : SOMETHING SENT FROM NEXTION I.E. GET REQEUEST
  if (dfd.endsWith(endChar)) {
    Serial.println(dfd);
    Serial.println("error");
    dfd = "";
  }
}
