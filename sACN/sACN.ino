/*
Teensy 4.1

GND       VIN
0-RX      GND
1-TX      3.3V
2-S1Dir   23
3-S1Stp   22
4-S1En    21
5         20
6         19 - SCL
7         18 - SDA
8         17
9         16
10        15
11        14
12        13 - LED
3.3V      GND
24        41 - Save/Back Pushbutton
25        40 - Next Pushbutton
26-S2Dir  39 - Down Pushbutton
27-S2Stp  38 - Up Pushbutton
28-S2En   37
29-X      36 - X
30        35
31-S3Stp  34
32-S3Dir  33 - S3 En

*/

float Version = 1.0;

#include "MyFunctions.h"
#include <Wire.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <AccelStepper.h>
#include <EEPROM.h>

// Navigation board, Probably Remove later?
#include <SparkFun_I2C_Expander_Arduino_Library.h>
SFE_PCA95XX io(PCA95XX_PCA9554);
int buttonUp = 1;
int buttonDown = 0;
int buttonRight = 3;
int buttonLeft = 2;
int buttonCenter = 4;
int ledBlue = 5;
int ledGreen = 6;
int ledRed = 7;

// Ethernet setup
byte mac[] = { 0xFA, 0xDE, 0xC0, 0xDE, 0x00, 0x01 };
IPAddress multicastIP(239, 255, 0, 1); // Example multicast address for Universe 1
IPAddress localIP(10, 90, 30, 92);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress gateway(10, 90, 30, 253);
IPAddress dns(0, 0, 0, 0);
unsigned int localPort = 5568;
//Moved to values for Display
//uint16_t currentUniverse = 1; // Start at universe 1
//uint16_t dmxChannel = 1; // Start at DMX channel 1

EthernetUDP udp;

// --- IP Address Variables ---
byte ipAddress[4] = {10, 90, 30, 100};
int selectedOctet = 0;
// --- Button State & Debouncing Variables ---
// We need to track the state for each button to achieve a "one-shot" press
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; // 50ms is a good starting point
unsigned long repeatDelay = 500;
unsigned long lastRepeatDelay = 0;
// Add variables to track the previous state of each button
int upButtonState;
int downButtonState;
int nextButtonState;
int prevButtonState;
int saveButtonState;
bool needsRedraw = true;
bool needsInvert = false;
unsigned long InvertStartTime = 0;
unsigned long lastButtonCheck = 0;
bool InvertFF = false;
const unsigned long buttonInterval = 20;  // ms
unsigned long lastAutoTime = 0;
const unsigned long AutoTime = 10000;  //10 seconds
const unsigned long ScreenOffTime = 10000;
unsigned long lastScreenOff = 0;
bool ScreenOff = false;
int AutoSteps = 7; // Number - 1
int AutoStep = 0; // Current Step
uint8_t AutoPan[] = {0 ,0, 128, 128, 255, 255, 128, 128};
uint8_t AutoTilt[] = {20 ,20, 128, 128, 230, 230, 128, 128};
uint8_t AutoDisc[] = {0 ,10, 50, 128, 255, 128, 50, 10};
uint8_t AutoLEDH[] = {0 ,10, 50, 128, 255, 128, 50, 10};
uint8_t AutoLEDS[] = {0 ,10, 255, 128, 0, 128, 255, 10};
uint8_t AutoLEDV[] = {255 ,255, 255, 255, 255, 255, 255, 255};
uint8_t AutoLEDM[] = {0 ,20, 40, 60, 180, 200, 220, 240};
uint8_t AutoFX[] = {5 ,21, 61, 0, 111, 0, 131, 0};
uint8_t AutoFXSpeed[] = {5 ,5, 5, 5, 5, 5, 5, 5};

const int LED = 13;
uint8_t DiscStepEn = 33;
uint8_t DiscStepStep = 31;
uint8_t DiscStepDir = 32;
uint8_t TiltStepEn = 28;
uint8_t TiltStepStep = 27;
uint8_t TiltStepDir = 26;
uint8_t PanStepEn = 4;
uint8_t PanStepStep = 3;
uint8_t PanStepDir = 2;
const float maxSpeed = 2500.0;  // Steps/sec (based on minDelay=400us -> ~2500 steps/sec)
const float minSpeed = 250.0;   // Steps/sec (based on maxDelay=4000us -> ~250 steps/sec)
float adjSpeed = 2500.0;
const float Pacceleration = 500.0;  // Steps/sec² for smooth ramp (adjust to reduce jerk)
const float Tacceleration = 500.0;  // Steps/sec² for smooth ramp (adjust to reduce jerk)
const float Dacceleration = 50.0;  // Steps/sec² for smooth ramp (adjust to reduce jerk)
uint8_t DiscCMD = 0;
unsigned long lastDiscAccel = 0;
const unsigned long DiscAccelTime = 10;
float discSpeed = 0.0;
float discDir = -1.0;

//Channels
uint8_t Pan1 = 0;
uint8_t Pan2 = 1;
uint8_t Tilt1 = 2;
uint8_t Tilt2 = 3;
uint8_t Disc1 = 4;
uint8_t Disc2 = 5;
uint8_t Disc3 = 6;
uint8_t L1H = 7;
uint8_t L1S = 8;
uint8_t L1V = 9;
uint8_t L1M = 10;
uint8_t L2H = 11; //7 for Single
uint8_t L2S = 12; //8 for Single
uint8_t L2V = 13; //9 for Single
uint8_t L2M = 14; //10 for Single
uint8_t L3H = 15; //7 for Single    //11 for EveryOther
uint8_t L3S = 16; //8 for Single    //12 for EveryOther
uint8_t L3V = 17; //9 for Single    //13 for EveryOther
uint8_t L3M = 18; //10 for Single   //14 for EveryOther
uint8_t L4H = 19; //7 for Single
uint8_t L4S = 20; //8 for Single
uint8_t L4V = 21; //9 for Single
uint8_t L4M = 22; //10 for Single
uint8_t L5H = 23; //7 for Single    //11 for EveryOther
uint8_t L5S = 24; //8 for Single    //12 for EveryOther
uint8_t L5V = 25; //9 for Single    //13 for EveryOther
uint8_t L5M = 26; //10 for Single   //14 for EveryOther
uint8_t L6H = 27; //7 for Single
uint8_t L6S = 28; //8 for Single
uint8_t L6V = 29; //9 for Single
uint8_t L6M = 30; //10 for Single
uint8_t L7H = 31; //7 for Single    //11 for EveryOther
uint8_t L7S = 32; //8 for Single    //12 for EveryOther
uint8_t L7V = 33; //9 for Single    //13 for EveryOther
uint8_t L7M = 34; //10 for Single   //14 for EveryOther
uint8_t L8H = 35; //7 for Single
uint8_t L8S = 36; //8 for Single
uint8_t L8V = 37; //9 for Single
uint8_t L8M = 38; //10 for Single
uint8_t L9H = 39; //7 for Single    //11 for EveryOther
uint8_t L9S = 40; //8 for Single    //12 for EveryOther
uint8_t L9V = 41; //9 for Single    //13 for EveryOther
uint8_t L9M = 42; //10 for Single   //14 for EveryOther
uint8_t L10H = 43; //7 for Single
uint8_t L10S = 44; //8 for Single
uint8_t L10V = 45; //9 for Single
uint8_t L10M = 46; //10 for Single
uint8_t L11H = 47; //7 for Single    //11 for EveryOther
uint8_t L11S = 48; //8 for Single    //12 for EveryOther
uint8_t L11V = 49; //9 for Single    //13 for EveryOther
uint8_t L11M = 50; //10 for Single   //14 for EveryOther
uint8_t L12H = 51; //7 for Single
uint8_t L12S = 52; //8 for Single
uint8_t L12V = 53; //9 for Single
uint8_t L12M = 54; //10 for Single
uint8_t PTSpeed = 55; // 11 for Single  //15 for EveryOther
uint8_t FXSel = 56; //12 for Single     //16 for EveryOther
uint8_t FXSpeed = 57; //13 for Single   //17 for EveryOther

//FX stuff
long FXRandomTime = 0;
long FXRandomLast = 0;
long FXRandomIndex = 0;
long FXSpinTime = 0;
long FXSpinLast = 0;
long FXSpinIndex = 0;
long FXMirrorTime = 0;
long FXMirrorLast = 0;
long FXMirrorIndex = 0;
long FXColorTime = 0;
long FXColorLast = 0;
long FXColorIndex = 0;

AccelStepper panStepper(AccelStepper::DRIVER, PanStepStep, PanStepDir);
AccelStepper discStepper(AccelStepper::DRIVER, DiscStepStep, DiscStepDir);
AccelStepper tiltStepper(AccelStepper::DRIVER, TiltStepStep, TiltStepDir);

bool TiltHomed = false;
int TiltCMD = 3050;
int HomePosT = 3050;
bool PanHomed = false;
int PanCMD = 7200;
int HomePosP = 7200;
const int StepsPerDMXT = 12; //6000 step is a rotation so 6000/255 = 23.59, so 12 for half tilt
const int StepsPerDMXP = 28; //8000 step is a rotation so 8000/255 = 31.37
int StepsScaled = 0;

// Variables for sACN handling
const unsigned long TIMEOUT = 2500; // 2.5 seconds timeout for top priority
unsigned long lastPacketTime = 0;
int currentPriority = 0;
byte dmxData[513]; // DMX values (512 channels + start code)

//Serial
char sBuffer[70];
const unsigned long SendTiming = 50;
unsigned long lastSendTime = 0;

//Functions
int menuPage = 0; // Current menu page
//int menuNum = 4;
const char* menuHeaders[] = {" DMX Addr.", "sACN Univs", "Ctrl Mode ", " DMX Mode ", " Pan Axis ", "Tilt Axis ", "Disc Axis ", " LEDs Hue ", "LEDs Satur", "LEDs Value", "LEDsMirror", " FX Select", " FX Speed "};
int menuType[] = {0 ,0 ,2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 0 = Integer, 1 = Yes/No, 2 = Control Mode, 3 = DMX Mode
int values[] = {0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Array of values: [dmxChannel, currentUniverse, DMX?]
int menuPages = 4; // or 13 if manual //sizeof(menuType) / sizeof(menuType[0]);
int DMXA = 0;
uint16_t sACNU = 1;
int ControlMode = 1;
int DMXMode = 1;

void setup() { //--------------------------------- Setup

  pinMode(PanStepEn, OUTPUT);
  pinMode(PanStepStep, OUTPUT);
  pinMode(PanStepDir, OUTPUT);
  digitalWrite(PanStepEn, LOW);
  digitalWrite(PanStepStep, LOW);
  digitalWrite(PanStepDir, LOW);
  pinMode(DiscStepEn, OUTPUT);
  pinMode(DiscStepStep, OUTPUT);
  pinMode(DiscStepDir, OUTPUT);
  digitalWrite(DiscStepEn, LOW);
  digitalWrite(DiscStepStep, LOW);
  digitalWrite(DiscStepDir, LOW);
  pinMode(TiltStepEn, OUTPUT);
  pinMode(TiltStepStep, OUTPUT);
  pinMode(TiltStepDir, OUTPUT);
  digitalWrite(TiltStepEn, LOW);
  digitalWrite(TiltStepStep, LOW);
  digitalWrite(TiltStepDir, LOW);
  pinMode(LED, OUTPUT); 
  digitalWrite(LED, HIGH);
  Serial.begin(921600);
  Serial.setTimeout(10);
  Serial.print("Version: ");
  Serial.println(Version);
  
  values[0] = EEPROM.read(0);
  DMXA = values[0];
  EEPROM.get(1, sACNU);
  values[1] = sACNU;
  //values[2] = EEPROM.read(3);
  ControlMode = values[2];
  values[3] = EEPROM.read(4);
  DMXMode = values[3];

  if (DMXA > 255){ DMXA = 0; }
  if (sACNU > 63999){ sACNU = 1; }
  if (sACNU == 0){ sACNU = 1; }
  if (ControlMode == 0){ ControlMode = 1; }
  if (ControlMode > 3){ ControlMode = 1; }
  if (DMXMode == 0){ DMXMode = 1; }
  if (DMXMode > 3){ DMXMode = 1; }

  if (ControlMode == 3){
    menuPages = 13;
  }

  // Initialize the Navigation boartd
  if (!io.begin()){
    Serial.println("Navigation button Error");
  }
  Wire.setClock(400000);

  io.pinMode(buttonUp, INPUT);
  io.pinMode(buttonDown, INPUT);
  io.pinMode(buttonLeft, INPUT);
  io.pinMode(buttonRight, INPUT);
  io.pinMode(buttonCenter, INPUT);
  io.pinMode(ledRed, OUTPUT);
  io.pinMode(ledGreen, OUTPUT);
  io.pinMode(ledBlue, OUTPUT);

  io.digitalWrite(ledRed, LOW);
  io.digitalWrite(ledGreen, LOW);
  io.digitalWrite(ledBlue, HIGH);

  // Read the initial state of the buttons
  upButtonState = io.digitalRead(buttonUp);
  downButtonState = io.digitalRead(buttonDown);
  nextButtonState = io.digitalRead(buttonRight);
  prevButtonState = io.digitalRead(buttonLeft);
  saveButtonState = io.digitalRead(buttonCenter);

  
  if (!DisplaySetup()){
    Serial.println("Display Error");
  } 
  DisplayDraw(menuPage, menuHeaders, values, menuType); // Update display

  pinMode(LED, OUTPUT);
  //analogWrite(LED, 0);
  Serial1.begin(38400);

  //Clear LEDs
  sBuffer[0] = 'D';              // Start with 'D'
  Serial1.write(sBuffer, 49);

  panStepper.setMaxSpeed(maxSpeed);
  panStepper.setAcceleration(Pacceleration);
  panStepper.setMinPulseWidth(50);
    
  discStepper.setMaxSpeed(maxSpeed);
  discStepper.setAcceleration(Dacceleration);
  discStepper.setMinPulseWidth(50);

  tiltStepper.setMaxSpeed(maxSpeed);
  tiltStepper.setAcceleration(Tacceleration);
  tiltStepper.setMinPulseWidth(50);

  // Start Ethernet on Teensy
  digitalWrite(LED, HIGH);
  Serial.println("Starting Ethernet");
  DisplayEthernet(0);
  Ethernet.begin(mac, 5);
  Serial.println("Ethernet Start Finished");
  DisplayEthernet(1);

  delay(1000);

  // start UDP
  udp.begin(localPort);
  
  // Join initial multicast group
  joinMulticastGroup(sACNU);

  lastScreenOff = millis();
}

void loop() { //--------------------------------- Main Loop
  
  // Check for incoming packets
  int packetSize = udp.parsePacket();
  if (packetSize > 0 && ControlMode == 1) {
   handlePacket(packetSize);
   if (DMXMode > 1){
    DMXModez();
   }
  }

  if (ControlMode == 2){
    Auto();
  } else if (ControlMode == 3){
    Manual();
  }

  //Call FXs
  FX();

  // Check for timeout
  if (millis() - lastPacketTime > TIMEOUT) {
    lastPacketTime = millis();
    if (Ethernet.linkStatus() != LinkOFF) {
      handleTimeout();
    }
  }

  //Screen Timeout
  if (millis() - lastScreenOff > ScreenOffTime && ScreenOff == false){
    DisplayOff();
    ScreenOff = true;
  }

  // Pan position from DMX if homed
  if (PanHomed == true){
    PanCMD = (dmxData[Pan1+DMXA] * StepsPerDMXP) + dmxData[Pan2+DMXA];
  }
  else {
    panStepper.setMaxSpeed(minSpeed);
  }
  
  // Tilt position from DMX if homed
  if (TiltHomed == true){
    TiltCMD = (dmxData[Tilt1+DMXA] * StepsPerDMXT) + dmxData[Tilt2+DMXA];
  }
  else {
    tiltStepper.setMaxSpeed(minSpeed);
  }

  if (ControlMode == 1){
  adjSpeed = map(dmxData[PTSpeed+DMXA], 0, 255, minSpeed, maxSpeed);
  } else {
    adjSpeed = maxSpeed;
  }
  panStepper.setMaxSpeed(adjSpeed);
  tiltStepper.setMaxSpeed(adjSpeed);

  panStepper.moveTo(PanCMD);  // This handles ramping to position
  tiltStepper.moveTo(TiltCMD);

  // Run steppers (non-blocking)
  discStepper.runSpeed();  // For velocity control
  panStepper.run();       // For position control with accel/decel
  tiltStepper.run();

  // Homing check (add limit switch if possible)
  if (panStepper.currentPosition() == HomePosP) {
    PanHomed = true;
    panStepper.setMaxSpeed(maxSpeed);
  }
  if (tiltStepper.currentPosition() == HomePosT) {
    TiltHomed = true;
    tiltStepper.setMaxSpeed(maxSpeed);
  }


  // Disc speed from DMX (velocity mode)
  if (millis() - lastDiscAccel > DiscAccelTime) { // If Accel time elapses, bring the speed up or down slowly, home made accel!
    lastDiscAccel = millis();
    if (DiscCMD < dmxData[Disc1+DMXA]) {
      DiscCMD = DiscCMD + 1;
    } else if (DiscCMD > dmxData[Disc1+DMXA]) {
      DiscCMD = DiscCMD - 1;
    }
  }
  discSpeed = map(DiscCMD, 0, 255, 0, maxSpeed);  // 0 to max
  if (TiltHomed == true && PanHomed == true){
    discStepper.setSpeed(discSpeed * discDir);  // Constant speed for disc
  }

  // Disc direction (only when stopped)
  if (discSpeed == 0) {
    discStepper.setSpeed(0);
    if (dmxData[Disc2+DMXA] > 127) {
      discDir = -1.0;
    } else {
      discDir = 1.0;
    }
  }

 // Send Data for Chips
  if (millis() - lastSendTime > SendTiming) {
    lastSendTime = millis();
    sBuffer[0] = 'D';              // Start with 'D'
    memcpy(&sBuffer[1], &dmxData[7], 48);
    Serial1.write(sBuffer, 49);
  }
  
  //Button Press check
  if (millis() - lastButtonCheck >= buttonInterval) {
  lastButtonCheck = millis();
  handleButtons();
  }

  //Redraw Screen if called for
 if (needsRedraw) {
   DisplayDraw(menuPage, menuHeaders, values,menuType); // Update display
   needsRedraw = false;
 }

  //Screen Invert code to not pause execution
  if (needsInvert == true){
    if (InvertFF == false){
      InvertStartTime = millis();
      DisplayInvert(true);
      InvertFF = true;
    }
    if (millis() - InvertStartTime >= 500){
      DisplayInvert(false);
      needsInvert = false;
      InvertFF = false;
    }
 }

}

void handlePacket(int packetSize) { //--------------------------------- sACN Packet
  byte buffer[638]; // Max possible size for sACN data packet
  udp.read(buffer, packetSize);

  // Parse priority and DMX data
  int priority = buffer[108]; // Priority is at byte 108 of sACN packet
  int universe = (buffer[113] << 8) | buffer[114]; // Universe at bytes 113-114

  if (universe != 1) return; // Only handle Universe 1 in this example

  // Handle priority logic
  if (priority >= currentPriority) {
    currentPriority = priority;
    lastPacketTime = millis();

    // Extract DMX data (start code at byte 125, DMX data at byte 126)
    memcpy(dmxData, &buffer[126], 512);
  }
}

void handleTimeout() {  //--------------------------------- Timeout
  Serial.println("Top priority source timed out.");
  currentPriority = 0;
  lastPacketTime = millis();
}

void DMXModez() {
  if (DMXMode == 2){
    //Pan1 through LED1 stuff left alone
    dmxData[PTSpeed+DMXA] = dmxData[11+DMXA]; 
    dmxData[FXSel+DMXA] = dmxData[12+DMXA];
    dmxData[FXSpeed+DMXA] = dmxData[13+DMXA];
    dmxData[L2H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L2S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L2V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L2M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L3H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L3S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L3V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L3M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L4H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L4S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L4V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L4M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L5H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L5S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L5V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L5M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L6H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L6S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L6V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L6M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L7H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L7S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L7V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L7M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L8H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L8S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L8V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L8M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L9H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L9S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L9V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L9M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L10H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L10S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L10V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L10M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L11H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L11S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L11V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L11M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L12H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L12S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L12V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L12M+DMXA] = dmxData[L1M+DMXA];
  } else if (DMXMode == 3){
    //Pan1 through LED2 stuff left alone
    dmxData[PTSpeed+DMXA] = dmxData[15+DMXA]; 
    dmxData[FXSel+DMXA] = dmxData[16+DMXA];
    dmxData[FXSpeed+DMXA] = dmxData[17+DMXA];
    dmxData[L3H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L3S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L3V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L3M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L4H+DMXA] = dmxData[L2H+DMXA];
    dmxData[L4S+DMXA] = dmxData[L2S+DMXA];
    dmxData[L4V+DMXA] = dmxData[L2V+DMXA];
    dmxData[L4M+DMXA] = dmxData[L2M+DMXA];
    dmxData[L5H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L5S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L5V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L5M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L6H+DMXA] = dmxData[L2H+DMXA];
    dmxData[L6S+DMXA] = dmxData[L2S+DMXA];
    dmxData[L6V+DMXA] = dmxData[L2V+DMXA];
    dmxData[L6M+DMXA] = dmxData[L2M+DMXA];
    dmxData[L7H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L7S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L7V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L7M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L8H+DMXA] = dmxData[L2H+DMXA];
    dmxData[L8S+DMXA] = dmxData[L2S+DMXA];
    dmxData[L8V+DMXA] = dmxData[L2V+DMXA];
    dmxData[L8M+DMXA] = dmxData[L2M+DMXA];
    dmxData[L9H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L9S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L9V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L9M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L10H+DMXA] = dmxData[L2H+DMXA];
    dmxData[L10S+DMXA] = dmxData[L2S+DMXA];
    dmxData[L10V+DMXA] = dmxData[L2V+DMXA];
    dmxData[L10M+DMXA] = dmxData[L2M+DMXA];
    dmxData[L11H+DMXA] = dmxData[L1H+DMXA];
    dmxData[L11S+DMXA] = dmxData[L1S+DMXA];
    dmxData[L11V+DMXA] = dmxData[L1V+DMXA];
    dmxData[L11M+DMXA] = dmxData[L1M+DMXA];
    dmxData[L12H+DMXA] = dmxData[L2H+DMXA];
    dmxData[L12S+DMXA] = dmxData[L2S+DMXA];
    dmxData[L12V+DMXA] = dmxData[L2V+DMXA];
    dmxData[L12M+DMXA] = dmxData[L2M+DMXA];
  }
}

void Auto(){

  // Auto Timer
  if (millis() - lastAutoTime >= AutoTime){
    lastAutoTime = millis();
    AutoStep = AutoStep + 1;
    if (AutoStep > AutoSteps){
      AutoStep = 0;
    }
  }

  //DMX Control
  dmxData[Pan1+DMXA] = AutoPan[AutoStep];
  dmxData[Tilt1+DMXA] = AutoTilt[AutoStep];
  dmxData[Disc1+DMXA] = AutoDisc[AutoStep];
  dmxData[L1H+DMXA] = AutoLEDH[AutoStep];
  dmxData[L1S+DMXA] = AutoLEDS[AutoStep];
  dmxData[L1V+DMXA] = AutoLEDV[AutoStep];
  dmxData[L1M+DMXA] = AutoLEDM[AutoStep];
  dmxData[L2H+DMXA] = AutoLEDH[AutoStep];
  dmxData[L2S+DMXA] = AutoLEDS[AutoStep];
  dmxData[L2V+DMXA] = AutoLEDV[AutoStep];
  dmxData[L2M+DMXA] = AutoLEDM[AutoStep];
  dmxData[L3H+DMXA] = AutoLEDH[AutoStep];
  dmxData[L3S+DMXA] = AutoLEDS[AutoStep];
  dmxData[L3V+DMXA] = AutoLEDV[AutoStep];
  dmxData[L3M+DMXA] = AutoLEDM[AutoStep];
  dmxData[L4H+DMXA] = AutoLEDH[AutoStep];
  dmxData[L4S+DMXA] = AutoLEDS[AutoStep];
  dmxData[L4V+DMXA] = AutoLEDV[AutoStep];
  dmxData[L4M+DMXA] = AutoLEDM[AutoStep];
  dmxData[L5H+DMXA] = AutoLEDH[AutoStep];
  dmxData[L5S+DMXA] = AutoLEDS[AutoStep];
  dmxData[L5V+DMXA] = AutoLEDV[AutoStep];
  dmxData[L5M+DMXA] = AutoLEDM[AutoStep];
  dmxData[L6H+DMXA] = AutoLEDH[AutoStep];
  dmxData[L6S+DMXA] = AutoLEDS[AutoStep];
  dmxData[L6V+DMXA] = AutoLEDV[AutoStep];
  dmxData[L6M+DMXA] = AutoLEDM[AutoStep];
  dmxData[L7H+DMXA] = AutoLEDH[AutoStep];
  dmxData[L7S+DMXA] = AutoLEDS[AutoStep];
  dmxData[L7V+DMXA] = AutoLEDV[AutoStep];
  dmxData[L7M+DMXA] = AutoLEDM[AutoStep];
  dmxData[L8H+DMXA] = AutoLEDH[AutoStep];
  dmxData[L8S+DMXA] = AutoLEDS[AutoStep];
  dmxData[L8V+DMXA] = AutoLEDV[AutoStep];
  dmxData[L8M+DMXA] = AutoLEDM[AutoStep];
  dmxData[L9H+DMXA] = AutoLEDH[AutoStep];
  dmxData[L9S+DMXA] = AutoLEDS[AutoStep];
  dmxData[L9V+DMXA] = AutoLEDV[AutoStep];
  dmxData[L9M+DMXA] = AutoLEDM[AutoStep];
  dmxData[L10H+DMXA] = AutoLEDH[AutoStep];
  dmxData[L10S+DMXA] = AutoLEDS[AutoStep];
  dmxData[L10V+DMXA] = AutoLEDV[AutoStep];
  dmxData[L10M+DMXA] = AutoLEDM[AutoStep];
  dmxData[L11H+DMXA] = AutoLEDH[AutoStep];
  dmxData[L11S+DMXA] = AutoLEDS[AutoStep];
  dmxData[L11V+DMXA] = AutoLEDV[AutoStep];
  dmxData[L11M+DMXA] = AutoLEDM[AutoStep];
  dmxData[L12H+DMXA] = AutoLEDH[AutoStep];
  dmxData[L12S+DMXA] = AutoLEDS[AutoStep];
  dmxData[L12V+DMXA] = AutoLEDV[AutoStep];
  dmxData[L12M+DMXA] = AutoLEDM[AutoStep];
  dmxData[FXSel+DMXA] = AutoFX[AutoStep];
  dmxData[FXSpeed+DMXA] = AutoFXSpeed[AutoStep];

}

void Manual() {
  //DMX Control
  dmxData[Pan1+DMXA] = values[4];
  dmxData[Tilt1+DMXA] = values[5];
  dmxData[Disc1+DMXA] = values[6];
  dmxData[L1H+DMXA] = values[7];
  dmxData[L1S+DMXA] = values[8];
  dmxData[L1V+DMXA] = values[9];
  dmxData[L1M+DMXA] = values[10];
  dmxData[L2H+DMXA] = values[7];
  dmxData[L2S+DMXA] = values[8];
  dmxData[L2V+DMXA] = values[9];
  dmxData[L2M+DMXA] = values[10];
  dmxData[L3H+DMXA] = values[7];
  dmxData[L3S+DMXA] = values[8];
  dmxData[L3V+DMXA] = values[9];
  dmxData[L3M+DMXA] = values[10];
  dmxData[L4H+DMXA] = values[7];
  dmxData[L4S+DMXA] = values[8];
  dmxData[L4V+DMXA] = values[9];
  dmxData[L4M+DMXA] = values[10];
  dmxData[L5H+DMXA] = values[7];
  dmxData[L5S+DMXA] = values[8];
  dmxData[L5V+DMXA] = values[9];
  dmxData[L5M+DMXA] = values[10];
  dmxData[L6H+DMXA] = values[7];
  dmxData[L6S+DMXA] = values[8];
  dmxData[L6V+DMXA] = values[9];
  dmxData[L6M+DMXA] = values[10];
  dmxData[L7H+DMXA] = values[7];
  dmxData[L7S+DMXA] = values[8];
  dmxData[L7V+DMXA] = values[9];
  dmxData[L7M+DMXA] = values[10];
  dmxData[L8H+DMXA] = values[7];
  dmxData[L8S+DMXA] = values[8];
  dmxData[L8V+DMXA] = values[9];
  dmxData[L8M+DMXA] = values[10];
  dmxData[L9H+DMXA] = values[7];
  dmxData[L9S+DMXA] = values[8];
  dmxData[L9V+DMXA] = values[9];
  dmxData[L9M+DMXA] = values[10];
  dmxData[L10H+DMXA] = values[7];
  dmxData[L10S+DMXA] = values[8];
  dmxData[L10V+DMXA] = values[9];
  dmxData[L10M+DMXA] = values[10];
  dmxData[L11H+DMXA] = values[7];
  dmxData[L11S+DMXA] = values[8];
  dmxData[L11V+DMXA] = values[9];
  dmxData[L11M+DMXA] = values[10];
  dmxData[L12H+DMXA] = values[7];
  dmxData[L12S+DMXA] = values[8];
  dmxData[L12V+DMXA] = values[9];
  dmxData[L12M+DMXA] = values[10];
  dmxData[FXSel+DMXA] = values[11];
  dmxData[FXSpeed+DMXA] = values[12];

}

// This function handles all button logic
void handleButtons() { //--------------------------------- Menu Buttons

   // Read all inputs in one I2C transaction
  uint8_t inputs = io.getInputRegister();  // Returns the full input register byte

  // Extract button states (assuming active-low buttons; adjust if needed)
  int readingUp = (inputs & (1 << buttonUp)) ? HIGH : LOW;
  int readingDown = (inputs & (1 << buttonDown)) ? HIGH : LOW;
  int readingNext = (inputs & (1 << buttonRight)) ? HIGH : LOW;
  int readingPrev = (inputs & (1 << buttonLeft)) ? HIGH : LOW;
  int readingSave = (inputs & (1 << buttonCenter)) ? HIGH : LOW;

  if (readingUp == HIGH && readingDown == HIGH){
    lastRepeatDelay = millis();
  }

  // If the button state has changed and a debounce time has passed
  if ((readingUp != upButtonState || (millis() - lastRepeatDelay) > repeatDelay) && (millis() - lastDebounceTime) > debounceDelay) {
    if (readingUp == LOW) { // Button was just pressed
    
      lastScreenOff = millis();
      ScreenOff = false;

      if (menuPage == 0){
        values[menuPage] = values[menuPage] + 1;
        if (values[menuPage] > 255){
          values[menuPage] = 1;
        }
      }
      if (menuPage == 1){
        values[menuPage] = values[menuPage] + 1;
        if (values[menuPage] > 63999){
          values[menuPage] = 1;
        }
      }
      if (menuPage == 2){
        values[menuPage] = values[menuPage] + 1;
        if (values[menuPage] > 3){
          values[menuPage] = 1;
        }
      }
      if (menuPage == 3){
        values[menuPage] = values[menuPage] + 1;
        if (values[menuPage] > 3){
          values[menuPage] = 1;
        }
      }
      if (menuPage >= 4 && menuPage <= menuPages - 1){
        values[menuPage] = values[menuPage] + 1;
        if (values[menuPage] > 255){
          values[menuPage] = 1;
        }
      }

      needsRedraw = true;
      lastDebounceTime = millis();
    }
  }
  upButtonState = readingUp;

  if ((readingDown != downButtonState || (millis() - lastRepeatDelay) > repeatDelay) && (millis() - lastDebounceTime) > debounceDelay) {
    if (readingDown == LOW) {

      lastScreenOff = millis();
      ScreenOff = false;

      if (menuPage == 0){
        values[menuPage] = values[menuPage] - 1;
        if (values[menuPage] < 0 ){
          values[menuPage] = 255;
        }
      }
      if (menuPage == 1){
        values[menuPage] = values[menuPage] - 1;
        if (values[menuPage] < 1){
          values[menuPage] = 63999;
        }
      }
      if (menuPage == 2){
        values[menuPage] = values[menuPage] - 1;
        if (values[menuPage] < 1 ){
          values[menuPage] = 3;
        }
      }
      if (menuPage == 3){
        values[menuPage] = values[menuPage] - 1;
        if (values[menuPage] < 1 ){
          values[menuPage] = 3;
        }
      }
      if (menuPage >= 4 && menuPage <= menuPages - 1){
        values[menuPage] = values[menuPage] - 1;
        if (values[menuPage] < 0 ){
          values[menuPage] = 255;
        }
      }

      needsRedraw = true;
      lastDebounceTime = millis();
    }
  }
  downButtonState = readingDown;

  if (readingNext != nextButtonState && (millis() - lastDebounceTime) > debounceDelay) {
    if (readingNext == LOW) {
            
      lastScreenOff = millis();
      ScreenOff = false;

      menuPage = menuPage + 1;
      if (menuPage >= menuPages){
        menuPage = 0;
      }
      needsRedraw = true;
      lastDebounceTime = millis();
    }
  }
  nextButtonState = readingNext;
  
  if (readingPrev != prevButtonState && (millis() - lastDebounceTime) > debounceDelay) {
    if (readingPrev == LOW) {
      
      lastScreenOff = millis();
      ScreenOff = false;

      menuPage = menuPage - 1;
      if (menuPage < 0 ){
        menuPage = menuPages -1;
      }
      needsRedraw = true;
      lastDebounceTime = millis();
    }
  }
  prevButtonState = readingPrev;

  if (readingSave != saveButtonState && (millis() - lastDebounceTime) > debounceDelay) {
    if (readingSave == LOW) {
      
      lastScreenOff = millis();
      ScreenOff = false;

      if (menuPage == 0){
        DMXA = values[0];
        EEPROM.update(0, DMXA);    //Write to EEPROM
      }
      if (menuPage == 1){
        sACNU = values[1];
        EEPROM.put(1, sACNU);    //Write to EEPROM
        joinMulticastGroup(sACNU);
      }
      if (menuPage == 2){
        ControlMode = values[2];
        //EEPROM.update(3, ControlMode);
        if (ControlMode == 3){
          menuPages = 13;
        } else {
          menuPages = 4;
        }
      }
      if (menuPage == 3){
        DMXMode = values[3];
        EEPROM.update(4, DMXMode);
      }

      needsRedraw = true;
      needsInvert = true;
      
      lastDebounceTime = millis();
    }
  }
  saveButtonState = readingSave;
}

// Calculate multicast IP for a given universe
IPAddress getMulticastIP(uint16_t universe) { //--------------------------------- MultiCast joining
  uint8_t highByte = (universe >> 8) & 0xFF;
  uint8_t lowByte = universe & 0xFF;
  return IPAddress(239, 255, highByte, lowByte);
}

// Join a new multicast group
void joinMulticastGroup(uint16_t universe) {
  // Stop the current UDP connection
  udp.stop();

  // Calculate new multicast IP
  IPAddress multicastIP = getMulticastIP(universe);

  // Join the new multicast group
  if (udp.beginMulticast(multicastIP, localPort)) {
    Serial.print("Joined multicast group: ");
    Serial.println(multicastIP);
  } else {
    Serial.println("Failed to join multicast group");
  }
}

void FX() { // List of FX calls
    if (dmxData[FXSel+DMXA] >= 1 && dmxData[FXSel+DMXA] <= 10){
      FXRandomOn(dmxData[FXSpeed+DMXA]); }
    else if (dmxData[FXSel+DMXA] >= 11 && dmxData[FXSel+DMXA] <= 20){
      FXRandomOff(dmxData[FXSpeed+DMXA]); }
    else if (dmxData[FXSel+DMXA] >= 21 && dmxData[FXSel+DMXA] <= 30){
      FXSpinOnSingle(dmxData[FXSpeed+DMXA]); } 
    else if (dmxData[FXSel+DMXA] >= 31 && dmxData[FXSel+DMXA] <= 40){
      FXSpinOnDual(dmxData[FXSpeed+DMXA]); }
    else if (dmxData[FXSel+DMXA] >= 41 && dmxData[FXSel+DMXA] <= 50){
      FXSpinOnTri(dmxData[FXSpeed+DMXA]); }
    else if (dmxData[FXSel+DMXA] >= 51 && dmxData[FXSel+DMXA] <= 60){
      FXSpinOnQuad(dmxData[FXSpeed+DMXA]); }
    else if (dmxData[FXSel+DMXA] >= 61 && dmxData[FXSel+DMXA] <= 70){
      FXSpinOffSingle(dmxData[FXSpeed+DMXA]); }
    else if (dmxData[FXSel+DMXA] >= 71 && dmxData[FXSel+DMXA] <= 80){
      FXSpinOffDual(dmxData[FXSpeed+DMXA]); }
    else if (dmxData[FXSel+DMXA] >= 81 && dmxData[FXSel+DMXA] <= 90){
      FXSpinOffTri(dmxData[FXSpeed+DMXA]); }
    else if (dmxData[FXSel+DMXA] >= 91 && dmxData[FXSel+DMXA] <= 100){
      FXSpinOffQuad(dmxData[FXSpeed+DMXA]); }
    else if (dmxData[FXSel+DMXA] >= 101 && dmxData[FXSel+DMXA] <= 110){
      FXMirrorBounceAll(dmxData[FXSpeed+DMXA]); }
    else if (dmxData[FXSel+DMXA] >= 111 && dmxData[FXSel+DMXA] <= 120){
      FXMirrorBounceAlt(dmxData[FXSpeed+DMXA]); }
    else if (dmxData[FXSel+DMXA] >= 121 && dmxData[FXSel+DMXA] <= 130){
      FXColorRainbowAll(dmxData[FXSpeed+DMXA]); }
    else if (dmxData[FXSel+DMXA] >= 131 && dmxData[FXSel+DMXA] <= 140){
      FXColorRainbowFlow(dmxData[FXSpeed+DMXA]); }
}

void FXRandomOn(char speed){
  FXRandomTime = map(speed, 1, 255, 10, 200);
  if (millis() - FXRandomLast > FXRandomTime){
    FXRandomLast = millis();
    FXRandomIndex = random(12);
  }
  switch (FXRandomIndex){
    case 0: dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 1: dmxData[L1V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 2: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 3: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 4: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 5: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 6: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 7: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 8: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 9: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 10: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L12V] = 0; break;
    case 11: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; break;
  }
}

void FXRandomOff(char speed){
  FXRandomTime = map(speed, 1, 255, 10, 200);
  if (millis() - FXRandomLast > FXRandomTime){
    FXRandomLast = millis();
    FXRandomIndex = random(12);
  }
  switch (FXRandomIndex){
    case 0: dmxData[L1V] = 0; break;
    case 1: dmxData[L2V] = 0; break;
    case 2: dmxData[L3V] = 0; break;
    case 3: dmxData[L4V] = 0; break;
    case 4: dmxData[L5V] = 0; break;
    case 5: dmxData[L6V] = 0; break;
    case 6: dmxData[L7V] = 0; break;
    case 7: dmxData[L8V] = 0; break;
    case 8: dmxData[L9V] = 0; break;
    case 9: dmxData[L10V] = 0; break;
    case 10: dmxData[L11V] = 0; break;
    case 11: dmxData[L12V] = 0; break;
  }
}

void FXSpinOnSingle(char speed){
  if (speed < 128){
    FXSpinTime = map(speed, 1, 127, 200, 10);
  } else {
    FXSpinTime = map(speed, 128, 255, 10, 200);
  }
  
  if (millis() - FXSpinLast > FXSpinTime){
    FXSpinLast = millis();
    
    if (speed < 128){
      FXSpinIndex = FXSpinIndex + 1;
    } else {
      if (FXSpinIndex == 0){
        FXSpinIndex = 11;
      } else {
        FXSpinIndex = FXSpinIndex - 1;
      }
    }
      if (FXSpinIndex >= 12){ FXSpinIndex = 0; }
  }
  switch (FXSpinIndex){
    case 0: dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 1: dmxData[L1V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 2: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 3: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 4: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 5: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 6: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 7: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 8: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 9: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 10: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L12V] = 0; break;
    case 11: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; break;
  }
}

void FXSpinOffSingle(char speed){
  if (speed < 128){
    FXSpinTime = map(speed, 1, 127, 200, 10);
  } else {
    FXSpinTime = map(speed, 128, 255, 10, 200);
  }
  if (millis() - FXSpinLast > FXSpinTime){
    FXSpinLast = millis();
    
    if (speed < 128){
      FXSpinIndex = FXSpinIndex + 1;
    } else {
      if (FXSpinIndex == 0){
        FXSpinIndex = 11;
      } else {
        FXSpinIndex = FXSpinIndex - 1;
      }
    }
      if (FXSpinIndex >= 12){ FXSpinIndex = 0; }
  }
  switch (FXSpinIndex){
    case 0: dmxData[L1V] = 0; break;
    case 1: dmxData[L2V] = 0; break;
    case 2: dmxData[L3V] = 0; break;
    case 3: dmxData[L4V] = 0; break;
    case 4: dmxData[L5V] = 0; break;
    case 5: dmxData[L6V] = 0; break;
    case 6: dmxData[L7V] = 0; break;
    case 7: dmxData[L8V] = 0; break;
    case 8: dmxData[L9V] = 0; break;
    case 9: dmxData[L10V] = 0; break;
    case 10: dmxData[L11V] = 0; break;
    case 11: dmxData[L12V] = 0; break;
  }
}

void FXSpinOnDual(char speed){ // 1/7 1/8 3/9 4/10 5/11 6/12
  if (speed < 128){
    FXSpinTime = map(speed, 1, 127, 200, 10);
  } else {
    FXSpinTime = map(speed, 128, 255, 10, 200);
  }
  if (millis() - FXSpinLast > FXSpinTime){
    FXSpinLast = millis();
    
    if (speed < 128){
      FXSpinIndex = FXSpinIndex + 1;
    } else {
      if (FXSpinIndex == 0){
        FXSpinIndex = 5;
      } else {
        FXSpinIndex = FXSpinIndex - 1;
      }
    }
      if (FXSpinIndex >= 6){ FXSpinIndex = 0; }
  }
  switch (FXSpinIndex){
    case 0: dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 1: dmxData[L1V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 2: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 3: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 4: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L12V] = 0; break;
    case 5: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; break;
  }
}

void FXSpinOffDual(char speed){ // 1/7 1/8 3/9 4/10 5/11 6/12
  if (speed < 128){
    FXSpinTime = map(speed, 1, 127, 200, 10);
  } else {
    FXSpinTime = map(speed, 128, 255, 10, 200);
  }
  if (millis() - FXSpinLast > FXSpinTime){
    FXSpinLast = millis();
    
    if (speed < 128){
      FXSpinIndex = FXSpinIndex + 1;
    } else {
      if (FXSpinIndex == 0){
        FXSpinIndex = 5;
      } else {
        FXSpinIndex = FXSpinIndex - 1;
      }
    }
      if (FXSpinIndex >= 6){ FXSpinIndex = 0; }
  }
  switch (FXSpinIndex){
    case 0: dmxData[L1V] = 0; dmxData[L7V] = 0; break;
    case 1: dmxData[L2V] = 0; dmxData[L8V] = 0; break;
    case 2: dmxData[L3V] = 0; dmxData[L9V] = 0; break;
    case 3: dmxData[L4V] = 0; dmxData[L10V] = 0; break;
    case 4: dmxData[L5V] = 0; dmxData[L11V] = 0; break;
    case 5: dmxData[L6V] = 0; dmxData[L12V] = 0; break;
  }
}

void FXSpinOnTri(char speed){ // 1/5/9 2/6/10 3/7/11 4/8/12
  if (speed < 128){
    FXSpinTime = map(speed, 1, 127, 200, 10);
  } else {
    FXSpinTime = map(speed, 128, 255, 10, 200);
  }
  if (millis() - FXSpinLast > FXSpinTime){
    FXSpinLast = millis();
    
    if (speed < 128){
      FXSpinIndex = FXSpinIndex + 1;
    } else {
      if (FXSpinIndex == 0){
        FXSpinIndex = 3;
      } else {
        FXSpinIndex = FXSpinIndex - 1;
      }
    }
      if (FXSpinIndex >= 4){ FXSpinIndex = 0; }
  }
  switch (FXSpinIndex){
    case 0: dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 1: dmxData[L1V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 2: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L12V] = 0; break;
    case 3: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; break;
  }
}

void FXSpinOffTri(char speed){ // 1/5/9 2/6/10 3/7/11 4/8/12
  if (speed < 128){
    FXSpinTime = map(speed, 1, 127, 200, 10);
  } else {
    FXSpinTime = map(speed, 128, 255, 10, 200);
  }
  if (millis() - FXSpinLast > FXSpinTime){
    FXSpinLast = millis();
    
    if (speed < 128){
      FXSpinIndex = FXSpinIndex + 1;
    } else {
      if (FXSpinIndex == 0){
        FXSpinIndex = 3;
      } else {
        FXSpinIndex = FXSpinIndex - 1;
      }
    }
      if (FXSpinIndex >= 4){ FXSpinIndex = 0; }
  }
  switch (FXSpinIndex){
    case 0: dmxData[L1V] = 0; dmxData[L5V] = 0; dmxData[L9V] = 0; break;
    case 1: dmxData[L2V] = 0; dmxData[L6V] = 0; dmxData[L10V] = 0; break;
    case 2: dmxData[L3V] = 0; dmxData[L7V] = 0; dmxData[L11V] = 0; break;
    case 3: dmxData[L4V] = 0; dmxData[L8V] = 0; dmxData[L12V] = 0; break;
  }
}

void FXSpinOnQuad(char speed){ // 1/4/7/10 2/5/8/11 3/6/9/12
  if (speed < 128){
    FXSpinTime = map(speed, 1, 127, 200, 10);
  } else {
    FXSpinTime = map(speed, 128, 255, 10, 200);
  }
  if (millis() - FXSpinLast > FXSpinTime){
    FXSpinLast = millis();
    
    if (speed < 128){
      FXSpinIndex = FXSpinIndex + 1;
    } else {
      if (FXSpinIndex == 0){
        FXSpinIndex = 2;
      } else {
        FXSpinIndex = FXSpinIndex - 1;
      }
    }
      if (FXSpinIndex >= 3){ FXSpinIndex = 0; }
  }
  switch (FXSpinIndex){
    case 0: dmxData[L2V] = 0; dmxData[L3V] = 0; dmxData[L5V] = 0; dmxData[L6V] = 0; dmxData[L8V] = 0; dmxData[L9V] = 0; dmxData[L11V] = 0; dmxData[L12V] = 0; break;
    case 1: dmxData[L1V] = 0; dmxData[L3V] = 0; dmxData[L4V] = 0; dmxData[L6V] = 0; dmxData[L7V] = 0; dmxData[L9V] = 0; dmxData[L10V] = 0; dmxData[L12V] = 0; break;
    case 2: dmxData[L1V] = 0; dmxData[L2V] = 0; dmxData[L4V] = 0; dmxData[L5V] = 0; dmxData[L7V] = 0; dmxData[L8V] = 0; dmxData[L10V] = 0; dmxData[L11V] = 0; break;
  }
}

void FXSpinOffQuad(char speed){ // 1/4/7/10 2/5/8/11 3/6/9/12
  if (speed < 128){
    FXSpinTime = map(speed, 1, 127, 200, 10);
  } else {
    FXSpinTime = map(speed, 128, 255, 10, 200);
  }
  if (millis() - FXSpinLast > FXSpinTime){
    FXSpinLast = millis();
    
    if (speed < 128){
      FXSpinIndex = FXSpinIndex + 1;
    } else {
      if (FXSpinIndex == 0){
        FXSpinIndex = 2;
      } else {
        FXSpinIndex = FXSpinIndex - 1;
      }
    }
      if (FXSpinIndex >= 3){ FXSpinIndex = 0; }
  }
  switch (FXSpinIndex){
    case 0: dmxData[L1V] = 0; dmxData[L4V] = 0; dmxData[L7V] = 0; dmxData[L10V] = 0; break;
    case 1: dmxData[L2V] = 0; dmxData[L5V] = 0; dmxData[L8V] = 0; dmxData[L11V] = 0; break;
    case 2: dmxData[L3V] = 0; dmxData[L6V] = 0; dmxData[L9V] = 0; dmxData[L12V] = 0; break;
  }
}

void FXMirrorBounceAll(char speed){
  FXMirrorTime = map(speed, 1, 255, 100, 4000);
  speed = speed / 2;
  int M1, M2, M3, M4, M5, M6, M7, M8, M9, M10, M11, M12;
  if (millis() - FXMirrorLast > FXMirrorTime){
    FXMirrorLast = millis();
    if (FXMirrorIndex == 0){ FXMirrorIndex = 1; } else { FXMirrorIndex = 0; }
  }
  switch (FXMirrorIndex){
    case 0: M1 = dmxData[L1M] + speed; M2 = dmxData[L2M] + speed; M3 = dmxData[L3M] + speed; M4 = dmxData[L4M] + speed; M5 = dmxData[L5M] + speed; M6 = dmxData[L6M] + speed; M7 = dmxData[L7M] + speed; M8 = dmxData[L8M] + speed; M9 = dmxData[L9M] + speed; M10 = dmxData[L10M] + speed; M11 = dmxData[L11M] + speed; M12 = dmxData[L12M] + speed; break;
    case 1: M1 = dmxData[L1M] - speed; M2 = dmxData[L2M] - speed; M3 = dmxData[L3M] - speed; M4 = dmxData[L4M] - speed; M5 = dmxData[L5M] - speed; M6 = dmxData[L6M] - speed; M7 = dmxData[L7M] - speed; M8 = dmxData[L8M] - speed; M9 = dmxData[L9M] - speed; M10 = dmxData[L10M] - speed; M11 = dmxData[L11M] - speed; M12 = dmxData[L12M] - speed; break;
  }
  dmxData[L1M] = constrain(M1, 0, 255);
  dmxData[L2M] = constrain(M2, 0, 255);
  dmxData[L3M] = constrain(M3, 0, 255);
  dmxData[L4M] = constrain(M4, 0, 255);
  dmxData[L5M] = constrain(M5, 0, 255);
  dmxData[L6M] = constrain(M6, 0, 255);
  dmxData[L7M] = constrain(M7, 0, 255);
  dmxData[L8M] = constrain(M8, 0, 255);
  dmxData[L9M] = constrain(M9, 0, 255);
  dmxData[L10M] = constrain(M10, 0, 255);
  dmxData[L11M] = constrain(M11, 0, 255);
  dmxData[L12M] = constrain(M12, 0, 255);
}

void FXMirrorBounceAlt(char speed){
  FXMirrorTime = map(speed, 1, 255, 100, 4000);
  speed = speed / 2;
  int M1, M2, M3, M4, M5, M6, M7, M8, M9, M10, M11, M12;
  if (millis() - FXMirrorLast > FXMirrorTime){
    FXMirrorLast = millis();
    if (FXMirrorIndex == 0){ FXMirrorIndex = 1; } else { FXMirrorIndex = 0; }
  }
  switch (FXMirrorIndex){
    case 0: M1 = dmxData[L1M] + speed; M2 = dmxData[L2M] - speed; M3 = dmxData[L3M] + speed; M4 = dmxData[L4M] - speed; M5 = dmxData[L5M] + speed; M6 = dmxData[L6M] - speed; M7 = dmxData[L7M] + speed; M8 = dmxData[L8M] - speed; M9 = dmxData[L9M] + speed; M10 = dmxData[L10M] - speed; M11 = dmxData[L11M] + speed; M12 = dmxData[L12M] - speed; break;
    case 1: M1 = dmxData[L1M] - speed; M2 = dmxData[L2M] + speed; M3 = dmxData[L3M] - speed; M4 = dmxData[L4M] + speed; M5 = dmxData[L5M] - speed; M6 = dmxData[L6M] + speed; M7 = dmxData[L7M] - speed; M8 = dmxData[L8M] + speed; M9 = dmxData[L9M] - speed; M10 = dmxData[L10M] + speed; M11 = dmxData[L11M] - speed; M12 = dmxData[L12M] + speed; break;
  }
  dmxData[L1M] = constrain(M1, 0, 255);
  dmxData[L2M] = constrain(M2, 0, 255);
  dmxData[L3M] = constrain(M3, 0, 255);
  dmxData[L4M] = constrain(M4, 0, 255);
  dmxData[L5M] = constrain(M5, 0, 255);
  dmxData[L6M] = constrain(M6, 0, 255);
  dmxData[L7M] = constrain(M7, 0, 255);
  dmxData[L8M] = constrain(M8, 0, 255);
  dmxData[L9M] = constrain(M9, 0, 255);
  dmxData[L10M] = constrain(M10, 0, 255);
  dmxData[L11M] = constrain(M11, 0, 255);
  dmxData[L12M] = constrain(M12, 0, 255);
}

void FXColorRainbowAll(char speed){
  if (speed < 128){
    FXColorTime = map(speed, 1, 127, 1000, 1);
  } else {
    FXColorTime = map(speed, 128, 255, 1, 1000);
  }
  if (millis() - FXColorLast > FXColorTime){
    FXColorLast = millis();
    
    if (speed < 128){
      FXColorIndex = FXColorIndex + 1;
      if (FXSpinIndex >= 255){ FXSpinIndex = 0; }
    } else {
      if (FXColorIndex == 0){
        FXColorIndex = 255;
      } else {
        FXColorIndex = FXColorIndex - 1;
      }
    }
  }
  dmxData[L1H] = FXColorIndex;
  dmxData[L2H] = FXColorIndex;
  dmxData[L3H] = FXColorIndex;
  dmxData[L4H] = FXColorIndex;
  dmxData[L5H] = FXColorIndex;
  dmxData[L6H] = FXColorIndex;
  dmxData[L7H] = FXColorIndex;
  dmxData[L8H] = FXColorIndex;
  dmxData[L9H] = FXColorIndex;
  dmxData[L10H] = FXColorIndex;
  dmxData[L11H] = FXColorIndex;
  dmxData[L12H] = FXColorIndex;
}

void FXColorRainbowFlow(char speed){
  if (speed < 128){
    FXColorTime = map(speed, 1, 127, 1000, 1);
  } else {
    FXColorTime = map(speed, 128, 255, 1, 1000);
  }
  if (millis() - FXColorLast > FXColorTime){
    FXColorLast = millis();
    
    if (speed < 128){
      FXColorIndex = FXColorIndex + 1;
      if (FXSpinIndex >= 256){ FXSpinIndex = 0; }
    } else {
      if (FXColorIndex == 0){
        FXColorIndex = 255;
      } else {
        FXColorIndex = FXColorIndex - 1;
      }
    }
  }
  dmxData[L1H] = FXColorIndex;
  dmxData[L2H] = (FXColorIndex + 21) % 255;
  dmxData[L3H] = (FXColorIndex + 42) % 255;
  dmxData[L4H] = (FXColorIndex + 63) % 255;
  dmxData[L5H] = (FXColorIndex + 84) % 255;
  dmxData[L6H] = (FXColorIndex + 105) % 255;
  dmxData[L7H] = (FXColorIndex + 126) % 255;
  dmxData[L8H] = (FXColorIndex + 147) % 2555;
  dmxData[L9H] = (FXColorIndex + 168) % 255;
  dmxData[L10H] = (FXColorIndex + 189) % 255;
  dmxData[L11H] = (FXColorIndex + 210) % 255;
  dmxData[L12H] = (FXColorIndex + 231) % 255;
}
