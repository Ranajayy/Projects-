#include <SPI.h>      ///included in stm32 arduino core
#include <Wire.h>     ///included in stm32 arduino core
#include <Adafruit_GFX.h> /// Adafruit GFX library v1.7.5 https://github.com/adafruit/Adafruit-GFX-Library
#define STM32OLED
//#define arduinomega

#ifdef STM32OLED

#include <Adafruit_SSD1306_STM32.h>
#define OLED_RESET PA14
Adafruit_SSD1306 display(OLED_RESET);
#else
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    22 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#endif


#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
///battery parameters
#define MAXMAH 4400
#define BS 3.0

static const unsigned char PROGMEM p0_8[] =
  //{ 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7E, 0x3C};
{ 0x7F, 0xF0,
  0x80, 0x08,
  0x80, 0x0C,
  0x80, 0x0C,
  0x80, 0x0C,
  0x80, 0x0C,
  0x80, 0x08,
  0x7F, 0xF0
};
/*
  static const unsigned char PROGMEM p0_16[] = {
  0x3F, 0xFF, 0xFF, 0x00,
  0x7F, 0xFF, 0xFF, 0x80,
  0xC0, 0x00, 0x01, 0x80,
  0xC0, 0x00, 0x01, 0xF0,
  0xC0, 0x00, 0x01, 0xF0,
  0xC0, 0x00, 0x01, 0xF0,
  0xC0, 0x00, 0x01, 0xF0,
  0xC0, 0x00, 0x01, 0xF0,
  0xC0, 0x00, 0x01, 0xF0,
  0xC0, 0x00, 0x01, 0xF0,
  0xC0, 0x00, 0x01, 0xF0,
  0xC0, 0x00, 0x01, 0xF0,
  0xC0, 0x00, 0x01, 0xF0,
  0xC0, 0x00, 0x01, 0x80,
  0x7F, 0xFF, 0xFF, 0x80,
  0x3F, 0xFF, 0xFF, 0x00
  };
*/
static const unsigned char PROGMEM p10_8[] = { //12.5
  0x7F, 0xF0,
  0x80, 0x08,
  0xB0, 0x0C,
  0xB0, 0x0C,
  0xB0, 0x0C,
  0xB0, 0x0C,
  0x80, 0x08,
  0x7F, 0xF0
};
static const unsigned char PROGMEM p25_8[] = {
  0x7F, 0xF0,
  0x80, 0x08,
  0xB8, 0x0C,
  0xB8, 0x0C,
  0xB8, 0x0C,
  0xB8, 0x0C,
  0x80, 0x08,
  0x7F, 0xF0
};
static const unsigned char PROGMEM p40_8[] = { //37.5
  0x7F, 0xF0,
  0x80, 0x08,
  0xBC, 0x0C,
  0xBC, 0x0C,
  0xBC, 0x0C,
  0xBC, 0x0C,
  0x80, 0x08,
  0x7F, 0xF0
};
/*
  static const unsigned char PROGMEM p25_16[] =
  {
  0x3F, 0xFF, 0xFF, 0x00,
  0x7F, 0xFF, 0xFF, 0x80,
  0xC0, 0x00, 0x01, 0x80,
  0x80, 0x00, 0x01, 0xF0,
  0x8E, 0x00, 0x01, 0xF0,
  0x8E, 0x00, 0x01, 0xF0,
  0x8E, 0x00, 0x01, 0xF0,
  0x8E, 0x00, 0x01, 0xF0,
  0x8E, 0x00, 0x01, 0xF0,
  0x8E, 0x00, 0x01, 0xF0,
  0x8E, 0x00, 0x01, 0xF0,
  0x8E, 0x00, 0x01, 0xF0,
  0x80, 0x00, 0x01, 0xF0,
  0xC0, 0x00, 0x01, 0x80,
  0x7F, 0xFF, 0xFF, 0x80,
  0x3F, 0xFF, 0xFF, 0x00
  };
*/
static const unsigned char PROGMEM p50_8[] = {
  0x7F, 0xF0,
  0x80, 0x08,
  0xBE, 0x0C,
  0xBE, 0x0C,
  0xBE, 0x0C,
  0xBE, 0x0C,
  0x80, 0x08,
  0x7F, 0xF0
};


static const unsigned char PROGMEM p60_8[] = { //62.5
  0x7F, 0xF0,
  0x80, 0x08,
  0xBF, 0x0C,
  0xBF, 0x0C,
  0xBF, 0x0C,
  0xBF, 0x0C,
  0x80, 0x08,
  0x7F, 0xF0
};
static const unsigned char PROGMEM p75_8[] = {
  0x7F, 0xF0,
  0x80, 0x08,
  0xBF, 0x8C,
  0xBF, 0x8C,
  0xBF, 0x8C,
  0xBF, 0x8C,
  0x80, 0x08,
  0x7F, 0xF0
};


/*
  static const unsigned char PROGMEM p75_16[] =  {
  0x3F, 0xFF, 0xFF, 0x00,
  0x7F, 0xFF, 0xFF, 0x80,
  0xC0, 0x00, 0x01, 0x80,
  0x80, 0x00, 0x01, 0xF0,
  0x8E, 0x73, 0x81, 0xF0,
  0x8E, 0x73, 0x81, 0xF0,
  0x8E, 0x73, 0x81, 0xF0,
  0x8E, 0x73, 0x81, 0xF0,
  0x8E, 0x73, 0x81, 0xF0,
  0x8E, 0x73, 0x81, 0xF0,
  0x8E, 0x73, 0x81, 0xF0,
  0x8E, 0x73, 0x81, 0xF0,
  0x80, 0x00, 0x01, 0xF0,
  0xC0, 0x00, 0x01, 0x80,
  0x7F, 0xFF, 0xFF, 0x80,
  0x3F, 0xFF, 0xFF, 0x00
  };
*/
static const unsigned char PROGMEM p90_8[] = { //87.5
  0x7F, 0xF0,
  0x80, 0x08,
  0xBF, 0xCC,
  0xBF, 0xCC,
  0xBF, 0xCC,
  0xBF, 0xCC,
  0x80, 0x08,
  0x7F, 0xF0
};
static const unsigned char PROGMEM p100_8[] = {
  0x7F, 0xF0,
  0x80, 0x08,
  0xBF, 0xEC,
  0xBF, 0xEC,
  0xBF, 0xEC,
  0xBF, 0xEC,
  0x80, 0x08,
  0x7F, 0xF0
};
/*
  static const unsigned char PROGMEM p100_16[] =  {
  0x3F, 0xFF, 0xFF, 0x00,
  0x7F, 0xFF, 0xFF, 0x80,
  0xC0, 0x00, 0x01, 0x80,
  0x80, 0x00, 0x01, 0xF0,
  0x8E, 0x73, 0x9D, 0xF0,
  0x8E, 0x73, 0x9D, 0xF0,
  0x8E, 0x73, 0x9D, 0xF0,
  0x8E, 0x73, 0x9D, 0xF0,
  0x8E, 0x73, 0x9D, 0xF0,
  0x8E, 0x73, 0x9D, 0xF0,
  0x8E, 0x73, 0x9D, 0xF0,
  0x8E, 0x73, 0x9D, 0xF0,
  0x80, 0x00, 0x01, 0xF0,
  0xC0, 0x00, 0x01, 0x80,
  0x7F, 0xFF, 0xFF, 0x80,
  0x3F, 0xFF, 0xFF, 0x00
  };
*/

//#if (SSD1306_LCDHEIGHT != 64)
//#error("Height incorrect, please fix Adafruit_SSD1306.h!");
//#endif

#ifdef arduinomega
///////////////INPUT-OUTPUT
#define ENCH1     21
#define ENDISC1    20
#define ENCH2     19
#define ENDISC2   18
#define ENCH3     17
#define ENDISC3   16
#define ENCH4     15
#define ENDISC4   14
#define ENCH5     13
#define ENDISC5   12
#define ENCH6     11
#define ENDISC6   10
#define ENCH7     9
#define ENDISC7   8
#define ENCH8     7
#define ENDISC8   6
#define SELECT0   5
#define SELECT1   4
#define SELECT2   3
#define BUTTON    2
#define ADC1      A0
#define ADC2      A1
#else
///////////////INPUT-OUTPUT
#define ENCH1     PB0
#define ENDISC1   PB1
#define ENCH2     PB8
#define ENDISC2   PB9
#define ENCH3     PB10
#define ENDISC3   PB11
#define ENCH4     PB12
#define ENDISC4   PB13
#define ENCH5     PB14
#define ENDISC5   PB15
#define ENCH6     PA3
#define ENDISC6   PA4
#define ENCH7     PA5
#define ENDISC7   PA11
#define ENCH8     PA12
#define ENDISC8   PA13
#define SELECT0   PA6
#define SELECT1   PA7
#define SELECT2   PA8
#define BUTTON    PA0
#define ADC1      PA1
#define ADC2      PA2
#endif
///////////////

int line_size = 9, text_size = 5, batt_length = 30;
int v[8], SOC[8], DOD[8], tmp[8], prev_temp[8], soc[8] , batt_act = 0;
float prev_soc = 0;
int batt_index = 0;
int shunt[8];

float c[8];

float vb[8];
float Q[8];
int ch[8];

unsigned long previousMillis = 0;
unsigned long millisPassed = 0;

/*initial_SOC()*/

float initial_SOC(float OCV)

{ float INI_SOC = 0;
  OCV = OCV / 3.0;
  Serial.print("OCV:");
  Serial.println(OCV);
  if (OCV < (3.339)) //0
  {
    INI_SOC = 0;
  }
  if (((3.339) <= OCV) && (OCV <= (3.47892))) //0,05 to 3,7653 %
  {
    INI_SOC = 26.55 * (OCV) - 88.6;
    Serial.println("here1");
  }
  if (((3.47892) < OCV ) && (OCV <= (3.54166))) // 3,7765 to  11.6075
  {
    INI_SOC = 125.0 * (OCV) - 431.1;
    Serial.println("here2");
  }
  if (((3.54166) < OCV) && (OCV <= (3.63538))) //11.60734 to 25.57162
  {
    INI_SOC = 149.0 * (OCV) - 516.1;
    Serial.println("here3");
  }
  if (((3.63538) < OCV ) && (OCV <= (3.70393))) //25,57072 to 49.15192
  {
    INI_SOC = 344.0 * (OCV) - 1225.0;
    Serial.println("here4");
  }
  if (((3.70393) < OCV ) && (OCV <= (3.75))) //49.1519352 to 59.725
  {
    INI_SOC = 229.5 * (OCV) - 800.9;
    Serial.println("here5");
  }
  if (((3.75) < OCV ) && (OCV <= (3.929577)) ) //59.7251119 to 79819663
  {
    INI_SOC = 111.9 * (OCV) - 359.9;
    Serial.println("here6");
  }
  if (((3.929577) < OCV ) && (OCV <= (4.03805))) //79.8196696 to 91.18764
  {
    INI_SOC = 104.8 * (OCV) - 332.0;
    Serial.println("here7");
  }
  if ( ((4.03805) <= OCV) && (OCV < (4.132)) ) //91.1877105 to
  {
    INI_SOC = 90.61 * (OCV) - 274.7;
    Serial.println("here8");
  }
  if (OCV > (4.132))
  {
    INI_SOC = 100.0;
  }
  Serial.print("INI_SOC:");
  Serial.println(INI_SOC);
  return INI_SOC;
}
/*CURRENT_SOC();*/
void CURRENT_SOC()
{ int i = 0;
  for (i = 0; i < 8; i++)
  { SOC[i] = initial_SOC(vb[i]);
    Serial.print("SOC");
    Serial.print(i + 1);
    Serial.print(":");
    Serial.println(SOC[i]);
  }
}
/*measuring()
  Select adc channel in multiplexer
  read values and calculate SOC
*/
void measuring(bool h)
{ int i = 0;
  for (i = 0; i < 8; i++)
  {
    switch (i)
    {
      case 0:
        digitalWrite(SELECT0, 0);   // BATTERY 1
        digitalWrite(SELECT1, 0);
        digitalWrite(SELECT2, 0);
        break;
      case 1:
        digitalWrite(SELECT0, 1);
        digitalWrite(SELECT1, 0);
        digitalWrite(SELECT2, 0);
        break;
      case 2:
        digitalWrite(SELECT0, 0);
        digitalWrite(SELECT1, 1);
        digitalWrite(SELECT2, 0);
        break;
      case 3:
        digitalWrite(SELECT0, 1);
        digitalWrite(SELECT1, 1);
        digitalWrite(SELECT2, 0);
        break;
      case 4:
        digitalWrite(SELECT0, 0);
        digitalWrite(SELECT1, 0);
        digitalWrite(SELECT2, 1);
        break;
      case 5:
        digitalWrite(SELECT0, 1);
        digitalWrite(SELECT1, 0);
        digitalWrite(SELECT2, 1);
        break;
      case 6:
        digitalWrite(SELECT0, 0);
        digitalWrite(SELECT1, 1);
        digitalWrite(SELECT2, 1);
        break;
      case 7:
        digitalWrite(SELECT0, 1);
        digitalWrite(SELECT1, 1);
        digitalWrite(SELECT2, 1);
        break;
    }
    v[i] = 0;
    c[i] = 0;
    shunt[i] = 0;
    vb[i] = 0;
    shunt[i] = analogRead(ADC1);
    Serial.println(shunt[i]);
    delay(5);
    v[i] = analogRead(ADC2);
    Serial.println(v[i]);
#ifdef arduinomega
    c[i] = (((((shunt[i] * 3.3 / 1023) / 3.3) * 0.3) - 0.15) / 0.01) - 0.72;
    //c[i]=c[i]*-1; //current sense corection
    vb[i] = (v[i] * 3.3 / 1023) * 15 / 3;
#else
    c[i] = ((((shunt[i] * 3.3 / 4095) / 3.3) * 0.3) - 0.15) / 0.01;
    //c[i]=c[i]*-1;//current sense corection
    vb[i] = (v[i] * 3.3 / 4095) * 15 / 3;
#endif
    delay(5);
    if (h == 1)
    {
      millisPassed = millis() - previousMillis;
      Q[i] = Q[i] + (c[i] * 1000.0) * (millisPassed / 3600000.0); //Q in mAh mus be reset at begining charge or discarge cicle
      previousMillis = millis();

      if (c[i] > 0)
      { ch[i] = 0; //charging mode

        SOC[i] = SOC[i] + (((Q[i]) / MAXMAH) * 100);
        Q[i] = 0;
      }
      else if (c[i] < 0) //
      { ch[i] = 1; //discharging mode
        DOD[i] = 100 - SOC[i];
        DOD[i] = DOD[i] + (((Q[i] * -1) / MAXMAH) * 100);
        SOC[i] = 100 - DOD[i];
        Q[i] = 0;
      }
      else if (c[i] == 0)
      { ch[i] = 2;
        Q[i] = 0;
      }
    }
  }


}
/* void batt_enable(int BATT)
  Enable batteries for discharge and disable discharge

*/
void batt_enable(int BATT)
{ switch (BATT)
  { case 1:
      {
         disp_marker(1);
        digitalWrite(ENCH1, 0);
        digitalWrite(ENDISC1, 1);
        batt_act = 0;
        break;
      }
    case 2:
      {
         disp_marker(2);
        digitalWrite(ENCH2, 0);
        digitalWrite(ENDISC2, 1);
        batt_act = 1;
        break;
      }
    case 3:
      {
        disp_marker(3);
        digitalWrite(ENCH3, 0);
        digitalWrite(ENDISC3, 1);
        batt_act = 2;
        break;
      }
    case 4:
      {
        disp_marker(4);
        digitalWrite(ENCH4, 0);
        digitalWrite(ENDISC4, 1);
        batt_act = 3;
        break;
      }
    case 5:
      {
        disp_marker(5);
        digitalWrite(ENCH5, 0);
        digitalWrite(ENDISC5, 1);
        batt_act = 4;
        break;
      }
    case 6:
      {
        disp_marker(6);
        digitalWrite(ENCH6, 0);
        digitalWrite(ENDISC6, 1);
        batt_act = 5;
        break;
      }
    case 7:
      {
        disp_marker(7);
        digitalWrite(ENCH7, 0);
        digitalWrite(ENDISC7, 1);
        batt_act = 6;
        break;
      }
    case 8:
      {
        disp_marker(8);
        digitalWrite(ENCH8, 0);
        digitalWrite(ENDISC8, 1);
        batt_act = 7;
        break;
      }
  }

}

/*void output_enable()
  Enable output if batteries are charged

*/




void output_enable()
{ switch (batt_act)
  { case 0:
      {
       // disp_marker(1);
        
        if (vb[0] > 10.2)
        { batt_enable(1);

          break;
        }
        else if (vb[1] > 10.2)
        {
          batt_enable(2);
          break;
        }
        else if (vb[2] > 10.2)
        {
          batt_enable(3);
          break;
        }
        else if (vb[3] > 10.2)
        {
          batt_enable(4);
          break;
        }
        else if (vb[4] > 10.2)
        {
          batt_enable(5);
          break;
        }
        else if (vb[5] > 10.2)
        {
          batt_enable(6);
          break;
        }
        else if (vb[6] > 10.2)
        {
          batt_enable(7);
          break;
        }
        else if (vb[7] > 10.2)
        {
          batt_enable(8);
          break;
        }
        else
        { batt_act = 0;
          break;
        }
      }
    case 1:
      { 
       
        if (vb[1] > 10.2)
        {
          batt_enable(2);
          break;
        }
        else if (vb[2] > 10.2)
        {
          batt_enable(3);
          break;
        }
        else if (vb[3] > 10.2)
        {
          batt_enable(4);
          break;
        }
        else if (vb[4] > 10.2)
        {
          batt_enable(5);
          break;
        }
        else if (vb[5] > 10.2)
        {
          batt_enable(6);
          break;
        }
        else if (vb[6] > 10.2)
        {
          batt_enable(7);
          break;
        }
        else if (vb[7] > 10.2)
        {
          batt_enable(8);
          break;
        }
        else if (vb[0] > 10.2)
        { batt_enable(1);
          break;
        }
        else
        { batt_act = 0;
          break;
        }
      }
    case 2:
      { 
       // disp_marker(2);
        if (vb[2] > 10.2)
        {
          batt_enable(3);
          break;
        }
        else if (vb[3] > 10.2)
        {
          batt_enable(4);
          break;
        }
        else if (vb[4] > 10.2)
        {
          batt_enable(5);
          break;
        }
        else if (vb[5] > 10.2)
        {
          batt_enable(6);
          break;
        }
        else if (vb[6] > 10.2)
        {
          batt_enable(7);
          break;
        }
        else if (vb[7] > 10.2)
        {
          batt_enable(8);
          break;
        }
        else if (vb[0] > 10.2)
        { batt_enable(1);
          break;
        }
        else if (vb[1] > 10.2)
        {
          batt_enable(2);
          break;
        }
        else
        { batt_act = 0;
          break;
        }
      }
    case 3:
      {
      //  disp_marker(3);
        if (vb[3] > 10.2)
        {
          batt_enable(4);
          break;
        }
        else if (vb[4] > 10.2)
        {
          batt_enable(5);
          break;
        }
        else if (vb[5] > 10.2)
        {
          batt_enable(6);
          break;
        }
        else if (vb[6] > 10.2)
        {
          batt_enable(7);
          break;
        }
        else if (vb[7] > 10.2)
        {
          batt_enable(8);
          break;
        }
        else if (vb[0] > 10.2)
        { batt_enable(1);
          break;
        }
        else if (vb[1] > 10.2)
        {
          batt_enable(2);
          break;
        }
        else if (vb[2] > 10.2)
        {
          batt_enable(3);
          break;
        }

        else
        { batt_act = 0;
          break;
        }
      }
    case 4:
      {
      //  disp_marker(4);
        
        if (vb[4] > 10.2)
        {
          batt_enable(5);
          break;
        }
        else if (vb[5] > 10.2)
        {
          batt_enable(6);
          break;
        }
        else if (vb[6] > 10.2)
        {
          batt_enable(7);
          break;
        }
        else if (vb[7] > 10.2)
        {
          batt_enable(8);
          break;
        }
        else if (vb[0] > 10.2)
        { batt_enable(1);
          break;
        }
        else if (vb[1] > 10.2)
        {
          batt_enable(2);
          break;
        }
        else if (vb[2] > 10.2)
        {
          batt_enable(3);
          break;
        }
        else if (vb[3] > 10.2)
        {
          batt_enable(4);
          break;
        }
        else
        { batt_act = 0;
          break;
        }
      }
    case 5:
      {
       // disp_marker(5);
        if (vb[5] > 10.2)
        {
          batt_enable(6);
          break;
        }
        else if (vb[6] > 10.2)
        {
          batt_enable(7);
          break;
        }
        else if (vb[7] > 10.2)
        {
          batt_enable(8);
          break;
        }
        else if (vb[0] > 10.2)
        { batt_enable(1);
          break;
        }
        else if (vb[1] > 10.2)
        {
          batt_enable(2);
          break;
        }
        else if (vb[2] > 10.2)
        {
          batt_enable(3);
          break;
        }
        else if (vb[3] > 10.2)
        {
          batt_enable(4);
          break;
        }
        else if (vb[4] > 10.2)
        {
          batt_enable(5);
          break;
        }
        else
        { batt_act = 0;
          break;
        }
      }
    case 6:
      { 
        
       // disp_marker(6);
        if (vb[6] > 10.2)
        {
          batt_enable(7);
          break;
        }
        else if (vb[7] > 10.2)
        {
          batt_enable(8);
          break;
        }
        else if (vb[0] > 10.2)
        { batt_enable(1);
          break;
        }
        else if (vb[1] > 10.2)
        {
          batt_enable(2);
          break;
        }
        else if (vb[2] > 10.2)
        {
          batt_enable(3);
          break;
        }
        else if (vb[3] > 10.2)
        {
          batt_enable(4);
          break;
        }
        else if (vb[4] > 10.2)
        {
          batt_enable(5);
          break;
        }
        else if (vb[5] > 10.2)
        {
          batt_enable(6);
          break;
        }
        else
        { batt_act = 0;
          break;
        }
      }
    case 7:
      { 
        
       // disp_marker(7);
        
        if (vb[7] > 10.2)
        {
          batt_enable(8);
          break;
        }
        else if (vb[0] > 10.2)
        { batt_enable(1);
          break;
        }
        else if (vb[1] > 10.2)
        {
          batt_enable(2);
          break;
        }
        else if (vb[2] > 10.2)
        {
          batt_enable(3);
          break;
        }
        else if (vb[3] > 10.2)
        {
          batt_enable(4);
          break;
        }
        else if (vb[4] > 10.2)
        {
          batt_enable(5);
          break;
        }
        else if (vb[5] > 10.2)
        {
          batt_enable(6);
          break;
        }
        else if (vb[6] > 10.2)
        {
          batt_enable(7);
          break;
        }
        else
        { batt_act = 0;
          break;
        }
      }
    case 8:
      { 
       // disp_marker(8);
        if (vb[0] > 10.2)
        { batt_enable(1);
          break;
        }
        else if (vb[1] > 10.2)
        {
          batt_enable(2);
          break;
        }
        else if (vb[2] > 10.2)
        {
          batt_enable(3);
          break;
        }
        else if (vb[3] > 10.2)
        {
          batt_enable(4);
          break;
        }
        else if (vb[4] > 10.2)
        {
          batt_enable(5);
          break;
        }
        else if (vb[5] > 10.2)
        {
          batt_enable(6);
          break;
        }
        else if (vb[6] > 10.2)
        {
          batt_enable(7);
          break;
        }
        else if (vb[7] > 10.2)
        {
          batt_enable(8);
          break;
        }
        else
        { batt_act = 0;
          break;
        }
      }
  }


}
/*
  charge_disable()
  disable charge in batteries if 12.55v is reached
*/


void charge_disable()
{ if (vb[0] > 12.55)
  { digitalWrite(ENCH1, 0);
  }
  if (vb[1] > 12.55)
  { digitalWrite(ENCH2, 0);
  }
  if (vb[2] > 12.55)
  { digitalWrite(ENCH3, 0);
  }
  if (vb[3] > 12.55)
  { digitalWrite(ENCH4, 0);
  }
  if (vb[4] > 12.55)
  { digitalWrite(ENCH5, 0);
  }
  if (vb[5] > 12.55)
  { digitalWrite(ENCH6, 0);
  }
  if (vb[6] > 12.55)
  { digitalWrite(ENCH7, 0);
  }
  if (vb[7] > 12.55)
  { digitalWrite(ENCH8, 0);
  }
}
/*
  void charge_enable()
  enable charge in batteries if below 12.45v

*/
void charge_enable()
{ if ((vb[0] < 12.45) && batt_act != 1)
  { digitalWrite(ENCH1, 1);
  }
  if ((vb[1] < 12.45) && batt_act != 2)
  { digitalWrite(ENCH2, 1);
  }
  if ((vb[2] < 12.45) && batt_act != 3)
  { digitalWrite(ENCH3, 1);
  }
  if ((vb[3] < 12.45) && batt_act != 4)
  { digitalWrite(ENCH4, 1);
  }
  if ((vb[4] < 12.45) && batt_act != 5)
  { digitalWrite(ENCH5, 1);
  }
  if ((vb[5] < 12.45) && batt_act != 6)
  { digitalWrite(ENCH6, 1);
  }
  if ((vb[6] < 12.45) && batt_act != 7)
  { digitalWrite(ENCH7, 1);
  }
  if ((vb[7] < 12.45) && batt_act != 8)
  { digitalWrite(ENCH8, 1);
  }
}

/*
  void control_switch()
  control the cycle, battery in use, batteries charging.
*/

void control_switch()
{
  charge_disable();
  switch (batt_act)
  { case 0:
      { output_enable();
        break;
      }
    case 1:
      { if ((vb[0] < 9.65) || (SOC[0] < 6))
        {
          output_enable();
          digitalWrite(ENDISC1, 0);
          digitalWrite(ENCH1, 1);
          break;
        }
      }
    case 2:
      { if ((vb[1] < 9.65) || (SOC[1] < 6))
        {
          output_enable();
          digitalWrite(ENDISC2, 0);
          digitalWrite(ENCH2, 1);
          break;
        }
      }
    case 3:
      { if ((vb[2] < 9.65) || (SOC[2] < 6))
        {
          output_enable();
          digitalWrite(ENDISC3, 0);
          digitalWrite(ENCH3, 1);
          break;
        }
      }
    case 4:
      { if ((vb[3] < 9.65) || (SOC[3] < 6))
        {
          output_enable();
          digitalWrite(ENDISC4, 0);
          digitalWrite(ENCH4, 1);
          break;
        }
      }
    case 5:
      { if ((vb[4] < 9.65) || (SOC[4] < 6))
        {
          output_enable();
          digitalWrite(ENDISC5, 0);
          digitalWrite(ENCH5, 1);
          break;
        }
      }
    case 6:
      { if ((vb[5] < 9.65) || (SOC[5] < 6))
        {
          output_enable();
          digitalWrite(ENDISC6, 0);
          digitalWrite(ENCH6, 1);
          break;
        }
      }
    case 7:
      { if ((vb[6] < 9.65) || (SOC[6] < 6))
        {
          output_enable();
          digitalWrite(ENDISC7, 0);
          digitalWrite(ENCH7, 1);
          break;
        }
      }
    case 8:
      { if ((vb[7] < 9.65) || (SOC[7] < 6))
        {
          output_enable();
          digitalWrite(ENDISC8, 0);
          digitalWrite(ENCH8, 1);
          break;
        }
      }

  }
  charge_enable();
}




//////////////

void setup()   {
  Serial.begin(115200);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64) c
  // init done

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);

  // Clear the buffer.

  display.clearDisplay();

  ////INPUT-OUTPUT SETUP
  pinMode(ENCH1, OUTPUT);
  pinMode(ENDISC1, OUTPUT);
  pinMode(ENCH2, OUTPUT);
  pinMode(ENDISC2, OUTPUT);
  pinMode(ENCH3, OUTPUT);
  pinMode(ENDISC3, OUTPUT);
  pinMode(ENCH4, OUTPUT);
  pinMode(ENDISC4, OUTPUT);
  pinMode(ENCH5, OUTPUT);
  pinMode(ENDISC5, OUTPUT);
  pinMode(ENCH6, OUTPUT);
  pinMode(ENDISC6, OUTPUT);
  pinMode(ENCH7, OUTPUT);
  pinMode(ENDISC7, OUTPUT);
  pinMode(ENCH8, OUTPUT);
  pinMode(ENDISC8, OUTPUT);
  pinMode(SELECT0, OUTPUT);
  pinMode(SELECT1, OUTPUT);
  pinMode(SELECT2, OUTPUT);
  pinMode(BUTTON, INPUT);
#ifndef arduinomega
  pinMode(ADC1, INPUT_ANALOG);
  pinMode(ADC2, INPUT_ANALOG);
#endif
  digitalWrite(ENDISC1, 0);
  digitalWrite(ENCH1, 0);
  digitalWrite(ENDISC2, 0);
  digitalWrite(ENCH2, 0);
  digitalWrite(ENDISC3, 0);
  digitalWrite(ENCH3, 0);
  digitalWrite(ENDISC4, 0);
  digitalWrite(ENCH4, 0);
  digitalWrite(ENDISC5, 0);
  digitalWrite(ENCH5, 0);
  digitalWrite(ENDISC6, 0);
  digitalWrite(ENCH6, 0);
  digitalWrite(ENDISC7, 0);
  digitalWrite(ENCH7, 0);
  digitalWrite(ENDISC8, 0);
  digitalWrite(ENCH8, 0);
  digitalWrite(SELECT0, 0);
  digitalWrite(SELECT1, 0);
  digitalWrite(SELECT2, 0);
  ///////////////////////////
  measuring(0);
  CURRENT_SOC();//estimate SOC by OCV

}
/*
  void display_batt_SOC(float SOC, int x, int y)
  get icon of SOC
*/

void display_batt_SOC(float SOC, int x, int y)
{

  if (SOC < 12.5)
  {
    display.drawBitmap(x, y, p0_8, 16, 8, 1); //0,16
    display.display();

  }

  else if (SOC < 25)
  {
    display.drawBitmap(x, y, p10_8, 16, 8, 1); //34, 16
    display.display();
  }

  else if (SOC < 37.5)
  {
    display.drawBitmap(x, y, p25_8, 16, 8, 1); //34, 16
    display.display();
  }

  else if (SOC < 50)
  {
    display.drawBitmap(x, y, p40_8, 16, 8, 1); //34, 16
    display.display();
  }

  else if (SOC < 62.5)
  {
    display.drawBitmap(x, y, p50_8, 16, 8, 1);
    display.display();
  }

  else if (SOC < 75)
  {
    display.drawBitmap(x, y, p60_8, 16, 8, 1);
    display.display();
  }

  else if (SOC < 87.5)
  {
    display.drawBitmap(x, y, p75_8, 16, 8, 1); //97,16
    display.display();
  }

  else if (SOC < 95)
  {
    display.drawBitmap(x, y, p90_8, 16, 8, 1); //97,16
    display.display();
  }

  else if ((95 <= SOC) && (SOC <= 100))
  {
    display.drawBitmap(x, y, p100_8, 16, 8, 1); //97, 100
    display.display();
  }

}

int batt_index_check(float SOC)
{
  int a = 0;

  if (SOC < 12.5)
  {

    a = 1;
  }

  else if (SOC < 25)
  {

    a = 2;
  }

  else if (SOC < 37.5)
  {
    a = 3;
  }

  else if (SOC < 50)
  {
    a = 4;
  }

  else if (SOC < 62.5)
  {
    a = 5;
  }

  else if (SOC < 75)
  {
    a = 6;
  }

  else if (SOC < 87.5)
  {
    a = 7;
  }

  else if (SOC < 95)
  {
    a = 8;
  }

  else if ((95 <= SOC) && (SOC <= 100))
  {
    a = 9;
  }

  return a;

}

/*void OLED_screens_8()
  organize SOC icons in oled display 8bits

*/
void OLED_screens_8()
{

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("1:");
  display.display();
  tmp[0] = batt_index_check(SOC[0]);

  if (prev_temp[0] != tmp[0])
  {
    prev_temp[0] = tmp[0];
    display.fillRect((2 * text_size + 1), 0, 10, 10, BLACK);
    display.display();

  }


  display_batt_SOC(SOC[0], (2 * text_size + 1), 0);



  if (soc[0] != SOC[0])
  {
    soc[0] = SOC[0];
    display.fillRect(0, line_size, 50, 10, BLACK);
    display.display();

  }

  display.setCursor(0, line_size);
  display.print(SOC[0]);
  display.display();
  display.print("%");
  display.display();




  //////////////////////////////////

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(batt_length + 1, 0);
  display.println("2:");
  display.display();
  tmp[1] = batt_index_check(SOC[1]);

  if (prev_temp[1] != tmp[1])
  {
    prev_temp[1] = tmp[1];
    display.fillRect((batt_length + 1) + (2 * text_size + 1), 0, 10, 10, BLACK);
    display.display();

  }

  display_batt_SOC(SOC[1], (batt_length + 1) + (2 * text_size + 1), 0);



  if (soc[1] != SOC[1])
  {
    soc[1] = SOC[1];

    display.fillRect(batt_length + 1, line_size, 50, 10, BLACK);
    display.display();

  }


  display.setCursor(batt_length + 1, line_size);
  display.print(SOC[1]);
  display.display();
  display.print("%");
  display.display();






  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor((batt_length + 1) * 2, 0);
  display.println("3:");
  display.display();


  tmp[2] = batt_index_check(SOC[2]);

  if (prev_temp[2] != tmp[2])
  {
    prev_temp[2] = tmp[2];
    display.fillRect(((batt_length + 1) * 2) + (2 * text_size + 1), 0, 10, 10, BLACK);
    display.display();

  }

  display_batt_SOC(SOC[2], ((batt_length + 1) * 2) + (2 * text_size + 1), 0);

  if (soc[2] != SOC[2])
  {
    soc[2] = SOC[2];

    display.fillRect((batt_length + 1) * 2, line_size, 50, 10, BLACK);
    display.display();

  }
  display.setCursor((batt_length + 1) * 2, line_size);
  display.print(SOC[2]);
  display.display();
  display.print("%");
  display.display();




  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor((batt_length + 1) * 3, 0);
  display.println("4:");
  display.display();
  tmp[3] = batt_index_check(SOC[3]);

  if (prev_temp[3] != tmp[3])
  {
    prev_temp[3] = tmp[3];
    display.fillRect(((batt_length + 1) * 3) + (2 * text_size + 1), 0, 10, 10, BLACK);
    display.display();

  }

  display_batt_SOC(SOC[3], ((batt_length + 1) * 3) + (2 * text_size + 1), 0);


  if (soc[3] != SOC[3])
  {
    soc[3] = SOC[3];
    display.fillRect((batt_length + 1) * 3, line_size, 50, 10, BLACK);
    display.display();

  }

  display.setCursor((batt_length + 1) * 3, line_size);
  display.print(SOC[3]);
  display.display();
  display.print("%");
  display.display();


  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, (line_size) * 4);
  display.println("5:");
  display.display();

  tmp[4] = batt_index_check(SOC[4]);

  if (prev_temp[4] != tmp[4])
  {
    prev_temp[4] = tmp[4];
    display.fillRect((2 * text_size + 1), (line_size) * 4, 10, 10, BLACK);
    display.display();

  }

  display_batt_SOC(SOC[4], (2 * text_size + 1), (line_size) * 4);


  if (soc[4] != SOC[4])
  {
    soc[4] = SOC[4];

    display.fillRect(0, line_size * 5, 50, 10, BLACK);
    display.display();

  }
  display.setCursor(0, line_size * 5);
  display.print(SOC[4]);
  display.display();
  display.print("%");
  display.display();


  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor((batt_length + 1), (line_size) * 4);
  display.println("6:");
  display.display();

  tmp[5] = batt_index_check(SOC[5]);

  if (prev_temp[5] != tmp[5])
  {
    prev_temp[5] = tmp[5];
    display.fillRect((batt_length + 1) + (2 * text_size + 1), (line_size) * 4, 10, 10, BLACK);
    display.display();

  }
  display_batt_SOC(SOC[5], (batt_length + 1) + (2 * text_size + 1), (line_size) * 4);


  if (soc[5] != SOC[5])
  {
    soc[5] = SOC[5];

    display.fillRect((batt_length + 1), line_size * 5, 50, 10, BLACK);
    display.display();

  }

  display.setCursor((batt_length + 1), line_size * 5);
  display.print(SOC[5]);
  display.display();
  display.print("%");
  display.display();



  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor((batt_length + 1) * 2, (line_size) * 4);
  display.println("7:");
  display.display();

  tmp[6] = batt_index_check(SOC[6]);

  if (prev_temp[6] != tmp[6])
  {
    prev_temp[6] = tmp[6];
    display.fillRect(((batt_length + 1) * 2) + (2 * text_size + 1), (line_size) * 4, 10, 10, BLACK);
    display.display();

  }

  display_batt_SOC(SOC[6], ((batt_length + 1) * 2) + (2 * text_size + 1), (line_size) * 4);

  if (soc[6] != SOC[6])
  {
    soc[6] = SOC[6];

    display.fillRect((batt_length + 1) * 2, line_size * 5, 50, 10, BLACK);
    display.display();

  }
  display.setCursor((batt_length + 1) * 2, line_size * 5);
  display.print(SOC[6]);
  display.display();
  display.print("%");
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor((batt_length + 1) * 3, (line_size) * 4);
  display.println("8:");
  display.display();

  tmp[7] = batt_index_check(SOC[7]);

  if (prev_temp[7] != tmp[7])
  {
    prev_temp[7] = tmp[7];
    display.fillRect(((batt_length + 1) * 3) + (2 * text_size + 1), (line_size) * 4, 10, 10, BLACK);
    display.display();

  }

  display_batt_SOC(SOC[7], ((batt_length + 1) * 3) + (2 * text_size + 1), (line_size) * 4);



  if (soc[7] != SOC[7])
  {
    soc[7] = SOC[7];

    display.fillRect((batt_length + 1) * 3, line_size * 5, 50, 10, BLACK);
    display.display();

  }
  display.setCursor((batt_length + 1) * 3, line_size * 5);
  display.print(SOC[7]);
  display.display();
  display.print("%");
  display.display();

}


void disp_marker(int bat )
{

  if (bat == 1)
  {
    uint16_t   color = BLACK;

    display.fillTriangle(2 * text_size + 1, line_size + 10 , 5, line_size + 13 , 15, line_size + 13 , WHITE);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), line_size + 10 , (batt_length + 1) + (2 * text_size + 1) - 5, line_size + 13 , (batt_length + 1) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(2 * text_size + 1, ( line_size * 4) - 3 , 5, (line_size * 4) - 6, 15, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), ( line_size * 4) - 3 , (batt_length + 1) + (2 * text_size + 1) - 5, (line_size * 4) - 6, (batt_length + 1) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

  }

  if (bat == 2)
  {
    uint16_t   color = BLACK;

    display.fillTriangle(2 * text_size + 1, line_size + 10 , 5, line_size + 13 , 15, line_size + 13 , color);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), line_size + 10 , (batt_length + 1) + (2 * text_size + 1) - 5, line_size + 13 , (batt_length + 1) + (2 * text_size + 1) + 5, line_size + 13 , WHITE);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(2 * text_size + 1, ( line_size * 4) - 3 , 5, (line_size * 4) - 6, 15, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), ( line_size * 4) - 3 , (batt_length + 1) + (2 * text_size + 1) - 5, (line_size * 4) - 6, (batt_length + 1) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

  }

  if (bat == 3)
  {
    uint16_t   color = BLACK;

    display.fillTriangle(2 * text_size + 1, line_size + 10 , 5, line_size + 13 , 15, line_size + 13 , color);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), line_size + 10 , (batt_length + 1) + (2 * text_size + 1) - 5, line_size + 13 , (batt_length + 1) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, line_size + 13 , WHITE);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(2 * text_size + 1, ( line_size * 4) - 3 , 5, (line_size * 4) - 6, 15, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), ( line_size * 4) - 3 , (batt_length + 1) + (2 * text_size + 1) - 5, (line_size * 4) - 6, (batt_length + 1) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

  }

  if (bat == 4)
  {
    uint16_t   color = BLACK;

    display.fillTriangle(2 * text_size + 1, line_size + 10 , 5, line_size + 13 , 15, line_size + 13 , color);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), line_size + 10 , (batt_length + 1) + (2 * text_size + 1) - 5, line_size + 13 , (batt_length + 1) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, line_size + 13 , WHITE);
    display.display();

    display.fillTriangle(2 * text_size + 1, ( line_size * 4) - 3 , 5, (line_size * 4) - 6, 15, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), ( line_size * 4) - 3 , (batt_length + 1) + (2 * text_size + 1) - 5, (line_size * 4) - 6, (batt_length + 1) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

  }

  if (bat == 5)
  {
    uint16_t   color = BLACK;

    display.fillTriangle(2 * text_size + 1, line_size + 10 , 5, line_size + 13 , 15, line_size + 13 , color);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), line_size + 10 , (batt_length + 1) + (2 * text_size + 1) - 5, line_size + 13 , (batt_length + 1) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(2 * text_size + 1, ( line_size * 4) - 3 , 5, (line_size * 4) - 6, 15, (line_size * 4) - 6 , WHITE);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), ( line_size * 4) - 3 , (batt_length + 1) + (2 * text_size + 1) - 5, (line_size * 4) - 6, (batt_length + 1) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

  }

  if (bat == 6)
  {
    uint16_t   color = BLACK;

    display.fillTriangle(2 * text_size + 1, line_size + 10 , 5, line_size + 13 , 15, line_size + 13 , color);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), line_size + 10 , (batt_length + 1) + (2 * text_size + 1) - 5, line_size + 13 , (batt_length + 1) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(2 * text_size + 1, ( line_size * 4) - 3 , 5, (line_size * 4) - 6, 15, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), ( line_size * 4) - 3 , (batt_length + 1) + (2 * text_size + 1) - 5, (line_size * 4) - 6, (batt_length + 1) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , WHITE);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

  }

  if (bat == 7)
  {
    uint16_t   color = BLACK;

    display.fillTriangle(2 * text_size + 1, line_size + 10 , 5, line_size + 13 , 15, line_size + 13 , color);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), line_size + 10 , (batt_length + 1) + (2 * text_size + 1) - 5, line_size + 13 , (batt_length + 1) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(2 * text_size + 1, ( line_size * 4) - 3 , 5, (line_size * 4) - 6, 15, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), ( line_size * 4) - 3 , (batt_length + 1) + (2 * text_size + 1) - 5, (line_size * 4) - 6, (batt_length + 1) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , WHITE);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

  }

  if (bat == 8)
  {
    uint16_t   color = BLACK;

    display.fillTriangle(2 * text_size + 1, line_size + 10 , 5, line_size + 13 , 15, line_size + 13 , color);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), line_size + 10 , (batt_length + 1) + (2 * text_size + 1) - 5, line_size + 13 , (batt_length + 1) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), line_size + 10 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, line_size + 13 , ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, line_size + 13 , color);
    display.display();

    display.fillTriangle(2 * text_size + 1, ( line_size * 4) - 3 , 5, (line_size * 4) - 6, 15, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle((batt_length + 1) + (2 * text_size + 1), ( line_size * 4) - 3 , (batt_length + 1) + (2 * text_size + 1) - 5, (line_size * 4) - 6, (batt_length + 1) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 2) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 2) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 2) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , color);
    display.display();

    display.fillTriangle(((batt_length + 1) * 3) + (2 * text_size + 1), ( line_size * 4) - 3 , ((batt_length + 1) * 3) + (2 * text_size + 1) - 5, (line_size * 4) - 6, ((batt_length + 1) * 3) + (2 * text_size + 1) + 5, (line_size * 4) - 6 , WHITE);
    display.display();

  }

}


void loop()
{
  measuring(1);
  CURRENT_SOC();
  control_switch();
  Serial.print("adc1=");
  Serial.println(shunt[0]);
  Serial.print("adc2=");
  Serial.println(v[1]);
  Serial.print("adc1=");
  Serial.println(shunt[1]);
  Serial.print("adc2=");
  Serial.println(v[0]);
  Serial.print("c1=");
  Serial.println(c[0]);
  Serial.print("c2=");
  Serial.println(c[1]);
  Serial.print("v1=");
  Serial.println(vb[0]);
  Serial.print("v2=");
  Serial.println(vb[1]);
  Serial.println(SOC[0]);
  OLED_screens_8();


  delay(1000);
}
