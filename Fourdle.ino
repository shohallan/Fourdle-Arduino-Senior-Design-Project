/*
  Fourdle Main Code
*/

//#define DEBUG_PRINT_EEPROM

// AVR library to use the sleep modes on the Atmega 2560
#include <avr/sleep.h>

// Low-Power library by rocketscream for ease of use when changing power modes
// https://github.com/rocketscream/RocketScream_LowPowerAVRZero
#include "RocketScream_LowPowerAVRZero.h"

// Rocket Scream RTCAVRZero - Version: Latest
#include <RocketScream_RTCAVRZero.h>

// Arduino library to use EEPROM
#include <EEPROM.h>

// Used for I2C protocol with keyboard
#include <Wire.h>

// User made function to check keyboard inputs
#include "CheckKeyboard.h"

// Adafruit GFX Library - Version: Latest
#include <Adafruit_GFX.h>

// Adafruit LED Backpack Library - Version: Latest
#include <Adafruit_LEDBackpack.h>

// Includes word bank data
#include "WordBank.h"

// Includes FastLED library
#include <FastLED.h>

// Includes various led pixel types for the 4x4 leds
#include <pixeltypes.h>

/*
  ~Wiring the 14-segment LED backpack~

    Connect CLK to the I2C clock - on the Mega it's digital #21
    Connect DAT to the I2C data - on the Mega it's digital #20
    Connect GND to common ground
    Connect VCC+ to power - 5V is best but 3V will work if that's
                            all you've got (it will be dimmer)
    Connect Vi2c to your microcontroller's logic level (3-5V)
                          - If you're using an Arduino, this is almost certainly 5V.
                            If its a 3V Arduino such as a Due, connect it to 3V
*/

// MACROS //
#define CARDKB_ADDR 0x5F
#define DISP14SEG_ADDR 0x70
#define LED_PIN 2
#define SPEAKER_PIN 3
#define NUM_LEDS 16

//LED colors
#define RED_C CRGB(0, 50, 0)
#define GREEN_C CRGB(50, 0, 0)
#define YELLOW_C CRGB(50, 50, 0)
#define WHITE_C CRGB(50, 50, 50)
#define BLACK_C CRGB(0, 0, 0)

//EEPROM Macros
#define WORD_PREV0(_x_) 236+_x_
#define WORD_PREV1(_x_) 240+_x_
#define WORD_PREV2(_x_) 244+_x_
#define WORD_RAND(_x_) 248+_x_
#define GAME_STATE 252+(inputShift%2)
#define WIN_STREAK 254
#define INPUT_SHIFT 255

//Abstracts Serial.print()
#define prints(_x_) Serial.print(_x_)

// VARIABLES //
// Used for 14-segment LED display
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// Used for transfering words stored in flash (word bank) to RAM
char wordBuffer[5] = "    ";

// Used for storing previously made guesses
char prevWord[4][5] = {"TEST", "TEST", "TEST", "TEST"};

// Used for storing the current random word
char randWord[5] = "GOOD";

// Letter incrementor - Keeps track of which letter is currently being typed
byte digitLoc;

// Boolean to check win state
bool win = false;

// Word Bank //
#define BANK_SIZE 500
const char * const PROGMEM wordBank[BANK_SIZE] = {
  ABLE, ACID, AGED, ALSO, AREA, ARMY, AWAY,
  BABY, BACK, BALL, BAND, BANK, BASE, BATH, BEAR, BEAT, BEEN, BEER, BELL, BELT, BEST, BILL, BIRD, BLOW, BLUE, BOAT, BODY, BOMB, BOND,
  BONE, BOOK, BOOM, BORN, BOSS, BOTH, BOWL, BULK, BURN, BUSH, BUSY,
  CALL, CALM, CAME, CAMP, CARD, CARE, CASE, CASH, CAST, CELL, CHAT, CHIP, CITY, CLUB, COAL, COAT, CODE, COLD, COME, COOK, COOL, COPE,
  COPY, CORE, COST, CREW, CROP,
  DARK, DATA, DATE, DAWN, DAYS, DEAD, DEAL, DEAN, DEAR, DEBT, DEEP, DENY, DESK, DIAL, DICK, DIET, DISC, DISK, DOES, DOOM, DOOR, DOSE,
  DOWN, DRAW, DREW, DROP, DRUG, DUAL, DUKE, DUST, DUTY,
  EACH, EARN, EASE, EAST, EASY, EDGE, ELSE, EVEN, EVER, EVIL, EXIT,
  FACE, FACT, FAIL, FAIR, FALL, FARM, FAST, FATE, FEAR, FEED, FEEL, FEET, FELL, FELT, FIGS, FILL, FILM, FIND, FINE, FIRE, FIRM, FISH,
  FIVE, FLAT, FLOW, FOOD, FOOT, FORD, FORM, FORT, FOUR, FREE, FROM, FUEL, FULL, FUND,
  GAIN, GAME, GATE, GAVE, GEAR, GENE, GIFT, GIRL, GIVE, GLAD, GOAL, GOES, GOLD, GOLF, GONE, GOOD, GRAY, GREW, GREY, GROW, GULF,
  HAIR, HALF, HALL, HAND, HANG, HARD, HARM, HATE, HAVE, HEAD, HEAR, HEAT, HELD, HELL, HELP, HERE, HERO, HIKE, HILL, HIRE, HOLD, HOLE,
  HOLY, HOME, HOPE, HOST, HOUR, HUGE, HUNG, HUNT, HURT,
  IDEA, INCH, INTO, IRON, ITEM,
  JACK, JANE, JEAN, JOHN, JOIN, JUMP, JURY, JUST,
  KEEN, KEEP, KENT, KEPT, KICK, KILL, KIND, KING, KNEE, KNEW, KNOW,
  LACK, LADY, LAID, LAKE, LAND, LANE, LAST, LATE, LEAD, LEFT, LESS, LIFE, LIFT, LIKE, LINE, LINK, LIST, LIVE, LOAD, LOAN, LOCK, LOGO,
  LONG, LOOK, LORD, LOSE, LOSS, LOST, LOVE, LUCK,
  MADE, MAIL, MAIN, MAKE, MALE, MANY, MARK, MASS, MATT, MEAL, MEAN, MEAT, MEET, MENU, MERE, MIKE, MILE, MILK, MILL, MIND, MINE, MISS,
  MODE, MOOD, MOON, MORE, MOST, MOVE, MUCH, MUST,
  NAME, NAVY, NEAR, NECK, NEED, NEWS, NEXT, NICE, NICK, NINE, NONE, NOSE, NOTE,
  OKAY, ONCE, ONLY, ONTO, OPEN, ORAL, OVER,
  PACE, PACK, PAGE, PAID, PAIN, PAIR, PALM, PARK, PART, PASS, PAST, PAVE, PEAR, PINS, PION, PIPE, PLAN, PLAY, PLOT, PLUG, PLUS, POLL,
  POOL, POOR, PORT, POST, PULL, PURE, PUSH,
  RACE, RAIL, RAIN, RANK, RARE, RATE, READ, REAL, REAR, RELY, RENT, REST, RICE, RICH, RIDE, RING, RISE, RISK, ROAD, ROCK, ROLE, ROLL,
  ROOF, ROOM, ROOT, ROSE, RULE, RUSH, RUTH,
  SAFE, SAID, SAKE, SALE, SALT, SAME, SAND, SAVE, SEAT, SEED, SEEK, SEEM, SEEN, SELF, SELL, SEND, SENT, SEPT, SHIP, SHOP, SHOT, SHOW,
  SHUT, SICK, SIDE, SIGN, SITE, SIZE, SKIN, SLIP, SLOW, SNOW, SOFT, SOIL, SOLD, SOLE, SOME, SONG, SOON, SORT, SOUL, SPOT, STAR, STAY,
  STEP, STOP, SUCH, SUIT, SURE,
  TAKE, TALE, TALK, TALL, TANK, TAPE, TASK, TEAM, TECH, TELL, TEND, TERM, TEST, TEXT, THAN, THAT, THEM, THEN, THEY, THIN, THIS, THUS,
  TILL, TIME, TINY, TOLD, TOLL, TONE, TONY, TOOK, TOOL, TOUR, TOWN, TREE, TRIP, TRUE, TUNE, TURN, TWIN, TYPE,
  UNIT, UPON, USED, USER,
  VARY, VAST, VERY, VICE, VIEW, VOTE,
  WAGE, WAIT, WAKE, WALK, WALL, WANT, WARD, WARM, WASH, WAVE, WAYS, WEAK, WEAR, WEEK, WELL, WENT, WERE, WEST, WHAT, WHEN, WHOM, WIDE,
  WIFE, WILD, WILL, WIND, WINE, WING, WIRE, WISE, WISH, WITH, WOOD, WORD, WORE, WORK,
  YARD, YEAH, YEAR, YOUR,
  ZERO, ZONE
};

// STRUCTURES //
struct LetterInfo {
  char letter;
  char flag;
};

// ENUMERATIONS //
enum interruptFlags {
  ON_DISPLAYS,
  DIM_DISPLAYS,
  OFF_DISPLAYS,
  IDLE_DISPLAYS
};
uint16_t DisplayFlags = ON_DISPLAYS;

enum gameState {
  GAME_START_SCREEN,
  GAME_BEGIN_PLAY,
  GAME_INPUT_GUESS1,
  GAME_INPUT_GUESS2,
  GAME_INPUT_GUESS3,
  GAME_INPUT_GUESS4,
  GAME_END_SCREEN
} GameState;

// Define 4x4 LEDs
CRGB leds4x4[NUM_LEDS];

// Used for EEPROM testing
int addr = 0;
byte romValue;
uint8_t inputShift;

// ~~ FUNCTIONS ~~ //
// Abstracts getting the right EEPROM location for the letter inputs.
// Input requires an unsigned integer from 0 to 3 (representing the letter).

uint8_t getInputLoc(uint8_t letLoc) {
  return letLoc + 4 * inputShift/*(uint8_t)EEPROM[INPUT_SHIFT]*/;
}

// Increments the letter input EEPROM location to the next 4 bytes.
// This is done to avoid writing too much to a single location and use up
// all 100,000 writes on the EEPROM byte.
void incrInputLoc(void) {
  inputShift = EEPROM[INPUT_SHIFT] + 1;
  if (inputShift >= 59) inputShift = 0;
  EEPROM.update(INPUT_SHIFT, inputShift);
  delay(4);
}

// Stores the previously guessed words into EEPROM for savestate use.
// Does not need to store the 4th guess because the game will be over.
void storePrevWord(void) {
  // Updates previous words 0-3, letter by letter
  for (size_t let = 0; let < 4; let++) {
    EEPROM.update(WORD_PREV0(let), (byte)prevWord[0][let]);
    EEPROM.update(WORD_PREV1(let), (byte)prevWord[1][let]);
    EEPROM.update(WORD_PREV2(let), (byte)prevWord[2][let]);
  }
  delay(48);
}

// Loads the previously guessed words from EEPROM into SRAM.
void loadPrevWord(void) {
  // Loads previous words 0-3, letter by letter
  for (size_t let = 0; let < 4; let++) {
    prevWord[0][let] = (char)EEPROM[WORD_PREV0(let)];
    prevWord[1][let] = (char)EEPROM[WORD_PREV1(let)];
    prevWord[2][let] = (char)EEPROM[WORD_PREV2(let)];
  }
}

// Stores the current random word into EEPROM for savestate use.
void storeRandWord(void) {
  for (size_t let = 0; let < 4; let++)
    EEPROM.update(WORD_RAND(let), (byte)randWord[let]);
  delay(16);
}

// Loads the current random word from EEPROM into SRAM.
void loadRandWord(void) {
  for (size_t let = 0; let < 4; let++)
    randWord[let] = (char)EEPROM[WORD_RAND(let)];
}

// Loads the current random word from EEPROM into given string.
void loadRandWord(char * getRand) {
  for (size_t let = 0; let < 4; let++)
    *(getRand + let) = (char)EEPROM[WORD_RAND(let)];
}

// Stores the current game state from SRAM into EEPROM.
void storeGameState(void) {
  EEPROM.update(GAME_STATE, (byte)GameState);
  delay(4);
}

// Loads the current game state from EEPROM into SRAM.
void loadGameState(void) {
  GameState = /*(enum gameState)*/EEPROM[GAME_STATE];
}

// Loads input data and sets digitLoc parameter to correct location
void loadInputData(byte * digitLoc) {
  wordBuffer[0] = EEPROM[getInputLoc(0)];
  wordBuffer[1] = EEPROM[getInputLoc(1)];
  wordBuffer[2] = EEPROM[getInputLoc(2)];
  wordBuffer[3] = EEPROM[getInputLoc(3)];
  *digitLoc = 0;
  for(byte i = 0; i < 4; i++){
    if(wordBuffer[i] != ' '){
      (*digitLoc)++;
    } else return;
  }
  //prints("digitLoc should be 4\n");
  return;
}

void playSound(uint8_t pin, uint16_t pitch, uint16_t length) {
  tone(pin, pitch, length);
  delay(50);
  noTone(pin);
}

inline void enableDisplayLEDs(void);

// Checks if entered guess is correct
// parameter: Takes the current guessed word number from 0-3 inclusive (e.g. first guess is 0, second is 1, etc.)
bool isGuessCorrect(byte guessNum){
  // Begin checking
  struct LetterInfo randLetInfo[4];
  struct LetterInfo userLetInfo[4];
  // Put entered word into userLetInfo and initialize flags
  for (byte i = 0; i < 4; i++){
    userLetInfo[i].letter = wordBuffer[i];
    userLetInfo[i].flag = '-';
    randLetInfo[i].letter = randWord[i];
    randLetInfo[i].flag = '-';
  } 
  // First Pass: Which letters are correct?
  for (byte i = 0; i < 4; i++) {
    if (wordBuffer[i] == randWord[i]) {
      wordBuffer[i] = '+';
      leds4x4[4*guessNum + i] = GREEN_C;
      userLetInfo[i].flag = 'G';
    }
  }
  
  // Check if all letters are green so the user wins
  bool isWin = true;
  for(byte i = 0; i < 4; i++) {
    if(userLetInfo[i].flag != 'G') {
      isWin = false;
      break;
    }
  }
  if(isWin) return true;
  
  // Second Pass: Which letters are in the word but are out of place?
  for (byte i = 0; i < 4; i++) {
    if(userLetInfo[i].flag == 'G') continue;
    for(byte let = 0; let < 4; let++){
      if(userLetInfo[i].letter == randLetInfo[let].letter){
        if(userLetInfo[let].flag != 'G' && randLetInfo[let].flag != 'Y'){
          wordBuffer[i] = '/'; 
          leds4x4[4*guessNum + i] = YELLOW_C;
          userLetInfo[i].flag = 'Y';
          randLetInfo[let].flag = 'Y';
          break;
        }
      }
    }
  }
  // Third pass: Which letters are not in the word?
  for (byte i = 0; i < 4; i++) {
    if (userLetInfo[i].flag != 'G' && userLetInfo[i].flag != 'Y') {
      wordBuffer[i] = '-'; 
      leds4x4[4*guessNum + i] = BLACK_C;
    }
  }
  // Reset word buffer
  for(byte i = 0; i < 4; i++) wordBuffer[i] = ' ';
  digitLoc = 0;
  return false;
}

void updateWordDisplay(void){
  for (size_t let = 0; let < 4; let++)
        alpha4.writeDigitAscii(let, wordBuffer[let]);
  alpha4.writeDisplay();
}

// ~~ MAIN CODE ~~ //
void setup() {
  Serial.begin(9600);

  FastLED.addLeds<WS2812, LED_PIN, RGB>(leds4x4, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(3, 50);
  FastLED.setMaxRefreshRate(0);
  FastLED.clear();
  FastLED.show();

  Wire.begin();

  // --Test setting up and displaying all segments of 14-seg display-- //
  alpha4.begin(DISP14SEG_ADDR);  // pass in the address for the 14-seg display

  // Set all 14-seg leds on
  for (size_t let = 0; let < 4; let++)
    alpha4.writeDigitRaw(let, 0xFFFF);
  alpha4.setBrightness(5);
  alpha4.writeDisplay();

  // --4x4 LED Test-- //
  for (uint8_t i = 0; i < NUM_LEDS - 12; i++) leds4x4[i] = WHITE_C;
  for (uint8_t i = 4; i < NUM_LEDS - 8; i++) leds4x4[i] = WHITE_C;
  for (uint8_t i = 8; i < NUM_LEDS - 4; i++) leds4x4[i] = WHITE_C;
  for (uint8_t i = 12; i < NUM_LEDS - 0; i++) leds4x4[i] = WHITE_C;
  FastLED.setBrightness(10);
  FastLED.show();

  digitLoc = 0;

  // EEPROM.update(INPUT_SHIFT, 0); // Initialize INPUT_SHIFT value

  //delay(100);

#ifdef DEBUG_PRINT_EEPROM
  // Prints all values in the Nano Every's EEPROM
  prints("Printing EEPROM data...\n");
  addr = 0;
  while (addr < EEPROM.length()) {
    prints(addr); prints('-'); prints(addr + 3); prints(": ");
    for (size_t i = 0; i < 4; i++) {
      // if(addr >= 0 && addr < 236) EEPROM.update(addr + i, ' '); // Clears input data to spaces
      if (addr >= 252) prints((byte)EEPROM[addr + i]);
      else prints((char)EEPROM[addr + i]);
      prints(' ');
    }
    prints("\n");
    addr += 4;
  }
  prints("...done\n\n");
  //delay(1000);
#endif

  randomSeed(analogRead(2)); // randomize using noise from analog pin 2

  //delay(200);

  // Plays begin setup sound
  playSound(SPEAKER_PIN, 986, 50);
  playSound(SPEAKER_PIN, 1098, 50);

  /////////////////////////////////////
  // LOAD EEPROM DATA FOR SAVESTATES //
  /////////////////////////////////////
  //DEBUG: Store previous words in EEPROM
  //storePrevWord();
  // Load the previous words from EEPROM
  loadPrevWord();
  

  // Print and load random word stored in EEPROM
  Serial.println("Random word in EEPROM:");
  char tempRandomWord[5];
  loadRandWord(tempRandomWord);
  loadRandWord();
  Serial.println(tempRandomWord);

  // Initialize the input shift ammount to the stored value in EEPROM
  inputShift = (uint8_t)EEPROM[INPUT_SHIFT];

  // Load the previous game state to return back to where the player left off
  loadGameState();
  // DEBUG: Set game state to Start screen
  // GameState = GAME_START_SCREEN;
  
  // Reload correct LED indicator values 
  switch(GameState){
    case GAME_INPUT_GUESS4:
      for (byte i = 0; i < 4; i++) wordBuffer[i] = prevWord[2][i];
      isGuessCorrect(2);

    case GAME_INPUT_GUESS3:
      for (byte i = 0; i < 4; i++) wordBuffer[i] = prevWord[1][i];
      isGuessCorrect(1);

    case GAME_INPUT_GUESS2:
      for (byte i = 0; i < 4; i++) wordBuffer[i] = prevWord[0][i];
      isGuessCorrect(0);
      FastLED.show();
      break;
  }

  // Load player input data and change digitLoc accordingly
  loadInputData(&digitLoc);
  
  // Starts real time counter using the internal counter
  RTCAVRZero.begin(false);


  // Enables the display LEDs flag so it will smoothly move to the dimming stages
  enableDisplayLEDs();

  // Plays end setup sound
  playSound(SPEAKER_PIN, 1250, 95);
}

void loop() {
  // Swaps between display power states to conserve power
  switch (DisplayFlags) {
    case IDLE_DISPLAYS: // Quickly ends switch statement when IDLE is set
      break;

    case ON_DISPLAYS:
      // Print to 14-seg display
      for (size_t let = 0; let < 4; let++)
        alpha4.writeDigitAscii(let, wordBuffer[let]);
      alpha4.setBrightness(5);
      alpha4.writeDisplay();
      FastLED.setBrightness(100);
      FastLED.setMaxPowerInVoltsAndMilliamps(3, 70);
      FastLED.show();
      DisplayFlags = IDLE_DISPLAYS;
      break;

    case DIM_DISPLAYS:
      playSound(SPEAKER_PIN, 200, 20);
      alpha4.setBrightness(0);
      alpha4.writeDisplay();
      FastLED.setBrightness(50);
      FastLED.setMaxPowerInVoltsAndMilliamps(3, 30);
      FastLED.show();
      DisplayFlags = IDLE_DISPLAYS;
      break;

    case OFF_DISPLAYS:
      playSound(SPEAKER_PIN, 200, 20);
      playSound(SPEAKER_PIN, 200, 20);
      alpha4.clear();
      alpha4.writeDisplay();
      FastLED.setBrightness(0);
      FastLED.setMaxPowerInVoltsAndMilliamps(1, 1);
      FastLED.show();
      DisplayFlags = IDLE_DISPLAYS;
      //LowPower.standby();
      break;
  }

  //------------------------------------------//
  // MAIN FINITE STATE MACHINE FOR GAME LOGIC //
  //------------------------------------------//
  if (GameState == GAME_START_SCREEN) {
    storeGameState();
    
    // Reset 4x4 LEDs
    for (uint8_t i = 0; i < NUM_LEDS; i++) leds4x4[i] = WHITE_C;
    FastLED.show();
    
    char wordleStartScreen[15] = "FOURDLEFOURDLE";
    byte i = 0;
    // Flash title message
    for (i = 0; i < 7; i++) {
      // Check for a keyboard input
      if (checkKeyboard(CARDKB_ADDR) != NULL) {
        i = 0xFF;
        break;
      }
      for (size_t let = 0; let < 4; let++)
        alpha4.writeDigitAscii(let, wordleStartScreen[i]);
      alpha4.writeDisplay();
      delay(200);
    }
    // Scroll title message
    for (i = 0; i < 11; i++) {
      // Check for a keyboard input
      if (checkKeyboard(CARDKB_ADDR) != NULL) {
        i = 0xFF;
        break;
      }
      for (size_t let = 0; let < 4; let++)
        alpha4.writeDigitAscii(let, wordleStartScreen[i + let]);
      alpha4.writeDisplay();
      delay(500);
    }
    // If a keyboard input was read, exit the title screen and start the game
    if (i == 0xFF) {
      enableDisplayLEDs();
      prints("Title screen exited\n");
      GameState = GAME_BEGIN_PLAY;
      alpha4.clear();
      alpha4.writeDisplay();
      prints(GameState);
    }
  }

  else if (GameState == GAME_BEGIN_PLAY) {
    // storeGameState();
    
    // Reset 14-segment display
    for(byte i = 0; i < 4; i++) wordBuffer[i] = ' ';
    digitLoc = 0;
    updateWordDisplay();
    
    // Reset 4x4 LEDs
    for (uint8_t i = 0; i < NUM_LEDS; i++) leds4x4[i] = WHITE_C;
    FastLED.show();
    
    // Generate a new random word
    prints("Generating random word...\n");
    int randNum = random(BANK_SIZE);
    strcpy_P(randWord, (PGM_P)pgm_read_word(&(wordBank[randNum])));
    prints("Word Bank Location "); Serial.print(randNum); Serial.print(": ");
    Serial.println(randWord); // print out a random word from the word bank
    
    storeRandWord(); // Store new random word into EEPROM
    
    //delay(1000);

    GameState = GAME_INPUT_GUESS1;
    prints("First Guess...\n");
  }

  else if (GameState == GAME_INPUT_GUESS1) {
    storeGameState();
    
    char key = checkKeyboard(CARDKB_ADDR);
    if(key != NULL){ // If the entered key is one of the game keys (alphabet, backspace, etc.)...
      enableDisplayLEDs();
      if(key == 0x08){ // If the key is a backspace character...
        if(digitLoc > 0){ // and letters were entered...
          playSound(3, 300, 20); //play sound through speaker
          playSound(3, 300, 20);
          // Delete (replace with whitespace) the previous digit location by first decrementing the digit location
          wordBuffer[--digitLoc] = ' '; 
          
          EEPROM.update(getInputLoc(digitLoc), ' ');
          delay(4);
        }
      }
      else if(key == ','){}
      else if(key == '.'){ // If the entered key is a space...
        // Save currently typed letters
        char tempSave[4];
        tempSave[0] = wordBuffer[0]; tempSave[1] = wordBuffer[1]; tempSave[2] = wordBuffer[2]; tempSave[3] = wordBuffer[3];
        // Display the win streak
        byte winStreak = EEPROM[WIN_STREAK];
        prints("Win streak: ");
        Serial.println(winStreak);
        wordBuffer[0] = 'W'; wordBuffer[1] = 'I'; wordBuffer[2] = 'N'; wordBuffer[3] = 'S';
        updateWordDisplay();
        delay(500);
        
        byte hundreds = winStreak/100;
        byte tens = (winStreak - hundreds*100)/10;
        byte ones = winStreak - hundreds*100 - tens*10;
        wordBuffer[0] = '0';
        wordBuffer[1] = hundreds + '0';
        wordBuffer[2] = tens + '0';
        wordBuffer[3] = ones + '0';
        updateWordDisplay();
        delay(500);
        
        // Restore previously typed letters
        wordBuffer[0] = tempSave[0]; wordBuffer[1] = tempSave[1]; wordBuffer[2] = tempSave[2]; wordBuffer[3] = tempSave[3];
        //alpha4.writeDisplay();
      }
      else if(key == ' '){
        // Restart game
        GameState = GAME_START_SCREEN;
        incrInputLoc();
        EEPROM.update(WIN_STREAK, 0); // resets win streak
        delay(4);
      }
      else if(key != 0x0D){ // Else if the enter key wasn't pressed...
        if(digitLoc < 4){ // And if all the digits aren't filled...
          // Add the inputted character into the current digit location then increment the digit location
          wordBuffer[digitLoc] = toupper((char)key);
          
          EEPROM.update(getInputLoc(digitLoc), wordBuffer[digitLoc]);
          delay(4);
          
          digitLoc++;
          
          playSound(3, 900, 95);
        }
      }
      else if (key == 0x0D && digitLoc == 4){ // Else if enter is pressed and the input buffer is full
        playSound(3, 1098, 50); //play sound through speaker
        playSound(3, 1250, 95);
        
        // Store guess into EEPROM
        for (byte i = 0; i < 4; i++) prevWord[0][i] = wordBuffer[i];
        storePrevWord();
        
        // Guess number is 0
        if(isGuessCorrect(0)){
          GameState = GAME_END_SCREEN;
          win = true;
        } 
        else{
          GameState = GAME_INPUT_GUESS2;
          prints("Second Guess...\n");
        }
        
        // Clear input data in EEPROM to represent current cleared display
        for(byte loc = 0; loc < 4; loc++) {
          wordBuffer[loc] = ' ';
          EEPROM.update(getInputLoc(loc), wordBuffer[loc]);
          delay(4);
        }
      }
    }
  }

  else if (GameState == GAME_INPUT_GUESS2) {
    storeGameState();
    
    char key = checkKeyboard(CARDKB_ADDR);
    if(key != NULL){ // If the entered key is not one of the game keys (alphabet, backspace, etc.)...
      enableDisplayLEDs();
      if(key == 0x08){ // If the key is a backspace character...
        if(digitLoc > 0){ // and letters were entered...
          playSound(3, 300, 20); //play sound through speaker
          playSound(3, 300, 20);
          // Delete (replace with whitespace) the previous digit location by first decrementing the digit location
          wordBuffer[--digitLoc] = ' '; 
          
          EEPROM.update(getInputLoc(digitLoc), ' ');
          delay(4);
        }
      }
      else if(key == '.'){ // If the entered key is a space...
        // Save currently typed letters
        char tempSave[4];
        tempSave[0] = wordBuffer[0]; tempSave[1] = wordBuffer[1]; tempSave[2] = wordBuffer[2]; tempSave[3] = wordBuffer[3];
        // Display the win streak
        byte winStreak = EEPROM[WIN_STREAK];
        prints("Win streak: ");
        Serial.println(winStreak);
        wordBuffer[0] = 'W'; wordBuffer[1] = 'I'; wordBuffer[2] = 'N'; wordBuffer[3] = 'S';
        updateWordDisplay();
        delay(500);
        
        byte hundreds = winStreak/100;
        byte tens = (winStreak - hundreds*100)/10;
        byte ones = winStreak - hundreds*100 - tens*10;
        wordBuffer[0] = '0';
        wordBuffer[1] = hundreds + '0';
        wordBuffer[2] = tens + '0';
        wordBuffer[3] = ones + '0';
        updateWordDisplay();
        delay(500);
        
        // Restore previously typed letters
        wordBuffer[0] = tempSave[0]; wordBuffer[1] = tempSave[1]; wordBuffer[2] = tempSave[2]; wordBuffer[3] = tempSave[3];
        //alpha4.writeDisplay();
      }
      else if(key == ','){
        // Save currently typed letters
        char tempSave[4];
        tempSave[0] = wordBuffer[0]; tempSave[1] = wordBuffer[1]; tempSave[2] = wordBuffer[2]; tempSave[3] = wordBuffer[3];
        
        // Save LED values
        CRGB colors[16];
        for(byte cIter = 0; cIter < 16; cIter++) colors[cIter] = leds4x4[cIter];
        
        // Showing guess 1
        for(byte i = 0; i < 4; i++){
          // Show the 1st previous word
          wordBuffer[i] = prevWord[0][i];
          // Highlight leds of previous word
          leds4x4[4 + i] = BLACK_C;
          leds4x4[8 + i] = BLACK_C;
          leds4x4[12 + i] = BLACK_C;
        }
        updateWordDisplay();
        FastLED.show();
        delay(500);
        
        // Restore previously typed letters
        wordBuffer[0] = tempSave[0]; wordBuffer[1] = tempSave[1]; wordBuffer[2] = tempSave[2]; wordBuffer[3] = tempSave[3];
        // Restore previous led values
        for(byte cIter = 0; cIter < 16; cIter++) leds4x4[cIter] = colors[cIter];
        //alpha4.writeDisplay();
      }
      else if(key == ' '){
        // Restart game
        GameState = GAME_START_SCREEN;
        incrInputLoc();
        EEPROM.update(WIN_STREAK, 0); // resets win streak
        delay(4);
      }
      else if(key != 0x0D){ // Else if the enter key wasn't pressed...
        if(digitLoc < 4){ // And if all the digits aren't filled...
          // Add the inputted character into the current digit location then increment the digit location
          wordBuffer[digitLoc] = toupper((char)key);
          
          EEPROM.update(getInputLoc(digitLoc), wordBuffer[digitLoc]);
          delay(4);
          
          digitLoc++;
          
          playSound(3, 900, 95);
        }
      }
      else if (key == 0x0D && digitLoc == 4){ // Else if enter is pressed and the input buffer is full
        playSound(3, 1098, 50); //play sound through speaker
        playSound(3, 1250, 95);
        
        // Store guess into EEPROM
        for (byte i = 0; i < 4; i++) prevWord[1][i] = wordBuffer[i];
        storePrevWord();
        
        // Guess number is 1
        if(isGuessCorrect(1)){
          GameState = GAME_END_SCREEN;
          win = true;
        } 
        else{
          GameState = GAME_INPUT_GUESS3;
          prints("Third Guess...\n");
        }
        
        // Clear input data in EEPROM to represent current cleared display
        for(byte loc = 0; loc < 4; loc++) {
          wordBuffer[loc] = ' ';
          EEPROM.update(getInputLoc(loc), wordBuffer[loc]);
          delay(4);
        }
      }
    }
  }

  else if (GameState == GAME_INPUT_GUESS3) {
    storeGameState();

    char key = checkKeyboard(CARDKB_ADDR);
    if(key != NULL){ // If the entered key is not one of the game keys (alphabet, backspace, etc.)...
      enableDisplayLEDs();
      if(key == 0x08){ // If the key is a backspace character...
        if(digitLoc > 0){ // and letters were entered...
          playSound(3, 300, 20); //play sound through speaker
          playSound(3, 300, 20);
          // Delete (replace with whitespace) the previous digit location by first decrementing the digit location
          wordBuffer[--digitLoc] = ' '; 
          
          EEPROM.update(getInputLoc(digitLoc), ' ');
          delay(4);
        }
      }
      else if(key == '.'){ // If the entered key is a space...
        // Save currently typed letters
        char tempSave[4];
        tempSave[0] = wordBuffer[0]; tempSave[1] = wordBuffer[1]; tempSave[2] = wordBuffer[2]; tempSave[3] = wordBuffer[3];
        // Display the win streak
        byte winStreak = EEPROM[WIN_STREAK];
        prints("Win streak: ");
        Serial.println(winStreak);
        wordBuffer[0] = 'W'; wordBuffer[1] = 'I'; wordBuffer[2] = 'N'; wordBuffer[3] = 'S';
        updateWordDisplay();
        delay(500);
        
        byte hundreds = winStreak/100;
        byte tens = (winStreak - hundreds*100)/10;
        byte ones = winStreak - hundreds*100 - tens*10;
        wordBuffer[0] = '0';
        wordBuffer[1] = hundreds + '0';
        wordBuffer[2] = tens + '0';
        wordBuffer[3] = ones + '0';
        updateWordDisplay();
        delay(500);
        
        // Restore previously typed letters
        wordBuffer[0] = tempSave[0]; wordBuffer[1] = tempSave[1]; wordBuffer[2] = tempSave[2]; wordBuffer[3] = tempSave[3];
        //alpha4.writeDisplay();
      }
      else if(key == ','){
        // Save currently typed letters
        char tempSave[4];
        tempSave[0] = wordBuffer[0]; tempSave[1] = wordBuffer[1]; tempSave[2] = wordBuffer[2]; tempSave[3] = wordBuffer[3];
        
        // Save LED values
        CRGB colors[16];
        for(byte cIter = 0; cIter < 16; cIter++) colors[cIter] = leds4x4[cIter];
        
        // Showing guess 1
        for(byte i = 0; i < 4; i++){
          // Show the 1st previous word
          wordBuffer[i] = prevWord[0][i];
          // Highlight leds of previous word
          leds4x4[4 + i] = BLACK_C;
          leds4x4[8 + i] = BLACK_C;
          leds4x4[12 + i] = BLACK_C;
        }
        updateWordDisplay();
        FastLED.show();
        delay(500);
        
        // Showing guess 2
        for(byte i = 0; i < 4; i++){
          // Show the 1st previous word
          wordBuffer[i] = prevWord[1][i];
          // Highlight leds of previous word
          leds4x4[0 + i] = BLACK_C;
          leds4x4[4 + i] = colors[4 + i];
          leds4x4[8 + i] = BLACK_C;
          leds4x4[12 + i] = BLACK_C;
        }
        updateWordDisplay();
        FastLED.show();
        delay(500);
        
        // Restore previously typed letters
        wordBuffer[0] = tempSave[0]; wordBuffer[1] = tempSave[1]; wordBuffer[2] = tempSave[2]; wordBuffer[3] = tempSave[3];
        // Restore previous led values
        for(byte cIter = 0; cIter < 16; cIter++) leds4x4[cIter] = colors[cIter];
        //alpha4.writeDisplay();
      }
      else if(key == ' '){
        // Restart game
        GameState = GAME_START_SCREEN;
        incrInputLoc();
        EEPROM.update(WIN_STREAK, 0); // resets win streak
        delay(4);
      }
      else if(key != 0x0D){ // Else if the enter key wasn't pressed...
        if(digitLoc < 4){ // And if all the digits aren't filled...
          // Add the inputted character into the current digit location then increment the digit location
          wordBuffer[digitLoc] = toupper((char)key);
          
          EEPROM.update(getInputLoc(digitLoc), wordBuffer[digitLoc]);
          delay(4);
          
          digitLoc++;
          
          playSound(3, 900, 95);
        }
      }
      else if (key == 0x0D && digitLoc == 4){ // Else if enter is pressed and the input buffer is full
        playSound(3, 1098, 50); //play sound through speaker
        playSound(3, 1250, 95);
        
        // Store guess into EEPROM
        for (byte i = 0; i < 4; i++) prevWord[2][i] = wordBuffer[i];
        storePrevWord();
        
        // Guess number is 2
        if(isGuessCorrect(2)){
          GameState = GAME_END_SCREEN;
          win = true;
        } 
        else{
          GameState = GAME_INPUT_GUESS4;
          prints("Last Guess...\n");
        }
        
        // Clear input data in EEPROM to represent current cleared display
        for(byte loc = 0; loc < 4; loc++) {
          wordBuffer[loc] = ' ';
          EEPROM.update(getInputLoc(loc), wordBuffer[loc]);
          delay(4);
        }
      }
    }
  }

  else if (GameState == GAME_INPUT_GUESS4) {
    storeGameState();
    
    char key = checkKeyboard(CARDKB_ADDR);
    if(key != NULL){ // If the entered key is not one of the game keys (alphabet, backspace, etc.)...
      enableDisplayLEDs();
      if(key == 0x08){ // If the key is a backspace character...
        if(digitLoc > 0){ // and letters were entered...
          playSound(3, 300, 20); //play sound through speaker
          playSound(3, 300, 20);
          // Delete (replace with whitespace) the previous digit location by first decrementing the digit location
          wordBuffer[--digitLoc] = ' '; 
          
          EEPROM.update(getInputLoc(digitLoc), ' ');
          delay(4);
        }
      }
      else if(key == '.'){ // If the entered key is a space...
        // Save currently typed letters
        char tempSave[4];
        tempSave[0] = wordBuffer[0]; tempSave[1] = wordBuffer[1]; tempSave[2] = wordBuffer[2]; tempSave[3] = wordBuffer[3];
        // Display the win streak
        byte winStreak = EEPROM[WIN_STREAK];
        prints("Win streak: ");
        Serial.println(winStreak);
        wordBuffer[0] = 'W'; wordBuffer[1] = 'I'; wordBuffer[2] = 'N'; wordBuffer[3] = 'S';
        updateWordDisplay();
        delay(500);
        
        byte hundreds = winStreak/100;
        byte tens = (winStreak - hundreds*100)/10;
        byte ones = winStreak - hundreds*100 - tens*10;
        wordBuffer[0] = '0';
        wordBuffer[1] = hundreds + '0';
        wordBuffer[2] = tens + '0';
        wordBuffer[3] = ones + '0';
        updateWordDisplay();
        delay(500);
        
        // Restore previously typed letters
        wordBuffer[0] = tempSave[0]; wordBuffer[1] = tempSave[1]; wordBuffer[2] = tempSave[2]; wordBuffer[3] = tempSave[3];
        //alpha4.writeDisplay();
      }
      else if(key == ','){
        // Save currently typed letters
        char tempSave[4];
        tempSave[0] = wordBuffer[0]; tempSave[1] = wordBuffer[1]; tempSave[2] = wordBuffer[2]; tempSave[3] = wordBuffer[3];
        
        // Save LED values
        CRGB colors[16];
        for(byte cIter = 0; cIter < 16; cIter++) colors[cIter] = leds4x4[cIter];
        
        // Showing guess 1
        for(byte i = 0; i < 4; i++){
          // Show the 1st previous word
          wordBuffer[i] = prevWord[0][i];
          // Highlight leds of previous word
          leds4x4[4 + i] = BLACK_C;
          leds4x4[8 + i] = BLACK_C;
          leds4x4[12 + i] = BLACK_C;
        }
        updateWordDisplay();
        FastLED.show();
        delay(500);
        
        // Showing guess 2
        for(byte i = 0; i < 4; i++){
          // Show the 1st previous word
          wordBuffer[i] = prevWord[1][i];
          // Highlight leds of previous word
          leds4x4[0 + i] = BLACK_C;
          leds4x4[4 + i] = colors[4 + i];
          leds4x4[8 + i] = BLACK_C;
          leds4x4[12 + i] = BLACK_C;
        }
        updateWordDisplay();
        FastLED.show();
        delay(500);
        
        // Showing guess 3
        for(byte i = 0; i < 4; i++){
          // Show the 1st previous word
          wordBuffer[i] = prevWord[2][i];
          // Highlight leds of previous word
          leds4x4[0 + i] = BLACK_C;
          leds4x4[4 + i] = BLACK_C;
          leds4x4[8 + i] = colors[8 + i];
          leds4x4[12 + i] = BLACK_C;
        }
        updateWordDisplay();
        FastLED.show();
        delay(500);
        
        // Restore previously typed letters
        wordBuffer[0] = tempSave[0]; wordBuffer[1] = tempSave[1]; wordBuffer[2] = tempSave[2]; wordBuffer[3] = tempSave[3];
        // Restore previous led values
        for(byte cIter = 0; cIter < 16; cIter++) leds4x4[cIter] = colors[cIter];
        //alpha4.writeDisplay();
      }
      else if(key == ' '){
        // Restart game
        GameState = GAME_START_SCREEN;
        incrInputLoc();
        EEPROM.update(WIN_STREAK, 0); // resets win streak
        delay(4);
      }
      else if(key != 0x0D){ // Else if the enter key wasn't pressed...
        if(digitLoc < 4){ // And if all the digits aren't filled...
          // Add the inputted character into the current digit location then increment the digit location
          wordBuffer[digitLoc] = toupper((char)key);
          
          EEPROM.update(getInputLoc(digitLoc), wordBuffer[digitLoc]);
          delay(4);
          
          digitLoc++;
          
          playSound(3, 900, 95);
        }
      }
      else if (key == 0x0D && digitLoc == 4){ // Else if enter is pressed and the input buffer is full
        playSound(3, 1098, 50); //play sound through speaker
        playSound(3, 1250, 95);
        
        // Store guess into EEPROM
        for (byte i = 0; i < 4; i++) prevWord[3][i] = wordBuffer[i];
        storePrevWord();
        
        GameState = GAME_END_SCREEN;
        
        // Guess number is 3
        if(isGuessCorrect(3)){
          win = true;
        } 
        else{
          win = false;
        }
        
        // Clear input data in EEPROM to represent current cleared display
        for(byte loc = 0; loc < 4; loc++) {
          wordBuffer[loc] = ' ';
          EEPROM.update(getInputLoc(loc), wordBuffer[loc]);
          delay(4);
        }
      }
    }
  }

  else if (GameState == GAME_END_SCREEN) {
    storeGameState();
    
    if(win){
      wordBuffer[0] = 'Y'; wordBuffer[1] = 'O'; wordBuffer[2] = 'U'; wordBuffer[3] = ' '; 
      updateWordDisplay();
      playSound(3, 1098, 50); //play sound through speaker
      playSound(3, 1250, 95);
      delay(500);
      
      wordBuffer[0] = ' '; wordBuffer[1] = 'W'; wordBuffer[2] = 'I'; wordBuffer[3] = 'N'; 
      updateWordDisplay();
      playSound(3, 1123, 50); //play sound through speaker
      playSound(3, 1320, 95);
      delay(500);
    }
    else{
      wordBuffer[0] = 'L'; wordBuffer[1] = 'O'; wordBuffer[2] = 'S'; wordBuffer[3] = 'E'; 
      updateWordDisplay();
      playSound(3, 800, 100); //play sound through speaker
      delay(500);
      
      for(byte i=0; i < 4; i++) wordBuffer[i] = randWord[i];
      updateWordDisplay();
      playSound(3, 1098, 100); //play sound through speaker
      delay(500);
    }
    
    // Return FSM back to the game start
    if(checkKeyboard(CARDKB_ADDR) != NULL){
      // Shifts input location in EEPROM to next 4 bytes so writes across EEPROM are saturated (only 100,000 writes/byte location)
      incrInputLoc();
      
      enableDisplayLEDs();
      GameState = GAME_BEGIN_PLAY;
      // Increment the win streak if win
      if(win) EEPROM.update(WIN_STREAK, EEPROM[WIN_STREAK] + 1);
      // Set win streak to 0 if lose
      else EEPROM.update(WIN_STREAK, 0);
    }
  }

  else {
    prints("ERROR: I should not be here. Game State Error. Resetting Game State to Title Screen.\n");
    GameState = GAME_START_SCREEN;
  }
  //------------------------------------------//
  //  END FINITE STATE MACHINE FOR GAME LOGIC //
  //------------------------------------------//
}

// INTERUPT FUNCTIONS //
inline void lowPowerMode(void) {
  //Serial.println("Powering down for 5 seconds...");
  //RTCAVRZero.enableAlarm(5, false);
  //RTCAVRZero.attachInterrupt(wakeUp);
  //delay(50);
  //LowPower.standby();
}

inline void enableDisplayLEDs(void) {
  prints("Updating LEDs...\n");
  //GameState = GAME_START_SCREEN;
  DisplayFlags = ON_DISPLAYS;
  RTCAVRZero.disableAlarm();

  // Set alarm to dim displays after 60 seconds
  RTCAVRZero.enableAlarm(60, false);
  RTCAVRZero.attachInterrupt(dimDisplayLEDs);
}

inline void dimDisplayLEDs(void) {
  prints("Dimming LEDs...\n");
  DisplayFlags = DIM_DISPLAYS;
  RTCAVRZero.disableAlarm();

  // Set alarm to turn off displays after 60 seconds
  RTCAVRZero.enableAlarm(60, false);
  RTCAVRZero.attachInterrupt(disableDisplayLEDs);
}

inline void disableDisplayLEDs(void) {
  prints("Disabling LEDs...\n");
  DisplayFlags = OFF_DISPLAYS;
  RTCAVRZero.disableAlarm();

  // DEBUG: Set alarm to turn on displays after 10 seconds
  // RTCAVRZero.enableAlarm(10, false);
  // RTCAVRZero.attachInterrupt(enableDisplayLEDs);
}