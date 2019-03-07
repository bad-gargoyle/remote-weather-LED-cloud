/*

# remote-weather-LED-cloud
<B>Weather LED (WS2812B strip) cloud based on Wemos D1 (ESP8266) controlled remotely via HTTP url over WiFi</B>

Project is merged work of bitluni (https://www.youtube.com/watch?v=7Dv70ci-MOw) and Ned Shelton (https://www.youtube.com/watch?v=hgXHqO7rQH8).
Most of the following code is strictly their work. And I have to say, that both projects are great (Gentlemen, my thanks and much appreciation),
but I missed the combination of a variety of lightning efects with the ability to controll it by a web server, so I decided to combine both projects
into one and upload it onto Wemos D1 with intergated WiFi module. Although I am not strong in C ++ (I played with it a few years ago, so forgive
not-so-clean-code or shortcomings), the effect is at least satisfactory (at least for me) and what is most important, I can now integrate the
"weather cloud" with my OpenHAB. ;-) Why? To control it via Astro (https://www.openhab.org/addons/bindings/astro/) for example and display current,
or maybe incoming weather conditions on it.

*/
// Poczatek WiFi
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <Adafruit_NeoPixel.h>

#ifndef STASSID
#define STASSID "<your_wifi_ssid>"
#define STAPSK  "<pass_to_your_wifi>"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

const int led = 13;

#define PIN D4
#define PIN2 D3
#define ledCount 60

Adafruit_NeoPixel strip = Adafruit_NeoPixel(ledCount, PIN, NEO_GRB + NEO_KHZ800); //first number controls the amount of pixels you have (add 4 so the drip falls off the edge)
//Adafruit_NeoPixel stripOld = Adafruit_NeoPixel(ledCount, 7, NEO_GRB + NEO_KHZ800);
//Adafruit_Neopixel stripNew = Adafruit_NeoPixel(ledCount, 8, NEO_GRB + NEO_KHZ800);
int oldR[ledCount];
int oldG[ledCount];
int oldB[ledCount];

int newR[ledCount];
int newG[ledCount];
int newB[ledCount];
int nmax = 255;

//---LED FX VARS
int idex = 0;                //-LED INDEX (0 to LED_COUNT-1
int ihue = 150;                //-HUE (0-255)
int ibright = 50;             //-BRIGHTNESS (0-255)
int isat = 240;                //-SATURATION (0-255)
int bouncedirection = 0;     //-SWITCH FOR COLOR BOUNCE (0-1)
float tcount = 0.0;          //-INC VAR FOR SIN LOOPS
int lcount = 0;              //-ANOTHER COUNTING VAR
int TOP_INDEX = ledCount/2;
int BOTTOM_INDEX = 0;
int thissat = 255;
int EVENODD = ledCount%2;
int timee = 0;
int sunnyBrightness = 200;

//long timee = 12;

void handleRoot() {
  String message = "<html><meta charset='utf-8'><head></head><body style='font-family: sans-serif; font-size: 12px'><B>Dostepne tryby swiecenia:</B><br><br>";
  message += "<a href='/ledoff'>/ledoff</a> - LEDs OFF<br><br>";
  message += "<a href='/sunrise'>/sunrise</a> - Sunrise<br>";
  message += "<a href='/sunset'>/sunset</a> - Sunset<br>";
  message += "<a href='/daysunny'>/daysunny</a> - Sunny day<br>";
  message += "<a href='/overcast'>/overcast</a> - Cloudy day<br>";
  message += "<a href='/daystormy'>/daystormy</a> - Stormy day<br>";
  message += "<a href='/daycold'>/daycold</a> - Cold day<br>";
  message += "<a href='/nightclear'>/nightclear</a> - Quiet night<br>";
  message += "<a href='/nightstormy'>/nightstormy</a> - Stormy night<br><br>";
  message += "<a href='/wildfire'>/wildfire</a> - Flames v1<br>";
  message += "<a href='/wildfire2'>/wildfire2</a> - Flames v2<br>";
  message += "<a href='/flame'>/flame</a> - Flames v3<br>";
  message += "<a href='/rainbow'>/rainbow</a> - Rainbow<br>";
  message += "<a href='/rainbowcycle'>/rainbowcycle</a> - Reinbow cycle<br>";
  message += "<a href='/colorcycle'>/colorcycle</a> - Color cycle<br><br>";
  message += "<a href='/rave'>/rave</a> - Mad lights<br>";
  message += "<a href='/randomburst'>/randomburst</a> - Rendom burst of color<br>";
  message += "<a href='/rgbpropeller'>/rgbpropeller</a> - RGB snake<br>";
  message += "<a href='/emslightsstrobe'>/emslightsstrobe</a> - EMS Light Strobe<br>";
  message += "<a href='/onecolor'>/onecolor</a> - Single color<br>";
  server.send(200, "text/html", message);
}

// Dopisac jak uzywac,
// wskazc budowe url dla onecolor

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}
// Koniec WiFi

void setup() {
  // WiFi poczatek
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/ledoff", []() {
    server.send(200, "text/plain", "ledoff");
    timee = 0;
  });

  server.on("/sunrise", []() {
    server.send(200, "text/plain", "sunrise");
    timee = 1;
  });

  server.on("/sunset", []() {
    server.send(200, "text/plain", "sunset");
    timee = 2;
  });

  server.on("/daysunny", []() {
    server.send(200, "text/plain", "daysunny");
    timee = 3;
  });

  server.on("/overcast", []() {
    server.send(200, "text/plain", "overcast");
    timee = 4;
  });

  server.on("/daystormy", []() {
    server.send(200, "text/plain", "daystormy");
    timee = 5;
  });

  server.on("/daycold", []() {
    server.send(200, "text/plain", "daycold");
    timee = 6;
  });

  server.on("/nightclear", []() {
    server.send(200, "text/plain", "nightclear");
    timee = 7;
  });

  server.on("/nightstormy", []() {
    server.send(200, "text/plain", "nightstormy");
    timee = 8;
  });

  server.on("/wildfire", []() {
    server.send(200, "text/plain", "wildfire");
    timee = 9;
  });

  server.on("/wildfire2", []() {
    server.send(200, "text/plain", "wildfire2");
    timee = 10;
  });

  server.on("/flame", []() {
    server.send(200, "text/plain", "flame");
    timee = 11;
  });

  server.on("/rainbow", []() {
    server.send(200, "text/plain", "rainbow");
    timee = 12;
  });

  server.on("/rainbowcycle", []() {
    server.send(200, "text/plain", "rainbowcycle");
    timee = 13;
  });

  server.on("/colorcycle", []() {
    server.send(200, "text/plain", "colorcycle");
    timee = 14;
  });

  server.on("/rave", []() {
    server.send(200, "text/plain", "rave");
    timee = 15;
  });

  server.on("/randomburst", []() {
    server.send(200, "text/plain", "randomburst");
    timee = 16;
  });

  server.on("/rgbpropeller", []() {
    server.send(200, "text/plain", "rgbpropeller");
    timee = 17;
  });

  server.on("/emslightsstrobe", []() {
    server.send(200, "text/plain", "emslightsstrobe");
    timee = 18;
  });

  //Dodac wiecej (podstawowych) kolorow jako tryby
  server.on("/onecolor", []() {
    server.send(200, "text/plain", "onecolor");
    timee = 19;
  });
  
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  //WiFi koniec
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  randomSeed(analogRead(0)); //EXTRA RANDOMNESS!!!!!
}

void loop() {
  boolean RunMenuMode = true;

  if(RunMenuMode){
    //long timee = millis();
    //timee /= 15000;

    server.handleClient();
    MDNS.update();
        
    //switch (timee % 17){
    switch (timee){
      case 0:
        one_color_RGB(0,0,0);
        break;
      case 1:
        Sunrise();
        break;
      case 2:
        Sunset();
        break;
      case 3:
        Day_Sunny();
        break;
      case 4:
        Overcast();
        break;
      case 5:
        Day_Stormy();
        break;
      case 6:
        Day_Cold();
        break;
      case 7:
        Night_Clear();
        break;
      case 8:
        Night_Stormy();
        break;
      case 9:
        Wildfire();
        break;
      case 10:
        Wildfire2();
        break;
      case 11:
        flame();
        break;
      case 12:
        Rainbow();
        break;
      case 13:
        //rainbowCycle(<time_to_full_cycle>);
        rainbowCycle(5);
        break;
      case 14:
        ColorCycle();
        break;
      case 15:
        Rave();
        break;
      case 16:
        random_burst();
        break;
      case 17:
        rgb_propeller();
        break;
      case 18:
        ems_lightsSTROBE();
        break;
      case 19:
        //one_color_RGB(150,0,255);
        one_color_RGB(0,0,0);
        break;
      default:
        one_color_RGB(0,0,0);
    }
  }
    else{ // Set RunMenuMode na "false" to run below functions directly
      //Sunrise();
      //Sunset();
      //Day_Sunny();
      //Day_Stormy();
      //Day_Cold();
      //Night_Clear();
      //Night_Stormy();
      //Overcast();
      //Wildfire();
      //Wildfire2();
      //Rainbow();
      //rainbowCycle(5);
      //ColorCycle();
      //flame();
      //Rave();
      //random_burst();
      //rgb_propeller();
      //ems_lightsSTROBE();
      //one_color_RGB(150,0,255);
    }
  }

void lightning(int length, int hue, int sat){ //200,240,128
  for(int i = 0; i<length;i++){
    flicker(hue, sat);
    strip.show();
  }
}
  
//============================EFFECTS===========================================================

void flicker(int thishue, int thissat) {
  int random_bright = random(0,255);
  int random_delay = random(10,100);
  int random_bool = random(random_bright);
  if (random_bool < 10) {
    delay(random_delay);
    for(int i = 0 ; i < ledCount; i++ ) {
      setHSV(i, thishue, thissat, random_bright); 
    }
  }
}

void one_color_RGB(char R, char G, char B){
  for(int i = 0; i < ledCount; i++){
    strip.setPixelColor(i,R,G,B);
  }
  strip.show();
}

void flame() {
  int idelay = random(0,35);
  float hmin = 0.1; float hmax = 45.0;
  float hdif = hmax-hmin;
  int randtemp = random(0,3);
  float hinc = (hdif/float(TOP_INDEX))+randtemp;
  int ihue = hmin;
  for(int i = 0; i <= TOP_INDEX; i++ ) {
    ihue = ihue + hinc;
    //leds[i] = CHSV(ihue, thissat, 255);
    setHSV(i,ihue,thissat,30);
    int ih = horizontal_index(i);    
    //leds[ih] = CHSV(ihue, thissat, 255);    
    setHSV(ih,ihue,thissat,30);
    //leds[TOP_INDEX].r = 255; leds[TOP_INDEX].g = 255; leds[TOP_INDEX].b = 255;  
    //strip.setPixelColor(TOP_INDEX, 255,255,255);              //Don't need no white pixel on top
    strip.show();    
    delay(idelay);
  }
}

void rgb_propeller() {
  idex++;
  int thishue = 80;
  int thisdelay = 100;
  int ghue = (thishue + 80) % 255;
  int bhue = (thishue + 160) % 255;
  int N3  = int(ledCount/3);
  int N6  = int(ledCount/6);  
  int N12 = int(ledCount/12);  
  for(int i = 0; i < N3; i++ ) {
    int j0 = (idex + i + ledCount - N12) % ledCount;
    int j1 = (j0+N3) % ledCount;
    int j2 = (j1+N3) % ledCount;    
    //leds[j0] = CHSV(thishue, thissat, 255);
    setHSV(j0,thishue,thissat,255);
    //leds[j1] = CHSV(ghue, thissat, 255);
    setHSV(j1,ghue,thissat,255);
    //leds[j2] = CHSV(bhue, thissat, 255);
    setHSV(j2,bhue,thissat,255);    
  }
  strip.show();    
  delay(thisdelay);  
}


void ems_lightsSTROBE() {
  int thishue = 0;
  int thathue = (thishue + 160) % 255;
  int thisdelay = 100;
  for(int x = 0 ; x < 5; x++ ) {
    for(int i = 0 ; i < TOP_INDEX; i++ ) {
        //leds[i] = CHSV(thishue, thissat, 255);
        setHSV(i,thishue,thissat,255);
    }
    strip.show(); delay(thisdelay); 
    one_color_RGB(0, 0, 0);
    strip.show(); delay(thisdelay);
  }
  for(int x = 0 ; x < 5; x++ ) {
    for(int i = TOP_INDEX ; i < ledCount; i++ ) {
        //leds[i] = CHSV(thathue, thissat, 255);
        setHSV(i,thathue,thissat,255);
    }
    strip.show(); delay(thisdelay);
    one_color_RGB(0, 0, 0);
    strip.show(); delay(thisdelay);
  }
}


void random_burst() {
  int idex = random(0, ledCount);
  int ihue = random(0, 255);  
  //leds[idex] = CHSV(ihue, thissat, 255);
  setHSV(idex,ihue,200,50);
  strip.show();
  delay(100);
}


void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*10; j++) { // 10 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {        //wheel
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

//============================================Helpers===============================================

word wordMap(word x, word in_min, word in_max, word out_min, word out_max)//word is an unsigned int. Is this faster than longs? maybe
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setHSV(int led, unsigned int hue, byte sat, byte val)            //Set led by hue, saturation, value
{
        unsigned char r,g,b;
        unsigned int H_accent = hue/60;
        unsigned int bottom = ((255 - sat) * val)>>8;
        unsigned int top = val;
        unsigned char rising  = ((top-bottom)  *(hue%60   )  )  /  60  +  bottom;
        unsigned char falling = ((top-bottom)  *(60-hue%60)  )  /  60  +  bottom;

        switch(H_accent) {
        case 0:
                r = top;
                g = rising;
                b = bottom;
                break;

        case 1:
                r = falling;
                g = top;
                b = bottom;
                break;

        case 2:
                r = bottom;
                g = top;
                b = rising;
                break;

        case 3:
                r = bottom;
                g = falling;
                b = top;
                break;

        case 4:
                r = rising;
                g = bottom;
                b = top;
                break;

        case 5:
                r = top;
                g = bottom;
                b = falling;
                break;
        }
        strip.setPixelColor(led, r, g, b);
}

void setAllHSV(unsigned int  h, byte  s, byte  v)        //Set all leds to one HSV
{
  for (int i = 0; i< ledCount;i++)
  {
    setHSV(i, h, s, v);
  }
}

//---FIND ADJACENT INDEX CLOCKWISE
int adjacent_cw(int i) {
  int r;
  if (i < ledCount - 1) {r = i + 1;}
  else {r = 0;}
  return r;
}

//---FIND ADJACENT INDEX COUNTER-CLOCKWISE
int adjacent_ccw(int i) {
  int r;
  if (i > 0) {r = i - 1;}
  else {r = ledCount - 1;}
  return r;
}

//---FIND INDEX OF HORIZONAL OPPOSITE LED
int horizontal_index(int i) {
  //-ONLY WORKS WITH INDEX < TOPINDEX
  if (i == BOTTOM_INDEX) {return BOTTOM_INDEX;}
  if (i == TOP_INDEX && EVENODD == 1) {return TOP_INDEX + 1;}
  if (i == TOP_INDEX && EVENODD == 0) {return TOP_INDEX;}
  return ledCount - i;  
}
