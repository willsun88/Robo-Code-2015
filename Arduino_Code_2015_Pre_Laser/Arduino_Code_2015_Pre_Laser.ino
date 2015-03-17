
// Arduino-IRremote Library
// Version 0.1 July, 2009
// Copyright 2009 Ken Shirriff
// http://arcfn.com

// Include Libraries in Sketch
#include <IRremote.h>
#include <Wire.h>

// Masks for Buttons
#define MASK_D3 0x8
#define MASK_D4 0x10
#define MASK_D5 0x20
#define MASK_D6 0x40
#define MASK_F1 0x1
#define MASK_F2 0x2
#define MASK_F3 0x4

// State Definitions for Sequence
#define F2_STOP   0
#define F2_UP     1
#define F2_DOWN   2
#define F2_SPIN   3

// IR Stuff
int RECV_PIN = 2; 
IRrecv irrecv(RECV_PIN);
decode_results results;

// NXT Motor Speed Pins
int PWMA = 3;   
int PWMB = 5;  
int PWMC = 6;  
int PWMD = 9;  
int PWME = 10;

// NXT Motor Direction Pins
int DIRA = A0;
int DIRB = A1;
int DIRC = A2;
int DIRD = A3;
int DIRE = A4;

// NXT Motor Speed Variables
byte SpdA = 0;
byte SpdB = 0;
byte SpdC = 0;
byte SpdD = 0;
byte SpdE = 0;

// NXT Motor Direction Variables
byte DirA = 0;
byte DirB = 0;
byte DirC = 0;
byte DirD = 0;
byte DirE = 0;

// NXT Motor Tachometer Pints
byte TACHC0 = 4;
byte TACHC1 = 7;
byte TACHD0 = 8;
byte TACHD1 = 11;
byte TACHE0 = 12;
byte TACHE1 = 13;

// NXT Motor Tachometer Variables
int TACHC = 0;
int TACHD = 0;
int TACHE = 0;

// Motor Speeds
byte Spd = 80;
byte Spdstr = 50;
byte Spdlft = 100;
byte Spdwrist = 100;
byte Spdfing = 50;

// Joystick Output Variables
byte SPDA = 0;
byte SPDB = 0;

// Button F2 Sequence Variables 
int F2_State = 0;
unsigned long F2_Time = 0;
int F2_Cycle = 0;

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
  irrecv.blink13(true); // Blink LED on arduino when IR signal is recieved
  
  // Set DC Motor Control Pins to be Outputs
  pinMode(PWMA, OUTPUT);  
  pinMode(PWMB, OUTPUT);
  pinMode(PWMC, OUTPUT);
  pinMode(PWMD, OUTPUT);
  pinMode(PWME, OUTPUT);
  
  pinMode(DIRA, OUTPUT);
  pinMode(DIRB, OUTPUT);
  pinMode(DIRC, OUTPUT);
  pinMode(DIRD, OUTPUT);
  pinMode(DIRE, OUTPUT);
  
  pinMode(TACHC, INPUT);
  pinMode(TACHD, INPUT);
  pinMode(TACHE, INPUT);
  
  // Set Motors to Stop
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  analogWrite(PWMC, 0);
  analogWrite(PWMD, 0);
  analogWrite(PWME, 0);
  
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
   
      SPDB = abs(Xr);
      analogWrite(SpdB, SPDB << 1); 
      SPDA = abs(Yr);
      analogWrite(SpdA, SPDA << 1);
      
      // Set Drive Motor Directions
      if(Xr > 0){
        DirA = 0;
      }
      else{
        DirB = 1;
      }
      
      if(Yr > 0){
        DirB = 0;
      }
      else{
        DirA = 1;
      } 
     
     /*
     // Set Motor E Speed and Direction
     if(~button & MASK_D4){
       SpdE = Spdlft;
       DirE = 0;
     }
     else if(~button & MASK_D5){
       SpdE = Spdlft;
       DirE = 1;
     }
     else{
       SpdE = 0;
     }
     
     // Set Motor 1 Direction and Speed (DC Motor)
     if(~button & MASK_D3){
       digitalWrite(DIRC, HIGH);  // Set Motor Direction to Forward
       analogWrite(PWMC, 255);    // Set motor to run at (255/255 = 100)% duty cycle
     }
     else if(~button & MASK_D6){
       digitalWrite(DIRC, LOW);  // Set Motor Direction to Reverse
       analogWrite(PWMC, 255); // Set motor to run at (255/255 = 100)% duty cycle
     }
     else{
       analogWrite(PWMC, 0); // Set motor to run at (0/255 = 0)% duty cycle
     }
   
     // Set Motor 1 Direction and Speed (DC Motor)
     if(~button & MASK_F1){
       digitalWrite(DIRD, HIGH);  // Set Motor Direction to Forward
       analogWrite(PWMD, 255);    // Set motor to run at (255/255 = 100)% duty cycle
     }
     else if(~button & MASK_F3){
       digitalWrite(DIRD, LOW);  // Set Motor Direction to Reverse
       analogWrite(PWMD, 255); // Set motor to run at (255/255 = 100)% duty cycle
     }
     else{
       analogWrite(PWMD, 0); // Set motor to run at (0/255 = 0)% duty cycle
     }
   
    }
    */
    
    irrecv.resume(); // Receive the next value
  }
  
   /* Perform Sequence 2
   switch (F2_State){
     case F2_STOP:
       if(~button & MASK_F2){
         F2_State = F2_UP;
         F2_Time = millis();
         F2_Cycle = 0;
       }
       break;
     case F2_UP:
       SpdA = Spdlft;
       DirA = 0;
       SpdB = 0;
       SpdC = 0;
       analogWrite(pwm_a, 0);    // set motor to run at (255/255 = 100)% duty cycle
       if((F2_Time + 750) < millis()){
         F2_State = F2_SPIN;
         F2_Time = millis();
         F2_Cycle++;
       }
       break;
     case F2_DOWN:
       SpdA = Spdlft;
       DirA = 1;
       SpdB = 0;
       SpdC = 0;
       analogWrite(pwm_a, 0);    // set motor to run at (255/255 = 100)% duty cycle
       if((F2_Time + 550) < millis()){
         F2_State = F2_UP;
         F2_Time = millis();
       }
       break;
     case F2_SPIN:
       SpdA = 0;
       SpdB = 0;
       SpdC = 0;
       digitalWrite(dir_a, LOW);  // Reverse motor direction, 1 high, 2 low
       analogWrite(pwm_a, 255);    // set motor to run at (255/255 = 100)% duty cycle
       if((F2_Time + 1000) < millis()){
         F2_State = F2_DOWN;
         F2_Time = millis();
       }
       if(F2_Cycle > 6){
         SpdA = 0;
         SpdB = 0;
         SpdC = 0;
         analogWrite(pwm_a, 0);    // set motor to run at (255/255 = 100)% duty cycle
         F2_State = F2_STOP;
       }
       break;
   
   }
   */
}
}
