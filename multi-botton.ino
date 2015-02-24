#include <Adafruit_NeoPixel.h>

#define LEDS 4
#define NUMPIXELS 12
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDS, NEO_GRB + NEO_KHZ800);

unsigned long timer = 0;
unsigned long gap = 1000;
int i = 0;

boolean isOn = false;

void setup(){
  pixels.begin(); // This initializes the NeoPixel library.
}

void loop(){

  if(timer + gap < millis()){
     
    if(isOn){
      pixels.setPixelColor(i, pixels.Color(0,0,0)); 
    } else {
      pixels.setPixelColor(i, pixels.Color(0,0,100)); 
    }
     isOn = !isOn;
     pixels.show();
     timer = millis();
     
//     i++;
//     if(i > pixels.numPixels()){
//      i=0;
//      timer=0;
//     }
  }
}

void startTimer(){
   timer = millis(); 
}

void cycle(uint32_t c, uint8_t wait) {

  for (int i=0; i < pixels.numPixels(); i=i++) {
      int px = i;
      pixels.setPixelColor(px, c);    //turn every third pixel on
//        Serial.print(F("current:")); Serial.print(px); Serial.println();
      
      int tail = px - 1;        
      if(tail < 0){
        tail = pixels.numPixels() - 1;
      }
      int off = px -2;
      if(off < 0){
        off = pixels.numPixels() + off;
      }

//        Serial.print(F("last:")); Serial.print(last); Serial.println();
      pixels.setPixelColor(tail, c/6);        //turn every third pixel off
      pixels.setPixelColor(off, 0);        //turn every third pixel off
      
      pixels.show();
    }
  }
