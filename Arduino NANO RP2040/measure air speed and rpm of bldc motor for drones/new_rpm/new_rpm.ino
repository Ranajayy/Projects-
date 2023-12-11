int revolutions = 0;
int prev_state , state , rpm = 0;
unsigned long prev = 0;
void setup()
{
  Serial.begin(9600);
  pinMode(22, INPUT);
  // attachInterrupt(digitalPinToInterrupt(2), interruptFunction, RISING);
  prev_state = digitalRead(22);
  // put your setup code here, to run once:
  prev = millis();
}

void loop()
{
  // Serial.println("Analog= " + String(analogRead(A0)));
  state = digitalRead(22);
  
 // Serial.println(state);
    if (state != prev_state)
    {
      if (state == 0)
      {
        revolutions++;
        //  Serial.println("rev " + String(revolutions));
        //  delayMicroseconds(104);
  
      }
      prev_state = state;
    }
    if (millis() - prev >= 1000)
    {
      rpm = revolutions * 60;
      Serial.println("rev " + String(revolutions));
      Serial.println("rpm " + String(rpm));
     revolutions = 0;
      prev = millis();
    }

  //  delay(500);

}
//void interruptFunction() //interrupt service routine
//{
//  revolutions++;
//}
