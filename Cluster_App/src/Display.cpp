#include "Display.h"

TFT_eSPI tft = TFT_eSPI();

void Display_Init(TFT_eSPI &tft)
{
    tft.init();
    tft.setRotation(2);
    // Fill screen with grey so we can see the effect of printing with and without 
    // a background colour defined
    tft.fillScreen(TFT_GREY);

    // Set "cursor" at top left corner of display (0,0) and select font 2
    // (cursor will move to next line automatically during printing with 'tft.println'
    //  or stay on the line is there is room for the text with tft.print)
    tft.setCursor(0, 0, 2);
    // Set the font colour to be white with a black background, set text size multiplier to 1
    tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);

}


void DisplayWrite(TFT_eSPI &tft, const char* ifsh)
{
    tft.println(ifsh);
}

void DisplayWrite(TFT_eSPI &tft, long value,int base)
{
    tft.println(value, base);
} 
void DisplayWrite(TFT_eSPI &tft, int value)
{
    tft.println(value);
}
void DisplayPrint(TFT_eSPI &tft, char value)
{
     tft.print(value);
}
void DisplayPrint(TFT_eSPI &tft, int value)
{
    tft.print(value);
}
void DisplayPrint(TFT_eSPI &tft, const char* value)
{
   tft.print(value);
}
void DisplayPrint(TFT_eSPI &tft, long value, int base)
{
    tft.print(value, base);
}
void DisplayPrint(TFT_eSPI &tft, int value, int base)
{
    tft.print(value, base);
}