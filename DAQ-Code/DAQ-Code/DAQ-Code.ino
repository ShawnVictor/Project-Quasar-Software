/**
 *   DAQ-Code
 *   Developed by: Shawn Victor
 *   Last Modified: 2-4-2020
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

int8_t sol1_wsv2 = 0;
int8_t sol2_osv4 = 0;
int8_t sol3_osv5 = 0;
int8_t sol4_nsv2 = 0;

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


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
