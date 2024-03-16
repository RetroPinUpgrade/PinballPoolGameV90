/**************************************************************************
    Pinball Pool Game is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <https://www.gnu.org/licenses/>.
*/

// These are exmaple values - fill these in with your machine's definitions

#define NUMBER_OF_LAMPS        60

// Lamp Numbers (defines for lamps)

#define LA_SMALL_1                         0 // Small 1
#define LA_SMALL_2                         1 // Small 2
#define LA_SMALL_3                         2 // Small 3
#define LA_SMALL_4                         3 // Small 4
#define LA_SMALL_5                         4 // Small 5
#define LA_SMALL_6                         5 // Small 6
#define LA_SMALL_7                         6 // Small 7
#define LA_SMALL_8                         7 // Small 8
#define LA_SMALL_9                         8 // Small 9
#define LA_SMALL_10                        9 // Small 10
#define LA_SMALL_11                       10 // Small 11
#define LA_SMALL_12                       11 // Small 12
#define LA_SMALL_13                       12 // Small 13
#define LA_SMALL_14                       13 // Small 14
#define LA_SMALL_15                       14 // Small 15
#define LA_LEFT_ARROW                     15 // Left Arrow
#define LA_BANK_SHOT_300                  16 // Bank Shot 300 1
#define LA_BANK_SHOT_600                  17 // Bank Shot 600 2
#define LA_BANK_SHOT_900                  18 // Bank Shot 900 3
#define LA_BANK_SHOT_1200                 19 // Bank Shot 1200 4
#define LA_BANK_SHOT_1500                 20 // Bank Shot 1500 5
#define LA_BANK_SHOT_5000                 21 // Bank Shot 5000 6
#define LA_OUTLANE_SPECIAL                22 // Outlane Special
#define SAME_PLAYER                       23 // Same Player Shoots Again - Playfield
#define LA_BIG_1                          24 // Big 1
#define LA_BIG_2                          25 // Big 2
#define LA_BIG_3                          26 // Big 3
#define LA_BIG_4                          27 // Big 4
#define LA_BIG_5                          28 // Big 5
#define LA_BIG_6                          29 // Big 6
#define LA_BIG_7                          30 // Big 7
#define LA_BIG_8                          31 // Big 8
#define LA_BIG_9                          32 // Big 9
#define LA_BIG_10                         33 // Big 10
#define LA_BIG_11                         34 // Big 11
#define LA_BIG_12                         35 // Big 12
#define LA_BIG_13                         36 // Big 13
#define LA_BIG_14                         37 // Big 14
#define LA_BIG_15                         38 // Big 15
#define LA_RIGHT_ARROW                    39 // Right Arrow
#define LA_SPSA_BACK                      40 // Same Player Shoots Again - Back Box
#define MATCH                             41 // Match
#define LA_SPINNER                        42 // Spinner and Kickback Arrow
#define LA_CREDIT_INDICATOR               43 // Credit Indicator
#define LA_SUPER_BONUS                    44 // Super Bonus
#define LA_5X                             45 // 5X
#define LA_3X                             46 // 3X
#define LA_2X                             47 // 2X
#define BALL_IN_PLAY                      48 // Ball in Play - not working
#define HIGH_SCORE                        49 // High Score
#define GAME_OVER                         50 // Game Over
#define TILT                              51 // Tilt
#define LA_1_PLAYER                       52 // 1 Player
#define LA_2_PLAYER                       53 // 2 Player
#define LA_3_PLAYER                       54 // 3 Player
#define LA_4_PLAYER                       55 // 4 Player
#define PLAYER_1_UP                       56 // Player 1 Up
#define PLAYER_2_UP                       57 // Player 2 Up
#define PLAYER_3_UP                       58 // Player 3 Up
#define PLAYER_4_UP                       59 // Player 4 Up


// Attract Animation Definitions


//#define RACK_BALL_START    24

#define NUM_OF_TRIANGLE_LAMPS_CW 37
struct AttractLampsDown {
  byte lightNumDown;
  byte rowDown;
};
struct AttractLampsDown AttractLampsDown[] = {
  {LA_BIG_7,           1},
  {LA_BIG_11,          2},
  {LA_BIG_8,           3},
  {LA_BIG_4,           4},
  {LA_BIG_3,           5},
  {LA_BIG_2,           6},
  {LA_BIG_1,           7},
  {LA_BIG_6,           8},
  {LA_BIG_10,          9},
  {LA_BIG_13,         10},
  {LA_BIG_15,         11},
  {LA_BIG_14,         12},
  {LA_BIG_12,         13},
  {LA_BIG_9,          14},
  {LA_BIG_5,          15},
  {LA_BANK_SHOT_300,  16},
  {LA_BANK_SHOT_600,  17},
  {LA_BANK_SHOT_900,  18},
  {LA_BANK_SHOT_1200, 19},
  {LA_BANK_SHOT_1500, 20},
  {LA_BANK_SHOT_5000, 21},
  {LA_SMALL_12,       22},
  {LA_SMALL_4,        23},
  {TILT,              23},
  {LA_SMALL_3,        24},
  {LA_SMALL_11,       25},
  {LA_RIGHT_ARROW,    25},
  {LA_SMALL_10,       26},
  {LA_SPSA_BACK,      26},
  {LA_SMALL_2,        27},
  {LA_LEFT_ARROW,     27},
  {LA_SMALL_1,        28},
  {LA_SMALL_9,        28},
  {LA_4_PLAYER,       29},
  {LA_3_PLAYER,       30},
  {LA_2_PLAYER,       31},
  {LA_1_PLAYER,       32}
};

//
// Rain down effect, 45 lamps, 22 frames
//

struct AttractLampsDown RollDown[] = {
  {LA_SMALL_1,         1},
  {LA_SMALL_2,         1},
  {LA_SMALL_3,         1},
  {LA_SMALL_4,         1},
  {LA_SMALL_9,         2},
  {LA_SMALL_10,        2},
  {LA_SMALL_11,        2},
  {LA_SMALL_12,        2},
  {LA_LEFT_ARROW,      4},
  {LA_RIGHT_ARROW,     4},
  {LA_SMALL_8,         5},
  {LA_BANK_SHOT_5000,  5},
  {LA_BANK_SHOT_1500,  6},
  {LA_BANK_SHOT_1200,  7},
  {LA_BANK_SHOT_900,   8},
  {LA_BANK_SHOT_600,   9},
  {LA_BANK_SHOT_300,  10},
  {LA_SPINNER,        11},
  {LA_SUPER_BONUS,    11},
  {LA_SMALL_5,        12},
  {LA_SMALL_6,        12},
  {LA_SMALL_13,       13},
  {LA_2X,             13},
  {LA_3X,             13},
  {LA_5X,             13},
  {LA_SMALL_14,       13},
  {LA_BIG_1,          15},
  {LA_BIG_2,          15},  
  {LA_BIG_3,          15},  
  {LA_BIG_4,          15},
  {LA_BIG_5,          15},
  {LA_SMALL_7,        15},
  {LA_OUTLANE_SPECIAL,15},
  {LA_BIG_6,          16},
  {LA_BIG_7,          16},
  {LA_BIG_8,          16},
  {LA_BIG_9,          16},
  {LA_SMALL_15,       16},
  {LA_BIG_10,         17},
  {LA_BIG_11,         17},  
  {LA_BIG_12,         17},  
  {LA_BIG_13,         18},  
  {LA_BIG_14,         18},  
  {LA_BIG_15,         19},  
  {SAME_PLAYER,       21},
};

//
// Left to right sweep, 46 lamps, 19 frames
//

struct AttractLampsDown SweepLefttoRight[] = {
  {LA_SMALL_5,         1},
  {LA_SMALL_13,        2},
  {LA_SPINNER,         3},
  {LA_BIG_1,           4},
  {LA_SMALL_1,         5},
  {LA_SMALL_9,         5},
  {LA_BIG_6,           5},
  {LA_2X,              6},
  {LA_BIG_2,           6},
  {LA_BIG_10,          6},
  {LA_SMALL_2,         7},
  {LA_SMALL_10,        7},
  {LA_LEFT_ARROW,      7},
  {LA_BIG_7,           7},
  {LA_BIG_13,          7},
  {LA_SUPER_BONUS,     8},
  {LA_3X,              8},
  {LA_BIG_3,           8},
  {LA_BIG_11,          8},
  {LA_BIG_15,          8},
  {SAME_PLAYER,        8},
  {LA_SMALL_3,         9},
  {LA_SMALL_11,        9},
  {LA_RIGHT_ARROW,     9},
  {LA_BIG_8,           9},
  {LA_BIG_14,          9},
  {LA_5X,             10},
  {LA_BIG_4,          10},
  {LA_BIG_12,         10},
  {LA_SMALL_4,        11},
  {LA_SMALL_12,       11},
  {LA_BIG_9,          11},
  {LA_BIG_5,          12},
  {LA_SMALL_8,        13},
  {LA_BANK_SHOT_300,  14},
  {LA_SMALL_14,       14},
  {LA_BANK_SHOT_600,  15},
  {LA_SMALL_6,        15},
  {LA_SMALL_7,        15},
  {LA_SMALL_15,       15},
  {LA_BANK_SHOT_900,  16},
  {LA_BANK_SHOT_1200, 17},
  {LA_BANK_SHOT_1500, 18},
  {LA_BANK_SHOT_5000, 19},
  {LA_OUTLANE_SPECIAL,19},
};



byte LampAnimation1[12][8] = {
// Rotating bars rack area
{ 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x87, 0x01, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x41, 0x08, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x21, 0x26, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x21, 0x52, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x42, 0x52, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x04, 0x44, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x88, 0x68, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x10, 0x69, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x10, 0x1D, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x90, 0x02, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00  },
};

//byte LampAnimation1a[12][8] = {
byte LampAnimation1a[15][8] = {
// Rotating bars outer lamps
{  0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00  },
{ 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00  },
{ 0x30, 0x30, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00  },
{ 0x40, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00  }
};


byte LeftRightSweep[13][8] = {
// Left Right Sweep
{ 0x10, 0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00  },
{ 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x02, 0x02, 0x80, 0x00, 0x00  },
{ 0x02, 0x82, 0x00, 0x40, 0x10, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x80, 0x04, 0x44, 0x50, 0x00, 0x00  },
{ 0x04, 0x04, 0x00, 0x80, 0xA0, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x08, 0x08, 0x20, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00  },
{ 0x08, 0x08, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00  },
{ 0x80, 0x20, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x20, 0x40, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00  }
};

byte LampAnimation3[15][8] = {
// 3 way sweep
// Most lamps version
{ 0x71, 0x00, 0x01, 0x1F, 0x00, 0x02, 0x00, 0x00  },
{ 0x00, 0x72, 0x42, 0xE0, 0x01, 0x00, 0x00, 0x00  },
{ 0x70, 0x80, 0x44, 0x00, 0x0E, 0x00, 0x00, 0x00  },
{ 0x00, 0x70, 0x48, 0x00, 0xB0, 0x00, 0x00, 0x00  },
{ 0x70, 0x74, 0x10, 0x00, 0x40, 0x02, 0x00, 0x00  },
{ 0x78, 0x00, 0x20, 0x21, 0x52, 0x02, 0x00, 0x00  },
{ 0x00, 0x78, 0x50, 0x42, 0x24, 0x00, 0x00, 0x00  },
{ 0x74, 0x00, 0x48, 0x84, 0x08, 0x00, 0x00, 0x00  },
{ 0x02, 0x70, 0x44, 0x08, 0x01, 0x00, 0x00, 0x00  },
{ 0x70, 0x01, 0x02, 0x10, 0x00, 0x02, 0x00, 0x00  },
{ 0x01, 0x71, 0x04, 0x10, 0x69, 0x02, 0x00, 0x00  },
{ 0x73, 0x83, 0x48, 0x88, 0x14, 0x00, 0x00, 0x00  },
{ 0x07, 0xF7, 0x50, 0x44, 0x82, 0x00, 0x00, 0x00  },
{ 0x7F, 0x8F, 0x48, 0x22, 0x80, 0x00, 0x00, 0x00  },
{ 0x70, 0x71, 0x04, 0x01, 0x00, 0x02, 0x00, 0x00  }
};

// Rack plus Bank Shot
byte Rack3waysweep[15][8] = {
{ 0x00, 0x00, 0x01, 0x1F, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x02, 0xE0, 0x01, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x04, 0x00, 0x0E, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x08, 0x00, 0x30, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x10, 0x00, 0x40, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x20, 0x21, 0x52, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x10, 0x42, 0x24, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x08, 0x84, 0x08, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x04, 0x08, 0x01, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x02, 0x10, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x04, 0x10, 0x69, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x08, 0x88, 0x14, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x10, 0x44, 0x02, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x08, 0x22, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00  }
};


byte LampAnimation4[5][8] = {
// Right Arrow
{ 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00  }
};

byte LampAnimation5[5][8] = {
// Left Arrow
{ 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00  }
};

byte LampAnimation6[5][8] = {
// Downward V
{ 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00  }
};

byte LampAnimation7[14][8] = {
// Downward Wave - no rack
{ 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00  },
{ 0x80, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x30, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00  },
{ 0x00, 0x30, 0x00, 0x00, 0x00, 0xE4, 0x00, 0x00  },
{ 0x40, 0x00, 0x40, 0x0E, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x40, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00  }
};


// Rack region large arrows

int RackLeftArrow[6] = {25, 26, 29, 30, 34, 37};
int RackRightArrow[6] = {26, 27, 31, 32, 34, 36};
int DownwardV[9] = {24, 28, 29, 32, 33, 35, 36, 37, 38};


// RoamingStage1a - small balls scanning

byte RoamingStage1a[16][8] = {
{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  }
};
/*
byte RoamingStage1a[7][8] = {
{ 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x02, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x04, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x08, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x20, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x10, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  }
};
*/

// RoamingStage1b - rack looping
byte RoamingStage1b[18][8] = {
{ 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x04, 0x44, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x08, 0x08, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
};

// RADAR Middle Left Pivot 22 rows plus blanks - pivot within PF
byte RADARMiddleLeftPivot[28][8] = {
{ 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x10, 0x00, 0x00, 0x21, 0x52, 0x00, 0x00, 0x00  },
{ 0x00, 0x10, 0x00, 0x40, 0x24, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00  },
{ 0x40, 0x40, 0x40, 0x10, 0x00, 0x80, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00  },
{ 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x20, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00  },
{ 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00  },
{ 0x08, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
// Blank rows
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  }
};

// RADAR Middle Right Pivot - pivot within PF
// 22 rows plus blanks
byte RADARMiddleRightPivot[28][8] = {

{ 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x10, 0x69, 0x00, 0x00, 0x00  },
{ 0x20, 0x20, 0x00, 0x80, 0x14, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x48, 0x02, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x01, 0x00, 0x20, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00  },
{ 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x80, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x01, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00  },
{ 0x02, 0x02, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00  },
{ 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x88, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
// Blank rows
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  }
};

// RADAR Super Bonus Pivot, SuperBonus lamp 44 always on
// Manually added blank rows 
byte RADARSuperBonusPivot[26][8] = {

{ 0x04, 0x04, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00  },
{ 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  }, // 1
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  }, // Added 0 row
{ 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00  }, // 6
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  }, // Added 0 row
{ 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x40, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x40, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x80, 0x28, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x80, 0x04, 0x44, 0x40, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x40, 0x12, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0x00, 0x00  },
{ 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00  }, // 18
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  }, // Added 0 row
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  }, // Added 0 row
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  }, // Added 0 row
{ 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x02, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  }
};

// RADAR Top Pivot - 20 rows plus 6 blanks
// Manually added blank rows 
byte RADARTopPivot[26][8] = {

{ 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x10, 0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x20, 0x00, 0x80, 0x00, 0x00  },
{ 0x00, 0x80, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x80, 0x04, 0x44, 0x50, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x08, 0x88, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x01, 0x20, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00  },
{ 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x04, 0x04, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
// Blank rows
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  }
};

// RADAR bottom Pivot - 15 rows plus 7 blanks - 38 always on
// Manually added blank rows 
byte RADARBotPivot[22][8] = {
{ 0x10, 0x10, 0x00, 0x21, 0x52, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00  },
{ 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x02, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x80, 0x04, 0x04, 0x50, 0x00, 0x00  },
{ 0x04, 0x04, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00  },
{ 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00  },
{ 0x80, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x20, 0x20, 0x00, 0x10, 0x29, 0x00, 0x00, 0x00  },
{ 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },
};





#define NUM_OF_SWITCHES     26

// Defines for switches

#define SW_10_PTS          0
#define SW_TOP_LEFT_TARGET 1
#define SW_BANK_SHOT       2
#define SW_CREDIT_RESET    5
#define SW_TILT            6
#define SW_OUTHOLE         7
#define SW_COIN_3          9
#define SW_COIN_1          8
#define SW_COIN_2          10
#define SW_SLAM            15
#define SW_SPINNER         16
#define SW_8_BALL          24
#define SW_7_15_BALL       25
#define SW_6_14_BALL       26
#define SW_5_13_BALL       27
#define SW_4_12_BALL       28
#define SW_3_11_BALL       29
#define SW_2_10_BALL       30
#define SW_1_9_BALL        31
#define SW_RIGHT_OUTLANE   32
#define SW_LEFT_OUTLANE    33
#define SW_RIGHT_SLING     35
#define SW_LEFT_SLING      36 
#define SW_BUMPER_BOTTOM   37
#define SW_BUMPER_RIGHT    38
#define SW_BUMPER_LEFT     39



// Defines for solenoids

#define SOL_CHIME_10        1 // 1
#define SOL_CHIME_100       2 // 2
#define SOL_CHIME_1000      3 // 3
#define SOL_CHIME_EXTRA     4 // 4
#define SOL_KNOCKER         5 // 5
#define SOL_OUTHOLE         6 // 6
#define SOL_KICKER          7 // 7
#define SOL_BUMPER_LEFT     8 // 8
#define SOL_BUMPER_RIGHT    9 // 9
#define SOL_BUMPER_BOTTOM  10 // 10
#define SOL_LEFT_SLING     11 // 11
#define SOL_RIGHT_SLING    13 // 13


// 
// Leave Coin Lockout and and flippers out of the defintions
//
// #define SOL_COIN_LOCKOUT     xx
// #define SOL_FLIPPERS         14


// SWITCHES_WITH_TRIGGERS are for switches that will automatically
// activate a solenoid (like in the case of a chime that rings on a rollover)
// but SWITCHES_WITH_TRIGGERS are fully debounced before being activated
#define NUM_SWITCHES_WITH_TRIGGERS        5

// PRIORITY_SWITCHES_WITH_TRIGGERS are switches that trigger immediately
// (like for pop bumpers or slings) - they are not debounced completely
#define NUM_PRIORITY_SWITCHES_WITH_TRIGGERS 5

// Define automatic solenoid triggers (switch, solenoid, number of 1/120ths of a second to fire)
// Put chime call before the solenoid call, seems to do things in reverse order

struct PlayfieldAndCabinetSwitch TriggeredSwitches[] = {
  { SW_LEFT_SLING, SOL_LEFT_SLING, 4 },
  { SW_RIGHT_SLING, SOL_RIGHT_SLING, 4 },
  { SW_BUMPER_LEFT, SOL_BUMPER_LEFT, 3 },
  { SW_BUMPER_RIGHT, SOL_BUMPER_RIGHT, 3 },
  { SW_BUMPER_BOTTOM, SOL_BUMPER_BOTTOM, 3 },
};
