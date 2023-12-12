#include <Preferences.h>
#include <OneButton.h>
#include <SPI.h>
#include <Wire.h>
#include "SparkFun_BNO08x_Arduino_Library.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Fonts/FreeSans9pt7b.h> // Include the FreeSans9pt7b font
#include <Fonts/FreeSans12pt7b.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SH1106 display(21, 22);

const float pi = 3.14159267;
float incline = 5.0;
float cosincline = cos(incline * (pi / 180));
int timer = 120;
double clicks = 0;
String inclineunit = "DEG";
String cosunit = "COS";
String timerunit = "s";
String clicksunit = "MOA";
float sidelevel = 0;
float batterylevel = 0.80;


Preferences preferences;




// IMU //
BNO08x myIMU;
float s_pitch, s_roll, s_yaw = 0;
float z_pitch = 0, z_roll = 0;
float level_thresh = 2.0; // change level of degree threshold here

// Buttons //
#define button1_pin 36
#define button2_pin 39
#define button3_pin 34
#define button4_pin 35
#define button5_pin 25

OneButton btn1 = OneButton(button1_pin, false, false);
OneButton btn2 = OneButton(button2_pin, false, false);
OneButton btn3 = OneButton(button3_pin, false, false);
OneButton btn4 = OneButton(button4_pin, false, false);
OneButton btn5 = OneButton(button5_pin, false, false);



//Arrays for Menu//
String menuItems[] = {"Set Timer", "Zero", "Calibration", "Clicks per Rev", "Unit", "Shot Count"};
int s_o_m = sizeof(menuItems) / sizeof(menuItems[0]);
String set_timer_opt[] = {"30s", "60s", "90s", "120s", "150s", "180s"};
int s_o_t = sizeof(set_timer_opt) / sizeof(set_timer_opt[0]);
String Zero_opt[] = {"Z Turret", "Z Inclination", "R Inclination", "Z Reticle", "R Reticle"};
int s_o_z = sizeof(Zero_opt) / sizeof(Zero_opt[0]);
String Calibration[] = {"Start Calibration"};
String Click_opt[] = {"60", "100", "120", "150"};
int s_o_c = sizeof(Click_opt) / sizeof(Click_opt[0]);
String Unit_opt[] = {"MIL", "MOA"};
int s_o_u = sizeof(Unit_opt) / sizeof(Unit_opt[0]);
String shot_opt[] = {"Shot Count A", "Shot Count B"};
int s_o_s = sizeof(shot_opt) / sizeof(shot_opt[0]);

int currentIndex = 0;

bool main_menu = false;
bool t_opt_m = false;
bool z_opt_m = false;
bool cpr_opt_m = false;
bool c_opt_m = false;
bool u_opt_m = false;
bool shot_m = false;
bool ts = false;

int count = 0;
int t = 0;
int p_time = 0;
int s_time = timer;
unsigned long time_pass = millis();


// encoder pinouts //

const int encoderPinA = 33;
const int encoderPinB = 32;
volatile long encoderPosition = 0;
int lastEncoderStateA = LOW;
int lastEncoderStateB = LOW;
long pos1 = 0, pos0 = 0;
int clicks_per_rev = 1;  //default value for clicks per revolution
int cal_click_val = 1; //default value for calibration value if no value saved in esp
bool Record = false;


// battery variables //

float battery_voltage = 0;
float vd_level = 0;
float vd_voltage = 0;
float percentage = 0;


// led pinouts //
#define red_led1 23
#define red_led2 19
#define green_led1 18
#define green_led2 5
#define green_led3 17
#define blue_led1 16
#define blue_led2 4


// shot coutns //
const float JERK_THRESHOLD = 20;
unsigned long settleStartTime = 0;  // Variable to store the start time of settling period
const unsigned long SETTLE_PERIOD = 5000;  // Settling period in milliseconds (5 seconds)
int shotCounter = 0;
int total_shots = 0;
bool isSettled = true;
bool t_shot = false;
bool c_shot = false;
bool p_shot_m = false;



void setup()
{
  Serial.begin(115200);
  check_cal();
  led_init();
  encoder_init();
  imu_init();
  oled_init();
  button_init();
}


void loop()
{

  button_ticks();
  battery();
  imu_data();
  led_control();

  if (!main_menu && !u_opt_m && !t_opt_m && !c_opt_m && !cpr_opt_m && !z_opt_m && !shot_m)
  {
    main_screen();
  }

  if (Record == true)
  {
    pos1 = encoderPosition;
  }
  if (c_shot)
  {
    drawshot();
  }
  if (t_shot)
  {
    drawTshot();
  }

}



// draw Menues //

void main_screen()
{
  s_pitch = s_pitch - z_pitch;
  s_roll = s_roll - z_roll;
  // Serial.println(s_roll);

  clicks = float(cal_click_val) / float( clicks_per_rev) * encoderPosition;


  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont(&FreeSans12pt7b);

  char buffer[10]; // Buffer to store the converted string

  if ((clicks >= 0) && (clicks < 9.99))
  {
    display.setCursor(48, 22);
    dtostrf(clicks, 2, 2, buffer); // Convert to string with 2 decimal places
    display.print(buffer);
  }
  else if ((clicks > 9.99) && (clicks < 99.99))
  {
    display.setCursor(35, 22);
    dtostrf(clicks, 3, 2, buffer); // Convert to string with 2 decimal places
    display.print(buffer);
  }
  else if (clicks > 99.99)
  {
    display.setCursor(27, 22);
    dtostrf(clicks, 4, 2, buffer); // Convert to string with 2 decimal places
    display.print(buffer);
  }
  else if ((clicks < 0) && (clicks > -9.99))
  {
    display.setCursor(40, 22);
    dtostrf(clicks, 2, 2, buffer); // Convert to string with 2 decimal places
    display.print(buffer);
  }
  else if (clicks < -9.99)
  {
    display.setCursor(27, 22);
    dtostrf(clicks, 3, 2, buffer); // Convert to string with 2 decimal places
    display.print(buffer);
  }


  if (s_roll > 2)
  {
    display.fillTriangle(12, 3, 4, 15, 20, 15, WHITE);
    display.fillRect(9, 15, 7, 12, WHITE);
  }
  else if (s_roll < -2)
  {
    display.fillTriangle(116, 3, 124, 15, 108, 15, WHITE);
    display.fillRect(113, 15, 7, 12, WHITE);
  }

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont(&FreeSans12pt7b); // Set the display to use FreeSans12pt7b font

  if (ts == true && millis() - time_pass > 1000)
  {
    time_pass = millis();
    timer = timer - 1;
    p_time = timer;
  }
  if (timer == 0)
  {
    ts = false;
    String stringWithoutS = removeCharacter(set_timer_opt[currentIndex], 's');
    timer = stringWithoutS.toInt();
  }

  display.setCursor(80, 62);
  display.print(String(timer));

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont(NULL); // Set the display to use FreeSans12pt7b font

  display.setCursor(70, 26);
  display.println(clicksunit);
  display.setCursor(1, 40);
  display.println(inclineunit);
  display.setCursor(1, 56);
  display.println(cosunit);
  display.setCursor(120, 56);
  display.println(timerunit);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont(&FreeSans9pt7b); // Set the display to use FreeSans12pt7b font
  display.setCursor(22, 46);
  display.println(incline, 1); //incline value should display to 1 decimal place (5.5 DEG)
  display.setCursor(22, 62);
  display.println(s_pitch, 3); //incline value should display to 3 decimal place (5.5 DEG)

  display.display();
}

void drawMenu(int index, bool highlight)
{
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);

  int visibleOptions = min(s_o_m, 3);
  int currentIndex = 0;

  for (int i = 0; i < visibleOptions; i++)
  {
    int optionIdx = (index + i) % s_o_m;

    if (highlight && i == currentIndex)
    {
      display.setTextColor(BLACK);
      display.fillRect(0, i * 18, 128, 18, WHITE);  // Adjusted rectangle height for better appearance
    }
    else
    {
      display.setTextColor(WHITE);
    }

    display.setCursor(5, i * 18 + 12);
    display.print(menuItems[optionIdx]);
  }

  display.display();

}

void drawTimerM(int index, bool highlight)
{
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);

  int visibleOptions = min(s_o_t, 3);
  int currentIndex = 0;

  for (int i = 0; i < visibleOptions; i++)
  {
    int optionIdx = (index + i) % s_o_t;

    if (highlight && i == currentIndex)
    {
      display.setTextColor(BLACK);
      display.fillRect(0, i * 18, 128, 18, WHITE);  // Adjusted rectangle height for better appearance
    }
    else
    {
      display.setTextColor(WHITE);
    }

    display.setCursor(5, i * 18 + 12);
    display.print(set_timer_opt[optionIdx]);
  }

  display.display();
}

void drawzero(int index, bool highlight)
{
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);

  int visibleOptions = min(s_o_z, 3);
  int currentIndex = 0;

  for (int i = 0; i < visibleOptions; i++)
  {
    int optionIdx = (index + i) % s_o_z;

    if (highlight && i == currentIndex)
    {
      display.setTextColor(BLACK);
      display.fillRect(0, i * 18, 128, 18, WHITE);  // Adjusted rectangle height for better appearance
    }
    else
    {
      display.setTextColor(WHITE);
    }

    display.setCursor(5, i * 18 + 12);
    display.print(Zero_opt[optionIdx]);
  }

  display.display();
}

void drawcal(int index, bool highlight)
{
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);
  display.setCursor(0, 18);
  display.print("Hold button 1 to");
  display.setCursor(0, 32);
  display.print("calibrate!");
  display.display();
}

void drawcpr(int index, bool highlight)
{
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);

  int visibleOptions = min(s_o_c, 3);
  int currentIndex = 0;

  for (int i = 0; i < visibleOptions; i++)
  {
    int optionIdx = (index + i) % s_o_c;

    if (highlight && i == currentIndex)
    {
      display.setTextColor(BLACK);
      display.fillRect(0, i * 18, 128, 18, WHITE);  // Adjusted rectangle height for better appearance
    }
    else
    {
      display.setTextColor(WHITE);
    }

    display.setCursor(5, i * 18 + 12);
    display.print(Click_opt[optionIdx]);
  }

  display.display();
}

void drawunit(int index, bool highlight)
{
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);

  int visibleOptions = min(s_o_u, 3);
  int currentIndex = 0;

  for (int i = 0; i < visibleOptions; i++)
  {
    int optionIdx = (index + i) % s_o_u;

    if (highlight && i == currentIndex)
    {
      display.setTextColor(BLACK);
      display.fillRect(0, i * 18, 128, 18, WHITE);  // Adjusted rectangle height for better appearance
    }
    else
    {
      display.setTextColor(WHITE);
    }

    display.setCursor(5, i * 18 + 12);
    display.print(Unit_opt[optionIdx]);
  }

  display.display();
}

void drawshotM(int index, bool highlight)
{
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);

  int visibleOptions = min(s_o_s, 3);
  int currentIndex = 0;

  for (int i = 0; i < visibleOptions; i++)
  {
    int optionIdx = (index + i) % s_o_s;

    if (highlight && i == currentIndex)
    {
      display.setTextColor(BLACK);
      display.fillRect(0, i * 18, 128, 18, WHITE);  // Adjusted rectangle height for better appearance
    }
    else
    {
      display.setTextColor(WHITE);
    }

    display.setCursor(5, i * 18 + 12);
    display.print(shot_opt[optionIdx]);
  }

  display.display();
}


void drawshot()
{
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);
  display.setCursor(0, 18);
  display.print("Shot Count: " + String(shotCounter));
  display.display();
}

void drawTshot()
{
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);
  display.setCursor(0, 18);
  display.print("Total Count: " + String(shotCounter + total_shots));
  display.display();
}



// button fucntions //

void button_ticks()
{
  btn1.tick();
  btn2.tick();
  btn3.tick();
  btn4.tick();
  btn5.tick();
  delay(10);
}

void button_init()
{
  btn1.setPressTicks(2000) ;
  btn1.attachClick(btn1_click);
  btn1.attachLongPressStart(btn1_long_press);
  btn1.attachLongPressStop(b1_long_stop);

  btn2.attachClick(btn2_click);

  btn3.setPressTicks(2000) ;
  btn3.attachLongPressStart(btn3_long_press);
  btn3.attachClick(btn3_click);

  btn4.attachClick(btn4_click);

  btn5.setPressTicks(3000);
  btn5.attachLongPressStart(btn5_long_press);
}

static void btn1_click()
{
  Serial.println("pressed1");

  if (main_menu)
  {
    currentIndex = (currentIndex - 1 + s_o_m) % s_o_m;
    drawMenu(currentIndex, true);
  }

  if (t_opt_m)
  {
    currentIndex = (currentIndex - 1 + s_o_t) % s_o_t;
    drawTimerM(currentIndex, true);
  }

  if (z_opt_m)
  {
    currentIndex = (currentIndex - 1 + s_o_z) % s_o_z;
    drawzero(currentIndex, true);
  }

  if (c_opt_m)
  {
    currentIndex = (currentIndex - 1 + s_o_c) % s_o_c;
    drawcal(currentIndex, true);
  }

  if (cpr_opt_m)
  {
    currentIndex = (currentIndex - 1 + s_o_c) % s_o_c;
    drawcpr(currentIndex, true);
  }

  if (u_opt_m)
  {
    currentIndex = (currentIndex - 1 + s_o_u) % s_o_u;
    drawunit(currentIndex, true);
  }
  if (shot_m)
  {
    currentIndex = (currentIndex - 1 + s_o_s) % s_o_s;
    drawshotM(currentIndex, true);
  }
}


static void btn2_click()
{
  if (main_menu)
  {

    if (menuItems[currentIndex] == "Set Timer")
    {
      currentIndex = 0;
      t_opt_m = true;
      main_menu = false;
      drawTimerM(currentIndex, true);
    }
    else if (menuItems[currentIndex] == "Zero")
    {
      currentIndex = 0;
      z_opt_m = true;
      main_menu = false;
      drawzero(currentIndex, true);
    }
    else if (menuItems[currentIndex] == "Calibration")
    {
      currentIndex = 0;
      c_opt_m = true;
      main_menu = false;
      drawcal(currentIndex, true);
    }
    else if (menuItems[currentIndex] == "Clicks per Rev")
    {
      currentIndex = 0;
      cpr_opt_m = true;
      main_menu = false;
      drawcpr(currentIndex, true);
    }
    else if (menuItems[currentIndex] == "Unit")
    {
      currentIndex = 0;
      u_opt_m = true;
      main_menu = false;
      drawunit(currentIndex, true);
    }
    else if (menuItems[currentIndex] == "Shot Count")
    {
      shot_m = true;
      currentIndex = 0;
      main_menu = false;
      drawshotM(currentIndex, true);
    }
  }

  else if (t_opt_m)
  {

    if (set_timer_opt[currentIndex] == "30s")
    {

      timer = 30;
      s_time = 30;
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(set_timer_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawTimerM(currentIndex, true);
    }
    else if (set_timer_opt[currentIndex] == "60s")
    {
      timer = 60;
      s_time = 60;
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(set_timer_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawTimerM(currentIndex, true);
    }
    else if (set_timer_opt[currentIndex] == "90s")
    {
      timer = 90;
      s_time = 90;
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(set_timer_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawTimerM(currentIndex, true);
    }
    else if (set_timer_opt[currentIndex] == "120s")
    {
      timer = 120;
      s_time = 120;
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(set_timer_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawTimerM(currentIndex, true);
    }
    else if (set_timer_opt[currentIndex] == "150s")
    {
      timer = 150;
      s_time = 150;
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(set_timer_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawTimerM(currentIndex, true);
    }
    else if (set_timer_opt[currentIndex] == "180s")
    {
      timer = 180;
      s_time = 180;
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(set_timer_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawTimerM(currentIndex, true);
    }
  }

  else if (z_opt_m)
  {
    if (Zero_opt[currentIndex] == "Z Turret")
    {
      encoderPosition = 0;
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print("Clicks Zeroed");
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawzero(currentIndex, true);
    }
    else if (Zero_opt[currentIndex] == "Z Inclination")
    {
      z_pitch = s_pitch;
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(Zero_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawzero(currentIndex, true);
    }
    else if (Zero_opt[currentIndex] == "R Inclination")
    {
      z_pitch = 0;
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(Zero_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawzero(currentIndex, true);
    }
    else if (Zero_opt[currentIndex] == "Z Rectile")
    {
      z_roll = s_roll;
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(Zero_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawzero(currentIndex, true);
    }

    else if (Zero_opt[currentIndex] == "R Rectile")
    {
      z_roll = 0;
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(Zero_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawzero(currentIndex, true);
    }
  }

  //  else if (c_opt_m)
  //  {
  //
  //  }

  else if (cpr_opt_m)
  {


    if ( Click_opt[currentIndex] == "60")
    {
      clicks_per_rev = Click_opt[currentIndex].toInt();
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(Click_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawcpr(currentIndex, true);
    }

    else if (Click_opt[currentIndex] == "100")
    {
      clicks_per_rev = Click_opt[currentIndex].toInt();
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(Click_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawcpr(currentIndex, true);
    }

    else if (Click_opt[currentIndex] == "120")
    {
      clicks_per_rev = Click_opt[currentIndex].toInt();
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(Click_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawcpr(currentIndex, true);
    }

    else if (Click_opt[currentIndex] == "150")
    {
      clicks_per_rev = Click_opt[currentIndex].toInt();
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(Click_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawcpr(currentIndex, true);
    }

  }

  else if (u_opt_m)
  {
    if ( Unit_opt[currentIndex] == "MIL")
    {
      clicksunit = "MIL";
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(Unit_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawunit(currentIndex, true);
    }
    else if (Unit_opt[currentIndex] == "MOA")
    {
      clicksunit = "MOA";
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setCursor(0, 18);
      display.print(Unit_opt[currentIndex]);
      display.setCursor(0, 32);
      display.print("selected!");
      display.display();
      delay(500);
      drawunit(currentIndex, true);
    }
  }

  else if (shot_m)
  {
    if (shot_opt[currentIndex] == "Shot Count A")
    {
      drawshot();
      c_shot = true;
      p_shot_m = true;
    }

    else if (shot_opt[currentIndex] == "Shot Count B")
    {
      drawTshot();
      t_shot = true;
      p_shot_m = true;
    }
  }

}


void btn3_click()
{
  if (main_menu)
  {
    currentIndex = (currentIndex + 1) % s_o_m;
    drawMenu(currentIndex, true);
  }

  if (t_opt_m)
  {
    currentIndex = (currentIndex + 1) % s_o_t;
    drawTimerM(currentIndex, true);
  }

  if (z_opt_m)
  {
    currentIndex = (currentIndex + 1) % s_o_z;
    drawzero(currentIndex, true);
  }

  if (c_opt_m)
  {
    drawcal(currentIndex, true);
  }

  if (cpr_opt_m)
  {
    currentIndex = (currentIndex + 1) % s_o_c;
    drawcpr(currentIndex, true);
  }

  if (u_opt_m)
  {
    currentIndex = (currentIndex + 1) % s_o_u;
    drawunit(currentIndex, true);
  }
  if (shot_m)
  {
    currentIndex = (currentIndex + 1) % s_o_s;
    drawshotM(currentIndex, true);
  }

}


void btn4_click()
{
  Serial.println("pressed 4 btn");

  if (main_menu == false && t_opt_m == false && z_opt_m == false && c_opt_m == false && cpr_opt_m == false && u_opt_m == false && shot_m == false)
  {

    t++;
    if (t == 1)
    {
      ts = true;
      Serial.println("start timer!");
    }
    if (t > 1)
    {
      t = 0;
      ts = false;
      timer = p_time;
      Serial.println("stop timer!");
    }
  }

  if (t_opt_m)
  {
    t_opt_m = false;
    main_menu = true;
    currentIndex = 0;
    drawMenu(currentIndex, true);
  }
  else if (z_opt_m)
  {
    z_opt_m = false;
    main_menu = true;
    currentIndex = 0;
    drawMenu(currentIndex, true);
  }
  else if (c_opt_m)
  {
    c_opt_m = false;
    main_menu = true;
    currentIndex = 0;
    drawMenu(currentIndex, true);
  }
  else if (cpr_opt_m)
  {
    cpr_opt_m = false;
    main_menu = true;
    currentIndex = 0;
    drawMenu(currentIndex, true);
  }
  else if (u_opt_m)
  {
    u_opt_m = false;
    main_menu = true;
    currentIndex = 0;
    drawMenu(currentIndex, true);
  }
  else if (shot_m && p_shot_m == false)
  {
    c_shot = false;
    t_shot = false;
    shot_m = false;
    main_menu = true;
    currentIndex = 0;
    drawMenu(currentIndex, true);


  }
  else if (p_shot_m)
  {
    c_shot = false;
    t_shot = false;
    p_shot_m = false;
    drawshotM(currentIndex, true);
  }

}

void btn1_long_press()
{
  Serial.println("b1 long");

  if (main_menu == false)
  {
    ts = false;
    timer = s_time;
  }

  if (c_opt_m)
  {
    Serial.println("calibration for encoder!");
    display.clearDisplay();
    display.setFont(&FreeSans9pt7b);
    display.setTextSize(1);
    display.setCursor(0, 18);
    display.print("Release to");
    display.setCursor(0, 32);
    display.print("Stop!");
    display.display();
    Record = true;
    pos0 = encoderPosition;  // instead of 10 current position of encoder is entered here
    cal_click_val = 0;
  }
}

void b1_long_stop()
{
  if (c_opt_m)
  {

    Record = false;
    cal_click_val = pos1 - pos0;

    preferences.begin("encoder", false);
    preferences.putInt("c_val", cal_click_val);
    Serial.println("calibration value Saved using Preferences");
    preferences.end();

    display.clearDisplay();
    display.setFont(&FreeSans9pt7b);
    display.setTextSize(1);
    display.setCursor(0, 18);
    display.print("clicks in 1");
    display.setCursor(0, 32);
    display.print("rev =" + String(cal_click_val) + " saved.");
    display.display();
    delay(1000);
    drawcal(currentIndex, true);
    encoderPosition = 0;
    clicks = 0;

  }
}

void btn3_long_press()
{

  if (shot_m)
  {
    shotCounter = 0;
  }
  else
  {
    encoderPosition = 0;
  }
}

void btn5_long_press()
{
  Serial.println("butn5 presssed!");

  count++;
  if (count == 1)
  {
    main_menu = true;
    drawMenu(currentIndex, true);

  }
  if (count > 1)
  {
    count = 0;
    main_menu = false;

  }
}






// oled fucntions //
void oled_init()
{
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.display();
  display.clearDisplay();
}

// encoder //

void check_cal()
{

  preferences.begin("encoder", false);
  cal_click_val = preferences.getInt("c_val", 0);
  total_shots = preferences.getInt("s_val", 0);
  shotCounter = preferences.getInt("ss_val", 0);

  if (cal_click_val == 0)
  {
    cal_click_val = 1;
    Serial.println("No values saved for calibration so default value is 1");
  }
  else
  {
    Serial.println("value found for calibration is " + String(cal_click_val));
  }

  if (total_shots == 0)
  {
    Serial.println("No value saved for total shots");
  }
  else
  {
    Serial.println("Total shots are " + String(total_shots));
  }

  if (shotCounter == 0)
  {
    Serial.println("No value saved for  shots");
  }
  else
  {
    Serial.println("shots are " + String(shotCounter));
  }

}

void encoder_init()
{
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  // Attach interrupts to the encoder pins
  attachInterrupt(digitalPinToInterrupt(encoderPinA), handleEncoderInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), handleEncoderInterrupt, CHANGE);

}

void handleEncoderInterrupt()
{
  int encoderStateA = digitalRead(encoderPinA);
  int encoderStateB = digitalRead(encoderPinB);

  // Detect changes on the A channel and update the position accordingly
  if ((encoderStateA == HIGH) && (lastEncoderStateA == LOW)) {
    if (encoderStateB == LOW)
    {
      encoderPosition++;
    }
    else
    {
      encoderPosition--;
    }
  }

  // Detect changes on the B channel and update the position accordingly
  if ((encoderStateB == HIGH) && (lastEncoderStateB == LOW)) {
    if (encoderStateA == LOW)
    {
      encoderPosition--;
    }
    else
    {
      encoderPosition++;
    }
  }

  // Update the last state variables
  lastEncoderStateA = encoderStateA;
  lastEncoderStateB = encoderStateB;
}

// battery //

void battery()
{
  vd_level = analogRead(15);
  //Serial.println(vd_level);
  vd_voltage = ((vd_level * 3.3) / 4095.0);
  float  battery_voltage = float((vd_voltage * (2700 + 10000)) / 10000);
  // Serial.println(battery_voltage);
  percentage = (battery_voltage / 4.2) * 100;
  // Serial.println(percentage);

}

// imu code //

void imu_init()
{
  Wire.begin();

  if (myIMU.begin() == false)
  {
    Serial.println("BNO08x not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
    while (1);
  }

  Serial.println("BNO08x found!");
  Wire.setClock(400000); //Increase I2C data rate to 400kHz
  setReports();
  Serial.println("Reading events");
}

void setReports(void)
{
  Serial.println("Setting desired reports");
  if (myIMU.enableRotationVector() == true)
  {
    Serial.println(F("Rotation vector enabled"));
    Serial.println(F("Output in form roll, pitch, yaw"));
  }
  else
  {
    Serial.println("Could not enable rotation vector");
  }

  if (myIMU.enableAccelerometer() == true) {
    Serial.println(F("Accelerometer enabled"));
    Serial.println(F("Output in form x, y, z, in m/s^2"));
  } else {
    Serial.println("Could not enable accelerometer");
  }
}

void imu_data()
{
  delay(10);

  if (myIMU.wasReset())
  {
    Serial.print("sensor was reset ");
    setReports();
  }

  // Has a new event come in on the Sensor Hub Bus?
  if (myIMU.getSensorEvent() == true)
  {

    // is it the correct sensor data we want?
    if (myIMU.getSensorEventID() == SENSOR_REPORTID_ROTATION_VECTOR)
    {

      float roll = (myIMU.getRoll()) * 180.0 / PI; // Convert roll to degrees
      float pitch = (myIMU.getPitch()) * 180.0 / PI; // Convert pitch to degrees
      float yaw = (myIMU.getYaw()) * 180.0 / PI; // Convert yaw / heading to degrees

      s_roll = roll;
      incline = pitch;
      cosincline = cos(pitch * (pi / 180));
      s_pitch = cosincline;
      s_yaw = yaw;

      //  Serial.print(roll, 1);
      //  Serial.print(F(", "));
      //  Serial.print(pitch, 1);
      //      Serial.print(F(", "));
      //      Serial.print(yaw, 1);
      //Serial.println();
    }

    if (myIMU.getSensorEventID() == SENSOR_REPORTID_ACCELEROMETER)
    {

      float x = myIMU.getAccelX();
      float y = myIMU.getAccelY();
      float z = myIMU.getAccelZ();

      float jerk = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
      // Serial.println(jerk);

      if (jerk > JERK_THRESHOLD && isSettled)
      {
        int store_shot;
        shotCounter++;
        isSettled = false;
        settleStartTime = millis();  // Record the start time of settling period

        store_shot = total_shots + shotCounter;

        preferences.begin("encoder", false);
        preferences.putInt("s_val", store_shot);
        Serial.println("Shots updated!");
        preferences.end();

        preferences.begin("encoder", false);
        preferences.putInt("ss_val", shotCounter);
        Serial.println("updated!");
        preferences.end();

        // Serial.print(F(" - Shot Fired! Count: "));
        // Serial.println(shotCounter);
      }
      else if (!isSettled)
      {
        // Check if settling period has elapsed
        if (millis() - settleStartTime > SETTLE_PERIOD)
        {
          isSettled = true;
          //  Serial.println(F("Values have settled."));
        }
      }
    }
  }
}

// led control fucntion //

void led_init()
{
  pinMode(red_led1, OUTPUT);
  pinMode(red_led2, OUTPUT);
  pinMode(green_led1, OUTPUT);
  pinMode(green_led2, OUTPUT);
  pinMode(green_led3, OUTPUT);
  pinMode(blue_led1, OUTPUT);
  pinMode(blue_led2, OUTPUT);
}

void led_control()
{
  if (s_roll > 2 && s_roll < 5)
  {
    digitalWrite(red_led1, HIGH);
    digitalWrite(red_led2, LOW);
    digitalWrite(green_led1, LOW);
    digitalWrite(green_led2, LOW);
    digitalWrite(green_led3, LOW);
    digitalWrite(blue_led1, LOW);
    digitalWrite(blue_led2, LOW);
  }
  else if (s_roll < -2 && s_roll > -5)
  {
    digitalWrite(red_led1, LOW);
    digitalWrite(red_led2, LOW);
    digitalWrite(green_led1, LOW);
    digitalWrite(green_led2, LOW);
    digitalWrite(green_led3, LOW);
    digitalWrite(blue_led1, HIGH);
    digitalWrite(blue_led2, LOW);
  }
  else if (s_roll > 5)
  {
    digitalWrite(red_led1, HIGH);
    digitalWrite(red_led2, HIGH);
    digitalWrite(green_led1, LOW);
    digitalWrite(green_led2, LOW);
    digitalWrite(green_led3, LOW);
    digitalWrite(blue_led1, LOW);
    digitalWrite(blue_led2, LOW);
  }
  else if (s_roll < -5)
  {
    digitalWrite(red_led1, LOW);
    digitalWrite(red_led2, LOW);
    digitalWrite(green_led1, LOW);
    digitalWrite(green_led2, LOW);
    digitalWrite(green_led3, LOW);
    digitalWrite(blue_led1, HIGH);
    digitalWrite(blue_led2, HIGH);
  }
  else if (s_roll >= 0 && s_roll < 2)
  {
    digitalWrite(red_led1, LOW);
    digitalWrite(red_led2, LOW);
    digitalWrite(green_led1, HIGH);
    digitalWrite(green_led2, HIGH);
    digitalWrite(green_led3, HIGH);
    digitalWrite(blue_led1, LOW);
    digitalWrite(blue_led2, LOW);
  }
}


String removeCharacter(String str, char charToRemove) {
  str.replace(String(charToRemove), ""); // Replace the character with an empty string
  return str;
}
