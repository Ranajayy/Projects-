
float revolutions = 0;
unsigned long rpm = 0; // max value 32,767 16 bit
long  startTime = 0;
long  elapsedTime;
// create servo object to control the ESC

int potValue;  // value from the analog pin

void setup()
{
  // Attach the ESC on pin 9
  //  ESC.attach(7, 1000, 2000); // (pin, min pulse width, max pulse width in microseconds)
  Serial.begin(9600);
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), interruptFunction, RISING);
}

void loop()
{
  revolutions = 0; rpm = 0;
  startTime = millis();
  delay(1000);
  //now let's see how many counts we've had from the hall effect sensor and calc the RPM
  elapsedTime = millis() - startTime; //finds the time, should be very close to 1 sec

  if (revolutions > 0)
  {
    rpm = (max(1, revolutions) * 60000) / elapsedTime;     //calculates rpm
  }
  String outMsg = String("RPM :") + rpm;
  Serial.println(outMsg);    // Send the signal to the ESC

}
void interruptFunction() //interrupt service routine
{
  revolutions++;
}
