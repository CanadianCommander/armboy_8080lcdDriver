#include "ssd1289SysCall.h"
#include "ssd1289.h"

void init(void * lcdStructure){
  initializeLCD(*(LCDConfig *)lcdStructure);
}

void initDefault(void * nop){
  initializeLCDDefault();
}

void clearDisplayCall(void * color){
  clearDisplay(*(uint16_t*)color);
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


void setClipRegionCall(void * arg){
  SetClipRegionCall * scr = (SetClipRegionCall*)arg;
  setClipRegion(scr->x, scr->y, scr->w, scr->h);
}

void clearClipRegionCall(void * arg){
  clearClipRegion();
}
