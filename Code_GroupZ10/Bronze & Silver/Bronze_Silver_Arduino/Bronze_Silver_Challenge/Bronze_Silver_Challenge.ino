//Libraries
#include <SPI.h>
#include <Pixy.h>

//Defining our pins
#define CTRL_PIN 3
#define US_TRIGGER_PIN 9
#define US_ECHO_PIN 8
#define IR_PIN 2
#define LEFT_M 4
#define LEFT_O 5
#define RIGHT_O 6
#define RIGHT_M 7
#define BUZZER 10

//Creating our pixy object
Pixy pixy;

//Declaring our global variables
int speed;
int distance;
int poll;
bool trigger = false;
bool yellow = false;
bool cornering = false;
bool ignore = false;
String h;
volatile boolean ir_state = false;
int avg_timer;

void setup() {

  Serial.begin(9600);
  
  //Declaring inputs and outputs
  pinMode(US_TRIGGER_PIN, OUTPUT);
  pinMode(US_ECHO_PIN, INPUT);
  pinMode(CTRL_PIN,OUTPUT);
  pinMode(IR_PIN,INPUT);
  pinMode(LEFT_O,OUTPUT);
  pinMode(BUZZER,OUTPUT);

//  pinMode(LEFT_M,OUTPUT);
//  pinMode(LEFT_O,OUTPUT);
//  pinMode(RIGHT_O,OUTPUT);
//  pinMode(RIGHT_M,OUTPUT);
//  analogWrite(LEFT_M,0);
//  analogWrite(LEFT_O,0);
//  analogWrite(RIGHT_O,0);
//  analogWrite(RIGHT_M,0);

  Serial.print("+++");
  Serial.println("ATID 3310, CH C, CN");
  analogWrite(CTRL_PIN,0);
  digitalWrite(BUZZER, LOW);

  //Initiating Pixy
  pixy.init();

  //Creating Gantry Interupt
  attachInterrupt(digitalPinToInterrupt(IR_PIN),IR_ISR,RISING);
}

void loop()
{
  int max_pulse = 0;
  int timer;

  //Pixy Monitor
  
  poll++;
  
  //Polling the Pixy
  if(poll % 20 == 0){
    
    //Finding the amount of blocks the pixy is seeing
    int blocks = pixy.getBlocks();

    //Looping through every block checking for signitures
    for (int i = 0 ; i < blocks; i++){

      //Checking for sig 1
      if(pixy.blocks[i].signature == 1){
        cornering = true;
      }
      //Checking for sig 2
      if(pixy.blocks[i].signature == 2){
        cornering = false;
      }
      //Checking for sig 3
      if(pixy.blocks[i].signature == 3){
        yellow = true;
        
        //Calling take_turn function
        take_turn();
      }
    }
  }

  //Ultrasonic Detection

  //If nothing is detected AND the input is 'Go' we run the buggy
  if(!ultraSonicDet() && h == "Go"){
    run(); 
  }

  //If something is detected OR the input is 'Stop' we stop the buggy
  if(ultraSonicDet() || h == "Stop"){
    stop();
  }

  //If the input is 'Ignore' we ignore the yellow marking
  if(h == "Ignore"){
    Ignore();
  }
  


  //Gantry Detection

  //Checks if an interupt occured
  if (ir_state == true){

    //loops 5 times
    for(int j = 0; j < 5; j++){

      //Checks how long the Gantry pulse was
      timer = pulseIn(IR_PIN,LOW);

      //Updates our max pulse
      if(timer > max_pulse){
        max_pulse = timer;
      }
    }

    //Checks is pulse length corresponds with Gantry 1
    if(max_pulse >= 500 && max_pulse < 1500){

      //Writes out to Supervisior
      Serial.write("Passing Gantry 1");
      Serial.write("\n");
    }

    //Checks is pulse length corresponds with Gantry 2
    if(max_pulse >= 1500 && max_pulse < 2500){

      //Writes out to Supervisior
      Serial.write("Passing Gantry 2");
      Serial.write("\n");
    }

    //Checks is pulse length corresponds with Gantry 3
    if(max_pulse >= 2500 && max_pulse <= 3500){

      //Writes out to Supervisior
      Serial.write("Passing Gantry 3");
      Serial.write("\n");
    }

    //Resets interupt variable
    ir_state = false;
  }
}

//Checks if an event occured with the supervisor
void serialEvent(){

  //Checks for bytes in the serial port
  if(Serial.available() > 0){

    //Reads in string and until end line
    h = Serial.readStringUntil('\n');
    //Trims the string
    h.trim();

    //Prints string to the serial monitor
    Serial.print(h);
    Serial.print("\n");
    trigger = true;
    
  }
}

//Function detecting obstacles
bool ultraSonicDet()
{
  long duration; 

  //Sets trigger to low
  digitalWrite(US_TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  //Sets trigger to high for 10 micro seconds sending ultrasonic waves
  digitalWrite(US_TRIGGER_PIN, HIGH);
  delayMicroseconds(10);

  //Sets trigger back to low
  digitalWrite(US_TRIGGER_PIN, LOW);

  //Measures the pulse duration of echoed waves
  duration = pulseIn(US_ECHO_PIN, HIGH);

  //converts this duration to a distance in cm
  distance = duration*(0.034/2);

  //If object is within 30cm returns true and turns on buzzer
  if(distance < 30){
    digitalWrite(BUZZER, HIGH);
    return true;
  }

  //returns false and turns off buzzer
  digitalWrite(BUZZER, LOW);
  return false;
}

//ISR function for the Gantry Inrterupt
void IR_ISR(){

  //Changes variable
  ir_state = true;
}

//starts the buggy at different speeds
void run()
{
  //if the pixy has seen blue
  if(cornering == true){

    //sets speed to 100 and send this to motors
    speed = 100;
    analogWrite(CTRL_PIN, speed);

    //prevents spamming
    if(trigger){
      Serial.write("Cornering");
      Serial.write("\n");
      trigger = false;
    }
  }

  //if the pixy has not seen blue or has seen green
  else{

    //sets speed to 150 and sends this to motors
    speed = 150;
    analogWrite(CTRL_PIN, speed);

    //prevents spamming
    if(trigger){
      Serial.write("Running");
      Serial.write("\n");
      trigger = false;
    }
  }
}

//stops the buggy
void stop()
{
  //sets spend to 0 and send this to motors
  speed = 0;
  analogWrite(CTRL_PIN,speed);

  //prents spamming
  if(trigger){
    Serial.write("Stopping");
    Serial.write("\n");
    trigger = false;
  }
}

//called if the pixy sees yellow
void take_turn(){

  //if were not ignoring yellow
  if(!ignore){
    //turn on overwrite for 600 milliseconds
    digitalWrite(LEFT_O, HIGH);
    delay(600);
    digitalWrite(LEFT_O, LOW);

    //prevents spamming
    if(yellow){
      Serial.write("I see Yellow");
      Serial.write("\n");
      yellow = false;
    }
  }

  //if we are ignoring
  else{

    //we ignore and resets ignore variable
    ignore = false;
  }
}

//called if we are ignoring
void Ignore(){
  
  //changes ignore variable
  ignore = !ignore;
}



