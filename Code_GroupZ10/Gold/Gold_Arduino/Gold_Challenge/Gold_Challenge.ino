//Libraies
#include <SPI.h>
#include <Pixy.h>

//Defining pins
#define LEFT_M_M   4
#define LEFT_M     5
#define RIGHT_M    6
#define RIGHT_M_M  7
#define US_ECHO_PIN 8
#define US_TRIGGER_PIN 9
#define LEFT_E     A5
#define RIGHT_E    A4
#define LEFT_E_O   A3
#define RIGHT_E_O  A2
#define WHEEL_SPEED_SENSOR A1

//Creating pixy object
Pixy pixy;

//Declaring global variables
double L_Error;
double R_Error;
double LMP;
double RMP;

double Left_Motor;
double Right_Motor;

double Z1;
double Z2;
double Z3;
double Z0;

int RPM;
int i = 0;
int j = 0;
unsigned long Prev_Time; // To store time
unsigned long Duration; // To store time difference
boolean Current_State; // Current state of IR input
boolean Prev_State; // State of IR sensor in previous loop

bool slower = false;

int curr;
int change;
int prev = 0;

void setup() {

  //Sets up serial port
  Serial.begin(9600);

  //Sets pinmodes
  pinMode(LEFT_M_M, OUTPUT);
  pinMode(LEFT_M, OUTPUT);
  pinMode(RIGHT_M, OUTPUT);
  pinMode(RIGHT_M_M, OUTPUT);
  pinMode(LEFT_E, INPUT);
  pinMode(RIGHT_E, INPUT);
  pinMode(LEFT_E_O, INPUT);
  pinMode(RIGHT_E_O, INPUT);
  pinMode(WHEEL_SPEED_SENSOR,INPUT);
  pinMode(US_TRIGGER_PIN, OUTPUT);
  pinMode(US_ECHO_PIN, INPUT);

  //Stops motors
  digitalWrite(LEFT_M_M, LOW);
  digitalWrite(LEFT_M, LOW);
  digitalWrite(RIGHT_M, LOW);
  digitalWrite(RIGHT_M_M, LOW);

  //Initiate pixy
  pixy.init();

  //Sets max motor powers
  LMP = 200;
  RMP = 200;

  //Sets the magnitude of each error
  Z0 = 0;
  Z1 = (LMP / 100) * 70;
  Z2 = (LMP / 100) * 200;
  Z3 = (LMP / 100) * 130;

  
  Prev_Time = 0;
  Prev_State = LOW; 


}

void loop(){

  //Checks the eyes for a positional error
  Check_Eyes();

  //Polls the pixy to check for signs (not used)
  
//  j++;
//  if(j % 25 == 0){
//    Pixy_Control();
//  }

  //changes max speed for pixy signs
  if(slower){
    //slower max speeds
    LMP = 185;
    RMP = 185;
  }
  else{
    //faster max speeds
    LMP = 210;
    RMP = 210;
  }

  //CALCULATE OUR POWER TO OUR MOTORS

  //if object is detected 
  if(ultraSonicDet()){
    
    //speed is set to zero
    Left_Motor = 0;
    Right_Motor = 0;
  }

  //change the motor speeds using the corresponding errors
  else{
    Left_Motor = LMP - L_Error;
    Right_Motor = RMP - R_Error;
  }
  
  //CHECK IF REVERSE IS REQUIRED

  //send motor speeds to motors using reverse
  if(Left_Motor < -50){
    
    //slows down the reverse speed using forward speed
    analogWrite(LEFT_M, 125);
    digitalWrite(LEFT_M_M, HIGH);
    delay(10);
  }

  //send motor speeds to motors using reverse
  else if(Left_Motor < 0){
    
    //slows down the reverse speed using forward speed
    analogWrite(LEFT_M, 100);
    digitalWrite(LEFT_M_M, HIGH);
    delay(10);
  }

  //send motor speeds to motors without using reverse
  else{
    analogWrite(LEFT_M_M, 0);
    analogWrite(LEFT_M, Left_Motor);
  }

  //The above is repeated for the right motor
  
  if(Right_Motor < -50){
    analogWrite(RIGHT_M, 125);
    digitalWrite(RIGHT_M_M, HIGH);
  }
  else if(Right_Motor < 0){
    analogWrite(RIGHT_M, 100);
    digitalWrite(RIGHT_M_M, HIGH);
  }
  else{
    analogWrite(RIGHT_M_M, 0);
    analogWrite(RIGHT_M, Right_Motor);
  }

  //The RPM is calculated
  Calculate_RPM();

  //Polling the RPM
  i++;
  if(i % 25 == 0){

    //Sends RPM value supervisior and serial monitor
    Serial.println(RPM);
    Serial.write(RPM);
  }

//  curr = millis(); 
//  change = curr - prev;
//  Serial.println(change);
//  prev = curr;

}

//Function that checks eyes and sets errors
void Check_Eyes(){

  //If the eyes see all black
  if(digitalRead(LEFT_E) == 1 && digitalRead(LEFT_E_O) == 1){
    //Sets error
    L_Error = Z0;
//    analogWrite(LEFT_M_M, 0);
//    analogWrite(LEFT_M, 230);
  }
  if(digitalRead(RIGHT_E) == 1 && digitalRead(RIGHT_E_O) == 1){
    R_Error = Z0;
//    analogWrite(RIGHT_M_M, 0);
//    analogWrite(RIGHT_M, 230);
  }


  //If only the inner eyes see whites
  if(digitalRead(LEFT_E) == 0 && digitalRead(LEFT_E_O) == 1){
    //Sets error
    L_Error = Z1;
//    analogWrite(LEFT_M_M, 0);
//    analogWrite(LEFT_M, 60);
  }
  if(digitalRead(RIGHT_E) == 0 && digitalRead(RIGHT_E_O) == 1){
    R_Error = Z1;
//    analogWrite(RIGHT_M_M, 0);
//    analogWrite(RIGHT_M, 60);
  }


  //If the outer eyes only see white
  if(digitalRead(LEFT_E_O) == 0 && digitalRead(LEFT_E) == 1){
    //Sets error
    L_Error = Z3;
//    analogWrite(LEFT_M, 0);
//    analogWrite(LEFT_M_M, 255);
  }
  if(digitalRead(RIGHT_E_O) == 0 && digitalRead(RIGHT_E) == 1){
    R_Error = Z3;
//    analogWrite(RIGHT_M, 0);
//    analogWrite(RIGHT_M_M, 255);
  }


  //If the inner eyes and the outer eyes see white
  if(digitalRead(LEFT_E) == 0 && digitalRead(LEFT_E_O) == 0){
    //Sets error
    L_Error = Z2;
//    analogWrite(LEFT_M_M, 0);
//    analogWrite(LEFT_M, 0);
  }
  if(digitalRead(RIGHT_E) == 0 && digitalRead(RIGHT_E_O) == 0){
    R_Error = Z2;
//    analogWrite(RIGHT_M_M, 0);
//    analogWrite(RIGHT_M, 0);
  }
}

//Function the calculates RPM
void Calculate_RPM(){

  //Checks state of wheel speed sensor
  Current_State = digitalRead(WHEEL_SPEED_SENSOR);

  //If the state has changed
  if( Prev_State != Current_State){
     //If input only changes from LOW to HIGH     
     if( Current_State == HIGH ){                  

         //Calculates the length of a revolution in microseconds
         Duration = ( micros() - Prev_Time );
         //Calculates RPM (rpm = (1/ revolution length) *1000*1000*60)      
         RPM = (60000000 / Duration);
         //Sets previous time for next loop              
         Prev_Time = micros();                 
       }
  }
  //Sets current state to previous state for next loop
  Prev_State = Current_State;

  //Serial.println(RPM);
}

//Function for detecting objects
bool ultraSonicDet(){
  
  long Pulse;
  int Obstruction; 
  
  //Sets trigger pin to low
  digitalWrite(US_TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  //Sets trigger to high for 10 microseconds sending waves
  digitalWrite(US_TRIGGER_PIN, HIGH);
  delayMicroseconds(10);

  //Sets trigger back to low
  digitalWrite(US_TRIGGER_PIN, LOW);

  //Finds pusle length of echoed waves
  Pulse = pulseIn(US_ECHO_PIN, HIGH);

  //Calculates obstruction distance
  Obstruction = Pulse*(0.034/2);

  //if the obstruction is with 10cm return true
  if(Obstruction < 10){
    return true;
  }
  //return false
  return false;
}

//Checks for signs (not used)
void Pixy_Control(){

  //Checks how many blocks there are
  int blocks = pixy.getBlocks();

  //loops through all these blocks
  for (int i = 0 ; i < blocks; i++){

    //if we see sig 1(blue) go slower
    if(pixy.blocks[i].signature == 1){
      slower = true;
    }
    //if we see sig 2(green) go faster again
    if(pixy.blocks[i].signature == 2){
      slower = false;
    }
  }
}
