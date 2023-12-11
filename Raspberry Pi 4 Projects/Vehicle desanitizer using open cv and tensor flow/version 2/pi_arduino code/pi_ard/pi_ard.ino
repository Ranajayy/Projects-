String buff;
int water_sensor = 2;
int button = 3;
int water_sensor_led = 4;
int water_pump = 5;
void setup()
{
  Serial.begin(9600);
  pinMode(water_sensor, INPUT);
  pinMode(button, INPUT);
  pinMode(water_pump, OUTPUT);
  pinMode(water_sensor_led, OUTPUT);
  digitalWrite(water_pump, HIGH);

}

void loop()
{
  if (Serial.available())
  {
    buff = Serial.readStringUntil('\n');
    Serial.println(buff);


    if (buff == "right"|| digitalRead(button) == 1)
    {
      digitalWrite(water_pump, HIGH);
    }

    else if (buff == "left" )
    { 
      digitalWrite(water_pump, LOW);
      delay(500);
    }
  }
  
   
  if (digitalRead(water_sensor) == 1)
  {
    digitalWrite(water_sensor_led, HIGH);
  }
  else if (digitalRead(water_sensor) == 0)
  {
    digitalWrite(water_sensor_led, LOW);
  }

}
