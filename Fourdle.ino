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

// VARIABLES //
// Used for 14-segment LED display
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// Used for transfering words stored in flash (word bank) to RAM
char wordBuffer[10];

// Word Bank //
#define BANK_SIZE 500
const char * const PROGMEM wordBank[BANK_SIZE] = {
  able, acid, aged, also, area, army, away, 
  baby, back, ball, band, bank, base, bath, bear, beat, been, beer, bell, belt, best, bill, bird, blow, blue, boat, body, bomb, bond, bone, book, boom, born, boss, both, bowl, bulk, burn, bush, busy, 
  call, calm, came, camp, card, care, Case, cash, cast, cell, chat, chip, city, club, coal, coat, code, cold, come, cook, cool, cope, copy, CORE, cost, crew, crop, 
  dark, data, date, dawn, days, dead, deal, dean, dear, debt, deep, deny, desk, dial, dick, diet, disc, disk, does, done, door, dose, down, draw, drew, drop, drug, dual, duke, dust, duty, 
  each, earn, ease, east, easy, edge, Else, even, ever, evil, Exit, 
  face, fact, fail, fair, fall, farm, fast, fate, fear, feed, feel, feet, fell, felt, file, fill, film, find, fine, fire, firm, fish, five, flat, flow, food, foot, ford, form, fort, four, Free, from, fuel, full, fund, 
  gain, game, gate, gave, gear, gene, gift, girl, give, glad, goal, goes, gold, Golf, gone, good, gray, grew, grey, grow, gulf, 
  hair, half, hall, hand, hang, hard, harm, hate, have, head, hear, heat, held, hell, help, here, hero, high, hill, hire, hold, hole, holy, home, hope, host, hour, huge, hung, hunt, hurt, 
  idea, inch, into, iron, item, 
  jack, jane, jean, john, join, jump, jury, just, 
  keen, keep, kent, kept, kick, kill, kind, king, knee, knew, know, 
  lack, lady, laid, lake, land, lane, last, late, lead, left, less, life, lift, like, line, link, list, live, load, loan, lock, logo, Long, look, lord, lose, loss, lost, love, luck, 
  made, mail, MAIN, make, male, many, Mark, mass, matt, meal, mean, meat, meet, menu, mere, mike, mile, milk, mill, mind, mine, miss, mode, mood, moon, more, most, move, much, must, 
  name, navy, near, neck, need, news, next, nice, nick, nine, none, nose, note, 
  okay, once, only, onto, open, oral, over, 
  pace, pack, page, paid, pain, pair, palm, park, part, pass, past, path, peak, pick, pink, pipe, plan, play, plot, plug, plus, poll, pool, poor, port, post, pull, pure, push, 
  race, rail, rain, rank, rare, rate, read, real, rear, rely, rent, rest, rice, rich, ride, ring, rise, risk, road, rock, role, roll, roof, room, root, rose, rule, rush, ruth, 
  safe, said, sake, sale, salt, same, sand, save, seat, seed, seek, seem, seen, self, sell, send, sent, sept, ship, shop, shot, show, shut, sick, side, sign, site, size, skin, slip, slow, snow, soft, soil, sold, sole, some, song, soon, sort, soul, spot, star, stay, step, stop, such, suit, sure, 
  take, tale, talk, tall, tank, tape, task, team, tech, tell, tend, term, test, text, than, that, them, then, they, thin, THIS, thus, till, Time, tiny, told, toll, TONE, tony, took, tool, tour, town, tree, trip, True, tune, turn, twin, type, 
  unit, upon, used, user, 
  vary, vast, very, vice, view, vote, 
  wage, wait, wake, walk, wall, want, ward, warm, wash, wave, ways, weak, wear, week, well, went, were, west, what, when, whom, wide, wife, wild, will, wind, wine, wing, wire, wise, wish, with, wood, Word, wore, work, 
  yard, yeah, year, your, 
  zero, zone
};


// Used for EEPROM testing
int addr = 0;
byte romValue;

void setup() {
  Serial.begin(9600);
  
  alpha4.begin(0x70);  // pass in the address
  
  // alpha4.writeDigitRaw(0, 0xFFFF);
  // alpha4.writeDigitRaw(1, 0xFFFF);
  // alpha4.writeDigitRaw(2, 0xFFFF);
  // alpha4.writeDigitRaw(3, 0xFFFF);
  
  // alpha4.writeDisplay();
  
  // --Test storing data into EEPROM-- //
  // Reading a value from EEPROM
  // EEPROM.read(address) [address is an integer value starting from 0] [returns a byte]
  
  //EEPROM.update(0, 2); // updates the value of the EEPROM location [update(address, data)]
  
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
  
  // Random word generator test //
  int randNum = random(BANK_SIZE);
  strcpy_P(wordBuffer, (PGM_P)pgm_read_word(&(wordBank[randNum])));
  
  Serial.print(randNum); Serial.print(": ");
  
  Serial.print(wordBuffer); // print out a random word from the word bank
  Serial.println();
  delay(10);
  //LowPower.powerSave(SLEEP_1S, ADC_OFF, BOD_OFF, TIMER2_OFF);
  LowPower.idle(SLEEP_1S, ADC_OFF, TIMER5_OFF, TIMER4_OFF,  TIMER3_OFF,  TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, 
      SPI_OFF, USART3_OFF, USART2_OFF, USART1_OFF, USART0_OFF, TWI_ON);
  delay(2);
}
