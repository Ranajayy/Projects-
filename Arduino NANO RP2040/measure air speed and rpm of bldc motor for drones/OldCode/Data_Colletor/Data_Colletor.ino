#include <Wire.h>
#include <DS3231.h>
#include <Adafruit_MPU6050.h>
#include "MPU6050_6Axis_MotionApps20.h"
#include "Adafruit_BMP085.h"
#include <AceButton.h>

using namespace ace_button;

#define STANDARD_BAROMETRIC_PRESSURE 102500

// Recording led and button
const byte recordingLedPin = 5; //original 5  
const byte recordingButtonPin = 6; //original 6 
bool isRecordingLastState = false;
bool isRecording = false;
unsigned long lastBlinkTime;
byte lastBlink = LOW;

bool isCalibrated = false;

// GY-521 (Gyro and acc)
MPU6050 accGyroMPU(0x69);
bool accGyroDMPReady = false;
uint8_t accGyroDevStatus;
uint8_t accGyroMPUIntStatus;
uint16_t accGyroPacketSize;
uint16_t accGyroFifoCount;
uint8_t accGyroFifoBuffer[64];

Adafruit_MPU6050 mpu;
sensors_event_t acc, g, temp;

Quaternion accGyroQuaternion;
VectorFloat accGyroGravity;
float accGyroRotation[3];
VectorInt16 accGyroAcceleration;
VectorInt16 accGyroRealAcceleration;
VectorInt16 accGyroRealWorldAcceleration;

int mean_ax,mean_ay,mean_az,mean_gx,mean_gy,mean_gz,state=0;
int buffersize=1000; 
int16_t ax, ay, az;
int16_t gx, gy, gz;

int ax_offset,ay_offset,az_offset,gx_offset,gy_offset,gz_offset;
int acel_deadzone=8;
int gyro_deadzone=1;

volatile bool accGyroMPUInterrupted = false;
void accGyroDMPInterrupt() {
  accGyroMPUInterrupted = true;
}

// RPM
unsigned long lastRPMCountTime;

float rpm1LastCountTime;
int revs1;
int rpm1;
void irRpm1Interrupt() {
  revs1++;
}

float rpm2LastCountTime;
int revs2;
int rpm2;
void irRpm2Interrupt() {
  revs2++;
}

float rpm3LastCountTime;
int revs3;
int rpm3;
void irRpm3Interrupt() {
  revs3++;
}

float rpm4LastCountTime;
int revs4;
int rpm4;
void irRpm4Interrupt() {
  revs4++;
}

// HW-290 (only pressure)
Adafruit_BMP085 bmp;
uint8_t bmpPressure;
uint8_t bmpAltitude;
uint8_t bmpSealevelPressure;
uint8_t bmpRealAltitude;
float bmpTemperature;

// PX4Airspeed
byte PX_ADDRESS_SLAVE = 0X28; 
byte PX_REGISTER_XY = 0X00;
byte PX_READ_LENGTH = 4;

const int16_t MS4525FullScaleRange = 1;
const int16_t MS4525MinScaleCounts = 1638;
const int16_t MS4525FullScaleCounts = 14746;
const int16_t MS4525Span=MS4525FullScaleCounts-MS4525MinScaleCounts;
const int16_t MS4525ZeroCounts=(MS4525MinScaleCounts+MS4525FullScaleCounts)/2;

uint16_t px_pressure;
float px_psi;
float px_velocity;
float px_airDensity = 1.225;

// Real time date
DS3231 rtc;
String dateTimeString;

// SD Write
unsigned long lastWrittenTime;
int writeFrequency = 100; // 4Hz (interval in ms)

// Recording button
AceButton button(recordingButtonPin);

// Recording button pressed
void recordButtonPressed() {
  // If calibration was not finished then do nothing
  if (isCalibrated) {
    if (isRecording) {
      // Inform SD writer that recording was stopped
      Wire.beginTransmission(2);  
      String mkFileString = ":closeFile\n";
      char mkFileToSend[mkFileString.length()+1];
      mkFileString.toCharArray(mkFileToSend, mkFileString.length()+1);  
      Wire.write(mkFileToSend);
      Wire.endTransmission();  
    } else {
      // Inform SD writer that recording was started
      Wire.beginTransmission(2);  
      String mkFileString = ":createNewFile\n";
      char mkFileToSend[mkFileString.length()+1];
      mkFileString.toCharArray(mkFileToSend, mkFileString.length()+1);  
      Wire.write(mkFileToSend);
      Wire.endTransmission();  
    }
    isRecording = !isRecording;
  }  
}

void handleButtonEvent(AceButton*, uint8_t, uint8_t);

void setup() {
  Serial.println("Starting module...");
  Wire.begin();
  Serial.begin(115200);
  delay(1000);

  // At first the led is turned on
  pinMode(recordingLedPin, OUTPUT);
  digitalWrite(recordingLedPin, HIGH);

  // Recording button
  pinMode(recordingButtonPin, INPUT_PULLUP);
  button.setEventHandler(handleButtonEvent);

  initRTC();
  initGyroAcc();
  initBmp();
  initIRRpmCounters();
}

void handleButtonEvent(AceButton* /* button */, uint8_t eventType,
    uint8_t /* buttonState */) {
  switch (eventType) {
    case AceButton::kEventPressed:
      recordButtonPressed();
      break;
    case AceButton::kEventReleased:
      break;
  }
}

void loop() 
{    
  button.check();
  
  readGyroAcc();
  readBmp();
  readAirspeed();
  readDateTime();

  if (lastRPMCountTime == 0 || millis() - lastRPMCountTime > writeFrequency) {
    readRPMs();
    lastRPMCountTime = millis();
  }  
  
  // Write to SD card if recording is enabled
  if (isRecording) {
    if (lastBlinkTime == 0 || (millis() - lastBlinkTime > 500.0)) {
      if (lastBlink == LOW) {
        digitalWrite(recordingLedPin, HIGH);
        lastBlink = HIGH;
      } else {
        digitalWrite(recordingLedPin, LOW);
        lastBlink = LOW;
      }
    }
    writeAll();
  } else {
    digitalWrite(recordingLedPin, LOW);
  }
  
  //printGyroAcc();
  //printBmpPressure();
  //printAirspeed();
  //printDateTime();
  //printRPMs();
}

String gyroAccString1 = "";
String gyroAccString2 = "";
String bmpString = "";
String pxString = "";
String rpmString = "";

char dateTimeToSend[100];
char gyroAccToSend1[100];
char gyroAccToSend2[100];
char gyroAccToSend3[100];
char gyroAccToSend4[100];
char bmpToSend[100];
char pxToSend[100];
char rpmToSend[100];

void writeAll() {
  if (lastWrittenTime == 0 || (millis() - lastWrittenTime > writeFrequency)) {

    //Serial.println("Writing to SD");    
        
    gyroAccString1 += accGyroRotation[2] * 180/M_PI;
    gyroAccString1 += ",";
    gyroAccString1 += accGyroRotation[1] * 180/M_PI;
    gyroAccString1 += ",";
    gyroAccString1 += accGyroRotation[0] * 180/M_PI;    
    gyroAccString1 += ",";
    gyroAccString2 += acc.acceleration.x * 0.101972;
    gyroAccString2 += ",";
    gyroAccString2 += acc.acceleration.y * 0.101972;
    gyroAccString2 += ",";
    gyroAccString2 += acc.acceleration.z * 0.101972;
    gyroAccString2 += ",";
    
    bmpString += bmpPressure;
    bmpString += ",";
    bmpString += bmpAltitude;
    bmpString += ",";
    bmpString += bmpTemperature;
    bmpString += ",";  
    
    pxString += px_pressure;
    pxString += ",";
    pxString += px_psi;
    pxString += ",";
    pxString += px_velocity;
    pxString += ",";    
    
    rpmString += rpm1;
    rpmString += ",";
    rpmString += rpm2;
    rpmString += ",";
    rpmString += rpm3;
    rpmString += ",";
    rpmString += rpm4;
    
    // Send dateTime
    
    Wire.beginTransmission(2);
    
    dateTimeString += ",";    
    dateTimeString.toCharArray(dateTimeToSend, dateTimeString.length()+1);

    //Serial.print("Sending: ");
    //Serial.println(dateTimeToSend);    
    
    Wire.write(dateTimeToSend);
    Wire.endTransmission();    
    
    // Send gyro and acc
    
    Wire.beginTransmission(2);
    
    gyroAccString1.toCharArray(gyroAccToSend1, gyroAccString1.length()+1);

    //Serial.print("Sending: ");
    //Serial.println(gyroAccToSend1);  

    Wire.write(gyroAccToSend1);
    Wire.endTransmission();

    Wire.beginTransmission(2);
    
    gyroAccString2.toCharArray(gyroAccToSend2, gyroAccString2.length()+1);

    //Serial.print("Sending: ");
    //Serial.println(gyroAccToSend2);  

    Wire.write(gyroAccToSend2);
    Wire.endTransmission();
    
    // Send BMP
    
    Wire.beginTransmission(2);
    
    bmpString.toCharArray(bmpToSend, bmpString.length()+1);

    //Serial.print("Sending: ");
    //Serial.println(bmpToSend);

    Wire.write(bmpToSend);
    Wire.endTransmission();  
    
    // Send PX
    
    Wire.beginTransmission(2);
    
    pxString.toCharArray(pxToSend, pxString.length()+1);

    //Serial.print("Sending: ");
    //Serial.println(pxToSend);

    Wire.write(pxToSend);
    Wire.endTransmission();
    
    // Send RPMs
    
    Wire.beginTransmission(2);     
    
    rpmString.toCharArray(rpmToSend, rpmString.length()+1);

    //Serial.print("Sending: ");
    //erial.println(rpmToSend);
    
    Wire.write(rpmToSend);
    Wire.endTransmission();
    
    // Send EOD
    Wire.beginTransmission(2);
    Wire.write("\n");
    Wire.endTransmission();

    gyroAccString1 = "";
    gyroAccString2 = "";
    bmpString = "";
    pxString = "";
    rpmString = "";
    
    lastWrittenTime = millis();
  }  
}

void initGyroAcc() {
  // Address conflict resolution (Gyro+Acc will have address 0x69 instead of 0x68 which is the same as RTC)
  pinMode(11, OUTPUT); //original (4)
  digitalWrite(11, HIGH); //original (4)

  // Init one library for acc retreive
  if (!mpu.begin(0x69)) {
    Serial.println("MPU6050 failed to find using library Adafruit_MPU6050");
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  // set gyro range to +- 500 deg/s
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  // set filter bandwidth to 21 Hz
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  
  // Init other library for gyro
  accGyroMPU.initialize();
  bool connectionTest = accGyroMPU.testConnection();
  if (connectionTest) {
    Serial.println("Gyro and Acc connected");
    accGyroDevStatus = accGyroMPU.dmpInitialize();
    accGyroMPU.setZAccelOffset(1788);
    if (accGyroDevStatus == 0) {
      accGyroMPU.setDMPEnabled(true);
      attachInterrupt(0, accGyroDMPInterrupt, RISING);
      accGyroMPUIntStatus = accGyroMPU.getIntStatus();
      accGyroDMPReady = true;
      accGyroPacketSize = accGyroMPU.dmpGetFIFOPacketSize();
    } else {
      Serial.print(F("DMP initialization failed with (code "));
      Serial.print(accGyroDevStatus);
      Serial.println(F(")"));
    }    
  }
  else {
    Serial.println("Gyro and Acc connection failed");
  }

  // Calibration
  Serial.println("Calibrating gyro/acc...\n");
  delay(2000);
  Serial.println("\nYour MPU6050 should be placed in horizontal position, with package letters facing up. \nDon't touch it until you see a finish message.\n");
  delay(3000);

  accGyroMPU.setXAccelOffset(0);
  accGyroMPU.setYAccelOffset(0);
  accGyroMPU.setZAccelOffset(0);
  accGyroMPU.setXGyroOffset(0);
  accGyroMPU.setYGyroOffset(0);
  accGyroMPU.setZGyroOffset(0);

  
  gyroAccMeanSensors();
  gyroAccCalibration();
  gyroAccMeanSensors();

  Serial.println("\nFINISHED!");
  isCalibrated = true;
  // Led will turn off and the device is ready for recording.
  digitalWrite(recordingLedPin, LOW);
  
}

void initBmp() {
  if (!bmp.begin()) {
    Serial.println("BMP not connected");
  } else {
    Serial.println("BMP connected");
  }
}

void initRTC() {
  rtc.begin();
  rtc.setDateTime(__DATE__, __TIME__);
}

void initIRRpmCounters() {
  attachInterrupt(digitalPinToInterrupt (2), irRpm1Interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt (3), irRpm1Interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt (9), irRpm1Interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt (10), irRpm1Interrupt, RISING);
}

void readGyroAcc() {
  // Get event for ACC and temperature from MPU chip
  mpu.getEvent(&acc, &g, &temp); 
  
  if (!accGyroDMPReady) return;
  
  accGyroMPUInterrupted = false;
  accGyroMPUIntStatus = accGyroMPU.getIntStatus();
  accGyroFifoCount = accGyroMPU.getFIFOCount();

  if ((accGyroMPUIntStatus & 0x10) || accGyroFifoCount == 1024) {
    accGyroMPU.resetFIFO();
    //Serial.println("AccGyro buffer overflow!");
  } else if (accGyroMPUIntStatus & 0x02) {
     while (accGyroFifoCount < accGyroPacketSize) accGyroFifoCount = accGyroMPU.getFIFOCount();
     accGyroMPU.getFIFOBytes(accGyroFifoBuffer, accGyroPacketSize);
     accGyroFifoCount -= accGyroPacketSize;

     accGyroMPU.dmpGetQuaternion(&accGyroQuaternion, accGyroFifoBuffer);
     accGyroMPU.dmpGetGravity(&accGyroGravity, &accGyroQuaternion);
     accGyroMPU.dmpGetYawPitchRoll(accGyroRotation, &accGyroQuaternion, &accGyroGravity);
     accGyroMPU.dmpGetAccel(&accGyroAcceleration, accGyroFifoBuffer);
     accGyroMPU.dmpGetLinearAccel(&accGyroRealAcceleration, &accGyroAcceleration, &accGyroGravity);
     accGyroMPU.dmpGetLinearAccelInWorld(&accGyroRealWorldAcceleration, &accGyroRealAcceleration, &accGyroQuaternion);
  }  
}

void readBmp() {
  bmpPressure = bmp.readPressure();
  bmpAltitude = bmp.readAltitude();
  bmpSealevelPressure = bmp.readSealevelPressure();
  bmpRealAltitude = bmp.readAltitude(STANDARD_BAROMETRIC_PRESSURE);
  bmpTemperature = bmp.readTemperature();
}

void readAirspeed() {
  Wire.beginTransmission(PX_ADDRESS_SLAVE);  
  Wire.write(PX_REGISTER_XY);
  Wire.endTransmission(false);
  
  Wire.requestFrom(PX_ADDRESS_SLAVE,PX_READ_LENGTH);
  byte buff[PX_READ_LENGTH];    
  Wire.readBytes(buff, PX_READ_LENGTH);

  byte pressure_h = buff[0];
  byte pressure_l = buff[1];
  px_pressure = (((uint16_t)pressure_h) << 8) | pressure_l;

  px_psi = (static_cast<float>(static_cast<int16_t>(px_pressure)-MS4525ZeroCounts))/static_cast<float>(MS4525Span)*static_cast<float>(MS4525FullScaleRange);
  
  if (px_psi<0) {
    px_velocity = -sqrt(-(2*px_psi) / px_airDensity);
  } else {
    px_velocity = sqrt((2*px_psi) / px_airDensity);
  }
  px_velocity = px_velocity*10;
}

void readRPMs() {
  float timeDifference;
  
  detachInterrupt(digitalPinToInterrupt(2));
  timeDifference = millis() - rpm1LastCountTime;
  rpm1 = ((revs1 / timeDifference) * 60000)/2;
  rpm1LastCountTime = millis();
  revs1 = 0;
  attachInterrupt(digitalPinToInterrupt (2), irRpm1Interrupt, RISING);

  detachInterrupt(digitalPinToInterrupt(3));
  timeDifference = millis() - rpm2LastCountTime;
  rpm2 = ((revs2 / timeDifference) * 60000)/2;
  rpm2LastCountTime = millis();
  revs2 = 0;
  attachInterrupt(digitalPinToInterrupt (3), irRpm2Interrupt, RISING);

  detachInterrupt(digitalPinToInterrupt(9));
  timeDifference = millis() - rpm3LastCountTime;
  rpm3 = ((revs3 / timeDifference) * 60000)/2;
  rpm3LastCountTime = millis();
  revs3 = 0;
  attachInterrupt(digitalPinToInterrupt (9), irRpm3Interrupt, RISING);

  detachInterrupt(digitalPinToInterrupt(10));
  timeDifference = millis() - rpm4LastCountTime;
  rpm4 = ((revs4 / timeDifference) * 60000)/2;
  rpm4LastCountTime = millis();
  revs4 = 0;
  attachInterrupt(digitalPinToInterrupt (10), irRpm4Interrupt, RISING);
}

void readDateTime() {
  RTCDateTime dateTime = rtc.getDateTime();
  String timeDateString = "";
  timeDateString += dateTime.year;
  timeDateString += "-";
  timeDateString += dateTime.month;
  timeDateString += "-";
  timeDateString += dateTime.day;
  timeDateString += " ";
  timeDateString += dateTime.hour;
  timeDateString += ":";
  timeDateString += dateTime.minute;
  timeDateString += ":";
  timeDateString += dateTime.second;
  dateTimeString = timeDateString;
}

void printGyroAcc() {
  Serial.print("Acceleration in G X: ");
  Serial.print(acc.acceleration.x * 0.101972);
  Serial.print(", Y: ");
  Serial.print(acc.acceleration.y * 0.101972);
  Serial.print(", Z: ");
  Serial.print(acc.acceleration.z * 0.101972);
  Serial.print(" g \t");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.print(" degC \t");
  
  if (!accGyroDMPReady) return;
  Serial.print("Rotation \t X: ");
  Serial.print(accGyroRotation[2] * 180/M_PI);
  Serial.print("\t Y: ");
  Serial.print(accGyroRotation[1] * 180/M_PI);
  Serial.print("\t Z: ");
  Serial.println(accGyroRotation[0] * 180/M_PI);  
  /*
  Serial.print("\t Real world acceleration \t X:");
  Serial.print(accGyroRealAcceleration.x);
  Serial.print("\t Y: ");
  Serial.print(accGyroRealAcceleration.y);
  Serial.print("\t Z: ");
  Serial.println(accGyroRealAcceleration.z);
  */
}

void printBmpPressure() {
  Serial.print("Pressure: ");
  Serial.print(bmpPressure);
  Serial.print(" Pa\t");
  Serial.print("Sea level pressure: ");
  Serial.print(bmpSealevelPressure);
  Serial.print(" Pa\t");
  Serial.print("Altitude: ");
  Serial.print(bmpAltitude);
  Serial.print(" meters\t");
  Serial.print("Real altitude: ");
  Serial.print(bmpRealAltitude);
  Serial.println(" meters\t");
}

void printAirspeed() {
  Serial.print("Pressure(PX): ");
  Serial.print(px_pressure);
  Serial.print("\t");
  Serial.print("PSI(PX): ");
  Serial.print(px_psi);
  Serial.print("\t");
  Serial.print("Velocity(PX): ");
  Serial.print(px_velocity);
  Serial.println("\t");
}

void printDateTime() {
  RTCDateTime dateTime = rtc.getDateTime();
  Serial.print("Date and time: ");
  Serial.print(dateTime.year);   Serial.print("-");
  Serial.print(dateTime.month);  Serial.print("-");
  Serial.print(dateTime.day);    Serial.print(" ");
  Serial.print(dateTime.hour);   Serial.print(":");
  Serial.print(dateTime.minute); Serial.print(":");
  Serial.print(dateTime.second); Serial.print("       ");
  Serial.println();
}

void printRPMs() {
  Serial.print("RPM1: ");
  Serial.print(rpm1);
  Serial.print("\t");
  Serial.print("RPM2: ");
  Serial.print(rpm2);
  Serial.print("\t");
  Serial.print("RPM3: ");
  Serial.print(rpm3);
  Serial.print("\t");
  Serial.print("RPM4: ");
  Serial.print(rpm4);
  Serial.print("\t");
  Serial.println();
}


void gyroAccMeanSensors(){
  long i=0,buff_ax=0,buff_ay=0,buff_az=0,buff_gx=0,buff_gy=0,buff_gz=0;

  while (i<(buffersize+101)){
    accGyroMPU.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    
    if (i>100 && i<=(buffersize+100)){
      buff_ax=buff_ax+ax;
      buff_ay=buff_ay+ay;
      buff_az=buff_az+az;
      buff_gx=buff_gx+gx;
      buff_gy=buff_gy+gy;
      buff_gz=buff_gz+gz;
    }
    if (i==(buffersize+100)){
      mean_ax=buff_ax/buffersize;
      mean_ay=buff_ay/buffersize;
      mean_az=buff_az/buffersize;
      mean_gx=buff_gx/buffersize;
      mean_gy=buff_gy/buffersize;
      mean_gz=buff_gz/buffersize;
    }
    i++;
    delay(2);
  }
}


void gyroAccCalibration(){
  ax_offset=-mean_ax/8;
  ay_offset=-mean_ay/8;
  az_offset=(16384-mean_az)/8;

  gx_offset=-mean_gx/4;
  gy_offset=-mean_gy/4;
  gz_offset=-mean_gz/4;
  while (1){
    int ready=0;
    accGyroMPU.setXAccelOffset(ax_offset);
    accGyroMPU.setYAccelOffset(ay_offset);
    accGyroMPU.setZAccelOffset(az_offset);

    accGyroMPU.setXGyroOffset(gx_offset);
    accGyroMPU.setYGyroOffset(gy_offset);
    accGyroMPU.setZGyroOffset(gz_offset);

    gyroAccMeanSensors();
    Serial.println("...");

    if (abs(mean_ax)<=acel_deadzone) ready++;
    else ax_offset=ax_offset-mean_ax/acel_deadzone;

    if (abs(mean_ay)<=acel_deadzone) ready++;
    else ay_offset=ay_offset-mean_ay/acel_deadzone;

    if (abs(16384-mean_az)<=acel_deadzone) ready++;
    else az_offset=az_offset+(16384-mean_az)/acel_deadzone;

    if (abs(mean_gx)<=gyro_deadzone) ready++;
    else gx_offset=gx_offset-mean_gx/(gyro_deadzone+1);

    if (abs(mean_gy)<=gyro_deadzone) ready++;
    else gy_offset=gy_offset-mean_gy/(gyro_deadzone+1);

    if (abs(mean_gz)<=gyro_deadzone) ready++;
    else gz_offset=gz_offset-mean_gz/(gyro_deadzone+1);

    if (ready==6) break;
  }
}
