#include <HCSR04.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>


#define servo_1_pin 14
#define servo_2_pin 27

// Initialize the servo objects
Servo servo_1;
Servo servo_2;



#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define  ir_1 26
#define ir_2 13

bool ir1_flag = false;
bool ir2_flag = false;

HCSR04 hc(18, 19); //initialisation class HCSR04 (trig pin , echo pin)
HCSR04 hc1(17, 5 ); //initialisation class HCSR04 (trig pin , echo pin)
HCSR04 hc2(16, 4); //initialisation class HCSR04 (trig pin , echo pin)
HCSR04 hc3(25, 33 ); //initialisation class HCSR04 (trig pin , echo pin)

int car_count = 4;
int car_thresh = 20;
bool parking_full = false;
bool u1 = true;
bool u2 = true;
bool u3 = true;
bool u4 = true;
unsigned long close_delay = 2000;


void setup()
{
  Serial.begin(115200);

  pinMode(ir_1, INPUT);
  pinMode(ir_2, INPUT);

  servo_1.attach(servo_1_pin);
  servo_2.attach(servo_2_pin);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.display();
  display.clearDisplay();
  display.setTextSize(1);
  display.invertDisplay(false);
  display.setTextColor(WHITE); // Draw white text
  display.clearDisplay();
  display.setCursor(35, 10);
  display.print("Available");
  display.setCursor(25, 25);
  display.print("Parking Spots");
  display.setCursor(59, 40);
  display.print(String(car_count));
  display.display();

}

void loop()
{
  parking_check();
  full_parking();
  entry_code();
  exit_code();
}

void update_car_count()
{
  display.clearDisplay();
  display.setCursor(35, 10);
  display.print("Available");
  display.setCursor(25, 25);
  display.print("Parking Spots");
  display.setCursor(59, 40);
  display.print(String(car_count));
  display.display();
}

void entry_code()
{
  while (digitalRead(ir_1) == LOW)
  {
    if (ir1_flag == false && parking_full == false)
    {
      ir1_flag = true;
      servo_1.write(90);
      delay(close_delay);
      servo_1.write(0);
    }
  }


  if (digitalRead(ir_1) == HIGH)
  {
    ir1_flag = false;
  }
}

void exit_code()
{
  while (digitalRead(ir_2) == LOW)
  {
    if (ir2_flag == false)
    {
      ir2_flag = true;
      servo_2.write(90);
      delay(close_delay);
      servo_2.write(0);
    }
  }

  if (digitalRead(ir_2) == HIGH)
  {
    ir2_flag = false;
  }
}

void full_parking()
{
  if (int(hc.dist()) <= car_thresh && int(hc1.dist()) <= car_thresh && int(hc2.dist()) <= car_thresh && int(hc3.dist()) <= car_thresh)
  {
    parking_full = true;
  }
  else
  {
    parking_full = false;
  }
}

void parking_check()
{
  if (int(hc.dist()) <= car_thresh && u1 == false)
  {
    u1 = true;
    if (car_count != 0)
    {
      car_count--;
      if (car_count > 0)
      {
        update_car_count();
      }
      else
      {
        display.clearDisplay();
        display.setCursor(25, 20);
        display.print("Parking Full");
        display.display();
      }
    }
  }

  else
  {
    if (int(hc.dist()) > car_thresh && u1 == true)
    {
      u1 = false;
      if (car_count != 3)
      {
        car_count++;
        update_car_count();
      }
    }
  }

  if (int(hc1.dist()) <= car_thresh && u2 == false)
  {
    u2 = true;
    if (car_count != 0)
    {
      car_count--;
      if (car_count > 0)
      {
        update_car_count();
      }
      else
      {
        display.clearDisplay();
        display.setCursor(25, 20);
        display.print("Parking Full");
        display.display();
      }
    }
  }

  else
  {
    if (int(hc1.dist()) > car_thresh && u2 == true)
    {
      u2 = false;
      if (car_count != 3)
      {
        car_count++;
        update_car_count();
      }
    }
  }

  if (int(hc2.dist()) <= car_thresh && u3 == false)
  {
    u3 = true;
    if (car_count != 0)
    {
      car_count--;
      if (car_count > 0)
      {
        update_car_count();
      }
      else
      {
        display.clearDisplay();
        display.setCursor(25, 20);
        display.print("Parking Full");
        display.display();
      }
    }
  }

  else
  {
    if (int(hc2.dist()) > car_thresh && u3 == true)
    {
      u3 = false;
      if (car_count != 3)
      {
        car_count++;
        update_car_count();
      }
    }
  }

  if (int(hc3.dist()) <= car_thresh && u4 == false)
  {
    u4 = true;
    if (car_count != 0)
    {
      car_count--;
      if (car_count > 0)
      {
        update_car_count();
      }
      else
      {
        display.clearDisplay();
        display.setCursor(25, 20);
        display.print("Parking Full");
        display.display();
      }
    }
  }

  else
  {
    if (int(hc3.dist()) > car_thresh && u4 == true)
    {
      u4 = false;
      if (car_count != 4)
      {
        car_count++;
        update_car_count();
      }
    }
  }
}
