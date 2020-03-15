#define buzzer   9
#define sol_wsv2_relay 2
#define sol_osv4_relay 3
#define sol_osv5_relay 4
#define sol_nsv2_relay 5
#define arming_led 11
#define igniter_led 6
#define wsv2_led 25
#define osv4_led 26
#define osv5_led 27
#define nsv2_led 28
#define minDataLength  10

IntervalTimer heartbeatTimer;
IntervalTimer indicatorLEDTimer;
IntervalTimer checkForProcessingMessage;
uint8_t sol1_wsv2_state = 1;
uint8_t sol2_osv4_state = 2;
uint8_t sol3_osv5_state = 3;
uint8_t sol4_nsv2_state = 4;
uint8_t igniter_state   = 5;
uint8_t ls1_wsv2_state  = 6;
uint8_t ls2_osv4_state  = 7;
uint8_t ls3_osv5_state  = 8;
uint8_t ls4_nsv2_state  = 9;
float     otc1          = 1;
float     otc2          = 2;
float     otc3          = 3;
float     otc4          = 4;
int     pt1_opt1        = 5;
int     pt2_opt2        = 6;
int     pt3_opt3        = 7;
int     pt4_npt1        = 8;
int     pt5_npt2        = 9;
int     load_cell       = 1;
int     scale           = 2;
int  processing_wsv2     = 0;
int  processing_osv4     = 0;
int  processing_osv5     = 0;
int  processing_nsv2     = 0;
int  processing_arming   = 0;
int  processing_igniter  = 0;
String dataLine = "";
String processingDataLine = "";
uint8_t last6_counter = 5;
int ledState = LOW;


void setup()
{
  Serial.begin(9600);
  Serial4.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(wsv2_led, OUTPUT);
  pinMode(osv4_led, OUTPUT);
  pinMode(osv5_led, OUTPUT);
  pinMode(nsv2_led, OUTPUT);
  pinMode(sol_wsv2_relay, OUTPUT);
  pinMode(sol_osv4_relay, OUTPUT);
  pinMode(sol_osv5_relay, OUTPUT);
  pinMode(sol_nsv2_relay, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(arming_led, OUTPUT);
  pinMode(igniter_led, OUTPUT);
  heartbeatTimer.begin(blinker,250000);
//  heartbeatTimer.begin(processingMessageCheck, 1000000);
  indicatorLEDTimer.begin(updateLEDIndicators, 50000);
  
}
void tloop()
{
  if(Serial4.available())
  {
    Serial.write(Serial4.read());
  }
}

void loop() {

  char c;
  
  if(Serial4.available())
  {
    c = Serial4.read();
    if(c == '\n')
    {
      parseData(dataLine);
      serialPrintAllSensors();
      dataLine = "";
    }
    else
    {
      dataLine += c; //append text to end of command
    }
  }

  //Processing Message check
  char c2;

  if(Serial.available())
  {
    c2 = Serial.read();
    if(c2 == '\n')
    {
      parseProcessingMessage(processingDataLine);
      processingDataLine = "";  
    }
    else
    {
      processingDataLine += c2;
    }
  }
}

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
    analogWrite(buzzer, 255);
  }
  else
  {
    digitalWrite(arming_led, LOW);
    analogWrite(buzzer, 0);
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
