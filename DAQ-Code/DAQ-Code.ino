/**
 *   DAQ-Code
 *   Developed by: Shawn Victor
 *   Last Modified: 2-8-2020
 */

 /**
  * ------------------------------------------------------------------
  *   TEENSY 3.6 PIN     |   SIGNAL NAME
  * ------------------------------------------------------------------
  *         0                 XBEE_TX
  *         1                 XBEE_RX
  *         2                 SOL4_NSV2
  *         3                 SOL3_OSV5
  *         4                 SOL2_OSV4
  *         5                 SOL1_WSV2
  *         6                 IGNITER
  *         7                 LS4_NSV2
  *         8                 LS3_OSV5
  *         9                 LS2_OSV4
  *         10                LS1_WSV2
  *         18                SDA
  *         19                SCL
  *         22                TC_DO
  *         23                TC_SCK
  *         24                OTC2_CS
  *         25                OTC3_CS
  *         26                OTC1_CS
  *         27                OTC4_CS
  *         28                ARMED_LED
  *         29                IGNITION_LED
  */

// Libraries
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_MAX31855.h"
#include <Adafruit_ADS1015.h>



// Macros
#define SOL4_NSV2                2
#define SOL3_OSV5                3
#define SOL2_OSV4                4
#define SOL1_WSV2                5
#define IGNITER                  6
#define LS4_NSV2                 7
#define LS3_OSV5                 8
#define LS2_OSV4                 9
#define LS1_WSV2                 10
#define TC_DO                    23
#define TC_SCK                   22
#define OTC2_CS                  24
#define OTC3_CS                  25
#define OTC1_CS                  26
#define OTC4_CS                  27
#define ARMED_LED                28
#define IGNITER_LED              29
#define BAUD_RATE                9600
#define HEARTBEAT_RATE           8
#define SERIAL_OUTPUT_RATE       8
#define ADC_REFRESH_RATE         4
#define TC_REFRESH_RATE          4


// Globals
bool sol1_wsv2 = HIGH;
bool sol2_osv4 = HIGH;
bool sol3_osv5 = LOW;
bool sol4_nsv2 = LOW;
bool igniter   = LOW;

bool armed_led = LOW;
bool igniter_led = LOW;

int8_t ls1_wsv2 = 0;
int8_t ls2_osv4 = 0;
int8_t ls3_osv5 = 0;
int8_t ls4_nsv2 = 0;

float pt1_opt1 = 0.0;
float pt2_opt2 = 0.0;
float pt3_opt3 = 0.0;
float pt4_npt1 = 0.0;
float pt5_npt2 = 0.0;

float otc1 = 0.0;
float otc2 = 0.0;
float otc3 = 0.0;
float otc4 = 0.0;

float load_cell = 0.0;
float scale = 0.0;

bool ledState = LOW;

String dataLine = "";

Adafruit_MAX31855 tc1(TC_SCK, OTC1_CS, TC_DO);
Adafruit_MAX31855 tc2(TC_SCK, OTC2_CS, TC_DO);
Adafruit_MAX31855 tc3(TC_SCK, OTC3_CS, TC_DO);
Adafruit_MAX31855 tc4(TC_SCK, OTC4_CS, TC_DO);

Adafruit_ADS1115 ads1(0x48);
Adafruit_ADS1115 ads2(0x49);
Adafruit_ADS1115 ads3(0x4A);

IntervalTimer heartbeatTimer;
IntervalTimer aquireADCData;
IntervalTimer aquireThermocoupleData;
IntervalTimer updateSerialPrinter;



// Code executed once
void setup() 
{
  // XBEE & Serial Monitor Baud Rate
  Serial1.begin(BAUD_RATE);

  // Setting Pin Modes
  setupPins();

  // Setting all Solenoid 
  setSolenoidsLow();

  // Setting up the 3 ADC Modules
  setupADC();


  // Setting all IntervalTimers Priorities (Lower Value --> Higher Priority)
  heartbeatTimer.priority(0);


  // Starting all IntervalTimres @ specified rates
  heartbeatTimer.begin(blinker, hzToMicro(HEARTBEAT_RATE));
  updateSerialPrinter.begin(serialPrintAllSensors, hzToMicro(SERIAL_OUTPUT_RATE));
  aquireADCData.begin(aquireADCs, hzToMicro(ADC_REFRESH_RATE));
  aquireThermocoupleData.begin(aquireTCs, hzToMicro(TC_REFRESH_RATE));
}



// Code will Continously Execute
void loop() 
{
  // Aquire all Limit Switch Input Data
  //aquireLSs();

  // Update the state of all Outputs
  outputUpdater();


  // XBEE Parsing System
  char c;

  if(Serial.available())
  {
    c = Serial.read();
    if(c == '\n')
    {
      parseData(dataLine);
      dataLine = "";
    }
    else
    {
      dataLine += c;
    }
  }
}



// Function that reads the digital signals from each limit switch
void aquireLSs()
{
  ls1_wsv2 = digitalRead(LS1_WSV2);
  ls2_osv4 = digitalRead(LS2_OSV4);
  ls3_osv5 = digitalRead(LS3_OSV5);
  ls4_nsv2 = digitalRead(LS4_NSV2);
}



// Function that reads the temperature data from each thermocouple
void aquireTCs()
{
  //noInterrupts();
  otc1 = tc1.readCelsius();
  otc2 = tc2.readCelsius();
  otc3 = tc3.readCelsius();
  otc4 = tc4.readCelsius();
  //interrupts();
}



// Function that reads the pressure data from each pressure transducer
void aquireADCs()
{
  aquireLSs();
  //noInterrupts();
  pt1_opt1 = 1 * ads1.readADC_SingleEnded(0) + 0;
  pt2_opt2 = 1 * ads1.readADC_SingleEnded(1) + 0;
  pt3_opt3 = 1 * ads1.readADC_SingleEnded(2) + 0;
  pt4_npt1 = 1 * ads1.readADC_SingleEnded(3) + 0;
  pt5_npt2 = 1 * ads2.readADC_SingleEnded(0) + 0;
  aquireLCs();
  //interrupts();
}



// Function that reads the force data from each load cell
void aquireLCs()
{
  load_cell = 1 * ads3.readADC_Differential_0_1() + 0;
  scale = 1 * ads3.readADC_Differential_2_3() + 0;
}



// Function that updates all outputs
void outputUpdater()
{
  digitalWrite(SOL1_WSV2, !sol1_wsv2);  
  digitalWrite(SOL2_OSV4, !sol2_osv4); 
  digitalWrite(SOL3_OSV5, !sol3_osv5); 
  digitalWrite(SOL4_NSV2, !sol4_nsv2); 
  digitalWrite(IGNITER, !igniter);
  digitalWrite(ARMED_LED, armed_led);
  digitalWrite(IGNITER_LED, igniter);
}



// Function that defines the addresses of each of the ADCs
void setupADC()
{
  ads1.begin();
  ads2.begin();
  ads3.begin();
}



// Function that defines the IO modes of each pin
void setupPins()
{
  pinMode(SOL1_WSV2, OUTPUT);
  pinMode(SOL2_OSV4, OUTPUT);
  pinMode(SOL3_OSV5, OUTPUT);
  pinMode(SOL4_NSV2, OUTPUT);
  pinMode(IGNITER, OUTPUT);
  pinMode(ARMED_LED, OUTPUT);
  pinMode(IGNITER_LED, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(LS1_WSV2, INPUT);
  pinMode(LS2_OSV4, INPUT);
  pinMode(LS3_OSV5, INPUT);
  pinMode(LS4_NSV2, INPUT);
}



void setSolenoidsLow()
{
  digitalWrite(SOL1_WSV2, HIGH);
  digitalWrite(SOL2_OSV4, HIGH);
  digitalWrite(SOL3_OSV5, HIGH);
  digitalWrite(SOL4_NSV2, HIGH);
}



int hzToMicro(int hz)
{
  return (1000000/hz);
}



void blinker()
{
  if(ledState == LOW)
  {
    ledState = HIGH;
  }
  else{ledState = LOW;}

  digitalWrite(13, ledState);
}



void serialPrintAllSensors()
{
  Serial1.println();
  Serial1.print("DWNLNK:{");
  
  Serial1.print(sol1_wsv2);
  Serial1.print(",");
  Serial1.print(sol2_osv4);
  Serial1.print(",");
  Serial1.print(sol3_osv5);
  Serial1.print(",");
  Serial1.print(sol4_nsv2);
  Serial1.print(",");
  
  Serial1.print(igniter);
  Serial1.print(",");
  
  Serial1.print(ls1_wsv2);
  Serial1.print(",");
  Serial1.print(ls2_osv4);
  Serial1.print(",");
  Serial1.print(ls3_osv5);
  Serial1.print(",");
  Serial1.print(ls4_nsv2);
  Serial1.print(",");

  Serial1.print(otc1);
  Serial1.print(",");
  Serial1.print(otc2);
  Serial1.print(",");
  Serial1.print(otc3);
  Serial1.print(",");
  Serial1.print(otc4);
  Serial1.print(",");

  Serial1.print(pt1_opt1);
  Serial1.print(",");
  Serial1.print(pt2_opt2);
  Serial1.print(",");
  Serial1.print(pt3_opt3);
  Serial1.print(",");
  Serial1.print(pt4_npt1);
  Serial1.print(",");
  Serial1.print(pt5_npt2);
  Serial1.print(",");

  Serial1.print(load_cell);
  Serial1.print(",");
  Serial1.print(scale);
  Serial1.print("}");
}



void parseData(String s)
{
  if(s.length() == 0 || s.length() < 10)
  {
    return;
  }

  String currentString = s;
  String subarray = "";

  subarray = currentString.substring(s.indexOf("{")+1, s.indexOf(","));
  if(subarray.toInt() == 0){sol1_wsv2 = HIGH;} else{sol1_wsv2 = LOW;}
  currentString = currentString.substring(s.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  if(subarray.toInt() == 0){sol2_osv4 = HIGH;} else{sol2_osv4 = LOW;}
  currentString = currentString.substring(s.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  if(subarray.toInt() == 0){sol3_osv5 = HIGH;} else{sol3_osv5 = LOW;}
  currentString = currentString.substring(s.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  if(subarray.toInt() == 0){sol4_nsv2 = HIGH;} else{sol4_nsv2 = LOW;}
  currentString = currentString.substring(s.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  if(subarray.toInt() == 0){armed_led = LOW;} else{armed_led = HIGH;}
  currentString = currentString.substring(s.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf("}"));
  if(subarray.toInt() == 1 && armed_led == HIGH){igniter = LOW; igniter_led = HIGH;} else{igniter = HIGH; igniter_led = LOW;}
}
