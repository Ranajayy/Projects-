void setup() {
Serial.begin(115200); // open serial port, set the baud rate to 115200 bps
}
void loop() {
Serial.println(analogRead(15)); //connect sensor and print the value to serial
delay(100);
}
