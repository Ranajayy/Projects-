#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "OneButton.h"
#include <SPI.h>
#include <SD.h>


#define CLK 2
#define DT 3
#define led 4
#define relay1 5
#define relay2 6
#define relay3 7
#define relay4 8
#define relay5 9
#define relay6 10
#define relay7 11
#define relay8 12
#define power_pin 14
#define send_sms_but 15
#define pinSW 22
#define but 23
#define buzz 24
#define delte_menu_but 25
#define chipSelect 53

#define MAX_FILE_COUNT 50 // Maximum number of files/folders to display
#define MAX_NAME_LENGTH 20 // Maximum length of file/folder names



// Set the LCD address to 0x27 for a 20x4 display
LiquidCrystal_I2C lcd(0x27, 20, 4);

// encoder button //
OneButton button(pinSW, true);
OneButton button2(send_sms_but, true);
OneButton button3(delte_menu_but, true);


// integers //

int msg_c = 0;
int fileCount = 0; // Number of files/folders in the array
int currentIndex = 0; // Index of the currently highlighted file/folder
int firstIndex = 0; // Index of the first file/folder to display on the screen
int lastIndex = 0; // Index of the last file/folder to display on the screen
int currentRelay = 1; // the current relay that is on
int currentStateCLK;
int lastStateCLK;
int count = 0;
int min_count = 0;
int sms_count = 0;
int n = 0;
int f_index = 0;
int f_count = 0;
int rf_count = 0;
int m_no_index = 3;
int rotate = 0;
// Define a struct to hold file/folder information
struct FileInfo
{
  char name[MAX_NAME_LENGTH];
  bool isFolder;
};
FileInfo fileList[MAX_FILE_COUNT]; // Array to hold file/folder information

unsigned long previousMillis = 0; // stores the last time the relay was updated
unsigned long interval = 250; // interval at which to update the relay (in milliseconds)
unsigned long new_sms_millis = 0;
unsigned long   file_disp_millis = 0;
unsigned long disp_sms_millis = 0;

String phoneNumber;
String message;
String files[50];
String rfiles[50];
String mobile_no[5] = {"+923005047036", "+923405243603", "+923052069794", "+923175486941", "+923055400609"};
String saved_text = "Hello from GSM Module kaise ho  sb";
const char* folderName = "/SMS";

bool sms_disp = false;
bool new_sms = false;
bool file_show_flag = false;
bool enc = false;
bool normal_mode = true;
bool delte_sms = false;
bool selec = false;
void(* resetFunc) (void) = 0;

void setup()
{
  lcd.init();
  lcd.clear();
  delay(50);

  pinMode(power_pin, INPUT);
  while (digitalRead(power_pin) == 0);

  Serial.begin(9600);
  Serial1.begin(9600);
  // Enable the SMS notification
  Serial1.println("AT+CMGF=1");
  updateSerial();
  Serial1.println("AT+CNMI=2,2,0,0,0");
  updateSerial();
  Serial1.flush();
  if (!SD.begin(chipSelect))
  {
    Serial.println("sd failed");
    while (1);
  }
  Serial.println("sd initialzied!");

  randomSeed(analogRead(0));
  lastStateCLK = digitalRead(CLK);

  pinMode(but, INPUT);
  pinMode(led, OUTPUT);
  pinMode(buzz, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(relay5, OUTPUT);
  pinMode(relay6, OUTPUT);
  pinMode(relay7, OUTPUT);
  pinMode(relay8, OUTPUT);

  attachInterrupt(0, updateEncoder, CHANGE);
  attachInterrupt(1, updateEncoder, CHANGE);

  button.setPressTicks(3000);
  button.attachClick(singleClick);
  button2.attachClick(send_sms_fucntion);
  button3.attachClick(delete_sms_menu);

  lcd.backlight();
  delay(500);
  lcd.noBacklight();
  delay(500);
  lcd.backlight();
  delay(500);
  lcd.noBacklight();

  relay_t();
  cycle(2);
  lcd.backlight();
  delay(500);
  lcd.noBacklight();
  delay(500);
  lcd.backlight();
  list_text_files();
  display_char("Awaiting Transmission...");
  new_sms_millis = millis();

}

void loop()
{

  if (digitalRead(power_pin) == 1)
  {
    main_code();
  }
  else
  {
    lcd.clear();
    lcd.noBacklight();
    resetFunc();
  }
}


void delete_sms_menu()
{
  if (delte_sms == false)
  {
    // Serial.println("in delte");
    normal_mode = false;
    delte_sms = true;
    if (n == 1)
    {
      n = 0;
      list_msg("/SMS");
      if (f_count > 0)
      {
        lcd.clear();
        lcd.setCursor(0, count);
        lcd.print(">");

        int cal1 = floor(f_index / 4);

        //Serial.println(cal1);
        int cal = 4 * cal1;
        lcd.setCursor(1, 0);
        lcd.print(files[cal]);
        lcd.setCursor(1, 1);
        lcd.print(files[cal + 1]);
        lcd.setCursor(1, 2);
        lcd.print(files[cal + 2]);
        lcd.setCursor(1, 3);
        lcd.print(files[cal + 3]);
      }
      else
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("no sms available");

      }
    }
  }
  else
  {
    delte_sms = false;
    normal_mode = true;
    n = 0;
    rotate = 0;
  }
}

void main_code()
{
  button.tick();
  button2.tick();
  button3.tick();

  if (normal_mode && delte_sms == false)
  {
    if (n == 0)
    {
      Serial.println("reseting millis!");
      lcd.clear();
      n = 1;
      new_sms_millis = millis();
      file_disp_millis = millis();
      file_show_flag = false;
      count = 0;
      f_index = 0;
      display_char("Awaiting Transmission...");
      min_count = 0;
    }

    recv_sms();

    if (millis() - new_sms_millis > 30000 && new_sms == false && file_show_flag == false && sms_disp == false) //1 hour! for test write 30 seconds
    {
      new_sms_millis = millis();
      file_disp_millis = millis();
      file_show_flag = true;
      Serial.println("show random files here!");
    }

    // print sms on display //

    if (digitalRead(but) == HIGH && new_sms == true)
    {
      Serial.println("but");
      disp_sms_millis = millis();
      new_sms = false;
      lcd.clear();
      display_sms(message);
      delay(200);
    }

    if (digitalRead(but) == HIGH && sms_disp == true)
    {
      lcd.clear();
      display_sms(message);
    }

    // remove sms from display //

    if (millis() - disp_sms_millis > 30000 && sms_disp == true) //10 mins for now 10 seconds
    {
      Serial.println("removed msg from display");
      new_sms_millis = millis();
      sms_disp = false;
      new_sms = false;
      lcd.clear();
      display_char("Awaiting Transmission...");
    }


    //display random file after 1min for 10 mins//

    if (file_show_flag == true && new_sms == false)
    {
      if (millis() - file_disp_millis > 10000)  // put 1 min here for now 10 seconds
      {
        //Serial.println(min_count);
        lcd.clear();
        file_disp_millis = millis();
        open_txt_file();
        min_count++;
      }

      if (min_count > 10)
      {
        min_count = 0;
        lcd.clear();
        display_char("Awaiting Transmission...");
        file_show_flag = false;
        new_sms_millis = millis();
      }
    }
  }

  else
  {
    if (enc == true)
    {
      enc = false;
      if (delte_sms == true && rotate == 0)
      {
        // Serial.println("rotate0 is working ");
        // Serial.println(f_index);
        lcd.clear();
        lcd.setCursor(0, count);
        lcd.print(">");

        int cal1 = floor(f_index / 4);

        //Serial.println(cal1);
        int cal = 4 * cal1;
        lcd.setCursor(1, 0);
        lcd.print(files[cal]);
        lcd.setCursor(1, 1);
        lcd.print(files[cal + 1]);
        lcd.setCursor(1, 2);
        lcd.print(files[cal + 2]);
        lcd.setCursor(1, 3);
        lcd.print(files[cal + 3]);
      }
      if ((delte_sms == true || selec == true) && rotate == 1)
      {
        Serial.println("rotate1 is working ");
        Serial.println(count);
        Serial.println("finex: " + (String)f_index);
        lcd.clear();
        lcd.setCursor(0, count);
        lcd.print(">");

        int cal1 = floor(m_no_index / 4);

        //Serial.println(cal1);
        int cal = 4 * cal1;
        lcd.setCursor(1, 0);
        lcd.print("DELETE");
        lcd.setCursor(1, 1);
        lcd.print("VIEW");
        lcd.setCursor(1, 2);
        lcd.print("BACK");
      }
    }
  }
}

void singleClick()
{

  if (normal_mode == false)
  {
    Serial.println("in normal false");
    if (delte_sms)
    {
      Serial.println("here");
      delte_sms = false;
      rotate = 1;
      lcd.clear();
      lcd.setCursor(0, count);
      lcd.print(">");

      int cal1 = floor(3 / 4);
      int cal = 4 * cal1;
      lcd.setCursor(1, 0);
      lcd.print("DELETE");
      lcd.setCursor(1, 1);
      lcd.print("VIEW");
      lcd.setCursor(1, 2);
      lcd.print("BACK");
      selec = true;
    }

    else if (selec == true  && count == 0)
    {
      rotate = 0;
      selec = false;
      delte_sms = true;

      // Serial.println("F_index: " + String(f_index));
      // Serial.println("file name: " + files[f_index]);

      if (SD.exists("/SMS/" + String(files[f_index])))
      {
        if (SD.remove("/SMS/" + String(files[f_index])))
        {
          Serial.println("File deleted.");
          f_index = 0;
          count = 0;
        }
        else
        {
          Serial.println("Error deleting file.");
        }
      }
      else
      {
        Serial.println("File does not exist.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("No files");
      }

      memset(files, 0, sizeof(files));
      list_msg("/SMS");
      lcd.clear();
      lcd.setCursor(0, count);
      lcd.print(">");

      int cal1 = floor(f_index / 4);
      int cal = 4 * cal1;

      lcd.setCursor(1, 0);
      lcd.print(files[cal]);
      lcd.setCursor(1, 1);
      lcd.print(files[cal + 1]);
      lcd.setCursor(1, 2);
      lcd.print(files[cal + 2]);
      lcd.setCursor(1, 3);
      lcd.print(files[cal + 3]);
    }

    else  if (selec ==  true && count == 1)
    {
      rotate = 0;
      count = 0;
      selec = false;
      Serial.println("sms view!");
      delte_sms = true;
      lcd.clear();
      if (SD.exists("/SMS/" + String(files[f_index])))
      {
        File file = SD.open("/SMS/" + String(files[f_index]));

        // Check if the file opened successfully
        if (!file)
        {
          Serial.println("Failed to open file");
          return;
        }
        String content = file.readString();
        file.close();
        display_file(content);
      }
      else
      {
        Serial.println("no sms to open!");
      }
      f_index = 0;
    }

    else if (selec == true && count == 2)
    {
      rotate = 0;
      f_index = 0;
      count = 0;
      selec = false;
      Serial.println("sms back!");
      delte_sms = true;
      list_msg("/SMS");
      if (f_count > 0)
      {
        lcd.clear();
        lcd.setCursor(0, count);
        lcd.print(">");

        int cal1 = floor(f_index / 4);

        //Serial.println(cal1);
        int cal = 4 * cal1;
        lcd.setCursor(1, 0);
        lcd.print(files[cal]);
        lcd.setCursor(1, 1);
        lcd.print(files[cal + 1]);
        lcd.setCursor(1, 2);
        lcd.print(files[cal + 2]);
        lcd.setCursor(1, 3);
        lcd.print(files[cal + 3]);
      }
    }
  }
}

void updateEncoder()
{
  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);
  if (currentStateCLK != lastStateCLK)
  {
    enc = true;
    if (digitalRead(DT) != currentStateCLK)
    {

      count--;

      if (rotate == 1)
      {
        m_no_index--;
        //  Serial.println("rotate1: " + String(m_no_index));
        if (m_no_index < 0)
        {
          m_no_index = 0;
          count = 0;
        }
      }

      else
      {
        f_index--;
        if (f_index < 0)
        {
          // Serial.println("rotate0: " + String(f_index));
          f_index = 0;
          count = 0;
        }
      }



      if (count < 0)
      {
        count = 0;
        f_index = 0;
        m_no_index = 0;
      }

    }

    else
    {

      count++;


      if (rotate == 1)
      {
        m_no_index++;
        if (m_no_index > 3 - 1)
        {
          m_no_index = 0;
          count = 0;
          //  Serial.println("rotate1: " + String(m_no_index));
        }
      }
      else
      {
        f_index++;
        if (f_index > f_count - 1)
        {

          f_index = 0;
          count = 0;
          // Serial.println("rotate0: " + String(f_index));
        }
      }

      if (count > 3)
      {
        count = 0;
      }

    }
  }

  lastStateCLK = currentStateCLK;
}


void send_sms_fucntion()
{
  n = 0;
  normal_mode = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sending SMS!");

  Send_Message(mobile_no[0]);
  Serial.println(mobile_no[0]);
  delay(6000);
  Send_Message(mobile_no[1]);
  Serial.println(mobile_no[1]);
  delay(6000);
  Send_Message(mobile_no[2]);
  Serial.println(mobile_no[2]);
  delay(6000);
  Send_Message(mobile_no[3]);
  Serial.println(mobile_no[3]);
  delay(6000);
  Serial.println(mobile_no[4]);
  Send_Message(mobile_no[4]);
  delay(6000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMS Sent!");
  delay(1000);
  normal_mode = true;
}


void list_text_files()
{

  File root = SD.open("/");
  if (!root)
  {
    Serial.println("Failed to open root directory");
    return;
  }

  while (File file = root.openNextFile())
  {

    if (!file.isDirectory())
    {
      Serial.println(file.name());
      rfiles[rf_count] = file.name();
      rf_count++;
    }
    file.close();
  }

}

void open_txt_file()
{
  //Serial.println(rf_count);
  int r_f_C = random(rf_count);
  Serial.println(r_f_C);
  File file = SD.open("/" + String(rfiles[r_f_C]));

  // Check if the file opened successfully
  if (!file)
  {
    Serial.println("Failed to open file");
    return;
  }
  String content = file.readString();
  display_char(content);
  file.close();
}

void list_msg(String folder_name)
{
  // Clear the file list array and reset the file count
  memset(fileList, 0, sizeof(fileList));
  fileCount = 0;

  // Open the current directory
  File directory = SD.open(folder_name);

  // Iterate over the files/folders in the directory
  for (File file = directory.openNextFile(); file && fileCount < MAX_FILE_COUNT; file = directory.openNextFile())
  {
    // Check if the file/folder name is too long
    if (strlen(file.name()) > MAX_NAME_LENGTH)
    {
      continue;
    }

    Serial.println(file.name());
    files[fileCount] = file.name();
    // Check if the file is a folder
    fileList[fileCount].isFolder = file.isDirectory();

    // Increment the file count
    fileCount++;
  }
  f_count = fileCount;
  //Serial.println(f_count);
  // Close the directory
  directory.close();
}

void save_msg()
{
  if (SD.exists(folderName))
  {
    Serial.println("Folder already exists.");
  }
  else {
    //Create folder if it doesn't exist
    if (SD.mkdir(folderName))
    {
      Serial.println("Folder created.");
    }
    else {
      Serial.println("Folder creation failed.");
      return;
    }
  }

  //Open a file inside the folder
  String path = "/SMS/" + String(sms_count) + ".txt";
  Serial.println(path);
  File myFile = SD.open(path.c_str(), FILE_WRITE);

  if (myFile)
  {
    myFile.println(message);

    //Close the file
    myFile.close();

  }
  else
  {
    Serial.println("failed");
  }
  //Write some text to the file

  Serial.println("Done.");
}

void cycle(int cycles)
{
  for (int i = 0; i < cycles; i++)
  {
    // reset currentRelay to 1 at the start of each cycle
    currentRelay = 1;

    // turn on the first relay to start the cycle
    digitalWrite(relay1, HIGH);

    // cycle through all relays
    for (int j = 0; j < 8; j++)
    {
      // wait for the interval before updating the relays
      unsigned long currentMillis = millis();
      while (currentMillis - previousMillis < interval)
      {
        currentMillis = millis();
      }
      previousMillis = currentMillis;

      // turn off the previous relay
      digitalWrite(currentRelay, LOW);

      // turn on the next relay
      currentRelay++;
      // Serial.println(currentRelay);

      if (currentRelay == 1) {
        digitalWrite(relay1, HIGH);
      } else if (currentRelay == 2) {
        digitalWrite(relay2, HIGH);
      } else if (currentRelay == 3) {
        digitalWrite(relay3, HIGH);
      } else if (currentRelay == 4) {
        digitalWrite(relay4, HIGH);
      } else if (currentRelay == 5) {
        digitalWrite(relay5, HIGH);
      } else if (currentRelay == 6) {
        digitalWrite(relay6, HIGH);
      } else if (currentRelay == 7) {
        digitalWrite(relay7, HIGH);
      } else if (currentRelay == 8) {
        digitalWrite(relay8, HIGH);
      }
    }

    // turn off all relays at the end of each cycle
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, LOW);
    digitalWrite(relay3, LOW);
    digitalWrite(relay4, LOW);
    digitalWrite(relay5, LOW);
    digitalWrite(relay6, LOW);
    digitalWrite(relay7, LOW);
    digitalWrite(relay8, LOW);
    delay(250);
    // wait for 2 seconds before starting the next cycle
  }
}

void relay_t()
{
  digitalWrite(relay1, HIGH);
  delay(500);
  digitalWrite(relay1, LOW);
  delay(500);
  digitalWrite(relay2, HIGH);
  delay(500);
  digitalWrite(relay2, LOW);
  delay(500);
  digitalWrite(relay3, HIGH);
  delay(500);
  digitalWrite(relay3, LOW);
  delay(500);
  digitalWrite(relay4, HIGH);
  delay(500);
  digitalWrite(relay4, LOW);
  delay(500);
  digitalWrite(relay5, HIGH);
  delay(500);
  digitalWrite(relay5, LOW);
  delay(500);
  digitalWrite(relay6, HIGH);
  delay(500);
  digitalWrite(relay6, LOW);
  delay(500);
  digitalWrite(relay7, HIGH);
  delay(500);
  digitalWrite(relay7, LOW);
  delay(500);
  digitalWrite(relay8, HIGH);
  delay(500);
  digitalWrite(relay8, LOW);
}

void Send_Message(String mobile_phone)
{
  Serial1.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  Serial1.println("AT+CMGS=\"" + String(mobile_phone) + "\""); //change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  Serial1.print(saved_text); //text content
  updateSerial();
  Serial1.write(0x1A);

}
void updateSerial()
{
  delay(500);
  while (Serial.available())
  {
    Serial1.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (Serial1.available())
  {
    Serial.write(Serial1.read());//Forward what Software Serial received to Serial Port
  }
}

void recv_sms()
{
  if (Serial1.available())
  {
    String response = Serial1.readString();
    Serial.println(response);
    if (response.indexOf("+CMT") != -1)
    {
      sms_disp = false;
      sms_count++;
      digitalWrite(buzz, HIGH);
      digitalWrite(led, HIGH);
      delay(500);
      digitalWrite(led, LOW);
      digitalWrite(buzz, LOW);
      cycle(2);
      lcd.clear();
      display_char("Transmission Recieved Acknowledged...?");
      new_sms = true;
      file_show_flag = false;
      // if the SMS is read successfully
      // phoneNumber = parsePhoneNumber(response);
      message = parseMessage(response);
      save_msg();
    }
    Serial1.flush();
  }
}

String parsePhoneNumber(String sms)
{
  int start = sms.indexOf("\"") + 1;
  int end = sms.indexOf("\"", start);
  return sms.substring(start, end);
}

String parseMessage(String sms)
{
  return sms.substring(51);
}


void display_char(String text)
{
  int length = text.length();
  int count = 0;
  int row = 0;
  int col = 0;
  int relay_letter_count = 0;
  if (text == "Transmission Recieved Acknowledged...?")
  {
    lcd.setCursor(0, 0);
    for (int i = 0; i < length; i++)
    {
      if (count >= 80)
      {
        //  Serial.println("here");
        while (digitalRead(but) == LOW);
        // Serial.println("here2");
        lcd.clear();
        count = 0;
        row = 0;
        col = 0;
        lcd.setCursor(0, 0);
      }
      if (count >= 20 && count < 40) {
        row = 1;
        col = count - 20;
        lcd.setCursor(col, row);
      }
      if (count >= 40 && count < 60) {
        row = 2;
        col = count - 40;
        lcd.setCursor(col, row);
      }
      if (count >= 60 && count < 80) {
        row = 3;
        col = count - 60;
        lcd.setCursor(col, row);
      }
      if (text.charAt(i) > 31)
      {
        lcd.print(text.charAt(i));
        col++;
        if (col >= 20)
        {
          col = 0;
          row++;
          if (row >= 4)
          {
            row = 0;
          }
          lcd.setCursor(0, row);
        }
        count++;
        relay_letter_count++;

        if (relay_letter_count > 8)
        {
          relay_letter_count = 1;
        }

        if (relay_letter_count == 1)
        {
          digitalWrite(relay1, LOW);
        }
        if (relay_letter_count == 2)
        {
          digitalWrite(relay2, LOW);
        }
        if (relay_letter_count == 3)
        {
          digitalWrite(relay3, LOW);
        }
        if (relay_letter_count == 4)
        {
          digitalWrite(relay4, LOW);
        }
        if (relay_letter_count == 5)
        {
          digitalWrite(relay5, LOW);
        }
        if (relay_letter_count == 6)
        {
          digitalWrite(relay6, LOW);
        }
        if (relay_letter_count == 7)
        {
          digitalWrite(relay7, LOW);
        }
        if (relay_letter_count == 8)
        {
          digitalWrite(relay8, LOW);
        }
        delay(250);
        if (relay_letter_count == 1)
        {
          digitalWrite(relay1, HIGH);
        }
        if (relay_letter_count == 2)
        {
          digitalWrite(relay2, HIGH);
        }
        if (relay_letter_count == 3)
        {
          digitalWrite(relay3, HIGH);
        }
        if (relay_letter_count == 4)
        {
          digitalWrite(relay4, HIGH);
        }
        if (relay_letter_count == 5)
        {
          digitalWrite(relay5, HIGH);
        }
        if (relay_letter_count == 6)
        {
          digitalWrite(relay6, HIGH);
        }
        if (relay_letter_count == 7)
        {
          digitalWrite(relay7, HIGH);
        }
        if (relay_letter_count == 8)
        {
          digitalWrite(relay8, HIGH);
        }
      }
    }
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    digitalWrite(relay3, HIGH);
    digitalWrite(relay4, HIGH);
    digitalWrite(relay5, HIGH);
    digitalWrite(relay6, HIGH);
    digitalWrite(relay7, HIGH);
    digitalWrite(relay8, HIGH);
  }
  else
  {
    lcd.setCursor(0, 0);
    for (int i = 0; i < length; i++)
    {
      if (count >= 80)
      {
        lcd.clear();
        count = 0;
        row = 0;
        col = 0;
        lcd.setCursor(0, 0);
      }
      if (count >= 20 && count < 40) {
        row = 1;
        col = count - 20;
        lcd.setCursor(col, row);
      }
      if (count >= 40 && count < 60) {
        row = 2;
        col = count - 40;
        lcd.setCursor(col, row);
      }
      if (count >= 60 && count < 80) {
        row = 3;
        col = count - 60;
        lcd.setCursor(col, row);
      }
      if (text.charAt(i) > 31)
      {
        lcd.print(text.charAt(i));
        col++;
        if (col >= 20)
        {
          col = 0;
          row++;
          if (row >= 4)
          {
            row = 0;
          }
          lcd.setCursor(0, row);
        }
        count++;
        delay(100);
      }
    }
  }
}

void display_sms(String text)
{
  int length = text.length();
  int count = 0;
  int row = 0;
  int col = 0;
  int relay_letter_count = 0;
  lcd.setCursor(0, 0);
  for (int i = 0; i < length; i++)
  {
    if (count >= 80)
    {
      Serial.println("here");
      while (digitalRead(but) == LOW);
      Serial.println("here2");
      lcd.clear();
      count = 0;
      row = 0;
      col = 0;
      lcd.setCursor(0, 0);
    }
    if (count >= 20 && count < 40) {
      row = 1;
      col = count - 20;
      lcd.setCursor(col, row);
    }
    if (count >= 40 && count < 60) {
      row = 2;
      col = count - 40;
      lcd.setCursor(col, row);
    }
    if (count >= 60 && count < 80) {
      row = 3;
      col = count - 60;
      lcd.setCursor(col, row);
    }
    if (text.charAt(i) > 31)
    {
      lcd.print(text.charAt(i));
      col++;
      if (col >= 20)
      {
        col = 0;
        row++;
        if (row >= 4)
        {
          row = 0;
        }
        lcd.setCursor(0, row);
      }
      count++;
      relay_letter_count++;

      if (relay_letter_count > 8)
      {
        relay_letter_count = 1;
      }

      if (relay_letter_count == 1)
      {
        digitalWrite(relay1, LOW);
      }
      else if (relay_letter_count == 2)
      {
        digitalWrite(relay2, LOW);
      }
      else if (relay_letter_count == 3)
      {
        digitalWrite(relay3, LOW);
      }
      else if (relay_letter_count == 4)
      {
        digitalWrite(relay4, LOW);
      }
      else if (relay_letter_count == 5)
      {
        digitalWrite(relay5, LOW);
      }
      else if (relay_letter_count == 6)
      {
        digitalWrite(relay6, LOW);
      }
      else if (relay_letter_count == 7)
      {
        digitalWrite(relay7, LOW);
      }
      else if (relay_letter_count == 8)
      {
        digitalWrite(relay8, LOW);
      }
      delay(250);
      if (relay_letter_count == 1)
      {
        digitalWrite(relay1, HIGH);
      }
      else if (relay_letter_count == 2)
      {
        digitalWrite(relay2, HIGH);
      }
      else if (relay_letter_count == 3)
      {
        digitalWrite(relay3, HIGH);
      }
      else if (relay_letter_count == 4)
      {
        digitalWrite(relay4, HIGH);
      }
      else if (relay_letter_count == 5)
      {
        digitalWrite(relay5, HIGH);
      }
      else if (relay_letter_count == 6)
      {
        digitalWrite(relay6, HIGH);
      }
      else if (relay_letter_count == 7)
      {
        digitalWrite(relay7, HIGH);
      }
      else if (relay_letter_count == 8)
      {
        digitalWrite(relay8, HIGH);
      }
    }
  }
  sms_disp = true;
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
  digitalWrite(relay5, HIGH);
  digitalWrite(relay6, HIGH);
  digitalWrite(relay7, HIGH);
  digitalWrite(relay8, HIGH);
}


void display_file(String text)
{
  int length = text.length();
  int count = 0;
  int row = 0;
  int col = 0;
  lcd.setCursor(0, 0);
  for (int i = 0; i < length; i++)
  {
    if (count >= 80)
    {
      while (digitalRead(but) == LOW);
      lcd.clear();
      count = 0;
      row = 0;
      col = 0;
      lcd.setCursor(0, 0);
    }
    if (count >= 20 && count < 40) {
      row = 1;
      col = count - 20;
      lcd.setCursor(col, row);
    }
    if (count >= 40 && count < 60) {
      row = 2;
      col = count - 40;
      lcd.setCursor(col, row);
    }
    if (count >= 60 && count < 80) {
      row = 3;
      col = count - 60;
      lcd.setCursor(col, row);
    }
    if (text.charAt(i) > 31)
    {
      lcd.print(text.charAt(i));
      col++;
      if (col >= 20)
      {
        col = 0;
        row++;
        if (row >= 4)
        {
          row = 0;
        }
        lcd.setCursor(0, row);
      }
      count++;
      delay(100);
    }
  }

}
