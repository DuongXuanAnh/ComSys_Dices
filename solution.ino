#include "funshield.h"

constexpr int buttonPins[] { button1_pin, button2_pin, button3_pin };
constexpr int buttonPinsCount = sizeof(buttonPins) / sizeof(buttonPins[0]);

byte segmentMap[] = {
  0xC0, // 0  0b11000000
  0xF9, // 1  0b11111001
  0xA4, // 2  0b10100100
  0xB0, // 3  0b10110000
  0x99, // 4  0b10011001
  0x92, // 5  0b10010010
  0x82, // 6  0b10000010
  0xF8, // 7  0b11111000
  0x80, // 8  0b10000000
  0x90  // 9  0b10010000
};

//----------------------------------------------------------------------------------
struct Buttons {
  
    void SetUp(){
       for (int i = 0; i < buttonPinsCount; ++i) {
          pinMode(buttonPins[i], INPUT);
       }
    }
    
} buttons;

//----------------------------------------------------------------------------------

struct Display {
  
    void SetUp(){
       pinMode(latch_pin, OUTPUT);
       pinMode(clock_pin, OUTPUT);
       pinMode(data_pin, OUTPUT);
    }

    void writeGlyphBitmask(byte glyph, byte pos_bitmask){
      digitalWrite(latch_pin, LOW);
      shiftOut( data_pin, clock_pin, MSBFIRST, glyph);
      shiftOut( data_pin, clock_pin, MSBFIRST, pos_bitmask);
      digitalWrite(latch_pin, HIGH);
    }
    
} ourDisplay;


void setup() {
    buttons.SetUp();
    ourDisplay.SetUp();
}

void loop() {
  
}
