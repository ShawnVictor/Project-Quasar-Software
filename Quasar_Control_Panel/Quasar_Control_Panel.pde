/*
 * Quasar_ControlPanel.ino
 * Code by: Shawn Victor
 * Last Modified: 1/9/2020 
 */


//Importing Libraries
import java.awt.Frame;
import java.awt.BorderLayout;
import javax.swing.JLabel;
import javax.swing.ImageIcon;
import controlP5.*;
import processing.serial.*;


// P&ID Image File
PImage diagram;
PImage logo;

// Data From Quasar Control Board
int     sol1_wsv2_state = 1;
int     sol2_osv4_state = 2;
int     sol3_osv5_state = 3;
int     sol4_nsv2_state = 4;
int     igniter_state   = 5;
int     ls1_wsv2_state  = 6;
int     ls2_osv4_state  = 7;
int     ls3_osv5_state  = 8;
int     ls4_nsv2_state  = 9;
float     otc1            = 1;
float     otc2            = 2;
float     otc3            = 3;
float     otc4            = 4;
float     pt1_opt1        = 5;
float     pt2_opt2        = 6;
float     pt3_opt3        = 7;
float     pt4_npt1        = 8;
float     pt5_npt2        = 9;
float     load_cell       = 1;
float     scale           = 2;

int processing_wsv2 = 0;
int processing_osv4 = 0;
int processing_osv5 = 0;
int processing_nsv2 = 0;
int processing_arming = 0;
int processing_ignite = 0;


// Linegraph Recommended Paramters
int linegraph_width  = 300;
int linegraph_height = 175;

// Serial Port Parameters
Serial serialPort;
String serialPortName = "COM7";
int    baud_rate      = 9600;
byte[] inBuffer       = new byte[100];
int    i           = 0;

// Used for Panel Interfacing
ControlP5 cp5;

// Settings for Plotter as saved in this file
JSONObject plotterConfigJSON;
String     topSketchPath = "";

// List of Plot Positions
int[] opt1_pos = {65,50};
int[] opt2_pos = {445,50};
int[] opt3_pos = {1585,50};
int[] npt1_pos = {1205,50};
int[] npt2_pos = {825,50};
int[] otc1_pos = {65,320};
int[] otc2_pos = {65,590};
int[] otc3_pos = {1585,320};
int[] otc4_pos = {1585,590};
int[] lc_pos   = {645, 760};
int[] scale_pos = {1025, 760};

int[] wsv2_led_pos = {980,300};
int[] osv4_led_pos = {885,370};
int[] osv5_led_pos = {975,445};
int[] nsv2_led_pos = {1050,555};

//List of LineGraphs
Graph  opt1_linegraph  = new Graph(opt1_pos[0], opt1_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  opt2_linegraph  = new Graph(opt2_pos[0], opt2_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  opt3_linegraph  = new Graph(opt3_pos[0], opt3_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  npt1_linegraph  = new Graph(npt1_pos[0], npt1_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  npt2_linegraph  = new Graph(npt2_pos[0], npt2_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  otc1_linegraph  = new Graph(otc1_pos[0], otc1_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  otc2_linegraph  = new Graph(otc2_pos[0], otc2_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  otc3_linegraph  = new Graph(otc3_pos[0], otc3_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  otc4_linegraph  = new Graph(otc4_pos[0], otc4_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  lc_linegraph    = new Graph(lc_pos[0], lc_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));
Graph  scale_linegraph = new Graph(scale_pos[0], scale_pos[1], linegraph_width, linegraph_height, color(71, 71, 71));

// Data Matrix for each LineGraph (rows are for each line graph within the single graph, columns are the datapoints)
float[][] opt1_linegraph_values = new float[1][100];
float[][] opt2_linegraph_values = new float[1][100];
float[][] opt3_linegraph_values = new float[1][100];
float[][] npt1_linegraph_values = new float[1][100];
float[][] npt2_linegraph_values = new float[1][100];
float[][] otc1_linegraph_values = new float[1][100];
float[][] otc2_linegraph_values = new float[1][100];
float[][] otc3_linegraph_values = new float[1][100];
float[][] otc4_linegraph_values = new float[1][100];
float[][] lc_linegraph_values   = new float[1][100];
float[][] scale_linegraph_values= new float[1][100];

// Sample Data
float[]   linegraph_sample = new float[100];

// Graph Colors
color[] graphColors = new color[3];




void setup()
{ 
  graphColors[0] = color(131, 255, 20); //GREEN
  graphColors[1] = color(147, 39, 143); //PURPLE
  graphColors[2] = color(193, 39, 45); //RED
 
  frame.setTitle("Quasar_ControlPanel");
  size(1905,1000);

  // Import Image of P&ID
  diagram = loadImage("LRA_P&ID.jpg");
  logo    = loadImage("Quasar Logo.jpg");

  // Settings Save File
  topSketchPath      = sketchPath();
  plotterConfigJSON  = loadJSONObject(topSketchPath + "/plotter_config.json");

  // Setup ControlP5
  cp5 = new ControlP5(this);

  // Clearing each Data Matrix for the Line Graphs
  clearAllDataValues();

  // Starting Serial Communications
  serialPort = new Serial(this, serialPortName, baud_rate);

   // Creating the Textfields for Graph Range modification
   createMinMaxRange();
   
   //Create The Sliders for Controlling Actuators
   createActuatorSliders();

   //Setup Parameters of Each Line Graph
   setChartSettings();
   System.out.println("Hey shawn I got here!");
}



void draw()
{
  if( serialPort.available() > 0 )
  {
    String myString = "";

    // Reads from the Serial Monitor and pull a single string data packet
    try {serialPort.readBytesUntil('\n', inBuffer);}
    catch(Exception e)
    {
      myString = new String(inBuffer);
    }
    myString = new String(inBuffer);


    // Parse and Update Sensor Values
    parseAndUpdateData(myString);

    // Updating Each of the Line Graph matrixs
    updateEachLineGraphMatrix();
  }


  // Set a Dark-Grey Background
  background(30); 
  image(diagram, 387, 285);
  image(logo, 0, 835);
  
  
  // Update Sensor Value Texts
   updateSensorText();
   
   //Update Status LEDs
   updateStatusLEDs();

  // Draw Each Axis of the Line Graphs
  drawEachLineGraphAxis();

  // Plot Each Graph
  plotEachGraph();
}



void updateSensorText()
{
  fill(40); color(0);stroke(147, 39, 143);strokeWeight(2);
  rect(740,432,100,25, 10.0); //OPT1 BOX
  textSize(12);stroke(225);fill(225); 
  text(pt1_opt1 + " psia", 830, 450);
  
  fill(40); color(0);stroke(147, 39, 143);strokeWeight(2);
  rect(1000,432,100,25, 10.0); //OPT2 BOX
  textSize(12);stroke(225);fill(225); 
  text(pt2_opt2 + " psia", 1090, 450);
  
  fill(40); color(0);stroke(147, 39, 143);strokeWeight(2);
  rect(1200,492,100,25, 10.0); //OPT3 BOX
  textSize(12);stroke(225);fill(225); 
  text(pt3_opt3 + " psia", 1290, 510);
  
  fill(40); color(0);stroke(147, 39, 143);strokeWeight(2);
  rect(875,535,100,25, 10.0); //NPT1 BOX
  textSize(12);stroke(225);fill(225); 
  text(pt4_npt1 + " psia", 965, 553);
  
  fill(40); color(0);stroke(147, 39, 143);strokeWeight(2);
  rect(950,630,100,25, 10.0); //NPT2 BOX
  textSize(12);stroke(225);fill(225); 
  text(pt5_npt2 + " psia", 1040, 648);
  
  fill(40); color(0);stroke(131, 255, 20);strokeWeight(2);
  rect(1120,430,100,25, 10.0); //Load Cell BOX
  textSize(12);stroke(225);fill(225); 
  text(load_cell + " lbf", 1210, 448);
  
  fill(40); color(0);stroke(131, 255, 20);strokeWeight(2);
  rect(1120,430,100,25, 10.0); //Load Cell BOX
  textSize(12);stroke(225);fill(225); 
  text(load_cell + " lbf", 1210, 448);
  
  fill(40); color(0);stroke(131, 255, 20);strokeWeight(2);
  rect(390,520,100,25, 10.0); //Load Cell BOX
  textSize(12);stroke(225);fill(225); 
  text(scale + " lbs", 480, 538);
  
  fill(40); color(0);stroke(193, 39, 45);strokeWeight(2);
  rect(390,375,100,25, 10.0); //OTC1
  textSize(12);stroke(225);fill(225); 
  text(otc1 + " F", 480, 393);
  
  fill(40); color(0);stroke(193, 39, 45);strokeWeight(2);
  rect(1390,500,100,25, 10.0); //OTC3
  textSize(12);stroke(225);fill(225); 
  text(otc2 + " F", 1480, 518);
  
  fill(40); color(0);stroke(193, 39, 45);strokeWeight(2);
  rect(1240,410,100,25, 10.0); //OTC2
  textSize(12);stroke(225);fill(225); 
  text(otc3 + " F", 1330, 428);
  
  fill(40); color(0);stroke(193, 39, 45);strokeWeight(2);
  rect(1275,525,100,25, 10.0); //OTC4
  textSize(12);stroke(225);fill(225); 
  text(otc4 + " F", 1365, 543);
}



void updateStatusLEDs()
{
  //WSV2 Status LED
  if(ls1_wsv2_state == 1)
  {
    fill(131, 255, 20); stroke(225);strokeWeight(1);
    circle(wsv2_led_pos[0], wsv2_led_pos[1], 15); //Green status LED
  }
  else
  {
    fill(193, 39, 45); stroke(225);strokeWeight(1);
    circle(wsv2_led_pos[0], wsv2_led_pos[1], 15); //RED status LED
  }
  
  //OSV4 Status LED
  if(ls2_osv4_state == 1)
  {
    fill(131, 255, 20); stroke(225);strokeWeight(1);
    circle(osv4_led_pos[0], osv4_led_pos[1], 15); //Green status LED
  }
  else
  {
    fill(193, 39, 45); stroke(225);strokeWeight(1);
    circle(osv4_led_pos[0], osv4_led_pos[1], 15); //RED status LED
  }
  
  //OSV5 Status LED
  if(ls3_osv5_state == 1)
  {
    fill(131, 255, 20); stroke(225);strokeWeight(1);
    circle(osv5_led_pos[0], osv5_led_pos[1], 15); //Green status LED
  }
  else
  {
    fill(193, 39, 45); stroke(225);strokeWeight(1);
    circle(osv5_led_pos[0], osv5_led_pos[1], 15); //RED status LED
  }
  
  //NSV2 Status LED
  if(ls4_nsv2_state == 1)
  {
    fill(131, 255, 20); stroke(225);strokeWeight(1);
    circle(nsv2_led_pos[0], nsv2_led_pos[1], 15); //Green status LED
  }
  else
  {
    fill(193, 39, 45); stroke(225);strokeWeight(1);
    circle(nsv2_led_pos[0], nsv2_led_pos[1], 15); //RED status LED
  }
}



void setChartSettings()
{
  //OPT1
  opt1_linegraph.xLabel=" Time(sec) ";
  opt1_linegraph.yLabel="Pressure(psia)";
  opt1_linegraph.Title="Tank Pressure";  
  opt1_linegraph.xDiv=7;  
  opt1_linegraph.xMax=0; 
  opt1_linegraph.yMax=int(getPlotterConfigString("pt1mx")); 
  opt1_linegraph.yMin=int(getPlotterConfigString("pt1mn"));

  //OPT2
  opt2_linegraph.xLabel=" Time(sec) ";
  opt2_linegraph.yLabel="Pressure(psia)";
  opt2_linegraph.Title="Feed Pressure";  
  opt2_linegraph.xDiv=7;  
  opt2_linegraph.xMax=0; 
  opt2_linegraph.yMax=int(getPlotterConfigString("pt2mx")); 
  opt2_linegraph.yMin=int(getPlotterConfigString("pt2mn"));

  //OPT3
  opt3_linegraph.xLabel=" Time(sec) ";
  opt3_linegraph.yLabel="Pressure(psia)";
  opt3_linegraph.Title="Chamber Pressure";  
  opt3_linegraph.xDiv=7;  
  opt3_linegraph.xMax=0; 
  opt3_linegraph.yMax=int(getPlotterConfigString("pt3mx")); 
  opt3_linegraph.yMin=int(getPlotterConfigString("pt3mn"));

  //NPT1
  npt1_linegraph.xLabel=" Time(sec) ";
  npt1_linegraph.yLabel="Pressure(psia)";
  npt1_linegraph.Title="Nitrogen Line Pressure";  
  npt1_linegraph.xDiv=7;  
  npt1_linegraph.xMax=0; 
  npt1_linegraph.yMax=int(getPlotterConfigString("pt4mx")); 
  npt1_linegraph.yMin=int(getPlotterConfigString("pt4mn"));

  //NPT2
  npt2_linegraph.xLabel=" Time(sec) ";
  npt2_linegraph.yLabel="Pressure(psia)";
  npt2_linegraph.Title="Pneumatic Pressure";  
  npt2_linegraph.xDiv=7;  
  npt2_linegraph.xMax=0; 
  npt2_linegraph.yMax=int(getPlotterConfigString("pt5mx")); 
  npt2_linegraph.yMin=int(getPlotterConfigString("pt5mn"));


  //OTC1
  otc1_linegraph.xLabel=" Time(sec) ";
  otc1_linegraph.yLabel="Temperature(F)";
  otc1_linegraph.Title="Ice Bath Temperature";  
  otc1_linegraph.xDiv=7;  
  otc1_linegraph.xMax=0; 
  otc1_linegraph.yMax=int(getPlotterConfigString("tc1mx")); 
  otc1_linegraph.yMin=int(getPlotterConfigString("tc1mn"));

  //OTC2
  otc2_linegraph.xLabel=" Time(sec) ";
  otc2_linegraph.yLabel="Temperature(F)";
  otc2_linegraph.Title="Igniter Temperature";  
  otc2_linegraph.xDiv=7;  
  otc2_linegraph.xMax=0; 
  otc2_linegraph.yMax=int(getPlotterConfigString("tc2mx")); 
  otc2_linegraph.yMin=int(getPlotterConfigString("tc2mn"));

  //OTC3
  otc3_linegraph.xLabel=" Time(sec) ";
  otc3_linegraph.yLabel="Temperature(F)";
  otc3_linegraph.Title="Engine Casing Temperature 1";  
  otc3_linegraph.xDiv=7;  
  otc3_linegraph.xMax=0; 
  otc3_linegraph.yMax=int(getPlotterConfigString("tc3mx")); 
  otc3_linegraph.yMin=int(getPlotterConfigString("tc3mn"));

  //OTC4
  otc4_linegraph.xLabel=" Time(sec) ";
  otc4_linegraph.yLabel="Temperature(F)";
  otc4_linegraph.Title="Engine Casing Temperature 2";  
  otc4_linegraph.xDiv=7;  
  otc4_linegraph.xMax=0; 
  otc4_linegraph.yMax=int(getPlotterConfigString("tc4mx")); 
  otc4_linegraph.yMin=int(getPlotterConfigString("tc4mn"));
  
  //Load Cell
  lc_linegraph.xLabel=" Time(sec) ";
  lc_linegraph.yLabel="Thrust(lbf)";
  lc_linegraph.Title="Engine Thrust";  
  lc_linegraph.xDiv=7;  
  lc_linegraph.xMax=0; 
  lc_linegraph.yMax=int(getPlotterConfigString("lcmx")); 
  lc_linegraph.yMin=int(getPlotterConfigString("lcmn"));
  
  //Scale
  scale_linegraph.xLabel=" Time(sec) ";
  scale_linegraph.yLabel="Weight(lb)";
  scale_linegraph.Title="Tank Weight";  
  scale_linegraph.xDiv=7;  
  scale_linegraph.xMax=0; 
  scale_linegraph.yMax=int(getPlotterConfigString("sclmx")); 
  scale_linegraph.yMin=int(getPlotterConfigString("sclmn"));
}



// handle gui actions
void controlEvent(ControlEvent theEvent) 
{
  // If the Event was triggered from a toggle switch...
  if(theEvent.isAssignableFrom(Toggle.class))
  {
    //Update Slider Color
    if(theEvent.getName().equals("wsv2"))
    {
      processing_wsv2 = (int)(theEvent.getValue());
      if(processing_wsv2 == 1)
      {
        cp5.get(Toggle.class, "wsv2").setColorActive(color(131, 255, 20)); //Set slider to green color
      }
      else
      {
        cp5.get(Toggle.class, "wsv2").setColorActive(color(193, 39, 45)); //Set slider to red color
      }
    }
    if(theEvent.getName().equals("osv4"))
    {
      processing_osv4 = (int)(theEvent.getValue());
      if(processing_osv4 == 1)
      {
        cp5.get(Toggle.class, "osv4").setColorActive(color(131, 255, 20)); //Set slider to green color
      }
      else
      {
        cp5.get(Toggle.class, "osv4").setColorActive(color(193, 39, 45)); //Set slider to red color
      }
    }
    if(theEvent.getName().equals("osv5"))
    {
      processing_osv5 = (int)(theEvent.getValue());
      if(processing_osv5 == 1)
      {
        cp5.get(Toggle.class, "osv5").setColorActive(color(131, 255, 20)); //Set slider to green color
      }
      else
      {
        cp5.get(Toggle.class, "osv5").setColorActive(color(193, 39, 45)); //Set slider to red color
      }
    }
    if(theEvent.getName().equals("nsv2"))
    {
      processing_nsv2 = (int)(theEvent.getValue());
      if(processing_nsv2 == 1)
      {
        cp5.get(Toggle.class, "nsv2").setColorActive(color(131, 255, 20)); //Set slider to green color
      }
      else
      {
        cp5.get(Toggle.class, "nsv2").setColorActive(color(193, 39, 45)); //Set slider to red color
      }
    }
    if(theEvent.getName().equals("Press to Arm System"))
    {
      processing_arming = (int)(theEvent.getValue());
    }
    if(theEvent.getName().equals("Press to Ignite"))
    {
      processing_ignite = (int)(theEvent.getValue());
    }
    //Send over serial data
    serialPort.write("P5C:{" + processing_wsv2 + "," + processing_osv4 + "," + processing_osv5 + "," + processing_nsv2 + "," + processing_arming + "," + processing_ignite + "}\n");
  }
  if (theEvent.isAssignableFrom(Textfield.class) || theEvent.isAssignableFrom(Toggle.class) || theEvent.isAssignableFrom(Button.class)) {
    String parameter = theEvent.getName();
    String value = "";
    if (theEvent.isAssignableFrom(Textfield.class))
      value = theEvent.getStringValue();
    else if (theEvent.isAssignableFrom(Toggle.class) || theEvent.isAssignableFrom(Button.class))
      value = theEvent.getValue()+"";

    plotterConfigJSON.setString(parameter, value);
    saveJSONObject(plotterConfigJSON, topSketchPath+"/plotter_config.json");
  }
  setChartSettings();
}



// get gui settings from settings file
String getPlotterConfigString(String id) 
{
  String r = "";
  try {
    r = plotterConfigJSON.getString(id);
  } 
  catch (Exception e) {
    r = "";
  }
  return r;
}



void plotEachGraph()
{
  //OPT1
  for(int i = 0; i < opt1_linegraph_values.length; i++)
  {
    opt1_linegraph.GraphColor = graphColors[1]; // Set index to i if you plan on using more colors
    opt1_linegraph.LineGraph(linegraph_sample, opt1_linegraph_values[i]);
  }

  //OPT2
  for(int i = 0; i < opt2_linegraph_values.length; i++)
  {
    opt2_linegraph.GraphColor = graphColors[1]; // Set index to i if you plan on using more colors
    opt2_linegraph.LineGraph(linegraph_sample, opt2_linegraph_values[i]);
  }

  //OPT3
  for(int i = 0; i < opt3_linegraph_values.length; i++)
  {
    opt3_linegraph.GraphColor = graphColors[1]; // Set index to i if you plan on using more colors
    opt3_linegraph.LineGraph(linegraph_sample, opt3_linegraph_values[i]);
  }

  //NPT1
  for(int i = 0; i < npt1_linegraph_values.length; i++)
  {
    npt1_linegraph.GraphColor = graphColors[1]; // Set index to i if you plan on using more colors
    npt1_linegraph.LineGraph(linegraph_sample, npt1_linegraph_values[i]);
  }

  //NPT2
  for(int i = 0; i < npt2_linegraph_values.length; i++)
  {
    npt2_linegraph.GraphColor = graphColors[1]; // Set index to i if you plan on using more colors
    npt2_linegraph.LineGraph(linegraph_sample, npt2_linegraph_values[i]);
  }


  //OTC1
  for(int i = 0; i < otc1_linegraph_values.length; i++)
  {
    otc1_linegraph.GraphColor = graphColors[2]; // Set index to i if you plan on using more colors
    otc1_linegraph.LineGraph(linegraph_sample, otc1_linegraph_values[i]);
  }

  //OTC2
  for(int i = 0; i < otc2_linegraph_values.length; i++)
  {
    otc2_linegraph.GraphColor = graphColors[2]; // Set index to i if you plan on using more colors
    otc2_linegraph.LineGraph(linegraph_sample, otc2_linegraph_values[i]);
  }

  //OTC3
  for(int i = 0; i < otc3_linegraph_values.length; i++)
  {
    otc3_linegraph.GraphColor = graphColors[2]; // Set index to i if you plan on using more colors
    otc3_linegraph.LineGraph(linegraph_sample, otc3_linegraph_values[i]);
  }

  //OTC4
  for(int i = 0; i < otc4_linegraph_values.length; i++)
  {
    otc4_linegraph.GraphColor = graphColors[2]; // Set index to i if you plan on using more colors
    otc4_linegraph.LineGraph(linegraph_sample, otc4_linegraph_values[i]);
  }
  
  //Load Cell
  for(int i = 0; i < lc_linegraph_values.length; i++)
  {
    lc_linegraph.GraphColor = graphColors[0]; // Set index to i if you plan on using more colors
    lc_linegraph.LineGraph(linegraph_sample, lc_linegraph_values[i]);
  }
  
  //Scale
  for(int i = 0; i < scale_linegraph_values.length; i++)
  {
    scale_linegraph.GraphColor = graphColors[0]; // Set index to i if you plan on using more colors
    scale_linegraph.LineGraph(linegraph_sample, scale_linegraph_values[i]);
  }
}



void drawEachLineGraphAxis()
{
  opt1_linegraph.DrawAxis();
  opt2_linegraph.DrawAxis();
  opt3_linegraph.DrawAxis();
  npt1_linegraph.DrawAxis();
  npt2_linegraph.DrawAxis();

  otc1_linegraph.DrawAxis();
  otc2_linegraph.DrawAxis();
  otc3_linegraph.DrawAxis();
  otc4_linegraph.DrawAxis();
  
  lc_linegraph.DrawAxis();
  scale_linegraph.DrawAxis();
}



void updateEachLineGraphMatrix()
{
  // Updating OPT1
  try
  {
    for( i = 0; i < opt1_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < opt1_linegraph_values[i].length-1; col++)
      {
        opt1_linegraph_values[i][col] = opt1_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      opt1_linegraph_values[i][opt1_linegraph_values[i].length-1] = pt1_opt1;
    }
  }catch(Exception e){System.out.println("OPT1 Failed to update!");}

  // Updating OPT2
  try
  {
    for( i = 0; i < opt2_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < opt2_linegraph_values[i].length-1; col++)
      {
        opt2_linegraph_values[i][col] = opt2_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      opt2_linegraph_values[i][opt2_linegraph_values[i].length-1] = pt2_opt2;
    }
  }catch(Exception e){System.out.println("OPT2 Failed to update!");}

  // Updating OPT3
  try
  {

    for( i = 0; i < opt3_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < opt3_linegraph_values[i].length-1; col++)
      {
        opt3_linegraph_values[i][col] = opt3_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      opt3_linegraph_values[i][opt3_linegraph_values[i].length-1] = pt3_opt3;
    }
  }catch(Exception e){System.out.println("OPT3 Failed to update!");}

  // Updating NPT1
  try
  {
    for( i = 0; i < npt1_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < npt1_linegraph_values[i].length-1; col++)
      {
        npt1_linegraph_values[i][col] = npt1_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      npt1_linegraph_values[i][npt1_linegraph_values[i].length-1] = pt4_npt1;
    }
  }catch(Exception e){System.out.println("NPT1 Failed to update!");}

  // Updating NPT2
  try
  {
    for( i = 0; i < npt2_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < npt2_linegraph_values[i].length-1; col++)
      {
        npt2_linegraph_values[i][col] = npt2_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      npt2_linegraph_values[i][npt2_linegraph_values[i].length-1] = pt5_npt2;
    }
  }catch(Exception e){System.out.println("NPT2 Failed to update!");}


  // Updating OTC1
  try
  {
    for( i = 0; i < otc1_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < otc1_linegraph_values[i].length-1; col++)
      {
        otc1_linegraph_values[i][col] = otc1_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      otc1_linegraph_values[i][otc1_linegraph_values[i].length-1] = otc1;
    }
  }catch(Exception e){System.out.println("OTC1 Failed to update!");}  
  
  // Updating OTC2
  try
  {
    for( i = 0; i < otc2_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < otc2_linegraph_values[i].length-1; col++)
      {
        otc2_linegraph_values[i][col] = otc2_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      otc2_linegraph_values[i][otc2_linegraph_values[i].length-1] = otc2;
    }
  }catch(Exception e){System.out.println("OTC2 Failed to update!");}  
  
  // Updating OTC3
  try
  {
    for( i = 0; i < otc3_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < otc3_linegraph_values[i].length-1; col++)
      {
        otc3_linegraph_values[i][col] = otc3_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      otc3_linegraph_values[i][otc3_linegraph_values[i].length-1] = otc3;
    }
  }catch(Exception e){System.out.println("OTC3 Failed to update!");}

  // Updating OTC4
  try
  {
    for( i = 0; i < otc4_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < otc4_linegraph_values[i].length-1; col++)
      {
        otc4_linegraph_values[i][col] = otc4_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      otc4_linegraph_values[i][otc4_linegraph_values[i].length-1] = otc4;
    }
  }catch(Exception e){System.out.println("OTC4 Failed to update!");}  
  
  // Updating Load Cell
  try
  {
    for( i = 0; i < lc_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < lc_linegraph_values[i].length-1; col++)
      {
        lc_linegraph_values[i][col] = lc_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      lc_linegraph_values[i][lc_linegraph_values[i].length-1] = load_cell;
    }
  }catch(Exception e){System.out.println("Load Cell Failed to update!");}  
  
  // Updating Scale
  try
  {
    for( i = 0; i < scale_linegraph_values.length; i++)
    {
      //Shifting each data value back one slot
      for(int col = 0; col < scale_linegraph_values[i].length-1; col++)
      {
        scale_linegraph_values[i][col] = scale_linegraph_values[i][col+1];
      }

      //Updating the matrix with the latest value
      scale_linegraph_values[i][scale_linegraph_values[i].length-1] = scale;
    }
  }catch(Exception e){System.out.println("Scale Failed to update!");}  
}



void parseAndUpdateData(String myString)
{
  String currentString = myString;
    String subarray = "";
    
    try
    {
      subarray = currentString.substring(myString.indexOf("{")+1, myString.indexOf(","));
    }
    catch (Exception e)
    {
      System.out.println("A data packet was unable to be parsed!");
      return;
    }
  //Serial.print(subarray);
    sol1_wsv2_state = Integer.parseInt(subarray);
    currentString = currentString.substring(myString.indexOf(",")+1);
    
    subarray = currentString.substring(0, currentString.indexOf(","));
    //Serial.print(subarray);
    sol2_osv4_state = Integer.parseInt(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf(","));
    //Serial.print(subarray);
    sol3_osv5_state = Integer.parseInt(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf(","));
    //Serial.println(subarray);
    sol4_nsv2_state = Integer.parseInt(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
  //-------------------------------------------------------------
    subarray = currentString.substring(0, currentString.indexOf(","));
    igniter_state = Integer.parseInt(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
  //-------------------------------------------------------------
    subarray = currentString.substring(0, currentString.indexOf(","));
    ls1_wsv2_state = Integer.parseInt(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf(","));
    ls2_osv4_state = Integer.parseInt(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf(","));
    ls3_osv5_state = Integer.parseInt(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf(","));
    ls4_nsv2_state = Integer.parseInt(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
  //-------------------------------------------------------------
    subarray = currentString.substring(0, currentString.indexOf(","));
    otc1 = Float.parseFloat(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
    
    subarray = currentString.substring(0, currentString.indexOf(","));
    otc2 = Float.parseFloat(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf(","));
    otc3 = Float.parseFloat(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf(","));
    otc4 = Float.parseFloat(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
  //-------------------------------------------------------------
    subarray = currentString.substring(0, currentString.indexOf(","));
    pt1_opt1 = Float.parseFloat(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf(","));
    pt2_opt2 = Float.parseFloat(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf(","));
    pt3_opt3 = Float.parseFloat(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf(","));
    pt4_npt1 = Float.parseFloat(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf(","));
    pt5_npt2 = Float.parseFloat(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
  //-------------------------------------------------------------
    subarray = currentString.substring(0, currentString.indexOf(","));
    load_cell = Float.parseFloat(subarray);
    currentString = currentString.substring(currentString.indexOf(",")+1);
  
    subarray = currentString.substring(0, currentString.indexOf("}"));
    scale = Float.parseFloat(subarray);
}



void createActuatorSliders()
{
    cp5.addToggle("wsv2").setPosition(wsv2_led_pos[0]-45, wsv2_led_pos[1]+30).setValue(0).setMode(ControlP5.SWITCH).setColorActive(color(193, 39, 45));
    cp5.addToggle("osv4").setPosition(osv4_led_pos[0]-10, osv4_led_pos[1]+30).setValue(0).setMode(ControlP5.SWITCH).setColorActive(color(193, 39, 45));
    cp5.addToggle("osv5").setPosition(osv5_led_pos[0]-47, osv5_led_pos[1]+30).setValue(0).setMode(ControlP5.SWITCH).setColorActive(color(193, 39, 45));
    cp5.addToggle("nsv2").setPosition(nsv2_led_pos[0]-44, nsv2_led_pos[1]+31).setValue(0).setMode(ControlP5.SWITCH).setColorActive(color(193, 39, 45));
    
    cp5.addToggle("Press to Arm System").setCaptionLabel("Press to Arm System").setPosition(1350,725).setValue(0).setSize(160,100).setColorActive(color(193,39,45));
    cp5.addToggle("Press to Ignite").setCaptionLabel("Press to Ignite").setPosition(1350,850).setValue(0).setSize(160,100).setColorActive(color(193,39,45));
}



void createMinMaxRange()
{
    cp5.addTextfield("pt1mx").setPosition(opt1_pos[0]-55, opt1_pos[1]-30).setText(getPlotterConfigString("pt1mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("pt1mn").setPosition(opt1_pos[0]-55, opt1_pos[1]+190).setText(getPlotterConfigString("pt1mn")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("pt2mx").setPosition(opt2_pos[0]-55, opt2_pos[1]-30).setText(getPlotterConfigString("pt2mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("pt2mn").setPosition(opt2_pos[0]-55, opt2_pos[1]+190).setText(getPlotterConfigString("pt2mn")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("pt3mx").setPosition(opt3_pos[0]-55, opt3_pos[1]-30).setText(getPlotterConfigString("pt3mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("pt3mn").setPosition(opt3_pos[0]-55, opt3_pos[1]+190).setText(getPlotterConfigString("pt3mn")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("pt4mx").setPosition(npt1_pos[0]-55, npt1_pos[1]-30).setText(getPlotterConfigString("pt4mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("pt4mn").setPosition(npt1_pos[0]-55, npt1_pos[1]+190).setText(getPlotterConfigString("pt4mn")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("pt5mx").setPosition(npt2_pos[0]-55, npt2_pos[1]-30).setText(getPlotterConfigString("pt5mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("pt5mn").setPosition(npt2_pos[0]-55, npt2_pos[1]+190).setText(getPlotterConfigString("pt5mn")).setWidth(40).setAutoClear(false);    

    cp5.addTextfield("tc1mx").setPosition(otc1_pos[0]-55, otc1_pos[1]-30).setText(getPlotterConfigString("tc1mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("tc1mn").setPosition(otc1_pos[0]-55, otc1_pos[1]+190).setText(getPlotterConfigString("tc1mn")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("tc2mx").setPosition(otc2_pos[0]-55, otc2_pos[1]-30).setText(getPlotterConfigString("tc2mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("tc2mn").setPosition(otc2_pos[0]-55, otc2_pos[1]+190).setText(getPlotterConfigString("tc2mn")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("tc3mx").setPosition(otc3_pos[0]-55, otc3_pos[1]-30).setText(getPlotterConfigString("tc3mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("tc3mn").setPosition(otc3_pos[0]-55, otc3_pos[1]+190).setText(getPlotterConfigString("tc3mn")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("tc4mx").setPosition(otc4_pos[0]-55, otc4_pos[1]-30).setText(getPlotterConfigString("tc4mx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("tc4mn").setPosition(otc4_pos[0]-55, otc4_pos[1]+190).setText(getPlotterConfigString("tc4mn")).setWidth(40).setAutoClear(false);
    
    cp5.addTextfield("lcmx").setPosition(lc_pos[0]-55, lc_pos[1]-30).setText(getPlotterConfigString("lcmx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("lcmn").setPosition(lc_pos[0]-55, lc_pos[1]+190).setText(getPlotterConfigString("lcmn")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("sclmx").setPosition(scale_pos[0]-55, scale_pos[1]-30).setText(getPlotterConfigString("sclmx")).setWidth(40).setAutoClear(false);
    cp5.addTextfield("sclmn").setPosition(scale_pos[0]-55, scale_pos[1]+190).setText(getPlotterConfigString("sclmn")).setWidth(40).setAutoClear(false);
}



void clearAllDataValues()
{
  // Clearing OPT1
  for(int row = 0; row < opt1_linegraph_values.length; row++)
  {
    for (int col = 0; col < opt1_linegraph_values[0].length ; col++) {
      
      if(row == 0) {linegraph_sample[col] = col;}
      else
      {
        opt1_linegraph_values[row][col] = 0;
      }
    }
  }

  // Clearing OPT2
  for(int row = 0; row < opt2_linegraph_values.length; row++)
  {
    for (int col = 0; col < opt2_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        opt2_linegraph_values[row][col] = 0;
      //}
    }
  }

  // Clearing OPT3
  for(int row = 0; row < opt3_linegraph_values.length; row++)
  {
    for (int col = 0; col < opt3_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        opt3_linegraph_values[row][col] = 0;
      //}
    }
  }

  // Clearing NPT1
  for(int row = 0; row < npt1_linegraph_values.length; row++)
  {
    for (int col = 0; col < npt1_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        npt1_linegraph_values[row][col] = 0;
      //}
    }
  }

  // Clearing NPT2
  for(int row = 0; row < npt2_linegraph_values.length; row++)
  {
    for (int col = 0; col < npt2_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        npt2_linegraph_values[row][col] = 0;
      //}
    }
  }

  // Clearing OTC1
  for(int row = 0; row < otc1_linegraph_values.length; row++)
  {
    for (int col = 0; col < otc1_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        otc1_linegraph_values[row][col] = 0;
      //}
    }
  }

  // Clearing OTC2
  for(int row = 0; row < otc2_linegraph_values.length; row++)
  {
    for (int col = 0; col < otc2_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        otc2_linegraph_values[row][col] = 0;
      //}
    }
  }

  // Clearing OTC3
  for(int row = 0; row < otc3_linegraph_values.length; row++)
  {
    for (int col = 0; col < otc3_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        otc3_linegraph_values[row][col] = 0;
      //}
    }
  }

  // Clearing OTC4
  for(int row = 0; row < otc4_linegraph_values.length; row++)
  {
    for (int col = 0; col < otc4_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        otc4_linegraph_values[row][col] = 0;
      //}
    }
  }
  
    // Clearing Load Cell
  for(int row = 0; row < lc_linegraph_values.length; row++)
  {
    for (int col = 0; col < lc_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        lc_linegraph_values[row][col] = 0;
      //}
    }
  }
  
    // Clearing Scale
  for(int row = 0; row < scale_linegraph_values.length; row++)
  {
    for (int col = 0; col < scale_linegraph_values[0].length ; col++) {
      
      //if(row == 0) {linegraph_sample[col] = col;}
      //else
      //{
        scale_linegraph_values[row][col] = 0;
      //}
    }
  }
}
