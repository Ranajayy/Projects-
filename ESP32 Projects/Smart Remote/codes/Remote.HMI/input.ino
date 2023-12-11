void inputFunction(){
  dfd += char(Serial2.read());
  if(dfd.length()>3 && dfd.substring(0,3)!="C:C") dfd="";
  else{
    if(dfd.substring((dfd.length()-1),dfd.length()) == "?"){
      String command = dfd.substring(3,6);
      String value = dfd.substring(6,dfd.length()-1);
      // NOTE : FOR TESTING
      Serial.println(command + " : " + value);
      // NOTE : TEST COMMAND "CMD"
      // NOTE : TEST COMMAND "GET"
      if(command == "GET"){
       // Serial2.print("xstr 0,110,320,40,0,BLACK,0,1,1,3,\"SCANNING...\"" + endChar);
        int n = WiFi.scanNetworks();
        String SSIDs = "";
        for (int i = 0; i < n; ++i) {
          if(i > 0)SSIDs += "\r\n";
          SSIDs += WiFi.SSID(i);
          Serial.println(WiFi.SSID(i));
        }
        Serial2.print("cb0.txt=\"" + String(n) + " Networks\"" + endChar);
        Serial2.print("cb0.path=\"" + SSIDs + "\"" + endChar);
      //  Serial2.print("xstr 0,110,320,40,0,WHITE,0,1,1,3,\"SCANNING...\"" + endChar);
      }
      if(command == "CON"){
        int colonLoc=value.indexOf(":");
        SSIDname = value.substring(0,colonLoc);
        SSIDpassword = value.substring(colonLoc+1);
        WiFi.begin(SSIDname.c_str(),SSIDpassword.c_str());
      }
      dfd="";
    }
  }
}
