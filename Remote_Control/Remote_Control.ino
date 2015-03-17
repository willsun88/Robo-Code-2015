
// Arduino-IRremote Library
// Version 0.1 July, 2009
// Copyright 2009 Ken Shirriff
// http://arcfn.com

// Include IR Library 
#include <IRremote.h>

// IR Stuff
int SEND_PIN = 3;
IRsend irsend;

// Button Pin Numbers
int D3 = 7;
int D4 = 4;
int D5 = 5;
int D6 = 6;
int F1 = 8;
int F2 = 9;
int F3 = 10;

// Joystick Pin Numbers
int JH = 0;
int JV = 1;
int JS = 2;

// Transmission Variable Names
unsigned long joy_horiz = 0;
unsigned long joy_vert = 0;
unsigned long tact_but = 0;
unsigned long signal = 0;

// Zeroing Variables
int horiz_zero = 0;
int vert_zero = 0;

// CRC8 Code
// From Github, user jlewallen
// https://github.com/jlewallen/arduino/blob/master/libraries/jlewallen/crc8.c
// CRC8 is a checksum
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
  // Begin Serial Communication
  Serial.begin(9600);
  
  // Set Motor Buttons to Inputs
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  
  // Set Motor Button Pullup Resistors to On
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
  digitalWrite(D5, HIGH);
  digitalWrite(D6, HIGH);
  
  // Set Function Buttons to Inputs
  pinMode(F1, INPUT);
  pinMode(F2, INPUT);
  pinMode(F3, INPUT);
  
  // Set Function Button Pullup Resistors to On
  digitalWrite(F1, HIGH);
  digitalWrite(F2, HIGH);
  digitalWrite(F3, HIGH);
  
  // Set Joystick Push Button as Input
  pinMode(JS, INPUT);
  
  // Turn Joystick Push Button Pullup Resistors On 
  digitalWrite(JS, HIGH);
  
  // Set the LED as an Output
  pinMode(SEND_PIN, OUTPUT);
  
  // Joystick Zeroing Variables
  horiz_zero = (analogRead(JH)/8)-64;
  vert_zero = (analogRead(JV)/8)-64;
  
  // Print "Setup Complete" to the Serial Line 
  Serial.println("Setup Complete");
}

void loop() {
  // Set CRC variable to a value of 0 
  byte crc = 0;
  
  // Read Horizontal Joystick Value and Write to Variable
  joy_horiz = analogRead(JH)/8;
  if (joy_horiz >= horiz_zero) {
    joy_horiz -= horiz_zero;
  }
  
  // Read Vertical Joystick Value and Write to Variable
  joy_vert = analogRead(JV)/8;
  if (joy_vert >= vert_zero) {
    joy_vert -= vert_zero;
  }
  
  // Assemble Button Bits for Transmission
  tact_but = digitalRead(F1);
  tact_but += digitalRead(F2)*2;
  tact_but += digitalRead(F3)*4;
  tact_but += digitalRead(D3)*8;
  tact_but += digitalRead(D4)*16;
  tact_but += digitalRead(D5)*32;
  tact_but += digitalRead(D6)*64;
  tact_but += digitalRead(JS)*128;
  
  // Assemble signal to transmit
  // Bits 1-7 = horiz
  // Bits 8-14 = vert
  // Bits 15-22 = buttons
  // Bits 23-25 = reserved
  // Bits 26-32 = CRC8
  signal = joy_horiz + (joy_vert << 7) + (tact_but << 14); 
  
  //Create CRC Signal
  crc = crc8_block(crc, (uint8_t*)&signal, 3);
  Serial.print("crc: ");
  Serial.println(crc,HEX);
  
  // Add CRC8 Code to signal for transmission
  signal = signal + ((unsigned long)crc << 24);
  
  //Transmit 32-bit code containing buttons, joystick positions, and CRC8 Code
  irsend.sendNEC(signal, 32);
  
  // Print Signal to Serial Port
  Serial.println(signal, HEX);
  
  // Wait 50 Milliseconds Before Compiling and Transmitting Next Code
  delay(50);
}

