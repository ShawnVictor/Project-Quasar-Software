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
IntervalTimer checkForProcessingMessage;

// DAQ Board States
uint8_t sol1_wsv2_state = 0;
uint8_t sol2_osv4_state = 0;
uint8_t sol3_osv5_state = 0;
uint8_t sol4_nsv2_state = 0;
uint8_t igniter_state = 0;
uint8_t ls1_wsv2_state = 0;
uint8_t ls2_osv4_state = 0;
uint8_t ls3_osv5_state = 0;
uint8_t ls4_nsv2_state = 0;
float otc1 = 0.0;
float otc2 = 0.0;
float otc3 = 0.0;
float otc4 = 0.0;
float pt1_opt1 = 0.0;
float pt2_opt2 = 0.0;
float pt3_opt3 = 0.0;
float pt4_npt1 = 0.0;
float pt5_npt2 = 0.0;



// Control Board States
bool wsv2_sw_state        = LOW;
bool osv4_sw_state        = LOW;
bool osv5_sw_state        = LOW;
bool nsv2_sw_state        = LOW;
bool emo_sw_state         = LOW;
bool armed_sw_state       = LOW;
bool ignition_sw_state    = LOW;
bool manual_mode_sw_state = LOW;

uint_8 buzzer_volume = 0;

bool arming_led_state       = LOW;
bool manual_mode_led_state  = LOW;
bool computing_led_state    = LOW;
bool sol1_wsv2_led_state    = LOW;
bool sol2_osv4_led_state    = LOW;
bool sol3_osv5_led_state    = LOW;
bool sol4_nsv2_led_state    = LOW;

String dataLine = "";
String processingDataLine = "";
uint8_t last6_counter = 5;
int ledState = LOW;



// This code will execute once.
void setup()
{
  
  // Setting up all Serial Lines.
  SERIAL_MONITOR.begin(SERIAL_MONITOR_BUAD);
  XBEE_SERIAL.begin(XBEE_SERIAL_BAUD);


  // Setting up all Pins.
  setupPins();


  // Setting IntervalTimer Priorities.
  heartbeat.priority(0);

  
  // Setting up IntervalTimers.
  heartbeat.begin(blinker, hzToMicro(4));
  indicatorLEDTimer.begin(updateLEDIndicators, hzToMicro(20));
  
}



// This code will execute continously.
void loop() 
{
  
  // STEP 1- Update all Input states
  updateInputStates();

  
  // STEP 2- Update Outputs of all LEDS & Buzzers
  updateOutputs();

  
  // STEP 3- XBEE Packet Check from the DAQ-Board
  checkForXBEEPacket();
  

  // STEP 4- Processing Packet check
  checkForProcessingPacket();
  
}



// Checks XBEE RX buffer for DAQ-Board Packets
void checkForXBEEPacket()
{
  
  char XBEE_last_read_character;
  
  if(XBEE_SERIAL.available())
  {
    XBEE_last_read_character = XBEE_SERIAL.read();

    // Check for the end of a packet.
    if(XBEE_last_read_character == '\n')
    {
      parseData(dataLine);
      sendProcessingPacket(); // Once a full DAQ-Packet has been received, send a processing packet
      dataLine = "";
    }
    else
    {
      // Append text to end of our constructing XBEE packet.
      dataLine += XBEE_last_read_character; 
    }
  }
  
}



//Checks SERIAL MONITOR RX Buffer for Processing Script Packets
void checkForProcessingPacket()
{
  
  char PROCESSING_last_read_character;

  if(SERIAL_MONITOR.available())
  {
    PROCESSING_last_read_character = SERIAL_MONITOR.read();

    // Check for the end of a packet.
    if(PROCESSING_last_read_character == '\n')
    {
      parseProcessingMessage(processingDataLine);
      sendDAQPacket(); // Once a full Processing-Packet has been received, send a Packet to the DAQ
      processingDataLine = "";  
    }
    else
    {
      // Append text to end of our constructing PROCESSING packet.
      processingDataLine += PROCESSING_last_read_character;
    }
  }
  
}



// Sends a DAQ-Board Packet though the XBEE TX Buffer
// WHAT A PACKET LOOKS LIKE: "\nULNK RCVD:{man_mode_sw, sol1_d_state, sol2_d_state, sol3_d_state, sol4_d_state, emo_sw_state, arming_sw_state, ignition_sw_state}"
void sendDAQPacket()
{
  XBEE_SERIAL.println();
  XBEE_SERIAL.print("ULNK RCVD:{");

  XBEE_SERIAL.print(manual_mode_sw_state);
  XBEE_SERIAL.print(",");
  
  XBEE_SERIAL.print(wsv2_sw_state);
  XBEE_SERIAL.print(",");
  XBEE_SERIAL.print(osv4_sw_state);
  XBEE_SERIAL.print(",");
  XBEE_SERIAL.print(osv5_sw_state);
  XBEE_SERIAL.print(",");
  XBEE_SERIAL.print(nsv2_sw_state);
  XBEE_SERIAL.print(",");

  XBEE_SERIAL.print(emo_sw_state);
  XBEE_SERIAL.print(",");

  XBEE_SERIAL.print(armed_sw_state);
  XBEE_SERIAL.print(",");

  XBEE_SERIAL.print(ignition_sw_state);
  XBEE_SERIAL.print("}");
}



// Send a Processing Script Packet though the Serial Monitor TX Buffer
// WHAT A PACKET LOOKS LIKE: "\nDLNK RCVD:{sol1, sol2, sol3, sol4, ls1, ls2, ls3, ls4, otc1, otc2, otc3, otc4, pt1, pt2, pt3, pt4, pt5, lc, scale}"
void sendProcessingPacket()
{
  SERIAL_MONITOR.println();
  SERIAL_MONITOR.print("DLNK RCVD:{");
  
  SERIAL_MONITOR.print(sol1_wsv2_state);
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(sol2_osv4_state);
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(sol3_osv5_state);
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(sol4_nsv2_state);
  SERIAL_MONITOR.print(",");
  
  SERIAL_MONITOR.print(igniter_state);
  SERIAL_MONITOR.print(",");
  
  SERIAL_MONITOR.print(ls1_wsv2_state);
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(ls2_osv4_state);
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(ls3_osv5_state);
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(ls4_nsv2_state);
  SERIAL_MONITOR.print(",");

  SERIAL_MONITOR.print(otc1);
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(otc2);
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(otc3);
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(otc4);
  SERIAL_MONITOR.print(",");

  SERIAL_MONITOR.print(pt1_opt1);
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(pt2_opt2);
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(pt3_opt3);
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(pt4_npt1);
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(pt5_npt2);
  SERIAL_MONITOR.print(",");

  SERIAL_MONITOR.print(load_cell);
  SERIAL_MONITOR.print(",");
  SERIAL_MONITOR.print(scale);
  SERIAL_MONITOR.print("}");
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
 * FUNCTION: Extacts the data from a String from the Processing Script and updates all globals 
 * INPUTS: String in the format: "\nPROS RCVD:{wsv2_d_state, osv4_d_state, osv5_d_state, nsv2_d_state, armed_sw_state, ignition_sw_state}"
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
  if(manual_mode_sw_state != HIGH){wsv2_sw_state = subarray.toInt();}
  currentString = currentString.substring(s.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  //Serial.print(subarray);
  if(manual_mode_sw_state != HIGH){osv4_sw_state = subarray.toInt()};
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  //Serial.print(subarray);
  if(manual_mode_sw_state != HIGH){osv5_sw_state = subarray.toInt()};
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  //Serial.print(subarray);
  if(manual_mode_sw_state != HIGH){nsv2_sw_state = subarray.toInt()};
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  //Serial.print(subarray);
  if(manual_mode_sw_state != HIGH){armed_sw_state = subarray.toInt()};
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf("}"));
  if(manual_mode_sw_state != HIGH){ignition_sw_state = subarray.toInt()};

}

void parseData(String s)
{
  if(s.length() == 0 || s.length() < minDataLength) 
  {
    return;
  }

  String currentString = s;
  String subarray = "";
  //Serial.print("This is the substring: ");
//-------------------------------------------------------------
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

//-------------------------------------------------------------
  subarray = currentString.substring(0, currentString.indexOf(","));
  igniter_state = subarray.toInt();
  currentString = currentString.substring(currentString.indexOf(",")+1);

//-------------------------------------------------------------
  subarray = currentString.substring(0, currentString.indexOf(","));
  ls1_wsv2_state = subarray.toInt();
  if(ls1_wsv2_state == 1){digitalWrite(wsv2_led, HIGH);}else{digitalWrite(wsv2_led, LOW);}
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  ls2_osv4_state = subarray.toInt();
  if(ls2_osv4_state == 1){digitalWrite(osv4_led, HIGH);}else{digitalWrite(osv4_led, LOW);}
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  ls3_osv5_state = subarray.toInt();
  if(ls3_osv5_state == 1){digitalWrite(osv5_led, HIGH);}else{digitalWrite(osv5_led, LOW);}
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf(","));
  ls4_nsv2_state = subarray.toInt();
  if(ls4_nsv2_state == 1){digitalWrite(nsv2_led, HIGH);}else{digitalWrite(nsv2_led, LOW);}
  currentString = currentString.substring(currentString.indexOf(",")+1);

//-------------------------------------------------------------
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

//-------------------------------------------------------------
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

//-------------------------------------------------------------
  subarray = currentString.substring(0, currentString.indexOf(","));
  load_cell = subarray.toFloat();
  currentString = currentString.substring(currentString.indexOf(",")+1);

  subarray = currentString.substring(0, currentString.indexOf("}"));
  scale = subarray.toFloat();

//-------------------------------------------------------------
  //Serial.print(millis());
  //Serial.println("Sucessfully got all data!");
}


void blinker()
{
  if(ledState == LOW){ledState = HIGH;}
  else{ledState = LOW;}
  digitalWrite(13, ledState);
}

void updateLEDIndicators()
{
  if(ls1_wsv2_state == 1){digitalWrite(wsv2_led, HIGH);}else{digitalWrite(wsv2_led, LOW);}

   if(ls2_osv4_state == 1)
  {
    digitalWrite(osv4_led, HIGH);
  }
  else
  {
    digitalWrite(osv4_led, LOW);
  }

   if(ls3_osv5_state == 1)
  {
    digitalWrite(osv5_led, HIGH);
  }
  else
  {
    digitalWrite(osv5_led, LOW);
  }

   if(ls4_nsv2_state == 1)
  {
    digitalWrite(nsv2_led, HIGH);
  }
  else
  {
    digitalWrite(nsv2_led, LOW);
  }

  if(processing_wsv2 == 1)
  {
    digitalWrite(sol_wsv2_relay, HIGH);
  }
  else
  {
    digitalWrite(sol_wsv2_relay, LOW);
  }

  if(processing_osv4 == 1)
  {
    digitalWrite(sol_osv4_relay, HIGH);
  }
  else
  {
    digitalWrite(sol_osv4_relay, LOW);
  }

  if(processing_osv5 == 1)
  {
    digitalWrite(sol_osv5_relay, HIGH);
  }
  else
  {
    digitalWrite(sol_osv5_relay, LOW);
  }

  if(processing_nsv2 == 1)
  {
    digitalWrite(sol_nsv2_relay, HIGH);
  }
  else
  {
    digitalWrite(sol_nsv2_relay, LOW);
  }

  if(processing_arming == 1)
  {
    digitalWrite(arming_led, HIGH);
    analogWrite(BUZZER, 255);
  }
  else
  {
    digitalWrite(arming_led, LOW);
    analogWrite(BUZZER, 0);
  }

  if(processing_igniter == 1 && processing_arming == 1)
  {
    digitalWrite(igniter_led, HIGH);
  }
  else
  {
    digitalWrite(igniter_led, LOW);
  }
}
