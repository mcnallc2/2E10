import controlP5.*; //controlP5 library
import processing.serial.*; //serial communication library

Serial port; //serial object
ControlP5 cp5; //controlP5 object

String feedback; //variable to store inputs from Xbee

void setup() 
{
  size(640,480); //sketch size
  background(255); //background colour
  cp5 = new ControlP5(this); //assign new object
  String portName = Serial.list()[0]; //store the available ports for serial object
  printArray(Serial.list()); //print list of available ports
  port = new Serial(this, portName, 9600); //assign new object
  port.bufferUntil( 10 ); //buffer every new line 
  
  //create a button which will print and send Go when pressed
  cp5.addButton("Go")
     .setPosition(0,450)
     .setSize(320,45)
     .setValue(0)
     .setColorBackground(color(0,255,0))
     .setColorForeground(color(0,100,0))
     .setColorCaptionLabel(color(0,0,0))
     ;
     
  //create a button which will print and send Stop when pressed
  cp5.addButton("Stop")
    .setPosition(320,450)
    .setSize(320,45)
    .setValue(100)
    .setColorBackground(color(255,0,0))
    .setColorForeground(color(100,0,0))
    .setColorCaptionLabel(color(0,0,0))
    ;
}

void draw() 
{

}

//this function is called everytime a button is pressed
public void controlEvent(ControlEvent theEvent)
{
  println(theEvent.getController().getName()); //print the name of button / Go or Stop
  port.write(theEvent.getController().getName()); //send the name of button to Arduino via Xbee
  port.write("\n"); //send a new line to Arduino via Xbee
}

//this function is called everytime the arduino is sending something
void serialEvent(Serial p)
{
  feedback = p.readStringUntil('\n'); //store the input until a new line
  feedback = trim(feedback); //trim the input i.e get rid of whitespace which is not needed
  println(feedback); //print the input to the console
}