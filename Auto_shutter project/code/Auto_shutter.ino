#define ENCA 2 // encoder pin no :
#define sensorPin 6  // choose the input pin (for PIR sensor)  
#define in1 5
#define in2 4

int pot_flag;
int pot_value;
int set_timer = 5;
int pos = 0;

int sensor = 0;
int Seconds = 0;
int flag = 0;
unsigned long delay_close = 0;
void setup()
{
  // put your setup code here, to run once
  Serial.begin(9600);// initialize serial communication at 9600 bits per second:
  pinMode(ENCA, INPUT);
  pinMode(sensorPin, INPUT); // declare sensor as input
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);
}

void loop()
{
  sensor = digitalRead(sensorPin);

  if (sensor == 1)
  {
    Serial.println("Motion detected!");
    if (flag == 0)
    {

      digitalWrite(in1, HIGH);// Door Open
      digitalWrite(in2, LOW); // Door Close
    }

    Seconds = 0;
    flag = 1;
    delay_close = millis(); // set start time
  }

  else
  {
    Serial.println("Motion stopped!");
  }

  if ((flag == 1) && (millis() - delay_close) > 999)
  {
    Seconds = Seconds + 1;
    delay_close = millis(); // set start time
  }

  if (Seconds > set_timer)
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    flag = 0;
    Seconds = 0;
  }


}

void readEncoder()
{
  pos++;
  Serial.println(pos);
  if (pos >= 15)
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    pos = 0;

  }
}
