#include <Filters.h>


float U1, U2, U3, volt_sig1, volt_sig2 = 0;
float testFrequency = 60;                     // test signal frequency (Hz)
float windowLength = 40.0 / testFrequency;   // how long to average the signal, for statistist
int Sensor = 0;

float intercept1 = -1.5; // to be adjusted based on calibration testing
float slope1 = 2.654173832210086;; // to be adjusted based on calibration testing
float intercept2 = - 1.5; // to be adjusted based on calibration testing
float slope2 = 3.086092715231788; // to be adjusted based on calibration testing
float intercept3 = -1.7; // to be adjusted based on calibration testing
float slope3 = 2.857142857142857; // to be adjusted based on calibration testing
float intercept4 = -1.69; // to be adjusted based on calibration testing
float slope4 = 1.186011150532184; // to be adjusted based on calibration testing

unsigned long printPeriod = 1000; //Refresh rate
unsigned long previousMillis = 0;
float current_Volts, max_volts, current_Volts2, current_Volts3, current_Volts4, current_Volts5; // Voltage
float vdc1, vdc2, vdc3, vdc4 = 0.0;

RunningStatistics inputStats;       //Easy life lines, actual calculation of the RMS requires a load of coding
RunningStatistics inputStats2;
RunningStatistics inputStats3;
RunningStatistics inputStats4;
RunningStatistics inputStats5;

float R1 = 30000.0;
float R2 = 7500.0;
float dc_off = 1.0;

unsigned long disp_mili = 0;

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);

  Serial1.write(255);
  Serial1.write(255);
  Serial1.write(255);
  Serial1.print("bkcmd=0"); // 0 disables errors, 2 enables errors
  Serial1.write(255);
  Serial1.write(255);
  Serial1.write(255);

  Serial1.write(255);
  Serial1.write(255);
  Serial1.write(255);
  Serial1.print("bkcmd=0"); // 0 disables errors, 2 enables errors
  Serial1.write(255);
  Serial1.write(255);
  Serial1.write(255);


  delay(500);
  Serial2.write(255);
  Serial2.write(255);
  Serial2.write(255);
  Serial2.print("bkcmd=0"); // 0 disables errors, 2 enables errors
  Serial2.write(255);
  Serial2.write(255);
  Serial2.write(255);

  Serial2.write(255);
  Serial2.write(255);
  Serial2.write(255);
  Serial2.print("bkcmd=0"); // 0 disables errors, 2 enables errors
  Serial2.write(255);
  Serial2.write(255);
  Serial2.write(255);


}

void loop()
{
  ac_v1();
  if (U1 < 10)
  {
    U1 = 0;
  }
  if (U2 < 10)
  {
    U2 = 0;
  }
  if (U3 < 10)
  {
    U3 = 0;
  }
  if (volt_sig1 < 10)
  {
    volt_sig1 = 0;
  }
  if (millis() - disp_mili > 1000)
  {
    ac_display_data();
    dc_voltage();
    dc_display_data();
    disp_mili = millis();
  }
}

float ac_v1()
{
  while ( true )
  {
    inputStats.setWindowSecs( windowLength );
    inputStats2.setWindowSecs( windowLength );
    inputStats3.setWindowSecs( windowLength );
    inputStats4.setWindowSecs( windowLength );
    inputStats5.setWindowSecs( windowLength );

    int Sensor = analogRead(A0);  // read the analog in value:
    int Sensor2 = analogRead(A1);
    int Sensor3 = analogRead(A2);
    int Sensor4 = analogRead(A3);
    inputStats.input(Sensor);  // log to Stats function
    inputStats2.input(Sensor2);  // log to Stats function
    inputStats3.input(Sensor3);  // log to Stats function
    inputStats4.input(Sensor4);  // log to Stats function

    if ((unsigned long)(millis() - previousMillis) >= printPeriod)
    {
      previousMillis = millis();   // update time every second

      current_Volts = intercept1 + slope1 * inputStats.sigma(); //Calibartions for offset and amplitude
      // current_Volts = current_Volts * (40.3231);             //Further calibrations for the amplitude
      U1 = current_Volts;
      current_Volts2 = intercept2 + slope2 * inputStats2.sigma(); //Calibartions for offset and amplitude
      //current_Volts2 = current_Volts2 * (40.3231);             //Further calibrations for the amplitude
      U2 = current_Volts2;
      current_Volts3 = intercept3 + slope3 * inputStats3.sigma(); //Calibartions for offset and amplitude
      //current_Volts3 = current_Volts3 * (40.3231);             //Further calibrations for the amplitude
      U3 = current_Volts3;
      current_Volts4 = intercept4 + slope4 * inputStats4.sigma(); //Calibartions for offset and amplitude
      //current_Volts4 = current_Volts4 * (40.3231);             //Further calibrations for the amplitude
      volt_sig1 = current_Volts4;

      // Serial.println(U1);
      //Serial.println(U2);
      //Serial.println(U3);
      //Serial.println(volt_sig1);
      // Serial.println("-------");
      break;
    }
  }
}

void dc_voltage()
{
  float rawValueRead1 = analogRead(A4);
  float vArduino1 = (rawValueRead1 * 5.0) / 1024.0;
  vdc1 = vArduino1 / (R2 / (R1 + R2)) - dc_off;
  float rawValueRead2 = analogRead(A4);
  float vArduino2 = (rawValueRead2 * 5.0) / 1024.0;
  vdc2 = vArduino2 / (R2 / (R1 + R2)) - dc_off;
  float rawValueRead3 = analogRead(A4);
  float vArduino3 = (rawValueRead3 * 5.0) / 1024.0;
  vdc3 = vArduino3 / (R2 / (R1 + R2)) - dc_off;
  float rawValueRead4 = analogRead(A4);
  float vArduino4 = (rawValueRead4 * 5.0) / 1024.0;
  vdc4 = vArduino4 / (R2 / (R1 + R2)) - dc_off;

  // Serial.print("Vdc = ");
  // Serial.println(vdc1);
  // Serial.print("Vdc2 = ");
  // Serial.println(vdc2);
  // Serial.print("Vdc3 = ");
  // Serial.println(vdc3);
  // Serial.print("Vdc4 = ");
  // Serial.println(vdc4);
  //  Serial.println();
}

void ac_display_data()
{
  Serial1.print("page0.t0.txt=");
  Serial1.print("\"");
  Serial1.print(U1);
  Serial1.print("\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("page0.t3.txt=");
  Serial1.print("\"");
  Serial1.print(U2);
  Serial1.print("\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.print("page0.t5.txt=");
  Serial1.print("\"");
  Serial1.print(U3);
  Serial1.print("\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);

  Serial1.print("page0.t7.txt=");
  Serial1.print("\"");
  Serial1.print(volt_sig1);
  Serial1.print("\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
}


void dc_display_data()
{
  Serial2.print("page0.t4.txt=");
  Serial2.print("\"");
  Serial2.print(vdc1);
  Serial2.print("\"");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.print("page0.t2.txt=");
  Serial2.print("\"");
  Serial2.print(vdc2);
  Serial2.print("\"");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.print("page0.t5.txt=");
  Serial2.print("\"");
  Serial2.print(vdc3);
  Serial2.print("\"");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);

  Serial2.print("page0.t7.txt=");
  Serial2.print("\"");
  Serial2.print(vdc4);
  Serial2.print("\"");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);

  if (vdc1 < 12)
  {
    Serial2.print("page0.p0.pic=");
    Serial2.print("0");
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
  }
  else
  {
    Serial2.print("page0.p0.pic=");
    Serial2.print("1");
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
  }

  if (vdc2 < 12)
  {
    Serial2.print("page0.p1.pic=");
    Serial2.print("0");
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
  }
  else
  {
    Serial2.print("page0.p1.pic=");
    Serial2.print("1");
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
  }

  if (vdc3 < 12)
  {
    Serial2.print("page0.p2.pic=");
    Serial2.print("0");
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
  }
  else
  {
    Serial2.print("page0.p2.pic=");
    Serial2.print("1");
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
  }

  if (vdc4 < 12)
  {
    Serial2.print("page0.p3.pic=");
    Serial2.print("0");
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
  }
  else
  {
    Serial2.print("page0.p3.pic=");
    Serial2.print("1");
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
  }


}
