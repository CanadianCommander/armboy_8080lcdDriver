#ifndef SSD1289_SYSCALL_H_
#define SSD1289_SYSCALL_H_
#include <stdint.h>

typedef struct {
  int x;
  int y;
  uint16_t w;
  uint16_t h;
} Rectangle;

typedef struct {
  uint16_t w, h;
  uint16_t * rgb;
} Bitmap;

#define BITMAP_DRAW_MODE_NN 0
#define BITMAP_DRAW_MODE_L  1
typedef struct {
  Bitmap * b;
  int x;
  int y;
  float scale;
  uint8_t mode;
} BitmapDraw;

typedef struct {
  Rectangle * r;
  uint16_t color;
} RectangleDraw;


#endif /*SSD1289_SYSCALL_H_*/
