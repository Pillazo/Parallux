// MyFunctions.h
#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

#include <Adafruit_SSD1306.h>

bool DisplaySetup();
void DisplayDraw(int menuPage, const char* menuHeaders[], int* values, int* menuType);
void DisplayEthernet(int stage);
void DisplayInvert(bool invert);
void joinMulticastGroup(uint16_t universe);
void handleButtons();
void DisplayOff();

#endif