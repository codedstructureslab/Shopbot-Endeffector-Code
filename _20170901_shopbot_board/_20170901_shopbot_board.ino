// bugs: when you call bolt, sometimes it throws in a jaws open immediately before
//before starting program - put gipper state into ungripped

//test update github - delete later

#include <Servo.h>
#ifndef ARDPRINTF
#define ARDPRINTF
#define ARDBUFFER 16
#include <stdarg.h>
#include <Arduino.h>

// mount the servos
Servo stinger_servo;
Servo jaw_servo;


 ////////////////////////////////////////////////////////
 ///////////Set all the variables and flags /////////////
 ////////////////////////////////////////////////////////

// Output channels from the Shopbot
int shopbot_OP2 = A0;
int shopbot_OP3 = A1;
int shopbot_OP5 = A2;
int shopbot_OP6 = A3;
int shopbot_OP7 = A7;
float shopbot_OP2_value = 0;
float shopbot_OP3_value = 0; 
float shopbot_OP5_value = 0; 
float shopbot_OP6_value = 0;
float shopbot_OP7_value = 0;

// flags to run the programs
bool program_1 = false;
bool program_2 = false;
bool program_3 = false;
bool program_4 = false;
bool program_5 = false;
bool program_6 = false;
bool program_7 = false;
bool program_8 = false;
bool program_9 = false;


// set flags for end effector opposite to what they should be at the start
// not using these for now, setting the program_1 flags above seems to be sufficient
bool jaws_open_flag = true;
bool stinger_engaged_flag = false;
bool jaw_engaged_flag = false;

//Motor B - gripper motor
int PWMB = 9; //Speed control
int BIN1 = 7; //Direction
int BIN2 = 8; //Direction
int close_time = 7750; //was 7750
int open_time = 7750;
//int small_time = 200;
//int close_time_half = 425;
//int open_time_half = 375;


//Motor A - Phillips screwdriver
int PWMA = 10; //Speed control
int AIN1 = 12; //Direction
int AIN2 = 11; //Directiono

int STBY = 13;
int driving_time = 3800; //was 4500 when working

// servos pinout location
int stinger_arduino_pin = 3;
int stinger_engaged = 1850; // in microseconds
int stinger_disengaged = 1150; // in microseconds was 1150, 1400 for bottom layer clearance
int stinger_loadingPos = 1500;
int stinger_enterPos = 1400; // enter-exit position

int jaw_arduino_pin = 5;
int jaw_open = 1825; 
int jaw_close = 1215; 
int jaw_flat = 1260;
int jaw_45 = 1550;

//bool JawState = true; //jaw open = true
bool GripperState = false; //ungripped = false

int jawState = 0;
// 0 = open
// 1 = closed
// 2 = flat
// 3 = exit
// 4 = enter


 //////////////////////////////
 ///////////Setup /////////////
 //////////////////////////////

void setup() {
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  Serial.begin(9600);
  
  stinger_servo.attach(stinger_arduino_pin);
  jaw_servo.attach(jaw_arduino_pin); 
  stinger_servo.writeMicroseconds(stinger_disengaged);
  jaw_servo.writeMicroseconds(jaw_open);
  
  Serial.println("starting program...");
}


 //////////////////////////////
 ///////////Main loop//////////
 //////////////////////////////

bool isLastStateReading=false;

void loop() {

  shopbot_OP6_value = analogRead(shopbot_OP6);
  delay(10); //maybe there is a better way to clear the multiplexer?
  shopbot_OP6_value = analogRead(shopbot_OP6);

  //Serial.print("(OP2, OP3, OP5, OP6) ");
  //ardprintf("%f %f %f %f", shopbot_OP2_value, shopbot_OP3_value, shopbot_OP5_value, shopbot_OP6_value);
  //Serial.println();

  //delay(1000);
  
  // pause Arduino in loop while the Shopbot sets the outputs (as they don't all get set simultaneously
  // which can cause the Arduino to interpret them incorrectly

  if (shopbot_OP6_value > 950 && isLastStateReading==false) {
    isLastStateReading=true;     
    shopbot_OP2_value = analogRead(shopbot_OP2);
    delay(10);
    shopbot_OP3_value = analogRead(shopbot_OP3);
    delay(10);
    shopbot_OP5_value = analogRead(shopbot_OP5);
    delay(10);
    shopbot_OP7_value = analogRead(shopbot_OP7);
      
 /////////////////////////////
 ///program 1 = open jaws ////
 /////////////////////////////
 
    if (shopbot_OP2_value > 950 && shopbot_OP3_value < 950 && shopbot_OP5_value < 950 && shopbot_OP7_value < 950) {
      program_1 = true;
      printState();
      Serial.println("open jaw reading");
      }
    else {
      program_1 = false;}
  
    if (program_1 == true && jawState != 0){
      Serial.println("Opening Jaw");
      jaw_servo.writeMicroseconds(jaw_open);
      stinger_servo.writeMicroseconds(stinger_disengaged);
      program_1 = false; 
      jawState = 0;
      resetOutputState();
      }
       
    
 //////////////////////////////
 ///program 2 = jaws close ////
 /////////////////////////////
 
    if (shopbot_OP2_value < 950 && shopbot_OP3_value > 950 && shopbot_OP5_value < 950 && shopbot_OP7_value < 950) {
      program_2 = true;
      printState();
      Serial.println("close jaw reading");
      }
    else {
      program_2 = false;}
    
    if (program_2 == true && jawState != 1) {
      Serial.println("Jaw Close");
      jaw_servo.writeMicroseconds(jaw_close);
       program_2 = false; 
       jawState = 1;
       resetOutputState();}

  
//////////////////////////////
 ///program 3 = bolt ////
 /////////////////////////////
    if (shopbot_OP2_value > 950 && shopbot_OP3_value > 950 && shopbot_OP5_value < 950 && shopbot_OP7_value < 950) {
      program_3 = true;
      printState();
      Serial.println("bolt reading");
      }
    else {
      program_3 = false;}
       
    if (program_3 == true && jawState == 1) {
       Serial.println("Bolt");
       stinger_servo.writeMicroseconds(1850); //motor 2, full speed, left
       delay(1000);
       move(1, 255, 0); //motor 2, full speed, left
       delay(driving_time); //go for 1 second
       stop(); //stop     
       stinger_servo.writeMicroseconds(stinger_enterPos); //motor 2, full speed, left
       delay(1000); //go for 1 second 
       stinger_servo.write(0);  //was 0 for some reason
       
       //added
       jaw_servo.writeMicroseconds(jaw_open);
       delay(500);
       jaw_servo.writeMicroseconds(jaw_close);
       delay(500);
       jaw_servo.writeMicroseconds(jaw_open);
       delay(500);
       jaw_servo.writeMicroseconds(jaw_close);
       delay(500);
         
       program_3 = false;
       resetOutputState();}

////////////////////////////////////
 ///program 4 = engage gripper ////
 /////////////////////////////////

    if (shopbot_OP2_value < 950 && shopbot_OP3_value < 950 && shopbot_OP5_value > 950 && shopbot_OP7_value < 950) {
       program_4 = true;
       printState();
       Serial.println("engage gripper reading");
       }
    else {
      program_4 = false;}
  
    if (program_4 == true && GripperState == false) {
        Serial.println("Engage gripper");
        move(0, 255, 1); //motor B, full speed, left
        delay(open_time); //go for 1 second
        stop();
        program_4 = false;
        GripperState = true;
        resetOutputState();}


//////////////////////////////////////
 ///program 5 = disengage gripper ////
 //////////////////////////////////////
 
    if (shopbot_OP2_value > 950 && shopbot_OP3_value < 950 && shopbot_OP5_value > 950 && shopbot_OP7_value < 950) {
      program_5 = true;
      printState();
      Serial.println("disengage gripper reading");
      }
    else {
      program_5 = false;}
  
    if (program_5 == true && GripperState == true) {
        Serial.println("Disengage gripper");
        move(0, 255, 0); //motor B, full speed, left
        delay(open_time); //go for 1 second
        stop();
        program_5 = false;
        GripperState = false;
        resetOutputState();}


 /////////////////////////////
 ///program 6 =  close flat ////
 /////////////////////////////
 
    if (shopbot_OP2_value < 950 && shopbot_OP3_value > 950 && shopbot_OP5_value > 950 && shopbot_OP7_value < 950) {
      //Serial.println("checking");
      program_6 = true;
      printState();
      Serial.println("jaw flat reading");
      }
    else {
      program_6 = false;}
    
    if (program_6 == true && jawState != 2){
        Serial.println("Jaw Flat");
        jaw_servo.writeMicroseconds(jaw_flat);
        stinger_servo.writeMicroseconds(1050);
        program_6 = false;
        jawState = 2;
        resetOutputState();}

    
 //////////////////////////////
 ///program 7 = jaws exit position ////
 /////////////////////////////
 
    if (shopbot_OP2_value > 950 && shopbot_OP3_value > 950 && shopbot_OP5_value > 950 && shopbot_OP7_value < 950) {
      program_7 = true;
      printState();
      Serial.println("jaw exit reading");
      }
    else {
      program_7 = false;}
    
    if (program_7 == true && jawState != 3) {
       Serial.println("Jaw Exit");
       jaw_servo.writeMicroseconds(1400);
       stinger_servo.writeMicroseconds(1300);
       program_7 = false;
       jawState = 3;
       resetOutputState();}


//////////////////////////////
 ///program 8 = jaws enter position ////
 /////////////////////////////
    
    if (shopbot_OP2_value < 950 && shopbot_OP3_value < 950 && shopbot_OP5_value < 950 && shopbot_OP7_value > 950) {
      program_8 = true;
      printState();
      Serial.println("jaw enter reading");
      }
    else {
      program_8 = false;}
       
    if (program_8 == true && jawState != 4) {
       Serial.println("Jaw Enter");
       jaw_servo.writeMicroseconds(1550);
       stinger_servo.writeMicroseconds(stinger_enterPos);
       program_8 = false;
       jawState = 4;
       resetOutputState();}
  }
  else{
    if(shopbot_OP6_value < 950 && isLastStateReading==true){
      isLastStateReading=false;}
  }
  delay(10);


///////////////////////////////////////
/////Serial monitor values /////////////
////////////////////////////////////////
   
  if(Serial.available() > 0){
      char c = Serial.read();
             
      //program 1
      if (c == 'o'){
        Serial.println("Open Jaw command");
        jaw_servo.writeMicroseconds(jaw_open);
        stinger_servo.writeMicroseconds(stinger_disengaged);
        jawState = 0;
        printState();}
        
      //program 2
      if (c == 'c'){
        Serial.println("Close Jaw command");
        jaw_servo.writeMicroseconds(jaw_close);
        stinger_servo.writeMicroseconds(stinger_disengaged);
        jawState = 1;
        printState();}
  
      //program 3
      if (c == 'b'){
        Serial.println("Bolt command");
        stinger_servo.writeMicroseconds(1850); //motor 2, full speed, left
        delay(1000);
        move(1, 255, 0); //motor 2, full speed, left
        delay(driving_time); //go for 1 second
        stop(); //stop     
        stinger_servo.writeMicroseconds(stinger_enterPos); //motor 2, full speed, left
        delay(1000); //go for 1 second 
        stinger_servo.write(0);  //was 0 for some reason
       
        //added
        jaw_servo.writeMicroseconds(jaw_open);
        delay(500);
        jaw_servo.writeMicroseconds(jaw_close);
        delay(500);
        jaw_servo.writeMicroseconds(jaw_open);
        delay(500);
        jaw_servo.writeMicroseconds(jaw_close);
        delay(500);
         
        program_3 = false;
        resetOutputState();
        printState();} 
  
      //program 4
        if (c == 'e'){
          Serial.println("engage gripper command");
          move(0, 255, 1); //motor B, full speed, left
          delay(open_time); //go for 1 second
          stop();
          GripperState = true;
          printState();} 
  
      //program 5
      if (c == 'd'){
        Serial.println("disengage gripper command");
        move(0, 255, 0); //motor B, full speed, left
        delay(close_time); //go for 1 second
        stop(); 
        GripperState = false;
        printState();}
  
      //program 6
      if (c == 'f'){
        Serial.println("Open Jaw Flat");
        jaw_servo.writeMicroseconds(jaw_flat);
        stinger_servo.writeMicroseconds(1050);
        jawState = 2;
        printState();}
  
      //program 7
        if (c == 'g'){
          Serial.println("Jaw Exit Command");
          jaw_servo.writeMicroseconds(1400);
          stinger_servo.writeMicroseconds(1300);
          jawState = 3;
          printState();}

      //program 7b
        if (c == 'h'){
          Serial.println("Jaw Enter Command");
          jaw_servo.writeMicroseconds(1650); //was 1550, but changed due to interference
          stinger_servo.writeMicroseconds(1400);
          jawState = 4;
          printState();}

          
      //program 8
      if (c == 'u'){
        Serial.println("Unbolt");
        stinger_servo.writeMicroseconds(1850); //motor 2, full speed, left
        delay(1000);
        move(1, 255, 1); //motor 2, full speed, left
        delay(driving_time); //go for 1 second
        stop(); //stop     
        stinger_servo.writeMicroseconds(stinger_enterPos); //motor 2, full speed, left
        delay(1000); //go for 1 second 
        stinger_servo.write(0);
        printState();} 
//        stinger_servo.writeMicroseconds(1850);; //motor 2, full speed, left
//        delay(1000);
//        move(1, 128, 1); //motor 2, full speed, left
//        delay(6000); //go for 1 second
//        stop(); //stop     
//        stinger_servo.writeMicroseconds(stinger_disengaged); //motor 2, full speed, left
//        delay(1000); //go for 1 second 
//        stinger_servo.write(0);
//        printState();} //motor 2, full speed, left
  
      //secret programs
      if (c == '['){
        Serial.println("Secret Program: Grip 1 sec");
        move(0, 255, 1); //motor B, full speed, left
        delay(1000); //go for 1 second
        stop(); }
  
      if (c == ']'){
        Serial.println("Secret Program: UnGrip 1 sec");
        move(0, 255, 0); //motor B, full speed, right
        delay(1000); //go for 1 second
        stop(); }

      if (c == '{'){
        Serial.println("Secret Program: Grip .25 sec");
        move(0, 255, 1); //motor B, full speed, left
        delay(250); //go for 1 second
        stop(); }
  
      if (c == '}'){
        Serial.println("Secret Program: UnGrip .25 sec");
        move(0, 255, 0); //motor B, full speed, right
        delay(250); //go for 1 second
        stop(); }

  
      if (c == ','){
        Serial.println("Secret Program: Screw drive 1 sec");
        move(1, 255, 0); //motor A, full speed, left
        delay(1000); //go for 1 second
        stop(); }       
  
      if (c == '.'){
        Serial.println("Secret Program: UnScrew drive 1 sec");
        move(1, 255, 1); //motor A, full speed, right
        delay(1000); //go for 1 second
        stop(); } 
  }
}

//////////////////////////////////
///DC motor control function//////
//////////////////////////////////

void move(int motor, int speed, int direction){
//Move specific motor at speed and direction
//motor: 0 for B 1 for A
//speed: 0 is off, and 255 is full speed
//direction: 0 clockwise, 1 counter-clockwise

  digitalWrite(STBY, HIGH); //disable standby

  boolean inPin1 = LOW;
  boolean inPin2 = HIGH;

  if(direction == 1){
    inPin1 = HIGH;
    inPin2 = LOW;
  }

  if (motor == 1) {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
    digitalWrite(AIN1, inPin1);
    digitalWrite(AIN2, inPin2);
    analogWrite(PWMA, speed);
    //Serial.println("motor a going");
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, inPin1);
    digitalWrite(BIN2, inPin2);
    analogWrite(PWMB, speed);
    //Serial.println("motor b going");
  }
}

void stop(){
//enable standby  
  digitalWrite(STBY, LOW); 
}

void printState(){
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("--------------------");
  Serial.print("JawState = ");
  Serial.println(jawState);
  Serial.print("GripperState = ");
  Serial.println(GripperState);
  
}

void resetOutputState(){
  shopbot_OP6_value = 0;
  shopbot_OP2_value = 0;
  shopbot_OP3_value = 0;
  shopbot_OP5_value = 0;
  shopbot_OP7_value = 0;
}


//////////////////////////////////////////////
// function to print out values more neatly///
//////////////////////////////////////////////

int ardprintf(char *str, ...)
{
  int i, count=0, j=0, flag=0;
  char temp[ARDBUFFER+1];
  for(i=0; str[i]!='\0';i++)  if(str[i]=='%')  count++;

  va_list argv;
  va_start(argv, count);
  for(i=0,j=0; str[i]!='\0';i++)
  {
    if(str[i]=='%')
    {
      temp[j] = '\0';
      Serial.print(temp);
      j=0;
      temp[0] = '\0';

      switch(str[++i])
      {
        case 'd': Serial.print(va_arg(argv, int));
                  break;
        case 'l': Serial.print(va_arg(argv, long));
                  break;
        case 'f': Serial.print(va_arg(argv, double));
                  break;
        case 'c': Serial.print((char)va_arg(argv, int));
                  break;
        case 's': Serial.print(va_arg(argv, char *));
                  break;
        default:  ;
      };
    }
    else 
    {
      temp[j] = str[i];
      j = (j+1)%ARDBUFFER;
      if(j==0) 
      {
        temp[ARDBUFFER] = '\0';
        Serial.print(temp);
        temp[0]='\0';
      }
    }
  };
  Serial.println();
  return count + 1;
}
#undef ARDBUFFER
#endif


