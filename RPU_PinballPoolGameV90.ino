/**************************************************************************
    Pinball Pool Game is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <https://www.gnu.org/licenses/>.
 
*/

#include "RPULite_Config.h"
#include "RPULite.h"
#include "PinballPoolGame.h"
#include "SelfTestAndAudit.h"
#include <EEPROM.h>


#define VERSION_NUMBER    90
#define DEBUG_MESSAGES     1
#define COIN_DOOR_TELEMETRY // If uncommented, coin door settings are sent to monitor on boot
//#define IN_GAME_TELEMETRY   // If uncommented, sends game status to monitor
//#define EXECUTION_MESSAGES  // If uncommented, sends game logic telemetry to monitor

#define USE_SCORE_OVERRIDES
#define USE_SCORE_ACHIEVEMENTS

// MachineState
//  0 - Attract Mode
//  negative - self-test modes
//  positive - game play
int MachineState = 0;
boolean MachineStateChanged = true;
#define MACHINE_STATE_ATTRACT         0
#define MACHINE_STATE_INIT_GAMEPLAY   1
#define MACHINE_STATE_INIT_NEW_BALL   2
#define MACHINE_STATE_UNVALIDATED     3
#define MACHINE_STATE_NORMAL_GAMEPLAY 4
#define MACHINE_STATE_COUNTDOWN_BONUS 90
#define MACHINE_STATE_MATCH_MODE      95
#define MACHINE_STATE_BALL_OVER       100
#define MACHINE_STATE_GAME_OVER       110

//
// Adjustment machine states
//
//                                                            Display value                                  
#define MACHINE_STATE_ADJUST_FREEPLAY                   -17   //  12
#define MACHINE_STATE_ADJUST_BALL_SAVE                  -18   //  13
#define MACHINE_STATE_ADJUST_NEXT_BALL_DURATION         -19   //  14
#define MACHINE_STATE_ADJUST_CHASEBALL_DURATION         -20   //  15
#define MACHINE_STATE_ADJUST_SPINNER_COMBO_DURATION     -21   //  16
#define MACHINE_STATE_ADJUST_ALLEY_MODE_DURATION        -22   //  17
#define MACHINE_STATE_ADJUST_SPINNER_THRESHOLD          -23   //  18
#define MACHINE_STATE_ADJUST_POP_THRESHOLD              -24   //  19
#define MACHINE_STATE_ADJUST_TILT_WARNING               -25   //  20
#define MACHINE_STATE_ADJUST_BALLS_OVERRIDE             -26   //  21
#define MACHINE_STATE_ADJUST_DONE                       -27   //  22

//
//  EEPROM Save Locations
//

#define EEPROM_FREE_PLAY_BYTE                           100
#define EEPROM_BALL_SAVE_BYTE                           101
#define EEPROM_CHASEBALL_DURATION_BYTE                  102
#define EEPROM_SPINNER_COMBO_DURATION_BYTE              103
#define EEPROM_SPINNER_THRESHOLD_BYTE                   104
#define EEPROM_POP_THRESHOLD_BYTE                       105
#define EEPROM_NEXT_BALL_DURATION_BYTE                  106
#define EEPROM_TILT_WARNING_BYTE                        107
#define EEPROM_BALLS_OVERRIDE_BYTE                      108
#define EEPROM_ALLEY_MODE_DURATION_BYTE                 109

#define TIME_TO_WAIT_FOR_BALL         100

#define TILT_WARNING_DEBOUNCE_TIME    1500

//
// Sound Effects
//

#define SOUND_EFFECT_NONE                 0
#define SOUND_EFFECT_ADD_PLAYER           1
#define SOUND_EFFECT_BALL_OVER            2
#define SOUND_EFFECT_GAME_OVER            3
#define SOUND_EFFECT_MACHINE_START        4
#define SOUND_EFFECT_ADD_CREDIT           5
#define SOUND_EFFECT_PLAYER_UP            6
#define SOUND_EFFECT_GAME_START           7
#define SOUND_EFFECT_EXTRA_BALL           8
#define SOUND_EFFECT_5K_CHIME             9
#define SOUND_EFFECT_BALL_CAPTURE        10
#define SOUND_EFFECT_POP_MODE            11
#define SOUND_EFFECT_POP_100             12
#define SOUND_EFFECT_POP_1000            13
#define SOUND_EFFECT_POP_1000b           14
#define SOUND_EFFECT_POP_1000c           15
#define SOUND_EFFECT_TILT_WARNING        16
#define SOUND_EFFECT_TILTED              17
#define SOUND_EFFECT_10_PTS              18
#define SOUND_EFFECT_100_PTS             19
#define SOUND_EFFECT_1000_PTS            20
#define SOUND_EFFECT_EXTRA               21
#define SOUND_EFFECT_SPINNER_COMBO       22
#define SOUND_EFFECT_KICKER_OUTLANE      23
#define SOUND_EFFECT_8_BALL_CAPTURE      24
#define SOUND_EFFECT_BALL_LOSS           25
#define SOUND_EFFECT_ROAMING_CAPTURE_I   26
#define SOUND_EFFECT_ROAMING_CAPTURE_II  27
#define SOUND_EFFECT_ROAMING_CAPTURE_III 28
#define SOUND_EFFECT_SCRAMBLE_BALL       29
#define SOUND_EFFECT_CARMEN_UP           30
#define SOUND_EFFECT_CARMEN_DOWN         31
#define SOUND_EFFECT_ROAMING_END         32
#define SOUND_EFFECT_ROAMING_COMPLETE    33
#define SOUND_EFFECT_ALLEY_I             34
#define SOUND_EFFECT_ALLEY_II            35
#define SOUND_EFFECT_ALLEY_III           36
#define SOUND_EFFECT_ALLEY_IV            37

//
// Game/machine global variables
//

unsigned long HighScore = 0;
unsigned long AwardScores[3];           // Score thresholds for awards
int Credits = 0;
int MaximumCredits = 20;
boolean FreePlayMode = false;
boolean MatchFeature = true;            //  Allows Match Feature to run

#define MAX_TILT_WARNINGS_MAX    2
#define MAX_TILT_WARNINGS_DEF    1      // Length of each segment
byte MaxTiltWarnings = MAX_TILT_WARNINGS_DEF;
byte NumTiltWarnings = 0;
unsigned long LastTiltWarningTime = 0;
boolean Tilted = false;

byte CurrentPlayer = 0;
byte CurrentBallInPlay = 1;
byte CurrentNumPlayers = 0;
unsigned long CurrentScores[4];
boolean SamePlayerShootsAgain = false;
byte CurrentAchievements[4];            // Score achievments

unsigned long CurrentTime = 0;
unsigned long BallTimeInTrough = 0;
unsigned long BallFirstSwitchHitTime = 0;

boolean BallSaveUsed = false;
#define BALLSAVENUMSECONDS_MAX   20
#define BALLSAVENUMSECONDS_DEF    0
byte BallSaveNumSeconds = BALLSAVENUMSECONDS_DEF;
#define BALLSPERGAME_MAX    5
#define BALLSPERGAME_DEF    3
#define BALLSPERGAME_MIN    3
byte BallsPerGame = BALLSPERGAME_DEF;

boolean HighScoreReplay = true;

byte Ten_Pts_Stack = 0;
byte Hundred_Pts_Stack = 0;
byte Thousand_Pts_Stack = 0;
int Silent_Thousand_Pts_Stack = 0;
byte Silent_Hundred_Pts_Stack = 0;
unsigned long ChimeScoringDelay = 0;

// Animation variables
boolean MarqueeDisabled = false;


// Flashing lamp queue
boolean FlashingLampQueueEmpty = false;

#define FLASHING_LAMP_QUEUE_SIZE 15

struct FlashingLampQueueEntry {
  byte lampNumber;
  boolean currentState;
  boolean finishState;
  unsigned long startTime;                // event start
  unsigned long nextTime;                 // next lamp transition
  unsigned long duration;                 // event length
  unsigned long startPeriod;
  unsigned long endPeriod;
};

FlashingLampQueueEntry FlashingLampQueue[FLASHING_LAMP_QUEUE_SIZE];


// Attract mode variables

// byte AttractHeadMode = 255;
byte AttractPlayfieldMode = 255;
unsigned long AttractSweepTime = 0;
byte AttractSweepLights = 1;
int SpiralIncrement = 0;
unsigned long RackDelayLength = 850;

unsigned long AttractStepTime = 0;
byte AttractStepLights = 0;

unsigned long ShowLampTimeOffset = 0; // Allows resetting of ShowLampAnimtation to start at frame 1
byte RackZigZag[15] = {24, 25, 26, 27, 28, 32, 31, 30, 29, 33, 34, 35, 37, 36, 38};

// Display variables

int CreditsDispValue = 0;       //  Value to be displayed in Credit window
byte CreditsFlashing = false;   //  Allow credits display to flash if true
int BIPDispValue = 0;           //  Value to be displayed in Ball In Play window
byte BIPFlashing = false;       //  Allow BIP display to flash if true

#define OVERRIDE_PRIORITY_NEXTBALL            10
#define OVERRIDE_PRIORITY_SPINNERCOMBO        20
#define OVERRIDE_PRIORITY_ALLEY_MODE_COMBO    25
#define OVERRIDE_PRIORITY_CHASEBALLMODE3      30
#define OVERRIDE_PRIORITY_ROAMING_MODE        40
byte OverrideScorePriority = 0; //  Priority of score overrides

//
// Global game logic variables
//
byte GameMode[4] = {1,1,1,1};                     // GameMode=1 normal play, GameMode=2 15 ball mode
byte FifteenBallCounter[4] = {0,0,0,0};           // Track how long in 15-Ball mode
unsigned long CheckBallTime = 0;                  // Debug timing variable
boolean FifteenBallQualified[4];                  // Set to true when all goals are met

byte ChaseBall = 0;                               // Game Mode 3 target ball
byte ChaseBallStage = 0;                          // Counter of Ball captured in Game Mode 3
unsigned long ChaseBallTimeStart = 0;             // Timer for ChaseBall
#define CHASEBALL_DURATION_MAX    31
#define CHASEBALL_DURATION_DEF    25              // Length of each segment
#define CHASEBALL_DURATION_MIN    19
byte ChaseBallDuration = CHASEBALL_DURATION_DEF;  // Length of each segment (seconds)


byte BankShotProgress = 0;                        // Track what Bank Shot step we are on
boolean SuperBonusLit  = false;                   // True if we have achieved rack of 8 balls
boolean EightBallTest[4] = {true,true,true,true}; // One time check per player.
unsigned long BankShotOffTime = 0;
unsigned long MarqueeOffTime = 0;
byte MarqueeMultiple = 0;

#define KICKER_SAVE_DURATION             3000             // How long does kicker saver remain working
boolean SpinnerKickerLit  = false;                        // Triggers spinner light and scoring, sets kicker on
unsigned long KickerOffTime = 0;                          //  Delayed off time for Kicker
boolean KickerReady = false;
boolean KickerUsed = true;
unsigned int SpinnerCount[4] = {0,0,0,0};                 // Spinner counter
byte SpinnerMode[4] = {0,0,0,0};                          // Spinner mode threshold
unsigned long SuperSpinnerTime = 0;                       // Start of SuperSpinner mode
unsigned long SuperSpinnerDuration = 0;                   // Length of SuperSpinner mode once triggered
#define SPINNER_COMBO_DURATION_MAX   16
#define SPINNER_COMBO_DURATION_DEF   12                   // Default for EEPROM settings - sec
#define SPINNER_COMBO_DURATION_MIN    8
byte Spinner_Combo_Duration = SPINNER_COMBO_DURATION_DEF; // Duration in seconds

boolean SuperSpinnerAllowed[4] = {false, false, false, false};        // Locks out SuperSpinner mode
boolean SpinnerComboHalf = false;               // True if BankShot half of combo is hit
unsigned int SpinnerDelta = 0;                  // Increment for spinner sound effect
#define SPINNER_THRESHOLD_MAX    70
#define SPINNER_THRESHOLD_DEF    50             // Default for EEPROM settings - spins
#define SPINNER_THRESHOLD_MIN    30
byte Spinner_Threshold = SPINNER_THRESHOLD_DEF; // Spinner hits to achieve mode - 50


unsigned int PopCount[4] = {0,0,0,0};       // Pop bumper counter
byte PopMode[4] = {0,0,0,0};                // Pop bumper scoring threshold
unsigned int PopDelta = 0;
#define POP_THRESHOLD_MAX        50
#define POP_THRESHOLD_DEF        40         // Default for EEPROM settings - hits
#define POP_THRESHOLD_MIN        30
byte Pop_Threshold = POP_THRESHOLD_DEF;     // Pop bumper hits to achieve mode - 40

boolean ArrowsLit[4] = {false,false,false,false}; // True when lanes 1-4 are collected
boolean ArrowTest = true;                   // One time check flag
boolean RightArrow = false;                 // Always start with Right arrow

unsigned int Balls[4]={0,0,0,0};            // Ball status
byte Goals[4]={0,0,0,0};
boolean GoalsDisplayToggle;                 // Display achieved goals if true

boolean OutlaneSpecial[4] = {false,false,false,false};  // True when Balls 1 thru 7 are collected, then 8 ball collected
byte BonusMult = 1;                         // Bonus multiplier, defaults to 1X
boolean BonusMultTenX = false;              // Set when 10X achieved

#define NEXT_BALL_DURATION_MAX  19
#define NEXT_BALL_DURATION_DEF  15              // Default for EEPROM settings - sec
#define NEXT_BALL_DURATION_MIN  11
byte NextBallDuration = NEXT_BALL_DURATION_DEF; // Length in msec next ball award is available

unsigned long NextBallTime = 0;
byte NextBall=0;                            // Keeps track of ball after current one being captured

byte MatchDigit = 0;                        // Relocated to make global


//
// Mode 4 - Roaming Lamps Mode Variables
//

unsigned int RoamingBalls = 0;                          // Letter status
byte RoamingStage = 0;
  // 0 - Mode inactive
  // 1 - Mode initial animation
  // 2 - Mode active
  // 3 - Mode Wrap-up animation
  // 4 - Mode completion animation

byte RoamingScores[15] = {5, 6, 7, 8, 10, 12, 14, 17, 20, 25, 31, 39, 50, 66, 100}; // Scores for each letter obtained (1000's)
//byte RoamingScores[15] = {5, 6, 8, 10, 12, 15, 18, 22, 28, 34, 43, 53, 65, 81, 100}; // Scores for each letter obtained (1000's)

#define ROAMING_STAGE_DURATION_MAX          25
#define ROAMING_STAGE_DURATION_DEF          19
#define ROAMING_STAGE_DURATION_MIN          13
byte RoamingStageDuration = ROAMING_STAGE_DURATION_DEF;

#define ROAMING_ROTATE_LAMP_LIMIT              11         // After collecting x lamps RoamingRotate stops working
#define ROAMING_WRAP_UP_DURATION             1400
boolean WrapUpSoundPlayed = false;                        // Toggle for closing sound effect
boolean ReverseRoam = false;
byte NumCapturedLetters = 0;                              // Track total
unsigned long RoamingRotateTime = 0;                      // Letter stepping timer
unsigned long RoamingModeTime = 0;                        // Time before mode ends
// Lamp rotation intervals for 2nd to 15th ball, prior is just an animation
unsigned long RoamingIntervals[14] = {2500, 2013, 1620, 1304, 1050, 845, 680,
                                      548, 441, 355, 286, 230, 185, 149};

//unsigned long RoamingIntervals[14] = {3000, 2412, 1939, 1559, 1254, 1008,
//                                      810, 651, 524, 421, 339, 272, 219, 176};
//unsigned long RoamingIntervals[14] = {3000, 2514, 2106, 1764, 1479, 1239, 1038,
//                                      870, 729, 612, 513, 429, 360, 300};
//unsigned long RoamingIntervals[14] = {1000, 838, 702, 588, 493, 413, 346, 290, 243, 204, 171, 143, 120, 100};
// byte RoamingChimeIncrement = 0;                           // Increment chimes for each rotation of lamps


// Alley Mode

#define ALLEY_MODE_DURATION_MAX            25
#define ALLEY_MODE_DURATION_DEF            19
#define ALLEY_MODE_DURATION_MIN            15
byte AlleyModeBaseTime = ALLEY_MODE_DURATION_DEF;         // FUTURE mode patterns last this long

#define ALLEY_MODE_ANIMATION_TIME          2000
#define ALLEY_MODE_ANIMATION_FINISH        2000
byte AlleyModeActive = 0;                                 // Mode active when > 0, set by trigger
unsigned long Alley_Mode_Start_Time = 0;
byte AlleyModeNumber = 0;                                 // Values correspond to alleys 1-4

byte AlleyModeSwitchCombinations[4][4] = {
{0, 1, 1, 0},
{1, 0, 1, 0},
{0, 1, 0, 1},
{1, 0, 0, 1}
};

boolean AlleyModePopsTrigger = false;
boolean AlleyModeSpinnerTrigger = false;

//
// Function prototypes
//

// Default is Speed=100, CW=true
void MarqueeAttract(byte Segment, int Speed=100, boolean CW=true, boolean priority=false);
void BankShotScoring(byte Sound=1);
void FlashingArrows(int lamparray[], int rate, int numlamps=6);
void PlaySoundEffect(byte soundEffectNum, boolean priority = false);
void ClearFlashingLampQueue(boolean emptyAndSetLamp = false);

void setup() {
  if (DEBUG_MESSAGES) {
    Serial.begin(115200);
  }

  // Tell the OS about game-specific lights and switches
  RPU_SetupGameSwitches(NUM_SWITCHES_WITH_TRIGGERS, NUM_PRIORITY_SWITCHES_WITH_TRIGGERS, TriggeredSwitches);

  if (DEBUG_MESSAGES) {
    Serial.write("Attempting to initialize the MPU\n");
  }
 
  // Set up the chips and interrupts
  RPU_InitializeMPU();
  RPU_DisableSolenoidStack();
  RPU_SetDisableFlippers(true);

  // Read parameters from EEProm
  ReadStoredParameters();
  RPU_SetCoinLockout((Credits >= MaximumCredits) ? true : false);

  byte dipBank = RPU_GetDipSwitches(0);

  // Use dip switches to set up game variables
  if (DEBUG_MESSAGES) {
    char buf[32];
    sprintf(buf, "DipBank 0 = 0x%02X\n", dipBank);
    Serial.write(buf);
  }
/*
  HighScore = RPU_ReadULFromEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, 10000);
  AwardScores[0] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_1_EEPROM_START_BYTE);
  AwardScores[1] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_2_EEPROM_START_BYTE);
  AwardScores[2] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_3_EEPROM_START_BYTE);
*/
  
  Credits = RPU_ReadByteFromEEProm(RPU_CREDITS_EEPROM_BYTE);
  if (Credits>MaximumCredits) Credits = MaximumCredits;

  //BallsPerGame = 3;

// Play Machine start tune - Have to set CurrentTime as we are not yet in the loop structure

  CurrentTime = millis();
  PlaySoundEffect(SOUND_EFFECT_MACHINE_START);

// Display SW Version

  CurrentNumPlayers = 1;
  CurrentScores[0] = VERSION_NUMBER;
  //CurrentScores[0] = 1234567;
    
  if (DEBUG_MESSAGES) {
    Serial.write("Done with setup\n");
  }

}

void ReadStoredParameters() {
  HighScore = RPU_ReadULFromEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, 10000);
  Credits = RPU_ReadByteFromEEProm(RPU_CREDITS_EEPROM_BYTE);
  if (Credits > MaximumCredits) Credits = MaximumCredits;

  ReadSetting(EEPROM_FREE_PLAY_BYTE, 0);
  FreePlayMode = (EEPROM.read(EEPROM_FREE_PLAY_BYTE)) ? true : false;

  BallSaveNumSeconds = ReadSetting(EEPROM_BALL_SAVE_BYTE, BALLSAVENUMSECONDS_DEF);
  if (BallSaveNumSeconds == 99) {                                         //  If set to 99
    BallSaveNumSeconds = BALLSAVENUMSECONDS_DEF;                          //  Set to default
    EEPROM.write(EEPROM_BALL_SAVE_BYTE, BALLSAVENUMSECONDS_DEF);          //  Write to EEPROM
  }
  if (BallSaveNumSeconds > BALLSAVENUMSECONDS_MAX) BallSaveNumSeconds = BALLSAVENUMSECONDS_MAX;


  MaxTiltWarnings = ReadSetting(EEPROM_TILT_WARNING_BYTE, MAX_TILT_WARNINGS_DEF);
  if (MaxTiltWarnings == 99) {                                            //  If set to 99
    MaxTiltWarnings = MAX_TILT_WARNINGS_DEF;                              //  Set to default
    EEPROM.write(EEPROM_TILT_WARNING_BYTE, MAX_TILT_WARNINGS_DEF);        //  Write to EEPROM
  }
  if (MaxTiltWarnings > MAX_TILT_WARNINGS_MAX) MaxTiltWarnings = MAX_TILT_WARNINGS_MAX;

  BallsPerGame = ReadSetting(EEPROM_BALLS_OVERRIDE_BYTE, BALLSPERGAME_DEF);
  if (BallsPerGame == 99) {                                               //  If set to 99
    BallsPerGame = BALLSPERGAME_DEF;                                      //  Set to default
    EEPROM.write(EEPROM_BALLS_OVERRIDE_BYTE, BALLSPERGAME_DEF);           //  Write to EEPROM
  }
  if (BallsPerGame > BALLSPERGAME_MAX) BallsPerGame = BALLSPERGAME_MAX;
  if (BallsPerGame < BALLSPERGAME_MIN) BallsPerGame = BALLSPERGAME_MIN;

#if 1
  // Use this coin door setting for Roaming mode 4, set to 0 for ChaseBall mode 3
  RoamingStageDuration = ReadSetting(EEPROM_CHASEBALL_DURATION_BYTE, ROAMING_STAGE_DURATION_DEF);
  if (RoamingStageDuration == 99) {                                            //  If set to 99
    RoamingStageDuration = ROAMING_STAGE_DURATION_DEF;                         //  Set to default
    EEPROM.write(EEPROM_CHASEBALL_DURATION_BYTE, ROAMING_STAGE_DURATION_DEF);  //  Write to EEPROM
  }
  if (RoamingStageDuration > ROAMING_STAGE_DURATION_MAX) RoamingStageDuration = ROAMING_STAGE_DURATION_MAX;
  if (RoamingStageDuration < ROAMING_STAGE_DURATION_MIN) RoamingStageDuration = ROAMING_STAGE_DURATION_MIN;

#else
  ChaseBallDuration = ReadSetting(EEPROM_CHASEBALL_DURATION_BYTE, CHASEBALL_DURATION_DEF);
  if (ChaseBallDuration == 99) {                                               //  If set to 99
    ChaseBallDuration = CHASEBALL_DURATION_DEF;                                //  Set to default
    EEPROM.write(EEPROM_CHASEBALL_DURATION_BYTE, CHASEBALL_DURATION_DEF);      //  Write to EEPROM
  }
  if (ChaseBallDuration > CHASEBALL_DURATION_MAX) ChaseBallDuration = CHASEBALL_DURATION_MAX;
  if (ChaseBallDuration < CHASEBALL_DURATION_MIN) ChaseBallDuration = CHASEBALL_DURATION_MIN;

#endif

  AlleyModeBaseTime = ReadSetting(EEPROM_ALLEY_MODE_DURATION_BYTE, ALLEY_MODE_DURATION_DEF);
  if (AlleyModeBaseTime == 99) {                                               //  If set to 99
    AlleyModeBaseTime = ALLEY_MODE_DURATION_DEF;                               //  Set to default
    EEPROM.write(EEPROM_ALLEY_MODE_DURATION_BYTE, ALLEY_MODE_DURATION_DEF);    //  Write to EEPROM
  }
  if (AlleyModeBaseTime > ALLEY_MODE_DURATION_MAX) AlleyModeBaseTime = ALLEY_MODE_DURATION_MAX;
  if (AlleyModeBaseTime < ALLEY_MODE_DURATION_MIN) AlleyModeBaseTime = ALLEY_MODE_DURATION_MIN;

  Spinner_Combo_Duration = ReadSetting(EEPROM_SPINNER_COMBO_DURATION_BYTE, SPINNER_COMBO_DURATION_DEF);
  if (Spinner_Combo_Duration == 99) {                                          //  If set to 99
    Spinner_Combo_Duration = SPINNER_COMBO_DURATION_DEF;                       //  Set to default
    EEPROM.write(EEPROM_SPINNER_COMBO_DURATION_BYTE, SPINNER_COMBO_DURATION_DEF);  //  Write to EEPROM
  }
  if (Spinner_Combo_Duration > SPINNER_COMBO_DURATION_MAX) Spinner_Combo_Duration = SPINNER_COMBO_DURATION_MAX;
  if (Spinner_Combo_Duration < SPINNER_COMBO_DURATION_MIN) Spinner_Combo_Duration = SPINNER_COMBO_DURATION_MIN;
  
  Spinner_Threshold = ReadSetting(EEPROM_SPINNER_THRESHOLD_BYTE, SPINNER_THRESHOLD_DEF);
  if (Spinner_Threshold == 99) {                                               //  If set to 99
    Spinner_Threshold = SPINNER_THRESHOLD_DEF;                                 //  Set to default
    EEPROM.write(EEPROM_SPINNER_THRESHOLD_BYTE, SPINNER_THRESHOLD_DEF);        //  Write to EEPROM
  }
  if (Spinner_Threshold > SPINNER_THRESHOLD_MAX) Spinner_Threshold = SPINNER_THRESHOLD_MAX;
  if (Spinner_Threshold < SPINNER_THRESHOLD_MIN) Spinner_Threshold = SPINNER_THRESHOLD_MIN;
    
  Pop_Threshold = ReadSetting(EEPROM_POP_THRESHOLD_BYTE, POP_THRESHOLD_DEF);
  if (Pop_Threshold == 99) {                                                   //  If set to 99
    Pop_Threshold = POP_THRESHOLD_DEF;                                         //  Set to default
    EEPROM.write(EEPROM_POP_THRESHOLD_BYTE, POP_THRESHOLD_DEF);                //  Write to EEPROM
  }
  if (Pop_Threshold > POP_THRESHOLD_MAX) Pop_Threshold = POP_THRESHOLD_MAX;
  if (Pop_Threshold < POP_THRESHOLD_MIN) Pop_Threshold = POP_THRESHOLD_MIN;
    
  NextBallDuration = ReadSetting(EEPROM_NEXT_BALL_DURATION_BYTE, NEXT_BALL_DURATION_DEF);
  if (NextBallDuration == 99) {                                                //  If set to 99
    NextBallDuration = NEXT_BALL_DURATION_DEF;                                 //  Set to default
    EEPROM.write(EEPROM_NEXT_BALL_DURATION_BYTE, NEXT_BALL_DURATION_DEF);      //  Write to EEPROM
  }
  if (NextBallDuration > NEXT_BALL_DURATION_MAX) NextBallDuration = NEXT_BALL_DURATION_MAX;
  if (NextBallDuration < NEXT_BALL_DURATION_MIN) NextBallDuration = NEXT_BALL_DURATION_MIN;
  
  AwardScores[0] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_1_EEPROM_START_BYTE);
  AwardScores[1] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_2_EEPROM_START_BYTE);
  AwardScores[2] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_3_EEPROM_START_BYTE);

}

byte ReadSetting(byte setting, byte defaultValue) {
  byte value = EEPROM.read(setting);
  if (value == 0xFF) {
    EEPROM.write(setting, defaultValue);
    return defaultValue;
  }
  return value;
}



//
//  ShowLampAnimation - Ver 3
//    Ver 1 - Modified from version in Meteor
//    Ver 2 - Altered to enable use of Aux board lamps
//    Ver 3 - Use baseTime offset from 0 to allow animations to start on frame 0

void ShowLampAnimation(byte animationNum[][8], byte frames, unsigned long divisor, 
unsigned long baseTime, byte subOffset, boolean dim, boolean reverse = false, 
byte keepLampOn = 99, boolean AuxLamps = false) {

  byte currentStep = ((baseTime-ShowLampTimeOffset) / divisor) % frames;
  if (reverse) currentStep = (frames - 1) - currentStep;

  byte lampNum = 0;
  for (int byteNum = 0; byteNum < ((AuxLamps)?2:8); byteNum++) {
//  for (int byteNum = 0; byteNum < 8; byteNum++) {
    for (byte bitNum = 0; bitNum < 8; bitNum++) {

      // if there's a subOffset, turn off lights at that offset
      if (subOffset) {
        byte lampOff = true;
        lampOff = animationNum[(currentStep + subOffset) % frames][byteNum] & (1 << bitNum);
        if (lampOff && lampNum != keepLampOn) RPU_SetLampState((lampNum + ((AuxLamps)?60:0) ), 0);
//        if (lampOff && lampNum != keepLampOn) RPU_SetLampState(lampNum, 0);
      }

      byte lampOn = false;
      lampOn = animationNum[currentStep][byteNum] & (1 << bitNum);
      if (lampOn) RPU_SetLampState((lampNum + ((AuxLamps)?60:0)), 1, dim);
//      if (lampOn) RPU_SetLampState(lampNum, 1, dim);

      lampNum += 1;
    }
#if not defined (RPU_OS_SOFTWARE_DISPLAY_INTERRUPT)
    if (byteNum % 2) RPU_DataRead(0);
#endif
  }
}


// Mata Hari version - includes setting of game player lamps on backglass
//
//    PLAYER_x_UP are the lamps adjacent to each display
//    PLAYER_X are the lamps at bottom left of backglass indicating how many player game is running
//

void SetPlayerLamps(byte numPlayers, byte playerOffset = 0, int flashPeriod = 0) {
  // For Mata Hari, the "Player Up" lights are all +4 of the "Player" lights
  // so this function covers both sets of lights. Putting a 4 in playerOffset
  // will turn on/off the player up lights.
  // Offset of 0 means lower backglass lamps, offset of 4 mean player lamps next to the score displays
  for (int count = 0; count < 4; count++) {
    RPU_SetLampState(LA_1_PLAYER + playerOffset + count, (numPlayers == (count + 1)) ? 1 : 0, 0, flashPeriod);
  }
}

//Meteor version - updates audit EEPROM memory locations
void AddCoinToAudit(byte switchHit) {

  unsigned short coinAuditStartByte = 0;

  switch (switchHit) {
    case SW_COIN_3: coinAuditStartByte = RPU_CHUTE_3_COINS_START_BYTE; break;
    case SW_COIN_2: coinAuditStartByte = RPU_CHUTE_2_COINS_START_BYTE; break;
    case SW_COIN_1: coinAuditStartByte = RPU_CHUTE_1_COINS_START_BYTE; break;
  }

  if (coinAuditStartByte) {
    RPU_WriteULToEEProm(coinAuditStartByte, RPU_ReadULFromEEProm(coinAuditStartByte) + 1);
  }

}


// Mata Hari version (and Meteor)
void AddCredit(boolean playSound = false, byte numToAdd = 1) {
  if (Credits < MaximumCredits) {
    Credits += numToAdd;
    if (Credits > MaximumCredits) Credits = MaximumCredits;
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    if (playSound) PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT, false);
    CreditsDispValue = Credits;
    //RPU_SetDisplayCredits(Credits);
    RPU_SetCoinLockout(false);
  } else {
    CreditsDispValue = Credits;
    //RPU_SetDisplayCredits(Credits);
    RPU_SetCoinLockout(true);
  }
}

void AddSpecialCredit() {
  AddCredit(false, 1);
  RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime, true);
  RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + 1);  
}

// Meteor version - uses updated function call in Attract Mode to allow game start following a
// a 4 player game
boolean AddPlayer(boolean resetNumPlayers = false) {

  if (Credits < 1 && !FreePlayMode) return false;
  if (resetNumPlayers) CurrentNumPlayers = 0;
  if (CurrentNumPlayers >= 4) return false;

  CurrentNumPlayers += 1;
  RPU_SetDisplay(CurrentNumPlayers - 1, 0);
  RPU_SetDisplayBlank(CurrentNumPlayers - 1, 0x30);

  if (!FreePlayMode) {
    Credits -= 1;
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    CreditsDispValue = Credits;
    //RPU_SetDisplayCredits(Credits);
    RPU_SetCoinLockout(false);
  }
  if (CurrentNumPlayers > 1){
  PlaySoundEffect(SOUND_EFFECT_ADD_PLAYER, true);
  }
  //Serial.println(F("AddPlayer - SetPlayerLamps(CurrentNumPlayers)"));
  SetPlayerLamps(CurrentNumPlayers);

  RPU_WriteULToEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE) + 1);

  return true;
}



int InitNewBall(bool curStateChanged, byte playerNum, int ballNum) {  //zzzzz

  if (curStateChanged) {

    //
    // Set pre-ball conditions & variables
    //

    Serial.println(F("----------InitNewBall----------"));
    
    CheckBallTime = CurrentTime;
    BallFirstSwitchHitTime = 0;
    MarqueeOffTime = 0;

    CreditsDispValue = Credits;

    SetPlayerLamps(playerNum+1, 4, 500);

    NumTiltWarnings = 0;
    LastTiltWarningTime = 0;
    Tilted = false;

    BallSaveUsed = false;
    if (BallSaveNumSeconds>0) {
      RPU_SetLampState(SAME_PLAYER, 1, 0, 500);
    }
    
    ArrowTest = true;
    RPU_SetLampState(LA_LEFT_ARROW, 0);  // Reset arrows
    RPU_SetLampState(LA_RIGHT_ARROW, 0);  // Reset arrows 

    SpinnerKickerLit  = false;
    KickerUsed = true;
    KickerReady = false;
    SuperSpinnerTime = 0;
    SuperSpinnerDuration = Spinner_Combo_Duration;
    SpinnerComboHalf = false;
    SpinnerDelta = 0;

    PopDelta = 0;

    NextBallTime = 0;
    NextBall = 0;

    // AlleyMode - reset triggers
    
    AlleyModePopsTrigger = false;
    AlleyModeSpinnerTrigger = false;

    RPU_SetLampState(LA_SPINNER, 0);       // Spinner and Kicker are off at beginning of a new ball

    BankShotProgress = 0;                   // Reset Bank shot for each ball
    BankShotLighting();                     // Set Lights for ball start

    BonusMultiplier(1);                     // Reset bonus multiplier and lights
    BonusMultTenX = false;                  // Reset flag
    
    Ten_Pts_Stack = 0;                      // Reset Scoring variables
    Hundred_Pts_Stack = 0;
    Thousand_Pts_Stack = 0;
    Silent_Thousand_Pts_Stack = 0;
    Silent_Hundred_Pts_Stack = 0;
    ChimeScoringDelay = 0;

    ClearFlashingLampQueue();                      // Clear FlashingLampQueue only

    //
    //  Chase Ball - Mode 3
    //
    
    ChaseBall = 0;                                 //  Game Mode 3 target ball
    ChaseBallStage = 0;                            //  Counter of Ball captured in Game Mode 3
    ChaseBallTimeStart = 0;                        //  Timer for ChaseBall

    // ComboShots and modes
    
    // Alley mode
    AlleyModeActive = 0;                           // Mode active or not, set by spinner trigger
    Alley_Mode_Start_Time = 0;
    AlleyModeNumber = 0;                           // Values correspond to alley lanes 1-4, 9-12

    //
    //  SuperBonus
    //

    if (OutlaneSpecial[CurrentPlayer] == false) {
      RPU_SetLampState(LA_OUTLANE_SPECIAL, 0);                    // Turn off lamp
    } else {
      RPU_SetLampState(LA_OUTLANE_SPECIAL, 1);                    // If current player is true, turn on lamp
    }
    if (GameMode[CurrentPlayer] == 1) {
      RPU_SetLampState(LA_SUPER_BONUS, 0);                        // Turn off SuperBonus lamp
      if (Balls[CurrentPlayer] & (0b1<<15)) {                     // If player already has SuperBonus
        RPU_SetLampState(LA_SUPER_BONUS, 1);                      // Turn on SuperBonus lamp
      }
      if ((Balls[CurrentPlayer] & (0b1<<7)) == 128) {             // If 8 ball collected, we have achieved SuperBonus
        RPU_SetLampState(LA_SUPER_BONUS, 1);                      // Turn on SuperBonus lamp
        Balls[CurrentPlayer] = 0x8000;                            // Wipe out all collected balls, set SuperBonus
        SetGoals(1);                                              // Duplicate of above
        EightBallTest[CurrentPlayer] = true;                      // Reset to enable getting 8 ball again
        if (!OutlaneSpecial[CurrentPlayer]) {                     // Achieving SuperBonus turns on Outlane Special
          OutlaneSpecial[CurrentPlayer] = true;
          RPU_SetLampState(LA_OUTLANE_SPECIAL, 1);                // Turn lamp on, signifying mode is active
        }
        ArrowsLit[CurrentPlayer] = false;
      }
    }


//
//  15 Ball Mode
//

    if (GameMode[CurrentPlayer] == 2) {                 // if 15 Ball mode already active
      FifteenBallCounter[CurrentPlayer] += 1;
      RPU_SetLampState(LA_SUPER_BONUS, 1, 0, 750);      // Turn on slow blink for Mode 2
    }
    if (FifteenBallCounter[CurrentPlayer] > 2) {        // Maximum balls in Mode 2
      GameMode[CurrentPlayer] = 1;                      // Reset to Mode 1
      PlaySoundEffect(SOUND_EFFECT_ROAMING_COMPLETE, true); // Mode end sound effect
      FifteenBallCounter[CurrentPlayer] = 0;            // Reset counter
      Balls[CurrentPlayer] = 0;                         // Reset all balls plus SuperBonus
      Goals[CurrentPlayer] = 0;                         // Reset all goals
      PopMode[CurrentPlayer] = 0;                       // Reset
      PopCount[CurrentPlayer] = 0;                      // Reset
      SpinnerMode[CurrentPlayer] = 0;                   // Reset
      SpinnerCount[CurrentPlayer] = 0;                  // Reset
      CurrentScores[CurrentPlayer] = CurrentScores[CurrentPlayer]/10*10;  // Remove goals
      RPU_SetLampState(LA_SUPER_BONUS, 0);              // Turn off
    }

    if (FifteenBallQualified[CurrentPlayer]) {
      FifteenBallQualified[CurrentPlayer] = false;      // Turn off
      GameMode[CurrentPlayer] = 2;                      // Set Game Mode to 15 Ball
      Balls[CurrentPlayer] = 0;                         // Reset all balls plus SuperBonus
      RPU_SetLampState(LA_SUPER_BONUS, 1, 0, 750);      // Turn on slow blink for Mode 2
      EightBallTest[CurrentPlayer] = false;             // Turn off for Mode 2
      ArrowsLit[CurrentPlayer] = false;                 // Turn off
      FifteenBallCounter[CurrentPlayer] = 1;            // Set to 1
    }

    SamePlayerShootsAgain = false;
    RPU_SetLampState(SAME_PLAYER, 0);
    RPU_SetLampState(LA_SPSA_BACK, 0);        

    RPU_SetDisableFlippers(false);
    RPU_EnableSolenoidStack(); 

    MarqueeDisabled = false;                        // In case SetGoals turned this off.
    //Serial.println("InitNewBall - BallLighting");
    BallLighting();                                 // Relight rack balls already collected
    
    if (RPU_ReadSingleSwitchState(SW_OUTHOLE)) {
      RPU_PushToTimedSolenoidStack(SOL_OUTHOLE, 4, CurrentTime + 100);
    }

    //ShowPlayerScores(0xFF, false, false); // Show player scores
    for (int count=0; count<CurrentNumPlayers; count++) {
      RPU_SetDisplay(count, CurrentScores[count], true, 2);
    }

    BIPDispValue = ballNum;
    RPU_SetLampState(BALL_IN_PLAY, 1);
    RPU_SetLampState(TILT, 0);

    CreditsFlashing = false;   //  credits display on steady
    BIPFlashing = false;       //  BIP display on steady

    OverrideScorePriority = 0; //  Set to default

  }
  
  // We should only consider the ball initialized when 
  // the ball is no longer triggering the SW_OUTHOLE
  if (RPU_ReadSingleSwitchState(SW_OUTHOLE)) {
    //Serial.println(F("--InitNewBall, ball still in outhole--"));
    return MACHINE_STATE_INIT_NEW_BALL;
  } else {
    return MACHINE_STATE_NORMAL_GAMEPLAY;
  }
  
}

////////////////////////////////////////////////////////////////////////////
//
//  Self test, audit, adjustments mode
//
////////////////////////////////////////////////////////////////////////////

#define ADJ_TYPE_LIST                 1
#define ADJ_TYPE_MIN_MAX              2
#define ADJ_TYPE_MIN_MAX_DEFAULT      3
#define ADJ_TYPE_SCORE                4
#define ADJ_TYPE_SCORE_WITH_DEFAULT   5
#define ADJ_TYPE_SCORE_NO_DEFAULT     6
byte AdjustmentType = 0;
byte NumAdjustmentValues = 0;
byte AdjustmentValues[8];
unsigned long AdjustmentScore;
byte *CurrentAdjustmentByte = NULL;
unsigned long *CurrentAdjustmentUL = NULL;
byte CurrentAdjustmentStorageByte = 0;
byte TempValue = 0;


int RunSelfTest(int curState, boolean curStateChanged) {
  int returnState = curState;
  CurrentNumPlayers = 0;

#if 0
  if (curStateChanged) {
    // Send a stop-all command and reset the sample-rate offset, in case we have
    //  reset while the WAV Trigger was already playing.
    StopAudio();
    PlaySoundEffect(SOUND_EFFECT_SELF_TEST_MODE_START-curState, 0);
  } else {
    if (SoundSettingTimeout && CurrentTime>SoundSettingTimeout) {
      SoundSettingTimeout = 0;
      StopAudio();
    }
  }
#endif


  // Any state that's greater than CHUTE_3 is handled by the Base Self-test code
  // Any that's less, is machine specific, so we handle it here.
  if (curState >= MACHINE_STATE_TEST_CHUTE_3_COINS) {
    returnState = RunBaseSelfTest(returnState, curStateChanged, CurrentTime, SW_CREDIT_RESET, SW_SLAM);
  } else {
    byte curSwitch = RPU_PullFirstFromSwitchStack();

    if (curSwitch == SW_SELF_TEST_SWITCH && (CurrentTime - GetLastSelfTestChangedTime()) > 250) {
      SetLastSelfTestChangedTime(CurrentTime);
      returnState -= 1;
    }

    if (curSwitch == SW_SLAM) {
      returnState = MACHINE_STATE_ATTRACT;
    }

    if (curStateChanged) {
      for (int count = 0; count < 4; count++) {
        RPU_SetDisplay(count, 0);
        RPU_SetDisplayBlank(count, 0x00);
      }
      Serial.print(F("Current Machine State is: "));
      Serial.println(curState, DEC);
      RPU_SetDisplayCredits(MACHINE_STATE_TEST_SOUNDS - curState);
      RPU_SetDisplayBallInPlay(0, false);
      CurrentAdjustmentByte = NULL;
      CurrentAdjustmentUL = NULL;
      CurrentAdjustmentStorageByte = 0;

      AdjustmentType = ADJ_TYPE_MIN_MAX;
      AdjustmentValues[0] = 0;
      AdjustmentValues[1] = 1;
      TempValue = 0;

      switch (curState) {
        case MACHINE_STATE_ADJUST_FREEPLAY:
          CurrentAdjustmentByte = (byte *)&FreePlayMode;
          CurrentAdjustmentStorageByte = EEPROM_FREE_PLAY_BYTE;
          break;
        case MACHINE_STATE_ADJUST_BALL_SAVE:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 6;
          AdjustmentValues[1] = 5;
          AdjustmentValues[2] = 10;
          AdjustmentValues[3] = 15;
          AdjustmentValues[4] = 20;
          AdjustmentValues[5] = 99;
          CurrentAdjustmentByte = &BallSaveNumSeconds;
          CurrentAdjustmentStorageByte = EEPROM_BALL_SAVE_BYTE;
          break;
        case MACHINE_STATE_ADJUST_CHASEBALL_DURATION:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 8;
          AdjustmentValues[0] = 13;
          AdjustmentValues[1] = 15;
          AdjustmentValues[2] = 17;
          AdjustmentValues[3] = 19;
          AdjustmentValues[4] = 21;
          AdjustmentValues[5] = 23;
          AdjustmentValues[6] = 25;
          AdjustmentValues[7] = 99;
          CurrentAdjustmentByte = &ChaseBallDuration;
          CurrentAdjustmentStorageByte = EEPROM_CHASEBALL_DURATION_BYTE;
          break;
        case MACHINE_STATE_ADJUST_SPINNER_COMBO_DURATION:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 6;
          AdjustmentValues[0] = 8;
          AdjustmentValues[1] = 10;
          AdjustmentValues[2] = 12;
          AdjustmentValues[3] = 14;
          AdjustmentValues[4] = 16;
          AdjustmentValues[5] = 99;
          CurrentAdjustmentByte = &Spinner_Combo_Duration;
          CurrentAdjustmentStorageByte = EEPROM_SPINNER_COMBO_DURATION_BYTE;
          break;
        case MACHINE_STATE_ADJUST_SPINNER_THRESHOLD:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 6;
          AdjustmentValues[0] = 30;
          AdjustmentValues[1] = 40;
          AdjustmentValues[2] = 50;
          AdjustmentValues[3] = 60;
          AdjustmentValues[4] = 70;
          AdjustmentValues[5] = 99;
          CurrentAdjustmentByte = &Spinner_Threshold;
          CurrentAdjustmentStorageByte = EEPROM_SPINNER_THRESHOLD_BYTE;
          break;
        case MACHINE_STATE_ADJUST_POP_THRESHOLD:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 6;
          AdjustmentValues[0] = 30;
          AdjustmentValues[1] = 35;
          AdjustmentValues[2] = 40;
          AdjustmentValues[3] = 45;
          AdjustmentValues[4] = 50;
          AdjustmentValues[5] = 99;
          CurrentAdjustmentByte = &Pop_Threshold;
          CurrentAdjustmentStorageByte = EEPROM_POP_THRESHOLD_BYTE;
          break;
        case MACHINE_STATE_ADJUST_NEXT_BALL_DURATION:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 6;
          AdjustmentValues[0] = 11;
          AdjustmentValues[1] = 13;
          AdjustmentValues[2] = 15;
          AdjustmentValues[3] = 17;
          AdjustmentValues[4] = 19;
          AdjustmentValues[5] = 99;
          CurrentAdjustmentByte = &NextBallDuration;
          CurrentAdjustmentStorageByte = EEPROM_NEXT_BALL_DURATION_BYTE;
          break;
        case MACHINE_STATE_ADJUST_TILT_WARNING:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 4;
          AdjustmentValues[0] = 0;
          AdjustmentValues[1] = 1;
          AdjustmentValues[2] = 2;
          AdjustmentValues[3] = 99;
          CurrentAdjustmentByte = &MaxTiltWarnings;
          CurrentAdjustmentStorageByte = EEPROM_TILT_WARNING_BYTE;
          break;
        case MACHINE_STATE_ADJUST_BALLS_OVERRIDE:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 3;
          AdjustmentValues[0] = 3;
          AdjustmentValues[1] = 5;
          AdjustmentValues[2] = 99;
          CurrentAdjustmentByte = &BallsPerGame;
          CurrentAdjustmentStorageByte = EEPROM_BALLS_OVERRIDE_BYTE;
          break;
        case MACHINE_STATE_ADJUST_ALLEY_MODE_DURATION:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 8;
          AdjustmentValues[0] = 15;
          AdjustmentValues[1] = 16;
          AdjustmentValues[2] = 17;
          AdjustmentValues[3] = 19;
          AdjustmentValues[4] = 21;
          AdjustmentValues[5] = 23;
          AdjustmentValues[6] = 25;
          AdjustmentValues[7] = 99;
          CurrentAdjustmentByte = &AlleyModeBaseTime;
          CurrentAdjustmentStorageByte = EEPROM_ALLEY_MODE_DURATION_BYTE;
          break;
        case MACHINE_STATE_ADJUST_DONE:
          returnState = MACHINE_STATE_ATTRACT;
          break;
      }

    }

    // Change value, if the switch is hit
    if (curSwitch == SW_CREDIT_RESET) {

      if (CurrentAdjustmentByte && (AdjustmentType == ADJ_TYPE_MIN_MAX || AdjustmentType == ADJ_TYPE_MIN_MAX_DEFAULT)) {
        byte curVal = *CurrentAdjustmentByte;
        curVal += 1;
        if (curVal > AdjustmentValues[1]) {
          if (AdjustmentType == ADJ_TYPE_MIN_MAX) curVal = AdjustmentValues[0];
          else {
            if (curVal > 99) curVal = AdjustmentValues[0];
            else curVal = 99;
          }
        }
        *CurrentAdjustmentByte = curVal;
        if (CurrentAdjustmentStorageByte) EEPROM.write(CurrentAdjustmentStorageByte, curVal);
/*        
        if (curState==MACHINE_STATE_ADJUST_SFX_AND_SOUNDTRACK) {
          StopAudio();
          PlaySoundEffect(SOUND_EFFECT_SELF_TEST_AUDIO_OPTIONS_START+curVal, 0);
          if (curVal>=3) SoundtrackSelection = curVal-3;
        } else if (curState==MACHINE_STATE_ADJUST_MUSIC_VOLUME) {
          if (SoundSettingTimeout) StopAudio();
          PlayBackgroundSong(MusicIndices[SoundtrackSelection][4] + RALLY_MUSIC_WAITING_FOR_SKILLSHOT);
          SoundSettingTimeout = CurrentTime+5000;
        } else if (curState==MACHINE_STATE_ADJUST_SFX_VOLUME) {
          if (SoundSettingTimeout) StopAudio();
          PlaySoundEffect(SOUND_EFFECT_GALAXY_LETTER_AWARD, ConvertVolumeSettingToGain(SoundEffectsVolume));
          SoundSettingTimeout = CurrentTime+5000;
        } else if (curState==MACHINE_STATE_ADJUST_CALLOUTS_VOLUME) {
          if (SoundSettingTimeout) StopAudio();
          PlaySoundEffect(SOUND_EFFECT_VP_SKILL_SHOT_1, ConvertVolumeSettingToGain(CalloutsVolume));
          SoundSettingTimeout = CurrentTime+3000;
        }*/
      } else if (CurrentAdjustmentByte && AdjustmentType == ADJ_TYPE_LIST) {
        byte valCount = 0;
        byte curVal = *CurrentAdjustmentByte;
        byte newIndex = 0;
        for (valCount = 0; valCount < (NumAdjustmentValues - 1); valCount++) {
          if (curVal == AdjustmentValues[valCount]) newIndex = valCount + 1;
        }
        *CurrentAdjustmentByte = AdjustmentValues[newIndex];
        if (CurrentAdjustmentStorageByte) EEPROM.write(CurrentAdjustmentStorageByte, AdjustmentValues[newIndex]);
      } else if (CurrentAdjustmentUL && (AdjustmentType == ADJ_TYPE_SCORE_WITH_DEFAULT || AdjustmentType == ADJ_TYPE_SCORE_NO_DEFAULT)) {
        unsigned long curVal = *CurrentAdjustmentUL;
        curVal += 5000;
        if (curVal > 100000) curVal = 0;
        if (AdjustmentType == ADJ_TYPE_SCORE_NO_DEFAULT && curVal == 0) curVal = 5000;
        *CurrentAdjustmentUL = curVal;
        if (CurrentAdjustmentStorageByte) RPU_WriteULToEEProm(CurrentAdjustmentStorageByte, curVal);
      }
/*
      if (curState == MACHINE_STATE_ADJUST_DIM_LEVEL) {
        RPU_SetDimDivisor(1, DimLevel);
      }*/
    }

    // Show current value
    if (CurrentAdjustmentByte != NULL) {
      RPU_SetDisplay(0, (unsigned long)(*CurrentAdjustmentByte), true);
    } else if (CurrentAdjustmentUL != NULL) {
      RPU_SetDisplay(0, (*CurrentAdjustmentUL), true);
    }

  }
/*
  if (curState == MACHINE_STATE_ADJUST_DIM_LEVEL) {
    //    for (int count = 0; count < 7; count++) RPU_SetLampState(MIDDLE_ROCKET_7K + count, 1, (CurrentTime / 1000) % 2);
  }*/

  if (returnState == MACHINE_STATE_ATTRACT) {
    // If any variables have been set to non-override (99), return
    // them to dip switch settings
    // Balls Per Game, Player Loses On Ties, Novelty Scoring, Award Score
    //    DecodeDIPSwitchParameters();
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    ReadStoredParameters();
  }

  return returnState;
}

////////////////////////////////////////////////////////////////////////////
//
//  Display Management functions
//
////////////////////////////////////////////////////////////////////////////
unsigned long LastTimeScoreChanged = 0;
unsigned long LastTimeOverrideAnimated = 0;
unsigned long LastFlashOrDash = 0;
#ifdef USE_SCORE_OVERRIDES
unsigned long ScoreOverrideValue[4] = {0, 0, 0, 0};
byte ScoreOverrideStatus = 0;
#define DISPLAY_OVERRIDE_BLANK_SCORE 0xFFFFFFFF
#endif
byte LastScrollPhase = 0;

byte MagnitudeOfScore(unsigned long score) {
  if (score == 0) return 0;

  byte retval = 0;
  while (score > 0) {
    score = score / 10;
    retval += 1;
  }
  return retval;
}

#ifdef USE_SCORE_OVERRIDES
void OverrideScoreDisplay(byte displayNum, unsigned long value, boolean animate) {
  if (displayNum > 3) return;
  ScoreOverrideStatus |= (0x10 << displayNum);
  if (animate) ScoreOverrideStatus |= (0x01 << displayNum);
  else ScoreOverrideStatus &= ~(0x01 << displayNum);
  ScoreOverrideValue[displayNum] = value;
}
#endif

byte GetDisplayMask(byte numDigits) {
  byte displayMask = 0;
  for (byte digitCount = 0; digitCount < numDigits; digitCount++) {
    displayMask |= (0x20 >> digitCount);
  }
  return displayMask;
}


void ShowPlayerScores(byte displayToUpdate, boolean flashCurrent, boolean dashCurrent, unsigned long allScoresShowValue = 0) {

#ifdef USE_SCORE_OVERRIDES
  if (displayToUpdate == 0xFF) ScoreOverrideStatus = 0;
#endif

  byte displayMask = 0x3F;
  unsigned long displayScore = 0;
  unsigned long overrideAnimationSeed = CurrentTime / 150;  // Default is 250 for animated scores to slide back and forth
  byte scrollPhaseChanged = false;

  byte scrollPhase = ((CurrentTime - LastTimeScoreChanged) / 100) % 16;  // Speed of score scrolling
  if (scrollPhase != LastScrollPhase) {
    LastScrollPhase = scrollPhase;
    scrollPhaseChanged = true;
  }

  boolean updateLastTimeAnimated = false;

  for (byte scoreCount = 0; scoreCount < 4; scoreCount++) {   // Loop on scores

#ifdef USE_SCORE_OVERRIDES
    // If this display is currently being overriden, then we should update it
    if (allScoresShowValue == 0 && (ScoreOverrideStatus & (0x10 << scoreCount))) {
      displayScore = ScoreOverrideValue[scoreCount];
      if (displayScore != DISPLAY_OVERRIDE_BLANK_SCORE) {
        byte numDigits = MagnitudeOfScore(displayScore);
        if (numDigits == 0) numDigits = 1;
        if (numDigits < (RPU_OS_NUM_DIGITS - 1) && (ScoreOverrideStatus & (0x01 << scoreCount))) {
          // This score is going to be animated (back and forth)
          if (overrideAnimationSeed != LastTimeOverrideAnimated) {
            updateLastTimeAnimated = true;
            byte shiftDigits = (overrideAnimationSeed) % (((RPU_OS_NUM_DIGITS + 1) - numDigits) + ((RPU_OS_NUM_DIGITS - 1) - numDigits));
            if (shiftDigits >= ((RPU_OS_NUM_DIGITS + 1) - numDigits)) shiftDigits = (RPU_OS_NUM_DIGITS - numDigits) * 2 - shiftDigits;
            byte digitCount;
            displayMask = GetDisplayMask(numDigits);
            for (digitCount = 0; digitCount < shiftDigits; digitCount++) {
              displayScore *= 10;
              displayMask = displayMask >> 1;
            }
            RPU_SetDisplayBlank(scoreCount, 0x00);
            RPU_SetDisplay(scoreCount, displayScore, false);
            RPU_SetDisplayBlank(scoreCount, displayMask);
          }
        } else {
          RPU_SetDisplay(scoreCount, displayScore, true, 1);
        }
      } else {
        RPU_SetDisplayBlank(scoreCount, 0);
      }

    } else {    // Start of non-overridden
#endif
#ifdef USE_SCORE_ACHIEVEMENTS
      boolean showingCurrentAchievement = false;
#endif      
      // No override, update scores designated by displayToUpdate
      if (allScoresShowValue == 0) {
        displayScore = CurrentScores[scoreCount];
#ifdef USE_SCORE_ACHIEVEMENTS
        displayScore += (CurrentAchievements[scoreCount]%10);
        if (CurrentAchievements[scoreCount]) showingCurrentAchievement = true;
#endif 
      }
      else displayScore = allScoresShowValue;

      // If we're updating all displays, or the one currently matching the loop, or if we have to scroll
      if (displayToUpdate == 0xFF || displayToUpdate == scoreCount || displayScore > RPU_OS_MAX_DISPLAY_SCORE || showingCurrentAchievement) {

        // Don't show this score if it's not a current player score (even if it's scrollable)
        if (displayToUpdate == 0xFF && (scoreCount >= CurrentNumPlayers && CurrentNumPlayers != 0) && allScoresShowValue == 0) {
          RPU_SetDisplayBlank(scoreCount, 0x00);
          continue;
        }

        if (displayScore > RPU_OS_MAX_DISPLAY_SCORE) {    //  Need to scroll
          // Score needs to be scrolled 
          //if ((CurrentTime - LastTimeScoreChanged) < 1000) {  // How long to wait to before 1st scroll
          if ((CurrentTime - LastTimeScoreChanged) < ((MachineState == 0)?1000:4000) ) {  // How long to wait to before 1st scroll
            // show score for four seconds after change
            RPU_SetDisplay(scoreCount, displayScore % (RPU_OS_MAX_DISPLAY_SCORE + 1), false);
            byte blank = RPU_OS_ALL_DIGITS_MASK;
#ifdef USE_SCORE_ACHIEVEMENTS
            if (showingCurrentAchievement && (CurrentTime/200)%2) {
              blank &= ~(0x01<<(RPU_OS_NUM_DIGITS-1));
            }
#endif            
            RPU_SetDisplayBlank(scoreCount, blank);            // Sets all digits on except singles for blinking
          } else {                          // Greater than x seconds so scroll
            // Scores are scrolled 10 digits and then we wait for 6
            if (scrollPhase < 11 && scrollPhaseChanged) {   // Scroll phase 0-10 is for actual scrolling
              byte numDigits = MagnitudeOfScore(displayScore);

              // Figure out top part of score
              unsigned long tempScore = displayScore;
              if (scrollPhase < RPU_OS_NUM_DIGITS) {    // Scrolling lowest 6 digits or less
                displayMask = RPU_OS_ALL_DIGITS_MASK;
                for (byte scrollCount = 0; scrollCount < scrollPhase; scrollCount++) {  
                  displayScore = (displayScore % (RPU_OS_MAX_DISPLAY_SCORE + 1)) * 10;  // *10 shift for each scrolled digit up to scrollPhase
                  displayMask = displayMask >> 1;                               // mask moves up to keep digits behind score blank.
                }
              } else {    // scrollPhase > num digits but less than 11, score is gone and display is now blank
                displayScore = 0;
                displayMask = 0x00;
              }

              // Add in lower part of score (front and back can be on screen at the same time)
              if ((numDigits + scrollPhase) > 10) {             // if total is > 10, score scrolled within 10 space window
                byte numDigitsNeeded = (numDigits + scrollPhase) - 10;  // eg. 12345 & 7 = 12-2 = 2
                for (byte scrollCount = 0; scrollCount < (numDigits - numDigitsNeeded); scrollCount++) {
                  tempScore /= 10;                              // Divide down to get the front end of number needed
                }
                displayMask |= GetDisplayMask(MagnitudeOfScore(tempScore));
                displayScore += tempScore;
              }
              RPU_SetDisplayBlank(scoreCount, displayMask);
              RPU_SetDisplay(scoreCount, displayScore);
            }
          }
        } else {      // End of scrolling portion above
          if (flashCurrent && displayToUpdate == scoreCount) {  // If flashing requested and this display is being updated
            unsigned long flashSeed = CurrentTime / 250;
            if (flashSeed != LastFlashOrDash) {
              LastFlashOrDash = flashSeed;
              if (((CurrentTime / 250) % 2) == 0) RPU_SetDisplayBlank(scoreCount, 0x00);
              else RPU_SetDisplay(scoreCount, displayScore, true, 2);
            }
          } else if (dashCurrent && displayToUpdate == scoreCount) {  // If dashing requested
            unsigned long dashSeed = CurrentTime / 50;
            if (dashSeed != LastFlashOrDash) {
              LastFlashOrDash = dashSeed;
              byte dashPhase = (CurrentTime / 60) % 36;
              byte numDigits = MagnitudeOfScore(displayScore);
              if (dashPhase < 12) {
                displayMask = GetDisplayMask((numDigits == 0) ? 2 : numDigits);
                if (dashPhase < 7) {          // Wipe out all the digits up to 6 based on dashPhase
                  // Wipes out digits progressively from left to right
                  for (byte maskCount = 0; maskCount < dashPhase; maskCount++) {
                    displayMask &= ~(0x01 << maskCount);
                  }
                } else {        //  for dashPhase from 7-11
                  // Show digits again from right to left
                  for (byte maskCount = 12; maskCount > dashPhase; maskCount--) {
                    displayMask &= ~(0x20 >> (maskCount - dashPhase - 1));
                  }
                }
                RPU_SetDisplay(scoreCount, displayScore);
                RPU_SetDisplayBlank(scoreCount, displayMask);
              } else {    // if not in dashPhase from 1-12, then up to 36 do nothing
                RPU_SetDisplay(scoreCount, displayScore, true, 2);
              }
            }
          } else {      // End of dashing
#ifdef USE_SCORE_ACHIEVEMENTS
            byte blank;
            blank = RPU_SetDisplay(scoreCount, displayScore, false, 2);
            if (showingCurrentAchievement && (CurrentTime/200)%2) {
              blank &= ~(0x01<<(RPU_OS_NUM_DIGITS-1));
            }
            RPU_SetDisplayBlank(scoreCount, blank);
#else
            RPU_SetDisplay(scoreCount, displayScore, true, 2);
#endif
          }
        }
      } // End if this display should be updated
#ifdef USE_SCORE_OVERRIDES
    } // End on non-overridden
#endif
  } // End loop on scores

  if (updateLastTimeAnimated) {
    LastTimeOverrideAnimated = overrideAnimationSeed;
  }

}

void ShowFlybyValue(byte numToShow, unsigned long timeBase) {
  byte shiftDigits = (CurrentTime - timeBase) / 120;
  byte rightSideBlank = 0;

  unsigned long bigVersionOfNum = (unsigned long)numToShow;
  for (byte count = 0; count < shiftDigits; count++) {
    bigVersionOfNum *= 10;
    rightSideBlank /= 2;
    if (count > 2) rightSideBlank |= 0x20;
  }
  bigVersionOfNum /= 1000;

  byte curMask = RPU_SetDisplay(CurrentPlayer, bigVersionOfNum, false, 0);
  if (bigVersionOfNum == 0) curMask = 0;
  RPU_SetDisplayBlank(CurrentPlayer, ~(~curMask | rightSideBlank));
}




//
//  Attract Mode
//

byte AttractLastHeadMode = 255;
unsigned long AttractOffset = 0;
#define ATTRACT_BLOCK_1             1115 // 175 1115
#define ATTRACT_BLOCK_2              800
#define ATTRACT_BLOCK_3              700

int RunAttractMode(int curState, boolean curStateChanged) {   //zzzzz

  int returnState = curState;

  // If this is the first time in the attract mode loop
  if (curStateChanged) {

//
// Reset EEPROM memory locations - uncomment, load and boot machine, comment, reload again.
//

#if 0
  RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, 5);
  RPU_WriteULToEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE, 0);
  RPU_WriteULToEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, 101000);
  RPU_WriteULToEEProm(RPU_AWARD_SCORE_1_EEPROM_START_BYTE, 270000);
  RPU_WriteULToEEProm(RPU_AWARD_SCORE_2_EEPROM_START_BYTE, 430000);
  RPU_WriteULToEEProm(RPU_AWARD_SCORE_3_EEPROM_START_BYTE, 650000);
  RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, 0);
  RPU_WriteULToEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE, 0);
  RPU_WriteULToEEProm(RPU_CHUTE_1_COINS_START_BYTE, 0);
  RPU_WriteULToEEProm(RPU_CHUTE_2_COINS_START_BYTE, 0);
  RPU_WriteULToEEProm(RPU_CHUTE_3_COINS_START_BYTE, 0);
#endif
#ifdef COIN_DOOR_TELEMETRY    // Send values to monitor if defined

    Serial.println();
    Serial.print(F("Version Number:        "));
    Serial.println(VERSION_NUMBER, DEC);
    Serial.println();
    Serial.println(F("EEPROM Values: "));
    Serial.println();
    Serial.print(F("Credits:                 "));
    Serial.println(RPU_ReadByteFromEEProm(RPU_CREDITS_EEPROM_BYTE), DEC);
    Serial.print(F("AwardScores[0]:          "));
    Serial.println(RPU_ReadULFromEEProm(RPU_AWARD_SCORE_1_EEPROM_START_BYTE), DEC);
    Serial.print(F("AwardScores[1]:          "));
    Serial.println(RPU_ReadULFromEEProm(RPU_AWARD_SCORE_2_EEPROM_START_BYTE), DEC);
    Serial.print(F("AwardScores[2]:          "));
    Serial.println(RPU_ReadULFromEEProm(RPU_AWARD_SCORE_3_EEPROM_START_BYTE), DEC);
    Serial.print(F("High Score:              "));
    Serial.println(RPU_ReadULFromEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, 10000), DEC);
    Serial.print(F("BALLS_OVERRIDE:          "));
    Serial.println(ReadSetting(EEPROM_BALLS_OVERRIDE_BYTE, 99), DEC);
    Serial.print(F("FreePlay  :              "));
    Serial.println(ReadSetting(EEPROM_FREE_PLAY_BYTE, 0), DEC);
    Serial.print(F("BallSaveNumSeconds       "));
    Serial.println(ReadSetting(EEPROM_BALL_SAVE_BYTE, 15), DEC);

    Serial.print(F("ChaseBallDuration:NotUsed"));
    Serial.println(ReadSetting(EEPROM_CHASEBALL_DURATION_BYTE, 20), DEC);
    Serial.print(F("RoamingStageDuration:    "));
    Serial.println(ReadSetting(EEPROM_CHASEBALL_DURATION_BYTE, ROAMING_STAGE_DURATION_DEF), DEC);

    Serial.print(F("Spinner_Combo_Duration:  "));
    Serial.println(ReadSetting(EEPROM_SPINNER_COMBO_DURATION_BYTE, 12), DEC);
    Serial.print(F("Spinner_Threshold:       "));
    Serial.println(ReadSetting(EEPROM_SPINNER_THRESHOLD_BYTE, 50), DEC);
    Serial.print(F("Pop_Threshold:           "));
    Serial.println(ReadSetting(EEPROM_POP_THRESHOLD_BYTE, 40), DEC);
    Serial.print(F("NextBallDuration:        "));
    Serial.println(ReadSetting(EEPROM_NEXT_BALL_DURATION_BYTE, 15), DEC);
    Serial.print(F("MaxTiltWarnings:         "));
    Serial.println(ReadSetting(EEPROM_TILT_WARNING_BYTE, 2), DEC);
    Serial.print(F("AlleyModeBaseTime:       "));
    Serial.println(ReadSetting(EEPROM_ALLEY_MODE_DURATION_BYTE, ALLEY_MODE_DURATION_DEF), DEC);

    Serial.println();
    Serial.println(F("Variable Values: "));
    Serial.println();

    Serial.print(F("BallsPerGame:             "));
    Serial.println(BallsPerGame, DEC);
    Serial.print(F("BallSaveNumSeconds:       "));
    Serial.println(BallSaveNumSeconds, DEC);
    Serial.print(F("ChaseBallDuration:        "));
    Serial.println(ChaseBallDuration, DEC);
    Serial.print(F("Spinner_Combo_Duration:   "));
    Serial.println(Spinner_Combo_Duration, DEC);
    Serial.print(F("Spinner_Threshold:        "));
    Serial.println(Spinner_Threshold, DEC);
    Serial.print(F("Pop_Threshold:            "));
    Serial.println(Pop_Threshold, DEC);
    Serial.print(F("NextBallDuration:         "));
    Serial.println(NextBallDuration, DEC);
    Serial.print(F("AlleyModeBaseTime:        "));
    Serial.println(AlleyModeBaseTime, DEC);


    Serial.print(F("Current Scores  :         "));
    Serial.print(CurrentScores[0], DEC);
    Serial.print(F("  "));
    Serial.print(CurrentScores[1], DEC);
    Serial.print(F("  "));
    Serial.print(CurrentScores[2], DEC);
    Serial.print(F("  "));
    Serial.println(CurrentScores[3], DEC);


#endif


    RPU_DisableSolenoidStack();
    RPU_TurnOffAllLamps();

    RPU_SetDisableFlippers(true);
    for (int count=0; count<4; count++) {
      RPU_SetDisplayBlank(count, 0x00);     
    }
    CreditsDispValue = Credits;
    CreditsFlashing = false;
    BIPDispValue = 0;
    BIPFlashing = false;

    //  If previous game has been played turn Match on and leave match value on display
    if (CurrentNumPlayers) {    
      BIPDispValue = ((int)MatchDigit * 10);
      RPU_SetLampState(MATCH, 1);
    }

    // If previous game has been played and ended with spinner combo flashing, MarqueeAttract will be disabled.
    MarqueeDisabled = false;
   
    RPU_SetLampState(GAME_OVER, 1);

    AttractLastHeadMode = 255;
    AttractPlayfieldMode = 255;

    AttractOffset = CurrentTime;  // Create offset for starting Classic Attract
    ClearFlashingLampQueue();     // Queue must be cleared to be ready for use first time
    
  } // End of CurrentStateChanged

  if ((CurrentTime/6000)%2==0) {          // Displays during attract, X seconds, two states.
    if (AttractLastHeadMode!=1) {         // 
      RPU_SetLampState(HIGH_SCORE, 1, 0, 250);
      RPU_SetLampState(GAME_OVER, 0);
      //Serial.println(F("RunnAttractModeMode - SetPlayerLamps(0, 4)"));
      SetPlayerLamps(0, 4);

      LastTimeScoreChanged = CurrentTime;
      CreditsDispValue = Credits;
    }
    ShowPlayerScores(0xFF, false, false, HighScore);
    AttractLastHeadMode = 1;
  } else {
      if (AttractLastHeadMode!=2) {
        RPU_SetLampState(HIGH_SCORE, 0);
        RPU_SetLampState(GAME_OVER, 1);
        CreditsDispValue = Credits;
        LastTimeScoreChanged = CurrentTime;
      }
      SetPlayerLamps(((CurrentTime/250)%4) + 1, 4);
      AttractLastHeadMode = 2;
      ShowPlayerScores(0xFF, false, false); // Show player scores
  }

//
//      Playfield Attract Modes
//

// Playfield attract modes are run over increments of time
// Use 100 msec segments, classic attract requires 128 segments
// Playfield attract mode numbers are arbitrary, they just must be different between segments

  int AttractPlayfieldSegment = ((CurrentTime-AttractOffset)/100)%1500;

//
// Classic Attract modes
// ClassicAttract sets playefield mode to 5
  if (AttractPlayfieldSegment < 128) {  //  Classic Eight Ball Attract Mode
    ClassicAttract(850, true, false);

  } else if (AttractPlayfieldSegment < 169) {   // Fast classic attract rack only
    ClassicAttract(100, true, false);

  } else if (AttractPlayfieldSegment < 218) {   // Fast classic attract rack and small lamps
    ClassicAttract(100, true, true);

  } else if (AttractPlayfieldSegment < 313) {   // Fast classic attract with marquee additions
    ClassicAttract(50, true, true);
    // Marquee Attract function for full playfield.
    MarqueeAttract(2, 150, false);              // Banks shot lamps
    MarqueeAttract(5, 150, true);               // Bonus multiplier lamps
    MarqueeAttract(6, 150, false);              // Player lamps
    RPU_SetLampState(SAME_PLAYER, 1, 0, 500);
    RPU_SetLampState(LA_SUPER_BONUS, 1, 0, 500);
    RPU_SetLampState(LA_SPINNER, 1, 0, 250);
    RPU_SetLampState(LA_OUTLANE_SPECIAL, 1, 0, 125);

  } else if (AttractPlayfieldSegment < 400) {   // Fast classic attract with rotating rack bars
    ClassicAttract(50, false, true);            // Only small lamps
    ShowLampAnimation(LampAnimation1, 12, 80, CurrentTime, 11, false, false, 99, false); // Rotating bars rack region
    // Marquee Attract function for full playfield.
    MarqueeAttract(2, 80, false);              // Banks shot lamps
    MarqueeAttract(5, 80, true);               // Bonus multiplier lamps
    MarqueeAttract(6, 80, false);              // Player lamps
    RPU_SetLampState(SAME_PLAYER, 1, 0, 110);
    RPU_SetLampState(LA_SUPER_BONUS, 1, 0, 100);
    RPU_SetLampState(LA_SPINNER, 1, 0, 120);
    RPU_SetLampState(LA_OUTLANE_SPECIAL, 1, 0, 100);
// End of Classic Attract

// Spiral sweep
  } else if (AttractPlayfieldSegment < 558) {  //  Reversing sweep style spiral animation
    if (AttractPlayfieldMode!=14) {
      Serial.write("Reversing Sweep Roll Down\n\r");
      RPU_TurnOffAttractLamps();               // Blank all lamps
      AttractSweepLights = 0;
      SpiralIncrement = 1;
    }
    if ((CurrentTime-AttractSweepTime) > 20) { // Value in msec delay between animation steps
      AttractSweepTime = CurrentTime;
      for (byte lightcountdown=0; lightcountdown<NUM_OF_TRIANGLE_LAMPS_CW; lightcountdown++) {
        byte dist = AttractSweepLights - AttractLampsDown[lightcountdown].rowDown;
        RPU_SetLampState(AttractLampsDown[lightcountdown].lightNumDown, (dist < 30), 0);
        //RPU_SetLampState(AttractLampsDown[lightcountdown].lightNumDown, (dist<8), (dist==0)?0:dist/3, (dist>5)?(100+AttractLampsDown[lightcountdown].lightNumDown):0);
      }
      AttractSweepLights = AttractSweepLights + SpiralIncrement;
      if (AttractSweepLights <= 0 || AttractSweepLights >= 66) {
        SpiralIncrement = -SpiralIncrement;
      }
    }
    AttractPlayfieldMode = 14;

// Downward sweep
  } else if (AttractPlayfieldSegment < 658) {  //  Downward sweep
    if (AttractPlayfieldMode!=3) {
      Serial.write("Sweep Roll Down\n\r");
      //RPU_TurnOffAttractLamps();               // Blank all lamps
      AttractSweepLights = 0;
      SpiralIncrement = 1;
    }   
    SweepAnimation(RollDown, 45, 44, 30, 18);
    //SweepAnimation(RollDown, 46, 26, 30, 6);  // Large wave rolling down
    AttractPlayfieldMode = 3;


// Left-Right sweep
  } else if (AttractPlayfieldSegment < 758) {  // Left-Right Sweep

    if (AttractPlayfieldMode!=4) {
      Serial.write("Sweep Left to Right\n\r");
      RPU_TurnOffAttractLamps();               // Blank all lamps
      AttractSweepLights = 0;
      SpiralIncrement = 1;
    }   
    SweepAnimation(SweepLefttoRight, 46, 19, 30, 3);
    AttractPlayfieldMode = 4;

//
// Marquee and Flashing Lamp Attract modes
//
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_2 + 50) {  // Marquee Attract
    if (AttractPlayfieldMode!=16) {
      // Rotating Marquee Lights
      RPU_TurnOffAttractLamps();
      if (DEBUG_MESSAGES) {
        Serial.write("Marquee Attract\n\r");
        Serial.println();
      }    
    }
    // Marquee Attract function for full playfield.
    MarqueeAttract(4, 100, true);
    AttractPlayfieldMode = 16;
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_2 + 88) {  // Cascade rack zig zag
      if (AttractPlayfieldMode!=12) {
        //RPU_TurnOffAttractLamps();             // Blank all lamps
        ClearRack();
        for (int i = 0; i < 15; i++) {            
          //AddToFlashingLampQueue( (LA_BIG_1 + i), 0, CurrentTime + i*125, 2000, 33, 200);
          //AddToFlashingLampQueue( (RackZigZag[i]), 0, CurrentTime + i*125, (1000 + 100*i), 33, 200);
          AddToFlashingLampQueue( (RackZigZag[i]), 0, CurrentTime + i*75, (1000 + 100*i), 33, 250);
          //AddToFlashingLampQueue( (LA_BIG_1 + i), 0, CurrentTime + i*125, 1000, 33, 200);
        }
      }
    ProcessFlashingLampQueue(CurrentTime);
    MarqueeAttract(2, 100);
    MarqueeAttract(3, 100);
    MarqueeAttract(5, 100);
    MarqueeAttract(7, 100);
    AttractPlayfieldMode = 12;
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_2 + 134) {  // Cascade rack 1 to 12
      if (AttractPlayfieldMode!=11) {
        //RPU_TurnOffAttractLamps();             // Blank all lamps
        ClearRack();
        AddToFlashingLampQueue(LA_BIG_1, 0, CurrentTime, 3000, 33, 250);
        AddToFlashingLampQueue(LA_BIG_2, 0, CurrentTime + 300, 3000, 33, 240);
        AddToFlashingLampQueue(LA_BIG_6, 0, CurrentTime + 300, 3000, 33, 240);
        AddToFlashingLampQueue(LA_BIG_3, 0, CurrentTime + 600, 3000, 33, 230);
        AddToFlashingLampQueue(LA_BIG_7, 0, CurrentTime + 600, 3000, 33, 230);
        AddToFlashingLampQueue(LA_BIG_10, 0, CurrentTime + 600, 3000, 33, 230);
        AddToFlashingLampQueue(LA_BIG_4, 0, CurrentTime + 900, 3000, 33, 220);
        AddToFlashingLampQueue(LA_BIG_8, 0, CurrentTime + 900, 3000, 33, 220);
        AddToFlashingLampQueue(LA_BIG_11, 0, CurrentTime + 900, 3000, 33, 220);
        AddToFlashingLampQueue(LA_BIG_13, 0, CurrentTime + 900, 3000, 33, 220);
        AddToFlashingLampQueue(LA_BIG_5, 0, CurrentTime + 1200, 3000, 33, 210);
        AddToFlashingLampQueue(LA_BIG_9, 0, CurrentTime + 1200, 3000, 33, 210);
        AddToFlashingLampQueue(LA_BIG_12, 0, CurrentTime + 1200, 3000, 33, 210);
        AddToFlashingLampQueue(LA_BIG_14, 0, CurrentTime + 1200, 3000, 33, 210);
        AddToFlashingLampQueue(LA_BIG_15, 0, CurrentTime + 1200, 3000, 33, 210);
      }
    ProcessFlashingLampQueue(CurrentTime);
    MarqueeAttract(2, 100);
    MarqueeAttract(3, 100);
    MarqueeAttract(5, 100);
    MarqueeAttract(7, 100);
    AttractPlayfieldMode = 11;
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_2 + 180) {  // Cascade rack 5 to 10
      if (AttractPlayfieldMode!=12) {
        //RPU_TurnOffAttractLamps();             // Blank all lamps
        ClearRack();
        AddToFlashingLampQueue(LA_BIG_5, 0, CurrentTime, 3000, 33, 250);
        AddToFlashingLampQueue(LA_BIG_4, 0, CurrentTime + 300, 3000, 33, 240);
        AddToFlashingLampQueue(LA_BIG_9, 0, CurrentTime + 300, 3000, 33, 240);
        AddToFlashingLampQueue(LA_BIG_3, 0, CurrentTime + 600, 3000, 33, 230);
        AddToFlashingLampQueue(LA_BIG_8, 0, CurrentTime + 600, 3000, 33, 230);
        AddToFlashingLampQueue(LA_BIG_12, 0, CurrentTime + 600, 3000, 33, 230);
        AddToFlashingLampQueue(LA_BIG_2, 0, CurrentTime + 900, 3000, 33, 220);
        AddToFlashingLampQueue(LA_BIG_7, 0, CurrentTime + 900, 3000, 33, 220);
        AddToFlashingLampQueue(LA_BIG_11, 0, CurrentTime + 900, 3000, 33, 220);
        AddToFlashingLampQueue(LA_BIG_14, 0, CurrentTime + 900, 3000, 33, 220);
        AddToFlashingLampQueue(LA_BIG_1, 0, CurrentTime + 1200, 3000, 33, 210);
        AddToFlashingLampQueue(LA_BIG_6, 0, CurrentTime + 1200, 3000, 33, 210);
        AddToFlashingLampQueue(LA_BIG_10, 0, CurrentTime + 1200, 3000, 33, 210);
        AddToFlashingLampQueue(LA_BIG_13, 0, CurrentTime + 1200, 3000, 33, 210);
        AddToFlashingLampQueue(LA_BIG_15, 0, CurrentTime + 1200, 3000, 33, 210);
      }
    ProcessFlashingLampQueue(CurrentTime);
    MarqueeAttract(2, 100);
    MarqueeAttract(3, 100);
    MarqueeAttract(5, 100);
    MarqueeAttract(7, 100);
    AttractPlayfieldMode = 12;
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_2 + 223) {  // Cascade outward spiral
      if (AttractPlayfieldMode!=13) {
        //RPU_TurnOffAttractLamps();             // Blank all lamps
        ClearRack();
        AddToFlashingLampQueue(LA_BIG_8, 0,  CurrentTime,       2000, 35, 250);
        AddToFlashingLampQueue(LA_BIG_4, 0,  CurrentTime + 200, 2200, 32, 240);
        AddToFlashingLampQueue(LA_BIG_3, 0,  CurrentTime + 400, 2400, 29, 230);
        AddToFlashingLampQueue(LA_BIG_2, 0,  CurrentTime + 600, 2600, 27, 220);
        AddToFlashingLampQueue(LA_BIG_1, 0,  CurrentTime + 800, 2800, 25, 210);

        AddToFlashingLampQueue(LA_BIG_7, 0,  CurrentTime,       2000, 33, 250);
        AddToFlashingLampQueue(LA_BIG_6, 0,  CurrentTime + 200, 2200, 32, 240);
        AddToFlashingLampQueue(LA_BIG_10, 0, CurrentTime + 400, 2400, 29, 230);
        AddToFlashingLampQueue(LA_BIG_13, 0, CurrentTime + 600, 2600, 27, 220);
        AddToFlashingLampQueue(LA_BIG_15, 0, CurrentTime + 800, 2800, 25, 210);

        AddToFlashingLampQueue(LA_BIG_11, 0, CurrentTime,       2000, 33, 250);
        AddToFlashingLampQueue(LA_BIG_14, 0, CurrentTime + 200, 2200, 32, 240);
        AddToFlashingLampQueue(LA_BIG_12, 0, CurrentTime + 400, 2400, 29, 230);
        AddToFlashingLampQueue(LA_BIG_9, 0,  CurrentTime + 600, 2600, 27, 220);
        AddToFlashingLampQueue(LA_BIG_5, 0,  CurrentTime + 800, 2800, 25, 210);
      }
    ProcessFlashingLampQueue(CurrentTime);
    MarqueeAttract(2, 100);
    MarqueeAttract(3, 100);
    MarqueeAttract(5, 100);
    MarqueeAttract(7, 100);
    AttractPlayfieldMode = 13;
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_2 + 280) {  // Marquee Attract
    if (AttractPlayfieldMode!=16) {
      // Rotating Marquee Lights
      RPU_TurnOffAttractLamps();
      if (DEBUG_MESSAGES) {
        Serial.write("Marquee Attract\n\r");
        Serial.println();
      }    
    }
    // Marquee Attract function for full playfield.
    MarqueeAttract(4, 100, false);
    AttractPlayfieldMode = 16;

// End of Marquee and Flashing Lamp Attract modes

//
// Sweeps
//
// Middle Right
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_1) {  //  Middle right pivot - sweeping
    if (AttractPlayfieldMode!=10) {
      RPU_TurnOffAttractLamps();               // Blank all lamps
      ShowLampTimeOffset = CurrentTime;
    }
    ShowLampAnimation(RADARMiddleRightPivot, 28, 25, CurrentTime, 24, false, false, 99, false); 
    AttractPlayfieldMode = 10;
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_1 + 14) {  //  Middle right pivot - full fill
    if (AttractPlayfieldMode!=100) {
      //RPU_TurnOffAttractLamps();               // Blank all lamps
      ShowLampTimeOffset = CurrentTime;
    }
    ShowLampAnimation(RADARMiddleRightPivot, 28, 50, CurrentTime, 0, false, false, 99, false); 
    AttractPlayfieldMode = 100;
// Middle Left
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_1 + 49) {  //  Middle left pivot - sweeping
    if (AttractPlayfieldMode!=11) {
      //RPU_TurnOffAttractLamps();               // Blank all lamps
      ShowLampTimeOffset = CurrentTime;
    }
    ShowLampAnimation(RADARMiddleLeftPivot, 28, 25, CurrentTime, 24, false, false, 99, false); 
    AttractPlayfieldMode = 11;
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_1 + 63) {  //  Middle left pivot - full fill
    if (AttractPlayfieldMode!=110) {
      //RPU_TurnOffAttractLamps();               // Blank all lamps
      ShowLampTimeOffset = CurrentTime;
    }
    ShowLampAnimation(RADARMiddleLeftPivot, 28, 50, CurrentTime, 0, false, false, 99, false); 
    AttractPlayfieldMode = 110;
// Middle Top
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_1 + 96) {  //  Middle top pivot - sweeping
    if (AttractPlayfieldMode!=10) {
      //RPU_TurnOffAttractLamps();               // Blank all lamps
      ShowLampTimeOffset = CurrentTime;
    }
    ShowLampAnimation(RADARTopPivot, 26, 25, CurrentTime, 22, false, false, 99, false); 
    AttractPlayfieldMode = 10;
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_1 + 110) {  //  Middle top pivot - full fill
    if (AttractPlayfieldMode!=100) {
      //RPU_TurnOffAttractLamps();               // Blank all lamps
      ShowLampTimeOffset = CurrentTime;
    }
    ShowLampAnimation(RADARTopPivot, 26, 50, CurrentTime, 0, false, false, 99, false); 
    AttractPlayfieldMode = 100;
// Middle Bottom
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_1 + 141) {  //  Middle bottom pivot - sweeping
    if (AttractPlayfieldMode!=11) {
      Serial.write("Middle bottom pivot\n\r");
      //RPU_TurnOffAttractLamps();               // Blank all lamps
      ShowLampTimeOffset = CurrentTime;
    }
    //ShowLampAnimation(RADARBotPivot, 21, 25, CurrentTime, 21 - (CurrentTime - ShowLampTimeOffset)/155, false, false, 38, false); 
    ShowLampAnimation(RADARBotPivot, 21, 25, CurrentTime, 17, false, false, 38, false); 
    AttractPlayfieldMode = 11;
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_1 + 155) {  //  Middle bottom pivot - full fill
    if (AttractPlayfieldMode!=11) {
      Serial.write("Middle bottom pivot\n\r");
      //RPU_TurnOffAttractLamps();               // Blank all lamps
      ShowLampTimeOffset = CurrentTime;
    }
    ShowLampAnimation(RADARBotPivot, 21, 25, CurrentTime, 0, false, false, 38, false); 
    AttractPlayfieldMode = 11;
// Super Bonus RADAR sweep
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_1 + 255) {  //  SuperBonus pivot - sweeping
    if (AttractPlayfieldMode!=10) {
      Serial.write("SuperBonus pivot\n\r");
      //RPU_TurnOffAttractLamps();               // Blank all lamps
      ShowLampTimeOffset = CurrentTime;
    }
    ShowLampAnimation(RADARSuperBonusPivot, 26, 25, CurrentTime, 25 - (CurrentTime - ShowLampTimeOffset)/400, false, false, 44, false); 
    //ShowLampAnimation(RADARSuperBonusPivot, 26, 30, CurrentTime, 22, false, false, 44, false); 
    AttractPlayfieldMode = 10;
  } else if (AttractPlayfieldSegment < ATTRACT_BLOCK_1 + 275) {  //  SuperBonus pivot - full fill
    if (AttractPlayfieldMode!=100) {
      Serial.write("SuperBonus pivot\n\r");
      //RPU_TurnOffAttractLamps();               // Blank all lamps
      ShowLampTimeOffset = CurrentTime;
    }
    ShowLampAnimation(RADARSuperBonusPivot, 26, 60, CurrentTime, 0, false, false, 44, false); 
    AttractPlayfieldMode = 100;

  } else if (AttractPlayfieldSegment < 5000) {  // Spiral attract again

    if (AttractPlayfieldMode!=15) {
      Serial.write("Spiral Attract\n\r");
      Serial.println();
      RPU_TurnOffAttractLamps(); // Start mode by blanking all lamps
      AttractSweepLights = 0;
      SpiralIncrement = 1;
    }
  
    if ((CurrentTime-AttractSweepTime) > 20) { // Value in msec delay between animation steps
      AttractSweepTime = CurrentTime;
      for (byte lightcountdown=0; lightcountdown<NUM_OF_TRIANGLE_LAMPS_CW; lightcountdown++) {
        byte dist = AttractSweepLights - AttractLampsDown[lightcountdown].rowDown;
        RPU_SetLampState(AttractLampsDown[lightcountdown].lightNumDown, (dist < 30), 0);
        //RPU_SetLampState(AttractLampsDown[lightcountdown].lightNumDown, (dist<8), (dist==0)?0:dist/3, (dist>5)?(100+AttractLampsDown[lightcountdown].lightNumDown):0);
        //if (lightcountdown==(NUM_OF_TRIANGLE_LAMPS_CW/2)) RPU_DataRead(0);
      }

      AttractSweepLights = AttractSweepLights + SpiralIncrement;
      if (AttractSweepLights <= 0 || AttractSweepLights >= 74) {
        SpiralIncrement = -SpiralIncrement;
      }
    }
  
    AttractPlayfieldMode = 15;

  
  }  // End of loop around playfield animations


// Decide whether we stay in Attract or not.  Watch switches for adding a player or credits.
// Change return state for either INIT_GAMEPLAY or start self test.

  byte switchHit;
  while ( (switchHit=RPU_PullFirstFromSwitchStack())!=SWITCH_STACK_EMPTY ) {
    if (switchHit==SW_CREDIT_RESET) {
      if (AddPlayer(true)) returnState = MACHINE_STATE_INIT_GAMEPLAY;
    } else if (switchHit==SW_COIN_1 || switchHit==SW_COIN_2 || switchHit==SW_COIN_3) {
      AddCoinToAudit(switchHit);
      AddCredit(true);
      CreditsDispValue = Credits;
    } else if (switchHit==SW_SELF_TEST_SWITCH && (CurrentTime-GetLastSelfTestChangedTime())>500) {
      returnState = MACHINE_STATE_TEST_LIGHTS;
      SetLastSelfTestChangedTime(CurrentTime);
    } else {
#ifdef DEBUG_MESSAGES
      char buf[128];
      sprintf(buf, "Switch 0x%02X\n", switchHit);
      Serial.write(buf);
#endif      
    }
  }
  return returnState;
}


boolean PlayerUpLightBlinking = false;
boolean MarqeeToggle;

int NormalGamePlay() {    //zzzzz
  int returnState = MACHINE_STATE_NORMAL_GAMEPLAY;

  // If the playfield hasn't been validated yet, flash score and player up num
  if (BallFirstSwitchHitTime==0) {
    if (!PlayerUpLightBlinking) {
      SetPlayerLamps((CurrentPlayer+1), 4, 250);
      PlayerUpLightBlinking = true;
    }
  } else {
    if (PlayerUpLightBlinking) {
      SetPlayerLamps((CurrentPlayer+1), 4);
      PlayerUpLightBlinking = false;

    }
  }
  ShowPlayerScores(CurrentPlayer, (BallFirstSwitchHitTime == 0) ? true : false, 
  (BallFirstSwitchHitTime > 0 && ((CurrentTime - LastTimeScoreChanged) > 4000)) ? true : false);

  if ( (BallFirstSwitchHitTime == 0) && GoalsDisplayValue(Goals[CurrentPlayer]) ) {   // If ball not in play and if any goals have been reached
    for (byte count = 0; count < 4; count++) {
      if (count != CurrentPlayer) {
        OverrideScoreDisplay(count, GoalsDisplayValue(Goals[CurrentPlayer]), false);  // Show achieved goals
      }
    }
    GoalsDisplayToggle = true;
  } else if ( (BallFirstSwitchHitTime > 0) && GoalsDisplayToggle) {
    ShowPlayerScores(0xFF, false, false);                                             //  Reset all score displays
    GoalsDisplayToggle = false;
  }
  
  ShowShootAgainLamp(); // Check if Ball save blinking should finish


//
// Game Mode stuff here
//

// Various telemetry here.

#ifdef IN_GAME_TELEMETRY

  if ((CurrentTime - CheckBallTime) > 10000) {  // Check every X seconds
    CheckBallTime = CurrentTime; 
    Serial.println();
    Serial.println(F("----------- Game Telemetry -----------"));  
    Serial.println();
//    Serial.print(F("CheckBallTime is:              "));
//    Serial.println(CheckBallTime, DEC);  
//    Serial.print(F("Current Balls1to7:             "));
//    Serial.println(Balls1to7, BIN);  
    Serial.print(F("GameMode[CrtPlyr] is: "));
    Serial.println(GameMode[CurrentPlayer], DEC);
    Serial.print(F("FifteenBallCounter[CrtPlyr] is: "));
    Serial.println(FifteenBallCounter[CurrentPlayer], DEC);
    Serial.println(F("Current Balls[]: 1------81------8"));
    Serial.print(F("                 "));
    Serial.println(Balls[CurrentPlayer], BIN);  

    Serial.print("Goals CurPlyr is: ");
    Serial.println(Goals[CurrentPlayer], BIN);
    //Serial.println(Goals[0], BIN);
    //Serial.println(Goals[1], BIN);
    //Serial.println(Goals[2], BIN);
    //Serial.println(Goals[3], BIN);
    Serial.println();

//    Serial.print("BallFirstSwitchHitTime is: ");
//    Serial.println(BallFirstSwitchHitTime, DEC);  

//    Serial.print(F("ArrowsLit Crt Plr is:         "));
//    Serial.println(ArrowsLit[CurrentPlayer], DEC);
//    Serial.print(F("ArrowTest is:                 "));
//    Serial.println(ArrowTest, DEC);

//    Serial.print(F("SpinnerKickerLit is:           "));
//    Serial.println(SpinnerKickerLit, DEC);  

//    Serial.println();
//    Serial.print(F("BankShotProgress is:           "));
//    Serial.println(BankShotProgress, DEC);  

//    Serial.print(F("BonusMult is:                  "));
//    Serial.println(BonusMult, DEC);  

//    Serial.print(F("OutlaneSpecial CurPlr is:     "));
//    Serial.println(OutlaneSpecial[CurrentPlayer], DEC);  

//    Serial.print(F("EightBallTest - Cur Plr is:    "));
//    Serial.println(EightBallTest[CurrentPlayer], DEC);  

//    Serial.print(F("SuperBonusLit       is:        "));
//    Serial.println(SuperBonusLit, DEC);  

//    Serial.print(F("PopCount is:                   "));
//    Serial.println(PopCount[CurrentPlayer], DEC);  
//    Serial.print(F("PopMode is:                    "));
//    Serial.println(PopMode[CurrentPlayer], DEC);  
//    Serial.print(F("SpinnerCount is:                   "));
//    Serial.println(SpinnerCount[CurrentPlayer], DEC);  
//    Serial.print("SuperSpinnerAllowed CrtPlyr is: ");
//    Serial.println(SuperSpinnerAllowed[CurrentPlayer], DEC);  

//    Serial.print(F("CurrentNumPlayers:           "));
//    Serial.println(CurrentNumPlayers, DEC);  
//    Serial.println();

#if 0
//  Roaming - Mode 4
    Serial.print(F("RoamingBalls:                  "));
    Serial.println(RoamingBalls, BIN);
    Serial.print(F("RoamingStage is:               "));
    Serial.println(RoamingStage, DEC);
    Serial.print(F("NumCapturedLetters is:         "));
    Serial.println(NumCapturedLetters, DEC);
    //RoamingRotateTime
    //RoamingModeTime
    Serial.println();
#endif

#if 0
//  ChaseBall - Mode 3
    Serial.print(F("ChaseBall stage is: "));
    Serial.println(ChaseBallStage, DEC);
    Serial.print(F("ChaseBall is:       "));
    Serial.println(ChaseBall, DEC);
#endif
#if 1
    Serial.print(F("NextBall is:        "));
    Serial.println(NextBall, DEC);
    Serial.print(F("NextBallTime is:    "));
    Serial.println(NextBallTime, DEC);
#endif

    Serial.print(F("OverrideScorePriority is:      "));
    Serial.println(OverrideScorePriority, DEC);

#if 0
    Serial.print(F("AlleyModeActive is:            "));
    Serial.println(AlleyModeActive, DEC);  
    Serial.print(F("AlleyModeNumber is:            "));
    Serial.println(AlleyModeNumber, DEC);  
    Serial.print(F("AlleyModePopsTrigger is:       "));
    Serial.println(AlleyModePopsTrigger, DEC);
    Serial.print(F("AlleyModeSpinnerTrigger is:    "));
    Serial.println(AlleyModeSpinnerTrigger, DEC);
#endif

    Serial.print(F("FlashingLampQueueEmpty is:     "));
    Serial.println(FlashingLampQueueEmpty, DEC);

    Serial.println();
    Serial.println();  

  }
#endif

//
// Common to all modes
//

// Process any flashing lamps

  ProcessFlashingLampQueue(CurrentTime);  

//
//  15 Ball mode qualified animation
//

if (FifteenBallQualified[CurrentPlayer]) {
  ShowLampAnimation(LampAnimation7, 14, 45, CurrentTime, 11, false, false, 99, false); // Downward sweep
  //ShowLampAnimation(LampAnimation7, 14, 40, CurrentTime, 5, false, false, 99, false); // Downward sweep
  FlashingArrows(DownwardV, 125, 9);
}


//
//  Chase Ball - Mode 3
//

  if ( GameMode[CurrentPlayer] == 3  && ((CurrentTime - ChaseBallTimeStart) < (ChaseBallDuration*1000)) ) {
    // Speed up based on duration
    int LampBlinkTime = 300;
    int AnimationRate = 70;
    if ((CurrentTime - ChaseBallTimeStart) < ((ChaseBallDuration*1000)/2)) {
      LampBlinkTime = 200;
      AnimationRate = 100;
      //Serial.println("Step 1");
    } else if ((CurrentTime - ChaseBallTimeStart) < ((ChaseBallDuration*1000)*3/4)) {
      LampBlinkTime = 100;
      AnimationRate = 75; 
      //Serial.println("Step 2--");
    } else {
      LampBlinkTime = 40;
      AnimationRate = 40;
      //Serial.println("Step 3---");
    }

    RPU_SetLampState((LA_SMALL_1 - 1 + ChaseBall), 1, 0, LampBlinkTime);
    ShowLampAnimation(LampAnimation3, 15, (AnimationRate), CurrentTime, 13, false, false, 99, false); // 3 way sweep

    if (OVERRIDE_PRIORITY_CHASEBALLMODE3 > OverrideScorePriority) {   // Check if priority should be raised
      OverrideScorePriority = OVERRIDE_PRIORITY_CHASEBALLMODE3;
    }
    if (OverrideScorePriority == OVERRIDE_PRIORITY_CHASEBALLMODE3) {
      for (byte count = 0; count < 4; count++) {
        if (count != CurrentPlayer) {
          OverrideScoreDisplay(count, (ChaseBallDuration - ((CurrentTime - ChaseBallTimeStart)/1000)), true); // Show time left
        }
      }
    }
  } else if ( GameMode[CurrentPlayer] == 3 ) {
    // ChaseBall has timed out, reset back to Mode 1 regular play
    PlaySoundEffect(SOUND_EFFECT_BALL_LOSS);
    GameMode[CurrentPlayer] = 1;
    ChaseBallTimeStart = 0;                 // Zero clock to halt mode
    ChaseBall = 0;
    BallLighting();
    BankShotLighting();
    OverrideScorePriority = 0;              //  Set back to zero
    ShowPlayerScores(0xFF, false, false);   //  Reset all score displays
  }


//
// Next Ball
//

  if ( (NextBall != 0) && ((CurrentTime - NextBallTime) < (NextBallDuration*1000)) ) {
    if (!((GameMode[CurrentPlayer] == 3) || (GameMode[CurrentPlayer] == 4)) ) {
      // Set next ball to rapid flashing - no longer used, FlashingLampQueue is done in CaptureBall
      //RPU_SetLampState((LA_SMALL_1 + (NextBall-1) + (CurrentPlayer%2 ? 8:0)), 1, 0, 100);
      
      if (OVERRIDE_PRIORITY_NEXTBALL > OverrideScorePriority) {       // Check if priority should be raised
        OverrideScorePriority = OVERRIDE_PRIORITY_NEXTBALL;
      }
      if (OverrideScorePriority == OVERRIDE_PRIORITY_NEXTBALL) {
        for (byte count = 0; count < 4; count++) {
          if (count != CurrentPlayer) {
            // Show time left
            OverrideScoreDisplay(count, (NextBallDuration - ((CurrentTime - NextBallTime)/1000)), false);
          }
        }
      } 
    }
  } else if (NextBallTime != 0) {
    #ifdef EXECUTION_MESSAGES
      Serial.println("---- NextBall is timed out ----");
    #endif
    NextBallFinish();                       // Close out mode
  }

//
//  ChimeScoring
//

  if ((Silent_Hundred_Pts_Stack != 0) || (Silent_Thousand_Pts_Stack != 0) || (Ten_Pts_Stack != 0) || (Hundred_Pts_Stack != 0) || (Thousand_Pts_Stack != 0)) {
    ChimeScoring();
  }

//
// 8 Ball target lighting - If 7 balls collected, turn on 8 Ball target
// Rev 2
//

  if ( (((Balls[CurrentPlayer] & (0b01111111<<(CurrentPlayer%2 ? 8:0)))>>(CurrentPlayer%2 ? 8:0))==127) && (EightBallTest[CurrentPlayer]) ) {
    EightBallTest[CurrentPlayer] = false;
    //Serial.println(F("Turning on small ball 8"));
    //Serial.println();
    RPU_SetLampState(LA_SMALL_8, 1);
  }


//
//  ArrowsLit mode - Set to 1 when lanes 1-4 collected
//  Ver 4 - Split GameMode logic

  if (GameMode[CurrentPlayer] == 1) {
    if ( (((Balls[CurrentPlayer] & (0b1111<<(CurrentPlayer%2 ? 8:0)))>>(CurrentPlayer%2 ? 8:0))==15) && (ArrowTest) ) {
      #ifdef EXECUTION_MESSAGES
        Serial.println(F("ArrowsLit Test - GameMode = 1"));
      #endif
      ArrowsLit[CurrentPlayer] = true;
      ArrowTest = false;
      ArrowToggle();
    }
  } else if (GameMode[CurrentPlayer] == 2) {
    if ( ((Balls[CurrentPlayer] & (0b1111) | ((Balls[CurrentPlayer] & (0b1111<<8)) >> 8))==15) && (ArrowTest) ) {
      #ifdef EXECUTION_MESSAGES
        Serial.println(F("ArrowsLit Test - GameMode = 2"));
      #endif
      ArrowsLit[CurrentPlayer] = true;
      ArrowTest = false;
      ArrowToggle();
    }
  }

//
// Super Spinner Mode - Ver 2 - Spinner Combo
// 

  if ( (SuperSpinnerAllowed[CurrentPlayer]) && ((CurrentTime - SuperSpinnerTime) < (SuperSpinnerDuration*1000)) ) {
    if (OVERRIDE_PRIORITY_SPINNERCOMBO > OverrideScorePriority) {       // Check if priority should be raised
      OverrideScorePriority = OVERRIDE_PRIORITY_SPINNERCOMBO;
    }
    if (OverrideScorePriority == OVERRIDE_PRIORITY_SPINNERCOMBO) {
      for (byte count = 0; count < 4; count++) {
        if (count != CurrentPlayer) {
          // Show time left
          OverrideScoreDisplay(count, (SuperSpinnerDuration - ((CurrentTime - SuperSpinnerTime)/1000)), false);
        }
      }
    }    
  } else if (SuperSpinnerAllowed[CurrentPlayer]) {
    SpinnerComboFinish();                       // Turn off mode, release marquee animation, etc
    BallLighting();
    if (SpinnerKickerLit) {
      RPU_SetLampState(LA_SPINNER, 1);        
    } else if ( (!SpinnerKickerLit) && (KickerOffTime != 0) ) {
      //  If FlashingLampQueue event is underway it will keep going and wrap up, no need to set lamp flashing here now.
      //RPU_SetLampState(LA_SPINNER, 1, 0, 100);
    } else {
      RPU_SetLampState(LA_SPINNER, 0);
    }
  }


//
//  SpinnerKickerLit mode - delayed Kicker turn off
//

  if ( ((CurrentTime - KickerOffTime) > KICKER_SAVE_DURATION) && (KickerOffTime != 0) ) {
    KickerReady = false;
    // FlashingLampQueue entry will time out by itself
    //RPU_SetLampState(LA_SPINNER, 0);
    KickerOffTime = 0;
  }

// Bank Shot Animation
  if ( ((CurrentTime - BankShotOffTime) < (750+(BankShotProgress*100)) /*&& (BankShotOffTime != 0)*/) ) {
    //Serial.println(F("Bank Shot Animation running"));
    // Call Marquee Attract for BankShot portion
    MarqueeAttract(2 , 80, false);
  } else if (BankShotOffTime != 0) {
    BankShotLighting();
    BankShotOffTime = 0;
  }

// Marquee Animation - Ver 3
//   animation after ball capture

  if ( (CurrentTime - MarqueeOffTime) < (600 * MarqueeMultiple) ) {
    MarqueeAttract(1 , 100, MarqeeToggle);        // Call Ball Rack portion only
    if (Balls[CurrentPlayer] & (0b10000000)) {    // Call additional portions if 8 ball captured
      MarqueeAttract(2 , 100, MarqeeToggle);
      MarqueeAttract(3 , 100, MarqeeToggle);
    }
  } else if (MarqueeOffTime != 0) {
    //Serial.print(F("MarqueeDisabled is:"));
    //Serial.println(MarqueeDisabled, DEC);
    if (!MarqueeDisabled) BallLighting();
    if (Balls[CurrentPlayer] & (0b10000000)) {
      BonusMultiplier(BonusMult);
      BankShotLighting();
    }
    MarqueeOffTime = 0;
    MarqeeToggle = !MarqeeToggle;
  }

  // Alley Mode Combo

    if (AlleyModeActive) {                                                     // If mode active
      unsigned long Alley_Reward = 10000*(unsigned long)(AlleyModeNumber);
      if (OVERRIDE_PRIORITY_ALLEY_MODE_COMBO > OverrideScorePriority) {        // Check if priority should be raised
        OverrideScorePriority = OVERRIDE_PRIORITY_ALLEY_MODE_COMBO;
      }
      switch (AlleyModeActive) {
        case 1:                                                                 // Intro animation
          if ((CurrentTime - Alley_Mode_Start_Time) < ALLEY_MODE_ANIMATION_TIME ) {
            MarqueeAttract(3, 75, true, true);                                  // Animate top playfield region, priority true
            unsigned int Countdown = ((AlleyModeBaseTime) - ((CurrentTime - Alley_Mode_Start_Time)/1000));
            // Over ride displays
            if (OverrideScorePriority == OVERRIDE_PRIORITY_ALLEY_MODE_COMBO) {
              OverrideScoreDisplay( ((CurrentPlayer%2)?((CurrentPlayer < 2)?0:2):((CurrentPlayer < 2)?1:3)), Alley_Reward, false);
              OverrideScoreDisplay( ((CurrentPlayer < 2)?2:0), Countdown, false);                   
              OverrideScoreDisplay( ((CurrentPlayer < 2)?3:1), Countdown, false);
            }
          } else {
            BallLighting();                                                     // Reset lamps
            // Initiate flashing pattern according to which step in the mode we are on
            // AlleyModeLetter steps from 1-4 steps
            // Lamp flashing sequence is shortened by ALLEY_MODE_ANIMATION_TIME amount 
            for (int count = 0; count < 4; count++) {
              if (AlleyModeSwitchCombinations[AlleyModeNumber - 1][count]) {
                // Upper row lamps
                AddToFlashingLampQueue((count), 0, CurrentTime, ((AlleyModeBaseTime*1000) - ALLEY_MODE_ANIMATION_TIME), 300, 75);  // Start lamp sequence
                // Lower row lamps - offset in time and shortened byt the same amount
                AddToFlashingLampQueue((count + 8), 0, (CurrentTime + 150), ((AlleyModeBaseTime*1000) - ALLEY_MODE_ANIMATION_TIME - 150), 300, 75);  // Start lamp sequence
              }
            }
            AlleyModeActive +=1;                                               // Move to next stage
          }
          break;
        case 2:                                                                 // Main combo section
          if ( (CurrentTime - Alley_Mode_Start_Time) < (AlleyModeBaseTime*1000) ) {
            unsigned int Countdown = ((AlleyModeBaseTime) - ((CurrentTime - Alley_Mode_Start_Time)/1000));
            //Serial.print(F("FutureModeBaseTime is:"));
            //Serial.println(FutureModeBaseTime, DEC);
            // Over ride displays
            if (OverrideScorePriority == OVERRIDE_PRIORITY_ALLEY_MODE_COMBO) {
              OverrideScoreDisplay( ((CurrentPlayer%2)?((CurrentPlayer < 2)?0:2):((CurrentPlayer < 2)?1:3)), Alley_Reward, false);
              OverrideScoreDisplay( ((CurrentPlayer < 2)?2:0), Countdown, false);                   
              OverrideScoreDisplay( ((CurrentPlayer < 2)?3:1), Countdown, false);
            }
          } else {                                                              // Timed out, close out mode
            AlleyModeFinish();                                                  // Close out mode
          }
          break;
        case 3:                                                                 // Player achieved all levels
          if ( (CurrentTime - Alley_Mode_Start_Time) < (ALLEY_MODE_ANIMATION_FINISH) ) {
            MarqueeAttract(3);                                                  // Animate top playfield region
            if (OverrideScorePriority == OVERRIDE_PRIORITY_ALLEY_MODE_COMBO) {
              OverrideScoreDisplay( ((CurrentPlayer%2)?((CurrentPlayer < 2)?0:2):((CurrentPlayer < 2)?1:3)), 40000, false);
              OverrideScoreDisplay( ((CurrentPlayer < 2)?2:0), 40000, false);                   
              OverrideScoreDisplay( ((CurrentPlayer < 2)?3:1), 40000, false);
            }
          } else {                                                              // Timed out, close out mode
            AlleyModeFinish();                                                  // Close out mode
          }
          break;
      } // end of Alley mode combo switch
    }




//
// Mode 4 - Roaming - NGP zzzzz
//

  if (GameMode[CurrentPlayer] == 4) {
    switch (RoamingStage) {
      case 1:                                                   // Animation until first switch hit
        { // Braces around case due to variable declaration
          if ( (CurrentTime - RoamingModeTime) < ((unsigned long)RoamingStageDuration*1000) ) {
            //Serial.println(F("Mode 4 Roaming stage 1"));
            if (OVERRIDE_PRIORITY_ROAMING_MODE > OverrideScorePriority) {          // Check if priority should be raised
              OverrideScorePriority = OVERRIDE_PRIORITY_ROAMING_MODE;
            }
            // Over ride displays
            if (OverrideScorePriority == OVERRIDE_PRIORITY_ROAMING_MODE) {
              unsigned int Countdown = (RoamingStageDuration - ((CurrentTime - RoamingModeTime)/1000));
              //Note: in stage one we only display a low RoamingScore value, no need to convert to unsigned long as in Stage 2
              OverrideScoreDisplay( ((CurrentPlayer%2)?((CurrentPlayer < 2)?0:2):((CurrentPlayer < 2)?1:3)), RoamingScores[(NumCapturedLetters)]*1000, false);
              OverrideScoreDisplay( ((CurrentPlayer < 2)?2:0), Countdown, false);
              OverrideScoreDisplay( ((CurrentPlayer < 2)?3:1), Countdown, false);
            }
            ShowLampAnimation(RoamingStage1a, 16, 136, CurrentTime, 15, false, false, 99); // Small lamps scanning
            ShowLampAnimation(RoamingStage1b, 18, 30, CurrentTime, 9, false, false, 99); // Rack looping
          } else {                                                // We have timed out
            PlaySoundEffect(SOUND_EFFECT_BALL_LOSS);
            RoamingFinish();
          }
        }
        break;
      case 2:                                                   // Rotating Letters
        { // Braces around case due to variable declaration
          if ( (CurrentTime - RoamingModeTime) < ((unsigned long)RoamingStageDuration*1000)) {
            //Serial.println(F("Mode 4 Roaming stage 2"));
            if (OVERRIDE_PRIORITY_ROAMING_MODE > OverrideScorePriority) {          // Check if priority should be raised
              OverrideScorePriority = OVERRIDE_PRIORITY_ROAMING_MODE;
            }
            if (OverrideScorePriority == OVERRIDE_PRIORITY_ROAMING_MODE) {
              // Override displays
              unsigned int Countdown = (RoamingStageDuration - ((CurrentTime - RoamingModeTime)/1000));
              unsigned long RoamingAwardScore = (unsigned long)(RoamingScores[NumCapturedLetters])*1000;
              OverrideScoreDisplay( ((CurrentPlayer%2)?((CurrentPlayer < 2)?0:2):((CurrentPlayer < 2)?1:3)), RoamingAwardScore, false);
              OverrideScoreDisplay( ((CurrentPlayer < 2)?2:0), Countdown, false);                   
              OverrideScoreDisplay( ((CurrentPlayer < 2)?3:1), Countdown, false);
            }
            if ( (CurrentTime - RoamingRotateTime) > RoamingIntervals[(NumCapturedLetters - 1)]) {
              RoamingRotateTime = CurrentTime;
              if (!ReverseRoam) { 
                // Shift up one letter
                RoamingBalls = ( (RoamingBalls & (0b011111111111111)) << 1) + ( (RoamingBalls & (0b100000000000000)) >> 14);
              } else {
                // Shift down one letter
                RoamingBalls = ( (RoamingBalls & (0b000000000000001)) << 14) + ( (RoamingBalls & (0b111111111111110)) >> 1);
              }
              BallLighting();
              PlaySoundEffect(SOUND_EFFECT_10_PTS + SpinnerDelta);
              SpinnerDelta += 1;
              if (SpinnerDelta > 3) SpinnerDelta = 0;             // Allow to use all 4 chimes
            }
          } else {                                                // We have timed out, do final animation
            RoamingStage = 3;
            OverrideScorePriority = 0;
            ShowPlayerScores(0xFF, false, false);                 // Reset all score displays
            RoamingModeTime = CurrentTime;
            WrapUpSoundPlayed = false;                            // Allow sound effect to play
          }
        }
        break;
      case 3:                                                   // Wrap-up animation
        if ( (CurrentTime - RoamingModeTime) < ROAMING_WRAP_UP_DURATION) {
            ShowLampAnimation(RoamingStage1a, 16, 91, CurrentTime, 15, false, false, 99); // Small lamps scanning
            ShowLampAnimation(RoamingStage1b, 18, 20, CurrentTime, 9, false, false, 99);  // Rack looping
            if (!WrapUpSoundPlayed) {
              WrapUpSoundPlayed = true;
              PlaySoundEffect(SOUND_EFFECT_ROAMING_END, true);
            }
        } else {                                                // We have completed wrap-up, shut off mode
          //PlaySoundEffect(SOUND_EFFECT_ROAMING_END, true);
          RoamingFinish();
        }
        break;
      case 4:                                                   // Mode completed, final light show
        if ( (CurrentTime - RoamingModeTime) < (ROAMING_WRAP_UP_DURATION*4/3) ) {
            ShowLampAnimation(RoamingStage1a, 16, 68, CurrentTime, 15, false, false, 99); // Small lamps scanning
            ShowLampAnimation(RoamingStage1b, 18, 15, CurrentTime, 9, false, false, 99);  // Rack looping
            if (!WrapUpSoundPlayed) {
              WrapUpSoundPlayed = true;
              PlaySoundEffect(SOUND_EFFECT_ROAMING_COMPLETE, true);
            }
        } else {                                                // We have completed wrap-up, shut off mode
          //PlaySoundEffect(SOUND_EFFECT_ROAMING_COMPLETE, true);
          RoamingFinish();
        }
        break;
    }
  } // End of Mode 4 NormalGamePlay




  // Check to see if ball is in the outhole
  if (RPU_ReadSingleSwitchState(SW_OUTHOLE)) {
    if (BallTimeInTrough==0) {
      BallTimeInTrough = CurrentTime;
    } else {  // -1-
      // Make sure the ball stays on the sensor for at least 
      // 0.5 seconds to be sure that it's not bouncing
      if ((CurrentTime-BallTimeInTrough)>500) {  // -2-
        if (BallFirstSwitchHitTime == 0 && !Tilted) {
//        if (BallFirstSwitchHitTime == 0 && NumTiltWarnings <= MaxTiltWarnings) {
          // Nothing hit yet, so return the ball to the player
          RPU_PushToTimedSolenoidStack(SOL_OUTHOLE, 4, CurrentTime);
          BallTimeInTrough = 0;
          returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
        } else {  // -3-
//        if (BallFirstSwitchHitTime==0) BallFirstSwitchHitTime = CurrentTime;
        
        // if we haven't used the ball save, and we're under the time limit, then save the ball
        if (  !BallSaveUsed && 
              ((CurrentTime-BallFirstSwitchHitTime)/1000)<((unsigned long)BallSaveNumSeconds) ) {
        
          RPU_PushToTimedSolenoidStack(SOL_OUTHOLE, 4, CurrentTime + 100);
          if (BallFirstSwitchHitTime>0) {
            BallSaveUsed = true;
            RPU_SetLampState(SAME_PLAYER, 0);
//            RPU_SetLampState(HEAD_SAME_PLAYER, 0);
          }
          BallTimeInTrough = CurrentTime;

          returnState = MACHINE_STATE_NORMAL_GAMEPLAY;          
        } else {
          // Note: Mode 4 is turned off instead at CountdownBonus
          if (GameMode[CurrentPlayer] == 3) {           // GameMode 3 ends with the ball
            (GameMode[CurrentPlayer] = 1);
          }
          ChaseBall = 0;                                // Reset Mode 3 in case active
          ChaseBallStage = 0;
          BIPDispValue = CurrentBallInPlay;             // Restore display in case pop count is displayed
          CreditsDispValue = Credits;                   // Restore display in case spinner count is displayed
          returnState = MACHINE_STATE_COUNTDOWN_BONUS;
        }
        } // -3-
      }  // -2-
    } // -1-
  } else {
    BallTimeInTrough = 0;
  }

  return returnState;
}



unsigned long InitGameStartTime = 0;
unsigned long InitGamePlayChime = 0;
boolean ChimeTrigger = true;

int InitGamePlay(boolean curStateChanged) {   //zzzzz
  int returnState = MACHINE_STATE_INIT_GAMEPLAY;

  if (curStateChanged) {
    InitGameStartTime = CurrentTime;
    RPU_SetCoinLockout((Credits>=MaximumCredits)?true:false);
    RPU_SetDisableFlippers(true);
    RPU_DisableSolenoidStack();
    RPU_TurnOffAllLamps();
    BIPDispValue = 1;
    //RPU_SetDisplayBallInPlay(1);
    if (Credits > 0) {
      RPU_SetLampState(LA_CREDIT_INDICATOR, 1);
    }
    InitGamePlayChime = CurrentTime+2500;
    ChimeTrigger = true;

    // Combined array variables for initialization

    for (int count=0; count<4; count++) {  // Set all to not captured
      OutlaneSpecial[count] = false;
      ArrowsLit[count] = false;
      EightBallTest[count] = true;
      PopCount[count]=0;
      //PopCountTest[count] = true;
      PopMode[count] = 0;
      Balls[count] = 0;
      Goals[count] = 0;
      FifteenBallQualified[count] = false;
      GameMode[count] = 1;              // default GameMode is 1
      FifteenBallCounter[count] = 0;
      SpinnerCount[count]=0;
      SpinnerMode[count]=0;
      SuperSpinnerAllowed[count] = false;
    }

    // Test value - force ball settings
    //           1   1 1
    //           6---2-0987654321
    //Balls[0] = 0b0101010100000000;
    //Balls[1] = 0b0000000000010101;
    //GameMode[0]=2;
    //
    // Bit 1 - SuperBonus reached
    // Bit 2 - NextBall met once
    // Bit 3 - Pop Mode > x
    // Bit 4 - Spinner Mode > x
    // Bit 5 - Spinner Combo achieved x times
    // Bit 6 - Scramble Ball
    // Bit 7 - 3 Goals achieved
    // Bit 8 - 5 Goals achieved
    //Goals[0]=0b00111100;
    
    // Turn on all small ball lights to start game
    for (int count=0; count<7; count++) {
      RPU_SetLampState((LA_SMALL_1+count+8), 1);
    }
    Serial.write("InitGame - state changed\n");

    // Set up general game variables

    CurrentPlayer = 0;
    CurrentNumPlayers = 1; // Added to match Meteor code
    CurrentBallInPlay = 1;
    SetPlayerLamps(CurrentNumPlayers);
    
    for (int count=0; count<4; count++) CurrentScores[count] = 0;
    // if the ball is in the outhole, then we can move on
    if (RPU_ReadSingleSwitchState(SW_OUTHOLE)) {
      Serial.println(F("InitGame - Ball in trough"));
      RPU_EnableSolenoidStack();
      RPU_SetDisableFlippers(false);
      returnState = MACHINE_STATE_INIT_NEW_BALL;
    } else {
   
    // Otherwise, let's see if it's in a spot where it could get trapped,
    // for instance, a saucer (if the game has one)
    //RPU_PushToSolenoidStack(SOL_SAUCER, 5, true);

    // And then set a base time for when we can continue
    InitGameStartTime = CurrentTime;
    }

    for (int count=0; count<4; count++) {
      RPU_SetDisplay(count, 0);
      RPU_SetDisplayBlank(count, 0x00);
    }
    ShowPlayerScores(0xFF, false, false);   //  Reset all score displays
  }


//
// Play game start tune
//

  if (ChimeTrigger) {
    ChimeTrigger = false;
    PlaySoundEffect(SOUND_EFFECT_GAME_START, true);
  }

  // Wait for TIME_TO_WAIT_FOR_BALL seconds, or until the ball appears
  // The reason to bail out after TIME_TO_WAIT_FOR_BALL is just
  // in case the ball is already in the shooter lane.
  if ((CurrentTime-InitGameStartTime)>TIME_TO_WAIT_FOR_BALL || RPU_ReadSingleSwitchState(SW_OUTHOLE)) {
    RPU_EnableSolenoidStack();
    RPU_SetDisableFlippers(false);
    returnState = MACHINE_STATE_INIT_NEW_BALL;
  }

  if (CurrentTime<=InitGamePlayChime) {
    returnState = MACHINE_STATE_INIT_GAMEPLAY;
  }

  return returnState;  
}


//
// Bonus Countdown - Ver 4
// SuperBonus handling added
// 15 Ball mode partially added
//

boolean CountdownDelayActive = false;
byte ChimeLoop = 0;
byte MaskCount = 0;
int BonusSpace = 117;
unsigned long NextBonusUpdate = 0;
unsigned long BonusWaitTime = 0;
unsigned int BonusBalls = 0;

int CountdownBonus(boolean curStateChanged) {       //zzzzz

  if (curStateChanged) {
    CreditsFlashing = false;                // Stop flashing
    BIPFlashing = false;                    // Stop flashing
    ShowPlayerScores(0xFF, false, false);   // Reset all score displays
    if (GameMode[CurrentPlayer] == 4 ) {    // If we are in Roaming mode revert to Mode 1
      RoamingFinish();
    }
    ClearFlashingLampQueue();               // Remove any lamps from queue
    BallLighting();                         // If we tilted balls would be out.
    BankShotLighting();                     // Reset lamps from 15 ball animation
    BonusMultiplier(BonusMult);             // Reset bonus lamps needed if Mode 2 animation was active
    RPU_SetLampState(LA_SPINNER, 0);
    if (GameMode[CurrentPlayer] == 2) RPU_SetLampState(LA_SUPER_BONUS, 0);
    MaskCount = 0;
    ChimeLoop = 0;
    BonusSpace = (!Tilted?267:134); // Interchime is 117, group gap is 267 msec
    NextBonusUpdate = CurrentTime;
    CountdownDelayActive = false;
    BonusWaitTime = 0;
    BonusBalls = Balls[CurrentPlayer];
    Serial.println(F("-----------------CountdownBonus-----------------"));
  }

// Code for counting down balls

  if (MaskCount < 15) {                                   // Count out all 15 balls
    if (BonusBalls & (0b1<<MaskCount)) {                  // If true, ball has been captured
      if ( ChimeLoop < (3 * ((!Tilted)?BonusMult:1)) ) {  // Add bonus multiplier to total
        if ((CurrentTime - NextBonusUpdate) > BonusSpace) {
          if (!Tilted) {
            RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, CurrentTime + 0, true);
            CurrentScores[CurrentPlayer] += 1000;
            if (GameMode[CurrentPlayer] == 2) {
              RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, CurrentTime + 0, true);
              CurrentScores[CurrentPlayer] += 1000;
            }
          }
          NextBonusUpdate = CurrentTime;
          ++ChimeLoop;
          if (ChimeLoop == 1) BonusSpace = ((!Tilted)?117:58);  // Create gap between chime groupings
        }
      } else {
          RPU_SetLampState((LA_BIG_1 + MaskCount), 0);                // Turn off lamp after chime sequence
          ChimeLoop = 0;
          BonusSpace = ((!Tilted)?267:134);  // Reset back to interchime spacing
//          BonusSpace = ((NumTiltWarnings < MaxTiltWarnings)?267:134);  // Reset back to interchime spacing
          ++MaskCount;
        }
    } else {
        ++MaskCount;
      }
    RPU_SetDisplay(CurrentPlayer, CurrentScores[CurrentPlayer], true);  
    return MACHINE_STATE_COUNTDOWN_BONUS;
  } 
  if (MaskCount == 15 && CountdownDelayActive == false) {
    BonusWaitTime = CurrentTime + 1000;
    CountdownDelayActive = true;
    return MACHINE_STATE_COUNTDOWN_BONUS;
  }
  if (CurrentTime < BonusWaitTime && CountdownDelayActive == true) {  //Delays transition back to regular play
    return MACHINE_STATE_COUNTDOWN_BONUS;
  }

  // Add loop to deal with SuperBonus
  if (BonusBalls & (0b1<<15)) {                            // Check if SuperBonus achieved
    MaskCount = 0;
    ChimeLoop = 0;
    BonusSpace = 267;                                   
    NextBonusUpdate = CurrentTime;
    CountdownDelayActive = false;
    BonusWaitTime = 0;
    RPU_SetLampState(LA_SUPER_BONUS, 0);
    if (GameMode[CurrentPlayer] == 1) {
      BonusBalls = (0b11111111<<(CurrentPlayer%2 ? 7:0)); // Reassign without SuperBonus so BonusCountdown can exit.
      //  Light 8 balls
      for(int i=0; i<8; i++) {
        RPU_SetLampState((LA_BIG_1 + i + (CurrentPlayer%2 ? 7:0)), 1);
      }
    } else {                                              // if 15 Ball Mode
      BonusBalls = 0x7FFF; // Reassign without SuperBonus so BonusCountdown can exit.
      //  Light 15 balls
      for(int i=0; i<15; i++) {
        RPU_SetLampState((LA_BIG_1 + i), 1);
      }
    }
    return MACHINE_STATE_COUNTDOWN_BONUS;
  }

    Serial.print(F("Bonus Countdown - end\n"));
    return MACHINE_STATE_BALL_OVER;

// End of CoundownBonus
} 

unsigned int BallTemp;

int RunGamePlayMode(int curState, boolean curStateChanged) {
  int returnState = curState;
  unsigned long scoreAtTop = CurrentScores[CurrentPlayer];
  
  // Very first time into gameplay loop
  if (curState==MACHINE_STATE_INIT_GAMEPLAY) {
    returnState = InitGamePlay(curStateChanged);    
  } else if (curState==MACHINE_STATE_INIT_NEW_BALL) {
    returnState = InitNewBall(curStateChanged, CurrentPlayer, CurrentBallInPlay);
  } else if (curState==MACHINE_STATE_NORMAL_GAMEPLAY) {
    returnState = NormalGamePlay();
  } else if (curState==MACHINE_STATE_COUNTDOWN_BONUS) {
    returnState = CountdownBonus(curStateChanged);
  } else if (curState==MACHINE_STATE_BALL_OVER) {
    if (SamePlayerShootsAgain) {
      returnState = MACHINE_STATE_INIT_NEW_BALL;
    } else {
      CurrentPlayer+=1;
      if (CurrentPlayer>=CurrentNumPlayers) {
        CurrentPlayer = 0;
        CurrentBallInPlay+=1;
      }
        
      if (CurrentBallInPlay>BallsPerGame) {
        CheckHighScores();
        PlaySoundEffect(SOUND_EFFECT_GAME_OVER);
        SetPlayerLamps(0, 4);
        for (int count=0; count<CurrentNumPlayers; count++) {
          RPU_SetDisplay(count, CurrentScores[count], true, 2);
        }

        returnState = MACHINE_STATE_MATCH_MODE;
      }
      else returnState = MACHINE_STATE_INIT_NEW_BALL;
    }
  } else if (curState==MACHINE_STATE_MATCH_MODE) {
    //Serial.print(F("Run ShowMatchSequence\n"));
    returnState = ShowMatchSequence(curStateChanged);
//  This line does nothing but force another pass until Attract gets called since nothing matches state
//    returnState = MACHINE_STATE_GAME_OVER;    
  } else {
    returnState = MACHINE_STATE_ATTRACT;
  }

  byte switchHit;
    while ( (switchHit=RPU_PullFirstFromSwitchStack())!=SWITCH_STACK_EMPTY ) {   // -A-

      if (!Tilted) {
  
      switch (switchHit) {
        case SW_SELF_TEST_SWITCH:
          returnState = MACHINE_STATE_TEST_LIGHTS;
          SetLastSelfTestChangedTime(CurrentTime);
          break; 

  //
  // Eight Ball game switch responses here
  //
  
        case SW_10_PTS:
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          Ten_Pts_Stack += 1;
          //PlaySoundEffect(SOUND_EFFECT_ROAMING_COMPLETE);
          if (ArrowsLit[CurrentPlayer]) ArrowToggle();
          RoamingRotate();
          // Test code
          //AlleyModeStart();
          break;
        case SW_TOP_LEFT_TARGET:
          //
          // Scramble ball - only functions in modes 1 and 2
          //
          if ( ((PopMode[CurrentPlayer]) > 0) && ((GameMode[CurrentPlayer] == 1) || (GameMode[CurrentPlayer] == 2)) ) {
            for (int i = 0; i < (1 + CurrentTime%5); i++) {   // Rotate balls by 1 to 6 slots
              BallTemp  = ( (Balls[CurrentPlayer] & (0b11111100111111)) << 1) + ( (Balls[CurrentPlayer] & (0b100000001000000)) >> 6);
              Balls[CurrentPlayer] = Balls[CurrentPlayer] & (0b1000000010000000);                 // Clear ball bits
              Balls[CurrentPlayer] = Balls[CurrentPlayer] | BallTemp;                             // Set ball bits
              #ifdef EXECUTION_MESSAGES
                Serial.print(F("ScrambleBall jumped by: "));
                Serial.println((1 + CurrentTime%5),DEC);
              #endif
            }
            // Play SOUND_EFFECT_BALL_OVER only if goal has not been set
            if (!(Goals[CurrentPlayer] & (0b1<<5))) {         // If Goal 6 not set
              PlaySoundEffect(SOUND_EFFECT_SCRAMBLE_BALL, true);
            }
            SetGoals(6);                                      // Set goal as completed
            //NextBall = 0;                                     // Cancel NextBall in case active
            //NextBallTime = 0;
            NextBallFinish();
            MarqueeTiming();
            PlaySoundEffect(SOUND_EFFECT_BALL_CAPTURE);       // Non-priority so overidden by SOUND_EFFECT_SCRAMBLE_BALL above
            Silent_Thousand_Pts_Stack += 2;
            Silent_Hundred_Pts_Stack += 5;
          } else {
            Hundred_Pts_Stack += 5;
          }
          break;
        case SW_7_15_BALL:
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          if (!CaptureBall(7)) {  // If ball already captured, CaptureBall is false
            Hundred_Pts_Stack += 5;
          }
          break;
        case SW_6_14_BALL:
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          if (!CaptureBall(6)) {  // If ball already captured, CaptureBall is false
            Hundred_Pts_Stack += 5;
          }
          break;
        case SW_5_13_BALL:
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          if (!CaptureBall(5)) {  // If ball already captured, CaptureBall is false
            Hundred_Pts_Stack += 5;
          }
          break;
        case SW_4_12_BALL:
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          if (!CaptureBall(4)) {  // If ball already captured, CaptureBall is false
            Hundred_Pts_Stack += 5;
          }
          break;
        case SW_3_11_BALL: // Balls[]
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          if (!CaptureBall(3) || (GameMode[CurrentPlayer] == 2)) {  // If ball already captured, CaptureBall is false
            if (ArrowsLit[CurrentPlayer] && RightArrow) {
              if (BankShotProgress < 4) {
                if (GameMode[CurrentPlayer] != 2) {
                  Hundred_Pts_Stack += 5;
                } 
              } else {
                Silent_Hundred_Pts_Stack += 5;
              }
              BankShotScoring((GameMode[CurrentPlayer] == 2)? 0:1); // Call BankShotScoring
            } else {
              if (GameMode[CurrentPlayer] != 2) {
                Hundred_Pts_Stack += 5;
              } /*else {
                Silent_Hundred_Pts_Stack += 5;
              }*/
            }
          }
          break;
        case SW_2_10_BALL: // Balls[]
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          if (!CaptureBall(2) || (GameMode[CurrentPlayer] == 2)) {  // If ball already captured, CaptureBall is false
            if (ArrowsLit[CurrentPlayer] && !RightArrow) {
              if (BankShotProgress < 4) {
                if (GameMode[CurrentPlayer] != 2) {
                  Hundred_Pts_Stack += 5;
                }
              } else {
                Silent_Hundred_Pts_Stack += 5;
              }
              BankShotScoring((GameMode[CurrentPlayer] == 2)? 0:1); // Call BankShotScoring
            } else {
              if (GameMode[CurrentPlayer] != 2) {
                Hundred_Pts_Stack += 5;
              } /*else {
                Silent_Hundred_Pts_Stack += 5;
              }*/
            }
          }
          break;
        case SW_1_9_BALL: // Balls[]
          if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
          if (!CaptureBall(1)) {  // If ball already captured, CaptureBall is false
            Hundred_Pts_Stack += 5;
          }
          break;
        case SW_8_BALL:
          switch (GameMode[CurrentPlayer]) {
            case 1:
              if (((Balls[CurrentPlayer] & (0b11111111<<(CurrentPlayer%2 ? 7:0)))>>(CurrentPlayer%2 ? 7:0))==(CurrentPlayer%2 ? 254:127)) { // Check 8 bits, shift by 7 since 8 is common.
                // Collect 8 Ball turn out small lamp
                RPU_SetLampState(LA_SMALL_8, (MarqueeDisabled)?0:1, 0, 75);
                MarqueeTiming();    // Call multiple times for longer animmation length
                MarqueeTiming();
                MarqueeTiming();
                MarqueeTiming();
                #ifdef EXECUTION_MESSAGES
                Serial.print(F("-------- Mode 1 8 Ball awarded --------"));
                #endif
                RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 900, true);
                Balls[CurrentPlayer] = (Balls[CurrentPlayer] | 0b10000000);   // Raise flag for 8 Ball
                PlaySoundEffect(SOUND_EFFECT_8_BALL_CAPTURE);
                Silent_Hundred_Pts_Stack += 5;
              } else {
                Hundred_Pts_Stack += 5;
              }
              break;
            case 2:
              if ((Balls[CurrentPlayer] & (0x7FFF))==(0x7F7F)) { // Check balls 1-7, 9-15.
                // Collect 8 Ball turn out small lamp
                RPU_SetLampState(LA_SMALL_8, (MarqueeDisabled)?0:1, 0, 75);
                MarqueeTiming();    // Call multiple times for longer animmation length
                MarqueeTiming();
                MarqueeTiming();
                MarqueeTiming();
                MarqueeTiming();
                MarqueeTiming();
                #ifdef EXECUTION_MESSAGES
                Serial.print(F("-------- 15 Ball 8 Ball awarded --------"));
                #endif
                RPU_SetDisableFlippers(true);
                FifteenBallCounter[CurrentPlayer] += 5;                       // Force mode to reset
                SamePlayerShootsAgain = true;                                 // Save players ball
                RPU_SetLampState(SAME_PLAYER, 1, 0, 150);
                RPU_SetLampState(LA_SPSA_BACK, 1, 0, 175);
                RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 900, true);
                RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 1000, true);
                RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 1100, true);
                Balls[CurrentPlayer] = (Balls[CurrentPlayer] | 0b10000000);   // Raise flag for 8 Ball
                PlaySoundEffect(SOUND_EFFECT_8_BALL_CAPTURE);
                Silent_Thousand_Pts_Stack += 100;
              } else {
                Hundred_Pts_Stack += 5;
              }
              break;
            case 4:
              #ifdef EXECUTION_MESSAGES
              Serial.println(F("-------- Roaming Mode Ball 8 Ball switch hit--------"));
              #endif
              if (!CaptureBall(8)) {  // If ball already captured, CaptureBall is false
                Hundred_Pts_Stack += 5;
              }
              break;
          }
          #ifdef EXECUTION_MESSAGES
          Serial.println(F("SW_8 SpinnerKicker code section"));
          #endif
          if (!SpinnerKickerLit) {
            SpinnerKickerLit = true;
            // Kicker/Spinner could be in a flashing event so remove 
            ClearFlashingLampQueueEntry(LA_SPINNER);
            RPU_SetLampState(LA_SPINNER, 1);
            KickerReady = true;  // KickerReady always true when SpinnerKickerLit
            KickerUsed = false;
            KickerOffTime = 0;
          }  
          break;
        case SW_SPINNER:
          SpinnerCount[CurrentPlayer] = ++SpinnerCount[CurrentPlayer];
          if (SpinnerCount[CurrentPlayer] > (Spinner_Threshold - 1)) {
            PlaySoundEffect(SOUND_EFFECT_BALL_OVER, true);            // Plays each increment
            SpinnerMode[CurrentPlayer] += 1;
            SetGoals(4);
            SpinnerCount[CurrentPlayer] = 0;
            if (SpinnerMode[CurrentPlayer] > 1) {
              SuperSpinnerAllowed[CurrentPlayer] = true;
              MarqueeDisabled = true;
              ClearRack();
              FlashingArrows(RackRightArrow, 125);
            }
          }
          #ifdef EXECUTION_MESSAGES
          Serial.print(F("Spinner Mode is:  "));
          Serial.println(SpinnerMode[CurrentPlayer], DEC);  
          Serial.print(F("Spinner Count is: "));
          Serial.println(SpinnerCount[CurrentPlayer], DEC);  
          #endif
          // Update timer
          if ( (SpinnerMode[CurrentPlayer] > 1) && (SuperSpinnerAllowed[CurrentPlayer] == true)/* && (SuperSpinnerTime == 0)*/ ){
            SuperSpinnerTime = CurrentTime;
            //Serial.print(F("SuperSpinnerTime is: "));
            //Serial.println(SuperSpinnerTime, DEC);  
          }
          //
          // Award SuperSpinnerCombo
          //
          if ( (SuperSpinnerAllowed[CurrentPlayer] == true) && (SpinnerComboHalf) ) {
            //SuperSpinnerAllowed[CurrentPlayer] == false;
            SpinnerComboHalf = false;
            SuperSpinnerDuration = 0;
            Silent_Thousand_Pts_Stack +=25;
            PlaySoundEffect(SOUND_EFFECT_SPINNER_COMBO, true);
            SetGoals(5);
          }
          if ( (SpinnerCount[CurrentPlayer] > (Spinner_Threshold - (Spinner_Threshold*.66))) && (SpinnerCount[CurrentPlayer] < Spinner_Threshold) ) {
          //if ( (SpinnerCount[CurrentPlayer] > (Spinner_Threshold - SPINNER_SHOW_IN_DISPLAY)) && (SpinnerCount[CurrentPlayer] < Spinner_Threshold) ) {
            CreditsFlashing = true;
            CreditsDispValue = (Spinner_Threshold - SpinnerCount[CurrentPlayer]);
          } else {
            CreditsFlashing = false;
            CreditsDispValue = Credits;
            //RPU_SetDisplayCredits(Credits);
          }
          // Spinner scoring
          if (!SpinnerKickerLit) {
            PlaySoundEffect(SOUND_EFFECT_10_PTS);
            CurrentScores[CurrentPlayer] += 10;
          } else if (SpinnerMode[CurrentPlayer] < 1){ 
            PlaySoundEffect(SOUND_EFFECT_1000_PTS);
            CurrentScores[CurrentPlayer] += 1000;
          } else {
            PlaySoundEffect(SOUND_EFFECT_10_PTS + SpinnerDelta);
            SpinnerDelta += 1;
            if (SpinnerDelta > 2) SpinnerDelta = 0;
            CurrentScores[CurrentPlayer] += ((SpinnerMode[CurrentPlayer] == 1)?500:0);
            //Silent_Hundred_Pts_Stack +=((SpinnerMode[CurrentPlayer] == 1)?5:0);
            Silent_Thousand_Pts_Stack +=1;
            if (SpinnerMode[CurrentPlayer] > 1) {
              PlaySoundEffect(SOUND_EFFECT_EXTRA);
              Silent_Thousand_Pts_Stack +=1;
            }
          }
          if (ArrowsLit[CurrentPlayer]) ArrowToggle();
          RoamingRotate();
          break;
        case SW_LEFT_OUTLANE:
          if (!KickerReady) {
            PlaySoundEffect(SOUND_EFFECT_1000_PTS);
            Silent_Hundred_Pts_Stack +=10;
          } else if ((KickerReady) && (!KickerUsed)) {
              SpinnerKickerLit = false;                  // Kicker turns off SpinnerKicker mode, leaves KickerReady running
              AddToFlashingLampQueue(LA_SPINNER, 0, CurrentTime, KICKER_SAVE_DURATION, 200, 50);
              //AddToFlashingLampQueue(LA_SPINNER, 0, CurrentTime, KICKER_SAVE_DURATION, 300, 75);
              #ifdef EXECUTION_MESSAGES
              Serial.println(F("Kicker saver flashing sequence started"));
              #endif
              RPU_PushToSolenoidStack(SOL_KICKER, 4);
              Thousand_Pts_Stack += 5;
              KickerUsed = true;
              KickerOffTime = CurrentTime;               // X seconds delay starting point
          } else {  // if ((KickerReady) && (KickerUsed)) - only case left
              RPU_PushToSolenoidStack(SOL_KICKER, 4);
              PlaySoundEffect(SOUND_EFFECT_KICKER_OUTLANE);
          }
          break;
        case SW_RIGHT_OUTLANE:
          if (!OutlaneSpecial[CurrentPlayer]) {
          RPU_PushToSolenoidStack(SOL_CHIME_1000, 3);
          CurrentScores[CurrentPlayer] += 1000;          
          } else {
          Thousand_Pts_Stack += 15;
          RPU_SetLampState(LA_OUTLANE_SPECIAL, 0);  // Turn lamp off after collecting Special
          OutlaneSpecial[CurrentPlayer] = false;     // Set to false, now collected.
          }
          break;
        case SW_SLAM:
          RPU_DisableSolenoidStack();
          RPU_SetDisableFlippers(true);
          RPU_TurnOffAllLamps();
          RPU_SetLampState(GAME_OVER, 1);
          delay(1000);
          return MACHINE_STATE_ATTRACT;
          break;
        case SW_TILT:
          // This should be debounced
          //Serial.print(F("SW_TILT was hit and NumTiltWarnings is:             "));
          //Serial.println(NumTiltWarnings, DEC);
          //Serial.print(F("Current Time is:                                    "));
          //Serial.println(CurrentTime, DEC);
          //Serial.print(F("LastTiltWarningTime is:                             "));
          //Serial.println(LastTiltWarningTime, DEC);
          //Serial.print(F("(CurrentTime - LastTiltWarningTime) is:             "));
          //Serial.println((CurrentTime - LastTiltWarningTime), DEC);
          //Serial.println();
          if ((CurrentTime - LastTiltWarningTime) > TILT_WARNING_DEBOUNCE_TIME) {
            //Serial.println(F("Greater than Tilt Debounce time"));
            LastTiltWarningTime = CurrentTime;
            NumTiltWarnings += 1;
            //Serial.print(F("NumTiltWarnings after increment is:                 "));
            //Serial.println(NumTiltWarnings, DEC);
            if (NumTiltWarnings > MaxTiltWarnings) {
              //Serial.print(F("(NumTiltWarnings > MaxTiltWarnings) is:              "));
              //Serial.println((NumTiltWarnings > MaxTiltWarnings), DEC);
              Tilted = true;
              #ifdef EXECUTION_MESSAGES
              Serial.println(F("!!!! Tilted !!!!"));
              #endif
              RPU_SetDisableFlippers(true);
              RPU_TurnOffAllLamps();
              PlaySoundEffect(SOUND_EFFECT_TILTED, true);
              RPU_SetLampState(TILT, 1);
              RPU_DisableSolenoidStack();
            }
            #ifdef EXECUTION_MESSAGES
            Serial.println(F("Tilt Warning!!!!"));
            #endif
            PlaySoundEffect(SOUND_EFFECT_TILT_WARNING, true);
          }
          break;
          case SW_BANK_SHOT:
          BankShotScoring(); // Call BankShotScoring
          if (SuperSpinnerAllowed[CurrentPlayer] == true){
            SpinnerComboHalf = true;
            SuperSpinnerDuration = Spinner_Combo_Duration;
            SuperSpinnerTime = CurrentTime;
            //Serial.print(F("SuperSpinnerTime is: "));
            //Serial.println(SuperSpinnerTime, DEC);  
            ClearRack();
            FlashingArrows(RackLeftArrow, 125);
          }
          break;
        case SW_BUMPER_BOTTOM:
          PopDelta = ++PopDelta;
        case SW_BUMPER_RIGHT:
          PopDelta = ++PopDelta;
        case SW_BUMPER_LEFT:
          PopCount[CurrentPlayer] = ++PopCount[CurrentPlayer];
          if (PopCount[CurrentPlayer] > (Pop_Threshold-1)) {
            PopMode[CurrentPlayer] += 1;
            SetGoals(3);
            PopCount[CurrentPlayer] = 0;
            PlaySoundEffect(SOUND_EFFECT_POP_MODE, true);
          }
          #ifdef EXECUTION_MESSAGES
          Serial.print(F("PopMode is:  "));
          Serial.println(PopMode[CurrentPlayer], DEC);  
          Serial.print(F("PopCount is: "));
          Serial.println(PopCount[CurrentPlayer], DEC);  
          #endif
          if ( (PopCount[CurrentPlayer] > (Pop_Threshold*.33)) && (PopCount[CurrentPlayer] < Pop_Threshold) ) {
              //  Set display flashing and change to pop hits left until mode achieved
              BIPFlashing = true;
              BIPDispValue = (Pop_Threshold - PopCount[CurrentPlayer]);
            } else {
              //  Set display static and set back to BIP
              BIPFlashing = false;
              BIPDispValue = CurrentBallInPlay;
            }
          if ((PopMode[CurrentPlayer]) < 1) {
            PlaySoundEffect(SOUND_EFFECT_POP_100);
            Silent_Hundred_Pts_Stack +=1;
            //CurrentScores[CurrentPlayer] += 100;
          } else if ((PopMode[CurrentPlayer]) == 1) {
            PlaySoundEffect(SOUND_EFFECT_POP_1000 + PopDelta);        
            Silent_Thousand_Pts_Stack += 1;
            //CurrentScores[CurrentPlayer] += 1000;          
          } else if ((PopMode[CurrentPlayer]) > 1) {
            PlaySoundEffect(SOUND_EFFECT_POP_1000 + PopDelta);        
            Silent_Thousand_Pts_Stack += 2;
            //CurrentScores[CurrentPlayer] += 2000;          
          }
          PopDelta = 0;
          if (ArrowsLit[CurrentPlayer]) ArrowToggle();
          RoamingRotate();
          break;
        case SW_RIGHT_SLING:
        case SW_LEFT_SLING:
          RPU_PushToSolenoidStack(SOL_CHIME_100, 3);
          CurrentScores[CurrentPlayer] += 100;
          if (ArrowsLit[CurrentPlayer]) ArrowToggle();
          RoamingRotate();
          break;
        case SW_COIN_1:
        case SW_COIN_2:
        case SW_COIN_3:
          AddCoinToAudit(switchHit);
          AddCredit(true);
          CreditsDispValue = Credits;
          //RPU_SetDisplayCredits(Credits, true);
          if (Credits > 0) {
            RPU_SetLampState(LA_CREDIT_INDICATOR, 1);
          }
          break;
        case SW_CREDIT_RESET:
          if (CurrentBallInPlay<2) {
            // If we haven't finished the first ball, we can add players
            AddPlayer();
            if (Credits == 0) {
              RPU_SetLampState(LA_CREDIT_INDICATOR, 0);
            }
          } else {
            // If the first ball is over, pressing start again resets the game
            if (Credits >= 1 || FreePlayMode) {
              if (!FreePlayMode) {
                Credits -= 1;
                RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
                CreditsDispValue = Credits;
                //RPU_SetDisplayCredits(Credits);
              }
              returnState = MACHINE_STATE_INIT_GAMEPLAY;
            }
          }
          if (DEBUG_MESSAGES) {
            Serial.write("Start game button pressed\n\r");
          }
          break;        
      }
     }
    }  // -A-

  if (scoreAtTop != CurrentScores[CurrentPlayer]) {
    //Serial.print(F("Score changed\n"));
    LastTimeScoreChanged = CurrentTime;                 // To control Score dashing 
      for (int awardCount = 0; awardCount < 3; awardCount++) {
        if (AwardScores[awardCount] != 0 && scoreAtTop < AwardScores[awardCount] && CurrentScores[CurrentPlayer] >= AwardScores[awardCount]) {
          // Player has just passed an award score, so we need to award it
          if (!SamePlayerShootsAgain) {
            SamePlayerShootsAgain = true;
            RPU_SetLampState(SAME_PLAYER, 1);
            RPU_SetLampState(LA_SPSA_BACK, 1);        
            PlaySoundEffect(SOUND_EFFECT_EXTRA_BALL, true);
          }
        }
      }
  }
  
  return returnState;
}


void loop() {
  // This line has to be in the main loop
  RPU_DataRead(0);

  CurrentTime = millis();
  int newMachineState = MachineState;

  // Machine state is self-test/attract/game play
  if (MachineState<0) {
    newMachineState = RunSelfTest(MachineState, MachineStateChanged);
  } else if (MachineState==MACHINE_STATE_ATTRACT) {
    newMachineState = RunAttractMode(MachineState, MachineStateChanged);
  } else {
    newMachineState = RunGamePlayMode(MachineState, MachineStateChanged);
  }

  if (newMachineState!=MachineState) {
    MachineState = newMachineState;
    MachineStateChanged = true;
  } else {
    MachineStateChanged = false;
  }

  RPU_ApplyFlashToLamps(CurrentTime);
  RPU_UpdateTimedSolenoidStack(CurrentTime);

  // Toggle Credits and BIP displays when needed
  if (MachineState >= 0) {
    RPU_SetDisplayCredits(CreditsDispValue, (CreditsFlashing)?((1-(CurrentTime/125)%2)):1, true);
    RPU_SetDisplayBallInPlay(BIPDispValue, (BIPFlashing)?((CurrentTime/125)%2):1, true);
  }

}

//
// **************************
// *  Eight Ball functions  *
// **************************
//

// SweepAnimation - amimation by frames is controlled by global animation variable AttractSweepLights
//
//  animationName[]  = struct defining lamps by animation frame
//  numLamps         = Number of lamps in animation
//  totalSteps       = How many steps to take before repeating
//  stepTime         = msec frame length
//  activeRows       = How many lamp frames are lit at any time

void SweepAnimation(struct AttractLampsDown animationName[], byte numLamps, byte totalSteps, 
                                            unsigned long stepTime, byte activeRows) {
  if ((CurrentTime - AttractSweepTime) > stepTime) {                          // Animation frame length
    AttractSweepTime = CurrentTime;
    for (byte lightcountdown=0; lightcountdown < numLamps; lightcountdown++) {
      byte dist = AttractSweepLights - animationName[lightcountdown].rowDown;
      RPU_SetLampState(animationName[lightcountdown].lightNumDown, (dist < activeRows), 0);
    }
    AttractSweepLights += 1;
    if (AttractSweepLights > totalSteps) AttractSweepLights = 0;
  }
}



// Classic attract mode with choice of speed.  
//   howFast is msec step size
//   smallLamps true adds in the small lamps in time with large ones
//   Playfield attract mode is 5
//   Added in a step with no lamps to make repeating cycles look better


void ClassicAttract(int howFast, boolean bigLamps, boolean smallLamps) {
  if (AttractPlayfieldMode != 5) {
    AttractPlayfieldMode = 5;
    RPU_TurnOffAttractLamps();
    AttractStepLights = 0;
    if (DEBUG_MESSAGES) {
      Serial.println("Classic Attract");
    }
  }
  if ((CurrentTime-AttractStepTime) > RackDelayLength) { // global variable msec delay between steps
    RackDelayLength = howFast;
    AttractStepTime = CurrentTime;
    AttractPlayfieldMode = 5;
    if (AttractStepLights < 15) {                        // Light lamps in order
      if (bigLamps) RPU_SetLampState((LA_BIG_1+AttractStepLights), 1, 0, 0);
      if (smallLamps) RPU_SetLampState((LA_SMALL_1+AttractStepLights), 1, 0, 0);
      //Serial.print("AttractStepLights: ");
      //Serial.println(AttractStepLights, DEC);
      AttractStepLights +=1;
    } else if (AttractStepLights == 15) {                // Blank rack for one time step
      if (bigLamps) ClearRack();
      if (smallLamps) ClearSmallBalls();
      //Serial.print("AttractStepLights: ");
      //Serial.println(AttractStepLights, DEC);
      AttractStepLights = 0;                             
    }
  }
}


// Roaming rotate balls forward
//   Bumpers, slings, spinner, call this during Roaming
//   Stop rotating when  ball count exceeds x

void RoamingRotate() {
  if (GameMode[CurrentPlayer] == 4) {                                  // Only active during Roaming
    unsigned int countOnes = RoamingBalls;
    byte numOnes = 0;
    for (int count = 0; count < 15; count++) {
      if ( (countOnes & 0b1) == 1 ) {
        numOnes++;
      }
      countOnes >>= 1;
    }
    if ( (numOnes > 0) && (numOnes < ROAMING_ROTATE_LAMP_LIMIT) ) {
      RoamingBalls = ( (RoamingBalls & (0b011111111111111)) << 1) + ( (RoamingBalls & (0b100000000000000)) >> 14);
      BallLighting();
    }
  }
}

// Cancel Next Ball

void NextBallFinish() {
  #ifdef EXECUTION_MESSAGES
  Serial.println(F("NextBallFinish"));
  #endif
  // Need to avoid clearing out any flashing lamp in the 1-4 Alleys if AlleyMode is active
    if (AlleyModeActive == 0) {
      ClearFlashingLampQueueEntry(NextBall - 1 + (CurrentPlayer%2 ? 8:0)); // Clear out any NextBall lamp from queue
      #ifdef EXECUTION_MESSAGES
      Serial.println(F("Previous NextBall timed out, flashing lamp is cancelled"));
      #endif
    } else if ( (NextBall > 4) && (NextBall < 8) ) {
      ClearFlashingLampQueueEntry(NextBall - 1 + (CurrentPlayer%2 ? 8:0));// Only clear out targets 5-7 if AlleyMode running
      #ifdef EXECUTION_MESSAGES
      Serial.println(F("Previous NextBall timed out, flashing lamp is cancelled and AlleyMode active, only clear flashing lamp 5-7"));
      #endif
    }
  BallLighting();
  NextBall = 0;
  NextBallTime = 0;                                                    // Set counter to zero
  OverrideScorePriority = 0;                                           // Set priority back to 0
  ShowPlayerScores(0xFF, false, false);                                // Reset all score displays
}



// Trigger AlleyMode - Handles removal of NextBall flashing in alley region

void AlleyModeStart() {
  if (GameMode[CurrentPlayer] == 1) {
    AlleyModePopsTrigger = false;                      // Set triggers to false and start mode
    AlleyModeSpinnerTrigger = false;
    AlleyModeActive = 1;                               // Triggers mode
    Alley_Mode_Start_Time = CurrentTime;
    AlleyModeNumber = 1;                               // First pattern
    // Need to remove flashing NextBall if it is in the Alley region
    if ( (NextBall > 0) && (NextBall < 5) ) {
      byte Cleared = ClearFlashingLampQueueEntry(NextBall - 1 + (CurrentPlayer%2 ? 8:0)); // Clear out any NextBall flashing lamps in alleys
      //Serial.print(F("NextBall flashing lamp in alleys was removed yes/no: "));
      //Serial.println(Cleared, DEC);
    }
    #ifdef EXECUTION_MESSAGES
    Serial.println(F("Started Alley mode."));
    #endif
  }
}

// Cancel AlleyMode

void AlleyModeFinish() {
  PlaySoundEffect(SOUND_EFFECT_BALL_LOSS);
  AlleyModeActive = 0;                                                // Turn off mode
  Alley_Mode_Start_Time = 0;                                          // Reset clock
  for (int count = 0; count < 4; count++) {
    ClearFlashingLampQueueEntry(count);                               // Remove any flashing Alley lane lamps 0-3
    ClearFlashingLampQueueEntry(count + 8);                           // Remove any flashing Alley lane lamps 8-11
  }
  AlleyModeNumber = 0;                                                // Disable further rewards
  OverrideScorePriority = 0;                                          // Set back to zero
  ShowPlayerScores(0xFF, false, false);                               // Reset all score displays
  #ifdef EXECUTION_MESSAGES
  Serial.println(F("AlleyModeFinish completed"));
  #endif
  BallLighting();                                                     // Reset lamps
}


boolean ClearFlashingLampQueueEntry(byte clearSingleEntry) {
  for (int count=0; count<FLASHING_LAMP_QUEUE_SIZE; count++) {
    if (FlashingLampQueue[count].lampNumber == clearSingleEntry) {
      RPU_SetLampState(FlashingLampQueue[count].lampNumber, FlashingLampQueue[count].finishState);
      FlashingLampQueue[count].lampNumber = 0xFF;
      #ifdef EXECUTION_MESSAGES
      Serial.println(F("FlashingLampQueue single entry cleared"));
      #endif
      return true;
    }
  }
  #ifdef EXECUTION_MESSAGES
  Serial.println(F("No FlashingLampQueue single entry found to remove"));
  #endif
  return false;
}

//
//  Clear flashing lamp queue - Ver 1
//    If emptyAndSetLamp is true, lamps will be set to requested end state when cleared from queue
//    if emptyAndSetLamp is false, lamp is cleared from queue, lamp is left in current state
//
void ClearFlashingLampQueue(boolean emptyAndSetLamp) {
  for (int count=0; count<FLASHING_LAMP_QUEUE_SIZE; count++) {
    if ((FlashingLampQueue[count].lampNumber != 0xFF) && emptyAndSetLamp) {
      RPU_SetLampState(FlashingLampQueue[count].lampNumber, FlashingLampQueue[count].finishState);
    }
    FlashingLampQueue[count].lampNumber = 0xFF;
  }
  Serial.println(F("FlashingLampQueue cleared"));
  FlashingLampQueueEmpty = true;
}


boolean AddToFlashingLampQueue(byte lampNumber, byte finishState, unsigned long startTime, 
                 unsigned long duration, unsigned long startPeriod, unsigned long endPeriod) {
  for (int count=0; count<FLASHING_LAMP_QUEUE_SIZE; count++) {    // Check for existing entry
    if (FlashingLampQueue[count].lampNumber == lampNumber) {
      FlashingLampQueue[count].lampNumber = 0xFF;                 // Remove from queue
      Serial.println(F("FlashingLampQueue duplicate removed"));
    }
  }
  for (int count=0; count<FLASHING_LAMP_QUEUE_SIZE; count++) {
    if (FlashingLampQueue[count].lampNumber == 0xFF) {
      FlashingLampQueue[count].lampNumber = lampNumber;
      FlashingLampQueue[count].currentState = 1;                  // Start sequence with lamp on
      FlashingLampQueue[count].finishState = finishState;
      FlashingLampQueue[count].startTime = startTime;             // 1st processing will turn lamp to current state
      FlashingLampQueue[count].nextTime = startTime;              // Next incremental lamp transition
      if (duration < 100) duration = 100;                         // Prevent overly short events
      FlashingLampQueue[count].duration = duration;
      FlashingLampQueue[count].startPeriod = startPeriod/2;       // Set to half period
      FlashingLampQueue[count].endPeriod = endPeriod/2;           // Set to half period
      FlashingLampQueueEmpty = false;
      Serial.println(F("FlashingLampQueue entry added"));

      return true;
    }
  }
  Serial.println(F("FlashingLampQueue is full"));
  return false;
}

void ProcessFlashingLampQueue(unsigned long pullTime) {
  //FlashingLampQueueEmpty = true;
  for (int count=0; count<FLASHING_LAMP_QUEUE_SIZE; count++) {
    if (FlashingLampQueue[count].lampNumber!=0xFF && FlashingLampQueue[count].nextTime < pullTime) {
      FlashingLampQueueEmpty = false;
      unsigned long nextdelta = 0;
      if (FlashingLampQueue[count].startPeriod > FlashingLampQueue[count].endPeriod) {
        nextdelta = FlashingLampQueue[count].startPeriod -
          (( (pullTime - FlashingLampQueue[count].startTime) * 
          (FlashingLampQueue[count].startPeriod - FlashingLampQueue[count].endPeriod) ) / 
          FlashingLampQueue[count].duration);
        //Serial.print(F("Flashing lamp queue nextdelta is: "));
        //Serial.println(nextdelta, DEC);
        if (nextdelta < FlashingLampQueue[count].endPeriod)  {      // trigger for ending an event
          RPU_SetLampState(FlashingLampQueue[count].lampNumber, FlashingLampQueue[count].finishState);
          FlashingLampQueue[count].lampNumber = 0xFF;
          #ifdef EXECUTION_MESSAGES
          Serial.println(F("nextdelta < endPeriod, set lamp to final state"));
          #endif
          return;
        }
      } else {
        nextdelta = FlashingLampQueue[count].startPeriod +
          (( (pullTime - FlashingLampQueue[count].startTime) * 
          (FlashingLampQueue[count].endPeriod - FlashingLampQueue[count].startPeriod) ) / 
          FlashingLampQueue[count].duration);
        //Serial.print(F("Flashing lamp queue nextdelta is: "));
        //Serial.println(nextdelta, DEC);
        if (nextdelta > FlashingLampQueue[count].endPeriod)  {      // trigger for ending an event
          RPU_SetLampState(FlashingLampQueue[count].lampNumber, FlashingLampQueue[count].finishState);
          FlashingLampQueue[count].lampNumber = 0xFF;
          #ifdef EXECUTION_MESSAGES
          Serial.println(F("nextdelta < endPeriod so set lamp to final state"));
          #endif
          return;
        }
      }

      // Transition lamp and determine next transition time
      RPU_SetLampState(FlashingLampQueue[count].lampNumber,FlashingLampQueue[count].currentState);
      FlashingLampQueue[count].currentState = !FlashingLampQueue[count].currentState;
      //FlashingLampQueue[count].startTime = FlashingLampQueue[count].startTime + nextdelta;
      FlashingLampQueue[count].nextTime = FlashingLampQueue[count].nextTime + nextdelta;
      //Serial.println(F("Flashing lamp queue transitioned a lamp"));
    }
  }
  //FlashingLampQueueEmpty = true;
  //Serial.println(F("ProcessFlashingLampQueue found queue is empty"));
  return;
}

void ShowFlashingLampQueueEntries() {
  Serial.println(F("FlashingLampQueue Entries:      "));
  for (int count = 0; count < FLASHING_LAMP_QUEUE_SIZE; count++) {
    Serial.print(FlashingLampQueue[count].lampNumber, DEC);
    Serial.print(F("  "));
    Serial.print(FlashingLampQueue[count].currentState, DEC);
    Serial.print(F("  "));
    Serial.print(FlashingLampQueue[count].finishState, DEC);
    Serial.print(F("  "));
    Serial.print(FlashingLampQueue[count].startTime, DEC);
    Serial.print(F("  "));
    Serial.print(FlashingLampQueue[count].nextTime, DEC);
    Serial.print(F("  "));
    Serial.print(FlashingLampQueue[count].duration, DEC);
    Serial.print(F("  "));
    Serial.print(FlashingLampQueue[count].startPeriod, DEC);
    Serial.print(F("  "));
    Serial.print(FlashingLampQueue[count].endPeriod, DEC);
    Serial.println();
  }
}

void RoamingFinish() {
  OverrideScorePriority = 0;
  ShowPlayerScores(0xFF, false, false);                 // Reset all score displays
  RoamingStage = 0;                                     // Halts activity
  RoamingModeTime = 0;
  NumCapturedLetters = 0;
  ReverseRoam = false;
  GameMode[CurrentPlayer] = 1;                          // Return to normal play
  BallLighting();
}


unsigned long GoalsDisplayValue(byte currentgoals) {
  unsigned long Result = 0;
  for(int i=0; i<6; i++) {                     // Filter lower 6 goals
    Result = Result * 10;
    if ( Goals[CurrentPlayer] & (0b100000 >> i)) {
      Result +=1;
    }
    //Serial.print("Result is: ");
    //Serial.println(Result, DEC);
  }
  return Result;
}



void ShowShootAgainLamp() {

  if (!BallSaveUsed && BallSaveNumSeconds > 0 && (CurrentTime - BallFirstSwitchHitTime) < ((unsigned long)(BallSaveNumSeconds - 1) * 1000)) {
    unsigned long msRemaining = ((unsigned long)(BallSaveNumSeconds - 1) * 1000) - (CurrentTime - BallFirstSwitchHitTime);
    RPU_SetLampState(SAME_PLAYER, 1, 0, (msRemaining < 1000) ? 100 : 500);
  } else {
    RPU_SetLampState(SAME_PLAYER, SamePlayerShootsAgain);
  }
}

//  SpinnerComboFinish - Completes Spinner Combo mode

void SpinnerComboFinish() {
  SuperSpinnerAllowed[CurrentPlayer] = false;
  MarqueeDisabled = false;
  SuperSpinnerDuration = Spinner_Combo_Duration;
  SpinnerComboHalf = false;
  OverrideScorePriority = 0;              //  Set back to zero
  ShowPlayerScores(0xFF, false, false);   //  Reset all score displays
}

//
//  SetGoals Ver 3b - trigger 15-Ball mode, and Ball Chase, Scramble Ball
//
// Bit 1 - SuperBonus reached
// Bit 2 - NextBall met once
// Bit 3 - Pop Mode > x
// Bit 4 - Spinner Mode > x
// Bit 5 - Spinner Combo achieved x times
// Bit 6 - Scramble Ball
// Bit 7 - 3 Goals achieved
// Bit 8 - 5 Goals achieved

void SetGoals(byte goalnum) {   // Set goal flag and update display score

  Goals[CurrentPlayer] = (Goals[CurrentPlayer] | (0b1<<(goalnum - 1))); // Raise flag

  // Count how many goals are met and update display
  unsigned int countOnes = Goals[CurrentPlayer];
  byte numOnes = 0;
  for (int count = 0; count < 6; count++) {
    if ( (countOnes & 0b1) == 1 ) {
      numOnes++;
    }
    countOnes >>= 1;
    //Serial.print(F("numOnes :  "));
    //Serial.println(numOnes);
  }
  #ifdef EXECUTION_MESSAGES
  Serial.print(F("SetGoals - This many goals met: "));
  Serial.println(numOnes, DEC);
  #endif
  CurrentScores[CurrentPlayer] = (CurrentScores[CurrentPlayer]/10*10 + numOnes);

#if 0
  // Chase Ball
  if ( numOnes == 3 && !(Goals[CurrentPlayer] & (0b1<<6)) ) {       // Start Ball chase
    #ifdef EXECUTION_MESSAGES
    Serial.println(F("SetGoals - Chase Ball section, 3 goals met"));
    #endif
    Goals[CurrentPlayer] = (Goals[CurrentPlayer] | (0b1<<6));       // Raise 7th bit
    ClearRack();
    ClearSmallBalls();
    //NextBall = 0;             // Cannot cancel NextBall here, over-ridden by CaptureBall
    //NextBallTime = 0;

    // Triple knocker signifying 3 goals met
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 750, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 900, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 1250, true);
    //MarqueeDisabled = true;
    GameMode[CurrentPlayer] = 3;                                    // Trigger Mode 3
    ChaseBallStage = 1;                                             // First part of challenge
    ChaseBallTimeStart = CurrentTime;
    // Chase ball is not shifted by player number
    ChaseBall = ( 1 + CurrentTime%7 );                              // ChaseBall is 1-7
    ShowPlayerScores(0xFF, true, false);                            // Set displays back to normal
    #ifdef EXECUTION_MESSAGES
    Serial.print(F("SetGoals - ChaseBall is set to : "));
    Serial.println(ChaseBall, DEC);
    Serial.println();
    #endif
    //BallLighting();
  }
#endif

#if 1
  //
  // Roaming Mode 4 start
  //
  if ( (numOnes == 3 || numOnes == 4) && !(Goals[CurrentPlayer] & (0b1<<6)) ) { // Enable Roaming
  //if ( numOnes == 1 && !(Goals[CurrentPlayer] & (0b1<<6)) ) {       // Enable Roaming
    #ifdef EXECUTION_MESSAGES
    Serial.println(F("SetGoals - Starting Mode 4 Roaming"));
    #endif
    // Set goal bit in CaptureBall instead of here
    //Goals[CurrentPlayer] = (Goals[CurrentPlayer] | (0b1<<6));       // Raise 7th bit
    AlleyModeFinish();                                              // Cancel if running
    NextBallFinish();                                               // Cancel if running
    GameMode[CurrentPlayer] = 4;                                    // Switch to Roaming mode
    RoamingBalls = 0;                                               // Clear values when mode starts
    RoamingStage = 1;                                               // Start mode activity
    NumCapturedLetters=0;                                           // Reset counter
    MarqueeOffTime = 0;                                             // Cancel previous animation
    ClearSmallBalls();
    ClearRack();
    // Triple knocker signifying 3 goals met
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 750, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 900, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 1250, true);
    RoamingRotateTime = CurrentTime;
    RoamingModeTime = CurrentTime;
  }
#endif

  //
  // AlleyMode
  //
  // Start only when pop goal and spinner goal met in a single ball.  Triggers are reset each ball.
  if (goalnum == 3) AlleyModePopsTrigger = true;                    // Set pops trigger
  if (goalnum == 4) AlleyModeSpinnerTrigger = true;                 // Set spinner trigger
  if ( (AlleyModePopsTrigger) && (AlleyModeSpinnerTrigger) ) {      // Trigger mode if both are set
    AlleyModeStart();                                               // Function checks for GameMode == 1
  }

  // 15 Ball Mode
  if ( numOnes == 5 && !(Goals[CurrentPlayer] & (0b1<<7)) ) {       // Start 15 Ball mode
    #ifdef EXECUTION_MESSAGES
    Serial.println(F("SetGoals - 5 goals met"));
    #endif
    Goals[CurrentPlayer] = (Goals[CurrentPlayer] | (0b1<<7));       // Raise 8th bit
    // Shut off applicable modes that might be running
    if (goalnum == 5) {
      SpinnerComboFinish();
    }
    AlleyModeFinish();                                              // Cancel if running
    NextBallFinish();                                               // Cancel if running
    RoamingFinish();
    ClearRack();
    //FlashingArrows(DownwardV, 125, 9);
    //RPU_TurnOffAttractLamps;
    SamePlayerShootsAgain = true;
    RPU_SetLampState(SAME_PLAYER, 1, 0, 150);
    RPU_SetLampState(LA_SPSA_BACK, 1, 0, 175);
    PlaySoundEffect(SOUND_EFFECT_8_BALL_CAPTURE);
    // 5 knocker hits signifying 5 goals met.  Sound effect includes 1 knocker hit at 900 msec
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 1050, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 1350, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 1450, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 1550, true);
    //RPU_DisableSolenoidStack();
    RPU_SetDisableFlippers(true);
    MarqueeDisabled = true;
    FifteenBallQualified[CurrentPlayer] = true;
  }
  
}

//
// CaptureBall 
//   Ver 6 - Alley Mode added
//   Ver 5 - Mode 4 Roaming mode added
//   Ver 4 - Mode 2 and 3 implemented
//
boolean CaptureBall(byte ballswitchnum) {
  #ifdef EXECUTION_MESSAGES
    Serial.print(F("CaptureBall - GameMode Crt Plyr is: "));
    Serial.println(GameMode[CurrentPlayer], DEC);
  #endif
  if (GameMode[CurrentPlayer] == 1) {                  // Normal game play
    boolean returnVal = false;                         // Duplicate of that used in Roaming section
    byte nextSound = SOUND_EFFECT_NONE;                // Holds value of next sound effect
    // Call with 1 for balls 1-7 or 9-15
    if (((Balls[CurrentPlayer] & (0b1<<((ballswitchnum-1) + (CurrentPlayer%2 ? 8:0))))>>((ballswitchnum-1) + (CurrentPlayer%2 ? 8:0))) == false) { // If flag is 0
      Balls[CurrentPlayer] = (Balls[CurrentPlayer] | (0b1<<((ballswitchnum-1) + (CurrentPlayer%2 ? 8:0))));          // Raise flag
      RPU_SetLampState((LA_SMALL_1 + (ballswitchnum-1) + (CurrentPlayer%2 ? 8:0)), (MarqueeDisabled)?0:1, 0, 75);    // Set to rapid flashing upon capture
      MarqueeTiming();
      Silent_Hundred_Pts_Stack +=5;                    // Award ball capture points

      //
      // Next Ball
      //
 
      //  Check if next ball award should be given
      if (ballswitchnum == NextBall) {
        //Serial.println(F("NextBall awarded"));
        //Serial.println();
        Silent_Thousand_Pts_Stack +=5;                                        // Award additional NextBall score
        nextSound = SOUND_EFFECT_5K_CHIME;
        //PlaySoundEffect(SOUND_EFFECT_5K_CHIME, true);
        SetGoals(2);
        // If AlleyMode not running, remove flashing lamp per normal.  If AlleyMode is running it already cleared out
        // flashing in alleys 1-4 so only clear 5-7.
        if (AlleyModeActive == 0) {
          ClearFlashingLampQueueEntry(NextBall - 1 + (CurrentPlayer%2 ? 8:0));// Clear out any flashing NextBall
          #ifdef EXECUTION_MESSAGES
          Serial.println(F("NextBall collected, flashing lamp is cancelled"));
          #endif
        } else if ( (NextBall > 4) && (NextBall < 8) ) {
          ClearFlashingLampQueueEntry(NextBall - 1 + (CurrentPlayer%2 ? 8:0));// Only clear out targets 5-7 if AlleyMode running
          #ifdef EXECUTION_MESSAGES
          Serial.println(F("NextBall collected and AlleyMode active, only clear flashing lamp 5-7"));
          #endif
        }
        BallLighting();
      } else {                                                                // Was not NextBall
        nextSound = SOUND_EFFECT_BALL_CAPTURE;
        //ShowPlayerScores(0xFF, false, false);                                 // Reset all score displays
      }

      #ifdef EXECUTION_MESSAGES
      Serial.println(F("Award of NextBall was assessed *****"));
      Serial.print(F("NextBall value is:            "));
      Serial.println(NextBall, DEC);
      Serial.println();
      #endif
      
      //  Check if next 'Next Ball' is a valid one for reward
      byte NextBallCheck = (ballswitchnum + 1);
      if (NextBallCheck > 7) NextBallCheck = 1;
      // If next ball is uncollected, set NextBall, if collected set to 0.
      if (((Balls[CurrentPlayer] & (0b1<<((NextBallCheck-1) + (CurrentPlayer%2 ? 8:0))))>>((NextBallCheck-1) + (CurrentPlayer%2 ? 8:0))) == false) {
        //Serial.println(F("NextBall is available"));
        //Serial.println();
 
        // At this point a new NextBall is about to be set.  If a previous NextBall was active and not collected above we need 
        // to cancel the flashing before setting the new one flashing.  Need to check for AlleyMode being active.
        if (NextBall) {                                               // If there is an active NextBall
          if (AlleyModeActive == 0) {
            ClearFlashingLampQueueEntry(NextBall - 1 + (CurrentPlayer%2 ? 8:0));// Clear out any flashing NextBall
            #ifdef EXECUTION_MESSAGES
            Serial.println(F("Previous NextBall flashing lamp is cancelled"));
            #endif
          } else if ( (NextBall > 4) && (NextBall < 8) ) {
            ClearFlashingLampQueueEntry(NextBall - 1 + (CurrentPlayer%2 ? 8:0));// Only clear out targets 5-7 if AlleyMode running
            #ifdef EXECUTION_MESSAGES
            Serial.println(F("Previous NextBall flashing lamp is cancelled and AlleyMode active, only clear flashing lamp 5-7"));
            #endif
          }
          BallLighting();
        }
        // Change to new NextBall
        NextBall = NextBallCheck;
        NextBallTime = CurrentTime;
 
        // If here NextBall is valid, offset switchnum down by one for lamp number
        // Start lamp flashing, note end state is on, only allowed for lanes 5-7 when AlleyMode is running
        if (AlleyModeActive == 0) {                                   // Flash chosen NextBall lamp
          AddToFlashingLampQueue((NextBall - 1 + (CurrentPlayer%2 ? 8:0)), 1, CurrentTime, (NextBallDuration*1000), 300, 75);
          #ifdef EXECUTION_MESSAGES
          Serial.println(F("NextBall flashing sequence started"));
          #endif
        } else if ( (NextBall > 4) && (NextBall < 8) ) {              // If AlleyMode running only flash lamps 5-7
          AddToFlashingLampQueue((NextBall - 1 + (CurrentPlayer%2 ? 8:0)), 1, CurrentTime, (NextBallDuration*1000), 300, 75);
          #ifdef EXECUTION_MESSAGES
          Serial.println(F("AlleyMode is running, only NextBall flashing for 5-7 allowed"));
          #endif
        }
 
        if (GameMode[CurrentPlayer] != 1) {             // If SetGoals call above has triggered new mode, then disallow Next Ball.
          #ifdef EXECUTION_MESSAGES
          Serial.println(F("NextBall is cancelled by GameMode change."));
          #endif
          NextBallFinish();                             // Halt NextBall
        }
      } else {
        if (NextBall) {                                               // If there is an active NextBall
          if (AlleyModeActive == 0) {
            ClearFlashingLampQueueEntry(NextBall - 1 + (CurrentPlayer%2 ? 8:0));// Clear out any flashing NextBall
            #ifdef EXECUTION_MESSAGES
            Serial.println(F("Previous NextBall flashing lamp is cancelled"));
            #endif
          } else if ( (NextBall > 4) && (NextBall < 8) ) {
            ClearFlashingLampQueueEntry(NextBall - 1 + (CurrentPlayer%2 ? 8:0));// Only clear out targets 5-7 if AlleyMode running
            #ifdef EXECUTION_MESSAGES
            Serial.println(F("Previous NextBall flashing lamp is cancelled and AlleyMode active, only clear flashing lamp 5-7"));
            #endif
          }
          BallLighting();
        }
        #ifdef EXECUTION_MESSAGES
        Serial.println(F("NextBall is taken"));
        Serial.println();
        #endif
        NextBall = 0;
        NextBallTime = 0;
        ShowPlayerScores(0xFF, false, false);           //  Reset all score displays
        OverrideScorePriority = 0;                      //  Set back to zero
      }
      returnVal=true;                                   // Will return true if ball not already captured
    } else {
      returnVal=false;                                  // Will return false if previously captured
    }



    // Alley mode combo, award score if player hits correct lane

    if ( AlleyModeActive && (ballswitchnum > 0) && (ballswitchnum < 5) ) {      // Mode active and Alley lane is from 1-4
      if (AlleyModeSwitchCombinations[AlleyModeNumber - 1][ballswitchnum - 1]) {// If this switch is part of the current pattern (range 1-4)
        for (int count = 0; count < 4; count++) {
          ClearFlashingLampQueueEntry(count);                                   // Remove any lower flashing Alley lane lamps
          ClearFlashingLampQueueEntry(count + 8);                               // Remove any upper flashing Alley lane lamps
        }
        BallLighting();                                                         // Reset lamps
        //Thousand_Pts_Stack += 10*(AlleyModeNumber);                             // 10k, 20k, .....
        Silent_Thousand_Pts_Stack += 10*(AlleyModeNumber);                      // 10k, 20k, .....
        switch (AlleyModeNumber) {
          case 1:
            nextSound = SOUND_EFFECT_ALLEY_I;
            //PlaySoundEffect(SOUND_EFFECT_ALLEY_I, true);
            break;
          case 2:
            nextSound = SOUND_EFFECT_ALLEY_II;
            //PlaySoundEffect(SOUND_EFFECT_ALLEY_II, true);
            break;
          case 3:
            nextSound = SOUND_EFFECT_ALLEY_III;
            //PlaySoundEffect(SOUND_EFFECT_ALLEY_III, true);
            break;
          case 4:
            nextSound = SOUND_EFFECT_ALLEY_IV;
            //PlaySoundEffect(SOUND_EFFECT_ALLEY_IV, true);
            break;
        }
        Alley_Mode_Start_Time = CurrentTime;                                    // Reset the clock
        AlleyModeActive = 1;                                                    // Reset to 1st stage of mode
        AlleyModeNumber += 1;                                                   // Increment number
        if (AlleyModeNumber > 4) {                                              // Player has completed the whole sequence
          AlleyModeActive = 3;                                                  // Sequence end stage
          AlleyModeNumber = 0;                                                  // Disable further rewards
        }
        returnVal=true;                                                         // Prevent default sounds when AlleyMode level achieved
      }
    }

    if (nextSound) PlaySoundEffect(nextSound, true);                            // Only play a sound if non-zero
    return returnVal;
  } else if (GameMode[CurrentPlayer] == 2){         // GameMode == 2, 15 Ball mode
  //
  // 15 Ball code - Game Mode 2
  //
  if ( (!(Balls[CurrentPlayer] & (0b1<<(ballswitchnum-1)))) && (!(Balls[CurrentPlayer] & (0b1<<((ballswitchnum-1) + (8))))) ) {
    #ifdef EXECUTION_MESSAGES
    Serial.println(F("15 ball - Balls are 0 1"));
    #endif
    Balls[CurrentPlayer] = (Balls[CurrentPlayer] | (0b1<<(ballswitchnum-1)));           // Raise flag
    RPU_SetLampState((LA_SMALL_1 + (ballswitchnum-1) + 0), (MarqueeDisabled)?0:1, 0, 75);    // Set to rapid flashing upon capture
    MarqueeTiming();
    Silent_Thousand_Pts_Stack +=5;                  // Award ball capture points
    PlaySoundEffect(SOUND_EFFECT_CARMEN_UP, true);
  } else if ( ( (Balls[CurrentPlayer] & (0b1<<(ballswitchnum-1)))) && (!(Balls[CurrentPlayer] & (0b1<<((ballswitchnum-1) + (8))))) ) {
    #ifdef EXECUTION_MESSAGES
    Serial.println(F("15 ball - Balls are 1 1"));
    #endif
    Balls[CurrentPlayer] = (Balls[CurrentPlayer] | (0b1<<((ballswitchnum-1) + 8)));     // Raise flag
    RPU_SetLampState((LA_SMALL_1 + (ballswitchnum-1) + 8), (MarqueeDisabled)?0:1, 0, 75);    // Set to rapid flashing upon capture
    MarqueeTiming();
    Silent_Thousand_Pts_Stack +=5;                  // Award ball capture points
    PlaySoundEffect(SOUND_EFFECT_CARMEN_DOWN, true);
  } else if ( ( (Balls[CurrentPlayer] & (0b1<<(ballswitchnum-1)))) && ( (Balls[CurrentPlayer] & (0b1<<((ballswitchnum-1) + (8))))) ) {
    #ifdef EXECUTION_MESSAGES
    Serial.println(F("15 ball - Balls are 0 1"));
    #endif
    Balls[CurrentPlayer] = Balls[CurrentPlayer] & (~(0b1<<(ballswitchnum-1)));             // Clear low ball bit
    RPU_SetLampState((LA_SMALL_1 + (ballswitchnum-1) + 0), 1, 0, (MarqueeDisabled)?0:75); // Turn on
    MarqueeTiming();
    Silent_Thousand_Pts_Stack -=10;                  // Award ball capture points
    PlaySoundEffect(SOUND_EFFECT_BALL_LOSS, true);
  } else { /*( ( (Balls[CurrentPlayer] & (0b1<<(ballswitchnum-1)))) && (!(Balls[CurrentPlayer] & (0b1<<((ballswitchnum-1) + (8))))) )*/
    #ifdef EXECUTION_MESSAGES
    Serial.println(F("15 ball - Balls are 0 0"));
    #endif
    Balls[CurrentPlayer] = Balls[CurrentPlayer] & (~(0b1<<(ballswitchnum-1) + 8));         // Clear high ball bit
    RPU_SetLampState((LA_SMALL_1 + (ballswitchnum-1) + 8), 1, 0, (MarqueeDisabled)?0:75); // Turn on
    MarqueeTiming();
    Silent_Thousand_Pts_Stack -=10;                  // Award ball capture points
    PlaySoundEffect(SOUND_EFFECT_BALL_LOSS, true);
  }
  return true;    // 15-Ball mode never returns false, ball change always happening

  } else if (GameMode[CurrentPlayer] == 3) {

  // ChaseBall code
    if (ChaseBall != ballswitchnum) {
      #ifdef EXECUTION_MESSAGES
      Serial.println(F("Invalid ChaseBall"));
      #endif
      return false;                                 // Return false defaults to 500 pts
    } else if (ChaseBallStage == 1) {               // Chase Ball mode is in 1st stage and is true
      #ifdef EXECUTION_MESSAGES
      Serial.print(F("CaptureBall - ChaseBall stage is: "));
      Serial.println(ChaseBallStage, DEC);
      Serial.print(F("CaptureBall - Valid ChaseBall was: "));
      Serial.println(ChaseBall, DEC);
      #endif
      ChaseBallTimeStart = CurrentTime;             // Reset clock
      Silent_Thousand_Pts_Stack +=5;                // Award score
      PlaySoundEffect(SOUND_EFFECT_5K_CHIME);
      RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 600, true);
      ChaseBallStage = 2;                           // Now chasing a 2nd Ball
      ClearSmallBalls();                            // Cancel 1st ChaseBall lamp
      #ifdef EXECUTION_MESSAGES
      Serial.print(F("CaptureBall - ChaseBall stage (from stage 1) is: "));
      Serial.println(ChaseBallStage, DEC);
      #endif
      // Update which ball is next target to chase, min increment is 1, max 6
      ChaseBall += (1 + CurrentTime%6);
      if ( ChaseBall > 7) {
        ChaseBall -= 7;
      }
      #ifdef EXECUTION_MESSAGES
      Serial.print(F("CaptureBall - Next ChaseBall is: "));
      Serial.println(ChaseBall, DEC);
      #endif
      return true;                                  // Ball captured here
    } else {                                        // ChaseBall Stage 2
      Thousand_Pts_Stack +=20;
      GameMode[CurrentPlayer] = 1;                  // Exit back to regular play
      BallLighting();
      BankShotLighting();
      ChaseBallTimeStart = 0;                       // Zero clock to halt mode
      ChaseBall = 0;                                // Reset to 0
      ShowPlayerScores(0xFF, false, false);         //  Reset all score displays
      OverrideScorePriority = 0;                    //  Set back to zero
      RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 750, true);
      RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 1100, true);
      RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 1250, true);
      #ifdef EXECUTION_MESSAGES
      Serial.print(F("CaptureBall - ChaseBall stage (from stage 2) is: "));
      Serial.println(ChaseBallStage, DEC);
      Serial.print(F("CaptureBall - 2nd ChaseBall was: "));
      Serial.println(ChaseBall, DEC);
      #endif
      return true;                                  // Ball captured here
    }

  } else if (GameMode[CurrentPlayer] == 4) {                              // Mode 4 - Roaming
    boolean returnVal = false;
    if (RoamingStage < 3) {                                               // Can only capture lamps in 1 or 2
      // For switchnum from 1-7 balls only
      if (ballswitchnum < 8) {
        // Check lower ball number first (1-7)
        if ( (RoamingBalls & (0b1<<(ballswitchnum - 1))) == false ) {     // If flag is 0
          RoamingBalls = (RoamingBalls | (0b1<<(ballswitchnum - 1)) );    // then raise flag
          NumCapturedLetters +=1;
          #ifdef EXECUTION_MESSAGES
          Serial.println(F("Roaming captured ball (1-7)"));
          #endif
          returnVal = true;                                               // Letter was captured
        // Check upper ball number (9-15), check 8 spots ahead (1 becomes 9, etc)
        } else if ( (RoamingBalls & (0b1<<(ballswitchnum + 7))) == false ) { // If flag is 0
          RoamingBalls = (RoamingBalls | (0b1<<(ballswitchnum +7)) );     // then raise flag
          NumCapturedLetters +=1;
          #ifdef EXECUTION_MESSAGES
          Serial.println(F("Roaming captured ball (9-15)"));
          #endif
          returnVal = true;                                               // Letter was captured
        }
      }
      if (ballswitchnum == 8) {                                           // Ball 8 can also capture
        if ( (RoamingBalls & (0b1<<(ballswitchnum - 1))) == false ) {     // If flag is 0
          RoamingBalls = (RoamingBalls | (0b1<<(ballswitchnum - 1)) );    // then raise flag
          NumCapturedLetters +=1;
          #ifdef EXECUTION_MESSAGES
          Serial.println(F("Roaming captured ball 8"));
          #endif
          returnVal = true;                                               // Letter was captured
        }
      }
      if (returnVal) {                                                    // If we captured a letter
        if (NumCapturedLetters == 1) {
          RoamingStage = 2;                    // Move to next stage
          // If we got here we scored at least one ball, set goal bit
          Goals[CurrentPlayer] = (Goals[CurrentPlayer] | (0b1<<6));       // Raise 7th bit
        }
        if (NumCapturedLetters == 15) {
          RoamingStage = 4;                                               // Completion stage
          WrapUpSoundPlayed = false;                                      // Allow sound effect to play
          ShowPlayerScores(0xFF, false, false);                           // Reset all score displays
        }
        RoamingModeTime = CurrentTime;
        RoamingRotateTime = CurrentTime;
        // Comment line below to prevent letters from reversing
        //ReverseRoam = !ReverseRoam;                                       // Next animation reverses
        Silent_Thousand_Pts_Stack += RoamingScores[NumCapturedLetters-1];
        // Roaming sound effects
        if (NumCapturedLetters < 6) {                                     // Initial sound effect 
          PlaySoundEffect(SOUND_EFFECT_ROAMING_CAPTURE_I, true);
        } else if (NumCapturedLetters < 11) {                             // Escalated sound
          PlaySoundEffect(SOUND_EFFECT_ROAMING_CAPTURE_II, true);
        } else {                                                          // Final sound
          PlaySoundEffect(SOUND_EFFECT_8_BALL_CAPTURE, true);
        }
        BallLighting();                                                   // Reset all lamps
      }
    } else {                                                              // RoamingStage > 2, Default scoring only for stages 3,4
      returnVal= false;                                                   // Don't actually have to set this, already set up at start of Mode 4 section
    }
    #ifdef EXECUTION_MESSAGES
    Serial.print(F("CaptureBall Mode 4 returnVal is: "));
    Serial.println(returnVal, BIN);
    Serial.print(F("NumCapturedLetters:              "));
    Serial.println(NumCapturedLetters, DEC);
    #endif
    return returnVal;                                                     // If false calling code will provide scoring and sound, true and calling code skips sounds and scoring
  }   // End of Mode 4

} // - End of CaptureBall


void ClearSmallBalls() {
  for(int i=0; i<15; i++) {                     // Turn out all small ball lamps
    //RPU_SetLampState((LA_BIG_1 + i), 0);  
    RPU_SetLampState((LA_SMALL_1 + i), 0);
  }
}

void ClearRack() {
  for(int i=0; i<15; i++) {                     // Turn out all ball lamps
    RPU_SetLampState((LA_BIG_1 + i), 0);  
    //RPU_SetLampState((LA_SMALL_1 + i), 0);
  }
}

void FlashingArrows(int lamparray[], int rate, int numlamps) {
  for (int i = 0; i < numlamps; i++) {
    RPU_SetLampState((lamparray[i]), 1, 0, rate);
    //RPU_SetLampState((RackRightArrow[i]), 1, 0, rate);  
  }
}

#if 1

//
// BonusMultiplier - call to reset to 1x, or 2,3,5x
// Ver 2 - Extended range
//
void BonusMultiplier(byte mult){
  switch (mult){
    case 2: // 2X
      RPU_SetLampState(LA_2X, 1);
      RPU_SetLampState(LA_3X, 0);
      RPU_SetLampState(LA_5X, 0);
      BonusMult = 2;
      break;
    case 3: // 3X
      RPU_SetLampState(LA_2X, 0);
      RPU_SetLampState(LA_3X, 1);
      RPU_SetLampState(LA_5X, 0);
      BonusMult = 3;
      break;
    case 5: // 5X
      RPU_SetLampState(LA_2X, 0);
      RPU_SetLampState(LA_3X, 0);
      RPU_SetLampState(LA_5X, 1);
      BonusMult = 5;
      break;
    case 7: // 7X
      RPU_SetLampState(LA_2X, 1, 0, 1000);
      RPU_SetLampState(LA_3X, 0);
      RPU_SetLampState(LA_5X, 1, 0, 1000);
      BonusMult = 7;
      break;
    case 8: // 8X
      RPU_SetLampState(LA_2X, 0);
      RPU_SetLampState(LA_3X, 1, 0, 750);
      RPU_SetLampState(LA_5X, 1, 0, 750);
      BonusMult = 8;
      break;
    case 10: // 10X
      RPU_SetLampState(LA_2X, 1, 0, 400);
      RPU_SetLampState(LA_3X, 1, 0, 500);
      RPU_SetLampState(LA_5X, 1, 0, 600);
      BonusMult = 10;
      break;
    default: // Set to default 1X
      RPU_SetLampState(LA_2X, 0);
      RPU_SetLampState(LA_3X, 0);
      RPU_SetLampState(LA_5X, 0);
      BonusMult = 1;
      break;
    }
  #ifdef EXECUTION_MESSAGES
  Serial.println(F("BonusMultiplier()"));
  #endif
}

#else

//
// BonusMultiplier - call to reset to 1x, or 2,3,5x
//

void BonusMultiplier(byte mult){
  switch (mult){
    case 2: // 2X
      RPU_SetLampState(LA_2X, 1);
      RPU_SetLampState(LA_3X, 0);
      RPU_SetLampState(LA_5X, 0);
      BonusMult = 2;
      break;
    case 3: // 3X
      RPU_SetLampState(LA_2X, 0);
      RPU_SetLampState(LA_3X, 1);
      RPU_SetLampState(LA_5X, 0);
      BonusMult = 3;
      break;
    case 5: // 5X
      RPU_SetLampState(LA_2X, 0);
      RPU_SetLampState(LA_3X, 0);
      RPU_SetLampState(LA_5X, 1);
      BonusMult = 5;
      break;
    default: // Set to default 1X
      RPU_SetLampState(LA_2X, 0);
      RPU_SetLampState(LA_3X, 0);
      RPU_SetLampState(LA_5X, 0);
      BonusMult = 1;
      break;
    }
  #ifdef EXECUTION_MESSAGES
  Serial.println(F("BonusMultiplier()"));
  #endif
}

#endif

//
// Bank Shot Lighting - handle post animation lighting
// Ver 2 - Extended BankShot range

void BankShotLighting(){
  #ifdef EXECUTION_MESSAGES
  Serial.println(F("BankShotLighting()"));
  #endif
  RPU_SetLampState(LA_BANK_SHOT_300, ((BankShotProgress - 0) ? 0:1));
  RPU_SetLampState(LA_BANK_SHOT_600, ((BankShotProgress - 1) ? 0:1));
  RPU_SetLampState(LA_BANK_SHOT_900, ((BankShotProgress - 2) ? 0:1));
  RPU_SetLampState(LA_BANK_SHOT_1200, ((BankShotProgress - 3) ? 0:1));
  RPU_SetLampState(LA_BANK_SHOT_1500, ((BankShotProgress - 4) ? 0:1));
  RPU_SetLampState(LA_BANK_SHOT_5000, ( ((BankShotProgress - 5) && (BankShotProgress - 6)) ? 0:1) );
  if (BankShotProgress > 6) {
    switch (BankShotProgress) {
      case 7:
      case 10:
      case 13:
        RPU_SetLampState(LA_BANK_SHOT_5000, 1, 0, 600);
        break;
      case 8:
      case 11:
      case 14:
        RPU_SetLampState(LA_BANK_SHOT_5000, 1, 0, 300);
        break;
      case 15:
        RPU_SetLampState(LA_BANK_SHOT_5000, 1, 0, 100);
        break;
      default:
        RPU_SetLampState(LA_BANK_SHOT_5000, 1);
        break;
    }
  }
}

//
// Bank Shot Scoring - handle scoring and sound only
// Ver 2 - Silent mode
// Ver 3 - Extended range

void BankShotScoring(byte sound){
  //Serial.print(F("BankShotScoring\n"));
  BankShotOffTime = CurrentTime;
  BankShotProgress += 1;
  if (BankShotProgress >= 16) BankShotProgress=16;
  switch (BankShotProgress) {
    case 1: // Just scoring
      if (sound) {
        Hundred_Pts_Stack +=3;
      } else {
        Silent_Hundred_Pts_Stack +=3;
      }
      break;
    case 2: // Scoring plus 2X
      if (sound) {
        Hundred_Pts_Stack +=6;
      } else {
        Silent_Hundred_Pts_Stack +=6;
      }
      BonusMultiplier(2); // Set to 2X 
      break;
    case 3: // Scoring plus 3X
      if (sound) {
        Hundred_Pts_Stack +=9;
      } else {
        Silent_Hundred_Pts_Stack +=9;
      }
      BonusMultiplier(3); // Set to 3X 
      break;
    case 4: // Scoring plus 5X
      if (sound) {
        Hundred_Pts_Stack +=12;
      } else {
        Silent_Hundred_Pts_Stack +=12;
      }
      BonusMultiplier(5); // Set to 5X 
      break;
    case 5:
      Silent_Hundred_Pts_Stack +=15;
      // Light and set SPSA
      SamePlayerShootsAgain = true;
      RPU_SetLampState(SAME_PLAYER, 1);
      RPU_SetLampState(LA_SPSA_BACK, 1);        
      if (sound) PlaySoundEffect(SOUND_EFFECT_EXTRA_BALL, true);
      break;
    case 6:
      Silent_Thousand_Pts_Stack +=5;
      if (sound) PlaySoundEffect(SOUND_EFFECT_5K_CHIME);
      // Nothing extra for this light, just the score
      break;
    case 7:
    case 8:
    case 10:
    case 11:
    case 13:
    case 14:
    case 15:
      Silent_Thousand_Pts_Stack +=5;
      if (sound) PlaySoundEffect(SOUND_EFFECT_5K_CHIME);
      break;
    case 9:
      Silent_Thousand_Pts_Stack +=5;
      if (sound) PlaySoundEffect(SOUND_EFFECT_MACHINE_START);
      BonusMultiplier(7); // Set to 7X       
      break;
    case 12:
      Silent_Thousand_Pts_Stack +=5;
      if (sound) PlaySoundEffect(SOUND_EFFECT_MACHINE_START);
      BonusMultiplier(8); // Set to 8X       
      break;
    case 16:
      if (BonusMultTenX) {                //  If already 10X 
        Silent_Thousand_Pts_Stack +=5;
        if (sound) PlaySoundEffect(SOUND_EFFECT_5K_CHIME);
        //BonusMultiplier(10); // Set to 10X       
      } else {                            //  One time extra when achieving 10X
        Silent_Thousand_Pts_Stack +=10;
        if (sound) PlaySoundEffect(SOUND_EFFECT_8_BALL_CAPTURE);
        BonusMultiplier(10); // Set to 10X
        BonusMultTenX = true;
      }
      break;
  }
}


//
// Arrow Toggle
//

void ArrowToggle() {
  if (RightArrow == false) {
    RPU_SetLampState(LA_RIGHT_ARROW, 1);
    RPU_SetLampState(LA_LEFT_ARROW, 0);
    RightArrow = true;
  } else {
    RPU_SetLampState(LA_RIGHT_ARROW, 0);
    RPU_SetLampState(LA_LEFT_ARROW, 1);
    RightArrow = false;    
  }
}


// Borrowed from Mata Hari code
//
// PlaySoundEffect
//
/*
#define SOUND_EFFECT_NONE                 0
#define SOUND_EFFECT_ADD_PLAYER           1
#define SOUND_EFFECT_BALL_OVER            2
#define SOUND_EFFECT_GAME_OVER            3
#define SOUND_EFFECT_MACHINE_START        4
#define SOUND_EFFECT_ADD_CREDIT           5
#define SOUND_EFFECT_PLAYER_UP            6
#define SOUND_EFFECT_GAME_START           7
#define SOUND_EFFECT_EXTRA_BALL           8
#define SOUND_EFFECT_5K_CHIME             9
#define SOUND_EFFECT_BALL_CAPTURE        10
#define SOUND_EFFECT_POP_MODE            11
#define SOUND_EFFECT_POP_100             12
#define SOUND_EFFECT_POP_1000            13
#define SOUND_EFFECT_POP_1000b           14
#define SOUND_EFFECT_POP_1000c           15
#define SOUND_EFFECT_TILT_WARNING        16
#define SOUND_EFFECT_TILTED              17
#define SOUND_EFFECT_10_PTS              18
#define SOUND_EFFECT_100_PTS             19
#define SOUND_EFFECT_1000_PTS            20
#define SOUND_EFFECT_EXTRA               21
#define SOUND_EFFECT_SPINNER_COMBO       22
#define SOUND_EFFECT_KICKER_OUTLANE      23
#define SOUND_EFFECT_8_BALL_CAPTURE      24
#define SOUND_EFFECT_BALL_LOSS           25
#define SOUND_EFFECT_ROAMING_CAPTURE_I   26
#define SOUND_EFFECT_ROAMING_CAPTURE_II  27
#define SOUND_EFFECT_ROAMING_CAPTURE_III 28
#define SOUND_EFFECT_SCRAMBLE_BALL       29
#define SOUND_EFFECT_CARMEN_UP           30
#define SOUND_EFFECT_CARMEN_DOWN         31
#define SOUND_EFFECT_ROAMING_END         32
#define SOUND_EFFECT_ROAMING_COMPLETE    33
#define SOUND_EFFECT_ALLEY_I             34
#define SOUND_EFFECT_ALLEY_II            35
#define SOUND_EFFECT_ALLEY_III           36
#define SOUND_EFFECT_ALLEY_IV            37
*/

// Array of sound effect durations
// 33 is extra long to create gap in sounds
int SoundEffectDuration[38] = 
{0, 400, 1275, 1458, 2513, 575, 900, 2579, 800, 800,
550, 1775, 0, 5, 5, 5, 200, 2000, 30, 30,
30, 30, 1305, 400, 1100, 350, 868, 1381, 1890, 1050,
900, 900, 1750, 2200, 600, 850, 1100, 1500};

unsigned long NextSoundEffectTime = 0;  

void PlaySoundEffect(byte soundEffectNum, boolean priority = false) {
//void PlaySoundEffect(byte soundEffectNum) {

unsigned long TimeStart;

  if (CurrentTime > NextSoundEffectTime) { // No sound effect running
    NextSoundEffectTime = CurrentTime + SoundEffectDuration[soundEffectNum];
    TimeStart = CurrentTime;
/*
    Serial.println(F("No sound was playing"));
    Serial.print(F("CurrentTime is:            "));
    Serial.println(CurrentTime, DEC);
    Serial.print(F("NextSoundEffectTime        "));
    Serial.println(NextSoundEffectTime, DEC);
    Serial.println((CurrentTime - NextSoundEffectTime), DEC);
    Serial.println();
*/
  } else {                                 // A sound effect is in process if here
      TimeStart = NextSoundEffectTime;
      #ifdef EXECUTION_MESSAGES
      Serial.println(F("Sound already playing"));
      #endif
      // Allow priority sounds to queue regardless
      if ( ((NextSoundEffectTime - CurrentTime) > 250) && (!priority) ) {
        #ifdef EXECUTION_MESSAGES
        Serial.println(F("Break out of playing sound"));
        #endif
        return;
      }
      #ifdef EXECUTION_MESSAGES
      if (priority) {
        Serial.println(F("Priority sound being queued"));
      }
      #endif
      NextSoundEffectTime = NextSoundEffectTime + 50 + SoundEffectDuration[soundEffectNum];
      
/*      Serial.print(F("soundEffectNum: "));
      Serial.println(soundEffectNum, DEC);
      Serial.print(F("SoundEffectDuration: "));
      Serial.println(SoundEffectDuration[soundEffectNum], DEC);
      Serial.print(F("NextSoundEffectTime: "));
      Serial.println(NextSoundEffectTime, DEC);
      Serial.print(F("CurentTime: "));
      Serial.println(CurrentTime, DEC);
      Serial.println();*/
    
  }

//  unsigned long count;  // not used?

  switch (soundEffectNum) {
    case SOUND_EFFECT_NONE:
      break;
    case SOUND_EFFECT_ADD_PLAYER:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart +200, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart +400, true);
      break;
    case SOUND_EFFECT_BALL_OVER:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart+166, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart+250, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart+500, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart+750, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart+1000, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart+1166, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart+1250, true);
      break;
    case SOUND_EFFECT_GAME_OVER:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart+0, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart+104, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart+217, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart+404, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart+517, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart+629, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart+817, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart+925, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart+1042, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart+1229, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart+1342, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart+1458, true);
      break;
    case SOUND_EFFECT_MACHINE_START:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart+217, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart+308, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart+412, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart+579, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart+679, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart+779, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart+946, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart+1046, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart+1150, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart+1313, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart+1417, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart+1513, true);
      break;
    case SOUND_EFFECT_ADD_CREDIT:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 111, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 209, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 310, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 477, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 575, true);
      break;
    case SOUND_EFFECT_PLAYER_UP:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 1, TimeStart+500, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart+600, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart+900, true);
      break;
    case SOUND_EFFECT_GAME_START:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 4, TimeStart + 500);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 679);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 846);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1013);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1208);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1313);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1408);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 1546);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 1746);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 1842);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 1942);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 2079);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 2246);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 2413);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 2579);
      break;
    case SOUND_EFFECT_EXTRA_BALL:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 100, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 200, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 300, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 500, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 600, true);
      break;
    case SOUND_EFFECT_5K_CHIME:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart +  0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart +  50);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart +   150);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart +  200);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart +   300);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart +  350);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart +   450);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart +  500);
      break;
    case SOUND_EFFECT_BALL_CAPTURE:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 100);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 200);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 300);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 400);
      break;
    case SOUND_EFFECT_POP_MODE:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 100);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 200);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 300);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 425);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 525);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 625);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 725);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 850);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 950);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 1050);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 1150);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1275);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1375);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1475);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1575);
      break;
    case SOUND_EFFECT_POP_100:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 0);
      break;
    case SOUND_EFFECT_POP_1000:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 10);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 20);
      break;
    case SOUND_EFFECT_POP_1000b:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 10);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 20);
      break;
    case SOUND_EFFECT_POP_1000c:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 10);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 20);
      break;
    case SOUND_EFFECT_TILT_WARNING:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 200);
      break;
    case SOUND_EFFECT_TILTED:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 0, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 400, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 800, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 1200, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1600, true);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 2000, true);
      break;
    case SOUND_EFFECT_10_PTS:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 0);
      break;
    case SOUND_EFFECT_100_PTS:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 0);
      break;
    case SOUND_EFFECT_1000_PTS:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 0);
      break;
    case SOUND_EFFECT_EXTRA:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 0);
      break;
    case SOUND_EFFECT_SPINNER_COMBO :
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 75);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 135);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 435);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 510);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 570);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 870);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 945);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 1005);
      RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, TimeStart + 1105);
      break;
    case SOUND_EFFECT_KICKER_OUTLANE:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 100);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 200);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 300);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 400);
      break;
    case SOUND_EFFECT_8_BALL_CAPTURE:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 100);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 200);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 300);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 400);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 500);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 600);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 700);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 800);
      RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 900);
      break;
    case SOUND_EFFECT_BALL_LOSS:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 200);
      break;
    case SOUND_EFFECT_ROAMING_CAPTURE_I:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 100);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 200);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 490);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 668);
      break;
    case SOUND_EFFECT_ROAMING_CAPTURE_II:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 100);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 200);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 490);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 668);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 1000);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1181);
      break;
    case SOUND_EFFECT_ROAMING_CAPTURE_III:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 100);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 200);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 490);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 668);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 1000);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1181);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 1472);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 1690);
      break;
    case SOUND_EFFECT_SCRAMBLE_BALL:
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 50);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 200);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 250);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 400);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 450);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 600);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 650);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 800);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 850);
      break;
    case SOUND_EFFECT_CARMEN_UP:                    // duration 525
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 90);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 288);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 523);
      break;
    case SOUND_EFFECT_CARMEN_DOWN:                  // duration 525
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart + 90);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart + 288);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 523);
      break;
    case SOUND_EFFECT_ROAMING_END:                  // duration 1250
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart +  100);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart +   200);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart +    300);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart +  450);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart +   550);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart +    650);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart +  800);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart +   900);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart +    1000);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 1250);
      break;
    case SOUND_EFFECT_ROAMING_COMPLETE:             // duration 1650
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 100);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart +    200);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart +   300);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 450);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart +    550);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart +   650);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 800);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3, TimeStart +    900);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3, TimeStart +  1000);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1250);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1450);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1550);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3, TimeStart + 1650);
      break;
    case SOUND_EFFECT_ALLEY_I:                      // duration 400
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart +   0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3,   TimeStart + 150);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3,    TimeStart + 250);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3,  TimeStart + 400);
      break;
    case SOUND_EFFECT_ALLEY_II:                     // duration 650
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart +   0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3,   TimeStart + 150);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3,    TimeStart + 250);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3,  TimeStart + 400);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3,    TimeStart + 550);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3,  TimeStart + 650);
      break;
    case SOUND_EFFECT_ALLEY_III:                    // duration 900
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart +   0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3,   TimeStart + 150);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3,    TimeStart + 250);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3,  TimeStart + 400);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3,    TimeStart + 550);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3,  TimeStart + 650);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3,   TimeStart + 800);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 900);
      break;
    case SOUND_EFFECT_ALLEY_IV:                     // duration 1200
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart +   0);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3,   TimeStart + 150);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3,    TimeStart + 250);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3,  TimeStart + 400);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_10, 3,    TimeStart + 550);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_1000, 3,  TimeStart + 650);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_100, 3,   TimeStart + 800);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 900);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 1000);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 1100);
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 3, TimeStart + 1200);
      break;
  }
}

//
// BallLighting - Reset lights - Ver 4
//  - Add Mode 4 Roaming
//  - Reset lights - using Ball[] variable
//  - 15 ball capable version
//
void BallLighting() {

// 'if' selects balls 1-7 for players 1,3 and 9-15 for player 2,4.  Bit mask is shifted 8 spots or not depending 
// on player number.  Deals with all 15 lamps.

  for(int i=0; i<15; i++) {                     // Turn out all ball lamps
    RPU_SetLampState((LA_BIG_1 + i), 0);  
    RPU_SetLampState((LA_SMALL_1 + i), 0);
  }

  if (GameMode[CurrentPlayer] == 1) {  // Mode 1
    for(int i=0; i<7; i++) {                    // Cycle through 7 balls to set the lamps
      if (((Balls[CurrentPlayer] & (0b1<<(i + (CurrentPlayer%2 ? 8:0))))>>(i + (CurrentPlayer%2 ? 8:0))) == true) {
        RPU_SetLampState((LA_BIG_1 + i + (CurrentPlayer%2 ? 8:0)), 1);       // if true turn large center lamp on
      } else {
        RPU_SetLampState((LA_SMALL_1 + i + (CurrentPlayer%2 ? 8:0)), 1);     // if large lamp is off, small lamp is on
      }
    }
    // Turn on Ball 8 as special case, check first if Ball 8 is captured, if not check if 7 balls captured.
    if ((Balls[CurrentPlayer] & (0b10000000)) == 128) {
      //Serial.println(F("Ball Lighting - Ball 8 is true, turn it on"));
      RPU_SetLampState(LA_BIG_8, 1);
    } else {
      if (((Balls[CurrentPlayer] & (0b01111111<<(0 + (CurrentPlayer%2 ? 8 : 0))))>>(CurrentPlayer%2 ? 8 : 0)) == 127) { // 7 balls captured, ready to capture ball 8
        RPU_SetLampState(LA_SMALL_8, 1);
      }
    }
  // end of Mode 1
  
  } else if (GameMode[CurrentPlayer] == 2) {
    for(int i=0; i<15; i++) {                      // Cycle through 15 balls to set the lamps
      if (i == 7) {
        i = 8;                                     // Skip 8 ball
      }
      if ( ((Balls[CurrentPlayer] & (0b1<<i)) >> i ) == true) {
        RPU_SetLampState((LA_BIG_1 + i), 1);      // if true turn large center lamp on
      } else {
        RPU_SetLampState((LA_SMALL_1 + i), 1);    // if large lamp is off, small lamp is on
      }
    }
    // Turn on Ball 8 as special case, check first if Ball 8 is captured, if not check if 7 balls captured.
    if ((Balls[CurrentPlayer] & (0b10000000)) == 128) {
      //Serial.println(F("Ball Lighting - Ball 8 is true, turn it on"));
      RPU_SetLampState(LA_BIG_8, 1);
    } else {
      if ( (Balls[CurrentPlayer] & (0b0111111101111111) ) == 0x7F7F) { // 7 balls captured, ready to capture ball 8. (32639)
        RPU_SetLampState(LA_SMALL_8, 1);
      }
    }
    // end of Mode 2

  } else if (GameMode[CurrentPlayer] == 4) { // if Roaming Mode 4
    //
    // Mode 4 - Roaming
    //

    // 15 big and small lamps are all turned off at top of function

    for(int i=0; i<15; i++) {
      if (RoamingBalls & (0b1 << i)) {
        RPU_SetLampState((LA_BIG_1 + i), 1);              // if true turn large rack lamp on
        //Serial.print(F("Turned on big letter: "));
        //Serial.println((LA_BIG_1 + i), DEC);
      } else {
        RPU_SetLampState((LA_SMALL_1 + i), 1);            // if large lamp is off, small lamp is on
        //Serial.print(F("Turned on small letter: "));
        //Serial.println((LA_SMALL_1 + i), DEC);
      }
    }
    //Serial.println(F("End of Mode 4 BallLighting"));
  }

  #ifdef EXECUTION_MESSAGES
  Serial.print(F("BallLighting - Mode: "));
  Serial.println(GameMode[CurrentPlayer], DEC);
  Serial.println();
  #endif

} // End of BallLighting


void MarqueeTiming() {
  if (MarqueeOffTime == 0) { 
    MarqueeOffTime = CurrentTime;
    MarqueeMultiple = 1;
    //Serial.println(F("MarqueeOffTime set to CurrentTime."));
  } else {
    MarqueeMultiple += 1;
    //Serial.println(F("MarqueeMultiple incremented."));
  }
}

//
// CheckHighScores
//

void CheckHighScores() {
  unsigned long highestScore = 0;
  int highScorePlayerNum = 0;
  for (int count = 0; count < CurrentNumPlayers; count++) {
    if (CurrentScores[count] > highestScore) highestScore = CurrentScores[count];
    highScorePlayerNum = count;
  }
  Serial.println();
  Serial.print(F("HighScore : "));
  Serial.println(HighScore, DEC);
  Serial.print(F("highestScore : "));
  Serial.println(highestScore, DEC);
  Serial.println();

  // Remove number of goals reached from high score
  highestScore = highestScore/10*10;

  if (highestScore > HighScore) {
    HighScore = highestScore;
    //Serial.println(F("highestScore was higher than HighScore: "));
    //Serial.println();
    if (HighScoreReplay) {
      AddCredit(false, 3);
      RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + 3);
    }
    RPU_WriteULToEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, highestScore);
    RPU_WriteULToEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE) + 1);

    for (int count = 0; count < 4; count++) {
      if (count == highScorePlayerNum) {
        RPU_SetDisplay(count, CurrentScores[count], true, 2);
      } else {
        RPU_SetDisplayBlank(count, 0x00);
      }
    }

    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 250, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, 3, CurrentTime + 500, true);
  }
}


// 
// Marquee Animation Timing
//

void MarqueeAttract(byte Segment, int Speed, boolean CW, boolean priority) {

if (MarqueeDisabled && (priority == false)) {
  return;
}

// Default speeds, Segment 1 = 100
  
  byte MQPhase1 = (CurrentTime/Speed)%4; // 4 steps
  byte MQPhase2 = (CurrentTime/(Speed*3))%3; // 3 steps
  if ((Segment == 1) || (Segment == 4)) {
    // Rotate CW
    // First leg
    RPU_SetLampState(LA_BIG_1, (CW == true)?(MQPhase1==0):(MQPhase1==3), 0); 
    RPU_SetLampState(LA_BIG_2, (CW == true)?(MQPhase1==1):(MQPhase1==2), 0);
    RPU_SetLampState(LA_BIG_3, (CW == true)?(MQPhase1==2):(MQPhase1==1), 0);
    RPU_SetLampState(LA_BIG_4, (CW == true)?(MQPhase1==3):(MQPhase1==0), 0);
    // Second leg
    RPU_SetLampState(LA_BIG_5, (CW == true)?(MQPhase1==0):(MQPhase1==3), 0); 
    RPU_SetLampState(LA_BIG_9, (CW == true)?(MQPhase1==1):(MQPhase1==2), 0);
    RPU_SetLampState(LA_BIG_12, (CW == true)?(MQPhase1==2):(MQPhase1==1), 0);
    RPU_SetLampState(LA_BIG_14, (CW == true)?(MQPhase1==3):(MQPhase1==0), 0);
    // Third leg
    RPU_SetLampState(LA_BIG_15, (CW == true)?(MQPhase1==0):(MQPhase1==3), 0); 
    RPU_SetLampState(LA_BIG_13, (CW == true)?(MQPhase1==1):(MQPhase1==2), 0);
    RPU_SetLampState(LA_BIG_10, (CW == true)?(MQPhase1==2):(MQPhase1==1), 0);
    RPU_SetLampState(LA_BIG_6, (CW == true)?(MQPhase1==3):(MQPhase1==0), 0);
    // Inner rack lamps 
    RPU_SetLampState(LA_BIG_7, (CW == true)?(MQPhase2==0):(MQPhase2==2), 0); 
    RPU_SetLampState(LA_BIG_11, (CW == true)?(MQPhase2==1):(MQPhase2==1), 0);
    RPU_SetLampState(LA_BIG_8, (CW == true)?(MQPhase2==2):(MQPhase2==0), 0);
  }

  byte MQPhase3 = (CurrentTime/(Speed*54/100))%6; //  6 steps
  if ((Segment == 2) || (Segment == 4)) {
    // Bank Shot lamps
    RPU_SetLampState(LA_BANK_SHOT_5000, ((CW == true)?(MQPhase3==0):(MQPhase3==5))||((CW == true)?(MQPhase3==1):(MQPhase3==0)), ((CW == true)?(MQPhase3==1):(MQPhase3==0))); 
    RPU_SetLampState(LA_BANK_SHOT_1500, ((CW == true)?(MQPhase3==1):(MQPhase3==4))||((CW == true)?(MQPhase3==2):(MQPhase3==5)), ((CW == true)?(MQPhase3==2):(MQPhase3==5)));
    RPU_SetLampState(LA_BANK_SHOT_1200, ((CW == true)?(MQPhase3==2):(MQPhase3==3))||((CW == true)?(MQPhase3==3):(MQPhase3==4)), ((CW == true)?(MQPhase3==3):(MQPhase3==4)));
    RPU_SetLampState(LA_BANK_SHOT_900, ((CW == true)?(MQPhase3==3):(MQPhase3==2))||((CW == true)?(MQPhase3==4):(MQPhase3==3)), ((CW == true)?(MQPhase3==4):(MQPhase3==3)));
    RPU_SetLampState(LA_BANK_SHOT_600, ((CW == true)?(MQPhase3==4):(MQPhase3==1))||((CW == true)?(MQPhase3==5):(MQPhase3==2)), ((CW == true)?(MQPhase3==5):(MQPhase3==2)));
    RPU_SetLampState(LA_BANK_SHOT_300, ((CW == true)?(MQPhase3==5):(MQPhase3==0))||((CW == true)?(MQPhase3==0):(MQPhase3==1)), ((CW == true)?(MQPhase3==0):(MQPhase3==1)));
  }

  byte MQPhase4 = (CurrentTime/(Speed*3/2))%3; //  3 steps
  byte MQPhase5 = (CurrentTime/(Speed*60/100))%8; //  8 steps

  if ((Segment == 5) || (Segment == 4)) {
    // Multiplier lamps
    RPU_SetLampState(LA_5X, MQPhase4==0||MQPhase4==1, 0); 
    RPU_SetLampState(LA_3X, MQPhase4==1||MQPhase4==2, 0);
    RPU_SetLampState(LA_2X, MQPhase4==2||MQPhase4==0, 0);
  }

  if ((Segment == 3) || (Segment == 4)) {
    // Upper alley Marquee
    // Top Row
    RPU_SetLampState(LA_SMALL_1, MQPhase5==0||MQPhase5==1||MQPhase5==2, MQPhase5==2); 
    RPU_SetLampState(LA_SMALL_2, MQPhase5==1||MQPhase5==2||MQPhase5==3, MQPhase5==3);
    RPU_SetLampState(LA_SMALL_3, MQPhase5==2||MQPhase5==3||MQPhase5==4, MQPhase5==4);
    RPU_SetLampState(LA_SMALL_4, MQPhase5==3||MQPhase5==4||MQPhase5==5, MQPhase5==5);
    // Bottom Row
    RPU_SetLampState(LA_SMALL_12, MQPhase5==4||MQPhase5==5||MQPhase5==6, MQPhase5==6); 
    RPU_SetLampState(LA_SMALL_11, MQPhase5==5||MQPhase5==6||MQPhase5==7, MQPhase5==7);
    RPU_SetLampState(LA_SMALL_10, MQPhase5==6||MQPhase5==7||MQPhase5==0, MQPhase5==0);
    RPU_SetLampState(LA_SMALL_9, MQPhase5==7||MQPhase5==0||MQPhase5==1, MQPhase5==1);
  }

  byte MQPhase6 = (CurrentTime/Speed)%4; // 4 steps

  if ((Segment == 6) || (Segment == 4)) {
    // Player Lamps
    RPU_SetLampState(LA_1_PLAYER, MQPhase6==0||MQPhase6==1, 0); 
    RPU_SetLampState(LA_2_PLAYER, MQPhase6==1||MQPhase6==2, 0);
    RPU_SetLampState(LA_3_PLAYER, MQPhase6==2||MQPhase6==3, 0);
    RPU_SetLampState(LA_4_PLAYER, MQPhase6==3||MQPhase6==0, 0);
  }

  if ((Segment == 7) || (Segment == 4)) {
    // Remaining lamps
    RPU_SetLampState(LA_SMALL_5, 1, 0, 125);
    RPU_SetLampState(LA_SMALL_13, 1, 0, 250);
    RPU_SetLampState(LA_SMALL_6, 1, 0, 125);
    RPU_SetLampState(LA_SMALL_14, 1, 0, 250);
    RPU_SetLampState(LA_SMALL_7, 1, 0, 125);
    RPU_SetLampState(LA_SMALL_15, 1, 0, 250);
 
    RPU_SetLampState(LA_SMALL_8, 1, 0, 250);
    RPU_SetLampState(SAME_PLAYER, 1, 0, 500);
    RPU_SetLampState(LA_SUPER_BONUS, 1, 0, 500);
    RPU_SetLampState(LA_SPINNER, 1, 0, 250);
    RPU_SetLampState(LA_OUTLANE_SPECIAL, 1, 0, 125);
  }
}

//
//  ChimeScoring Ver 3 - Handles negative numbers
//

void ChimeScoring() {

int WaitTimeShort = 25;
int WaitTimeLong = 100;

  if ( (CurrentTime - ChimeScoringDelay) > (((Silent_Thousand_Pts_Stack) || (Silent_Hundred_Pts_Stack))?WaitTimeShort:WaitTimeLong) ) {
    if (Silent_Hundred_Pts_Stack > 0) {
      CurrentScores[CurrentPlayer] += 100;
      Silent_Hundred_Pts_Stack -= 1;
      //Serial.print(F("Silent_Hundred_Pts_Stack is: "));
      //Serial.println(Silent_Hundred_Pts_Stack, DEC);
    } else if (Silent_Thousand_Pts_Stack > 0) {          // Stack counts down
      CurrentScores[CurrentPlayer] += 1000;
      Silent_Thousand_Pts_Stack -= 1;
      //Serial.print(F("Silent_Thousand_Pts_Stack is: "));
      //Serial.println(Silent_Thousand_Pts_Stack, DEC);
    } else if (Silent_Thousand_Pts_Stack < 0) {          // Stack counts up
      CurrentScores[CurrentPlayer] -= 1000;
      Silent_Thousand_Pts_Stack += 1;
      //Serial.print(F("Silent_Thousand_Pts_Stack is: "));
      //Serial.println(Silent_Thousand_Pts_Stack, DEC);
    } else if (Ten_Pts_Stack > 0) {
      PlaySoundEffect(SOUND_EFFECT_10_PTS);
      CurrentScores[CurrentPlayer] += 10;
      Ten_Pts_Stack -= 1;
      //Serial.print(F("Ten_Pts_Stack is: "));
      //Serial.println(Ten_Pts_Stack, DEC);
    } else if (Hundred_Pts_Stack > 0) {
      PlaySoundEffect(SOUND_EFFECT_100_PTS);
      CurrentScores[CurrentPlayer] += 100;
      Hundred_Pts_Stack -= 1;
      //Serial.print(F("Hundred_Pts_Stack is: "));
      //Serial.println(Hundred_Pts_Stack, DEC);
    } else if (Thousand_Pts_Stack > 0) {
      PlaySoundEffect(SOUND_EFFECT_1000_PTS);
      CurrentScores[CurrentPlayer] += 1000;
      Thousand_Pts_Stack -= 1;
      //Serial.print(F("Thousand_Pts_Stack is: "));
      //Serial.println(Thousand_Pts_Stack, DEC);
    }
    
    if (Silent_Thousand_Pts_Stack > 0) {
      ChimeScoringDelay = ChimeScoringDelay + WaitTimeShort;
    } else {
      ChimeScoringDelay = ChimeScoringDelay + WaitTimeLong;
    }
    //Serial.print(F("ChimeScoringDelay is: "));
    //Serial.println(ChimeScoringDelay, DEC);

    ChimeScoringDelay = CurrentTime;
  }
}


//
// Match Mode - ShowMatchSequence
//

unsigned long MatchSequenceStartTime = 0;
unsigned long MatchDelay = 150;
//byte MatchDigit = 0;
byte NumMatchSpins = 0;
byte ScoreMatches = 0;

int ShowMatchSequence(boolean curStateChanged) {
  if (!MatchFeature) return MACHINE_STATE_ATTRACT;

  if (curStateChanged) {
    MatchSequenceStartTime = CurrentTime;
    MatchDelay = 2250;          // Wait this long after game end to start match mode
    //MatchDigit = random(0, 10);
    //MatchDigit = 1;    // Test value, force to be 1 (10)
    MatchDigit = CurrentTime%10;
    NumMatchSpins = 0;
    RPU_SetLampState(MATCH, 1, 0);
    RPU_SetDisableFlippers();
    ScoreMatches = 0;
  }

  if (NumMatchSpins < 40) {
    if ( (CurrentTime - MatchSequenceStartTime) > (MatchDelay) ) {
//    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      MatchDigit += 1;
      if (MatchDigit > 9) MatchDigit = 0;
      //PlaySoundEffect(10+(MatchDigit%2));
      RPU_PushToTimedSolenoidStack(SOL_CHIME_EXTRA, 1, 0, true);
      BIPDispValue = ((int)MatchDigit * 10);
      MatchDelay += 50 + 4 * NumMatchSpins;
      NumMatchSpins += 1;
      RPU_SetLampState(MATCH, NumMatchSpins % 2, 0);

      if (NumMatchSpins == 40) {
        RPU_SetLampState(MATCH, 0);
        MatchDelay = CurrentTime - MatchSequenceStartTime;
      }
    }
  }

  if (NumMatchSpins >= 40 && NumMatchSpins <= 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      if ( (CurrentNumPlayers > (NumMatchSpins - 40)) && ((CurrentScores[NumMatchSpins - 40] / 10) % 10) == MatchDigit) {
        ScoreMatches |= (1 << (NumMatchSpins - 40));
        AddSpecialCredit();
        MatchDelay += 1000;
        NumMatchSpins += 1;
        RPU_SetLampState(MATCH, 1);
      } else {
        NumMatchSpins += 1;
      }
      if (NumMatchSpins == 44) {
        MatchDelay += 5000;
      }
    }
  }

  if (NumMatchSpins > 43) {
    RPU_SetLampState(MATCH, 1);
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      return MACHINE_STATE_ATTRACT;
    }
  }

  for (int count = 0; count < 4; count++) {
    if ((ScoreMatches >> count) & 0x01) {
      // If this score matches, we're going to flash the last two digits
      if ( (CurrentTime / 200) % 2 ) {
        RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) & 0x0F);
      } else {
        RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) | 0x30);
      }
    }
  }

  return MACHINE_STATE_MATCH_MODE;
}
