/**************************************************************************
 *     This file is part of the RPU OS for Arduino Project.

    I, Dick Hamill, the author of this program disclaim all copyright
    in order to make this program freely available in perpetuity to
    anyone who would like to use it. Dick Hamill, 6/1/2020

    RPU OS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    RPU OS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <https://www.gnu.org/licenses/>.
 */


 
#include <Arduino.h>
#include <EEPROM.h>
//#define DEBUG_MESSAGES    1
#define RPU_CPP_FILE
#include "RPULite_Config.h"
#include "RPULite.h"

#ifndef RPU_OS_HARDWARE_REV
#define RPU_OS_HARDWARE_REV 1
#endif

// To use this library, take the example_RPU_Config.h, 
// edit it for your hardware and game parameters and put
// it in your game's code folder as RPU_Config.h
// (so when you fetch new versions of the library, you won't 
// overwrite your config)
#include "RPULite_Config.h"

#ifdef RPU_OS_USE_7_DIGIT_DISPLAYS
#define RPU_NUM_DIGITS     7
#define RPU_00_MASK        0x60
#else
#define RPU_NUM_DIGITS     6
#define RPU_00_MASK        0x30
#endif

// Global variables
volatile byte DisplayDigits[5][RPU_NUM_DIGITS];
volatile byte DisplayDigitEnable[5];
#ifdef RPU_OS_DIMMABLE_DISPLAYS
volatile boolean DisplayDim[5];
#endif
volatile boolean DisplayOffCycle = false;
volatile byte CurrentDisplayDigit=0;
volatile byte LampStates[RPU_NUM_LAMP_BITS], LampDim0[RPU_NUM_LAMP_BITS], LampDim1[RPU_NUM_LAMP_BITS];
volatile byte LampFlashPeriod[RPU_MAX_LAMPS];
byte DimDivisor1 = 2;
byte DimDivisor2 = 3;

volatile byte SwitchesMinus2[5];
volatile byte SwitchesMinus1[5];
volatile byte SwitchesNow[5];
#ifdef RPU_OS_USE_DIP_SWITCHES
byte DipSwitches[4];
#endif


#define SOLENOID_STACK_SIZE 64
#define SOLENOID_STACK_EMPTY 0xFF
volatile byte SolenoidStackFirst;
volatile byte SolenoidStackLast;
volatile byte SolenoidStack[SOLENOID_STACK_SIZE];
boolean SolenoidStackEnabled = true;
volatile byte CurrentSolenoidByte = 0xFF;

#define TIMED_SOLENOID_STACK_SIZE 32
struct TimedSolenoidEntry {
  byte inUse;
  unsigned long pushTime;
  byte solenoidNumber;
  byte numPushes;
  byte disableOverride;
};
TimedSolenoidEntry TimedSolenoidStack[32];

#define SWITCH_STACK_SIZE   64
#define SWITCH_STACK_EMPTY  0xFF
volatile byte SwitchStackFirst;
volatile byte SwitchStackLast;
volatile byte SwitchStack[SWITCH_STACK_SIZE];

#if (RPU_OS_HARDWARE_REV==1)
#define ADDRESS_U10_A           0x14
#define ADDRESS_U10_A_CONTROL   0x15
#define ADDRESS_U10_B           0x16
#define ADDRESS_U10_B_CONTROL   0x17
#define ADDRESS_U11_A           0x18
#define ADDRESS_U11_A_CONTROL   0x19
#define ADDRESS_U11_B           0x1A
#define ADDRESS_U11_B_CONTROL   0x1B
#define ADDRESS_SB100           0x10

#elif (RPU_OS_HARDWARE_REV==2)
#define ADDRESS_U10_A           0x00
#define ADDRESS_U10_A_CONTROL   0x01
#define ADDRESS_U10_B           0x02
#define ADDRESS_U10_B_CONTROL   0x03
#define ADDRESS_U11_A           0x08
#define ADDRESS_U11_A_CONTROL   0x09
#define ADDRESS_U11_B           0x0A
#define ADDRESS_U11_B_CONTROL   0x0B
#define ADDRESS_SB100           0x10
#define ADDRESS_SB100_CHIMES    0x18
#define ADDRESS_SB300_SQUARE_WAVES  0x10
#define ADDRESS_SB300_ANALOG        0x18

#elif (RPU_OS_HARDWARE_REV==3)

#define ADDRESS_U10_A           0x88
#define ADDRESS_U10_A_CONTROL   0x89
#define ADDRESS_U10_B           0x8A
#define ADDRESS_U10_B_CONTROL   0x8B
#define ADDRESS_U11_A           0x90
#define ADDRESS_U11_A_CONTROL   0x91
#define ADDRESS_U11_B           0x92
#define ADDRESS_U11_B_CONTROL   0x93
#define ADDRESS_SB100           0xA0
#define ADDRESS_SB100_CHIMES    0xC0
#define ADDRESS_SB300_SQUARE_WAVES  0xA0
#define ADDRESS_SB300_ANALOG        0xC0

#endif 




#if (RPU_OS_HARDWARE_REV==1) or (RPU_OS_HARDWARE_REV==2)

#if defined(__AVR_ATmega2560__)
#error "ATMega requires RPU_OS_HARDWARE_REV of 3, check RPU_Config.h and adjust settings"
#endif

void RPU_DataWrite(int address, byte data) {
  
  // Set data pins to output
  // Make pins 5-7 output (and pin 3 for R/W)
  DDRD = DDRD | 0xE8;
  // Make pins 8-12 output
  DDRB = DDRB | 0x1F;

  // Set R/W to LOW
  PORTD = (PORTD & 0xF7);

  // Put data on pins
  // Put lower three bits on 5-7
  PORTD = (PORTD&0x1F) | ((data&0x07)<<5);
  // Put upper five bits on 8-12
  PORTB = (PORTB&0xE0) | (data>>3);

  // Set up address lines
  PORTC = (PORTC & 0xE0) | address;

  // Wait for a falling edge of the clock
  while((PIND & 0x10));

  // Pulse VMA over one clock cycle
  // Set VMA ON
  PORTC = PORTC | 0x20;
  
  // Wait while clock is low
  while(!(PIND & 0x10));
  // Wait while clock is high
// Doesn't seem to help --  while((PIND & 0x10));

  // Set VMA OFF
  PORTC = PORTC & 0xDF;

  // Unset address lines
  PORTC = PORTC & 0xE0;
  
  // Set R/W back to HIGH
  PORTD = (PORTD | 0x08);

  // Set data pins to input
  // Make pins 5-7 input
  DDRD = DDRD & 0x1F;
  // Make pins 8-12 input
  DDRB = DDRB & 0xE0;
}



byte RPU_DataRead(int address) {
  
  // Set data pins to input
  // Make pins 5-7 input
  DDRD = DDRD & 0x1F;
  // Make pins 8-12 input
  DDRB = DDRB & 0xE0;

  // Set R/W to HIGH
  DDRD = DDRD | 0x08;
  PORTD = (PORTD | 0x08);

  // Set up address lines
  PORTC = (PORTC & 0xE0) | address;

  // Wait for a falling edge of the clock
  while((PIND & 0x10));

  // Pulse VMA over one clock cycle
  // Set VMA ON
  PORTC = PORTC | 0x20;

  // Wait a full clock cycle to make sure data lines are ready
  // (important for faster clocks)
  // Wait while clock is low
  while(!(PIND & 0x10));

  // Wait for a falling edge of the clock
  while((PIND & 0x10));
  
  // Wait while clock is low
  while(!(PIND & 0x10));

  byte inputData = (PIND>>5) | (PINB<<3);

  // Set VMA OFF
  PORTC = PORTC & 0xDF;

  // Wait for a falling edge of the clock
// Doesn't seem to help  while((PIND & 0x10));

  // Set R/W to LOW
  PORTD = (PORTD & 0xF7);

  // Clear address lines
  PORTC = (PORTC & 0xE0);

  return inputData;
}


void WaitClockCycle(int numCycles=1) {
  for (int count=0; count<numCycles; count++) {
    // Wait while clock is low
    while(!(PIND & 0x10));
  
    // Wait for a falling edge of the clock
    while((PIND & 0x10));
  }
}

#elif (RPU_OS_HARDWARE_REV==3)

// Rev 3 connections
// Pin D2 = IRQ
// Pin D3 = CLOCK
// Pin D4 = VMA
// Pin D5 = R/W
// Pin D6-12 = D0-D6
// Pin D13 = SWITCH
// Pin D14 = HALT
// Pin D15 = D7
// Pin D16-30 = A0-A14

#if defined(__AVR_ATmega328P__)
#error "RPU_OS_HARDWARE_REV 3 requires ATMega2560, check RPU_Config.h and adjust settings"
#endif


void RPU_DataWrite(int address, byte data) {
  
  // Set data pins to output
  DDRH = DDRH | 0x78;
  DDRB = DDRB | 0x70;
  DDRJ = DDRJ | 0x01;

  // Set R/W to LOW
  PORTE = (PORTE & 0xF7);

  // Put data on pins
  // Lower Nibble goes on PortH3 through H6
  PORTH = (PORTH&0x87) | ((data&0x0F)<<3);
  // Bits 4-6 go on PortB4 through B6
  PORTB = (PORTB&0x8F) | ((data&0x70));
  // Bit 7 goes on PortJ0
  PORTJ = (PORTJ&0xFE) | (data>>7);  

  // Set up address lines
  PORTH = (PORTH & 0xFC) | ((address & 0x0001)<<1) | ((address & 0x0002)>>1); // A0-A1
  PORTD = (PORTD & 0xF0) | ((address & 0x0004)<<1) | ((address & 0x0008)>>1) | ((address & 0x0010)>>3) | ((address & 0x0020)>>5); // A2-A5
  PORTA = ((address & 0x3FC0)>>6); // A6-A13
  PORTC = (PORTC & 0x3F) | ((address & 0x4000)>>7) | ((address & 0x8000)>>9); // A14-A15

  // Wait for a falling edge of the clock
  while((PINE & 0x20));

  // Pulse VMA over one clock cycle
  // Set VMA ON
  PORTG = PORTG | 0x20;

  // Wait while clock is low
  while(!(PINE & 0x20));

  // Set VMA OFF
  PORTG = PORTG & 0xDF;

  // Unset address lines
  PORTH = (PORTH & 0xFC);
  PORTD = (PORTD & 0xF0);
  PORTA = 0;
  PORTC = (PORTC & 0x3F);
  
  // Set R/W back to HIGH
  PORTE = (PORTE | 0x08);

  // Set data pins to input
  DDRH = DDRH & 0x87;
  DDRB = DDRB & 0x8F;
  DDRJ = DDRJ & 0xFE;
  
}



byte RPU_DataRead(int address) {
  
  // Set data pins to input
  DDRH = DDRH & 0x87;
  DDRB = DDRB & 0x8F;
  DDRJ = DDRJ & 0xFE;

  // Set R/W to HIGH
  DDRE = DDRE | 0x08;
  PORTE = (PORTE | 0x08);

  // Set up address lines
  PORTH = (PORTH & 0xFC) | ((address & 0x0001)<<1) | ((address & 0x0002)>>1); // A0-A1
  PORTD = (PORTD & 0xF0) | ((address & 0x0004)<<1) | ((address & 0x0008)>>1) | ((address & 0x0010)>>3) | ((address & 0x0020)>>5); // A2-A5
  PORTA = ((address & 0x3FC0)>>6); // A6-A13
  PORTC = (PORTC & 0x3F) | ((address & 0x4000)>>7) | ((address & 0x8000)>>9); // A14-A15

  // Wait for a falling edge of the clock
  while((PINE & 0x20));

  // Pulse VMA over one clock cycle
  // Set VMA ON
  PORTG = PORTG | 0x20;

  // Wait a full clock cycle to make sure data lines are ready
  // (important for faster clocks)
  // Wait while clock is low
  while(!(PINE & 0x20));

  // Wait for a falling edge of the clock
  while((PINE & 0x20));
  
  // Wait while clock is low
  while(!(PINE & 0x20));

  byte inputData;
  inputData = (PINH & 0x78)>>3;
  inputData |= (PINB & 0x70);
  inputData |= PINJ << 7;

  // Set VMA OFF
  PORTG = PORTG & 0xDF;

  // Set R/W to LOW
  PORTE = (PORTE & 0xF7);

  // Unset address lines
  PORTH = (PORTH & 0xFC);
  PORTD = (PORTD & 0xF0);
  PORTA = 0;
  PORTC = (PORTC & 0x3F);

  return inputData;
}


void WaitClockCycle(int numCycles=1) {
  for (int count=0; count<numCycles; count++) {
    // Wait while clock is low
    while(!(PINE & 0x20));
  
    // Wait for a falling edge of the clock
    while((PINE & 0x20));
  }
}

#endif


void TestLightOn() {
  RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL) | 0x08);
}

void TestLightOff() {
  RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL) & 0xF7);
}



void InitializeU10PIA() {
  // CA1 - Self Test Switch
  // CB1 - zero crossing detector
  // CA2 - NOR'd with display latch strobe
  // CB2 - lamp strobe 1
  // PA0-7 - output for switch bank, lamps, and BCD
  // PB0-7 - switch returns
  
  RPU_DataWrite(ADDRESS_U10_A_CONTROL, 0x38);
  // Set up U10A as output
  RPU_DataWrite(ADDRESS_U10_A, 0xFF);
  // Set bit 3 to write data
  RPU_DataWrite(ADDRESS_U10_A_CONTROL, RPU_DataRead(ADDRESS_U10_A_CONTROL)|0x04);
  // Store F0 in U10A Output
  RPU_DataWrite(ADDRESS_U10_A, 0xF0);
  
  RPU_DataWrite(ADDRESS_U10_B_CONTROL, 0x33);
  // Set up U10B as input
  RPU_DataWrite(ADDRESS_U10_B, 0x00);
  // Set bit 3 so future reads will read data
  RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL)|0x04);

}

#ifdef RPU_OS_USE_DIP_SWITCHES
void ReadDipSwitches() {
  byte backupU10A = RPU_DataRead(ADDRESS_U10_A);
  byte backupU10BControl = RPU_DataRead(ADDRESS_U10_B_CONTROL);

  // Turn on Switch strobe 5 & Read Switches
  RPU_DataWrite(ADDRESS_U10_A, 0x20);
  RPU_DataWrite(ADDRESS_U10_B_CONTROL, backupU10BControl & 0xF7);
  // Wait for switch capacitors to charge
#ifndef RPU_UPDATED_TIMING  
  WaitClockCycle(RPU_NUM_SWITCH_LOOPS);
#else 
  delayMicroseconds(RPU_SWITCH_DELAY_IN_MICROSECONDS);
#endif
  DipSwitches[0] = RPU_DataRead(ADDRESS_U10_B);
 
  // Turn on Switch strobe 6 & Read Switches
  RPU_DataWrite(ADDRESS_U10_A, 0x40);
  RPU_DataWrite(ADDRESS_U10_B_CONTROL, backupU10BControl & 0xF7);
  // Wait for switch capacitors to charge
#ifndef RPU_UPDATED_TIMING  
  WaitClockCycle(RPU_NUM_SWITCH_LOOPS);
#else 
  delayMicroseconds(RPU_SWITCH_DELAY_IN_MICROSECONDS);
#endif
  DipSwitches[1] = RPU_DataRead(ADDRESS_U10_B);

  // Turn on Switch strobe 7 & Read Switches
  RPU_DataWrite(ADDRESS_U10_A, 0x80);
  RPU_DataWrite(ADDRESS_U10_B_CONTROL, backupU10BControl & 0xF7);
  // Wait for switch capacitors to charge
#ifndef RPU_UPDATED_TIMING  
  WaitClockCycle(RPU_NUM_SWITCH_LOOPS);
#else 
  delayMicroseconds(RPU_SWITCH_DELAY_IN_MICROSECONDS);
#endif
  DipSwitches[2] = RPU_DataRead(ADDRESS_U10_B);

  // Turn on U10 CB2 (strobe 8) and read switches
  RPU_DataWrite(ADDRESS_U10_A, 0x00);
  RPU_DataWrite(ADDRESS_U10_B_CONTROL, backupU10BControl | 0x08);
  // Wait for switch capacitors to charge
#ifndef RPU_UPDATED_TIMING  
  WaitClockCycle(RPU_NUM_SWITCH_LOOPS);
#else 
  delayMicroseconds(RPU_SWITCH_DELAY_IN_MICROSECONDS);
#endif
  DipSwitches[3] = RPU_DataRead(ADDRESS_U10_B);

  RPU_DataWrite(ADDRESS_U10_B_CONTROL, backupU10BControl);
  RPU_DataWrite(ADDRESS_U10_A, backupU10A);
}
#endif

void InitializeU11PIA() {
  // CA1 - Display interrupt generator
  // CB1 - test connector pin 32
  // CA2 - lamp strobe 2
  // CB2 - solenoid bank select
  // PA0-7 - display digit enable
  // PB0-7 - solenoid data

  RPU_DataWrite(ADDRESS_U11_A_CONTROL, 0x31);
  // Set up U11A as output
  RPU_DataWrite(ADDRESS_U11_A, 0xFF);
  // Set bit 3 to write data
  RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL)|0x04);
  // Store 00 in U11A Output
  RPU_DataWrite(ADDRESS_U11_A, 0x00);
  
  RPU_DataWrite(ADDRESS_U11_B_CONTROL, 0x30);
  // Set up U11B as output
  RPU_DataWrite(ADDRESS_U11_B, 0xFF);
  // Set bit 3 so future reads will read data
  RPU_DataWrite(ADDRESS_U11_B_CONTROL, RPU_DataRead(ADDRESS_U11_B_CONTROL)|0x04);
  // Store 9F in U11B Output
  RPU_DataWrite(ADDRESS_U11_B, 0x9F);
  CurrentSolenoidByte = 0x9F;
  
}


int SpaceLeftOnSwitchStack() {
  if (SwitchStackFirst>=SWITCH_STACK_SIZE || SwitchStackLast>=SWITCH_STACK_SIZE) return 0;
  if (SwitchStackLast>=SwitchStackFirst) return ((SWITCH_STACK_SIZE-1) - (SwitchStackLast-SwitchStackFirst));
  return (SwitchStackFirst - SwitchStackLast) - 1;
}

void PushToSwitchStack(byte switchNumber) {
  if ((switchNumber>39 && switchNumber!=SW_SELF_TEST_SWITCH)) return;

  // If the switch stack last index is out of range, then it's an error - return
  if (SpaceLeftOnSwitchStack()==0) return;

  // Self test is a special case - there's no good way to debounce it
  // so if it's already first on the stack, ignore it
  if (switchNumber==SW_SELF_TEST_SWITCH) {
    if (SwitchStackLast!=SwitchStackFirst && SwitchStack[SwitchStackFirst]==SW_SELF_TEST_SWITCH) return;
  }

  SwitchStack[SwitchStackLast] = switchNumber;
  
  SwitchStackLast += 1;
  if (SwitchStackLast==SWITCH_STACK_SIZE) {
    // If the end index is off the end, then wrap
    SwitchStackLast = 0;
  }
}


byte RPU_PullFirstFromSwitchStack() {
  // If first and last are equal, there's nothing on the stack
  if (SwitchStackFirst==SwitchStackLast) return SWITCH_STACK_EMPTY;

  byte retVal = SwitchStack[SwitchStackFirst];

  SwitchStackFirst += 1;
  if (SwitchStackFirst>=SWITCH_STACK_SIZE) SwitchStackFirst = 0;

  return retVal;
}


boolean RPU_ReadSingleSwitchState(byte switchNum) {
  if (switchNum>39) return false;

  int switchByte = switchNum/8;
  int switchBit = switchNum%8;
  if ( ((SwitchesNow[switchByte])>>switchBit) & 0x01 ) return true;
  else return false;
}


int SpaceLeftOnSolenoidStack() {
  if (SolenoidStackFirst>=SOLENOID_STACK_SIZE || SolenoidStackLast>=SOLENOID_STACK_SIZE) return 0;
  if (SolenoidStackLast>=SolenoidStackFirst) return ((SOLENOID_STACK_SIZE-1) - (SolenoidStackLast-SolenoidStackFirst));
  return (SolenoidStackFirst - SolenoidStackLast) - 1;
}


void RPU_PushToSolenoidStack(byte solenoidNumber, byte numPushes, boolean disableOverride) {
  if (solenoidNumber>14) return;

  // if the solenoid stack is disabled and this isn't an override push, then return
  if (!disableOverride && !SolenoidStackEnabled) return;

  // If the solenoid stack last index is out of range, then it's an error - return
  if (SpaceLeftOnSolenoidStack()==0) return;

  for (int count=0; count<numPushes; count++) {
    SolenoidStack[SolenoidStackLast] = solenoidNumber;
    
    SolenoidStackLast += 1;
    if (SolenoidStackLast==SOLENOID_STACK_SIZE) {
      // If the end index is off the end, then wrap
      SolenoidStackLast = 0;
    }
    // If the stack is now full, return
    if (SpaceLeftOnSolenoidStack()==0) return;
  }
}

void PushToFrontOfSolenoidStack(byte solenoidNumber, byte numPushes) {
  // If the stack is full, return
  if (SpaceLeftOnSolenoidStack()==0  || !SolenoidStackEnabled) return;

  for (int count=0; count<numPushes; count++) {
    if (SolenoidStackFirst==0) SolenoidStackFirst = SOLENOID_STACK_SIZE-1;
    else SolenoidStackFirst -= 1;
    SolenoidStack[SolenoidStackFirst] = solenoidNumber;
    if (SpaceLeftOnSolenoidStack()==0) return;
  }
  
}

byte PullFirstFromSolenoidStack() {
  // If first and last are equal, there's nothing on the stack
  if (SolenoidStackFirst==SolenoidStackLast) return SOLENOID_STACK_EMPTY;
  
  byte retVal = SolenoidStack[SolenoidStackFirst];

  SolenoidStackFirst += 1;
  if (SolenoidStackFirst>=SOLENOID_STACK_SIZE) SolenoidStackFirst = 0;

  return retVal;
}


boolean RPU_PushToTimedSolenoidStack(byte solenoidNumber, byte numPushes, unsigned long whenToFire, boolean disableOverride) {
  for (int count=0; count<TIMED_SOLENOID_STACK_SIZE; count++) {
    if (!TimedSolenoidStack[count].inUse) {
      TimedSolenoidStack[count].inUse = true;
      TimedSolenoidStack[count].pushTime = whenToFire;
      TimedSolenoidStack[count].disableOverride = disableOverride;
      TimedSolenoidStack[count].solenoidNumber = solenoidNumber;
      TimedSolenoidStack[count].numPushes = numPushes;
      return true;
    }
  }
  return false;
}


void RPU_UpdateTimedSolenoidStack(unsigned long curTime) {
  for (int count=0; count<TIMED_SOLENOID_STACK_SIZE; count++) {
    if (TimedSolenoidStack[count].inUse && TimedSolenoidStack[count].pushTime<curTime) {
      RPU_PushToSolenoidStack(TimedSolenoidStack[count].solenoidNumber, TimedSolenoidStack[count].numPushes, TimedSolenoidStack[count].disableOverride);
      TimedSolenoidStack[count].inUse = false;
    }
  }
}



volatile int numberOfU10Interrupts = 0;
volatile int numberOfU11Interrupts = 0;
volatile byte InsideZeroCrossingInterrupt = 0;


void InterruptService2() {
  byte u10AControl = RPU_DataRead(ADDRESS_U10_A_CONTROL);
  if (u10AControl & 0x80) {
    // self test switch
    if (RPU_DataRead(ADDRESS_U10_A_CONTROL) & 0x80) PushToSwitchStack(SW_SELF_TEST_SWITCH);
    RPU_DataRead(ADDRESS_U10_A);
  }

  // If we get a weird interupt from U11B, clear it
  byte u11BControl = RPU_DataRead(ADDRESS_U11_B_CONTROL);
  if (u11BControl & 0x80) {
    RPU_DataRead(ADDRESS_U11_B);    
  }

  byte u11AControl = RPU_DataRead(ADDRESS_U11_A_CONTROL);
  byte u10BControl = RPU_DataRead(ADDRESS_U10_B_CONTROL);

  // If the interrupt bit on the display interrupt is on, do the display refresh
  if (u11AControl & 0x80) {
    // Backup U10A
    byte backupU10A = RPU_DataRead(ADDRESS_U10_A);
    
    // Disable lamp decoders & strobe latch
    RPU_DataWrite(ADDRESS_U10_A, 0xFF);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) | 0x08);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) & 0xF7);
#ifdef RPU_OS_USE_AUX_LAMPS
    // Also park the aux lamp board 
    RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL) | 0x08);
    RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL) & 0xF7);    
#endif

    // Blank Displays
    RPU_DataWrite(ADDRESS_U10_A_CONTROL, RPU_DataRead(ADDRESS_U10_A_CONTROL) & 0xF7);
    RPU_DataWrite(ADDRESS_U11_A, (RPU_DataRead(ADDRESS_U11_A) & 0x03) | 0x01);
    RPU_DataWrite(ADDRESS_U10_A, 0x0F);

    // Write current display digits to 5 displays
    for (int displayCount=0; displayCount<5; displayCount++) {

      if (CurrentDisplayDigit<RPU_NUM_DIGITS) {
        // The BCD for this digit is in b4-b7, and the display latch strobes are in b0-b3 (and U11A:b0)
        byte displayDataByte = ((DisplayDigits[displayCount][CurrentDisplayDigit])<<4) | 0x0F;
        byte displayEnable = ((DisplayDigitEnable[displayCount])>>CurrentDisplayDigit)&0x01;
  
        // if this digit shouldn't be displayed, then set data lines to 0xFX so digit will be blank
        if (!displayEnable) displayDataByte = 0xFF;
#ifdef RPU_OS_DIMMABLE_DISPLAYS        
        if (DisplayDim[displayCount] && DisplayOffCycle) displayDataByte = 0xFF;
#endif        
  
        // Set low the appropriate latch strobe bit
        if (displayCount<4) {
          displayDataByte &= ~(0x01<<displayCount);
        }
        // Write out the digit & strobe (if it's 0-3)
        RPU_DataWrite(ADDRESS_U10_A, displayDataByte);
        if (displayCount==4) {            
          // Strobe #5 latch on U11A:b0
          RPU_DataWrite(ADDRESS_U11_A, RPU_DataRead(ADDRESS_U11_A) & 0xFE);
        }

        // Need to delay a little to make sure the strobe is low for long enough
        WaitClockCycle(4);

        // Put the latch strobe bits back high
        if (displayCount<4) {
          displayDataByte |= 0x0F;
          RPU_DataWrite(ADDRESS_U10_A, displayDataByte);
        } else {
          RPU_DataWrite(ADDRESS_U11_A, RPU_DataRead(ADDRESS_U11_A) | 0x01);
          
          // Set proper display digit enable
#ifdef RPU_OS_USE_7_DIGIT_DISPLAYS          
          byte displayDigitsMask = (0x02<<CurrentDisplayDigit) | 0x01;
#else
          byte displayDigitsMask = (0x04<<CurrentDisplayDigit) | 0x01;
#endif          
          RPU_DataWrite(ADDRESS_U11_A, displayDigitsMask);
        }
      }
    }

    // Stop Blanking (current digits are all latched and ready)
    RPU_DataWrite(ADDRESS_U10_A_CONTROL, RPU_DataRead(ADDRESS_U10_A_CONTROL) | 0x08);

    // Restore 10A from backup
    RPU_DataWrite(ADDRESS_U10_A, backupU10A);    

    CurrentDisplayDigit = CurrentDisplayDigit + 1;
    if (CurrentDisplayDigit>=RPU_NUM_DIGITS) {
      CurrentDisplayDigit = 0;
      DisplayOffCycle ^= true;
    }
    numberOfU11Interrupts+=1;
  }

  // If the IRQ bit of U10BControl is set, do the Zero-crossing interrupt handler
  if ((u10BControl & 0x80) && (InsideZeroCrossingInterrupt==0)) {
    InsideZeroCrossingInterrupt = InsideZeroCrossingInterrupt + 1;

    byte u10BControlLatest = RPU_DataRead(ADDRESS_U10_B_CONTROL);

    // Backup contents of U10A
    byte backup10A = RPU_DataRead(ADDRESS_U10_A);

    // Latch 0xFF separately without interrupt clear
    RPU_DataWrite(ADDRESS_U10_A, 0xFF);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) | 0x08);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) & 0xF7);
    // Read U10B to clear interrupt
    RPU_DataRead(ADDRESS_U10_B);

    // Turn off U10BControl interrupts
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, 0x30);

    // Copy old switch values
    byte switchCount;
    byte startingClosures;
    byte validClosures;
    for (switchCount=0; switchCount<5; switchCount++) {
      SwitchesMinus2[switchCount] = SwitchesMinus1[switchCount];
      SwitchesMinus1[switchCount] = SwitchesNow[switchCount];

      // Enable playfield strobe
      RPU_DataWrite(ADDRESS_U10_A, 0x01<<switchCount);
      RPU_DataWrite(ADDRESS_U10_B_CONTROL, 0x34);

      // Delay for switch capacitors to charge
#ifndef RPU_UPDATED_TIMING  
      WaitClockCycle(RPU_NUM_SWITCH_LOOPS);
#else 
      delayMicroseconds(RPU_SWITCH_DELAY_IN_MICROSECONDS);
#endif
      
      // Read the switches
      SwitchesNow[switchCount] = RPU_DataRead(ADDRESS_U10_B);

      //Unset the strobe
      RPU_DataWrite(ADDRESS_U10_A, 0x00);

      // Some switches need to trigger immediate closures (bumpers & slings)
      startingClosures = (SwitchesNow[switchCount]) & (~SwitchesMinus1[switchCount]);
      boolean immediateSolenoidFired = false;
      // If one of the switches is starting to close (off, on)
      if (startingClosures) {
        // Loop on bits of switch byte
        for (byte bitCount=0; bitCount<8 && immediateSolenoidFired==false; bitCount++) {
          // If this switch bit is closed
          if (startingClosures&0x01) {
            byte startingSwitchNum = switchCount*8 + bitCount;
            // Loop on immediate switch data
            for (int immediateSwitchCount=0; immediateSwitchCount<NumGamePrioritySwitches && immediateSolenoidFired==false; immediateSwitchCount++) {
              // If this switch requires immediate action
              if (GameSwitches && startingSwitchNum==GameSwitches[immediateSwitchCount].switchNum) {
                // Start firing this solenoid (just one until the closure is validate
                PushToFrontOfSolenoidStack(GameSwitches[immediateSwitchCount].solenoid, 1);
                immediateSolenoidFired = true;
              }
            }
          }
          startingClosures = startingClosures>>1;
        }
      }

      immediateSolenoidFired = false;
      validClosures = (SwitchesNow[switchCount] & SwitchesMinus1[switchCount]) & ~SwitchesMinus2[switchCount];
      // If there is a valid switch closure (off, on, on)
      if (validClosures) {
        // Loop on bits of switch byte
        for (byte bitCount=0; bitCount<8; bitCount++) {
          // If this switch bit is closed
          if (validClosures&0x01) {
            byte validSwitchNum = switchCount*8 + bitCount;
            // Loop through all switches and see what's triggered
            for (int validSwitchCount=0; validSwitchCount<NumGameSwitches; validSwitchCount++) {

              // If we've found a valid closed switch
              if (GameSwitches && GameSwitches[validSwitchCount].switchNum==validSwitchNum) {

                // If we're supposed to trigger a solenoid, then do it
                if (GameSwitches[validSwitchCount].solenoid!=SOL_NONE) {
                  if (validSwitchCount<NumGamePrioritySwitches && immediateSolenoidFired==false) {
                    PushToFrontOfSolenoidStack(GameSwitches[validSwitchCount].solenoid, GameSwitches[validSwitchCount].solenoidHoldTime);
                  } else {
                    RPU_PushToSolenoidStack(GameSwitches[validSwitchCount].solenoid, GameSwitches[validSwitchCount].solenoidHoldTime);
                  }
                } // End if this is a real solenoid
              } // End if this is a switch in the switch table
            } // End loop on switches in switch table
            // Push this switch to the game rules stack
            PushToSwitchStack(validSwitchNum);
          }
          validClosures = validClosures>>1;
        }        
      }

      // There are no port reads or writes for the rest of the loop, 
      // so we can allow the display interrupt to fire
      interrupts();
      
      // Wait so total delay will allow lamp SCRs to get to the proper voltage
#ifndef RPU_UPDATED_TIMING  
      WaitClockCycle(RPU_NUM_LAMP_LOOPS);
#else 
      delayMicroseconds(RPU_TIMING_LOOP_PADDING_IN_MICROSECONDS);
#endif
      
      noInterrupts();
    }
    RPU_DataWrite(ADDRESS_U10_A, backup10A);

    // If we need to turn off momentary solenoids, do it first
    byte momentarySolenoidAtStart = PullFirstFromSolenoidStack();
    if (momentarySolenoidAtStart!=SOLENOID_STACK_EMPTY) {
      CurrentSolenoidByte = (CurrentSolenoidByte&0xF0) | momentarySolenoidAtStart;
      RPU_DataWrite(ADDRESS_U11_B, CurrentSolenoidByte);
    } else {
      CurrentSolenoidByte = (CurrentSolenoidByte&0xF0) | SOL_NONE;
      RPU_DataWrite(ADDRESS_U11_B, CurrentSolenoidByte);
    }

#ifndef RPU_OS_USE_AUX_LAMPS
    for (int lampBitCount = 0; lampBitCount<RPU_NUM_LAMP_BITS; lampBitCount++) {
      byte lampData = 0xF0 + lampBitCount;

      interrupts();
      RPU_DataWrite(ADDRESS_U10_A, 0xFF);
      noInterrupts();
      
      // Latch address & strobe
      RPU_DataWrite(ADDRESS_U10_A, lampData);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      

      RPU_DataWrite(ADDRESS_U10_B_CONTROL, 0x38);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      

      RPU_DataWrite(ADDRESS_U10_B_CONTROL, 0x30);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      

      // Use the inhibit lines to set the actual data to the lamp SCRs 
      // (here, we don't care about the lower nibble because the address was already latched)
      byte lampOutput = LampStates[lampBitCount];
      // Every other time through the cycle, we OR in the dim variable
      // in order to dim those lights
      if (numberOfU10Interrupts%DimDivisor1) lampOutput |= LampDim0[lampBitCount];
      if (numberOfU10Interrupts%DimDivisor2) lampOutput |= LampDim1[lampBitCount];

      RPU_DataWrite(ADDRESS_U10_A, lampOutput);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      
    }

    // Latch 0xFF separately without interrupt clear
    RPU_DataWrite(ADDRESS_U10_A, 0xFF);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) | 0x08);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) & 0xF7);

#else 

    for (int lampBitCount=0; lampBitCount<15; lampBitCount++) {
      byte lampData = 0xF0 + lampBitCount;

      interrupts();
      RPU_DataWrite(ADDRESS_U10_A, 0xFF);
      noInterrupts();
      
      // Latch address & strobe
      RPU_DataWrite(ADDRESS_U10_A, lampData);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      

      RPU_DataWrite(ADDRESS_U10_B_CONTROL, 0x38);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      

      RPU_DataWrite(ADDRESS_U10_B_CONTROL, 0x30);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      

      // Use the inhibit lines to set the actual data to the lamp SCRs 
      // (here, we don't care about the lower nibble because the address was already latched)
      byte lampOutput = LampStates[lampBitCount];
      // Every other time through the cycle, we OR in the dim variable
      // in order to dim those lights
      if (numberOfU10Interrupts%DimDivisor1) lampOutput |= LampDim0[lampBitCount];
      if (numberOfU10Interrupts%DimDivisor2) lampOutput |= LampDim1[lampBitCount];

      RPU_DataWrite(ADDRESS_U10_A, lampOutput);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      

    }
    // Latch 0xFF separately without interrupt clear
    // to park 0xFF in main lamp board
    RPU_DataWrite(ADDRESS_U10_A, 0xFF);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) | 0x08);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) & 0xF7);

    for (int lampBitCount=15; lampBitCount<22; lampBitCount++) {
      byte lampOutput = (LampStates[lampBitCount]&0xF0) | (lampBitCount-15);
      // Every other time through the cycle, we OR in the dim variable
      // in order to dim those lights
      if (numberOfU10Interrupts%DimDivisor1) lampOutput |= LampDim0[lampBitCount];
      if (numberOfU10Interrupts%DimDivisor2) lampOutput |= LampDim1[lampBitCount];

      interrupts();
      RPU_DataWrite(ADDRESS_U10_A, 0xFF);
      noInterrupts();

      RPU_DataWrite(ADDRESS_U10_A, lampOutput | 0xF0);
      RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL) | 0x08);
      RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL) & 0xF7);    
      RPU_DataWrite(ADDRESS_U10_A, lampOutput);
    }
    
    RPU_DataWrite(ADDRESS_U10_A, 0xFF);
    RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL) | 0x08);
    RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL) & 0xF7);

#endif 

    interrupts();
    noInterrupts();

    InsideZeroCrossingInterrupt = 0;
    RPU_DataWrite(ADDRESS_U10_A, backup10A);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, u10BControlLatest);

    // Read U10B to clear interrupt
    RPU_DataRead(ADDRESS_U10_B);
    numberOfU10Interrupts+=1;
  }
}


#if defined (RPU_OS_SOFTWARE_DISPLAY_INTERRUPT)

ISR(TIMER1_COMPA_vect) {    //This is the interrupt request
  // Backup U10A
  byte backupU10A = RPU_DataRead(ADDRESS_U10_A);
  
  // Disable lamp decoders & strobe latch
  RPU_DataWrite(ADDRESS_U10_A, 0xFF);
  RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) | 0x08);
  RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) & 0xF7);
#ifdef RPU_OS_USE_AUX_LAMPS
  // Also park the aux lamp board 
  RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL) | 0x08);
  RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL) & 0xF7);    
#endif

  // Blank Displays
  RPU_DataWrite(ADDRESS_U10_A_CONTROL, RPU_DataRead(ADDRESS_U10_A_CONTROL) & 0xF7);
  RPU_DataWrite(ADDRESS_U11_A, (RPU_DataRead(ADDRESS_U11_A) & 0x03) | 0x01);
  RPU_DataWrite(ADDRESS_U10_A, 0x0F);

  // Write current display digits to 5 displays
  for (int displayCount=0; displayCount<5; displayCount++) {

    if (CurrentDisplayDigit<RPU_NUM_DIGITS) {
      // The BCD for this digit is in b4-b7, and the display latch strobes are in b0-b3 (and U11A:b0)
      byte displayDataByte = ((DisplayDigits[displayCount][CurrentDisplayDigit])<<4) | 0x0F;
      byte displayEnable = ((DisplayDigitEnable[displayCount])>>CurrentDisplayDigit)&0x01;

      // if this digit shouldn't be displayed, then set data lines to 0xFX so digit will be blank
      if (!displayEnable) displayDataByte = 0xFF;
#ifdef RPU_OS_DIMMABLE_DISPLAYS        
      if (DisplayDim[displayCount] && DisplayOffCycle) displayDataByte = 0xFF;
#endif        

      // Set low the appropriate latch strobe bit
      if (displayCount<4) {
        displayDataByte &= ~(0x01<<displayCount);
      }
      // Write out the digit & strobe (if it's 0-3)
      RPU_DataWrite(ADDRESS_U10_A, displayDataByte);
      if (displayCount==4) {            
        // Strobe #5 latch on U11A:b0
        RPU_DataWrite(ADDRESS_U11_A, RPU_DataRead(ADDRESS_U11_A) & 0xFE);
      }

      // Need to delay a little to make sure the strobe is low for long enough
      WaitClockCycle(4);

      // Put the latch strobe bits back high
      if (displayCount<4) {
        displayDataByte |= 0x0F;
        RPU_DataWrite(ADDRESS_U10_A, displayDataByte);
      } else {
        RPU_DataWrite(ADDRESS_U11_A, RPU_DataRead(ADDRESS_U11_A) | 0x01);
        
        // Set proper display digit enable
#ifdef RPU_OS_USE_7_DIGIT_DISPLAYS          
        byte displayDigitsMask = (0x02<<CurrentDisplayDigit) | 0x01;
#else
        byte displayDigitsMask = (0x04<<CurrentDisplayDigit) | 0x01;
#endif          
        RPU_DataWrite(ADDRESS_U11_A, displayDigitsMask);
      }
    }
  }

  // Stop Blanking (current digits are all latched and ready)
  RPU_DataWrite(ADDRESS_U10_A_CONTROL, RPU_DataRead(ADDRESS_U10_A_CONTROL) | 0x08);

  // Restore 10A from backup
  RPU_DataWrite(ADDRESS_U10_A, backupU10A);    

  CurrentDisplayDigit = CurrentDisplayDigit + 1;
  if (CurrentDisplayDigit>=RPU_NUM_DIGITS) {
    CurrentDisplayDigit = 0;
    DisplayOffCycle ^= true;
  }
}

void InterruptService3() {
  byte u10AControl = RPU_DataRead(ADDRESS_U10_A_CONTROL);
  if (u10AControl & 0x80) {
    // self test switch
    if (RPU_DataRead(ADDRESS_U10_A_CONTROL) & 0x80) PushToSwitchStack(SW_SELF_TEST_SWITCH);
    RPU_DataRead(ADDRESS_U10_A);
  }

  // If we get a weird interupt from U11B, clear it
  byte u11BControl = RPU_DataRead(ADDRESS_U11_B_CONTROL);
  if (u11BControl & 0x80) {
    RPU_DataRead(ADDRESS_U11_B);    
  }

  byte u11AControl = RPU_DataRead(ADDRESS_U11_A_CONTROL);
  byte u10BControl = RPU_DataRead(ADDRESS_U10_B_CONTROL);

  // If the interrupt bit on the display interrupt is on, do the display refresh
  if (u11AControl & 0x80) {
    RPU_DataRead(ADDRESS_U11_A);
    numberOfU11Interrupts+=1;
  }

  // If the IRQ bit of U10BControl is set, do the Zero-crossing interrupt handler
  if ((u10BControl & 0x80) && (InsideZeroCrossingInterrupt==0)) {
    InsideZeroCrossingInterrupt = InsideZeroCrossingInterrupt + 1;

    byte u10BControlLatest = RPU_DataRead(ADDRESS_U10_B_CONTROL);

    // Backup contents of U10A
    byte backup10A = RPU_DataRead(ADDRESS_U10_A);

    // Latch 0xFF separately without interrupt clear
    RPU_DataWrite(ADDRESS_U10_A, 0xFF);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) | 0x08);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) & 0xF7);
    // Read U10B to clear interrupt
    RPU_DataRead(ADDRESS_U10_B);

    // Turn off U10BControl interrupts
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, 0x30);

    // Copy old switch values
    byte switchCount;
    byte startingClosures;
    byte validClosures;
    for (switchCount=0; switchCount<5; switchCount++) {
      SwitchesMinus2[switchCount] = SwitchesMinus1[switchCount];
      SwitchesMinus1[switchCount] = SwitchesNow[switchCount];

      // Enable playfield strobe
      RPU_DataWrite(ADDRESS_U10_A, 0x01<<switchCount);
      RPU_DataWrite(ADDRESS_U10_B_CONTROL, 0x34);

      // Delay for switch capacitors to charge
#ifndef RPU_UPDATED_TIMING  
      WaitClockCycle(RPU_NUM_SWITCH_LOOPS);
#else 
      delayMicroseconds(RPU_SWITCH_DELAY_IN_MICROSECONDS);
#endif
      
      // Read the switches
      SwitchesNow[switchCount] = RPU_DataRead(ADDRESS_U10_B);

      //Unset the strobe
      RPU_DataWrite(ADDRESS_U10_A, 0x00);

      // Some switches need to trigger immediate closures (bumpers & slings)
      startingClosures = (SwitchesNow[switchCount]) & (~SwitchesMinus1[switchCount]);
      boolean immediateSolenoidFired = false;
      // If one of the switches is starting to close (off, on)
      if (startingClosures) {
        // Loop on bits of switch byte
        for (byte bitCount=0; bitCount<8 && immediateSolenoidFired==false; bitCount++) {
          // If this switch bit is closed
          if (startingClosures&0x01) {
            byte startingSwitchNum = switchCount*8 + bitCount;
            // Loop on immediate switch data
            for (int immediateSwitchCount=0; immediateSwitchCount<NumGamePrioritySwitches && immediateSolenoidFired==false; immediateSwitchCount++) {
              // If this switch requires immediate action
              if (GameSwitches && startingSwitchNum==GameSwitches[immediateSwitchCount].switchNum) {
                // Start firing this solenoid (just one until the closure is validate
                PushToFrontOfSolenoidStack(GameSwitches[immediateSwitchCount].solenoid, 1);
                immediateSolenoidFired = true;
              }
            }
          }
          startingClosures = startingClosures>>1;
        }
      }

      immediateSolenoidFired = false;
      validClosures = (SwitchesNow[switchCount] & SwitchesMinus1[switchCount]) & ~SwitchesMinus2[switchCount];
      // If there is a valid switch closure (off, on, on)
      if (validClosures) {
        // Loop on bits of switch byte
        for (byte bitCount=0; bitCount<8; bitCount++) {
          // If this switch bit is closed
          if (validClosures&0x01) {
            byte validSwitchNum = switchCount*8 + bitCount;
            // Loop through all switches and see what's triggered
            for (int validSwitchCount=0; validSwitchCount<NumGameSwitches; validSwitchCount++) {

              // If we've found a valid closed switch
              if (GameSwitches && GameSwitches[validSwitchCount].switchNum==validSwitchNum) {

                // If we're supposed to trigger a solenoid, then do it
                if (GameSwitches[validSwitchCount].solenoid!=SOL_NONE) {
                  if (validSwitchCount<NumGamePrioritySwitches && immediateSolenoidFired==false) {
                    PushToFrontOfSolenoidStack(GameSwitches[validSwitchCount].solenoid, GameSwitches[validSwitchCount].solenoidHoldTime);
                  } else {
                    RPU_PushToSolenoidStack(GameSwitches[validSwitchCount].solenoid, GameSwitches[validSwitchCount].solenoidHoldTime);
                  }
                } // End if this is a real solenoid
              } // End if this is a switch in the switch table
            } // End loop on switches in switch table
            // Push this switch to the game rules stack
            PushToSwitchStack(validSwitchNum);
          }
          validClosures = validClosures>>1;
        }        
      }

      // There are no port reads or writes for the rest of the loop, 
      // so we can allow the display interrupt to fire
      interrupts();
      
      // Wait so total delay will allow lamp SCRs to get to the proper voltage
#ifndef RPU_UPDATED_TIMING  
      WaitClockCycle(RPU_NUM_LAMP_LOOPS);
#else 
      delayMicroseconds(RPU_TIMING_LOOP_PADDING_IN_MICROSECONDS);
#endif
      
      noInterrupts();
    }
    RPU_DataWrite(ADDRESS_U10_A, backup10A);

    // If we need to turn off momentary solenoids, do it first
    byte momentarySolenoidAtStart = PullFirstFromSolenoidStack();
    if (momentarySolenoidAtStart!=SOLENOID_STACK_EMPTY) {
      CurrentSolenoidByte = (CurrentSolenoidByte&0xF0) | momentarySolenoidAtStart;
      RPU_DataWrite(ADDRESS_U11_B, CurrentSolenoidByte);
    } else {
      CurrentSolenoidByte = (CurrentSolenoidByte&0xF0) | SOL_NONE;
      RPU_DataWrite(ADDRESS_U11_B, CurrentSolenoidByte);
    }

#ifndef RPU_OS_USE_AUX_LAMPS
    for (int lampBitCount = 0; lampBitCount<RPU_NUM_LAMP_BITS; lampBitCount++) {
      byte lampData = 0xF0 + lampBitCount;

      interrupts();
      RPU_DataWrite(ADDRESS_U10_A, 0xFF);
      noInterrupts();
      
      // Latch address & strobe
      RPU_DataWrite(ADDRESS_U10_A, lampData);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      

      RPU_DataWrite(ADDRESS_U10_B_CONTROL, 0x38);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      

      RPU_DataWrite(ADDRESS_U10_B_CONTROL, 0x30);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      

      // Use the inhibit lines to set the actual data to the lamp SCRs 
      // (here, we don't care about the lower nibble because the address was already latched)
      byte lampOutput = LampStates[lampBitCount];
      // Every other time through the cycle, we OR in the dim variable
      // in order to dim those lights
      if (numberOfU10Interrupts%DimDivisor1) lampOutput |= LampDim0[lampBitCount];
      if (numberOfU10Interrupts%DimDivisor2) lampOutput |= LampDim1[lampBitCount];

      RPU_DataWrite(ADDRESS_U10_A, lampOutput);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      
    }

    // Latch 0xFF separately without interrupt clear
    RPU_DataWrite(ADDRESS_U10_A, 0xFF);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) | 0x08);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) & 0xF7);

#else 

    for (int lampBitCount=0; lampBitCount<15; lampBitCount++) {
      byte lampData = 0xF0 + lampBitCount;

      interrupts();
      RPU_DataWrite(ADDRESS_U10_A, 0xFF);
      noInterrupts();
      
      // Latch address & strobe
      RPU_DataWrite(ADDRESS_U10_A, lampData);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      

      RPU_DataWrite(ADDRESS_U10_B_CONTROL, 0x38);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      

      RPU_DataWrite(ADDRESS_U10_B_CONTROL, 0x30);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      

      // Use the inhibit lines to set the actual data to the lamp SCRs 
      // (here, we don't care about the lower nibble because the address was already latched)
      byte lampOutput = LampStates[lampBitCount];
      // Every other time through the cycle, we OR in the dim variable
      // in order to dim those lights
      if (numberOfU10Interrupts%DimDivisor1) lampOutput |= LampDim0[lampBitCount];
      if (numberOfU10Interrupts%DimDivisor2) lampOutput |= LampDim1[lampBitCount];

      RPU_DataWrite(ADDRESS_U10_A, lampOutput);
#ifdef RPU_SLOW_DOWN_LAMP_STROBE      
      WaitClockCycle();
#endif      

    }
    // Latch 0xFF separately without interrupt clear
    // to park 0xFF in main lamp board
    RPU_DataWrite(ADDRESS_U10_A, 0xFF);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) | 0x08);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, RPU_DataRead(ADDRESS_U10_B_CONTROL) & 0xF7);

    for (int lampBitCount=15; lampBitCount<22; lampBitCount++) {
      byte lampOutput = (LampStates[lampBitCount]&0xF0) | (lampBitCount-15);
      // Every other time through the cycle, we OR in the dim variable
      // in order to dim those lights
      if (numberOfU10Interrupts%DimDivisor1) lampOutput |= LampDim0[lampBitCount];
      if (numberOfU10Interrupts%DimDivisor2) lampOutput |= LampDim1[lampBitCount];

      interrupts();
      RPU_DataWrite(ADDRESS_U10_A, 0xFF);
      noInterrupts();

      RPU_DataWrite(ADDRESS_U10_A, lampOutput | 0xF0);
      RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL) | 0x08);
      RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL) & 0xF7);    
      RPU_DataWrite(ADDRESS_U10_A, lampOutput);
    }
    
    RPU_DataWrite(ADDRESS_U10_A, 0xFF);
    RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL) | 0x08);
    RPU_DataWrite(ADDRESS_U11_A_CONTROL, RPU_DataRead(ADDRESS_U11_A_CONTROL) & 0xF7);

#endif 

    interrupts();
    noInterrupts();

    InsideZeroCrossingInterrupt = 0;
    RPU_DataWrite(ADDRESS_U10_A, backup10A);
    RPU_DataWrite(ADDRESS_U10_B_CONTROL, u10BControlLatest);

    // Read U10B to clear interrupt
    RPU_DataRead(ADDRESS_U10_B);
    numberOfU10Interrupts+=1;
  }
}



#endif 




byte RPU_SetDisplay(int displayNumber, unsigned long value, boolean blankByMagnitude, byte minDigits) {
  if (displayNumber<0 || displayNumber>4) return 0;

  byte blank = 0x00;

  for (int count=0; count<RPU_NUM_DIGITS; count++) {
    blank = blank * 2;
    if (value!=0 || count<minDigits) blank |= 1;
    DisplayDigits[displayNumber][(RPU_NUM_DIGITS-1)-count] = value%10;
    value /= 10;    
  }

  if (blankByMagnitude) DisplayDigitEnable[displayNumber] = blank;

  return blank;
}

void RPU_SetDisplayBlank(int displayNumber, byte bitMask) {
  if (displayNumber<0 || displayNumber>4) return;
  
  DisplayDigitEnable[displayNumber] = bitMask;
}

// This is confusing -
// Digit mask is like this
//   bit=   b7 b6 b5 b4 b3 b2 b1 b0
//   digit=  x  x  6  5  4  3  2  1
//   (with digit 6 being the least-significant, 1's digit
//  
// so, looking at it from left to right on the display
//   digit=  1  2  3  4  5  6
//   bit=   b0 b1 b2 b3 b4 b5

/*
void RPU_SetDisplayBlankByMagnitude(int displayNumber, unsigned long value, byte minDigits) {
  if (displayNumber<0 || displayNumber>4) return;

  DisplayDigitEnable[displayNumber] = 0x20;
  if (value>9 || minDigits>1) DisplayDigitEnable[displayNumber] |= 0x10;
  if (value>99 || minDigits>2) DisplayDigitEnable[displayNumber] |= 0x08;
  if (value>999 || minDigits>3) DisplayDigitEnable[displayNumber] |= 0x04;
  if (value>9999 || minDigits>4) DisplayDigitEnable[displayNumber] |= 0x02;
  if (value>99999 || minDigits>5) DisplayDigitEnable[displayNumber] |= 0x01;
}
*/

byte RPU_GetDisplayBlank(int displayNumber) {
  if (displayNumber<0 || displayNumber>4) return 0;
  return DisplayDigitEnable[displayNumber];
}

#if defined(RPU_OS_SOFTWARE_DISPLAY_INTERRUPT) && defined(RPU_OS_ADJUSTABLE_DISPLAY_INTERRUPT)
void RPU_SetDisplayRefreshConstant(int intervalConstant) {
  cli();
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for selected increment
  OCR1A = intervalConstant;
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();
}
#endif


/*
void RPU_SetDisplayBlankForCreditMatch(boolean creditsOn, boolean matchOn) {
  DisplayDigitEnable[4] = 0;
  if (creditsOn) DisplayDigitEnable[4] |= 0x03;
  if (matchOn) DisplayDigitEnable[4] |= 0x18;
}
*/

void RPU_SetDisplayFlash(int displayNumber, unsigned long value, unsigned long curTime, int period, byte minDigits) {
  // A period of zero toggles display every other time
  if (period) {
    if ((curTime/period)%2) {
      RPU_SetDisplay(displayNumber, value, true, minDigits);
    } else {
      RPU_SetDisplayBlank(displayNumber, 0);
    }
  }
  
}


void RPU_SetDisplayFlashCredits(unsigned long curTime, int period) {
  if (period) {
    if ((curTime/period)%2) {
      DisplayDigitEnable[4] |= 0x06;
    } else {
      DisplayDigitEnable[4] &= 0x39;
    }
  }
}


void RPU_SetDisplayCredits(int value, boolean displayOn, boolean showBothDigits) {
#ifdef RPU_OS_USE_6_DIGIT_CREDIT_DISPLAY_WITH_7_DIGIT_DISPLAYS
  DisplayDigits[4][2] = (value%100) / 10;
  DisplayDigits[4][3] = (value%10);
  byte enableMask = DisplayDigitEnable[4] & RPU_OS_MASK_SHIFT_1;
#else
  DisplayDigits[4][1] = (value%100) / 10;
  DisplayDigits[4][2] = (value%10);
  byte enableMask = DisplayDigitEnable[4] & RPU_OS_MASK_SHIFT_1;
#endif 

  if (displayOn) {
#ifdef RPU_OS_USE_6_DIGIT_CREDIT_DISPLAY_WITH_7_DIGIT_DISPLAYS
  if (value>9 || showBothDigits) enableMask |= RPU_OS_MASK_SHIFT_2;
#else
  if (value>9 || showBothDigits) enableMask |= RPU_OS_MASK_SHIFT_2; 
#endif
    else enableMask |= 0x04;
  }

  DisplayDigitEnable[4] = enableMask;
}

void RPU_SetDisplayMatch(int value, boolean displayOn, boolean showBothDigits) {
  RPU_SetDisplayBallInPlay(value, displayOn, showBothDigits);
}

void RPU_SetDisplayBallInPlay(int value, boolean displayOn, boolean showBothDigits) {
#ifdef RPU_OS_USE_6_DIGIT_CREDIT_DISPLAY_WITH_7_DIGIT_DISPLAYS
  DisplayDigits[4][5] = (value%100) / 10;
  DisplayDigits[4][6] = (value%10); 
  byte enableMask = DisplayDigitEnable[4] & RPU_OS_MASK_SHIFT_2;
#else
  DisplayDigits[4][4] = (value%100) / 10;
  DisplayDigits[4][5] = (value%10); 
  byte enableMask = DisplayDigitEnable[4] & RPU_OS_MASK_SHIFT_2;
#endif

  if (displayOn) {
#ifdef RPU_OS_USE_6_DIGIT_CREDIT_DISPLAY_WITH_7_DIGIT_DISPLAYS
  if (value>9 || showBothDigits) enableMask |= RPU_OS_MASK_SHIFT_1;
#else
  if (value>9 || showBothDigits) enableMask |= RPU_OS_MASK_SHIFT_1;
#endif
      else enableMask |= 0x20;
  }

  DisplayDigitEnable[4] = enableMask;
}


/*
void RPU_SetDisplayBIPBlank(byte digitsOn) {
  if (digitsOn==0) DisplayDigitEnable[4] &= 0x0F;
  else if (digitsOn==1) DisplayDigitEnable[4] = (DisplayDigitEnable[4] & 0x0F)|0x20;
  else if (digitsOn==2) DisplayDigitEnable[4] = (DisplayDigitEnable[4] & 0x0F)|0x30;  
}
*/

void RPU_SetDimDivisor(byte level, byte divisor) {
  if (level==1) DimDivisor1 = divisor;
  if (level==2) DimDivisor2 = divisor;
}

void RPU_SetLampState(int lampNum, byte s_lampState, byte s_lampDim, int s_lampFlashPeriod) {
  if (lampNum>=RPU_MAX_LAMPS || lampNum<0) return;
  
  if (s_lampState) {
    int adjustedLampFlash = s_lampFlashPeriod/50;
    
    if (s_lampFlashPeriod!=0 && adjustedLampFlash==0) adjustedLampFlash = 1;
    if (adjustedLampFlash>250) adjustedLampFlash = 250;
    
    // Only turn on the lamp if there's no flash, because if there's a flash
    // then the lamp will be turned on by the ApplyFlashToLamps function
    if (s_lampFlashPeriod==0) LampStates[lampNum/4] &= ~(0x10<<(lampNum%4));
    LampFlashPeriod[lampNum] = adjustedLampFlash;
  } else {
    LampStates[lampNum/4] |= (0x10<<(lampNum%4));
    LampFlashPeriod[lampNum] = 0;
  }

  if (s_lampDim & 0x01) {    
    LampDim0[lampNum/4] |= (0x10<<(lampNum%4));
  } else {
    LampDim0[lampNum/4] &= ~(0x10<<(lampNum%4));
  }

  if (s_lampDim & 0x02) {    
    LampDim1[lampNum/4] |= (0x10<<(lampNum%4));
  } else {
    LampDim1[lampNum/4] &= ~(0x10<<(lampNum%4));
  }

}


void RPU_ApplyFlashToLamps(unsigned long curTime) {
  for (int count=0; count<RPU_MAX_LAMPS; count++) {
    if ( LampFlashPeriod[count]!=0 ) {
      unsigned long adjustedLampFlash = (unsigned long)LampFlashPeriod[count] * (unsigned long)50;
      if ((curTime/adjustedLampFlash)%2) {
        LampStates[count/4] &= ~(0x10<<(count%4));
      } else {
        LampStates[count/4] |= (0x10<<(count%4));
      }
    } // end if this light should flash
  } // end loop on lights
}


void RPU_FlashAllLamps(unsigned long curTime) {
  for (int count=0; count<RPU_MAX_LAMPS; count++) {
    RPU_SetLampState(count, 1, 0, 500);  
  }

  RPU_ApplyFlashToLamps(curTime);
}

void RPU_TurnOffAllLamps() {
  for (int count=0; count<RPU_MAX_LAMPS; count++) {
    RPU_SetLampState(count, 0, 0, 0);  
  }
}

void RPU_TurnOffAttractLamps() {
  for (int count=0; count<RPU_MAX_LAMPS; count++) {
    if (count==40) {
      count = 42;
    } else if (count==43) {
      count = 44;
    } else if (count==48) {
      count = 52;
    }
    RPU_SetLampState(count, 0, 0, 0);
  }
}


void RPU_InitializeMPU() {
  // Wait for board to boot
  delayMicroseconds(50000);
  delayMicroseconds(50000);


#if (RPU_OS_HARDWARE_REV==1) or (RPU_OS_HARDWARE_REV==2)
  // Start out with everything tri-state, in case the original
  // CPU is running
  // Set data pins to input
  // Make pins 2-7 input
  DDRD = DDRD & 0x03;
  // Make pins 8-13 input
  DDRB = DDRB & 0xC0;
  // Set up the address lines A0-A5 as input (for now)
  DDRC = DDRC & 0xC0;
  unsigned long startTime = millis();
  boolean sawHigh = false;
  boolean sawLow = false;
  // for three seconds, look for activity on the VMA line (A5)
  // If we see anything, then the MPU is active so we shouldn't run
  while ((millis()-startTime)<1000) {
    if (digitalRead(A5)) sawHigh = true;
    else sawLow = true;
  }
  // If we saw both a high and low signal, then someone is toggling the 
  // VMA line, so we should hang here forever (until reset)
  if (sawHigh && sawLow) {
    Serial.print("Nano - saw High/Low, halt Arduino\n");
    while (1);
  }
  Serial.print("Nano - no High/Low, run Arduino\n");

#elif (RPU_OS_HARDWARE_REV==3)
  for (byte count=2; count<32; count++) pinMode(count, INPUT);

  // Decide if halt should be raised (based on switch) 
  pinMode(13, INPUT);
  if (digitalRead(13)==0) {
    // Switch indicates the Arduino should run, so HALT the 6800
    pinMode(14, OUTPUT); // Halt
    digitalWrite(14, LOW);
    Serial.print("Mega - switch CLOSED halt 6800, run Arduino\n");
  } else {
    // Let the 6800 run 
    pinMode(14, OUTPUT); // Halt
    digitalWrite(14, HIGH);
    Serial.print("Mega - switch OPEN run 6800, halt Arduino\n");
    while(1);
  }  
  
#endif  

  

#if (RPU_OS_HARDWARE_REV==1)
  // Arduino A0 = MPU A0
  // Arduino A1 = MPU A1
  // Arduino A2 = MPU A3
  // Arduino A3 = MPU A4
  // Arduino A4 = MPU A7
  // Arduino A5 = MPU VMA
  // Set up the address lines A0-A7 as output
  DDRC = DDRC | 0x3F;
  // Set up D13 as address line A5 (and set it low)
  DDRB = DDRB | 0x20;
  PORTB = PORTB & 0xDF;
#elif (RPU_OS_HARDWARE_REV==2) 
  // Set up the address lines A0-A7 as output
  DDRC = DDRC | 0x3F;
  // Set up D13 as address line A7 (and set it high)
  DDRB = DDRB | 0x20;
  PORTB = PORTB | 0x20;
#elif (RPU_OS_HARDWARE_REV==3) 
  pinMode(3, INPUT); // CLK
  pinMode(4, OUTPUT); // VMA
  pinMode(5, OUTPUT); // R/W
  for (byte count=6; count<13; count++) pinMode(count, INPUT); // D0-D6
  pinMode(13, INPUT); // Switch
  pinMode(14, OUTPUT); // Halt
  pinMode(15, INPUT); // D7
  for (byte count=16; count<32; count++) pinMode(count, OUTPUT); // Address lines are output
  digitalWrite(5, HIGH);  // Set R/W line high (Read)
  digitalWrite(4, LOW);  // Set VMA line LOW
#endif

#if (RPU_OS_HARDWARE_REV==1) or (RPU_OS_HARDWARE_REV==2)
  // Arduino 2 = /IRQ (input)
  // Arduino 3 = R/W (output)
  // Arduino 4 = Clk (input)
  // Arduino 5 = D0
  // Arduino 6 = D1
  // Arduino 7 = D3
  // Set up control lines & data lines
  DDRD = DDRD & 0xEB;
  DDRD = DDRD | 0xE8;

  digitalWrite(3, HIGH);  // Set R/W line high (Read)
  digitalWrite(A5, LOW);  // Set VMA line LOW
#endif 

  // Interrupt line (IRQ)
  pinMode(2, INPUT);

  // Prep the address bus (all lines zero)
  RPU_DataRead(0);

  // Set up the PIAs
  InitializeU10PIA();
  InitializeU11PIA();

  // Read values from MPU dip switches
#ifdef RPU_OS_USE_DIP_SWITCHES  
  ReadDipSwitches();
#endif 
  
  // Reset address bus
  RPU_DataRead(0);

  // Reset solenoid stack
  SolenoidStackFirst = 0;
  SolenoidStackLast = 0;

  // Reset switch stack
  SwitchStackFirst = 0;
  SwitchStackLast = 0;

  CurrentDisplayDigit = 0; 

  // Set default values for the displays
  for (int displayCount=0; displayCount<5; displayCount++) {
    for (int digitCount=0; digitCount<6; digitCount++) {
      DisplayDigits[displayCount][digitCount] = 0;
    }
    DisplayDigitEnable[displayCount] = 0x03;
#ifdef RPU_OS_DIMMABLE_DISPLAYS    
    DisplayDim[displayCount] = false;
#endif
  }

  // Turn off all lamp states
  for (int lampNibbleCounter=0; lampNibbleCounter<RPU_NUM_LAMP_BITS; lampNibbleCounter++) {
    LampStates[lampNibbleCounter] = 0xFF;
    LampDim0[lampNibbleCounter] = 0x00;
    LampDim1[lampNibbleCounter] = 0x00;
  }

  for (int lampFlashCount=0; lampFlashCount<RPU_MAX_LAMPS; lampFlashCount++) {
    LampFlashPeriod[lampFlashCount] = 0;
  }

  // Reset all the switch values 
  // (set them as closed so that if they're stuck they don't register as new events)
  byte switchCount;
  for (switchCount=0; switchCount<5; switchCount++) {
    SwitchesMinus2[switchCount] = 0xFF;
    SwitchesMinus1[switchCount] = 0xFF;
    SwitchesNow[switchCount] = 0xFF;
  }

  // Hook up the interrupt
#if not defined (RPU_OS_SOFTWARE_DISPLAY_INTERRUPT)
  attachInterrupt(digitalPinToInterrupt(2), InterruptService2, LOW);
#else
/*
  cli();
  TCCR2A|=(1<<WGM21);     //Set the CTC mode
  OCR2A=0xBA;            //Set the value for 3ms
  TIMSK2|=(1<<OCIE2A);   //Set the interrupt request
  TCCR2B|=(1<<CS22);     //Set the prescale 1/64 clock
  sei();                 //Enable interrupt
*/  

  cli();
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for selected increment
  OCR1A = RPU_OS_SOFTWARE_DISPLAY_INTERRUPT_INTERVAL;
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();
  
  attachInterrupt(digitalPinToInterrupt(2), InterruptService3, LOW);
#endif  
  RPU_DataRead(0);  // Reset address bus

  // Cleary all possible interrupts by reading the registers
  RPU_DataRead(ADDRESS_U11_A);
  RPU_DataRead(ADDRESS_U11_B);
  RPU_DataRead(ADDRESS_U10_A);
  RPU_DataRead(ADDRESS_U10_B);
  RPU_DataRead(0);  // Reset address bus

}

byte RPU_GetDipSwitches(byte index) {
#ifdef RPU_OS_USE_DIP_SWITCHES
  if (index>3) return 0x00;
  return DipSwitches[index];
#else
  return 0x00 & index;
#endif
}


void RPU_SetupGameSwitches(int s_numSwitches, int s_numPrioritySwitches, PlayfieldAndCabinetSwitch *s_gameSwitchArray) {
  NumGameSwitches = s_numSwitches;
  NumGamePrioritySwitches = s_numPrioritySwitches;
  GameSwitches = s_gameSwitchArray;
}


/*
void RPU_SetupGameLights(int s_numLights, PlayfieldLight *s_gameLightArray) {
  NumGameLights = s_numLights;
  GameLights = s_gameLightArray;
}
*/
/*
void RPU_SetContinuousSolenoids(byte continuousSolenoidMask = CONTSOL_DISABLE_FLIPPERS | CONTSOL_DISABLE_COIN_LOCKOUT) {
  CurrentSolenoidByte = (CurrentSolenoidByte&0x0F) | continuousSolenoidMask;
  RPU_DataWrite(ADDRESS_U11_B, CurrentSolenoidByte);
}
*/


void RPU_SetCoinLockout(boolean lockoutOn, byte solbit) {
  if (lockoutOn) {
    CurrentSolenoidByte = CurrentSolenoidByte & ~solbit;
  } else {
    CurrentSolenoidByte = CurrentSolenoidByte | solbit;
  }
  RPU_DataWrite(ADDRESS_U11_B, CurrentSolenoidByte);
}


void RPU_SetDisableFlippers(boolean disableFlippers, byte solbit) {
  if (disableFlippers) {
    CurrentSolenoidByte = CurrentSolenoidByte | solbit;
  } else {
    CurrentSolenoidByte = CurrentSolenoidByte & ~solbit;
  }
  
  RPU_DataWrite(ADDRESS_U11_B, CurrentSolenoidByte);
}

void RPU_SetContinuousSolenoidBit(boolean bitOn, byte solbit) {

  if (bitOn) {
    CurrentSolenoidByte = CurrentSolenoidByte | solbit;
  } else {
    CurrentSolenoidByte = CurrentSolenoidByte & ~solbit;
  }
  RPU_DataWrite(ADDRESS_U11_B, CurrentSolenoidByte);
}


byte RPU_ReadContinuousSolenoids() {
  return RPU_DataRead(ADDRESS_U11_B);
}


void RPU_DisableSolenoidStack() {
  SolenoidStackEnabled = false;
}


void RPU_EnableSolenoidStack() {
  SolenoidStackEnabled = true;
}



void RPU_CycleAllDisplays(unsigned long curTime, byte digitNum) {
  int displayDigit = (curTime/250)%10;
  unsigned long value;
#ifdef RPU_OS_USE_7_DIGIT_DISPLAYS
  value = displayDigit*1111111;
#else  
  value = displayDigit*111111;
#endif

  byte displayNumToShow = 0;
  byte displayBlank = RPU_OS_ALL_DIGITS_MASK;

  if (digitNum!=0) {
    displayNumToShow = (digitNum-1)/6;
#ifdef RPU_OS_USE_7_DIGIT_DISPLAYS
    displayBlank = (0x40)>>((digitNum-1)%7);
#else    
    displayBlank = (0x20)>>((digitNum-1)%6);
#endif    
  }

  for (int count=0; count<5; count++) {
    if (digitNum) {
      RPU_SetDisplay(count, value);
      if (count==displayNumToShow) RPU_SetDisplayBlank(count, displayBlank);
      else RPU_SetDisplayBlank(count, 0);
    } else {
      RPU_SetDisplay(count, value, true);
    }
  }
}

#ifdef RPU_OS_USE_SQUAWK_AND_TALK

void RPU_PlaySoundSquawkAndTalk(byte soundByte) {

  byte oldSolenoidControlByte, soundLowerNibble, soundUpperNibble;

  // mask further zero-crossing interrupts during this 
  noInterrupts();

  // Get the current value of U11:PortB - current solenoids
  oldSolenoidControlByte = RPU_DataRead(ADDRESS_U11_B);
  soundLowerNibble = (oldSolenoidControlByte&0xF0) | (soundByte&0x0F); 
  soundUpperNibble = (oldSolenoidControlByte&0xF0) | (soundByte/16); 
    
  // Put 1s on momentary solenoid lines
  RPU_DataWrite(ADDRESS_U11_B, oldSolenoidControlByte | 0x0F);

  // Put sound latch low
  RPU_DataWrite(ADDRESS_U11_B_CONTROL, 0x34);

  // Let the strobe stay low for a moment
  delayMicroseconds(32);

  // Put sound latch high
  RPU_DataWrite(ADDRESS_U11_B_CONTROL, 0x3C);
  
  // put the new byte on U11:PortB (the lower nibble is currently loaded)
  RPU_DataWrite(ADDRESS_U11_B, soundLowerNibble);
        
  // wait 138 microseconds
  delayMicroseconds(138);

  // put the new byte on U11:PortB (the uppper nibble is currently loaded)
  RPU_DataWrite(ADDRESS_U11_B, soundUpperNibble);

  // wait 76 microseconds
  delayMicroseconds(145);

  // Restore the original solenoid byte
  RPU_DataWrite(ADDRESS_U11_B, oldSolenoidControlByte);

  // Put sound latch low
  RPU_DataWrite(ADDRESS_U11_B_CONTROL, 0x34);

  interrupts();
}
#endif

// With hardware rev 1, this function relies on D13 being connected to A5 because it writes to address 0xA0
// A0  - A0   0
// A1  - A1   0   
// A2  - n/c  0
// A3  - A2   0
// A4  - A3   0
// A5  - D13  1
// A6  - n/c  0
// A7  - A4   1
// A8  - n/c  0
// A9  - GND  0
// A10 - n/c  0
// A11 - n/c  0
// A12 - GND  0
// A13 - n/c  0
#ifdef RPU_OS_USE_SB100
void RPU_PlaySB100(byte soundByte) {

#if (RPU_OS_HARDWARE_REV==1)
  PORTB = PORTB | 0x20;
#endif 

  RPU_DataWrite(ADDRESS_SB100, soundByte);

#if (RPU_OS_HARDWARE_REV==1)
  PORTB = PORTB & 0xDF;
#endif 
  
}

#if (RPU_OS_HARDWARE_REV==2)
void RPU_PlaySB100Chime(byte soundByte) {

  RPU_DataWrite(ADDRESS_SB100_CHIMES, soundByte);

}
#endif 
#endif


#ifdef RPU_OS_USE_DASH51
void RPU_PlaySoundDash51(byte soundByte) {

  // This device has 32 possible sounds, but they're mapped to 
  // 0 - 15 and then 128 - 143 on the original card, with bits b4, b5, and b6 reserved
  // for timing controls.
  // For ease of use, I've mapped the sounds from 0-31
  
  byte oldSolenoidControlByte, soundLowerNibble, displayWithSoundBit4, oldDisplayByte;

  // mask further zero-crossing interrupts during this 
  noInterrupts();

  // Get the current value of U11:PortB - current solenoids
  oldSolenoidControlByte = RPU_DataRead(ADDRESS_U11_B);
  oldDisplayByte = RPU_DataRead(ADDRESS_U11_A);
  soundLowerNibble = (oldSolenoidControlByte&0xF0) | (soundByte&0x0F); 
  displayWithSoundBit4 = oldDisplayByte;
  if (soundByte & 0x10) displayWithSoundBit4 |= 0x02;
  else displayWithSoundBit4 &= 0xFD;
    
  // Put 1s on momentary solenoid lines
  RPU_DataWrite(ADDRESS_U11_B, oldSolenoidControlByte | 0x0F);

  // Put sound latch low
  RPU_DataWrite(ADDRESS_U11_B_CONTROL, 0x34);

  // Let the strobe stay low for a moment
  delayMicroseconds(68);

  // put bit 4 on Display Enable 7
  RPU_DataWrite(ADDRESS_U11_A, displayWithSoundBit4);

  // Put sound latch high
  RPU_DataWrite(ADDRESS_U11_B_CONTROL, 0x3C);
  
  // put the new byte on U11:PortB (the lower nibble is currently loaded)
  RPU_DataWrite(ADDRESS_U11_B, soundLowerNibble);
        
  // wait 180 microseconds
  delayMicroseconds(180);

  // Restore the original solenoid byte
  RPU_DataWrite(ADDRESS_U11_B, oldSolenoidControlByte);

  // Restore the original display byte
  RPU_DataWrite(ADDRESS_U11_A, oldDisplayByte);

  // Put sound latch low
  RPU_DataWrite(ADDRESS_U11_B_CONTROL, 0x34);

  interrupts();
}

#endif

#if (RPU_OS_HARDWARE_REV>=2 && defined(RPU_OS_USE_SB300))

void RPU_PlaySB300SquareWave(byte soundRegister, byte soundByte) {
  RPU_DataWrite(ADDRESS_SB300_SQUARE_WAVES+soundRegister, soundByte);
}

void RPU_PlaySB300Analog(byte soundRegister, byte soundByte) {
  RPU_DataWrite(ADDRESS_SB300_ANALOG+soundRegister, soundByte);
}

#endif 

// EEProm Helper functions

void RPU_WriteByteToEEProm(unsigned short startByte, byte value) {
  EEPROM.write(startByte, value);
}

byte RPU_ReadByteFromEEProm(unsigned short startByte) {
  byte value = EEPROM.read(startByte);

  // If this value is unset, set it
  if (value==0xFF) {
    value = 0;
    RPU_WriteByteToEEProm(startByte, value);
  }
  return value;
}



unsigned long RPU_ReadULFromEEProm(unsigned short startByte, unsigned long defaultValue) {
  unsigned long value;

  value = (((unsigned long)EEPROM.read(startByte+3))<<24) | 
          ((unsigned long)(EEPROM.read(startByte+2))<<16) | 
          ((unsigned long)(EEPROM.read(startByte+1))<<8) | 
          ((unsigned long)(EEPROM.read(startByte)));

  if (value==0xFFFFFFFF) {
    value = defaultValue; 
    RPU_WriteULToEEProm(startByte, value);
  }
  return value;
}


void RPU_WriteULToEEProm(unsigned short startByte, unsigned long value) {
  EEPROM.write(startByte+3, (byte)(value>>24));
  EEPROM.write(startByte+2, (byte)((value>>16) & 0x000000FF));
  EEPROM.write(startByte+1, (byte)((value>>8) & 0x000000FF));
  EEPROM.write(startByte, (byte)(value & 0x000000FF));
}
