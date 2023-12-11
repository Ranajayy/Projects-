#include <FreqCount.h>//https://github.com/PaulStoffregen/FreqCount/archive/master.zip
#include <LiquidCrystal.h>


void setup() {
  Serial.begin(9600);
 
  pinMode(7, INPUT);
  FreqCount.begin(1000);
}
unsigned long f; float f0;
int x, n = 3, r;

void loop() {

  if (digitalRead(7) == HIGH) {
    n++;
    x = 0;
    delay(100);
  }

  if (n == 1) {
    x++;
    if (x == 1) {
      FreqCount.begin(100);
    } r = -1;
    Serial.println("T = 0.1 s ");
  }
  if (n == 2) {
    x++;
    if (x == 1) {
      FreqCount.begin(10000);
    } r = 1;
    Serial.println("T = 10 s ");
  }
  if (n == 3) {
    x++;
    if (x == 1) {
      FreqCount.begin(1000);
    } r = 0;
    Serial.println("T = 1 s  ");
  }
  if (n > 3) {
    n = 1;
  }
  Serial.print("F = ");
  if (f >= 1000000 && n == 3) {
    f0 = f / 1000000.0;
    Serial.println(f0, 6 + r);
    Serial.println(" MHz");
  }
  if (f < 1000000 && n == 3) {
    f0 = f / 1000.0;
    Serial.println(f0, 3 + r);
    Serial.println(" kHz");
  }
  if (f >= 100000 && n == 1) {
    f0 = f / 100000.0;
    Serial.println(f0, 6 + r);
    Serial.println(" MHz");
  }
  if (f < 100000 && n == 1) {
    f0 = f / 100.0;
    Serial.println(f0, 3 + r);
    Serial.println(" kHz");
  }
  if (f >= 10000000 && n == 2) {
    f0 = f / 10000000.0;
    Serial.println(f0, 6 + r);
    Serial.println("MHz");
  }
  if (f < 10000000 && n == 2) {
    f0 = f / 10000.0;
    Serial.println(f0, 3 + r);
    Serial.println(" kHz");
  }

  if (FreqCount.available()) {

    f = FreqCount.read();

//    Serial.setCursor(10, 1); Serial.print("***");
  }
  delay(200);
}
