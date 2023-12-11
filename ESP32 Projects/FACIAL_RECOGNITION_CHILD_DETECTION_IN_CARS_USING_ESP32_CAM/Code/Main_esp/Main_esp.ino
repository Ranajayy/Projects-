#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "esp_camera.h"
//#include "fd_forward.h"

HardwareSerial gsmserial(1);
HardwareSerial gpsserial(2);
TinyGPSPlus gps;
String smsStatus, senderNumber, receivedDate, msg, gsm = "";


BLEServer* pServer;
BLECharacteristic *Child_status = NULL;
BLECharacteristic *Location_status = NULL;
BLECharacteristic *battery_status = NULL;
BLECharacteristic *Alarm_status = NULL;
BLECharacteristic *GSM_status = NULL;
bool  deviceConnected = false;
std::string rcv4 = "";
std::string rcv3;
#define SERVICE_UUID          "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define child_uuid            "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define Location_uuid         "688091db-1736-4179-b7ce-e42a724a6a68"
#define Battery_uuid          "0515e27d-dd91-4f96-9452-5f43649c1819"
#define alarm_uuid            "de3ef5b0-333a-4d2f-8902-f777217e27e6"
#define gsm_uuid              "e8f97b98-e942-4ab5-a8dd-18c7d32ee961"

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


bool initCamera()
{

  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  esp_err_t result = esp_camera_init(&config);

  if (result != ESP_OK)
  {
    return false;
  }

  return true;
}
mtmn_config_t mtmn_config = {0};

class MyServerCallbacks: public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer)
    {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks1: public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *Child_status)
    {
      std::string rcv = Child_status->getValue();
      if (rcv.length() > 0)
      {
        Serial.print("recived:\n");
        for (int i = 0; i < rcv.length(); i++)
        {
          Serial.print(rcv[i]);
        }
      }
    }
};
class MyCallbacks2: public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *Location_status)
    {
      std::string rcv2 = Location_status->getValue();
      if (rcv2.length() > 0)
      {
        Serial.print("\nrecived2:");
        for (int i = 0; i < rcv2.length(); i++)
        {
          Serial.print(rcv2[i]);
        }
      }
    }
};
class MyCallbacks3: public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *battery_status)
    {
      rcv3 = battery_status->getValue();
      if (rcv3.length() > 0)
      {
        Serial.print("\nrecived3:");
        for (int i = 0; i < rcv3.length(); i++)
        {
          Serial.print(rcv3[i]);
        }
      }
    }
};
class MyCallbacks4: public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *Alarm_status)
    {
      rcv4 = Alarm_status->getValue();
      if (rcv4.length() > 0)
      {
        Serial.print("\nrecived3:");
        for (int i = 0; i < rcv4.length(); i++)
        {
          Serial.print(rcv4[i]);
        }
      }
    }
};


unsigned long battery_millis = 0;
unsigned long time_out = 0;
char bat[10];
int vd_level = 0 ;
float vd_voltage = 0;
float battery_voltage = 0;
float battery_percentage = 0;
double latt = 0;
double lngg = 0;
unsigned int len, gsm_index;
int child_2 = 0;



void setup()
{
  Serial.begin(115200);
  gsmserial.begin(9600, SERIAL_8N1, 13, 12);
  gpsserial.begin(9600, SERIAL_8N1, 14, 15);
  Serial.println("Starting BLE work!");


  BLEDevice::init("THIRD EYE");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  Child_status = pService->createCharacteristic(
                   child_uuid,
                   BLECharacteristic::PROPERTY_READ |
                   BLECharacteristic::PROPERTY_NOTIFY
                 );
  Child_status->setValue("Hi 1");
  Child_status->setCallbacks(new MyCallbacks1());
  Child_status->addDescriptor(new BLE2902());


  Location_status = pService->createCharacteristic(
                      Location_uuid,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  Location_status->setValue("Hi 2");
  Location_status->setCallbacks(new MyCallbacks2());
  Location_status->addDescriptor(new BLE2902());

  battery_status = pService->createCharacteristic(
                     Battery_uuid,
                     BLECharacteristic::PROPERTY_READ |
                     BLECharacteristic::PROPERTY_WRITE |
                     BLECharacteristic::PROPERTY_NOTIFY
                   );
  battery_status->setValue("Hi 3");
  battery_status->setCallbacks(new MyCallbacks3());
  battery_status->addDescriptor(new BLE2902());

  Alarm_status = pService->createCharacteristic(
                   alarm_uuid,
                   BLECharacteristic::PROPERTY_READ |
                   BLECharacteristic::PROPERTY_WRITE |
                   BLECharacteristic::PROPERTY_NOTIFY
                 );
  Alarm_status->setValue("Alarm");
  Alarm_status->setCallbacks(new MyCallbacks4());
  Alarm_status->addDescriptor(new BLE2902());


  GSM_status = pService->createCharacteristic(
                 gsm_uuid,
                 BLECharacteristic::PROPERTY_READ |
                 BLECharacteristic::PROPERTY_NOTIFY |
                 BLECharacteristic::PROPERTY_INDICATE

               );
  GSM_status->addDescriptor(new BLE2902());


  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();



  if (!initCamera())
  {

    Serial.printf("Failed to initialize camera...");
    return;
  }

  mtmn_config = mtmn_init_config();

  smsStatus = "";
  senderNumber = "";
  receivedDate = "";
  msg = "";

  gsmserial.print("AT+CMGF=1\r"); //SMS text mode
  pinMode(4, INPUT);// for battery
  pinMode(2, INPUT); //for child 2

}

void loop()
{
  while (gsmserial.available())
  {
    parseData(gsmserial.readString());
    //  Serial.print("in GSm loop!");
  }
  while (Serial.available())
  {
    gsmserial.println(Serial.readString());
    //Serial.print("in GSm loop!");
  }
  while (gpsserial.available() > 0)
  {
    if (gps.encode(gpsserial.read()))
    {
      display_gps_Info();
      //   Serial.print("in GPS loop!");
      break;
    }
  }
  vd_level = analogRead(4);
  // Serial.println(vd_level);
  vd_voltage = float(3.3 / 4096) * vd_level;
  battery_voltage = float((vd_voltage * (3300 + 12000)) / 12000);
  battery_percentage = float((battery_voltage - 3.4) / 0.8) * 100;
  //Serial.println( battery_percentage);
  dtostrf(battery_percentage, 1, 2, bat);
  char LAT[10];
  char LNG[10];
  dtostrf(latt, 1, 6, LAT);
  dtostrf(lngg, 1, 6, LNG);

  if (deviceConnected)
  {
    camera_fb_t * frame;
    frame = esp_camera_fb_get();

    dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, frame->width, frame->height, 3);
    fmt2rgb888(frame->buf, frame->len, frame->format, image_matrix->item);

    esp_camera_fb_return(frame);

    box_array_t *boxes = face_detect(image_matrix, &mtmn_config);
    int a = int(boxes);

//    if (a != NULL && digitalRead(2) == 1 )
//    {
//      Serial.print("both childs are present on the seat\n");
//      Child_status->setValue("3");
//      Child_status->notify();
//      Location_status->setValue(LAT);
//      Location_status->notify();
//      Location_status->setValue(LNG);
//      Location_status->notify();
//      delay(3);
//    }

    if (boxes != NULL)
    {
      Serial.print("child1 present on seat\n");
      dl_lib_free(boxes->score);
      dl_lib_free(boxes->box);
      dl_lib_free(boxes->landmark);
      dl_lib_free(boxes);

      Child_status->setValue("1");
      Child_status->notify();
      Location_status->setValue(LAT);
      Location_status->notify();
      Location_status->setValue(LNG);
      Location_status->notify();
      delay(3);
    }


    else if (digitalRead(2) == 1)
    {
      Serial.print("child2 is present on the seat\n");

      Child_status->setValue("2");
      Child_status->notify();
      Location_status->setValue(LAT);
      Location_status->notify();
      Location_status->setValue(LNG);
      Location_status->notify();
      delay(3);
    }

    else if (boxes == NULL&&digitalRead(0)==0)
    {
      Serial.println("There is no child1 on the Seat");
      Child_status->setValue("4");
      Child_status->notify();

      delay(3);
    }

    dl_matrix3du_free(image_matrix);

  }

  //  if (deviceConnected)
  //  {
  //
  //    if (millis() - battery_millis > 5)
  //    {
  //      battery_status->setValue(bat);
  //      battery_status->notify();
  //      delay(3);
  //
  //      battery_millis = millis();
  //    }


  //      gsmserial.println("AT");
  //      time_out = millis();
  //      while (!gsmserial.available())
  //      {
  //        if (millis() - time_out > 10000)
  //        {
  //          GSM_status->setValue("ERROR!");
  //          GSM_status->notify();
  //          break;
  //        }
  //      }
  //      if (gsmserial.available())
  //      {
  //        gsm = gsmserial.readString();
  //        //Remove sent "AT Command" from the response string.
  //        gsm_index = gsm.indexOf("\r");
  //        gsm.remove(0, gsm_index + 2);
  //        gsm.trim();
  //        if (gsm == "OK")
  //        {
  //          GSM_status->setValue("OK");
  //          GSM_status->notify();
  //        }
  //
  //      }
  //    }


  //    if (battery_percentage <= 10)
  //    {
  //      battery_status->setValue("LOW Battery!");
  //      battery_status->notify();
  //      camera_fb_t * frame;
  //      frame = esp_camera_fb_get();
  //
  //      dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, frame->width, frame->height, 3);
  //       fmt2rgb888(frame->buf, frame->len, frame->format, image_matrix->item);
  //
  //      esp_camera_fb_return(frame);
  //
  //        box_array_t *boxes = face_detect(image_matrix, &mtmn_config);
  //      if (boxes != NULL)
  //      {
  //        Child_status->setValue("1 Low battery");
  //        Child_status->notify();
  //        dl_lib_free(boxes->score);
  //        dl_lib_free(boxes->box);
  //        dl_lib_free(boxes->landmark);
  //        dl_lib_free(boxes);
  //        esp_sleep_enable_timer_wakeup(5 * 1000000); //60 secs
  //        esp_deep_sleep_start();

  //      }
  //      else if (digitalRead(2) == 1)
  //      {
  //        Child_status->setValue("2 Low battery");
  //        Child_status->notify();
  //        esp_sleep_enable_timer_wakeup(5 * 1000000); //60 secs
  //        esp_deep_sleep_start();
  //
  //      }
  //      else if (boxes == NULL && deviceConnected == false)
  //      {
  //        esp_sleep_enable_timer_wakeup(5 * 1000000); //60 secs
  //        esp_deep_sleep_start();
  //
  //      }
  //      else if (boxes == NULL && deviceConnected)
  //      {
  //
  //        Child_status->setValue("0");
  //        Child_status->notify();
  //        esp_sleep_enable_timer_wakeup(5 * 1000000); //120 secs
  //        esp_deep_sleep_start();
  //      }
  //
  //      dl_matrix3du_free(image_matrix);
  //
  //    }
  //
  //
  //    else if ( gsm != "OK")
  //    {
  //      GSM_status->setValue("ERROR!");
  //      GSM_status->notify();
  //      camera_fb_t * frame;
  //    frame = esp_camera_fb_get();
  //
  //    dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, frame->width, frame->height, 3);
  //    fmt2rgb888(frame->buf, frame->len, frame->format, image_matrix->item);
  //
  //    esp_camera_fb_return(frame);
  //
  //    box_array_t *boxes = face_detect(image_matrix, &mtmn_config);
  //      if (boxes != NULL)
  //      {
  //        Child_status->setValue("1 GSM ERROR!");
  //        Child_status->notify();
  //        dl_lib_free(boxes->score);
  //        dl_lib_free(boxes->box);
  //        dl_lib_free(boxes->landmark);
  //        dl_lib_free(boxes);
  //        esp_sleep_enable_timer_wakeup(5 * 1000000); //60 secs
  //        esp_deep_sleep_start();

  //      }
  //      else if (digitalRead(2) == 1)
  //      {
  //        Child_status->setValue("2 GSM ERROR!");
  //        Child_status->notify();
  //        esp_sleep_enable_timer_wakeup(5 * 1000000); //60 secs
  //        esp_deep_sleep_start();
  //
  //      }
  //      else if (boxes == NULL && deviceConnected == false)
  //      {
  //        esp_sleep_enable_timer_wakeup(5 * 1000000); //60 secs
  //        esp_deep_sleep_start();
  //
  //      }
  //      else if (boxes == NULL && deviceConnected)
  //      {
  //
  //        Child_status->setValue("0");
  //        Child_status->notify();
  //        esp_sleep_enable_timer_wakeup(5 * 1000000); //120 secs
  //        esp_deep_sleep_start();
  //      }
  //
  //      dl_matrix3du_free(image_matrix);
  //    }
  //  }

//  if (deviceConnected == false)
//  {
//    Serial.print("no device connected\n");
//    camera_fb_t * frame;
//    frame = esp_camera_fb_get();
//
//    dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, frame->width, frame->height, 3);
//    fmt2rgb888(frame->buf, frame->len, frame->format, image_matrix->item);
//
//    esp_camera_fb_return(frame);
//
//    box_array_t *boxes = face_detect(image_matrix, &mtmn_config);
//    if (boxes != NULL)
//    {
//      dl_lib_free(boxes->score);
//      dl_lib_free(boxes->box);
//      dl_lib_free(boxes->landmark);
//      dl_lib_free(boxes);
//
//      String c1_latt = String(latt);
//      String c1_logg = String(lngg);
//      String ms = "warning child 1 is present and device not found!! LATITUDE IS: " + c1_latt + " LONGITUDE IS: " + c1_logg;
//
//      Serial.println(ms);
//      Send_message(ms, "+923052069794");// alert in message will be sent to phone!!// alert in message will be sent to phone!!
//      if (deviceConnected == true)
//      {
//        Alarm_status->setValue("1");
//        Alarm_status->notify();
//        esp_sleep_enable_timer_wakeup(5 * 1000000); //60 secs
//        esp_light_sleep_start();
//      }
//    }
//    else if (digitalRead(2) == 1)
//    {
//      String c2_latt = String(latt);
//      String c2_logg = String(lngg);
//      String ms = "warning child 2 is present and device not found!! LATITUDE IS: " + c2_latt + " LONGITUDE IS: " + c2_logg;
//
//      Serial.println(ms);
//      Send_message(ms, "+923052069794");// alert in message will be sent to phone!!// alert in message will be sent to phone!!
//
//      if (deviceConnected == true)
//      {
//        Alarm_status->setValue("1");
//        Alarm_status->notify();
//        esp_sleep_enable_timer_wakeup(5 * 1000000); //60 secs
//        esp_light_sleep_start();
//      }
//
//    }
//    dl_matrix3du_free(image_matrix);
//
//  }

  //      if(deviceConnected==false)
  //         {
  //            esp_sleep_enable_timer_wakeup(5 * 1000000); //deep sleep for 5 seconds and check again for connection
  //            esp_deep_sleep_start();
  //
  //         }
  else
  {
    Serial.println("no device");
  }
  delay(1000);
}

void display_gps_Info()
{
  if (gps.location.isValid())
  {
    latt = gps.location.lat();
    lngg = gps.location.lng();
    Serial.print("Latitude: ");
    Serial.println(latt, 6);
    Serial.print("Longitude: ");
    Serial.println(lngg, 6);
  }
  else
  {
    Serial.println("No GPS data available");
  }
  delay(1000);
}

void parseData(String buff)
{
  Serial.println(buff);

  unsigned int len, index;

  //Remove sent "AT Command" from the response string.
  index = buff.indexOf("\r");
  buff.remove(0, index + 2);
  buff.trim();

  if (buff != "OK")
  {
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();

    buff.remove(0, index + 2);

    if (cmd == "+CMTI") {
      //get newly arrived memory location and store it in temp
      index = buff.indexOf(",");
      String temp = buff.substring(index + 1, buff.length());
      temp = "AT+CMGR=" + temp + "\r";
      //get the message stored at memory location "temp"
      gsmserial.println(temp);
    }
    else if (cmd == "+CMGR")
    {
      extractSms(buff);


      //      if (senderNumber == PHONE)
      //      {
      //        doAction();
      //      }
      //    }
    }
    else {
      //The result of AT Command is "OK"
    }
  }
}

void extractSms(String buff)
{
  unsigned int index;

  index = buff.indexOf(",");
  smsStatus = buff.substring(1, index - 1);
  buff.remove(0, index + 2);

  senderNumber = buff.substring(0, 13);
  buff.remove(0, 19);

  receivedDate = buff.substring(0, 20);
  buff.remove(0, buff.indexOf("\r"));
  buff.trim();

  index = buff.indexOf("\n\r");
  buff = buff.substring(0, index);
  buff.trim();
  msg = buff;
  buff = "";
  msg.toLowerCase();
}

void Send_message(String text, String number)
{
  gsmserial.print("AT+CMGF=1\r");
  delay(1000);
  gsmserial.print("AT+CMGS=\"" + number + "\"\r");
  delay(1000);
  gsmserial.print(text);
  delay(100);
  gsmserial.write(0x1A); //ascii code for ctrl-26 //gsmserial.println((char)26); //ascii code for ctrl-26
  delay(1000);
  Serial.println("SMS Sent Successfully.");
}
