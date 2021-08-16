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

constexpr byte LETTER_D = 0b10100001;   // d

//----------------------------------------------------------------------------------

constexpr int diceType[] { 4, 6, 8, 12, 20, 100 };
constexpr int diceType_Count = sizeof(diceType)/sizeof(diceType[0]);

enum Mode {normal, configuration};

Mode actualMode = configuration;

//---------------------------------------------------------------------------------

int result = 0; // Vysledek souctu hodnoty, kt. dostaneme pri hazeni kostky

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

constexpr byte pos_bitmask[] { 8, 4, 2, 1 }; // Pozice 4 znaku od prava (3, 2, 1, 0)

struct Display {

  int numberOfLedActive = 0;
  int ledPosition = 0; // the most right digit on display
  
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

    int exponent(int base, int exponent){
        int result = 1;
        for(int i = 0; i < exponent; i++){
          result *= base;
        }
        return result;
    }

    void DisplayNumber(){
         unsigned long num_to_show = result % exponent(10, ledPosition+1) / exponent(10, ledPosition);
         writeGlyphBitmask(segmentMap[num_to_show], pos_bitmask[ledPosition]);    
    }

    // @tom: udelat drobet lepe, co kdyby se veslo na displej milion mist? 
    void DisplayConfiguration(){
          writeGlyphBitmask(segmentMap[dice.throwTimes], pos_bitmask[3]); // Na 1. pozice zleva je kolik krat mame hazet
          writeGlyphBitmask(LETTER_D, pos_bitmask[2]); // Na 2. pozice zleva je pismeno D

          if(diceType[dice.index] < 10){
            writeGlyphBitmask(segmentMap[diceType[dice.index]], pos_bitmask[1]);
          }
          else if(diceType[dice.index] < 100){
            writeGlyphBitmask(segmentMap[diceType[dice.index]/10], pos_bitmask[1]);
            writeGlyphBitmask(segmentMap[diceType[dice.index]%10], pos_bitmask[0]);
          }
          else if(diceType[dice.index] == 100){
            writeGlyphBitmask(segmentMap[0], pos_bitmask[1]);
            writeGlyphBitmask(segmentMap[0], pos_bitmask[0]);
          }
    }

    void DisplayMode(){
        if(actualMode == configuration){
             DisplayConfiguration();
          }else if(actualMode == normal){
             DisplayNumber();
          }
    }
    
    // @tom: udelat drobet lepe, co kdyby se veslo na displej milion mist? 
    void multiplexing(){
         
          if(actualMode == configuration){
             numberOfLedActive = 4;
          }else if(actualMode == normal){
             if (result < 10) numberOfLedActive = 1;
             else if (result < 100) numberOfLedActive = 2;
             else if (result < 1000) numberOfLedActive = 3;
             else if (result < 10000) numberOfLedActive = 4;
          }

          ledPosition++;
          ledPosition = ledPosition % numberOfLedActive;  
    }

   
   
} ourDisplay;

//-------------------------------------------------------------------

const unsigned long periodicDelay = 50;
unsigned long timerDelay = 0;

struct Button1{
  
    int lastState = ON;
    int currentState;
    
    void Press(){
      currentState = digitalRead(button1_pin);  
         if(currentState == ON){  
          if(actualMode == configuration){
            actualMode = normal;
//            Serial.println("Change to normal");
          }
          if(actualMode == normal){
            if((unsigned long) millis() - timerDelay >= periodicDelay){
//                Serial.println(dice.resultSum());
                result = dice.resultSum();
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
//            Serial.println(dice.getThrowTimes());
              dice.getThrowTimes();
          }  
          if(actualMode == normal){
//            Serial.println("Change to configuration mode");
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
//            Serial.println(dice.getType());
              dice.getType();
          }  
          if(actualMode == normal){
//            Serial.println("Change to configuration mode");
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

void setup() {
//    Serial.begin(9600);
    buttons.SetUp();
    ourDisplay.SetUp();
}

void loop() {
  ourDisplay.multiplexing();
  buttons.Handler();
  ourDisplay.DisplayMode();
}
