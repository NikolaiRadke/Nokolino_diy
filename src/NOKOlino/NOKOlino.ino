/* NOKOlino V1.0 12.03.2018 - Nikolai Radke
 *  
 *  Sketch for Mini-NOKO-Monster
 *  for Attiny45/85 | 8 Mhz - remember to flash your bootloader first!
 *  SoftwareSerial needs 8 MHz to work correctly.
 *  
 *  Flash-Usage: 3.724 (1.8.2 | ATTiny 1.0.2 | Linux X86_64 | ATtiny85)
 *  
 *  Circuit:
 *  1: RST | PB5  free
 *  2: A3  | PB3  free          
 *  3: A2  | PB4  Busy JQ6500 - 8
 *  4: GND        GND
 *  5: D0  | PB0  free
 *  6: D1  | PB1  RX JQ6500   - 9   
 *  7: D2  | PB2  Button      - GND
 *  8: VCC        VCC
 *  
 *  Sleepmodes:
 *  0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
 *  6=1sec, 7=2sec, 8=4sec, 9=8sec
 */

#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

//--------------------------------------------------------------------------------
// Configuation
#define Time         10             // Say something every statistical 10 minutes
#define Volume       20             // Volume 0-30 - 20 is recommended 

//#define Breadboard                // Breadboard or PCB?

// Voice set selection
//#define Set_8MBit_16MBit
#define Set_32MBit
//#define Set_own

#ifdef Set_8MBit_16MBit
  #define Button_event 20           // Last button event number (20.mp3)
  #define Time_event   69           // Last time event number (69.mp3)
#endif
#ifdef Set_32MBit
  #define Button_event 40
  #define Time_event   163
#endif
#ifdef Set_own
  #define Button_event 0
  #define Time_event   0
#endif

#ifdef Breadboard
  #define Offset       0.4          // Battery measuring error
  #define maxInput     50           // Max. value from busy line. 
#else
  #define Offset       0.6
  #define maxInput     0
#endif

// Optional - comment out with // to disable
#define Batterywarning             // NOKOlino gives a warning when battery is low
//---------------------------------------------------------------------------------

// Optional battery warning
#define minCurrent   3.50 +Offset   // Low power warning current + measuring error
#define battLow      3.30 +Offset   // Minimal voltage before JQ6500 fails

// Hardware pins
#define TX      0
#define RX      1
#define Busy    2

// ADC and BOD
#ifndef cbi
  #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
  #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
#define BODS 7                       // BOD sleep bit in MCUCR
#define BODSE 2                      // BOD sleep enable bit in MCUCR

// Variables
uint16_t address,seed;
volatile boolean f_wdt = 1;          // Volatile -> it is an interrupt routine
boolean low=false;

SoftwareSerial mp3(TX,RX);           // TX to D0, RX to D1

int main(void) {

#ifdef Batterywarning
  uint16_t current;
  double vref;
  uint16_t counter=10;               // Check shortly after startup
#endif

init(); 
{
  // Power saving
  MCUCR |= _BV(BODS) | _BV(BODSE);   // Disable brown out detection - default?
  ACSR |= _BV(ACD);                  // Disable analog comparator - default?
  DDRB &= ~(1<<PB2);                 // D2 INPUT
  PORTB |= (1<<PB2);                 // D2 HIGH 
  
  // Start JQ6500
  mp3.begin(9600);
  mp3.write("\x7E\x02\x0C\xEF");     // Reset JQ6500
  setup_watchdog(5);                 // Sleep 500ms
  attiny_sleep();                 
  mp3.write("\x7E\x03\x06");
  mp3.write(Volume);                 // Set volume
  mp3.write("\xEF"); 
  setup_watchdog(3);
  attiny_sleep();                    // Sleep 128ms
  mp3.write("\x7E\x03\x11\x04\xEF"); // No loop
  attiny_sleep();                    // Sleep 128ms
  mp3.write("\x7E\x02\x0A\xEF");     // Sleep, JQ6500!

  // Randomize number generator
  address=eeprom_read_word(0);       // Read EEPROM address
  if ((address<2) || (address>(EEPROM.length()-3)))             
  {                                  // Initialize EEPROM and size for first use or after end of cycle
    address = 2;                     // Starting address
    eeprom_write_word(0,address);    // Write starting address
    eeprom_write_word(address,0);    // Write seed 0
  }
  seed=eeprom_read_word(address);    // Read seed
  if (seed>900)                      // After 900 write-cyles move to another address
  {                                  // to keep the EEPROM alive
    seed=0;
    address+=2;
    eeprom_write_word(0,address);
  }
  randomSeed(seed);                  // Randomize
  seed++;                            // New seed
  eeprom_write_word(address,seed);   // Save new seed for next startup
}

// Main loop
while(1)
{
  // Wait for button or time and go to sleep - ~8 times/second         
  if (!low) 
  {
    if (!(PINB & (1<<PB2))) JQ6500_play(random(0,Button_event+1));      // Button event
    else if (random(0,Time*60*8)==1) JQ6500_play(random(Button_event+1,Time_event+1)); // Time event
    else attiny_sleep();
  }
  
  // Check current, if defined
  #ifdef Batterywarning
  if (counter==0)                
  {
    current=MeasureVCC();
    vref=1024*1.1f/(double)current;
    if (vref<=minCurrent)            // Current below minimum
    {
      if (vref<=battLow) low=true;   // Power too low for JQ6500
      else JQ6500_play(Time_event+1);// NOKOLINO says "Beep"
    }
    else low=false;
    counter=400;                     // Every minute, 50x 128ms + some sleeping ms
  }
  counter--;
  #endif
}}

void JQ6500_play(uint8_t f)          // Plays MP3 number f
{
  attiny_sleep();                    // Without pause, pull-up messes the busy signal
  mp3.write("\x7E\x04\x03\x01");     // Play file number f
  mp3.write(f);
  mp3.write("\xEF");
  attiny_sleep();
  while (analogRead(A2)>maxInput) attiny_sleep(); // Check busy
  mp3.write("\x7E\x02\x0A\xEF");     // Go back to sleep, JQ6500!
  attiny_sleep();
}

void attiny_sleep()                  // Sleep to save power
{  
  cbi(ADCSRA,ADEN);                  // Switch ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
  sleep_mode();                                         
  sbi(ADCSRA,ADEN);                  // Switch ADC on
}

void setup_watchdog(uint8_t mode)    // Setup wake time
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

uint16_t MeasureVCC(void)            // Thank you, Tim!
{
  PRR    &=~_BV(PRADC); 
  ADCSRA  =_BV(ADEN)|_BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0); 
  ADMUX   =_BV(REFS2) | 0x0c; 
  _delay_ms(1);  
  ADCSRA  |=_BV(ADSC);
  while (!(ADCSRA&_BV(ADIF))); 
  ADCSRA  |=_BV(ADIF);
  return ADC;
}

ISR(WDT_vect)                       // Set global flag
{
  f_wdt=1; 
}







