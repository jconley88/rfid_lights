/*
  RFID lock light
  Jon Conley
  This code was adapted from Nathan Seidle's example code for interacting with the WS2801 and A_CAVIS' hookup guide.
   * https://github.com/sparkfun/WS2812_Breakout/blob/V_1.1b/Firmware/SparkFun_LED_Strip_Example/SparkFun_LED_Strip_Example.ino
   * https://learn.sparkfun.com/tutorials/sparkfun-rfid-starter-kit-hookup-guide/example-project
*/
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

#define WHITE                   0xEEEEEE
#define RED			0xFF0000
#define YELLOW			0xFFFF00
#define GREEN			0x008000
#define BLUE			0x0000FF

// Changeable configuration
const int dsa = 3;
const long STRIP_COLOR = 0xffffff;
const int DURATION_IN_SECONDS = 3;

// WS2801 Light strip configuration
int SDI = 7; //Red wire (not the red 5V wire!)
int CKI = 6; //Green wire
int LED_DATA_PIN = 7;
const int STRIP_LENGTH = 100;
long strip_colors[STRIP_LENGTH];

int ledPin = 13; //On board LED

SoftwareSerial rSerial(8, 999); // RX, TX
const int tagLen = 16;
const int idLen = 13;
const int kTags = 2;
unsigned long left_time = 0;
unsigned long right_time = 0;
unsigned long time_diff = 0;

// Put your known tags here!
char knownTags[kTags][idLen] = {
   "82003C873B02"
};

char knownLeftTags[kTags][idLen] = {
  "82003BA07D64",
  "82003C6FEA3B"
};

// Empty array to hold a freshly scanned tag
char newTag[idLen];
bool leftTag;
bool keyPresent = false;

Adafruit_NeoPixel leds = Adafruit_NeoPixel(STRIP_LENGTH, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
   // Start the hardware and software serial ports
   Serial.begin(9600);
   rSerial.begin(9600);
   Serial.println("Setup"); // Debug
   // setup light strip
//   pinMode(SDI, OUTPUT);
//   pinMode(CKI, OUTPUT);
//   pinMode(ledPin, OUTPUT);
    leds.begin();  // Call this to start up the LED strip.
    off();
    Serial.println("c");
    on(0xCCCCCC);
    delay(3000);
    Serial.println("d");
    on(0xDDDDDD);
    delay(3000);
    Serial.println("e");
    on(0xEEEEEE);
    delay(3000);
    Serial.println("f");
    on(0xFFFFFF);
    delay(3000);
    off();
    flutter();
    
    off();
  
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
        leftKeySuccess();
        keyPresent = true;
    } else if (!leftTag && right_tag_read) {
        rightKeySuccess();
        keyPresent = true;
    } else {
      flutter();
    }
    
    if (left_tag_read == true) {
          Serial.println("Known left tag read");
    } else if (right_tag_read == true) {
          Serial.println("Known right tag read");      
    } else {
        // This prints out unknown cards so you can add them to your knownTags as needed
        Serial.print("Unknown tag! "); // Debug
        Serial.print(newTag);  // Debugsa
        Serial.print("\n\n");  // Debug
    }
    if (leftTag) {
      Serial.println("On left sensor");
    } else {
      Serial.println("On right sensor");      
    }

    Serial.print("\nLast left read ");
    Serial.println(left_time);
    Serial.print("Last right read ");
    Serial.println(right_time);
    Serial.print("Read diff ");
    Serial.println(time_diff);
    Serial.println("========================================");
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

void leftKeySuccess() {
  temporary_on(GREEN, 1000);
  temporary_on(BLUE, 1000);
  off();
}

void rightKeySuccess() {
  temporary_on(YELLOW, 1000);
  temporary_on(RED, 1000);
  off();
}

// Turns the lights on and off rapidly
void flutter() {
  int wait = 10;
  for( int i = 0; i < 20; i++ ) {
    on(WHITE);
    delay(wait);
    off();
    delay(wait);
  }
}

//Turns all leds the same color
void on(long color) {
    for(int i = 0 ; i < STRIP_LENGTH ; i++) {
      leds.setPixelColor(i, color);  // Set just thiseas one
    }
    leds.show();
}

void temporary_on(long color, int wait) {
  
}

//Turns all LEDS off
void off(void) {
  for (int i=0; i<STRIP_LENGTH; i++)
  {
    leds.setPixelColor(i, 0);
  }
  leds.show();
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



