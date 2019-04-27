/*************************************************************
  Alexis Ward Final Project
  email: aleward@seas.upenn.edu

 *************************************************************
  Morse Code Reader
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include "BST.h"

#include <SPI.h>
#include <WiFi101.h>
#include <Ethernet.h>
#include <BlynkSimpleMKR1000.h>

// BLYNK SETUP
char auth[] = "78be2aeab5a84165bfdb52c07a50141f";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "AirPennNet-Device";
char pass[] = "penn1740wifi";
BlynkTimer timer;
WidgetTerminal terminal(V0);
WidgetTerminal instructions(V1);

// ORIGINAL VARIABLES

// Stores a binary tree I made to represent codes
BST* morse;

// Sensors connected to pins:
const int LSENSOR = A0;//0; // input pin for the light sensor resistor
const int TSENSOR = A1; // input pin for the temp sensor resistor
const int SPACE = 5; // the input pin where the
                      // pushbutton is connected

// RGB Light characteristics connected to pins
const int redPin = 0;
const int greenPin = 8;
const int bluePin = 10;

int initTemp = 0; // variable used to store the value
                  // coming from the temperature sensor
bool prevTemp = false;  // If current temperature is above needed
bool pprevTemp = false; // threshold, compare with last two temps
                        // to not be fooled by outliers
                        
// Values to allow for program setup
bool start = false; // If body temp is scanned yet, if not run scan
bool loading = false; // If body temp is still being checked
int dots = 0; // for Loading's "..."

int ambientVal = 0; // Ambient light value

int count = 1;   // Count for temperature and light duration
int offTime = 0; // Count for how long the light is off

int lightChange = LOW; // OFF - tracks when light turns on and off

String currToken = "";   // Used to find each letter
String currMessage = ""; // Press SEND/SPACE button and this may 
                         // correspond with a command 
int oldButton = 0;   // Tracks press/release of SEND/SPACE button
bool spaced = false; // If the button is pressed once, this becomes 
                     // true - checks for double click (SEND)

bool started = false;

// Action commands:
bool SOS = false;
bool RAINBOW = false;
bool RED = false;
bool GREEN = false;
bool BLUE = false;
bool WHITE = false;
bool YELLOW = false;
bool PURPLE = false;
bool CYAN = false;

void setColor(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}

void setup()
{
  // Debug console
  Serial.begin(9600); // open the serial port to send
                      // data back to the computer at
                      // 9600 bits per second

  Blynk.begin(auth, ssid, pass);
  // Clear the terminal content
  terminal.clear();

  // Create BST
  morse = new BST();
  setColor(0, 0, 0);
  // Get initial temp (allows better check if room is already hot)
  initTemp = analogRead(TSENSOR) / 1024.f * 500.f;

  // First message
  terminal.println("CHECKING IDENTITY\nHold finger to the sensor, and do not move.\n");
  terminal.flush();

  instructions.clear();
  instructions.println("The black semicylinder on the left will take your body temperature to determine if you are human.");
  instructions.println("If you have inhumanly cold hands like me, you will have trouble.");
  instructions.flush();

  // Pin setups
  pinMode(SPACE, INPUT);
  // Output for colors
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT); 
}

//int tcount = 0;
void loop() {
  Blynk.run();
  timer.run(); // Initiates BlynkTimer
  
  // PRE-MORSE: CHECK FOR HUMAN PRESENT lol
  if (!start) {
    
    // Record average ambient light value during this time
    ambientVal += analogRead(LSENSOR);

    // Scale temperature value
    int tval = analogRead(TSENSOR) / 1024.f * 500.f;

    // See if temperature value passes human-body-threshold
    bool bodyTemp = (initTemp > 76.f && initTemp <= 79.f && tval > 79.f) || 
                    (initTemp <= 76.f && tval > 1.f + initTemp);
    bool forAHotDay = (initTemp > 79.f) && (tval > initTemp + 1.5);

    // if temperature IS around body temp (and not a one loop fluke)
    if (!loading && (bodyTemp || forAHotDay)){ 
                    //&& prevTemp && pprevTemp) {
      loading = true;
      terminal.print("Loading");
      terminal.flush();
      delay(3000); // maybe more
    } 
    
    // If that temperature was found as not-a-fluke, check three 
    // more times and display loading animation
    else if (loading && dots < 3) {
      // If still body temp:
      if ((bodyTemp || forAHotDay)/* && prevTemp && pprevTemp*/) {
        // Had to do this for Blynk to print live
        if (dots == 0) {
          terminal.print(" .");
        } else if (dots == 0) {
          terminal.print(" . ");
        } else {
          terminal.print(".");
        }
        terminal.flush();
        dots++;
        delay(1000);
      } 
      // else put out failure message
      else {
        terminal.println("\nFAILED - PLEASE TRY AGAIN\nHold finger to the sensor, and do not move.\n");
        terminal.flush();
        loading = false;
        dots = 0;
      }
    }
      
    // If loading completes (all temp checks passed), accept!
    else if (dots == 3) {
      // Divide total ambience by number of samples
      ambientVal /= count;
      // Reset count and start morse reading!
      count = -1;
      start = true;
      
      // Show that we can continue
      terminal.clear();
      //terminal.println("\n\nACTIVATED");
      terminal.println("ACTIVATED");
      terminal.flush();
      delay(100);
      terminal.println("Send your message:\n");
      terminal.flush();

      // Update instructions
      instructions.clear();
      instructions.println("The Right Button (closest to the light) is your tool to write in Morse. Make sure to get the timing right!\n");
      instructions.println("The Left Button has two functions.");
      instructions.println("   SPACE: Single-click creates a space character.");
      instructions.println("   SEND: Double click sends/clears the current message and creates a new one.\n");
      instructions.println("Try to command the right-most light! Type a color and hit SEND, and there may be some surprise effects as well!");
      instructions.flush();
    }

    // The previous temperature checks
    pprevTemp = prevTemp;
    prevTemp = bodyTemp || forAHotDay;

    /*if (count == 10) {
      ambientVal /= count;
      count = -1;
      start = true;
      // Show that we can continue
      terminal.clear();
      //terminal.println("\n\nACTIVATED");
      terminal.println("ACTIVATED");
      terminal.flush();
      delay(100);
      terminal.println("Send your message:\n");
      terminal.flush();
    }*/
    count++;
    delay(10);
  }

  // UNDERSTANDING THE MORSE
  else {
    sos(); // A possible effect
    rainbow();
    
    // Read current light value from sensor (compared to ambient)
    int lval = analogRead(LSENSOR) - ambientVal;     

    // See when the light is turned on (might raise threshold)
    if (lightChange == LOW && (lval >= 17/* && lval <= 22*/)) {
      lightChange = HIGH;
      started = true;
      offTime = 0;
    }
      
    // See when light is turned off
    else if (lightChange == HIGH && (lval >= -1 && lval <= 3)) {
      lightChange = LOW;
      
      // Decide what character (. vs -) to use from 'count'
      if (count > 0 && count < 33) { // Short  duration
        currToken += ".";
      } else if (count >= 33) { // Long duration
        currToken += "-";
      }
      count = 0; // reset
    } 

    // Time track: ON
    else if (lightChange == HIGH) {
      spaced = false;
      count++;
    }

    // Time track: OFF
    else if (started && lightChange == LOW) {
      offTime++;
      
      // Decide if the space is long enough to end current token
      // Call BST function to translate
      if (offTime == 50) {
        String thisLet = morse->getTokenLetter(currToken);
        // Show the value
        if (thisLet[0] != '\0') {
          terminal.print(thisLet);
          terminal.flush(); 
        }
        // Add this to an overall string and reset token
        currMessage += thisLet;
        currToken = "";
      }
    }

    // SPACE / SEND Button
    int button = digitalRead(SPACE);
    if ((button == HIGH) && (oldButton == LOW)) {
      // Double click - SEND MESSSAGE
      if (spaced) {
        terminal.clear();
        //terminal.println("\n "); 
        terminal.println("New Message:\n"); 
        terminal.flush();
        // And turn some action true if message is applicable
        // (And set others as false (unless SOS?)

        if (currMessage.length() <= 8 && currMessage[0] == ' ') {
          String newMess = "";
          for (int indx = 1; indx < currMessage.length(); indx++) {
            newMess+= currMessage[indx];
          }
          currMessage = newMess;
        }

        sosCheck();
        rbowCheck();
        red();
        green();
        blue();
        white();
        yellow();
        purple();
        cyan();
        stopLight();
        
        spaced = false;
        currMessage = "";
      } 
      // Single click - SPACE
      else {
        terminal.print(" ");
        terminal.flush();
        currMessage += " ";
        spaced = true;
      }
    }
    oldButton = button;

    delay(5); // stop the program for
               // some time
  }
  
  /*delay(100);
  if (tcount % 10 == 0) {
    //terminal.clear();
  }
  if (tcount % 2 == 0) {
    terminal.print("Hi!");
  } else {
    terminal.print("Ho!");
  }
  terminal.flush();
  tcount++;*/
}


// COLORS

int currCol[3] = {0, 0, 0};

void setFalse(bool* a, bool* b, bool* c, 
              bool* d, bool* e, bool* f) {
   *a = false;
   *b = false;
   *c = false;
   *d = false;
   *e = false;
   *f = false;
}


void red() {
  if (!RED) {
    if (currMessage.length() <= 4 && currMessage[0] == 'r' && 
        currMessage[1] == 'e' && currMessage[2] == 'd') {
      if (!SOS) {
        setColor(8, 0, 0);
      }
      currCol[0] = 8;
      currCol[1] = 0;
      currCol[2] = 0;
      RED = true;
      RAINBOW = false;
      setFalse(&GREEN, &BLUE, &WHITE, &YELLOW, &PURPLE, &CYAN);
    }
  }
}

void green() {
  if (!GREEN) {
    if (currMessage.length() <= 6 && currMessage[0] == 'g' && 
        currMessage[1] == 'r' && currMessage[2] == 'e' && 
        currMessage[3] == 'e' && currMessage[4] == 'n') {
      if (!SOS) {
        setColor(0, 5, 0);
      } 
      currCol[0] = 0;
      currCol[1] = 5;
      currCol[2] = 0;
      GREEN = true;
      RAINBOW = false;
      setFalse(&RED, &BLUE, &WHITE, &YELLOW, &PURPLE, &CYAN);
    }
  }
}

void blue() {
  if (!BLUE) {
    if (currMessage.length() <= 5 && currMessage[0] == 'b' && 
        currMessage[1] == 'l' && currMessage[2] == 'u' && 
        currMessage[3] == 'e') {
      if (!SOS) {
        setColor(0, 0, 4);
      }
      currCol[0] = 0;
      currCol[1] = 0;
      currCol[2] = 4;
      BLUE = true;
      RAINBOW = false;
      setFalse(&RED, &GREEN, &WHITE, &YELLOW, &PURPLE, &CYAN);
    }
  }
}

void white() {
  if (!WHITE) {
    if (currMessage.length() <= 6 && currMessage[0] == 'w' && 
        currMessage[1] == 'h' && currMessage[2] == 'i' && 
        currMessage[3] == 't' && currMessage[4] == 'e') {
      if (!SOS) {
        setColor(5, 4, 1);
      }
      currCol[0] = 5;
      currCol[1] = 4;
      currCol[2] = 1;
      WHITE = true;
      RAINBOW = false;
      setFalse(&RED, &GREEN, &BLUE, &YELLOW, &PURPLE, &CYAN);
    }
  }
}

void yellow() {
  if (!YELLOW) {
    if (currMessage.length() <= 7 && currMessage[0] == 'y' && 
        currMessage[1] == 'e' && currMessage[2] == 'l' && 
        currMessage[3] == 'l' && currMessage[4] == 'o' && 
        currMessage[5] == 'w') {
      if (!SOS) {
        setColor(6, 2, 0);
      }
      currCol[0] = 6;
      currCol[1] = 2;
      currCol[2] = 0;
      YELLOW = true;
      RAINBOW = false;
      setFalse(&RED, &GREEN, &BLUE, &WHITE, &PURPLE, &CYAN);
    }
  }
}

void purple() {
  if (!PURPLE) {
    if (currMessage.length() <= 7 && currMessage[0] == 'p' && 
        currMessage[1] == 'u' && currMessage[2] == 'r' && 
        currMessage[3] == 'p' && currMessage[4] == 'l' && 
        currMessage[5] == 'e') {
      if (!SOS) {
        setColor(3, 0, 3);
      }
      currCol[0] = 3;
      currCol[1] = 0;
      currCol[2] = 3;
      PURPLE = true;
      RAINBOW = false;
      setFalse(&RED, &GREEN, &BLUE, &WHITE, &YELLOW, &CYAN);
    }
  }
}

void cyan() {
  if (!CYAN) {
    if (currMessage.length() <= 5 && currMessage[0] == 'c' && 
        currMessage[1] == 'y' && currMessage[2] == 'a' && 
        currMessage[3] == 'n') {
      if (!SOS) {
        setColor(0, 5, 2);
      }
      currCol[0] = 0;
      currCol[1] = 5;
      currCol[2] = 2;
      CYAN = true;
      RAINBOW = false;
      setFalse(&RED, &GREEN, &BLUE, &WHITE, &YELLOW, &PURPLE);
    }
  }
}

int sosCount = 0;

void sosCheck() {
  if (!SOS) {
    if (currMessage.length() <= 4 && currMessage[0] == 's' && 
        currMessage[1] == 'o' && currMessage[2] == 's') {
      currCol[0] = 8;
      currCol[1] = 0;
      currCol[2] = 0;
      SOS = true;
      RAINBOW = false;
      setFalse(&RED, &GREEN, &BLUE, &WHITE, &YELLOW, &PURPLE);
      CYAN = false;
      sosCount = 0;
    }
  }
}

void sos() {
  if (SOS) {
    if (sosCount == 0 || sosCount == 45 || sosCount == 90 ||
        sosCount == 180 || sosCount == 265 || sosCount == 350 ||
        sosCount == 480 || sosCount == 525 || sosCount == 570) {
      setColor(currCol[0] * 10, currCol[1] * 10, currCol[2] * 10);
    } 
    else if (sosCount == 20 || sosCount == 65 || sosCount == 110 ||
               sosCount == 240 || sosCount == 325 || sosCount == 410 ||
               sosCount == 500 || sosCount == 545 || sosCount == 590) {
      setColor(0, 0, 0);
    } 
    else if (sosCount == 800) {
      sosCount = -1;
    }
    sosCount++;
  }
}

void rbowCheck() {
  if (!RAINBOW) {
    if (currMessage.length() <= 4 && currMessage[0] == 'g' && 
        currMessage[1] == 'a' && currMessage[2] == 'y') {
      SOS = false;
      RAINBOW = true;
      setFalse(&RED, &GREEN, &BLUE, &WHITE, &YELLOW, &PURPLE);
      CYAN = false;
      sosCount = 0;
    }
  }
}

int r = 0;
int g = 0;
int b = 0;
void rainbow() {
  if (RAINBOW) {
    if (b < 100) {
      b++;
    } else if (b == 100) { 
      b = 0;
    }
    if (g < 250) {
      g ++;
    } else if (g == 250) {
      g = 0;
    }
    if (r < 300) {
      r++;
    } else if (r == 300) {
      r = 0;
    }
    setColor(r / 40, g / 50, b / 60);
  }
}

void stopLight() {
  if (currMessage.length() <= 5 && currMessage[0] == 's' && 
      currMessage[1] == 't' && currMessage[2] == 'o' && 
      currMessage[3] == 'p') {
    setColor(0, 0, 0);
    SOS = false;
    RAINBOW = false;
    setFalse(&RED, &GREEN, &BLUE, &WHITE, &YELLOW, &PURPLE);
    CYAN = false;
    sosCount = 0;
  }
}
