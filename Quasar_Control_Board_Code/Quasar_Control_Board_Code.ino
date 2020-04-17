/**
 *   Quasar_Control_Board_Code
 *   Developed by: Shawn Victor & Nikhita Ramanujam
 *   Last Modified: 3-16-2020
 */

 /**
  * ------------------------------------------------------------------
  *   TEENSY 3.6 PIN     |   SIGNAL NAME
  * ------------------------------------------------------------------
  *         0                 XBEE_TX
  *         1                 XBEE_RX
  *         2                 ARMING_SWITCH_SIG
  *         3                 IGNITION_SWITCH_SIG
  *         4                 MANUAL_MODE_SWITCH_SIG
  *         5                 SOL1_WSV2_SWITCH_SIG
  *         6                 SOL2_OSV4_SWITCH_SIG
  *         7                 SOL3_OSV5_SWITCH_SIG
  *         8                 SOL4_NSV2_SWITCH_SIG
  *         9                 BUZZER
  *         10                --
  *         11                ARMING_LED_INDICATOR
  *         12                MANUAL_MODE_LED_INDICATOR
  *         24                COMPUTING_LED_INDICATOR
  *         25                SOL1_WSV2_LED_INDICATOR
  *         26                SOL2_OSV4_LED_INDICATOR
  *         27                SOL3_OSV5_LED_INDICATOR
  *         28                SOL4_NSV2_LED_INDICATOR

  */



// Macros
#define SERIAL_MONITOR      Serial
#define SERIAL_MONITOR_BAUD 9600
#define XBEE_SERIAL         Serial4
#define XBEE_SERIAL_BAUD    9600
#define ARMING_SWITCH       2
#define IGNITION_SWITCH     3
#define MANUAL_MODE_SWITCH  4
#define SOL1_WSV2_SWITCH    5
#define SOL2_OSV4_SWITCH    6
#define SOL3_OSV5_SWITCH    7
#define SOL4_NSV2_SWITCH    8
#define BUZZER              9
#define ARMING_LED          11
#define MANUAL_MODE_LED     12
#define COMPUTING_LED       24
#define SOL1_WSV2_LED       25
#define SOL2_OSV4_LED       26
#define SOL3_OSV5_LED       27
#define SOL4_NSV2_LED       28



// Libraries



// Globals
IntervalTimer heartbeat;
IntervalTimer indicatorLEDTimer;
IntervalTimer checkForProcessingMessage;

//From DAQ
int sol1_wsv2_state = 0;
int sol2_osv4_state = 0;
int sol3_osv5_state = 0;
int sol4_nsv2_state = 0;
int igniter_state = 0;
int ls1_wsv2_state = 0;
int ls2_osv4_state = 0;
int ls3_osv5_state = 0;
int ls4_nsv2_state = 0;
float otc1 = 0;
float otc2 = 0;
float otc3 = 0;
float otc4 = 0;
float pt1_opt1 = 0;
float pt2_opt2 = 0;
float pt3_opt3 = 0;
float pt4_npt1 = 0;
float pt5_npt2 = 0;
float load_cell = 0;
float scale = 0;

//From Controller
bool wsv2_sw_state        = LOW;
bool osv4_sw_state        = LOW;
bool osv5_sw_state        = LOW;
bool nsv2_sw_state        = LOW;
bool armed_sw_state       = LOW;
bool ignition_sw_state    = LOW;
bool manual_mode_sw_state = LOW;
uint8_t buzzer_volume = 0;
bool arming_led_state       = LOW;
bool manual_mode_led_state  = LOW;
bool computing_led_state    = LOW;
bool sol1_wsv2_led_state    = LOW;
bool sol2_osv4_led_state    = LOW;
bool sol3_osv5_led_state    = LOW;
bool sol4_nsv2_led_state    = LOW;

//From Processing Serial
int processing_wsv2;
int processing_osv4;
int processing_osv5;
int processing_nsv2;
int processing_arming;
int processing_igniter;

String dataLine = "";
String processingDataLine = "";
uint8_t last6_counter = 5;
int ledState = LOW;



// This code will execute once.
void setup()
{
  
  // Setting up all Serial Lines.
  SERIAL_MONITOR.begin(9600);
  XBEE_SERIAL.begin(9600);


  // Setting up all Pins.
  setupPins();


  // Setting IntervalTimer Priorities.
  heartbeat.priority(0);

  
  // Setting up IntervalTimers.
  heartbeat.begin(blinker, hzToMicro(4));
  
}



// This code will execute continously.
void loop() 
{
  
  // Update all Input states
  updateInputStates();

  
  // Update Outputs of all LEDS & Buzzers
  updateOutputs();

  
  // XBEE Message Check
  char XBEE_last_read_character;


  // Check if data is available from the XBEE RX Buffer.
  if(XBEE_SERIAL.available())
  {
    XBEE_last_read_character = XBEE_SERIAL.read();

    // Check for the end of a packet.
    if(XBEE_last_read_character == '\n')
    {
      parseData(dataLine);
      serialPrintAllSensors();
      dataLine = "";
    }
    else
    {
      // Append text to end of our constructing XBEE packet.
      dataLine += XBEE_last_read_character; 
    }
  }


  //Processing Message check
  char PROCESSING_last_read_character;

  if(SERIAL_MONITOR.available())
  {
    PROCESSING_last_read_character = SERIAL_MONITOR.read();

    // Check for the end of a packet.
    if(PROCESSING_last_read_character == '\n')
    {
      parseProcessingMessage(processingDataLine);
      processingDataLine = "";  
    }
    else
    {
      // Append text to end of our constructing PROCESSING packet.
      processingDataLine += PROCESSING_last_read_character;
    }
  }
  
}



// Declares all of the Pins.
void setupPins()
{
  
  // Switch Inputs.
  pinMode(ARMING_SWITCH,      INPUT);
  pinMode(IGNITION_SWITCH,    INPUT);
  pinMode(MANUAL_MODE_SWITCH, INPUT);
  pinMode(SOL1_WSV2_SWITCH,   INPUT);
  pinMode(SOL2_OSV4_SWITCH,   INPUT);
  pinMode(SOL3_OSV5_SWITCH,   INPUT);
  pinMode(SOL4_NSV2_SWITCH,   INPUT);

  // LED & Buzzer Outputs.
  pinMode(BUZZER,          OUTPUT);
  pinMode(ARMING_LED,      OUTPUT);
  pinMode(MANUAL_MODE_LED, OUTPUT);
  pinMode(COMPUTING_LED,   OUTPUT);
  pinMode(SOL1_WSV2_LED,   OUTPUT);
  pinMode(SOL2_OSV4_LED,   OUTPUT);
  pinMode(SOL3_OSV5_LED,   OUTPUT);
  pinMode(SOL4_NSV2_LED,   OUTPUT);
  
}



// Update Input States
void updateInputStates()
{
  armed_sw_state = digitalRead(ARMING_SWITCH);  
  ignition_sw_state = digitalRead(IGNITION_SWITCH);
  manual_mode_sw_state = digitalRead(MANUAL_MODE_SWITCH);
  wsv2_sw_state = digitalRead(SOL1_WSV2_SWITCH);
  osv4_sw_state = digitalRead(SOL2_OSV4_SWITCH);
  osv5_sw_state = digitalRead(SOL3_OSV5_SWITCH);
  nsv2_sw_state = digitalRead(SOL4_NSV2_SWITCH);
}



// Updates all Output Pins.
void updateOutputs()
{
  
  // If Armed...
  if( ARMING_SWITCH == HIGH )
  {
    analogWrite(BUZZER, 1000);
    digitalWrite(ARMING_LED, HIGH);
  }
  else
  {
    analogWrite(BUZZER, 0);
    digitalWrite(ARMING_LED, LOW);
  }

  // Updates all Output Pins.
  digitalWrite(MANUAL_MODE_LED, manual_mode_sw_state);
  digitalWrite(COMPUTING_LED, computing_led_state);
  digitalWrite(SOL1_WSV2_LED, sol1_wsv2_led_state);
  digitalWrite(SOL2_OSV4_LED, sol2_osv4_led_state);
  digitalWrite(SOL3_OSV5_LED, sol3_osv5_led_state);
  digitalWrite(SOL4_NSV2_LED, sol4_nsv2_led_state);
}



void tloop()
{
  if(Serial4.available())
  {
    Serial.write(Serial4.read());
  }
}



/*
 * FUNCTION: Converts a Freqency Parameter in Hz to Microseconds
 * INPUT: Integer Frequency in Hz
 * OUTPUT: Integer Period in microseconds
 */
int hzToMicro(int hz)
{
  return (1000000/hz);
}



/*
 *    FUNCTION: Extacts the data from a String from the Processing Script and updates all globals 
 *    INPUTS: String in the format: "PROS RCVD:{<WSV2-ds>,<OSV4-ds>,<OSV5-ds>,<NSV2-ds>,<ARMING>,<IGNITER>,<>,<>,<>,<>,<>}"
 *        
 */
void parseProcessingMessage(String s)
{
  if(s.length() == 0 || s.length() < 10) //17 is the expect number
  {
    return;
  }

  String currentString = s;
  String subarray = "";

  subarray = currentString.substring(s.indexOf("{")+1, s.indexOf(","));
  //Serial.print(subarray);
  processing_wsv2 = subarray.toInt();
  currentString = currentString.substring(s.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  //Serial.print(subarray);
  processing_osv4 = subarray.toInt();
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  //Serial.print(subarray);
  processing_osv5 = subarray.toInt();
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  //Serial.print(subarray);
  processing_nsv2 = subarray.toInt();
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  //Serial.print(subarray);
  processing_arming = subarray.toInt();
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf("}"));
  processing_igniter = subarray.toInt();

}


 /*
 *    FUNCTION: Extracts the data from a String from the XBEE and updates all of the globals
 *    INPUTS:   String in the format: "DWNLNK RCVD{<WSV2-cs>, <OSV4-cs>, <OSV5-cs>, <NSV2-cs>, <IGNTR-cs>, <LS1-WSV2>, <LS2-OSV4>, <LS3-OSV5>, <LS4-NSV2>, <OTC1>, <OTC2>, <OTC3>, <OTC4>, <OPT1>, <OPT2>, <OPT3>, <NPT1>, <NPT2>, <LOAD_CELL>, <SCALE>}"
 
 */
void parseData(String s)
{
  if(s.length() == 0 || s.length() < 20) 
  {
    return;
  }

  String currentString = s;
  String subarray = "";
  //Serial.print("This is the substring: ");
//SOLENOIDS-------------------------------------------------------------
  subarray = currentString.substring(s.indexOf("{")+1, s.indexOf(","));
  //Serial.print(subarray);
  sol1_wsv2_state = subarray.toInt();
  currentString = currentString.substring(s.indexOf(",")+1);
  
  subarray = currentString.substring(0, currentString.indexOf(","));
  //Serial.print(subarray);
  sol2_osv4_state = subarray.toInt();
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  //Serial.print(subarray);
  sol3_osv5_state = subarray.toInt();
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  //Serial.println(subarray);
  sol4_nsv2_state = subarray.toInt();
  currentString = currentString.substring(currentString.indexOf(",")+1);

//IGNITER-------------------------------------------------------------
  subarray = currentString.substring(0, currentString.indexOf(","));
  igniter_state = subarray.toInt();
  currentString = currentString.substring(currentString.indexOf(",")+1);

//LIMIT_SWITCH-------------------------------------------------------------
  subarray = currentString.substring(0, currentString.indexOf(","));
  ls1_wsv2_state = subarray.toInt();
  if(ls1_wsv2_state == 1){digitalWrite(SOL1_WSV2_LED, HIGH);}else{digitalWrite(SOL1_WSV2_LED, LOW);}
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  ls2_osv4_state = subarray.toInt();
  if(ls2_osv4_state == 1){digitalWrite(SOL2_OSV4_LED, HIGH);}else{digitalWrite(SOL2_OSV4_LED, LOW);}
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  ls3_osv5_state = subarray.toInt();
  if(ls3_osv5_state == 1){digitalWrite(SOL3_OSV5_LED, HIGH);}else{digitalWrite(SOL3_OSV5_LED, LOW);}
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  ls4_nsv2_state = subarray.toInt();
  if(ls4_nsv2_state == 1){digitalWrite(SOL4_NSV2_LED, HIGH);}else{digitalWrite(SOL4_NSV2_LED, LOW);}
  currentString = currentString.substring(currentString.indexOf(",")+1);

//THERMOCOUPLES-------------------------------------------------------------
  subarray = currentString.substring(0, currentString.indexOf(","));
  otc1 = subarray.toFloat();
  currentString = currentString.substring(currentString.indexOf(",")+1);
  
  subarray = currentString.substring(0, currentString.indexOf(","));
  otc2 = subarray.toFloat();
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  otc3 = subarray.toFloat();
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  otc4 = subarray.toFloat();
  currentString = currentString.substring(currentString.indexOf(",")+1);

//PRESSURE_TRANSDUCERS-------------------------------------------------------------
  subarray = currentString.substring(0, currentString.indexOf(","));
  pt1_opt1 = subarray.toFloat();
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  pt2_opt2 = subarray.toFloat();
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  pt3_opt3 = subarray.toFloat();
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  pt4_npt1 = subarray.toFloat();
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  pt5_npt2 = subarray.toFloat();
  currentString = currentString.substring(currentString.indexOf(",")+1);

//LOAD_CELL_&_SCALE-------------------------------------------------------------
  subarray = currentString.substring(0, currentString.indexOf(","));
  load_cell = subarray.toFloat();
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf("}"));
  scale = subarray.toFloat();

//-------------------------------------------------------------
  //Serial.print(millis());
  //Serial.println("Sucessfully got all data!");
}



/*
*   FUNCTION: Responsible for the Board's Heartbeat
*   INPUTS:   N/A
*   OUTPUTS:  N/A
*/
void blinker()
{
  if(ledState == LOW){ledState = HIGH;}
  else{ledState = LOW;}

  digitalWrite(13, ledState);
}



void serialPrintAllSensors()
{
  Serial.println();
  Serial.print("DLNK RCVD:{");
  
  Serial.print(sol1_wsv2_state);
  Serial.print(",");
  Serial.print(sol2_osv4_state);
  Serial.print(",");
  Serial.print(sol3_osv5_state);
  Serial.print(",");
  Serial.print(sol4_nsv2_state);
  Serial.print(",");
  
  Serial.print(igniter_state);
  Serial.print(",");
  
  Serial.print(ls1_wsv2_state);
  Serial.print(",");
  Serial.print(ls2_osv4_state);
  Serial.print(",");
  Serial.print(ls3_osv5_state);
  Serial.print(",");
  Serial.print(ls4_nsv2_state);
  Serial.print(",");

  Serial.print(otc1);
  Serial.print(",");
  Serial.print(otc2);
  Serial.print(",");
  Serial.print(otc3);
  Serial.print(",");
  Serial.print(otc4);
  Serial.print(",");

  Serial.print(pt1_opt1);
  Serial.print(",");
  Serial.print(pt2_opt2);
  Serial.print(",");
  Serial.print(pt3_opt3);
  Serial.print(",");
  Serial.print(pt4_npt1);
  Serial.print(",");
  Serial.print(pt5_npt2);
  Serial.print(",");

  Serial.print(load_cell);
  Serial.print(",");
  Serial.print(scale);
  Serial.print("}");
}
