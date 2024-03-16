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

#ifndef RPU_OS_H


#define RPU_OS_MAJOR_VERSION  2
#define RPU_OS_MINOR_VERSION  0

struct PlayfieldAndCabinetSwitch {
  byte switchNum;
  byte solenoid;
  byte solenoidHoldTime;
};


// Arduino wiring
// J5       DEFINITION   ARDUINO
// (pin 34) IRQ           2		if your header doesn't have pin 34, 
//					clip a lead to the top of R134
// PIN 32 - NOT USED
// PIN 31 - GND           GND  
// PIN 30 - +5V           VIN
// PIN 29 - Not Used
// PIN 28 - Ext Mem
// PIN 27 - Phi2 (clock)  4
// PIN 26 - VMA *         A5
// PIN 25 - /RESET
// PIN 24 - /HLT *        GND
// PIN 23 - R/W *         3
// PIN 22 - A0 *          A0
// PIN 21 - A1 *          A1
// PIN 20 - A2
// PIN 19 - A3 *          A2
// PIN 18 - A4 *          A3
// PIN 17 - A5		  (13 through buffer)
// PIN 16 - A6
// PIN 15 - A7 *          A4
// PIN 14 - A8
// PIN 13 - A9 *          GND
// PIN 12 - A10           
// PIN 11 - A11           
// PIN 10 - A12 *         GND
// PIN 9 - A13            
// PIN 8 - D0             5
// PIN 7 - D1             6
// PIN 6 - D2             7 
// PIN 5 - D3             8
// PIN 4 - D4             9
// PIN 3 - D5             10
// PIN 2 - D6             11
// PIN 1 - D7             12

// PIA @ U11 = A4, A7, !A9, !A12      
// PIA @ U10 = A3, A7, !A9, !A12


#define SW_SELF_TEST_SWITCH 0x7F
#define SOL_NONE 0x0F
#define SWITCH_STACK_EMPTY  0xFF
#define CONTSOL_DISABLE_FLIPPERS      0x40
#define CONTSOL_DISABLE_COIN_LOCKOUT  0x20



// Function Prototypes

//   Initialization
void RPU_InitializeMPU(); // This function used to take clock speed as a parameter - now delays are in defines at the top of this file
void RPU_SetupGameSwitches(int s_numSwitches, int s_numPrioritySwitches, PlayfieldAndCabinetSwitch *s_gameSwitchArray);
//void RPU_SetupGameLights(int s_numLights, PlayfieldLight *s_gameLightArray);
byte RPU_GetDipSwitches(byte index);

// EEProm Helper Functions
//unsigned long RPU_ReadHighScoreFromEEProm();
//void RPU_WriteHighScoreToEEProm(unsigned long score);
byte RPU_ReadByteFromEEProm(unsigned short startByte);
void RPU_WriteByteToEEProm(unsigned short startByte, byte value);
unsigned long RPU_ReadULFromEEProm(unsigned short startByte, unsigned long defaultValue=0);
void RPU_WriteULToEEProm(unsigned short startByte, unsigned long value);

//   Swtiches
byte RPU_PullFirstFromSwitchStack();
boolean RPU_ReadSingleSwitchState(byte switchNum);

//   Solenoids
void RPU_PushToSolenoidStack(byte solenoidNumber, byte numPushes, boolean disableOverride = false);
void RPU_SetCoinLockout(boolean lockoutOn = false, byte solbit = CONTSOL_DISABLE_COIN_LOCKOUT);
void RPU_SetDisableFlippers(boolean disableFlippers = true, byte solbit = CONTSOL_DISABLE_FLIPPERS);
void RPU_SetContinuousSolenoidBit(boolean bitOn, byte solBit = 0x10);
byte RPU_ReadContinuousSolenoids();
void RPU_DisableSolenoidStack();
void RPU_EnableSolenoidStack();
boolean RPU_PushToTimedSolenoidStack(byte solenoidNumber, byte numPushes, unsigned long whenToFire, boolean disableOverride = false);
void RPU_UpdateTimedSolenoidStack(unsigned long curTime);

//   Displays
byte RPU_SetDisplay(int displayNumber, unsigned long value, boolean blankByMagnitude=false, byte minDigits=2);
void RPU_SetDisplayBlank(int displayNumber, byte bitMask);
void RPU_SetDisplayCredits(int value, boolean displayOn = true, boolean showBothDigits=true);
void RPU_SetDisplayMatch(int value, boolean displayOn = true, boolean showBothDigits=true);
void RPU_SetDisplayBallInPlay(int value, boolean displayOn = true, boolean showBothDigits=true);
void RPU_SetDisplayFlash(int displayNumber, unsigned long value, unsigned long curTime, int period=500, byte minDigits=2);
void RPU_SetDisplayFlashCredits(unsigned long curTime, int period=100);
void RPU_CycleAllDisplays(unsigned long curTime, byte digitNum=0); // Self-test function
byte RPU_GetDisplayBlank(int displayNumber);
#if defined(RPU_OS_SOFTWARE_DISPLAY_INTERRUPT) && defined(RPU_OS_ADJUSTABLE_DISPLAY_INTERRUPT)
void RPU_SetDisplayRefreshConstant(int intervalConstant);
#endif

//   Lamps
void RPU_SetLampState(int lampNum, byte s_lampState, byte s_lampDim=0, int s_lampFlashPeriod=0);
void RPU_ApplyFlashToLamps(unsigned long curTime);
void RPU_FlashAllLamps(unsigned long curTime); // Self-test function
void RPU_TurnOffAllLamps();
void RPU_TurnOffAttractLamps();
void RPU_SetDimDivisor(byte level=1, byte divisor=2); // 2 means 50% duty cycle, 3 means 33%, 4 means 25%...

//   Sound
#ifdef RPU_OS_USE_SQUAWK_AND_TALK
void RPU_PlaySoundSquawkAndTalk(byte soundByte);
#endif
#ifdef RPU_OS_USE_SB100
void RPU_PlaySB100(byte soundByte);

#if (RPU_OS_HARDWARE_REV==2)
void RPU_PlaySB100Chime(byte soundByte);
#endif 

#endif

#ifdef RPU_OS_USE_DASH51
void RPU_PlaySoundDash51(byte soundByte);
#endif

#if (RPU_OS_HARDWARE_REV>=2 && defined(RPU_OS_USE_SB300))
void RPU_PlaySB300SquareWave(byte soundRegister, byte soundByte);
void RPU_PlaySB300Analog(byte soundRegister, byte soundByte);
#endif 


//   General
byte RPU_DataRead(int address);


#ifdef RPU_CPP_FILE
  int NumGameSwitches = 0;
  int NumGamePrioritySwitches = 0;
//  int NumGameLights = 0;
  
//  PlayfieldLight *GameLights = NULL;
  PlayfieldAndCabinetSwitch *GameSwitches = NULL;
#endif


#define RPU_OS_H
#endif
