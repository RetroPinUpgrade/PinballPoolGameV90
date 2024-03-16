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

#ifndef RPU_CONFIG_H

/***

  Use this file to set game-specific and hardware-specific parameters

***/

// Hardware Rev 1 generally uses an Arduino Nano & (option) 74125
// Hardware Rev 2 uses an Arduino Nano, a 74155, and a 74240
// Hardware Rev 3 uses a MEGA 2560 clone
#define RPU_OS_HARDWARE_REV   3


#define RPU_OS_USE_DIP_SWITCHES 
//#define RPU_OS_USE_SQUAWK_AND_TALK
//#define RPU_OS_USE_SB100
//#define RPU_OS_USE_SB300
//#define USE_CHIMES
//#define USE_WAV_TRIGGER
//#define USE_WAV_TRIGGER_1p3
//#define RPU_OS_USE_AUX_LAMPS
//#define RPU_OS_USE_7_DIGIT_DISPLAYS
//#define RPU_OS_DIMMABLE_DISPLAYS
#define RPU_OS_SOFTWARE_DISPLAY_INTERRUPT
//#define RPU_OS_ADJUSTABLE_DISPLAY_INTERRUPT

// Fast boards might need a slower lamp strobe
//#define RPU_SLOW_DOWN_LAMP_STROBE

// Depending on the number of digits, the RPU_OS_SOFTWARE_DISPLAY_INTERRUPT_INTERVAL
// can be adjusted in order to change the refresh rate of the displays.
// The original -17 / MPU-100 boards ran at 320 Hz 
// The Alltek runs the displays at 440 Hz (probably so 7-digit displays won't flicker)
// The value below is calculated with this formula:
//       Value = (interval in ms) * (16*10^6) / (1*1024) - 1 
//          (must be <65536)
// Choose one of these values (or do whatever)
//  Value         Frequency 
//  48            318.8 Hz
//  47            325.5 Hz
//  46            332.4 Hz increments   (I use this for 6-digits displays)
//  45            339.6 Hz
//  40            381 Hz
//  35            434 Hz     (This would probably be good for 7-digit displays)
//  34            446.4 Hz      
#define RPU_OS_SOFTWARE_DISPLAY_INTERRUPT_INTERVAL  48  

#ifdef RPU_OS_USE_6_DIGIT_CREDIT_DISPLAY_WITH_7_DIGIT_DISPLAYS
#define RPU_OS_MASK_SHIFT_1            0x60
#define RPU_OS_MASK_SHIFT_2            0x0C
#else
#define RPU_OS_MASK_SHIFT_1            0x30
#define RPU_OS_MASK_SHIFT_2            0x06
#endif

#ifdef RPU_OS_USE_7_DIGIT_DISPLAYS
#define RPU_OS_MAX_DISPLAY_SCORE  9999999
#define RPU_OS_NUM_DIGITS         7
#define RPU_OS_ALL_DIGITS_MASK    0x7F
#else
#define RPU_OS_MAX_DISPLAY_SCORE  999999
#define RPU_OS_NUM_DIGITS         6
#define RPU_OS_ALL_DIGITS_MASK    0x3F
#endif

#ifdef RPU_OS_USE_AUX_LAMPS
#define RPU_NUM_LAMP_BITS 22
#define RPU_MAX_LAMPS     88
#else
#define RPU_NUM_LAMP_BITS 15
#define RPU_MAX_LAMPS     60
#endif 

#define CONTSOL_DISABLE_FLIPPERS      0x40
#define CONTSOL_DISABLE_COIN_LOCKOUT  0x20



// If this define (RPU_UPDATED_TIMING) is in here
// then the interrupt timing will be independent of the 
// MPU clock, so the RPU_NUM_SWITCH_LOOPS and RPU_NUM_LAMP_LOOPS
// won't be used
#define RPU_UPDATED_TIMING   1

#ifndef RPU_UPDATED_TIMING
// This define needs to be set for the number of loops 
// needed to get a delay of 80 us
// So, set it to (0.000080) / (1/Clock Frequency)
// Assuming Frequency = 500kHz,  40 = (0.000080) / (1/500000)
#define RPU_NUM_SWITCH_LOOPS 70
// 60 us
// So, set this to (0.000060) / (1/Clock Frequency)
#define RPU_NUM_LAMP_LOOPS   30
#else
#define RPU_SWITCH_DELAY_IN_MICROSECONDS 200
#define RPU_TIMING_LOOP_PADDING_IN_MICROSECONDS  50
#endif 

// Fast boards might need a slower lamp strobe
#define RPU_SLOW_DOWN_LAMP_STROBE  0

#define RPU_CREDITS_EEPROM_BYTE          5
#define RPU_HIGHSCORE_EEPROM_START_BYTE  1
#define RPU_AWARD_SCORE_1_EEPROM_START_BYTE      10
#define RPU_AWARD_SCORE_2_EEPROM_START_BYTE      14
#define RPU_AWARD_SCORE_3_EEPROM_START_BYTE      18
#define RPU_TOTAL_PLAYS_EEPROM_START_BYTE        26
#define RPU_TOTAL_REPLAYS_EEPROM_START_BYTE      30
#define RPU_TOTAL_HISCORE_BEATEN_START_BYTE      34
#define RPU_CHUTE_2_COINS_START_BYTE             38
#define RPU_CHUTE_1_COINS_START_BYTE             42
#define RPU_CHUTE_3_COINS_START_BYTE             46

#define RPU_CONFIG_H
#endif
