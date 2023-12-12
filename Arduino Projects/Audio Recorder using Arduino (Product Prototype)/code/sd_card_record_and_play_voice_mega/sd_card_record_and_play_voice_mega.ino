#include <TMRpcm.h>
#include <SPI.h>



#define SD_ChipSelectPin 4
TMRpcm audio;

int rotary = A1;
int encoder_val = 0;
int recordLed = 2;
int mic_pin = A0;
int sample_rate = 16000;
int record_btn = 3;
int btn1 = 5;
int btn2 = 6;
int btn3 = 7;
int btn4 = 8;

String filename;
int val = 0;
int flag = 0;
int flag1 = 0;
int counter = 0;

void setup()
{
  Serial.begin(9600);
  //Sets up the pins
  audio.speakerPin = 9;
  pinMode(12, OUTPUT);
  pinMode(record_btn, INPUT);
  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);
  pinMode(btn3, INPUT);
  pinMode(btn4, INPUT);
  pinMode(recordLed, OUTPUT);


  if (!SD.begin(SD_ChipSelectPin))
  {
    return;
  }
  else
  {
    Serial.println("SD OK");
  }
  // The audio library needs to know which CS pin to use for recording
  audio.CSPin = SD_ChipSelectPin;
  audio.volume(5);

}


void loop()
{
  encoder_val = analogRead(rotary);

  val = map(encoder_val, 0, 1023, 1, 12);
  Serial.print(" encoder_val    :");
  Serial.println(val);

  if (digitalRead(btn1) == 1)
  {
    filename = String(0 + val) + String(".wav");
    char charBuf[50];
    filename.toCharArray(charBuf, 50);
    Serial.print("charBuf :");
    Serial.println(charBuf);

    if (digitalRead(record_btn) == 1 ) {
      byte adcsra = ADCSRA;
      byte adcsrb = ADCSRB;
      Serial.println("startRecording btn1_1 ");
      audio.startRecording(charBuf, sample_rate, mic_pin, 0);
      //    while (digitalRead(record_btn));
      while (digitalRead(btn1));
      audio.stopRecording(charBuf);
      ADCSRB = adcsrb;
      ADCSRA = adcsra;
    }
    else {
      Serial.println("playing btn1_1 ");
      audio.play(charBuf);
      while (digitalRead(btn1));
    }
  }

  if (digitalRead(btn2) == 1)
  {
    filename = String(12 + val) + String(".wav");
    char charBuf[50];
    filename.toCharArray(charBuf, 50);
    Serial.print("charBuf :");
    Serial.println(charBuf);

    if (digitalRead(record_btn) == 1 )
    {
      byte adcsra = ADCSRA;
      byte adcsrb = ADCSRB;
      Serial.println("startRecording btn2 ");
      audio.startRecording(charBuf, sample_rate, mic_pin, 0);
      //    while (digitalRead(record_btn));
      while (digitalRead(btn2));
      audio.stopRecording(charBuf);
      ADCSRB = adcsrb;
      ADCSRA = adcsra;
    }

    else
    {
      Serial.println("playing btn2 ");
      audio.play(charBuf);
      while (digitalRead(btn2));
    }
  }

  if (digitalRead(btn3) == 1)
  {

    filename = String(24 + val) + String(".wav");

    char charBuf[50];
    filename.toCharArray(charBuf, 50);
    Serial.print("charBuf :");
    Serial.println(charBuf);

    if (digitalRead(record_btn) == 1 ) {
      byte adcsra = ADCSRA;
      byte adcsrb = ADCSRB;
      Serial.println("startRecording btn3 ");
      audio.startRecording(charBuf, sample_rate, mic_pin, 0);
      //    while (digitalRead(record_btn));
      while (digitalRead(btn3));
      audio.stopRecording(charBuf);
      ADCSRB = adcsrb;
      ADCSRA = adcsra;
    }
    else {
      Serial.println("playing btn3 ");
      audio.play(charBuf);
      while (digitalRead(btn3));
    }
  }


  if (digitalRead(btn4) == 1) {

    filename = String(36 + val) + String(".wav");

    char charBuf[50];
    filename.toCharArray(charBuf, 50);
    Serial.print("charBuf :");
    Serial.println(charBuf);

    if (digitalRead(record_btn) == 1 ) {
      byte adcsra = ADCSRA;
      byte adcsrb = ADCSRB;
      Serial.println("startRecording btn4 ");
      audio.startRecording(charBuf, sample_rate, mic_pin, 0);
      //    while (digitalRead(record_btn));
      while (digitalRead(btn4));
      audio.stopRecording(charBuf);
      ADCSRB = adcsrb;
      ADCSRA = adcsra;
    }
    else {
      Serial.println("playing btn4 ");
      audio.play(charBuf);
      while (digitalRead(btn4));
    }
  }
}
