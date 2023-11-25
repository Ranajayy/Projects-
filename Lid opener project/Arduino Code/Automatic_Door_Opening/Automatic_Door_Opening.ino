#define sensorPin 6// choose the input pin (for PIR sensor)  
#define in1 5
#define in2 4
#define pot_pin A0

int pot_flag;
int pot_value;
int set_timer = 5;

int sensor = 0;
int pot_current_value = 0;
unsigned long pot_milis = 0;
int pot_previous_value = 0;
int Seconds = 0;
int flag = 0;
long delay_Start;

void setup()
{ // put your setup code here, to run once
  Serial.begin(9600);// initialize serial communication at 9600 bits per second:

  pinMode(sensorPin, INPUT); // declare sensor as input

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pot_value = analogRead(pot_pin);

  while (pot_value != 0)
  {
    digitalWrite(in1, LOW);// Door Open
    digitalWrite(in2, HIGH); // Door Close
    delay(2000);
    pot_value = analogRead(pot_pin);
    if (pot_value <= 1)
    {
      digitalWrite(in1, LOW);// Door Open
      digitalWrite(in2, LOW); // Door Close
      break;
    }

  }

}

void loop()
{
  pot_value = analogRead(pot_pin);
  sensor = digitalRead(sensorPin);
  Serial.println(pot_value);

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
    delay_Start = millis(); // set start time
  }

  else
  {
    Serial.println("Motion stopped!");
  }

  if ((flag == 1) && (millis() - delay_Start) > 999)
  {
    Seconds = Seconds + 1;
    delay_Start = millis(); // set start time
  }

  if (Seconds > set_timer)
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    flag = 0;
    Seconds = 0;

    if (sensor == 1)
    {
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      flag = 1;
    }

  }

  if (flag == 1 && pot_value >= 100)
  {
    Serial.println("in pot==100");
    digitalWrite(in1, LOW); // Door Open
    digitalWrite(in2, LOW); // Door Close
  }

  if (flag == 0 && pot_value <= 1)
  {
    digitalWrite(in1, LOW); // Door Open
    digitalWrite(in2, LOW); // Door Close
  }

  if (pot_value >= pot_previous_value - 5 && pot_value <= pot_previous_value + 5 && pot_value >1 && pot_value<100)
  {

    if (millis() - pot_milis > 10000)
    {
      if ((pot_value > 1 && pot_value < 100))
      {
        digitalWrite(in1, HIGH); // Door Open
        digitalWrite(in2, LOW); // Door Close
        flag = 3;
        pot_milis = millis();

      }
    }

  }

  else
  {
    pot_milis = millis();
  }

  if (flag == 3 && pot_value >= 100)
  {
    digitalWrite(in1, LOW); // Door Open
    digitalWrite(in2, LOW); // Door Close
    flag = 0;
  }

  pot_previous_value = pot_value;
}
