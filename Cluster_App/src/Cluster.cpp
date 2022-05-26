#include "Cluster.h"


TFT_eSPI *clusterTft;

float       ltx;    // Saved x coord of bottom of needle
uint16_t    osx, osy; // Saved x & y coords
uint32_t    updateTime;       // time for next update
int         old_analog; // Value last displayed
int         value[6]        = {0, 0, 0, 0, 0, 0};
int         old_value[6]    = { -1, -1, -1, -1, -1, -1};
int         d;
uint8_t     isInited;


uint8_t Cluster_Init(TFT_eSPI &passedTft)
{
    uint8_t status = 0x00u;

    ltx = 0;                                    // Saved x coord of bottom of needle
    osx = 120, osy = 120;                       // Saved x & y coords
    updateTime = 0;                             // time for next update   
    old_analog =  -999;                         // Value last displayed 
    d = 0;

    clusterTft = &passedTft;
    clusterTft->init();
    clusterTft->setRotation(3);
    clusterTft->fillScreen(TFT_BLACK);
    analogMeter(); // Draw analogue meter
    updateTime = millis(); // Next update time

    status = 0x01u;
    isInited = 0x01;
    
    return status;
}

void Cluster_DeInit(void)
{
  isInited = 0x00u;
}

uint8_t getIsInited(void)
{
   return isInited;
}

void Cluster_Main(TFT_eSPI &passedTft)
{
  if(!getIsInited())
  {
    Cluster_Init(passedTft);
  }
  if (updateTime <= millis()) 
  {
    updateTime = millis() + 235; // Update meter every 35 milliseconds
 
    // Create a Sine wave for testing
    d += 4; 
    if (d >= 360) 
    {
        d = 0;
    }

    value[0] = 50 + 50 * sin((d + 0) * 0.0174532925);
    plotNeedle(value[0], 0); // It takes between 2 and 14ms to replot the needle with zero delay
  }
}
// #########################################################################
//  Draw the analogue meter on the screen
// #########################################################################
void analogMeter()
{

  // Meter outline
  clusterTft->fillRect(0, 0, 239, 131, TFT_GREY);
  clusterTft->fillRect(1, 3, 234, 125, TFT_WHITE);

  clusterTft->setTextColor(TFT_BLACK);  // Text colour

  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  for (int i = -50; i < 51; i += 5) {
    // Long scale tick length
    int tl = 15;

    // Coodinates of tick to draw
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (100 + tl) + 120;
    uint16_t y0 = sy * (100 + tl) + 150;
    uint16_t x1 = sx * 100 + 120;
    uint16_t y1 = sy * 100 + 150;

    // Coordinates of next tick for zone fill
    float sx2 = cos((i + 5 - 90) * 0.0174532925);
    float sy2 = sin((i + 5 - 90) * 0.0174532925);
    int x2 = sx2 * (100 + tl) + 120;
    int y2 = sy2 * (100 + tl) + 150;
    int x3 = sx2 * 100 + 120;
    int y3 = sy2 * 100 + 150;

    // Yellow zone limits
    //if (i >= -50 && i < 0) {
    //  clusterTft->fillTriangle(x0, y0, x1, y1, x2, y2, TFT_YELLOW);
    //  clusterTft->fillTriangle(x1, y1, x2, y2, x3, y3, TFT_YELLOW);
    //}

    // Green zone limits
    if (i >= 0 && i < 25) 
    {
      clusterTft->fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREEN);
      clusterTft->fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREEN);
    }

    // Orange zone limits
    if (i >= 25 && i < 50) 
    {
      clusterTft->fillTriangle(x0, y0, x1, y1, x2, y2, TFT_ORANGE);
      clusterTft->fillTriangle(x1, y1, x2, y2, x3, y3, TFT_ORANGE);
    }

    // Short scale tick length
    if (i % 25 != 0) 
    {
        tl = 8;
    }


    // Recalculate coords incase tick lenght changed
    x0 = sx * (100 + tl) + 120;
    y0 = sy * (100 + tl) + 150;
    x1 = sx * 100 + 120;
    y1 = sy * 100 + 150;

    // Draw tick
    clusterTft->drawLine(x0, y0, x1, y1, TFT_BLACK);

    // Check if labels should be drawn, with position tweaks
    if (i % 25 == 0) {
      // Calculate label positions
      x0 = sx * (100 + tl + 10) + 120;
      y0 = sy * (100 + tl + 10) + 150;
      switch (i / 25) {
        case -2: clusterTft->drawCentreString("0", x0+4, y0-4, 1); break;
        case -1: clusterTft->drawCentreString("25", x0+2, y0, 1); break;
        case 0: clusterTft->drawCentreString("50", x0, y0, 1); break;
        case 1: clusterTft->drawCentreString("75", x0, y0, 1); break;
        case 2: clusterTft->drawCentreString("100", x0-2, y0-4, 1); break;
      }
    }

    // Now draw the arc of the scale
    sx = cos((i + 5 - 90) * 0.0174532925);
    sy = sin((i + 5 - 90) * 0.0174532925);
    x0 = sx * 100 + 120;
    y0 = sy * 100 + 150;

    // Draw scale arc, don't draw the last part
    if (i < 50) 
    {
        clusterTft->drawLine(x0, y0, x1, y1, TFT_BLACK);
    }  
  }

  clusterTft->drawString("Km/h", (3 + 230 - 40), (119 - 20), 2); // Units at bottom right
  clusterTft->drawCentreString("Km/h", 120, 75, 4); // Comment out to avoid font 4
  clusterTft->drawRect(1, 3, 236, 126, TFT_BLACK); // Draw bezel line

  plotNeedle(0, 0); // Put meter needle at 0
}

// #########################################################################
// Update needle position
// This function is blocking while needle moves, time depends on ms_delay
// 10ms minimises needle flicker if text is drawn within needle sweep area
// Smaller values OK if text not in sweep area, zero for instant movement but
// does not look realistic... (note: 100 increments for full scale deflection)
// #########################################################################
void plotNeedle(int value, byte ms_delay)
{

  char buf[8]; 

    clusterTft->setTextColor(TFT_BLACK, TFT_WHITE);
    dtostrf(value, 4, 0, buf);
    clusterTft->drawRightString(buf, 33, (119 - 20), 2);

    if (value < -10) 
    {
        value = -10; // Limit value to emulate needle end stops
    }
    if (value > 110) 
    {
        value = 110;
    }

    // Move the needle until new value reached
    while (!(value == old_analog)) 
    {
        if (old_analog < value) old_analog++;
        else old_analog--;

        if (ms_delay == 0) old_analog = value; // Update immediately if delay is 0

        float sdeg = map(old_analog, -10, 110, -150, -30); // Map value to angle
        // Calculate tip of needle coords
        float sx = cos(sdeg * 0.0174532925);
        float sy = sin(sdeg * 0.0174532925);

        // Calculate x delta of needle start (does not start at pivot point)
        float tx = tan((sdeg + 90) * 0.0174532925);

        // Erase old needle image
        clusterTft->drawLine((120 + 24 * ltx) - 1, (150 - 24), osx - 1, osy, TFT_WHITE);
        clusterTft->drawLine((120 + 24 * ltx), (150 - 24), osx, osy, TFT_WHITE);
        clusterTft->drawLine((120 + 24 * ltx) + 1, (150 - 24), osx + 1, osy, TFT_WHITE);

        // Re-plot text under needle
        clusterTft->setTextColor(TFT_BLACK, TFT_WHITE);
        clusterTft->drawCentreString("Km/h", 120, 75, 4); // // Comment out to avoid font 4

        // Store new needle end coords for next erase
        ltx = tx;
        osx = (sx * 98 + 120);
        osy = (sy * 98 + 150);

        // Draw the needle in the new postion, magenta makes needle a bit bolder
        // draws 3 lines to thicken needle
        clusterTft->drawLine((120 + 24 * ltx) - 1, (150 - 24), osx - 1, osy, TFT_RED);
        clusterTft->drawLine((120 + 24 * ltx), (150 - 24), osx, osy, TFT_MAGENTA);
        clusterTft->drawLine((120 + 24 * ltx) + 1, (150 - 24), osx + 1, osy, TFT_RED);

        // Slow needle down slightly as it approaches new postion
        if (abs(old_analog - value) < 10) ms_delay += ms_delay / 5;

        // Wait before next update
        delay(ms_delay);
    }
}