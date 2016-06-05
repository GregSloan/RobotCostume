#include <FatReader.h>
#include <SdReader.h>
#include <avr/pgmspace.h>
#include "WaveUtil.h"
#include "WaveHC.h"
#include <Wire.h>
 
 
SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the filesystem on the card
FatReader f;      // This holds the information for the file we're play
 
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time
uint8_t prev_wire_value = 0;
uint8_t prev_level = 0;
const char TYFTC[] PROGMEM = "TYFTC.WAV";
const char LEVEL1[] PROGMEM = "LEVEL1.WAV";
const char LEVEL2[] PROGMEM = "LEVEL2.WAV";
const char LEVEL3[] PROGMEM = "LEVEL3.WAV";
const char LEVEL4[] PROGMEM = "LEVEL4.WAV";
const char LEVEL5[] PROGMEM = "LEVEL5.WAV";
const char LEVEL6[] PROGMEM = "LEVEL6.WAV";
const char EFFECT1[] PROGMEM = "EFFECT1.WAV";
const char EFFECT2[] PROGMEM = "EFFECT2.WAV";
const char EFFECT3[] PROGMEM = "EFFECT3.WAV";
const char EFFECT4[] PROGMEM = "EFFECT4.WAV";

const char* const string_table[] PROGMEM = {TYFTC, LEVEL1, LEVEL2, LEVEL3, LEVEL4, LEVEL5, LEVEL6, EFFECT1, EFFECT2, EFFECT3, EFFECT4};
char buffer[12];
bool event_flag = false;

// this handy function will return the number of bytes currently free in RAM, great for debugging!   
int freeRam(void)
{
  extern int  __bss_end; 
  extern int  *__brkval; 
  int free_memory; 
  if((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__bss_end); 
  }
  else {
    free_memory = ((int)&free_memory) - ((int)__brkval); 
  }
  return free_memory; 
} 

void receiveEvent(int howMany) {
  
  prev_wire_value = (int)Wire.read();
  event_flag = true;
  
  
}


void handleEvent() {
  Serial.println("In Handle");
  Serial.println(prev_wire_value);
  event_flag = false;
  
  if (prev_wire_value == 13) {
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[0])));
    playcomplete(buffer);
    
  }

  if (prev_wire_value >= 1 and prev_wire_value < 13) {
    int level = prev_wire_value/2;
    if (level == 0) level = 1;
    if (level != prev_level) {
      strcpy_P(buffer, (char*)pgm_read_word(&(string_table[level])));
      playcomplete(buffer);
      prev_level = level;
    }
  prev_wire_value=0;
}

if (prev_wire_value > 13 && prev_wire_value < 18){
    Serial.println((char*)pgm_read_word(&(string_table[prev_wire_value-7])));
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[prev_wire_value-7])));
    playcomplete(buffer);
  }
}

void sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  putstring("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  putstring(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}
 
void setup() {
  // set up serial port
  Wire.begin(0x8);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
  putstring_nl("WaveHC with 6 buttons");
  
   putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  Serial.println(freeRam());      // if this is under 150 bytes it may spell trouble!
  
  // Set the output pins for the DAC control. This pins are defined in the library
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
 
 
  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)  
    putstring_nl("Card init. failed!");  // Something went wrong, lets print out why
    sdErrorCheck();
    while(1);                            // then 'halt' - do nothing!
  }
  
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);
 
// Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {     // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                             // we found one, lets bail
  }
  if (part == 5) {                       // if we ended up not finding one  :(
    putstring_nl("No valid FAT partition!");
    sdErrorCheck();      // Something went wrong, lets print out why
    while(1);                            // then 'halt' - do nothing!
  }
  
  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(),DEC);     // FAT16 or FAT32?
  
  // Try to open the root directory
  if (!root.openRoot(vol)) {
    putstring_nl("Can't open root dir!"); // Something went wrong,
    while(1);                             // then 'halt' - do nothing!
  }
  
  // Whew! We got past the tough parts.
  putstring_nl("Ready!");
}
 
void loop() {
  if (event_flag) {
    Serial.println("about to handle");
    handleEvent();
  
  }
  
}
 
 
// Plays a full file from beginning to end with no pause.
void playcomplete(char *name) {
  // call our helper to find and play this name
  playfile(name);
  while (wave.isplaying) {
  // do nothing while its playing
  }
  // now its done playing
}
 
void playfile(char *name) {
  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) {
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV"); return;
  }
  
  // ok time to play! start playback
  wave.play();
}

