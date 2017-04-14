/*
  Nathan Seidle
  SparkFun Electronics 2011
  
  This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
  
  Controlling an LED strip with individually controllable RGB LEDs. This stuff is awesome.
  
  The SparkFun (individually controllable) RGB strip contains a bunch of WS2801 ICs. These
  are controlled over a simple data and clock setup. The WS2801 is really cool! Each IC has its
  own internal clock so that it can do all the PWM for that specific LED for you. Each IC
  requires 24 bits of 'greyscale' data. This means you can have 256 levels of red, 256 of blue,
  and 256 levels of green for each RGB LED. REALLY granular.
 
  To control the strip, you clock in data continually. Each IC automatically passes the data onto
  the next IC. Once you pause for more than 500us, each IC 'posts' or begins to output the color data
  you just clocked in. So, clock in (24bits * 32LEDs = ) 768 bits, then pause for 500us. Then
  repeat if you wish to display something new.
  
  This example code will display bright red, green, and blue, then 'trickle' random colors down 
  the LED strip.
  
  You will need to connect 5V/Gnd from the Arduino (USB power seems to be sufficient).
  
  For the data pins, please pay attention to the arrow printed on the strip. You will need to connect to
  the end that is the begining of the arrows (data connection)--->
  
  If you have a 4-pin connection:
  Blue = 5V
  Red = SDI
  Green = CKI
  Black = GND
  
  If you have a split 5-pin connection:
  2-pin Red+Black = 5V/GND
  Green = CKI
  Red = SDI
 */
int SDI = 2; //Red wire (not the red 5V wire!)
int CKI = 3; //Green wire
int ledPin = 13; //On board LED
const int STRIP_LENGTH = 32; //32 LEDs on this strip
long strip_colors[STRIP_LENGTH];

//colors
const long BRIGHT_RED = 0xAA0000;
const long GREEN = 0x001000;
const long BLUE = 0x000010;
//0xFF0000; //Bright Red
//0x00FF00; //Bright Green
//0x007700; // 1/2 Green
//0x0000FF; //Bright Blue
//0x010000; //Faint red
//0x800000; //1/2 red (0x80 = 128 out of 256)
const long STRIP_COLOR = BRIGHT_RED;

//morse
String morse;
const int DOT_LENGTH = 350;
const int DASH_LENGTH = 3 * DOT_LENGTH;
const int ELEMENT_GAP_LENGTH = 3 * DOT_LENGTH;
const int WORD_GAP_LENGTH = 7 * DOT_LENGTH;
const int LOOP_GAP_LENGTH = 10 * DOT_LENGTH;

void setup() {
  pinMode(SDI, OUTPUT);
  pinMode(CKI, OUTPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  on(0x010101);
}

int new_color;
unsigned long red, green, blue;
unsigned long red_shifted, green_shifted, sum;
void loop() {
  while (Serial.available() > 0) {
    red = Serial.parseInt();
    Serial.println(red);
    green = Serial.parseInt();
    blue = Serial.parseInt();
      new_color = 0;
      red_shifted = red << 16;
      green_shifted = green << 8;
      sum = red_shifted + green_shifted + blue;
      Serial.print("sum ");
      Serial.println(red_shifted + green_shifted + blue);
      Serial.print("red shifted ");
      Serial.println(red_shifted);
      Serial.print("green shifted ");
      Serial.println(green_shifted);
      on(red_shifted + green_shifted + blue);
  }
//  for(char c = 0; c < MESSAGE.length(); c++){
//    if (MESSAGE[c] == ' ') {
//      delay(WORD_GAP_LENGTH);
//    } else {
//      Serial.print(MESSAGE[c]);
//      Serial.print(" ");
//      morse = CHAR_TO_MORSE[MESSAGE[c]];
//      delay(ELEMENT_GAP_LENGTH);
//      for(char i = 0; i < morse.length(); i++){
//        switch (morse[i]){
//          case '.':
//            Serial.print(".");
//            on(STRIP_COLOR);
//            delay(DOT_LENGTH);
//            off();
//            delay(DOT_LENGTH);gf
//            break;
//          case '-':
//            Serial.print("-");
//            on(STRIP_COLOR);
//            delay(DASH_LENGTH);
//            off();
//            delay(DOT_LENGTH);
//            break;
//          default:
//            Serial.println("Invalid morse data");
//        }
//      }
//    }
//    Serial.println(); 
//  }
//  flutter(BLUE);
//  Serial.println();
//  Serial.println("------------------------------");
//  Serial.println();
//  delay(LOOP_GAP_LENGTH);
}

void flutter(long color) {
  for( int i = 0; i < 10; i++ ) {
    on(color);
    off();
  }
}

void on(long color) {
    for(int x = 0 ; x < STRIP_LENGTH ; x++)
      strip_colors[x] = color;
    post_frame();
}

void off(void) {
    for(int x = 0 ; x < STRIP_LENGTH ; x++)
      strip_colors[x] = 0;
    post_frame();
}

//Throws random colors down the strip array
void addRandom(void) {
  int x;
  
  //First, shuffle all the current colors down one spot on the strip
  for(x = (STRIP_LENGTH - 1) ; x > 0 ; x--)
    strip_colors[x] = strip_colors[x - 1];
    
  //Now form a new RGB color
  long new_color = 0;
  for(x = 0 ; x < 3 ; x++){
    new_color <<= 8;
    new_color |= random(0xFF); //Give me a number from 0 to 0xFF
    //new_color &= 0xFFFFF0; //Force the random number to just the upper brightness levels. It sort of works.
  }
  
  strip_colors[0] = new_color; //Add the new random color to the strip
}

//Takes the current strip color array and pushes it out
void post_frame (void) {
  //Each LED requires 24 bits of data
  //MSB: R7, R6, R5..., G7, G6..., B7, B6... B0 
  //Once the 24 bits have been delivered, the IC immediately relays these bits to its neighbor
  //Pulling the clock low for 500us or more causes the IC to post the data.

  for(int LED_number = 0 ; LED_number < STRIP_LENGTH ; LED_number++) {
    long this_led_color = strip_colors[LED_number]; //24 bits of color data

    for(byte color_bit = 23 ; color_bit != 255 ; color_bit--) {
      //Feed color bit 23 first (red data MSB)
      
      digitalWrite(CKI, LOW); //Only change data when clock is low
      
      long mask = 1L << color_bit;
      //The 1'L' forces the 1 to start as a 32 bit number, otherwise it defaults to 16-bit.
      
      if(this_led_color & mask) 
        digitalWrite(SDI, HIGH);
      else
        digitalWrite(SDI, LOW);
  
      digitalWrite(CKI, HIGH); //Data is latched when clock goes high
    }
  }

  //Pull clock low to put strip into reset/post mode
  digitalWrite(CKI, LOW);
  delayMicroseconds(500); //Wait for 500us to go into reset
}

