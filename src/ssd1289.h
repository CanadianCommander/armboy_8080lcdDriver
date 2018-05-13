#ifndef LCD8080_H_
#define LCD8080_H_
#include "config.h"
#include "hardware/hardware.h"
#include "ssd1289SysCall.h"

typedef struct {
  Pin lcdDC;
  Pin lcdWR;
  Pin lcdRD;

  Pin lcdCS;
  // note F_CS is NC
  Pin lcdRST;

  Pin data0;
  Pin data1;
  Pin data2;
  Pin data3;
  Pin data4;
  Pin data5;
  Pin data6;
  Pin data7;
  Pin data8;
  Pin data9;
  Pin data10;
  Pin data11;
  Pin data12;
  Pin data13;
  Pin data14;
  Pin data15;

} LCDConfig;

#define SSD1289_WIDTH 320
#define SSD1289_HEIGHT 240
#define TRANSPARENT_COLOR 0xFFFF

/**
  configure lcd for use & clear to black. CALL FIRST
*/
void initializeLCD(LCDConfig lcdConf);
void initializeLCDDefault();
/**
  power up the LCD controller
*/
void powerOnLCD();

/**
  clear the entire display to the given color
*/
void clearDisplay(uint16_t color);

/**
  every thing outside of this box will not be drawn. you can use this to draw
  a sub section of a bitmap for example.
*/
void setClipRegion(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/**
  resets the clipping region to the default (entire window)
  @see setClipRegion
*/
void clearClipRegion();

/**
  draw rectangle described by r ... thats it
*/
void drawRectangle(Rectangle * r, uint16_t color);

/**
  draw bitmap at the given coordinates
*/
void drawBitmap(Bitmap * b, int x, int y);

/**
  like draw bitmap but with scale option. (using nearest neighbor)
  @see drawBitmap
*/
void drawBitmapNNScale(Bitmap * b, int x, int y, float scale);

/**
  like draw bitmap but with scale option (using Linear interpolation).
  use drawBitmapNNScale for speed, this for quality
  @see drawBitmapNNScale
*/
void drawBitmapLScale(Bitmap * b, int x, int y, float scale);

#endif /*LCD8080_H_*/
