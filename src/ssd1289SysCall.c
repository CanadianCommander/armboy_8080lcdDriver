#include "ssd1289SysCall.h"
#include "ssd1289.h"

void init(void * lcdStructure){
  initializeLCD(*(LCDConfig *)lcdStructure);
}

void initDefault(void * nop){
  initializeLCDDefault();
}

void drawBitmapCall(void * drawCmd){
  BitmapDraw * bd = (BitmapDraw *)drawCmd;

  switch(bd->mode){
    case BITMAP_DRAW_MODE_NN:
      drawBitmapNNScale(bd->b,bd->x,bd->y,bd->scale);
      break;
    case BITMAP_DRAW_MODE_L:
      drawBitmapLScale(bd->b,bd->x,bd->y,bd->scale);
      break;
    default:
      return;
  }
}

void drawRectangleCall(void * recCmd){
  RectangleDraw * rd = (RectangleDraw*)recCmd;
  drawRectangle(rd->r, rd->color);
}
