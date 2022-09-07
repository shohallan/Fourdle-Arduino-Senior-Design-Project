/*
Fourdle Main Code
*/

// AVR library to use the sleep modes on the Atmega 2560
#include <avr/sleep.h>

// Low-Power library by rocketscream for ease of use when changing power modes
// https://github.com/rocketscream/Low-Power
#include "LowPower.h"

// Arduino library to use EEPROM
#include <EEPROM.h>

//
#include <Wire.h>

// Adafruit GFX Library - Version: Latest 
#include <Adafruit_GFX.h>

// Adafruit LED Backpack Library - Version: Latest 
#include <Adafruit_LEDBackpack.h>

// Includes word bank data
#include "WordBank.h"

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

// VARIABLES //
// Used for 14-segment LED display
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// Used for transfering words stored in flash (word bank) to RAM
char wordBuffer[10] = "          ";

// Letter incrementor - Keeps track of which letter is currently being typed
byte digitLoc;

// Word Bank //
#define BANK_SIZE 500
const char * const PROGMEM wordBank[BANK_SIZE] = {
  ABLE, ACID, AGED, ALSO, AREA, ARMY, AWAY, 
  BABY, BACK, BALL, BAND, BANK, BASE, BATH, BEAR, BEAT, BEEN, BEER, BELL, BELT, BEST, BILL, BIRD, BLOW, BLUE, BOAT, BODY, BOMB, BOND, BONE, BOOK, BOOM, BORN, BOSS, BOTH, BOWL, BULK, BURN, BUSH, BUSY, 
  CALL, CALM, CAME, CAMP, CARD, CARE, CASE, CASH, CAST, CELL, CHAT, CHIP, CITY, CLUB, COAL, COAT, CODE, COLD, COME, COOK, COOL, COPE, COPY, CORE, COST, CREW, CROP, 
  DARK, DATA, DATE, DAWN, DAYS, DEAD, DEAL, DEAN, DEAR, DEBT, DEEP, DENY, DESK, DIAL, DICK, DIET, DISC, DISK, DOES, DONE, DOOR, DOSE, DOWN, DRAW, DREW, DROP, DRUG, DUAL, DUKE, DUST, DUTY, 
  EACH, EARN, EASE, EAST, EASY, EDGE, ELSE, EVEN, EVER, EVIL, EXIT, 
  FACE, FACT, FAIL, FAIR, FALL, FARM, FAST, FATE, FEAR, FEED, FEEL, FEET, FELL, FELT, FIGS, FILL, FILM, FIND, FINE, FIRE, FIRM, FISH, FIVE, FLAT, FLOW, FOOD, FOOT, FORD, FORM, FORT, FOUR, FREE, FROM, FUEL, FULL, FUND, 
  GAIN, GAME, GATE, GAVE, GEAR, GENE, GIFT, GIRL, GIVE, GLAD, GOAL, GOES, GOLD, GOLF, GONE, GOOD, GRAY, GREW, GREY, GROW, GULF, 
  HAIR, HALF, HALL, HAND, HANG, HARD, HARM, HATE, HAVE, HEAD, HEAR, HEAT, HELD, HELL, HELP, HERE, HERO, HIKE, HILL, HIRE, HOLD, HOLE, HOLY, HOME, HOPE, HOST, HOUR, HUGE, HUNG, HUNT, HURT, 
  IDEA, INCH, INTO, IRON, ITEM, 
  JACK, JANE, JEAN, JOHN, JOIN, JUMP, JURY, JUST, 
  KEEN, KEEP, KENT, KEPT, KICK, KILL, KIND, KING, KNEE, KNEW, KNOW, 
  LACK, LADY, LAID, LAKE, LAND, LANE, LAST, LATE, LEAD, LEFT, LESS, LIFE, LIFT, LIKE, LINE, LINK, LIST, LIVE, LOAD, LOAN, LOCK, LOGO, LONG, LOOK, LORD, LOSE, LOSS, LOST, LOVE, LUCK, 
  MADE, MAIL, MAIN, MAKE, MALE, MANY, MARK, MASS, MATT, MEAL, MEAN, MEAT, MEET, MENU, MERE, MIKE, MILE, MILK, MILL, MIND, MINE, MISS, MODE, MOOD, MOON, MORE, MOST, MOVE, MUCH, MUST, 
  NAME, NAVY, NEAR, NECK, NEED, NEWS, NEXT, NICE, NICK, NINE, NONE, NOSE, NOTE, 
  OKAY, ONCE, ONLY, ONTO, OPEN, ORAL, OVER, 
  PACE, PACK, PAGE, PAID, PAIN, PAIR, PALM, PARK, PART, PASS, PAST, PAVE, PEAR, PINS, PION, PIPE, PLAN, PLAY, PLOT, PLUG, PLUS, POLL, POOL, POOR, PORT, POST, PULL, PURE, PUSH, 
  RACE, RAIL, RAIN, RANK, RARE, RATE, READ, REAL, REAR, RELY, RENT, REST, RICE, RICH, RIDE, RING, RISE, RISK, ROAD, ROCK, ROLE, ROLL, ROOF, ROOM, ROOT, ROSE, RULE, RUSH, RUTH, 
  SAFE, SAID, SAKE, SALE, SALT, SAME, SAND, SAVE, SEAT, SEED, SEEK, SEEM, SEEN, SELF, SELL, SEND, SENT, SEPT, SHIP, SHOP, SHOT, SHOW, SHUT, SICK, SIDE, SIGN, SITE, SIZE, SKIN, SLIP, SLOW, SNOW, SOFT, SOIL, SOLD, SOLE, SOME, SONG, SOON, SORT, SOUL, SPOT, STAR, STAY, STEP, STOP, SUCH, SUIT, SURE, 
  TAKE, TALE, TALK, TALL, TANK, TAPE, TASK, TEAM, TECH, TELL, TEND, TERM, TEST, TEXT, THAN, THAT, THEM, THEN, THEY, THIN, THIS, THUS, TILL, TIME, TINY, TOLD, TOLL, TONE, TONY, TOOK, TOOL, TOUR, TOWN, TREE, TRIP, TRUE, TUNE, TURN, TWIN, TYPE, 
  UNIT, UPON, USED, USER, 
  VARY, VAST, VERY, VICE, VIEW, VOTE, 
  WAGE, WAIT, WAKE, WALK, WALL, WANT, WARD, WARM, WASH, WAVE, WAYS, WEAK, WEAR, WEEK, WELL, WENT, WERE, WEST, WHAT, WHEN, WHOM, WIDE, WIFE, WILD, WILL, WIND, WINE, WING, WIRE, WISE, WISH, WITH, WOOD, WORD, WORE, WORK, 
  YARD, YEAH, YEAR, YOUR, 
  ZERO, ZONE
};


// Used for EEPROM testing
int addr = 0;
byte romValue;

void setup() {
  Serial.begin(9600);
  
  Wire.begin();
  
  // --Test setting up and displaying all segments of 14-seg display-- //
  alpha4.begin(DISP14SEG_ADDR);  // pass in the address for the 14-seg display
  
  alpha4.writeDigitRaw(0, 0xFFFF);
  alpha4.writeDigitRaw(1, 0xFFFF);
  alpha4.writeDigitRaw(2, 0xFFFF);
  alpha4.writeDigitRaw(3, 0xFFFF);
  
  alpha4.writeDisplay();
  
  // --Test storing data into EEPROM-- //
  // Reading a value from EEPROM
  // EEPROM.read(address) [address is an integer value starting from 0] [returns a byte]
  
  //EEPROM.update(0, 2); // updates the value of the EEPROM location [update(address, data)]
  
  digitLoc = 0;
  
  while(addr < 10){
    romValue = EEPROM.read(addr);
    Serial.print(addr);
    Serial.print(": ");
    Serial.print(romValue);
    Serial.println();
    addr += 1;
  }
  delay(100);
  
  randomSeed(analogRead(2)); // randomize using noise from analog pin 2
  

  //LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void loop() {
  // EEPROM reading test //
  // romValue = EEPROM.read(addr);
  // Serial.print(addr);
  // Serial.print(": ");
  // Serial.print(romValue);
  // Serial.println();
  // addr += 1;
  // if(addr >= 512) addr = 0;
  // delay(500);
  
  // Power management test //
  // Serial.println("Powering down...");
  // Serial.println("Waking up in 8 seconds...");
  // //delay(100);
  // LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  // Serial.println("Awake!");
  
  // --Random word generator test-- //
  //int randNum = random(BANK_SIZE);
  //strcpy_P(wordBuffer, (PGM_P)pgm_read_word(&(wordBank[randNum])));
  
  //Serial.print(randNum); Serial.print(": ");
  
  // Print to 14-seg display
  alpha4.writeDigitAscii(0, wordBuffer[0]);
  alpha4.writeDigitAscii(1, wordBuffer[1]);
  alpha4.writeDigitAscii(2, wordBuffer[2]);
  alpha4.writeDigitAscii(3, wordBuffer[3]);
  alpha4.writeDisplay();
  
  //Serial.print(wordBuffer); // print out a random word from the word bank
  //Serial.println();
  
  // --Power Management Test-- //
  //delay(1000);
  //delay(10);
  //LowPower.powerSave(SLEEP_1S, ADC_OFF, BOD_OFF, TIMER2_OFF);
  //LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
  //LowPower.idle(SLEEP_1S, ADC_OFF, TIMER5_OFF, TIMER4_OFF,  TIMER3_OFF,  TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, 
  //    SPI_OFF, USART3_OFF, USART2_OFF, USART1_OFF, USART0_OFF, TWI_ON);
  //delay(2);
  
  // --Keyboard Test-- //
  
  
  Wire.requestFrom(CARDKB_ADDR, 1);
  while (Wire.available()){
    byte key = Wire.read();

    if (key != 0){ // If the keyboard is being pressed...
      if (key > 0x7F){
        Serial.print(key, HEX);
      }
      else{
        Serial.print((char)key);
        if(key == 0x7F && digitLoc > 0){      // If key is a backspace character and the digit location is >0
          wordBuffer[digitLoc-- - 1] = ' ';   // Delete (replace with whitespace) the previous digit location then decrement the digit location
        }
        else if(key != 0x7F && digitLoc < 4){ // Else if not a backspace character and the digit location is <4
          wordBuffer[digitLoc++] = (char)key; // Add the inputted character into the curren digit location then increment the digit location
        }
      }
    }
  }
}
