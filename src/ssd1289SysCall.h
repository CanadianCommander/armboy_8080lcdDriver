#ifndef SSD1289_SYSCALL_H_
#define SSD1289_SYSCALL_H_
#include "ssd1289.h"

#define BITMAP_DRAW_MODE_NN 0
#define BITMAP_DRAW_MODE_L  1
typedef struct {
  Bitmap * b;
  uint16_t x;
  uint16_t y;
  float scale;
  uint8_t mode;
} BitmapDraw;

typedef struct {
  Rectangle * r;
  uint16_t color;
} RectangleDraw;


#endif /*SSD1289_SYSCALL_H_*/
