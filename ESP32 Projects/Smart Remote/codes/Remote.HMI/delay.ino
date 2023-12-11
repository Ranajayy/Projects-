
void delayFunction() {
  digitalWrite(2, !(digitalRead(2)));
  // NOTE : STUFF HERE
  Serial.println(SSIDname + " : " + SSIDpassword);
  Serial.println(WiFi.localIP());
  Serial.println("STATUS : " + String(WiFi.status()));
  //  0: WL_IDLE --temp      if WiFibegin is called
  //  1: WL_NO_SSID_AVAIL    No SSID avail
  //  2: WL_SCAN_COMPLETE    Network SSID scan complete
  //  3: WL_CONNECTED        connected to SSID
  //  4: WL_CONNECT_FAILED   failed to connect to SSID
  //  5: WL_CONNECTION_LOST  connection is lost
  //  6: WL_DISCONNECTED     disconnected from network
  if (WiFi.status() == 3)
  {
    preferences.begin("wifi", false);
    preferences.putString("ssid", SSIDname);
    preferences.putString("password", SSIDpassword);
    Serial.println("Network Credentials Saved using Preferences");
    preferences.end();
    Serial2.print("r0.pco=1024" + endChar);
    Serial2.print("r0.bco=1024" + endChar);


    Serial2.print("page0.t1.txt=");
    Serial2.write(0x22);
    Serial2.print("Connected  to : " + String(WiFi.SSID()));
    Serial2.write(0x22);
    Serial2.write(0xFF);
    Serial2.write(0xFF);
    Serial2.write(0xFF);

    Serial2.print("page0.t1.txt=");
    Serial2.write(0x22);
    Serial2.print("Connected  to : " + String(WiFi.SSID()));
    Serial2.write(0x22);
    Serial2.write(0xFF);
    Serial2.write(0xFF);
    Serial2.write(0xFF);
  } else {
    Serial2.print("r0.pco=63488" + endChar);
    Serial2.print("r0.bco=63488" + endChar);

    Serial2.print("page0.t1.txt=");
    Serial2.write(0x22);
    Serial2.print("Not Connected");
    Serial2.write(0x22);
    Serial2.write(0xFF);
    Serial2.write(0xFF);
    Serial2.write(0xFF);

    Serial2.print("page0.t1.txt=");
    Serial2.write(0x22);
    Serial2.print("Not Connected");
    Serial2.write(0x22);
    Serial2.write(0xFF);
    Serial2.write(0xFF);
    Serial2.write(0xFF);
  }
}
