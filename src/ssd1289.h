#ifndef LCD8080_H_
#define LCD8080_H_
#include "config.h"
#include "hardware/hardware.h"

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

typedef struct {
  uint16_t w, h;
  uint16_t * rgb;
} Bitmap;


#define SSD1289_WIDTH 320
#define SSD1289_HEIGHT 240
#define TRANSPARENT_COLOR 0xFFFF

/**
  configuer lcd for use & clear to black. CALL FIRST
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
  draw bitmap at the given coordinates
*/
void drawBitmap(Bitmap * b, uint16_t x, uint16_t y);

/**
  like draw bitmap but with scale option
  @see drawBitmap
*/
void drawBitmapScale(Bitmap * b, uint16_t x, uint16_t y, uint16_t scale);


#endif /*LCD8080_H_*/
