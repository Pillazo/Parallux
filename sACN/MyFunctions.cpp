// MyFunctions.cpp
#include <Arduino.h>
#include "MyFunctions.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Screen configuration
  #define SCREEN_WIDTH 128
  #define SCREEN_HEIGHT 64
  #define OLED_RESET -1
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool DisplaySetup() {
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    return false;
  }

}

void DisplayOff() {
    display.clearDisplay();
    display.display();
}

void DisplayEthernet(int stage) {
  if (stage == 0){
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("Starting Ethernet"));
    display.display();
  }
  if (stage == 1){    
    display.println(F("Ethernet Start Finish"));
    display.display();
  }
}

void DisplayInvert(bool invert) {
  display.invertDisplay(invert);
}

void DisplayDraw(int menuPage, const char* menuHeaders[], int* values, int* menuType) {

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Draw header
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(menuHeaders[menuPage]);
  //display.drawFastHLine(0, 14, display.width(), SSD1306_WHITE);
  
  // Integer Menu Type
  if (menuType[menuPage] == 0){
    // Convert value to string and center it
    char numberStr[10]; // Buffer to hold string version of number
    sprintf(numberStr, "%d", values[menuPage]); // Convert number to string
    int16_t xtext, ytext;
    uint16_t textWidth, textHeight;
    display.getTextBounds(numberStr, 0, 0, &xtext, &ytext, &textWidth, &textHeight);
    int cursorX = (SCREEN_WIDTH - textWidth) / 2; // Center horizontally
    int cursorY = (SCREEN_HEIGHT - textHeight) / 2; // Center vertically
    display.setCursor(cursorX, cursorY - 3);
    display.println(numberStr);
  }

  // Yes or No Menu Type
  if (menuType[menuPage] == 1){
    display.setCursor(0,22);
    if (values[menuPage] == 1){
    display.println("    Yes");
    }
    else {
    display.println("    No");
    }
  }

  //Control Mode Menu
  if (menuType[menuPage] == 2){
    display.setCursor(0,22);
    if (values[menuPage] == 1){
    display.println("   sACN   ");
    }
    else if (values[menuPage] == 2){
    display.println("   Auto   ");
    }
    else if (values[menuPage] == 3){
    display.println("  Manual  ");
    }
  }

  //DMX Mode Menu
  if (menuType[menuPage] == 3){
    display.setCursor(0,22);
    if (values[menuPage] == 1){
    display.println("   Full   ");
    }
    else if (values[menuPage] == 2){
    display.println("  Single  ");
    }
    else if (values[menuPage] == 3){
    display.println("EveryOther");
    }
  }


  //Buttons
   // Define the bottom third (y=43 to y=64)
  int boxHeight = 21; // Bottom third height
  int boxWidth = SCREEN_WIDTH / 4; // 32 pixels per box
  int startY = SCREEN_HEIGHT - boxHeight; // y=43
  
  // Draw four boxes
  for (int i = 0; i < 4; i++) {
    int x = i * boxWidth;
    display.drawRect(x, startY, boxWidth, boxHeight, SSD1306_WHITE);
  }
  
  // Draw Up Triangle in first box (centered)
  int x1 = boxWidth / 2; // Center of first box (x=16)
  int y1 = startY + boxHeight / 2; // Center of box vertically (y=53)
  display.drawTriangle(x1 - 8, y1 + 6, x1, y1 - 6, x1 + 8, y1 + 6, SSD1306_WHITE);
  
  // Draw Down Triangle in second box
  int x2 = boxWidth + boxWidth / 2; // Center of second box (x=48)
  display.drawTriangle(x2 - 8, y1 - 6, x2, y1 + 6, x2 + 8, y1 - 6, SSD1306_WHITE);
  
  // Draw Right Triangle in third box
  int x3 = 2 * boxWidth + boxWidth / 2; // Center of third box (x=80)
  display.drawTriangle(x3 - 6, y1 - 6, x3 + 6, y1, x3 - 6, y1 + 6, SSD1306_WHITE);
  
  // Draw Back Arrow in fourth box
  int x4 = 3 * boxWidth + boxWidth / 2; // Center of fourth box (x=112)
  display.drawLine(x4 - 6, y1, x4 + 6, y1, SSD1306_WHITE); // Horizontal line
  display.drawLine(x4 - 6, y1, x4, y1 - 4, SSD1306_WHITE); // Top diagonal
  display.drawLine(x4 - 6, y1, x4, y1 + 4, SSD1306_WHITE); // Bottom diagonal
  
  
  display.display();
}