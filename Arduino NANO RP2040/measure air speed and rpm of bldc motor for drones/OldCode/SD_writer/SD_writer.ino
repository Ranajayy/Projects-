#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "cppQueue.h"

File csvFile;
int const pinCS = 12;

unsigned long lastWrittenLineTime;
unsigned long lastFlushTime;

bool sdInitialized = false;
bool fileIsReady = false;

char receiveBuffer[40];
int receiveLength;
boolean lineIsReady = false;

String receivedBufferString = "";

String currentLine;

void setup() {  
  Wire.begin(2);
  Serial.begin(9600);  
  Wire.onReceive(receiveEvent);
}

void loop() {    
  if (!sdInitialized) {
    initSd();
  }
  
  if (csvFile && (millis() - lastFlushTime > 1000)) {
    csvFile.flush();
    lastFlushTime = millis();   
  }
  
  if (lineIsReady && currentLine != "") {
    if (currentLine == ":createNewFile\n"){
      createNewCSVFile();
    } else if (currentLine == ":closeFile\n") {
      csvFile.flush();
      lastFlushTime = millis();
      csvFile.close();
      fileIsReady = false;
    } else {
      writeToFile(currentLine);
    }    
    currentLine = "";
    lineIsReady = false;
  }  
}

void receiveEvent(int l)
{
  while (Wire.available()) {
    char c = Wire.read();
    currentLine += c;
    //Serial.print(c);
    if (c == '\n')  {
      //Serial.print(currentLine);
      lineIsReady = true;            
    }
  }
}

void initSd() {
  Serial.print("SD card initialization...");
  if (!SD.begin(pinCS))
  {
    Serial.println(" inicialization failed!");
    delay(1000);
  } else {
    Serial.println(" inicialization successful!");
    sdInitialized = true;
  }  
}

void writeToFile(String toWrite) {
  if (fileIsReady && toWrite != "") {
    Serial.print("Writing to file: ");
    Serial.print(toWrite);
    csvFile.print(toWrite);
    lastWrittenLineTime = millis();
  }  
}

void createNewCSVFile() {  
  if (!sdInitialized) {
    return;
  }
  
  int filenamenumber = findLastRecordFileNumber()+1;  
  //int filenamenumber = 10;
  Serial.print("File number: ");
  Serial.println(filenamenumber);

  String filename = "RECORD";
  if (filenamenumber < 10) {
    filename += "0";
  }
  filename += String(filenamenumber);
  filename += ".csv";
  
  Serial.print("Creating new file: ");
  Serial.println(filename);
  csvFile = SD.open(filename, FILE_WRITE);

  if (csvFile) {
    Serial.println("File created!");
    csvFile.println("datetime,rotX,rotY,rotZ,accX,accY,accZ,pressure,altitude,temperature,pxPressure,pxPsi,pxVelocity,rpm1,rpm2,rpm3,rpm4");
    fileIsReady = true;    
  } else {
    Serial.println("File cannot be created or opened!");
    delay(1000);
    createNewCSVFile();
  } 
  lastWrittenLineTime = millis();
}

int findLastRecordFileNumber() {
  int lastfilenumber = 0;
  File root = SD.open("/");
  while(true) {
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }
    String filename = entry.name();
    if (!entry.isDirectory() && filename.startsWith("RECORD")) {
      char filenameChar[filename.length()];
      filename.toCharArray(filenameChar, filename.length());

      char fileNumber[2];
      fileNumber[0] = filenameChar[6];
      fileNumber[1] = filenameChar[7];

      String fileNumberString = String(fileNumber);
      
      int fileNumberINT = fileNumberString.toInt();
      if (lastfilenumber < fileNumberINT) lastfilenumber = fileNumberINT;            
    }
    entry.close();
  }
  return lastfilenumber;
}
