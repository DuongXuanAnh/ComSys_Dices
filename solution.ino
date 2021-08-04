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

constexpr int diceType[] { 4, 6, 8, 12, 20, 100 };
constexpr int diceType_Count = sizeof(diceType)/sizeof(diceType[0]);

enum Mode {normal, configuration};

Mode actualMode = configuration;

//---------------------------------------------------------------------------------
struct Dice{
  
  int index = 0;
  int throwTimes = 1;
  int maxThrowTimes = 9;

  int getType(){
    if(index == diceType_Count - 1){
       index = 0;
       return diceType[0];
    }
    return diceType[++index];
  }

  int getThrowTimes(){
    
    if(throwTimes == maxThrowTimes){
      throwTimes = 0;
    }
    return ++throwTimes;
  }

  int randomResultOneRound(){
     return 1 + rand() % diceType[index];
  }

  int resultSum(){

      int sum = 0;

      for(int i = 0; i < throwTimes; i++){
        sum += randomResultOneRound();     
      }

     return sum;
  }
  
} dice;

//----------------------------------------------------------------------------------

const unsigned long periodicDelay = 100;
unsigned long timerDelay = 0;

struct Button1{
  
    int lastState = ON;
    int currentState;
    
    void Press(){
      currentState = digitalRead(button1_pin);  
         if(currentState == ON){  
          if(actualMode == configuration){
            actualMode = normal;
            Serial.println("Change to normal");
          }
          if(actualMode == normal){
            if((unsigned long) millis() - timerDelay >= periodicDelay){
                Serial.println(dice.resultSum());
              timerDelay = millis();
            }
            
           
          }
       }
    }
   
    
} button1;

struct Button2{
  
    int lastState = ON;
    int currentState;

    void Press(){
      currentState = digitalRead(button2_pin);
       if(lastState == OFF && currentState == ON){    
           if(actualMode == configuration){
            Serial.println(dice.getThrowTimes());
          }  
          if(actualMode == normal){
            Serial.println("Change to configuration mode");
            actualMode = configuration;
          }
       }
        lastState = currentState;
    }
   
    
} button2;

struct Button3{
  
    int lastState = ON;
    int currentState;

    void Press(){
      currentState = digitalRead(button3_pin);
       if(lastState == OFF && currentState == ON){ 
         
          if(actualMode == configuration){
            Serial.println(dice.getType());
          }  
          if(actualMode == normal){
            Serial.println("Change to configuration mode");
            actualMode = configuration;
          }
         
       }
        lastState = currentState;
    }
    
} button3;
//----------------------------------------------------------------------------------

struct Buttons {
  
    void SetUp(){
       for (int i = 0; i < buttonPinsCount; ++i) {
          pinMode(buttonPins[i], INPUT);
       }
    }

    void Handler(){
      button1.Press();
      button2.Press();
      button3.Press();
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

//-------------------------------------------------------------------

void setup() {
    Serial.begin(9600);
    buttons.SetUp();
    ourDisplay.SetUp();
}

void loop() {
  buttons.Handler();
}
