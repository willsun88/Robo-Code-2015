
// Arduino-IRremote Library
// Version 0.1 July, 2009
// Copyright 2009 Ken Shirriff
// http://arcfn.com

// Include Libraries in Sketch
#include <IRremote.h>
// #include <Wire.h>
// #include <Servo.h>

// Masks for Buttons
#define MASK_D3 0x8
#define MASK_D4 0x10
#define MASK_D5 0x20
#define MASK_D6 0x40
#define MASK_F1 0x1
#define MASK_F2 0x2
#define MASK_F3 0x4


// IR Stuff
int RECV_PIN = 2; 
IRrecv irrecv(RECV_PIN);
decode_results results;

// Motor Shield Stuff
int pwm_a = 3;   //PWM control for motor outputs 1 and 2 is on digital pin 3
int pwm_b = 5;  //PWM control for motor outputs 3 and 4 is on digital pin 11
int pwm_c = 6;
int pwm_d = 9;
int pwm_e = 10;
int dir_a = A0;  //direction control for motor outputs 1 and 2 is on digital pin 12
int dir_b = A1;  //direction control for motor outputs 3 and 4 is on digital pin 13
int dir_c = A2;
int dir_d = A3;
int dir_e = A4;


// Motor Speed Variables
byte SpdA = 0;
byte SpdB = 0;
byte SpdC = 0;
byte SpdD = 0;
byte SpdE = 0;
byte DirA = 0;
byte DirB = 0;
byte DirC = 0;
byte DirD = 0;
byte DirE = 0;

// Motor Max Speeds
byte Spd = 80;
byte Spdstr = 50;
byte Spdlft = 100;

// CRC8 Code
// Github, user jlewallen
// https://github.com/jlewallen/arduino/blob/master/libraries/jlewallen/crc8.c
uint8_t crc8_update(uint8_t crc, uint8_t data) {
  uint8_t updated = crc ^ data;
  for (uint8_t i = 0; i < 8; ++i) {
    if ((updated & 0x80 ) != 0) {
      updated <<= 1;
      updated ^= 0x07;
    }
    else {
      updated <<= 1;
    }
  }
  return updated;
}

uint8_t crc8_block(uint8_t crc, uint8_t *data, uint8_t sz) {
  while (sz > 0) {
    crc = crc8_update(crc, *data++);
    sz--;
  }
  return crc;
}

void setup()
{
  // Begin Serial Communication at a Frequency of 9600 Baud
  Serial.begin(9600);
  
  irrecv.enableIRIn(); // Start IR Reciever
  
  // Set DC Motor Control Pins to be Outputs
  pinMode(pwm_a, OUTPUT);  
  pinMode(pwm_b, OUTPUT);
  pinMode(pwm_c, OUTPUT);  
  pinMode(pwm_d, OUTPUT);
  pinMode(pwm_e, OUTPUT);  
  pinMode(dir_a, OUTPUT);
  pinMode(dir_b, OUTPUT);
  pinMode(dir_c, OUTPUT);
  pinMode(dir_d, OUTPUT);
  pinMode(dir_e, OUTPUT);
  
  // Set Motors A and B on Ardumoto Shield to Stop
  analogWrite(pwm_a, 0);
  analogWrite(pwm_b, 0);
  analogWrite(pwm_c, 0);
  analogWrite(pwm_d, 0);
  analogWrite(pwm_e, 0);
  
  Serial.println("Setup Complete");
}

void loop() {
  // Create variables for joystick, buttons, and CRC8 logic and initialize to 0
  int X = 0;
  int Y = 0;
  int Xr = 0;
  int Yr = 0;
  int button = 0xFFFF;
  int crc = 0;
  
  // Recieve IR Signal
  if (irrecv.decode(&results)) {
    // Print IR Signal in Hexadecimal onto Serial Line
    Serial.println(results.value, HEX); 
    
    // Check Code for Proper CRC8 value
    crc = crc8_block(crc, (uint8_t*)&results.value, 3);
    Serial.print("crc: ");
    Serial.println(crc,HEX);
    // Serial.println(results.value, BIN);
   
    // if CRC8 code is good, continue to decode motors
    if(crc == (0xFF & (results.value >> 24))){
      X = 0x7F & results.value; // unpack horizontal joystick position
      Y = 0x7F & (results.value >> 7); // unpack vertical joystick position
      button = 0xFF & (results.value >> 14); // unpack button status
      
      // Step 1 : Translate to (0,0) center
      X -= 64; 
      Y -= 64;
      
      //Step 2: Crop Graph to Remove Illegal Zones
      while(Y > (-X+64)){
       X-=1;
       Y-=1;
      }
      while(Y > (X+64)){
        X+=1;
        Y-=1;
      }
      while(Y < (-X-64)) {
        X+=1;
        Y+=1;
      }
      while(Y < (X-64)) {
        X-=1;
        Y+=1;
      }
  
      //Step 3: Scale Graph according to maximum motor speed
      X *= (Spdstr*1.414)/64;
      Y *= (Spd*1.414)/64;
  
      //Step 4: Rotate 45 degrees clockwise to convert to motor speeds.
      Xr = (.707 * X) + (.707 * Y);
      Yr = (-.707 * X) + (.707 * Y);
   
      SpdB = abs(Xr);
      SpdA = abs(Yr);
      
      // Set Drive Motor Directions
      if(Xr > 0){
        DirB = 0;
      }
      else{
        DirB = 1;
      }
      
      if(Yr > 0){
        DirA = 0;
      }
      else{
        DirA = 1;
      } 
     
     
     
     // Set Motor A Speed and Direction
     /*if(~button & MASK_D4){
       SpdA = Spdlft;
       DirA = 0;
     }
     else if(~button & MASK_D5){
       SpdA = Spdlft;
       DirA = 1;
     }
     else{
       SpdA = 0;
     }
     // Set Motor D Speed and Direction
     if(~button & MASK_F1){
       SpdD = Spdlft;
       DirD = 0;
     }
     else if(~button & MASK_F2){
       SpdD = Spdlft;
       DirD = 1;
     }
     else{
       SpdD = 0;
     }
     // Set Motor E Speed and Direction
     if(~button & MASK_D6){
       SpdE = Spdlft;
       DirE = 0;
     }
     else if(~button & MASK_D3){
       SpdE = Spdlft;
       DirE = 1;
     }
     else{
       SpdE = 0;
     }
   */
    }
    
    
    
    irrecv.resume(); // Receive the next value
  }
  //Write values to hardware here
  //Use: AnalogWrite, DigitalWrite
  digitalWrite(dir_e, DirE);
  digitalWrite(dir_d, DirD);
  digitalWrite(dir_c, DirC);
  digitalWrite(dir_b, DirB);
  digitalWrite(dir_a, DirA); 
  analogWrite(pwm_e, SpdE);
  analogWrite(pwm_d, SpdD);
  analogWrite(pwm_c, SpdC);
  analogWrite(pwm_b, SpdB);
  analogWrite(pwm_a, SpdA);
  
   
}

