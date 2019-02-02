/***************************************************
  Function Pad
  Automata Development
  Configured for Blender 2.80

  Adafruit Trellis libraries written by Limor Fried/Ladyada for Adafruit Industries.

  MIT license, all text above must be included in any redistribution
****************************************************/

#include <Wire.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <Adafruit_Trellis.h>

// Configure Trellis
Adafruit_Trellis matrix0 = Adafruit_Trellis();
Adafruit_TrellisSet trellis =  Adafruit_TrellisSet(&matrix0);
#define NUMTRELLIS 1
#define numKeys (NUMTRELLIS * 16)

// Connect Trellis Vin to 5V and Ground to ground.
#define INTPIN 4
// Connect I2C SDA pin to your Arduino SDA line
// Connect I2C SCL pin to your Arduino SCL line

// Define Pins for other controls
#define Button1 5
#define Button2 6
#define Button3 7
#define Button4 8

#define Light1 9
#define Light2 10
#define Light3 11
#define Light4 13

#define Rotary1 A1
#define Rotary2 A0
#define RotarySW A2

// Define calibration constants
#define MouseSpeed 6
#define ScrollSpeed -1
#define FastScrollSpeed -2
#define ProfileSelectIncrement 4
#define LightTimeout 30000 // 30 seconds

// Define variables to track current mode
int currentProfile = 0;
bool currentProfileLights[16];
bool lightsEnabled = true;
bool lightsOn = true;
bool profileMode = false;
bool mousexMode = false;
bool mouseyMode = false;
bool fastScrollMode = false;
bool firstRun = true;

// Define variables to track dial movement
bool R1 = false;
bool R2 = false;
bool select = false;
int dialChange = 0;
int dialCounter = 0;
int lastDialChange = 0;
unsigned long lastActionTime = 0;
unsigned long lastKeypadCheck = 0;

/***************************************************
  Program Profiles
****************************************************/
#define NumProfiles 6

// Modifier keys to press/hold for each button
// escape, up, down, delete, shift, control, alt, os
byte defaultKeys[] = {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000100, B00000100, B00000100, B00000000};
byte chromeKeys[] = {B10000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000};
byte fusionKeys[] = {B00000000, B00000100, B00000000, B00000000, B00000000, B00000000, B00001000, B00000100, B00000000, B00000000, B00001000, B00000000, B00000000, B00000100, B00000100, B10000000};
byte blenderKeys[] = {B10000000, B00000010, B00000000, B00001000, B00000000, B00000000, B00000000, B00000000, B00001000, B00000000, B00000000, B00000000, B00001000, B00001000, B00000000, B00000010};
byte hfKeys[] = {B00000100, B00000000, B00000000, B00000100, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00001100, B00001100, B00000000};

// String to type for each button
char* defaultStrs[] = {"", "", "", "", "", "", "", "", "", "", "", "", "c", "x", "v", ""};
char* chromeStrs[] = {"", "", "u", "", "l", "g", "r", "", "c", "a", "", "o", "i", "h", "v", "d"};
char* fusionStrs[] = {"", "s", "p", "i", "l", "r", "r", "p", "c", "a", "a", "o", "", "f", "b", "d"};
char* blenderStrs[] = {"", "a", "i", "d", "", "y", "s", "e", "x", "", "x", "r", "z", "y", "g", "a"};
char* hfStrs[] = {"z", "v", "s", "y", "", "i", "o", "", "", "", "", "", "", "d", "d", ""};

// Last Character to press/hold for each button
int defaultChars[] = {177, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 176};
char numpadChars[] = {'7', '8', '9', '*', '4', '5', '6', '-', '1', '2', '3', '+', '/', '0', '.', '\n'};
int chromeChars[] = {32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int fusionChars[] = {177, 176, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0};
int blenderChars[] = {0, 0, 0, 0, 179, 0, 0, 0, 0, 122, 0, 0, 0, 0, 0, 98};
int hfChars[] = {0, 0, 0, 0, 210, 0, 0, 32, 211, 44, 46, 214, 128, 0, 0, 176};

// Light setting for each button
bool blankLights[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool defaultLights[] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1};
bool defaultIcon[] = {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0};
bool numpadLights[] = {1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1};
bool numpadIcon[] = {1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1};
bool chromeLights[] = {1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1};
bool chromeIcon[] = {1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0};
bool fusionLights[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
bool fusionIcon[] = {1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0};
bool blenderLights[] = {1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1};
bool blenderIcon[] = {1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0};
bool hfLights[] = {0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0};
bool hfIcon[] = {1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0};

// Light settings for modes 2-4
bool mousexLights[] = {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0};
bool mouseyLights[] = {0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0};

/***************************************************
  Function Definitions
****************************************************/
// Function to clear mode variables and indicators
void resetMode() {
  // Reset mode variables
  profileMode = false;
  mousexMode = false;
  mouseyMode = false;
  firstRun = true;

  // Turn off mode lights
  analogWrite(Light1, 0);
  analogWrite(Light2, 0);
  analogWrite(Light3, 0);

  // Reset dial position
  dialCounter = 0;

  // Clear Trellis lights
  memcpy(currentProfileLights, blankLights, sizeof(blankLights[0])*16);
  trellis.clear();
  trellis.writeDisplay();

  // Clear Stored keypresses
  while(trellis.readSwitches()){delay(20);}
}

// Function to read rotary switch and mode selectors
void readInputs() {
  // Read rotary state
  R1 = !digitalRead(Rotary1);
  R2 = !digitalRead(Rotary2);
  select = !digitalRead(RotarySW);

  // Reset light timeout if anything is activated
  if ((dialChange != lastDialChange) || select || !digitalRead(Button1) || !digitalRead(Button2) || !digitalRead(Button3) || !digitalRead(Button4)) {
    lastActionTime = millis();
    lastDialChange = dialChange;
  }

  // Track rotary position
  if(dialChange == 1) {
    if(!R1 && !R2) {
      dialChange = 0;
      dialCounter++;
    }
  }
  else if(dialChange == -1) {
    if(!R1 && !R2) {
      dialChange = 0;
      dialCounter--;
    }
  }
  else {
    if(R1){dialChange = 1;}
    else if(R2){dialChange = -1;}
  }

  // Read mode selectors
  if(!digitalRead(Button1)) {
    if (!profileMode) {
      resetMode();
      profileMode = true;
      analogWrite(Light1, 5);
    }
    else {
      resetMode();
    }
    delay(100);
    while (!digitalRead(Button1)) {
      delay(50);
    }
    delay(100);
  }
  else if(!digitalRead(Button2)) {
    if (!mousexMode) {
      resetMode();
      mousexMode = true;
      analogWrite(Light2, 5);
    }
    else {
      resetMode();
    }
    delay(100);
    while (!digitalRead(Button2)) {
      delay(50);
    }
    delay(100);
  }
  else if(!digitalRead(Button3)) {
    if (!mouseyMode) {
      resetMode();
      mouseyMode = true;
      analogWrite(Light3, 5);
    }
    else {
      resetMode();
    }
    delay(100);
    while (!digitalRead(Button3)) {
      delay(50);
    }
    delay(100);
  }
  else if(!digitalRead(Button4)) {
    if (!fastScrollMode) {
      fastScrollMode = true;
      analogWrite(Light4, 5);
    }
    else {
      fastScrollMode = false;
      analogWrite(Light4, 0);
    }
    delay(250);
    while (!digitalRead(Button4)) {
      delay(50);
    }
    delay(250);
  }
}

//Function to wait for the select key to be released (with debouncing)
void waitForSelectRelease() {
  delay(100);
  while(select) {
    delay(50);
    readInputs();
  }
  delay(100);
}

// Function to press profile modifier keys
// escape, up, down, delete, shift, control, alt, os pressModifiers
void pressModifiers(byte keyList) {
  if (bitRead(keyList, 7)) {Keyboard.write(KEY_ESC);}
  if (bitRead(keyList, 6)) {Keyboard.write(KEY_UP_ARROW);}
  if (bitRead(keyList, 5)) {Keyboard.write(KEY_DOWN_ARROW);}
  if (bitRead(keyList, 4)) {Keyboard.write(KEY_DELETE);}
  if (bitRead(keyList, 3)) {Keyboard.press(KEY_LEFT_SHIFT);}
  if (bitRead(keyList, 2)) {Keyboard.press(KEY_LEFT_CTRL);}
  if (bitRead(keyList, 1)) {Keyboard.press(KEY_LEFT_ALT);}
  if (bitRead(keyList, 0)) {Keyboard.press(KEY_LEFT_GUI);}
}

// Function to release profile modifier keys
// escape, up, down, delete, shift, control, alt, os
void releaseModifiers(byte keyList) {
  if (bitRead(keyList, 3)) {Keyboard.release(KEY_LEFT_SHIFT);}
  if (bitRead(keyList, 2)) {Keyboard.release(KEY_LEFT_CTRL);}
  if (bitRead(keyList, 1)) {Keyboard.release(KEY_LEFT_ALT);}
  if (bitRead(keyList, 0)) {Keyboard.release(KEY_LEFT_GUI);}
}

// Function to get the current light set
void getLights() {
  if(lightsOn && lightsEnabled){
    switch(currentProfile) {
      case 1:
        memcpy(currentProfileLights, numpadLights, sizeof(numpadLights[0])*16);
        break;
      case 2:
        memcpy(currentProfileLights, chromeLights, sizeof(chromeLights[0])*16);
        break;
      case 3:
        memcpy(currentProfileLights, fusionLights, sizeof(fusionLights[0])*16);
        break;
      case 4:
        memcpy(currentProfileLights, blenderLights, sizeof(blenderLights[0])*16);
        break;
      case 5:
        memcpy(currentProfileLights, hfLights, sizeof(hfLights[0])*16);
        break;
      default:
        memcpy(currentProfileLights, defaultLights, sizeof(defaultLights[0])*16);
        break;
    }
  } else {
    memcpy(currentProfileLights, blankLights, sizeof(blankLights[0])*16);
  }
}

// Function to get the current profile icon
void getIcon() {
  switch(currentProfile) {
    case 1:
      memcpy(currentProfileLights, numpadIcon, sizeof(numpadIcon[0])*16);
      break;
    case 2:
      memcpy(currentProfileLights, chromeIcon, sizeof(chromeIcon[0])*16);
      break;
    case 3:
      memcpy(currentProfileLights, fusionIcon, sizeof(fusionIcon[0])*16);
      break;
    case 4:
      memcpy(currentProfileLights, blenderIcon, sizeof(blenderIcon[0])*16);
      break;
    case 5:
      memcpy(currentProfileLights, hfIcon, sizeof(hfIcon[0])*16);
      break;
    default:
      memcpy(currentProfileLights, defaultIcon, sizeof(defaultIcon[0])*16);
      break;
  }
}

// Function to display the current light set
void setLights(bool lightsOverride = false) {
  if((lightsOn && lightsEnabled) || lightsOverride){
    // go through every button
    for (uint8_t i=0; i<numKeys; i++) {
      // set the key light
      if (currentProfileLights[i]) {
        trellis.setLED(i);
      }
      else {
        trellis.clrLED(i);
      }
    }
  } else {
    trellis.clear();
  }
  // tell the trellis to set the LEDs we requested
  trellis.writeDisplay();
}

// Function to find the sign of a value
static inline int8_t sgn(int val) {
 if (val < 0) return -1;
 if (val==0) return 0;
 return 1;
}

/***************************************************
  Initialization function
****************************************************/
void setup() {
  //Serial.begin(9600);

  // Define pin modes and enable pullups
  pinMode(INTPIN, INPUT_PULLUP);

  pinMode(Button1, INPUT_PULLUP);
  pinMode(Button2, INPUT_PULLUP);
  pinMode(Button3, INPUT_PULLUP);
  pinMode(Button4, INPUT_PULLUP);

  pinMode(Light1, OUTPUT);
  pinMode(Light2, OUTPUT);
  pinMode(Light3, OUTPUT);
  pinMode(Light4, OUTPUT);

  pinMode(RotarySW, INPUT_PULLUP);
  pinMode(Rotary1, INPUT_PULLUP);
  pinMode(Rotary2, INPUT_PULLUP);

  // Initialize Trellis
  trellis.begin(0x70);

  // light up all the LEDs in order
  for (uint8_t i=0; i<numKeys; i++) {
    trellis.setLED(i);
    trellis.writeDisplay();
    delay(50);
  }
  // then turn them off
  for (uint8_t i=0; i<numKeys; i++) {
    trellis.clrLED(i);
    trellis.writeDisplay();
    delay(50);
  }

  // initialize control over the mouse and keyboard
  Mouse.begin();
  Keyboard.begin();
}

/***************************************************
  Main loop function
****************************************************/
void loop() {  
  // Profile Selection
  if(profileMode) {
    if(abs(dialCounter) > ProfileSelectIncrement || firstRun) {
      currentProfile = currentProfile + sgn(dialCounter);
      dialCounter = 0;

      if(currentProfile < 0) {
        currentProfile = 0; //(NumProfiles - 1);
      }
      // currentProfile = currentProfile % NumProfiles;
      else if(currentProfile >= NumProfiles) {
        currentProfile = NumProfiles - 1;
      }

      getIcon();
      setLights(true);
    }

    if(firstRun){
      firstRun = false;
    }

    if(select) {
      waitForSelectRelease();
      resetMode();
    }
  }

  // Mouse X
  else if(mousexMode) {
    if(dialCounter != 0) {
      Mouse.move(dialCounter*MouseSpeed, 0);
      dialCounter = 0;
    }

    if(firstRun){
      memcpy(currentProfileLights, mousexLights, sizeof(mousexLights[0])*16);
      setLights();

      firstRun = false;
    }

    if(select) {
      Mouse.click(MOUSE_LEFT);
      waitForSelectRelease();
    }
  }

  // Mouse Y
  else if(mouseyMode) {
    if(dialCounter != 0) {
      Mouse.move(0, dialCounter*MouseSpeed);
      dialCounter = 0;
    }

    if(firstRun){
      memcpy(currentProfileLights, mouseyLights, sizeof(mouseyLights[0])*16);
      setLights();

      firstRun = false;
    }

    if(select) {
      Mouse.click(MOUSE_LEFT);
      waitForSelectRelease();
    }
  }

  // Hotkeys
  else if((millis() - lastKeypadCheck) > 20) {
    // If a button was just pressed or released...
    if (trellis.readSwitches()) {
      lastActionTime = millis();

      // go through every button
      for (uint8_t i=0; i<numKeys; i++) {
        // if it was pressed...
        if (trellis.justPressed(i)) {
          switch(currentProfile) {
            case 1:
              Keyboard.press(numpadChars[i]);
              break;
            case 2:
              pressModifiers(chromeKeys[i]);
              Keyboard.print(chromeStrs[i]);
              releaseModifiers(chromeKeys[i]);
              Keyboard.press(chromeChars[i]);
              break;
            case 3:
              pressModifiers(fusionKeys[i]);
              Keyboard.print(fusionStrs[i]);
              releaseModifiers(fusionKeys[i]);
              Keyboard.press(fusionChars[i]);
              break;
            case 4:
              pressModifiers(blenderKeys[i]);
              Keyboard.print(blenderStrs[i]);
              releaseModifiers(blenderKeys[i]);
              Keyboard.press(blenderChars[i]);
              break;
            case 5:
              pressModifiers(hfKeys[i]);
              Keyboard.print(hfStrs[i]);
              releaseModifiers(hfKeys[i]);
              Keyboard.press(hfChars[i]);
              break;
            default:
              pressModifiers(defaultKeys[i]);
              Keyboard.print(defaultStrs[i]);
              releaseModifiers(defaultKeys[i]);
              Keyboard.press(defaultChars[i]);
              break;
          }
        }
        // if it was released...
        else if (trellis.justReleased(i)) {
          switch(currentProfile) {
            case 1:
              Keyboard.release(numpadChars[i]);
              break;
            case 2:
              Keyboard.release(chromeChars[i]);
              break;
            case 3:
              Keyboard.release(fusionChars[i]);
              break;
            case 4:
              Keyboard.release(blenderChars[i]);
              break;
            case 5:
              Keyboard.release(hfChars[i]);
              break;
            default:
              Keyboard.release(defaultChars[i]);
              break;
          }
        }

        // set the key light
        if (trellis.isKeyPressed(i) || currentProfileLights[i]) {
          trellis.setLED(i);
        }
        else {
          trellis.clrLED(i);
        }
      }
      // tell the trellis to set the LEDs we requested
      trellis.writeDisplay();
    }

    if(firstRun){
      getLights();
      setLights();

      firstRun = false;
    }

    lastKeypadCheck = millis();
  }

  // Scroll
  else {
    if(dialCounter != 0) {
      if(fastScrollMode) {
        Mouse.move(0, 0, dialCounter*FastScrollSpeed);
      }
      else {
        Mouse.move(0, 0, dialCounter*ScrollSpeed);
      }
      dialCounter = 0;
    }

    if(firstRun){
      getLights();
      setLights();

      firstRun = false;
    }

    if(select) {
      Keyboard.releaseAll();

      if(lightsEnabled) {
        // Turn off lights
        lightsEnabled = false;
      }
      else {
        // Turn on lights
        lightsEnabled = true;
      }

      getLights();
      setLights();

      waitForSelectRelease();
    }
  }

  // Light Timeout
  if(((millis() - lastActionTime) > LightTimeout) && lightsOn) {
    lightsOn = false;
    setLights();
    delay(20);
  }
  else if(((millis() - lastActionTime) < LightTimeout) && !lightsOn) {
    lightsOn = true;
    setLights();
  }

  readInputs();
  delay(1);
}
