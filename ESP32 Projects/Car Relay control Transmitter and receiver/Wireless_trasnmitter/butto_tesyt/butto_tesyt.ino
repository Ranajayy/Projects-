#include "OneButton.h"
#define button1_pin 34
#define button2_pin  35
#define button3_pin  19
#define button4_pin  21
#define button5_pin  33
#define button6_pin  32
#define button7_pin  26
#define button8_pin  27
#define button9_pin  25


OneButton btn = OneButton(button1_pin, false, false);
OneButton btn2 = OneButton(button2_pin, false, false);
OneButton btn3 = OneButton(button3_pin, false, false);
OneButton btn4 = OneButton(button4_pin, false, false);
OneButton btn5 = OneButton(button5_pin, false, false);
OneButton btn6 = OneButton(button6_pin, false, false);
OneButton btn7 = OneButton(button7_pin, false, false);
OneButton btn8 = OneButton(button8_pin, false, false);
OneButton btn9 = OneButton(button9_pin, false, false);

void setup()
{
  Serial.begin(115200);
  Serial.println("One Button Example with polling.");
  btn.setPressTicks(5000) ;
  btn.attachClick(b1_click);
  btn.attachLongPressStart(b1_long_press);
  btn2.attachClick(b2_click);
  btn3.attachClick(b3_click);
  btn4.attachClick(b4_click);
  btn5.attachClick(b5_click);
  btn6.attachClick(b6_click);
  btn7.attachClick(b7_click);
  btn8.attachClick(b8_click);
  btn9.attachClick(b9_click);


}



void loop()
{
  button_ticks();
}


void button_ticks()
{
  btn.tick();
  btn2.tick();
  btn3.tick();
  btn4.tick();
  btn5.tick();
  btn6.tick();
  btn7.tick();
  btn8.tick();
  btn9.tick();
  delay(10);
}
static void b1_click()
{
  Serial.println("Clicked!");
}
static void b2_click()
{
  Serial.println("Clicked!");
}
static void b3_click()
{
  Serial.println("Clicked!");
}
static void b4_click()
{
  Serial.println("Clicked!");
}
static void b5_click()
{
  Serial.println("Clicked!");
}
static void b6_click()
{
  Serial.println("Clicked!");
}
static void b7_click()
{
  Serial.println("Clicked!");
}
static void b8_click()
{
  Serial.println("Clicked!");
}
static void b9_click()
{
  Serial.println("Clicked!");
}

void b1_long_press()
{
  Serial.println("pairing mode on!");
}
