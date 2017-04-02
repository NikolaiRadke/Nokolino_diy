/* NOKOlino_easy V1.0 02.04.2017 - Nikolai Radke
 *  
 *  Easy sketch for Mini-NOKO-Monster
 *  for Attiny45/85 | 8 Mhz - remember to flash your bootloader first!
 *  SoftwareSerial needs 8 MHz to work correct.
 *  
 *  Flash-Usage: 3.046 (1.8.2 | ATTiny 1.0.2 | Linux X86_64 | ATtiny85)
 *  
 *  Circuit:
 *  1: RST | PB5  free
 *  2: A3  | PB3  free          
 *  3: A2  | PB4  Busy JQ6500 - 8
 *  4: GND        GND
 *  5: D0  | PB0  TX JQ6500   - 10  (unused)
 *  6: D1  | PB1  RX JQ6500   - 9  
 *  7: D2  | PB2  Button      - GND
 *  8: VCC        VCC
 *  
 */

#include <SoftwareSerial.h>

//--------------------------------------------------------------------------------
// Configuation
#define Time        10              // Say something every statistical 10 minutes
#define Volume      30              // Volume 0-30

// Debugging
#define maxInput    10              // Max. value of analogRead from busy line
#define maxBusy     70              // x128mseconds to timeout busy check - 70 = 9s

// Hardware pins
#define TX          0
#define RX          1
#define Busy        2
#define Batt        3

// Variables
unsigned int seed;

SoftwareSerial mp3(TX,RX);           // TX to D0, RX to D1

void setup()
{
  pinMode(2,INPUT);
  digitalWrite(2,HIGH);              // Pull-Up high
  mp3.begin(9600);
  mp3.write("\x7E\x02\x0C\xEF");     // Reset JQ6500
  delay(500);
  mp3.write("\x7E\x03\x06");
  mp3.write(Volume);                 // Set volume
  mp3.write("\xEF"); 
  delay(150);
  mp3.write("\x7E\x02\x0A\xEF");     // Sleep, JQ6500!

  // Randomize number generator
  seed=eeprom_read_word(0);          // Read seed
  randomSeed(seed);
  eeprom_write_word(0,seed+1);       // Save new seed for next startup
}

void loop()
{
  if (!(PINB & (1<<PB2))) JQ6500_play(random(0,21));           // Button event
  else if (random(0,Time*60*8)==1) JQ6500_play(random(21,69)); // Time event
  else delay(128);
}

void check_busy()
{
  byte busy_counter=0;
  delay(120);
  while (analogRead(Busy)>maxInput) // Check busy line
  {
    delay(120);
    if (busy_counter>maxBusy) break; // Check timeout - checking inside while()
    busy_counter++;                  // didn't work well, however.
  }
}

void JQ6500_play(byte v)             // Plays MP3 number v
{
  mp3.write("\x7E\x04\x03\x01");     // Play file number v
  mp3.write(v);
  mp3.write("\xEF");
  check_busy();                      // Check busy line
  mp3.write("\x7E\x02\x0A\xEF");     // Go back to sleep, JQ6500!
}



