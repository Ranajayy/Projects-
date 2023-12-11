#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESP_Mail_Client.h>
#include "time.h"


const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;


#define BUTTON_PIN_BITMASK 0x9000000000
#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP  30


int day_count = 0;
int prev_day_count = 0;
int vd_level = 0 ;
float vd_voltage = 0;
float battery_voltage = 0;
int normal_mode = 0;
bool io_wakeup_flag = false;
bool low_bat_flag = false;
bool timer_flag = false;
bool wifi_setup_flag = false;
//input_output
#define reed_switch 36
#define butt_1 39
#define buzz 22
#define battery_pin 34
#define led_pin 23
#define OUTPUTX 21

String current_day = "";
String prev_day = "";
int start_up = 0;
int but_fl = 0;
int reed_fl = 0;
int bat_fl = 0;
int but_count_fl, but_counter = 0;
unsigned long led_millis, reed_millis, bat_millis , butt1_millis, time_millis = 0;
bool reed_flag = false;
int wifi_count = 0;
//mail settings//
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* The sign in credentials */
#define AUTHOR_EMAIL "testoctathorn@gmail.com"
#define AUTHOR_PASSWORD "jcujcijsnylstnqk"
SMTPSession smtp;
void smtpCallback(SMTP_Status status);

String email_set_user = "";

Preferences settings;
String WIFI_SSID = "";
String WIFI_PASS = "";
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80); //Server on port 80
bool wifi_sta_connect = false;

bool reed_mail_flag = false;
bool butt_mail_flag = false;

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<h2>Octathorn<h2>
<h3> HTML Form ESP32</h3>

<form action="/action_page">
  WIFI SSID:<br>
  <input type="text" name="SSID" value="">
  <br>
  WIFI PASS:<br>
  <input type="text" name="PASS" value="">
  <br>
   Email:<br>
  <input type="text" name="mail" value="@gmail.com">
  <br>
   Phone #:<br>
  <input type="text" name="mobile" value="">
  <br>
Select Your Carrier:
<select name="Cell">
<option>AT&T: number@txt.att.net
<option>Boost Mobile: number@sms.myboostmobile.com
<option>C-Spire: number@cspire1.com
<option>Consumer Cellular: number@mailmymobile.net
<option>Cricket: number@sms.cricketwireless.net
<option>Google Fi (Project Fi): number@msg.fi.google.com 
<option>Metro PCS: number@mymetropcs.com
<option>Mint Mobile: number@mailmymobile.net
<option>Page Plus: number@vtext.com 
<option>Republic Wireless: number@text.republicwireless.com
<option>Simple Mobile: number@smtext.com 
<option>Sprint: number@messaging.sprintpcs.com 
<option>T-Mobile:@tmomail.net
<option>Ting CDMA: number@message.ting.com 
<option>Ting GSM: number@tmomail.net
<option>U.S. Cellular: @email.uscc.net
<option>U.S. Mobile: number@vtext.com
<option>Verizon:@vtext.com
<option>Virgin Mobile:@vmobl.com
<option>Xfinity Mobile:@vtext.com
<option>Visible: number@vtext.com 
</select>
<br>  
  <br>
  <input type="submit" value="Submit">
</form> 

</body>
</html>
)=====";


String printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  
  }
 // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  String da=String(timeinfo.tm_mday);
return da;
}


void handleRoot() {
  String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}

void handleForm()
{
  String ssid = server.arg("SSID");
  String pass = server.arg("PASS");
  String mail = server.arg("mail");
  String mobile = server.arg("mobile");
  String Cell = server.arg("Cell");

  Serial.print("WIFI SSID:");
  Serial.println(ssid);

  Serial.print("WIFI PASS:");
  Serial.println(pass);

  Serial.print("Email:");
  Serial.println(mail);
  Serial.print("mobile:");
  Serial.println(mobile);
   String c=Cell.substring(Cell.indexOf('@'));
   email_set_user=mobile+c;
  Serial.println("carrier");
  Serial.println(email_set_user);

  settings.begin("credentials", false);
  settings.putString("ssid", ssid);
  settings.putString("password", pass);
  settings.putString("email",email_set_user);
  Serial.println("Network Credentials Saved using Preferences");
  settings.end();
  
  wifi_sta_connect = true;


  String s = "<a href='/'> Go Back </a>";
  server.send(200, "text/html", s);
}

void setup(void)
{
  Serial.begin(115200);

  
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  print_wakeup_reason();
  print_GPIO_wake_up();  
    
  pinMode(reed_switch,INPUT);
  pinMode(led_pin,OUTPUT);
  pinMode(buzz,OUTPUT);

  settings.begin("credentials", false);
  start_up=settings.getInt("start_up",0);


if(start_up==0)
{


  WiFi.softAP("esp32", "123456789");
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println("WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/action_page", handleForm);
  server.begin();
  Serial.println("HTTP server started");
   
}

else
{
    wifi_sta_connect = true;
}

}

void loop()
{
  
  server.handleClient();
  vd_level = analogRead(battery_pin);
  vd_voltage = float(3.3 / 4096) * vd_level;
  battery_voltage = float((vd_voltage * (5800 + 10000)) / 5800);
  
//low bat mode
if(battery_voltage<3)
{
  low_bat_flag=true;
  
  if(bat_fl==0)
  {
    bat_fl=1;
  bat_millis=millis();
  }
  if(millis()-bat_millis>30000)
  {
    Serial.println(battery_voltage);
    Serial.println("LOW battery mode");
    bat_millis=millis();
    tone(buzz, 1000, 100);
  }  
}

if(battery_voltage>3)
{

  low_bat_flag=false;
  bat_fl=0;
  bat_millis=millis();
  if(io_wakeup_flag==true)
  {
    normal_mode=0;
  }
  if(timer_flag==true)
  {
     normal_mode=1;
  }
  

}
  /////////////////

  
  if (wifi_sta_connect == true)
  {
    wifi_count=0;
    settings.begin("credentials", false);

    WIFI_SSID = settings.getString("ssid", "");
    WIFI_PASS = settings.getString("password", "");
    wifi_sta_connect = false;
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) 
    {
      Serial.print('.');
      delay(1000);
      if(wifi_count==10)
     {
      break;
    }
    
    wifi_count++;
    
    }
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      
      if(start_up==0)
      {
        Serial.println("will come hgere one time on start!");
          start_up=1;
          settings.begin("credentials", false);
          settings.putInt("start_up", start_up);
          settings.end();
          prev_day=printLocalTime();
          settings.begin("credentials", false);
          settings.putString("prev_day", prev_day);
          settings.end();       
      }

    Serial.println(WiFi.localIP());
    
    if(io_wakeup_flag==true)
    {
      normal_mode=0;
    }
    
     if(io_wakeup_flag==false)
    {
         normal_mode=1;
     }
     
   if(io_wakeup_flag==true && wifi_setup_flag==true)
  {
    wifi_setup_flag=false;
    normal_mode=1;
  }
  //Serial.println("normal_mode"+String(normal_mode));
  }

/////////////////////////////// normal mode ////////////

 if(normal_mode==1 && low_bat_flag==false)
 {
   normal_mode=0;
   Serial.println("hello");
   digitalWrite(led_pin,HIGH);
   delay(500);
   digitalWrite(led_pin,LOW);
  
   current_day=printLocalTime();
   Serial.println("date today is:"+current_day);
   Serial.println("date prev is:"+prev_day);
   
     settings.begin("credentials", false);
    prev_day = settings.getString("prev_day", "");

   
   if(current_day!=prev_day)
 {
  Serial.println("day has changed!");
  prev_day=current_day;
          settings.begin("credentials", false);
          settings.putString("prev_day", prev_day);
          settings.end();
          day_count++;
          settings.begin("credentials", false);
          settings.putInt("day_count", day_count);
          settings.end();
 }

     settings.begin("credentials", false);
    day_count = settings.getInt("day_count", 0);

  if(day_count>=7)
 {  //send_mail
   settings.begin("credentials", false);
    email_set_user = settings.getString("email", "");
    send_mail(email_set_user,"<div style=\"color:#2f4468;\"><h1>Normal MODE!</h1><p>- Sent from ESP_BlackWater board</p></div>");
  day_count=0;
 }

   esp_deep_sleep_start();

 }




///////////////alarm mode///////////////
if(digitalRead(reed_switch)==HIGH)
{
  if(reed_fl==0)
  {
    reed_fl=1;
  reed_millis=millis();
  }
  
  if(millis()-reed_millis>3000)
  {
    reed_flag=true;
     reed_millis=millis();
    digitalWrite(buzz,HIGH);
    Serial.println("in alarm mode");
    reed_mail_flag=true;

    
  }
  
}

else
{
  reed_fl=0;
   reed_millis=millis();
}




if(reed_flag==true && digitalRead(butt_1)==HIGH)
{
  Serial.println("alarm mode off!");
   reed_flag=false;
   digitalWrite(buzz,LOW);  
       if(reed_mail_flag==true)
       {
        reed_mail_flag=false;
           settings.begin("credentials", false);
          email_set_user = settings.getString("email", "");
        send_mail(email_set_user,"<div style=\"color:#2f4468;\"><h1>Alarm MODE!</h1><p>- Sent from ESP_BlackWater board</p></div>");
       }
}



//test mode
if(digitalRead(butt_1)==HIGH)
{
  if(but_fl==0)
  {
    but_fl=1;
     butt1_millis=millis();
  }
 // Serial.println("butt1 high");

  if(millis()-butt1_millis>3000)
  {
    Serial.println("in test mode");
    
     butt1_millis=millis();
   digitalWrite(buzz,HIGH);  
   butt_mail_flag=true;
   

  } 
}

else
{
  if(reed_flag==true)
  {
    reed_flag=true;
   }
   else
   {
  but_fl=0;
   butt1_millis=millis();
   digitalWrite(buzz,LOW);  
   if(butt_mail_flag==true)
   {
    butt_mail_flag=false;
       settings.begin("credentials", false);
    email_set_user = settings.getString("email", "");
    send_mail(email_set_user,"<div style=\"color:#2f4468;\"><h1>BLACK WATER TEST MODE!</h1><p>- Sent from ESP_BlackWater board</p></div>");
   }
  
}
}


// setup mode
if(digitalRead(butt_1)==HIGH)
{
  if(but_count_fl==0)
  {
      but_counter++;
      Serial.println(but_counter);
      delay(250);
    but_count_fl=1;
    time_millis=millis();
  }


  if(but_counter==3 )
  {
    Serial.println("setup mode!");
    but_count_fl=0;
    but_counter=0;
    WiFi.disconnect();
    WiFi.softAP("esp32", "123456789");
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println("WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/action_page", handleForm);
  server.begin();
  Serial.println("HTTP server started");
  wifi_setup_flag=true;
  io_wakeup_flag=false;
  }
}

else
{
     but_count_fl=0;
     if(millis()-time_millis>3000)
     {
      time_millis=millis();
      but_counter=0;
      but_count_fl=0;
     }

}

  ////test mode
//if(digitalRead(butt_1)==HIGH)
//{
//  if(but_fl==0)
//  {
//    but_fl=1;
//     butt1_millis=millis();
//  }
// // Serial.println("butt1 high");
//
//  if(millis()-butt1_millis>3000)
//  {
//    Serial.println("in test mode");
//     butt1_millis=millis();
//   digitalWrite(buzz,HIGH);  
//    //send mail
//  } 
//}
//
//else
//{
//  if(reed_flag==true)
//  {
//    reed_flag=true;
//   }
//   else
//   {
//  but_fl=0;
//   butt1_millis=millis();
//   digitalWrite(buzz,LOW);  
//}
//
//
//}

}

void smtpCallback(SMTP_Status status) 
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()) {
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");
  }
}

void send_mail(String RECIPIENT_EMAIL, String htmlMsg)
{
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the session config data */
  ESP_Mail_Session session;

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = "ESP_BlackWater";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "Mail from BlackWater Alert App";
  message.addRecipient("", RECIPIENT_EMAIL);

  /*Send HTML message*/

  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  if (!smtp.connect(&session))
    return;

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());

}

void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL");io_wakeup_flag=true; break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer");timer_flag=true; break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

/*
Method to print the GPIO that triggered the wakeup
*/
void print_GPIO_wake_up(){
  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  Serial.print("GPIO that triggered the wake up: GPIO ");
  Serial.println((log(GPIO_reason))/log(2), 0);
}
