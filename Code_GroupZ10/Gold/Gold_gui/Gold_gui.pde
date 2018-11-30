import processing.serial.*; //serial communication library

Serial port; //serial object
PFont font; //PFont object

String RPM; //variable to store the RPM

void setup() 
{
  size(640,480); //sketch size
  background(255); //background colour

  font = createFont("Arial", 40); //create the font object and assign the font and character size
  textFont(font, 40); //setup the text with the assigned font and character size
}

void draw() 
{

}

//this function is called everytime the arduino is sending something
void serialEvent(Serial p)
{
  RPM = p.readStringUntil('\n'); //store the input until a new line
  RPM = trim(RPM); //trim the input i.e get rid of whitespace which is not needed
  println(RPM); //print the input to the console
  
  background(255); //fill the background
  text(RPM,100,100); //print the RPM onto the sketch
}