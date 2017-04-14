/*
  RFID lock light
  Jon Conley
  This code was adapted from Nathan Seidle's example code for interacting with the WS2801 and A_CAVIS' hookup guide.
   * https://github.com/sparkfun/WS2812_Breakout/blob/V_1.1b/Firmware/SparkFun_LED_Strip_Example/SparkFun_LED_Strip_Example.ino
   * https://learn.sparkfun.com/tutorials/sparkfun-rfid-starter-kit-hookup-guide/example-project
*/
#include <SoftwareSerial.h>

// Changeable configuration
const int dsa = 3;
const long STRIP_COLOR = 0xffffff;
const int DURATION_IN_SECONDS = 3;

// WS2801 Light strip configuration
int SDI = 7; //Red wire (not the red 5V wire!)
int CKI = 6; //Green wire
const int STRIP_LENGTH = 32; //32 LEDs on this strip
long strip_colors[STRIP_LENGTH];

int ledPin = 13; //On board LED

SoftwareSerial rSerial(8, 999); // RX, TX
//SoftwareSerial sensor2Serial(7, 8); // RX, TX
// For SparkFun's tags, we will receive 16 bytes on every
// tag read, but throw four away. The 13th space will always
// be 0, since proper strings in Arduino end with 0

// These constants hold the total tag length (tagLen) and
// the length of the part we want to keep (idLen),
// plus the total number of tags we want to check against (kTags)
const int tagLen = 16;
const int idLen = 13;
const int kTags = 1;
unsigned long left_time = 0;
unsigned long right_time = 0;
unsigned long time_diff = 0;

// Put your known tags here!
char knownTags[kTags][idLen] = {
   "82003C873B02"
};

char knownLeftTags[kTags][idLen] = {
  "82003C6FEA3B"
};

// Empty array to hold a freshly scanned tag
char newTag[idLen];
bool leftTag;
bool keyPresent = false;

void setup() {
   Serial.println("Setup"); // Debug
   // setup light strip
   pinMode(SDI, OUTPUT);
   pinMode(CKI, OUTPUT);
   pinMode(ledPin, OUTPUT);
   off();
//   on(0xffff00);
//   delay(2000);
//   on(0xff0000);
//   delay(2000);
//   on(0x00ff00);
//   delay(2000);
//   on(0x0000ff);
//   delay(2000);
  
   // Start the hardware and software serial ports
   Serial.begin(9600);
   rSerial.begin(9600);
}

int new_color;
unsigned long red, green, blue;
unsigned long red_shifted, green_shifted, sum;
bool left_tag_read = false;
bool right_tag_read = false;
void loop() {
  // This makes sure the whole tag is in the serial buffer before
  // reading, the Arduino can read faster than the ID module can deliver!
  if (rSerial.available() == tagLen) { 
    leftTag = true;
    read_tag_rSerial(newTag);
  }
  
  if (Serial.available() == tagLen) { 
    leftTag = false;
    read_tag(newTag);
  }

  if (strlen(newTag)== 0) {
    return;
  } else {
    int left = 0;
    int right = 0;
    for (int ct=0; ct < kTags; ct++){
      left += checkTag(newTag, knownLeftTags[ct]);
      if (left > 0) {
        left_tag_read = true;
      }
      right += checkTag(newTag, knownTags[ct]);
      if (right > 0) {
        right_tag_read = true;
      }
    }
    
    if (leftTag && left_tag_read) {
        on(0x00ff00);
        delay(1000);
        on(0x0000ff);
        delay(1000);
        off();
        keyPresent = true;
        left_time = millis();
    } else if (!leftTag && right_tag_read) {
        on(0xffff00);
        delay(1000);
        on(0xff0000);
        delay(1000);
        off();
        keyPresent = true;
        right_time = millis();
    } else {
      flutter();
    }
    
    if (left_tag_read == true) {
          Serial.println("Known left tag read");
    } else if (right_tag_read == true) {
          Serial.println("Known right tag read");      
    } else {
      if (leftTag) {
        Serial.print("Unknown tag! "); // Debug
      } else {
        Serial.print("Unknown tag! "); // Debug
      }
        // This prints out unknown cards so you can add them to your knownTags as needed
        
        Serial.print(newTag);  // Debug
        Serial.print("\n\n");  // Debug
    }
    if (leftTag) {
      Serial.println("On left sensor");
    } else {
      Serial.println("On right sensor");      
    }
    // If newTag matched any of the tags
    // we checked against, total will be 1
    if (right_time >= left_time) {
      time_diff = right_time - left_time;
    } else {
      time_diff = left_time - right_time;
    }

    Serial.println("");
    Serial.print("Last left read ");
    Serial.println(left_time);
    Serial.print("Last right read ");
    Serial.println(right_time);
    Serial.print("Read diff ");
    Serial.println(time_diff);
    Serial.println("========================================");
    
//    if (keyPresent && time_diff < 3000) {
//    if (keyPresent) {
//      on(STRIP_COLOR);
//      delay(DURATION_IN_SECONDS * 1000);
//      off();
//    }

  }
  //TODO The following make more sense as local variables.zs
  // Once newTag has been checked, fill it with zeroes
  // to get ready for the next tag read
  for (int c=0; c < idLen; c++) {
    newTag[c] = 0;
  }
  left_tag_read = false;
  right_tag_read = false;
  keyPresent = false;
}
  
void read_tag_rSerial(char* newTag) {
  // Counter for the newTag array
  int i = 0;
  // Variable to hold each byte read from the serial buffer
  int readByte;
  while (rSerial.available()) {
    // Take each byte out of the serial buffer, one at a time
    readByte = rSerial.read();

    /* This will skip the first byte (2, STX, start of text) and the last three,
    ASCII 13, CR/carriage return, ASCII 10, LF/linefeed, and ASCII 3, ETX/end of 
    text, leaving only the unique part of the tag string. It puts the byte into
    the first space in the array, then steps ahead one spot */
    if (readByte != 2 && readByte!= 13 && readByte != 10 && readByte != 3) {
      newTag[i] = readByte;
      i++;
    }

    // If we see ASCII 3, ETX, the tag is over
    if (readByte == 3) {
      return;
    }
  }
}

void read_tag(char* newTag) {
  // Counter for the newTag array
  int i = 0;
  // Variable to hold each byte read from the serial buffer
  int readByte;
  while (Serial.available()) {
    // Take each byte out of the serial buffer, one at a time
    readByte = Serial.read();

    /* This will skip the first byte (2, STX, start of text) and the last three,
    ASCII 13, CR/carriage return, ASCII 10, LF/linefeed, and ASCII 3, ETX/end of 
    text, leaving only the unique part of the tag string. It puts the byte into
    the first space in the array, then steps ahead one spot */
    if (readByte != 2 && readByte!= 13 && readByte != 10 && readByte != 3) {
      newTag[i] = readByte;
      i++;
    }

    // If we see ASCII 3, ETX, the tag is over
    if (readByte == 3) {
      return;
    }
  }
}

// Turns the lights on and off rapidly
void flutter() {
  for( int i = 0; i < 10; i++ ) {
    on(0xffffffff);
    off();
  }
}

//Turns all leds the same color
void on(long color) {
    for(int x = 0 ; x < STRIP_LENGTH ; x++)
      strip_colors[x] = color;
    post_frame();
}

//Turns all LEDS off
void off(void) {
    for(int x = 0 ; x < STRIP_LENGTH ; x++)
      strip_colors[x] = 0;
    post_frame();
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

// This function steps through both newTag and one of the known
// tags. If there is a mismatch anywhere in the tag, it will return 0,
// but if every character in the tag is the same, it returns 1
int checkTag(char nTag[], char oTag[]) {
    for (int i = 0; i < idLen; i++) {
      if (nTag[i] != oTag[i]) {
        return 0;
      }
    }
  return 1;
}



