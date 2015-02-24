#include <Adafruit_CC3000.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <avr/wdt.h>

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIV2); // you can change this clock speed

#define WLAN_SSID       "House of Win"           // cannot be longer than 32 characters!
#define WLAN_PASS       "p3wp3wp3w"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define LEDS 4
#define NUMPIXELS 12

const int32_t ip = cc3000.IP2U32(192, 168, 1, 9);

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDS, NEO_GRB + NEO_KHZ800);

const uint8_t in1 = 6;
const uint8_t in2 = 7;
const uint8_t in3 = 8;
const uint8_t in4 = 9;
//const uint8_t in5 = 11;
//const uint8_t in6 = 12;
//const uint8_t in7 = 13;

// Colors
const uint32_t off = pixels.Color(0,0,0);

// Cycle vals
boolean doCycle = true;
unsigned long cycleDelay = 75;
unsigned long cycleStopTime;
uint32_t cycleColor;
uint8_t cyclePixel = 0;

// Blink vals
boolean doBlink = false;
boolean lastCycleState = false;
unsigned long blinkDelay = 750;
unsigned long blinkStopTime;
uint32_t blinkColor = pixels.Color(0,75,0);
uint8_t blinkStart = 0;
uint8_t blinkEnd = NUMPIXELS;

// Cycle vals
enum InActiveState {
  in,
  out,
  change,
  disable
};


boolean doInActive = false;
float inActiveR;
float inActiveG;
float inActiveB;
uint8_t inActiveCH = 40;

float inActiveRH;
float inActiveGH;
float inActiveBH;
float inActiveRI;
float inActiveGI;
float inActiveBI;

uint8_t inActiveOffset = 90;
//uint8_t inActiveBrightness = 255;
const uint16_t inActiveFadeDelay = 5000;
InActiveState inActiveCurrentState = change;
uint8_t inActiveCurrentOffset = 0;
float inActiveDimInterval = inActiveFadeDelay / inActiveOffset;
//uint8_t inActiveCurrentBrightnessInterval = inActiveBrightness / inActiveOffset;

const unsigned long inActiveDelay = 5000;
unsigned long inActiveStopTime;

// latch inputs
boolean isLatched = true;
unsigned long latchDelay = 4000;
unsigned long latchStopTime;

void setup(){
  Serial.begin(115200);
  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
  
  pinMode(in1, INPUT_PULLUP);
  pinMode(in2, INPUT_PULLUP);
  pinMode(in3, INPUT_PULLUP);
  pinMode(in4, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  //  pinMode(in5, INPUT_PULLUP);
//  pinMode(in6, INPUT_PULLUP);
//  pinMode(in7, INPUT_PULLUP);
//  pinMode(in8, INPUT_PULLUP);
  
  pixels.begin(); // This initializes the NeoPixel library.
  colorWipe(off);
  pixels.show();
  Serial.println(F("Neopixels initialized"));
  
  // Init stop time
  cycleStopTime = millis();
  blinkStopTime = millis();
  
  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  
  connectToWifi();
  colorWipe(off);  
  
  doCycle = false;  
  doBlink = true;
  latchStopTime = millis() + (blinkDelay * 4);
  
  wdt_enable(WDTO_2S);
  Serial.println("setup complete");
}

// Interrupt is called once a millisecond, 
SIGNAL(TIMER0_COMPA_vect) 
{

  if(doInActive){
    inActiveStopTime = inActive(inActiveStopTime);
  }
  
  if(doCycle){
   cycleStopTime = cycle(cycleStopTime); 
  }
  
  if(doBlink){
     blinkStopTime = blinkInterval(blinkStopTime); 
  }

}

void loop(){
  
  if(!isLatched){
    if(digitalRead(in1) == LOW){
     Serial.println(F("in 1 LOW"));
     btnAction("PUT","/twinkle/start","{\"room\":\"Living Room\"}",9,11); // 4,5
    }
    if(digitalRead(in2) == LOW){
     Serial.println(F("in 2 low"));
     btnAction("PUT","/rooms/toggle","{\"room\":\"Living Room\"}",2,6); // 9, 11    
    }
    if(digitalRead(in3) == LOW){
     Serial.println(F("in 3 low")); 
     btnAction("PUT","/fx/clear","{\"room\":\"Living Room\"}",0,11); // 6,7
    }
    if(digitalRead(in4) == LOW){
     Serial.println(F("in 4 low")); 
     btnAction("PUT","/transitions/start/heavy","{\"room\":\"Living Room\"}",7,9); // 1,2     
    }
    if(digitalRead(A2) == LOW){
     Serial.println(F("in A2 low"));
     isLatched = true;
     if(inActiveCurrentState == disable){
       inActiveCurrentState = in; 
       doInActive = true;
       Serial.println(F("Re-enabling LEDs"));
     } else {
       inActiveCurrentState = disable;
       // doInActive is set to off during the off cycle
       Serial.println(F("Disabling LEDs"));
     }
    }
    if(digitalRead(A3) == LOW){
     btnAction("", "", "", 0,2);
     Serial.println(F("in A3 low"));
//////       btnAction("PUT","/fx/clear","{\"room\":\"Living Room\"}",1,2); // 8,9
    }
//    if(digitalRead(in6) == LOW){
//     Serial.println(F("in 6 is low")); 
//    }
//    if(digitalRead(in7) == LOW){
//     Serial.println(F("in 7 low"));
//     btnAction("PUT","/fx/clear","{\"room\":\"Living Room\"}",1,2); // 8,9
//    }
//    if(digitalRead(in8) == LOW){
//     Serial.println(F("in 8 is low")); 
//    }
  }
  
  if(isLatched){
    latchStopTime = latchCheck(latchStopTime);
  }
  
  wdt_reset();
}

unsigned long latchCheck(unsigned long stopTime){
  unsigned long  now = millis();
  
  if(now > stopTime){
    stopTime = millis() + latchDelay;
    // re-enable btns
    isLatched = false;
    // back to in active
    doInActive = true;
    // turn off other fx
    doBlink = false;
    doCycle = false;
    
    Serial.println(F("Unlatching btn"));
  }
  
  return stopTime;
}

unsigned long inActive(unsigned long stopTime){
  unsigned long  now = millis();
  uint8_t bri;
  
  if(now > stopTime){
    switch (inActiveCurrentState) {
      case change:
        inActiveRH = inActiveRandomColor();
        inActiveGH = inActiveRandomColor();
        inActiveBH = inActiveRandomColor();
        
        inActiveRI = inActiveRH / inActiveOffset;
        inActiveGI = inActiveGH / inActiveOffset;
        inActiveBI = inActiveBH / inActiveOffset;
        
        inActiveR = 0;
        inActiveG = 0;
        inActiveB = 0;
        stopTime = millis() + inActiveDimInterval;
        
        inActiveCurrentState = in;
//        Serial.print(F("Inactive color change - R:")); Serial.print(inActiveRH); Serial.print(F(" G: ")); Serial.print(inActiveGH); Serial.print(F(" B: ")); Serial.println(inActiveBH);
        break;
      case in:
        // bri up

        if(inActiveCurrentOffset == inActiveOffset){
          stopTime = millis() + inActiveDelay;
          inActiveCurrentState = out;
          inActiveCurrentOffset = 0;
        } else {
          stopTime = millis() + inActiveDimInterval;
        }
        
        inActiveR = inActiveR + inActiveRI;
        inActiveG = inActiveG + inActiveGI;
        inActiveB = inActiveB + inActiveBI;

//        aura(inActiveR, inActiveG, inActiveB);
        colorWipe(pixels.Color(inActiveR, inActiveG, inActiveB));
        inActiveCurrentOffset++;
//        Serial.print(F("Inactive fade in R:")); Serial.print(inActiveR); Serial.print(F(" G: ")); Serial.print(inActiveG); Serial.print(F(" B: ")); Serial.println(inActiveB);
//        Serial.print(F("Inactive fade in RI:")); Serial.print(inActiveRI); Serial.print(F(" GI: ")); Serial.print(inActiveGI); Serial.print(F(" BI: ")); Serial.println(inActiveBI);        
        break;
      case out:
        // bri down

        if(inActiveCurrentOffset == inActiveOffset){
          stopTime = millis() + inActiveFadeDelay;
          inActiveCurrentState = change;
          inActiveCurrentOffset = 0;
        } else {
          stopTime = millis() + inActiveDimInterval; 
        }
        
        inActiveR = inActiveR - inActiveRI;
        inActiveG = inActiveG - inActiveGI;
        inActiveB = inActiveB - inActiveBI;

        colorWipe(pixels.Color(inActiveR, inActiveG, inActiveB));

        inActiveCurrentOffset++;
//        Serial.print(F("Inactive fade out R:")); Serial.print(inActiveR); Serial.print(F(" G: ")); Serial.print(inActiveG); Serial.print(F(" B: ")); Serial.println(inActiveB);
        break;
      case disable:
        colorWipe(off);
        doInActive = false;
        break;
    }

//    Serial.print(F("Inactive cycle state: ")); Serial.print(inActiveCurrentState); Serial.print(F(" - current offset: ")); Serial.println(inActiveCurrentOffset);
  } else {
      
  }
  
  return stopTime;
}

uint8_t inActiveRandomColor(){
  return random(inActiveCH);
}

unsigned long blinkInterval(unsigned long stopTime){
  unsigned long  now = millis();
  
  if(now > stopTime){
    stopTime = millis() + blinkDelay;
    
    if(!lastCycleState){
      colorSection(blinkColor, blinkStart, blinkEnd);
    } else {
      colorWipe(off);
    }
    
    lastCycleState = !lastCycleState;
  }
  
  return stopTime;
}

unsigned long cycle(unsigned long stopTime){
  unsigned long  now = millis();
  
  if(now > stopTime){
    stopTime = millis() + cycleDelay;
    int prevPixel = cyclePixel - 1;
    int offPixel = prevPixel - 1;
    if(cyclePixel > pixels.numPixels() - 1){
      cyclePixel = 0;
      prevPixel = pixels.numPixels() - 1;
      offPixel = prevPixel - 1;
    }
    if(offPixel < 0){
       offPixel = pixels.numPixels() - 1; 
    }
    pixels.setPixelColor(cyclePixel,cycleColor);
    pixels.setPixelColor(prevPixel, cycleColor / 2);
    pixels.setPixelColor(offPixel, off);
    pixels.show();
    cyclePixel++;
  }
  
  return stopTime;
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c) {
  for(uint16_t i=0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
    
  }
  pixels.show();
}

void aura(uint8_t r, uint8_t g, uint8_t b){
  uint8_t largest = r;
  if(largest < g){
    largest = g;
  }
  if(largest < b){
    largest = b;
  }
  uint8_t whiteTrigger = 0;
  for(uint16_t i=0; i < pixels.numPixels(); i++){
    
//    whiteTrigger = i / r;
    Serial.println(whiteTrigger);
    if(whiteTrigger > 10 ){
      pixels.setPixelColor(i, pixels.Color(largest, largest, largest));
//      whiteTrigger++;
    } else {
      pixels.setPixelColor(i, pixels.Color(r,g,b));
    }
  }
  pixels.show(); 
}

// Fill the dots one after the other with a color
void colorSection(uint32_t c, int start, int last) {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
    if(i >= start && i <= last){
      pixels.setPixelColor(i, c);
    } else {
      pixels.setPixelColor(i, off);
    }
  }
  pixels.show();
}

void btnAction(String verb, String path, String data, int sectionStart, int sectionEnd){
  isLatched = true;
//  doBlink = true;
  doBlink = false;
  lastCycleState = false;
  // disable other fx
//  doCycle = false;
  doCycle = true;
  doInActive = false;
  
  blinkColor = pixels.Color(120,120,0);
  colorWipe(off);
  // Send Api
  apiCall(verb, path, data);
  
  Serial.println(F("btn action completed"));
}


void apiCall(String verb, String path, String data){
//  colorWipe(off);
  
  Serial.println(F("-------------------------------------"));
  Serial.print(F("Attempting to connect to verb: ")); Serial.print(verb);
  Serial.print(F("\r\nUsing path: ")); Serial.print(path);
  Serial.print(F("\r\nWith data: ")); Serial.print(data);
  
  Serial.print(F("\r\ncheckConnected: ")); Serial.print(cc3000.checkConnected()); Serial.println();
  
  if(!cc3000.checkConnected()){
    Serial.println(F("Wifi detected as not connected. Starting up wifi.."));
    connectToWifi();
    Serial.println(F("Wifi all set, continuing with request"));
  }
  Serial.println(F("Sending request to base server."));
  
  // Start blink cycle
//  blinkStopTime = blinkInterval(selectedColor, blinkStopTime);
  
  Adafruit_CC3000_Client client = cc3000.connectTCP(ip, 8080);
  if (client.connected()) {
    Serial.println("Connected!");
    client.println(verb + " "+ path + " HTTP/1.1");
    client.println("Accept: application/json");
    client.println("User-Agent: Arduino-Lamp");
    client.println("Content-Type: application/json");
    client.print("Content-Length: "); client.print(data.length()); client.println();
    client.println("Connection: close");
    client.println();
    
    if(data.length() > 1){
      Serial.println(F("Sending data.."));
      client.println(data);
    }
    
  } else {
    Serial.println(F("Connection failed"));    
    return;
  }
  
  Serial.println(F("-------------------------------------"));
  while (client.connected()) {
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }

  }
  client.close();
  
  Serial.println(F("\n\nDisconnecting"));
  
}
void connectToWifi(){
  
  // Set up the CC3000, connect to the access point, and get an IP address.
  Serial.println(F("\nInitializing..."));
  cycleColor = pixels.Color(50,0,0);
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  cycleColor = pixels.Color(40,10,0);
  
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
  Serial.println(F("Connected!"));
  cycleColor = pixels.Color(30,20,0);
  
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(10);
  }  
  cycleColor = pixels.Color(10,40,0);
  
  while (!displayConnectionDetails()) {
    delay(10);
  }
  
  cycleColor = pixels.Color(0,50,0);
  Serial.println(F("Wifi all setup"));
  
//  blinkStopTime = millis();
}

// Display connection details like the IP address.
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}
