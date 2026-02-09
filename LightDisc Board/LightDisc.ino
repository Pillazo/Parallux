#include <WiFi.h>
#include <Arduino.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_PWMServoDriver.h>

float Version = 1.16;
/*
Version
1.0 - basic working
1.1 - too slow between chips, changed serial recieve logic
1.11 - added self-healing to serial stream
1.12 - made it 115200 baud instead of 38400, also moved the serial send up, above the I2C stuff.
1.13 - dropped back to 38400 cause of communication issues, may go back up, there's some bauds between, also could have been another noise issue.
1.14 - Moved Serial recieve to after homing sequence, added 100ms timing to thermals.
1.15 - reduced freq of pwm frpm 1000 to 200, changed serial size from 65 to 49.
1.16 - back up to 400k i2c baud
*/


/*
ESP32-C3 - Configured as 'Adafruit QT Py ESP32-C3'
  X - 5 -- 5V
  X - 6    GND
  X - 7    3.3V
SDA - 8    4 - X
SCL - 9    3 - X
 X - 10    2 - Temp 3
 RX -20    1 - Temp 2
 TX -21 -- 0 - Temp 1

*/

//Thermo Variables
const unsigned long ThermalTiming = 100;
unsigned long lastThermal1 = 0;
unsigned long lastThermal2 = 0;
unsigned long lastThermal3 = 0;
int ThermistorPin1 = 0;
int ThermistorPin2 = 1;
int ThermistorPin3 = 2;
int Vo;
float Res1 = 10000;
float Res2 = 10000;
float Res3 = 10000;
float ratio;
float logR2, R2, T1, T2, T3;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
float TLimit = 80.0;
char TempPacket[13];
char t1Str[5], t2Str[5], t3Str[5];

//PWM for LEDs
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
const int res = 16; //12-bit PWM vs the DMX 8 bit, so 16 is the divisor 
#define LEDR1 0 //Pin 6
#define LEDG1 1 //Pin 7
#define LEDB1 2 //Pin 8
#define LEDW1 3 //Pin 9
#define LEDR2 11 //Pin 18
#define LEDG2 10 //Pin 17
#define LEDB2 9 //Pin 16
#define LEDW2 8 //Pin 15
#define LEDR3 4 //Pin 10
#define LEDG3 5 //Pin 11
#define LEDB3 6 //Pin 12
#define LEDW3 7 //Pin 13
uint8_t LEDW1Power = 1; //Intensity of each output, as given by the DMX
uint8_t LEDR1Power = 1;
uint8_t LEDG1Power = 1;
uint8_t LEDB1Power = 1;
uint8_t LEDW2Power = 1;
uint8_t LEDR2Power = 1;
uint8_t LEDG2Power = 1;
uint8_t LEDB2Power = 1;
uint8_t LEDW3Power = 1;
uint8_t LEDR3Power = 1;
uint8_t LEDG3Power = 1;
uint8_t LEDB3Power = 1;
uint8_t H1 = 0; // DMX Input Hue
uint8_t S1 = 0; // DMX Input Saturation
uint8_t V1 = 0; // DMX Input Value
uint8_t H2 = 0; // DMX Input Hue
uint8_t S2 = 0; // DMX Input Saturation
uint8_t V2 = 0; // DMX Input Value
uint8_t H3 = 0; // DMX Input Hue
uint8_t S3 = 0; // DMX Input Saturation
uint8_t V3 = 0; // DMX Input Value

//Stepper
Adafruit_MCP23X17 mcp;
#define SS11 0 //GPA0 - Pin 21
#define SS12 1 //GPA1 - Pin 22
#define SS13 2 //GPA2 - Pin 23
#define SS14 3 //GPA3 - Pin 24
#define SS21 4 //GPA4 - Pin 25
#define SS22 5 //GPA5 - Pin 26
#define SS23 6 //GPA6 - Pin 27
#define SS24 7 //GPA7 - Pin 28
#define SS31 8 //GPB0 - Pin 0
#define SS32 9 //GPB1 - Pin 1
#define SS33 10 //GPB2 - Pin 2
#define SS34 11 //GPB3 - Pin 3
int SS1step = 0;
bool Stepper1Homed = false;
int Step1CMD = 0;
int Step1Pos = 1023;
int Step1Dif = 0;
int SS2step = 0;
bool Stepper2Homed = false;
int Step2CMD = 0;
int Step2Pos = 1023;
int Step2Dif = 0;
int SS3step = 0;
bool Stepper3Homed = false;
int Step3CMD = 0;
int Step3Pos = 1023;
int Step3Dif = 0;
const unsigned long StepTiming = 1;
unsigned long lastStep1Time = 0;
unsigned long lastStep2Time = 0;
unsigned long lastStep3Time = 0;
int StepsPerDMX = 4; //8 probably full rotation
int StepsScaled = 0;

//DMX Data
#define BUFFER_SIZE 49  // Define the maximum buffer size
bool newData = false;
int bufferIndex = 0;
char serialBuffer[BUFFER_SIZE];
char sBuffer[70];
int n;

void setup() {
  WiFi.mode(WIFI_OFF);
  btStop();
  Wire.begin(8, 9);
  Wire.setClock(400000);
  Serial.begin(9600);
  delay(500);
  Serial.println("Begin");
  Serial.print("Version: ");
  Serial.println(Version);
  Serial0.begin(38400); //115200
  pinMode(ThermistorPin1, INPUT); 
  pinMode(ThermistorPin2, INPUT); 
  pinMode(ThermistorPin3, INPUT); 
  
  if (!mcp.begin_I2C()) {
    Serial.println("MCP Error.");
  }

  mcp.pinMode(SS11, OUTPUT);
  mcp.pinMode(SS12, OUTPUT);
  mcp.pinMode(SS13, OUTPUT);
  mcp.pinMode(SS14, OUTPUT);
  mcp.pinMode(SS21, OUTPUT);
  mcp.pinMode(SS22, OUTPUT);
  mcp.pinMode(SS23, OUTPUT);
  mcp.pinMode(SS24, OUTPUT);
  mcp.pinMode(SS31, OUTPUT);
  mcp.pinMode(SS32, OUTPUT);
  mcp.pinMode(SS33, OUTPUT);
  mcp.pinMode(SS34, OUTPUT);

  if (!pwm.begin()){
    Serial.println("PCA Error");
  }

  pwm.setPWMFreq(200);
  pwm.setPWM(LEDR1, 0, 4096); //All off
  pwm.setPWM(LEDG1, 0, 4096); 
  pwm.setPWM(LEDB1, 0, 4096); 
  pwm.setPWM(LEDW1, 0, 4096); 
  pwm.setPWM(LEDR2, 0, 4096); 
  pwm.setPWM(LEDG2, 0, 4096); 
  pwm.setPWM(LEDB2, 0, 4096); 
  pwm.setPWM(LEDW2, 0, 4096); 
  pwm.setPWM(LEDR3, 0, 4096); 
  pwm.setPWM(LEDG3, 0, 4096); 
  pwm.setPWM(LEDB3, 0, 4096); 
  pwm.setPWM(LEDW3, 0, 4096); 


}

void loop() {
  // Temp read
    if (millis() - lastThermal1 > ThermalTiming) {
  lastThermal1 = millis();
  Vo = analogRead(ThermistorPin1);
  R2 = Res1 * (4095.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T1 = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T1 = T1 - 273.15;
  dtostrf(T1, 4, 1, t1Str);
  //T1 = (T1 * 9.0)/ 5.0 + 32.0; 
  }
  
    if (millis() - lastThermal2 > ThermalTiming) {
  lastThermal2 = millis();
  Vo = analogRead(ThermistorPin2);
  R2 = Res2 * (4095.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T2 = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T2 = T2 - 273.15;
  dtostrf(T2, 4, 1, t2Str);
  //T2 = (T2 * 9.0)/ 5.0 + 32.0;
  }
  
    if (millis() - lastThermal3 > ThermalTiming) {
  lastThermal3 = millis();
  Vo = analogRead(ThermistorPin3); // Different logic because of accidental extra resistor! Remove when fixed!
  ratio = (float)Vo / (4095.0 - (float)Vo);
  R2 = 1.0 / ((ratio / Res3) - (1.0 / 10000.0));
  if (R2 < 0) R2 = 1.0;
  logR2 = log(R2);
  T3 = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T3 = T3 - 273.15 - 10;
  dtostrf(T3, 4, 1, t3Str);
  //T3 = (T3 * 9.0)/ 5.0 + 32.0;
  }

  // Read serial data non-blocking
  if (Serial0.available() > 0 && Stepper1Homed == true && Stepper2Homed == true && Stepper3Homed == true) {
    if (Serial0.peek() == 'D') {
      n = Serial0.readBytes(serialBuffer, BUFFER_SIZE);    
      newData = true;
    }
    else {
      Serial0.read();
    }
  }

  if (newData && Stepper1Homed == true && Stepper2Homed == true && Stepper3Homed == true) {

     if (serialBuffer[0] == 'D') { //6 for 'D' then then Hue, Saturation and Value channels, then 1 for stepper. Repeat HSV+Stepper for the other 2 LEDs.
      H1 = (unsigned char)serialBuffer[1];
      S1 = (unsigned char)serialBuffer[2];
      V1 = (unsigned char)serialBuffer[3];
      Step1CMD = (unsigned char)serialBuffer[4];
      H2 = (unsigned char)serialBuffer[5];
      S2 = (unsigned char)serialBuffer[6];
      V2 = (unsigned char)serialBuffer[7];
      Step2CMD = (unsigned char)serialBuffer[8];
      H3 = (unsigned char)serialBuffer[9];
      S3 = (unsigned char)serialBuffer[10];
      V3 = (unsigned char)serialBuffer[11];
      Step3CMD = (unsigned char)serialBuffer[12];

      //Send data first
      snprintf(TempPacket, sizeof(TempPacket), "%s%s%s", t1Str, t2Str, t3Str); // Temps to char

      sBuffer[0] = 'D';              // Start with 'D'
      memcpy(&sBuffer[1], &serialBuffer[13], 36); // Middle is other data for / from boards of DMX or Temp
      memcpy(&sBuffer[37], &TempPacket[0], 12); // Add temp from this board
      
      Serial0.write(sBuffer, 49);
      //Serial.println(sBuffer);

      hsvToRgbw(H1, S1, V1, &LEDR1Power, &LEDG1Power, &LEDB1Power, &LEDW1Power);
      hsvToRgbw(H2, S2, V2, &LEDR2Power, &LEDG2Power, &LEDB2Power, &LEDW2Power);
      hsvToRgbw(H3, S3, V3, &LEDR3Power, &LEDG3Power, &LEDB3Power, &LEDW3Power);
      
      if (T1 < TLimit && T1 > 0) {  //If within temp limit, allow change
        pwm.setPWM(LEDR1, 0, LEDR1Power * res); 
        pwm.setPWM(LEDG1, 0, LEDG1Power * res); 
        pwm.setPWM(LEDB1, 0, LEDB1Power * res); 
        pwm.setPWM(LEDW1, 0, LEDW1Power * res); 
      }
      if (T2 < TLimit && T2 > 0) {
        pwm.setPWM(LEDR2, 0, LEDR2Power * res); 
        pwm.setPWM(LEDG2, 0, LEDG2Power * res); 
        pwm.setPWM(LEDB2, 0, LEDB2Power * res); 
        pwm.setPWM(LEDW2, 0, LEDW2Power * res); 
      }
      if (T3 < TLimit && T3 > 0) {
        pwm.setPWM(LEDR3, 0, LEDR3Power * res); 
        pwm.setPWM(LEDG3, 0, LEDG3Power * res); 
        pwm.setPWM(LEDB3, 0, LEDB3Power * res); 
        pwm.setPWM(LEDW3, 0, LEDW3Power * res); 
      }
    }

    // Reset buffer and flag
    newData = false;

  }

//Either too hot or unconnected
  if (T1 >= TLimit || T1 < 0) {
    pwm.setPWM(LEDR1, 0, 4096); 
    pwm.setPWM(LEDG1, 0, 4096); 
    pwm.setPWM(LEDB1, 0, 4096); 
    pwm.setPWM(LEDW1, 0, 4096); 
  }
  if (T2 >= TLimit || T2 < 0) {
    pwm.setPWM(LEDR2, 0, 4096); 
    pwm.setPWM(LEDG2, 0, 4096); 
    pwm.setPWM(LEDB2, 0, 4096); 
    pwm.setPWM(LEDW2, 0, 4096); 
  }
  if (T3 >= TLimit || T3 < 0) {
    pwm.setPWM(LEDR3, 0, 4096); 
    pwm.setPWM(LEDG3, 0, 4096); 
    pwm.setPWM(LEDB3, 0, 4096); 
    pwm.setPWM(LEDW3, 0, 4096); 
  }

  if (millis() - lastStep1Time > StepTiming) {
  lastStep1Time = millis();
    MoveStepper1();
  }
  if (millis() - lastStep2Time > StepTiming) {
  lastStep2Time = millis();
    MoveStepper2();
  }
  if (millis() - lastStep3Time > StepTiming) {
  lastStep3Time = millis();
    MoveStepper3();
  }

  if (Step1Pos == 0){
    Stepper1Homed = true;
  }
  if (Step2Pos == 0){
    Stepper2Homed = true;
  }
  if (Step3Pos == 0){
    Stepper3Homed = true;
  }

}


void MoveStepper1() {
  StepsScaled = Step1CMD * StepsPerDMX;
  Step1Dif = StepsScaled - Step1Pos;
  if (Step1Dif == 0){
    return;
  }
  else if (Step1Dif < 0){
    SS1step = SS1step - 1;
    if (SS1step < 0) { SS1step = 3;}
    Step1Pos = Step1Pos - 1;
  }
  else if (Step1Dif > 0){
    SS1step = SS1step + 1;
    if (SS1step > 3) { SS1step = 0;}
    Step1Pos = Step1Pos + 1;
  }

  if (SS1step == 0) {
    mcp.digitalWrite(SS11, HIGH);
    mcp.digitalWrite(SS12, LOW);
    mcp.digitalWrite(SS13, LOW);
    mcp.digitalWrite(SS14, HIGH);
  }
  else if (SS1step == 1) {
    mcp.digitalWrite(SS11, LOW);
    mcp.digitalWrite(SS12, LOW);
    mcp.digitalWrite(SS13, HIGH);
    mcp.digitalWrite(SS14, HIGH);
  }
  else if (SS1step == 2) {
    mcp.digitalWrite(SS11, LOW);
    mcp.digitalWrite(SS12, HIGH);
    mcp.digitalWrite(SS13, HIGH);
    mcp.digitalWrite(SS14, LOW);
  }
  else if (SS1step == 3) {
    mcp.digitalWrite(SS11, HIGH);
    mcp.digitalWrite(SS12, HIGH);
    mcp.digitalWrite(SS13, LOW);
    mcp.digitalWrite(SS14, LOW);
  }
}

void MoveStepper2() {
  StepsScaled = Step2CMD * StepsPerDMX;
  Step2Dif = StepsScaled - Step2Pos;
  if (Step2Dif == 0){
    return;
  }
  else if (Step2Dif < 0){
    SS2step = SS2step - 1;
    if (SS2step < 0) { SS2step = 3;}
    Step2Pos = Step2Pos - 1;
  }
  else if (Step2Dif > 0){
    SS2step = SS2step + 1;
    if (SS2step > 3) { SS2step = 0;}
    Step2Pos = Step2Pos + 1;
  }

  if (SS2step == 0) {
    mcp.digitalWrite(SS21, HIGH);
    mcp.digitalWrite(SS22, LOW);
    mcp.digitalWrite(SS23, LOW);
    mcp.digitalWrite(SS24, HIGH);
  }
  else if (SS2step == 1) {
    mcp.digitalWrite(SS21, LOW);
    mcp.digitalWrite(SS22, LOW);
    mcp.digitalWrite(SS23, HIGH);
    mcp.digitalWrite(SS24, HIGH);
  }
  else if (SS2step == 2) {
    mcp.digitalWrite(SS21, LOW);
    mcp.digitalWrite(SS22, HIGH);
    mcp.digitalWrite(SS23, HIGH);
    mcp.digitalWrite(SS24, LOW);
  }
  else if (SS2step == 3) {
    mcp.digitalWrite(SS21, HIGH);
    mcp.digitalWrite(SS22, HIGH);
    mcp.digitalWrite(SS23, LOW);
    mcp.digitalWrite(SS24, LOW);
  }
}

void MoveStepper3() {
  StepsScaled = Step3CMD * StepsPerDMX;
  Step3Dif = StepsScaled - Step3Pos;
  if (Step3Dif == 0){
    return;
  }
  else if (Step3Dif < 0){
    SS3step = SS3step - 1;
    if (SS3step < 0) { SS3step = 3;}
    Step3Pos = Step3Pos - 1;
  }
  else if (Step3Dif > 0){
    SS3step = SS3step + 1;
    if (SS3step > 3) { SS3step = 0;}
    Step3Pos = Step3Pos + 1;
  }

  if (SS3step == 0) {
    mcp.digitalWrite(SS31, HIGH);
    mcp.digitalWrite(SS32, LOW);
    mcp.digitalWrite(SS33, LOW);
    mcp.digitalWrite(SS34, HIGH);
  }
  else if (SS3step == 1) {
    mcp.digitalWrite(SS31, LOW);
    mcp.digitalWrite(SS32, LOW);
    mcp.digitalWrite(SS33, HIGH);
    mcp.digitalWrite(SS34, HIGH);
  }
  else if (SS3step == 2) {
    mcp.digitalWrite(SS31, LOW);
    mcp.digitalWrite(SS32, HIGH);
    mcp.digitalWrite(SS33, HIGH);
    mcp.digitalWrite(SS34, LOW);
  }
  else if (SS3step == 3) {
    mcp.digitalWrite(SS31, HIGH);
    mcp.digitalWrite(SS32, HIGH);
    mcp.digitalWrite(SS33, LOW);
    mcp.digitalWrite(SS34, LOW);
  }
}

//Absolutely copied from Gemini lol ty ty
void hsvToRgbw(uint8_t h_byte, uint8_t s_byte, uint8_t v_byte,
               uint8_t* r_out, uint8_t* g_out, uint8_t* b_out, uint8_t* w_out) {
   
    // FIX 1: Early exit for S=0 (Pure White/Gray)
    // When saturation is zero, the result is achromatic (gray).
    // The Value (V) component directly controls the White LED intensity.
    if (s_byte == 0) {
        *r_out = 0;
        *g_out = 0;
        *b_out = 0;
        *w_out = v_byte; // W = V (guaranteed full white at V=255)
        return;
    }
   
    // --- Part 1: Hue to Pure RGB (R_pure, G_pure, B_pure) ---

    // Segment division. 256/6 = 42.66. We use 42 for integer segment width.
    const uint8_t segment_width = 42;
    uint8_t segment_index = h_byte / segment_width; // 0 to 5, or 6
    uint8_t segment_pos = h_byte % segment_width;   // 0 to 41, or 0 to 3 for index 6

    // Calculate Rising and Falling components (0-255 scale)
    uint8_t rising_component = (uint8_t)(((long)segment_pos * 255) / segment_width);
    uint8_t falling_component = 255 - rising_component;

    uint8_t R_pure, G_pure, B_pure;

    // Assign R, G, B based on the 6 segments (assuming Max V)
    switch (segment_index) {
        case 0: R_pure = 255; G_pure = rising_component;   B_pure = 0;   break; // R -> Y
        case 1: R_pure = falling_component;  G_pure = 255; B_pure = 0;   break; // Y -> G
        case 2: R_pure = 0;   G_pure = 255; B_pure = rising_component;   break; // G -> C
        case 3: R_pure = 0;   G_pure = falling_component;  B_pure = 255; break; // C -> B
        case 4: R_pure = rising_component;   G_pure = 0;   B_pure = 255; break; // B -> M
        case 5: R_pure = 255; G_pure = 0;   B_pure = falling_component; break; // M -> R
        // Handle the remaining 252-255 range as Max Red
        default: R_pure = 255; G_pure = 0;   B_pure = 0; break;
    }

    // --- Part 2: Saturation Blending (HSV -> Target RGB) ---
   
    // 1. Calculate the chromatic (pure color) components scaled by Saturation (S)
    uint8_t S_chroma = s_byte;
   
    // R_mid, G_mid, B_mid are the primary colors scaled down by S
    uint8_t R_mid = (uint8_t)(((long)R_pure * S_chroma) / 255);
    uint8_t G_mid = (uint8_t)(((long)G_pure * S_chroma) / 255);
    uint8_t B_mid = (uint8_t)(((long)B_pure * S_chroma) / 255);

    // 2. Calculate the gray/white component scaled by Inverse Saturation (1-S)
    uint8_t W_gray = 255 - S_chroma;

    // 3. Combine to get the Target RGB (R_target, G_target, B_target)
    uint8_t R_target = R_mid + W_gray;
    uint8_t G_target = G_mid + W_gray;
    uint8_t B_target = B_mid + W_gray;

    // --- Part 3: White Extraction (Target RGB -> RGBW Split) ---

    // 1. Find the component that can be pulled to the dedicated White LED
    uint8_t W_extract = min(R_target, min(G_target, B_target));

    // 2. Separate Chromatic and White components
    uint8_t R_color = R_target - W_extract; // Pure color left over for R
    uint8_t G_color = G_target - W_extract; // Pure color left over for G
    uint8_t B_color = B_target - W_extract; // Pure color left over for B
    uint8_t W_final = W_extract;            // Dedicated White LED component

    // --- Part 4: Apply Value (V) Dimming ---
   
    // The V_byte acts as a master brightness scaler for all four final channels.
   
    *r_out = (uint8_t)(((long)R_color * v_byte) / 255);
    *g_out = (uint8_t)(((long)G_color * v_byte) / 255);
    *b_out = (uint8_t)(((long)B_color * v_byte) / 255);
    *w_out = (uint8_t)(((long)W_final * v_byte) / 255);
}