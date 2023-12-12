#include <WiFi.h>
#include <Preferences.h>
#include <WebServer.h>
#include "RTClib.h"
#include <Wire.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include "OneButton.h"


#define API_KEY "AIzaSyC0vdd0Mj6Qvr5ezETebhMZ_JyMAjkoOaE"
#define DATABASE_URL "https://esp32-firebase-test1-2eace-default-rtdb.firebaseio.com/"

#define pressure_sens_pin 34
#define transducer_pin 35

#define power_button_pin 23
#define floor_up_but_pin 13
#define floor_down_but_pin 18

#define wifi_ssid_setbut 32

#define floor_up_but_pin 13
#define floor_down_but_pin  18
#define power_button_pin  23

OneButton btn = OneButton(floor_up_but_pin, false, false);
OneButton btn2 = OneButton(floor_down_but_pin, false, false);
OneButton btn3 = OneButton(power_button_pin, false, false);

String uid;

String power_buton_time = "";
String down_buton_time = "";
String up_buton_time = "";

int p_b, power_b, up_b, dwn_b = 0;
String WIFI_SSID, WIFI_PASS;

FirebaseJson json;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseData fbdo;

Preferences settings;
IPAddress local_ip(192, 168, 1, 4);
IPAddress gateway(192, 168, 1, 4);
IPAddress subnet(255, 255, 255, 0);

bool wifi_flag = false;
WebServer server(80); // Server on port 80
bool wifi_sta_connect = false;


String pressure_val = "";
String transducer_val = "";
String pool_id = "";
String parentPath = "";
bool state = false;

String Time = "";
String Date = "";
String prev_date = "";
int c1, c2, c3 = 0;
unsigned long request_time = 5000;
unsigned long request_millis = 0;
unsigned long request_millis_2 = 0;
unsigned long request_time_2 =  15000;  //30 mins=1800000 miliseconds for now 10 seconds

int mx = 0;
int prev_mx = -1;

bool ft_chk = true;

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<h2>POOL FLOOR UP<h2>
<h3> HTML Form ESP32</h3>

<form action="/action_page">
  WIFI SSID:<br>
  <input type="text" name="SSID" value="">
  <br>
  WIFI PASS:<br>
  <input type="text" name="PASS" value="">
  <br>  
  POOL ID:<br>
  <input type="text" name="ID" value="">
  <br>  
  CURENT TIME [FORMAT:YYYYMMDDHHMMSS]:<br>
  <input type="text" name="TIME" value="">
  <br> 
  <br>
  <input type="submit" value="Submit">
</form> 

</body>
</html>
)=====";



void handleRoot()
{
    String s = MAIN_page;             // Read HTML contents
    server.send(200, "text/html", s); // Send web page
}

void handleForm()
{
  String time_page="";
  String ssid = server.arg("SSID");
  String pass = server.arg("PASS");
  pool_id = server.arg("ID");
  time_page=server.arg("TIME");
  Serial.print("WIFI SSID:");
  Serial.println(ssid);

  Serial.print("WIFI PASS:");
  Serial.println(pass);

  Serial.print("Pool Id is: ");
  Serial.println(pool_id);

  settings.begin("credentials", false);
  settings.putString("ssid", ssid);
  settings.putString("password", pass);
  settings.putString("id", pool_id);
  Serial.println("Network Credentials Saved using Preferences");
  settings.end();
  adjust_time_rtc(time_page);
  wifi_sta_connect = true;
  String s = "<a href='/'> Go Back </a>";
  server.send(200, "text/html", s);
}

void setup()
{

    Serial.begin(115200);

    pinMode(wifi_ssid_setbut, INPUT);


    btn.attachLongPressStart(up_btn_send);
    btn2.attachLongPressStart(dwn_btn_send);
    btn3.attachLongPressStart(pow_btn_send);
    
    btn.attachLongPressStop(up_btn_send_off);
    btn2.attachLongPressStop(dwn_btn_send_off);
    btn3.attachLongPressStop(pow_btn_send_off);
    
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        while (1);
    }

      rtc.adjust(DateTime(__DATE__, __TIME__));
      request_millis = millis();
     

}

void loop()
{

    server.handleClient();
    btn.tick();
    btn2.tick();
    btn3.tick();
    wifi_credentials_update();
    transducer_val = transducer_code();
    pressure_val = pressure_code();
    time_rtc_check();

    
    if (wifi_sta_connect == true)
    {
        wifi_sta_connect = false;
        wifi_connect();
    }

    if (WiFi.status() == WL_CONNECTED)
    {
         // request after every 5 seconds to update trasnducer and button states on database
        if (millis() - request_millis > request_time)
        {
           send_normal_data();
           request_millis = millis();
        }

        if (millis() - request_millis_2 > request_time_2||ft_chk==true)
        {
           ft_chk=false;
           send_pressure_trasnducer_data();
           request_millis_2 = millis();
        }

       // check if any button was pressed and update database
        if (state == true)
        {
            send_button_data();
            state = false;
        }
    }
}


void time_rtc_check()
{
     char buf1[] = "hh";
    char buf2[] = "mm";
    char buf3[] = "ss";
    char buf4[]="YYYY";
    char buf5[]="MM";
    char buf6[]="DD";
    DateTime now = rtc.now();
    Time = String(now.toString(buf4)) + "/" + String(now.toString(buf5)) + "/" + String(now.toString(buf6)) + "  " + String(now.toString(buf1)) + ":" + String(now.toString(buf2)) + ":" + String(now.toString(buf3));
    Date=String(now.toString(buf4)) + "," + String(now.toString(buf5)) + "," + String(now.toString(buf6));

}

void send_button_data()
{
    if (Firebase.ready())
    {
        Serial.println("sending data to firebase!");
        parentPath = "/"+pool_id+"/";
        String history_path = "/History_"+pool_id+"-up_b/"+String(up_b);        
        String history_path2 = "/History_"+pool_id+"-pow_b/"+ String(power_b);
        String history_path3 = "/History_"+pool_id+"-dwn_b/"+ String(dwn_b);

        json.set("transducer/value", transducer_val);
        json.set("pressure/value", pressure_val);
        if (power_buton_time != "")
            json.set("power_button/pressed_at", power_buton_time);

        if (up_buton_time != "")
            json.set("up_button/pressed_at", up_buton_time);

        if (down_buton_time != "")
            json.set("down_button/pressed_at", down_buton_time);

        Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
        json.clear();

        if (up_buton_time != ""&& up_buton_time.indexOf("OFF")<0)
        {
            json.set("/up_button_details/", up_buton_time);
            Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, history_path.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
            json.clear();
        }
        if (power_buton_time != ""&& power_buton_time.indexOf("OFF")<0)
        {
            json.set("/power_button_details/", power_buton_time);
            Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, history_path2.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
            json.clear();
        }
        if (down_buton_time != "" && down_buton_time.indexOf("OFF")<0)
        {
            json.set("/down_button_details/", down_buton_time);
            Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, history_path3.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
            json.clear();
        }
    }
}


void send_pressure_trasnducer_data()
{
    parentPath = "/"+pool_id+"/";
    String history_path = "/"+pool_id+"-"+String(Date)+"/"+String(p_b);
            if (p_b>48||Date!=prev_date)
            {
              prev_date=Date;
                p_b = 0;
            }
    if (Firebase.ready())
    {          
          if(mx>prev_mx)
          {
            prev_mx=mx;
            json.set("/presure_time/", "@"+String(Time)+"@");
            json.set("/presure_details/", "@"+pressure_val+"@");
            Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, history_path.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
            json.clear();
            p_b++;
         }
    }
    
}

void send_normal_data()
{
        parentPath = "/"+pool_id+"/";
        json.set("transducer/value", transducer_val);
        json.set("pressure/value", pressure_val);

        if (power_buton_time != "")
            json.set("power_button/pressed_at", power_buton_time);

        if (up_buton_time != "")
            json.set("up_button/pressed_at", up_buton_time);

        if (down_buton_time != "")
            json.set("down_button/pressed_at", down_buton_time);

        Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
        json.clear();
  
}

void wifi_connect()
{
    settings.begin("credentials", false);

    WIFI_SSID = settings.getString("ssid", "");
    WIFI_PASS = settings.getString("password", "");
    pool_id= settings.getString("id", "");
    wifi_sta_connect = false;
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.println("connected to wifi!");
    config.api_key = API_KEY;
    auth.user.email = "rana_jayy123@gmail.com";
    auth.user.password = "pIoneer090";
    config.database_url = DATABASE_URL;
    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    // Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    // Assign the maximum retry of token generation
    config.max_token_generation_retry = 5;

    // Initialize the library with the Firebase authen and config
    Firebase.begin(&config, &auth);

    // Getting the user UID might take a few seconds
    Serial.println("Getting User UID");
    while ((auth.token.uid) == "")
    {
        Serial.print('.');
        delay(1000);
    }
    // Print user UID
    uid = auth.token.uid.c_str();
    Serial.print("User UID: ");
    Serial.println(uid);
   
}

void wifi_credentials_update()
{
    if (digitalRead(wifi_ssid_setbut) == HIGH)
    {
        if (wifi_flag == false)
        {

            WiFi.mode(WIFI_AP);
            WiFi.softAP("esp32", "123456789");
            WiFi.softAPConfig(local_ip, gateway, subnet);
            Serial.println("");
            Serial.print("Connected to ");
            Serial.println("WiFi");
            Serial.print("Access IP address: ");
            Serial.println(local_ip);
            Serial.print(" to set WIFI");
            server.on("/", handleRoot);
            server.on("/action_page", handleForm);
            server.begin();
            Serial.println("HTTP server started");
            wifi_flag = true;
        }
    }

    else
    {
       if(wifi_flag==true)
       {
         wifi_flag = false;
         ESP.restart();
       }
       
        settings.begin("credentials", false);

        WIFI_SSID = settings.getString("ssid", "");
        WIFI_PASS = settings.getString("password", "");
        pool_id= settings.getString("id", "");
        if (WIFI_SSID == "" || WIFI_PASS == ""||pool_id=="")
        {
            Serial.println("No credentials found!");
        }
        else
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                wifi_sta_connect = true;
            }
        }
    }
}

String transducer_code()
{
    int vals = analogRead(transducer_pin);
    int x = map(vals, 0, 4095, 0, 240);
    if(x<=100)
    {
      x=x+6;
    }
    return String(x);
}

String pressure_code()
{
    int valc = analogRead(pressure_sens_pin);
    mx = map(valc, 0, 4095, 0, 160);

    return String(mx);
}

void up_btn_send()
{
  
  state=true;
  up_buton_time = "ON " + String(Time);
  up_b++;
}

void dwn_btn_send()
{
  state=true;
  down_buton_time = "ON " + String(Time);
  dwn_b++;
}
void pow_btn_send()
{
  
  state=true;
  power_buton_time = "ON " + String(Time);
  power_b++;
}

void up_btn_send_off()
{
  state=true;
  up_buton_time ="OFF "+String(Time);
}

void dwn_btn_send_off()
{
  state=true;
  down_buton_time ="OFF "+String(Time);
}

void pow_btn_send_off()
{
  state=true;
  power_buton_time ="OFF "+String(Time);
}

void adjust_time_rtc(String d)
{

  String input;
  int year, month, day, hour, minute, second;

  Serial.println("Enter date and time in YYYYMMDDhhmmss format: ");
  while (input.length() < 14) 
  {
    input = d;
    input.trim();
  }

  year = input.substring(0, 4).toInt();
  month = input.substring(4, 6).toInt();
  day = input.substring(6, 8).toInt();
  hour = input.substring(8, 10).toInt();
  minute = input.substring(10, 12).toInt();
  second = input.substring(12, 14).toInt();

  Serial.println(year);
  Serial.println(month);
  Serial.println(day);
  Serial.println(hour);
  Serial.println(minute);
  Serial.println(second);
 rtc.adjust(DateTime(year, month, day, hour, minute, second));

}
