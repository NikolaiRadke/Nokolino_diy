/* NOKOlino V1.0 12.02.2017 - Nikolai Radke
 *  
 *  Sketch for Mini-NOKO-Monster
 *  for Attiny45/85 | 8 Mhz - remember to flash your bootloader first!
 *  SoftwareSerial needs 8 MHz to work correct.
 *  
 *  Flash-Usage: 3.438 (1.8.1 | ATTiny 1.0.2 | Linux X86_64)
 *  
 *  Circuit:
 *  1: RST | PB5  free
 *  2: A3  | PB3  Battery VCC             
 *  3: A2  | PB4  Busy JQ6500 - 8
 *  4: GND        GND
 *  5: D0  | PB0  TX JQ6500   - 10  (unused)
 *  6: D1  | PB1  RX JQ6500   - 9   (1kOhm)
 *  7: D2  | PB2  Button      - GND
 *  8: 5V         5V
 *  
 *  Sleepmodes:
 *  0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
 *  6=1sec, 7=2sec, 8=4sec, 9=8sec
 */

#include "JQ6500_Serial.h"
#include <avr/sleep.h>
#include <avr/interrupt.h>

//-------------------------------------------------------------------------
// Configuation
#define Time    10        // Say something every statistical 10 minutes
#define Volume  30        // Volume 0-30

// Optional - comment out to disable
#define Batterywarning    // Nokolino gives a warning whenn battery is low
//-------------------------------------------------------------------------

// Optional battery warning
#define minV    3.30 // Minimal voltage bevor battery fails

#define Offset  0.10 // Voltage messuring error
#define minCurrent (1023/5)*(minV+Offset)

// Hardware pins
#define MP3TX   0
#define MP3RX   1
#define Busy    2
#define Batt    3

// ADC and BOD
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
#define BODS 7  // BOD sleep bit in MCUCR
#define BODSE 2 // BOD sleep enable bit in MCUCR

// Variables
uint16_t seed;
volatile boolean f_wdt = 1;

JQ6500_Serial mp3(MP3TX,MP3RX); // TX: D0, RX: D1

int main(void) {

#ifdef Batterywarning
  uint16_t current;
  uint16_t counter=0;
#endif

init(); 
{
  // Power saving
  MCUCR |= _BV(BODS) | _BV(BODSE); // Disable brown out detection
  ACSR |= _BV(ACD);  // Disable analog comparator
  DDRB &= ~(1<<PB2); // D2 INPUT
  PORTB |= (1<<PB2); // D2 HIGH 

  // Start JQ6500
  mp3.begin(9600);
  mp3.reset();
  setup_watchdog(5); // Sleep 500ms
  attiny_sleep();
  mp3.setVolume(Volume); // Max. volume
  mp3.setLoopMode(MP3_LOOP_NONE);
  setup_watchdog(3); // Sleep 128ms
  attiny_sleep();
  mp3.sleep();

  // Randomize number generator
  seed=eeprom_read_word(0);    // Read seed
  randomSeed(seed);
  eeprom_write_word(0,seed+1); // Save new seed for next startup
}

// Main loop
while(1)
{
  // Wait for button/time and go to sleep - ~8 times/second
  attiny_sleep(); // Sleep 128ms         
  if (!(PINB & (1<<PB2))) JQ6500_play(random(0,21));      // Button event
  if (random(0,Time*60*8)==1) JQ6500_play(random(21,69)); // Time event

  // Check current, if defined
  #ifdef Batterywarning
  if (counter==50*8) // Every minute, 50x 128ms + some sleeping ms
  {
    current=analogRead(Batt); // Calculate power level from 5 measurements
    for (uint8_t help=4;help>0;help--)
    {
     setup_watchdog(0);
     attiny_sleep();  // Sleep 16ms
     setup_watchdog(3);
     current+=analogRead(Batt);
    }
    if ((current/5)<=minCurrent) JQ6500_play(70);  // NOKOLINO says "Beep"
    counter=0;
  }
  counter++;
  #endif
}}

void JQ6500_play(uint8_t v) // Plays MP3 number v
{
  mp3.playFileByIndexNumber(v);
  while (analogRead(Busy)>50); // Check busy
  setup_watchdog(8);           // Sleep 4sec - let NOKOlino finish his file
  attiny_sleep();
  setup_watchdog(3);
  mp3.sleep();                 // Go bach to sleep, JQ6500!
}

void attiny_sleep() // Sleep to save power
{  
  cbi(ADCSRA,ADEN); // Switch ADC OFF
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
  sleep_enable();
  sleep_mode();                        
  sleep_disable();                     
  sbi(ADCSRA,ADEN); // Switch ADC ON
}

void setup_watchdog(uint8_t mode) // Setup wake time
{
  uint8_t bb;
  bb=mode & 7;
  if (mode > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  MCUSR &= ~(1<<WDRF);
  WDTCR |= (1<<WDCE) | (1<<WDE);
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}

ISR(WDT_vect) // Set global flag
{
  f_wdt=1; 
}

