#include "ssd1289.h"
#include <stdio.h>

/** LCD REGISTERS **/
#define LCD_OSCILLATOR 0x0
#define LCD_DRIVER_OUT_C 0x1
#define LCD_DRIVE_WAVEFORM 0x2
#define LCD_POWER_CONTROL_1 0x3
#define LCD_DISPLAY_CONTROL 0x7
#define LCD_POWER_CONTROL_2 0xC
#define LCD_SLEEP_MODE 0x10
#define LCD_ENTRY_MODE 0x11
#define LCD_DISPLAY_DATA 0x22
#define LCD_FRAME_FREQ 0x25
#define LCD_HORZ_RAM_ADDR 0x44
#define LCD_VERT_RAM_START_ADDR 0x45
#define LCD_VERT_RAM_END_ADDR 0x46
#define LCD_RAM_ADDR_X 0x4F
#define LCD_RAM_ADDR_Y 0x4E
/*******************/

static LCDConfig currentConfig;

static Rectangle clippingRec = {0,0,320,240};

static uint16_t prevData =0;
/*******************************************************/
/************** BIG BLOCK OF IO ABSTRACTION ************/
/*******************************************************/

//normal set pin has way to much branching. NEED SPEED!
#define SET_PIN_FAST(p,l) *(p.setReg + (0x1 & ~l)) |= p.pin

inline static void changeDataPinDirection(uint8_t dir){
  setPinDirection(currentConfig.data0.bank,currentConfig.data0.pin,dir);
  setPinDirection(currentConfig.data1.bank,currentConfig.data1.pin,dir);
  setPinDirection(currentConfig.data2.bank,currentConfig.data2.pin,dir);
  setPinDirection(currentConfig.data3.bank,currentConfig.data3.pin,dir);
  setPinDirection(currentConfig.data4.bank,currentConfig.data4.pin,dir);
  setPinDirection(currentConfig.data5.bank,currentConfig.data5.pin,dir);
  setPinDirection(currentConfig.data6.bank,currentConfig.data6.pin,dir);
  setPinDirection(currentConfig.data7.bank,currentConfig.data7.pin,dir);

  setPinDirection(currentConfig.data8.bank,currentConfig.data8.pin,dir);
  setPinDirection(currentConfig.data9.bank,currentConfig.data9.pin,dir);
  setPinDirection(currentConfig.data10.bank,currentConfig.data10.pin,dir);
  setPinDirection(currentConfig.data11.bank,currentConfig.data11.pin,dir);
  setPinDirection(currentConfig.data12.bank,currentConfig.data12.pin,dir);
  setPinDirection(currentConfig.data13.bank,currentConfig.data13.pin,dir);
  setPinDirection(currentConfig.data14.bank,currentConfig.data14.pin,dir);
  setPinDirection(currentConfig.data15.bank,currentConfig.data15.pin,dir);
}

// data order 1 -> 17 (ignoring nc pins). LOW 16 bits
inline static void setRawData(uint16_t data){
  if(data == prevData){
    return;
  }
  else {
    prevData = data;
  }

  //set 1
  SET_PIN_FAST(currentConfig.data0, (0x1 & (data >> 0)));
  SET_PIN_FAST(currentConfig.data1, (0x1 & (data >> 1)));
  SET_PIN_FAST(currentConfig.data2, (0x1 & (data >> 2)));
  SET_PIN_FAST(currentConfig.data3, (0x1 & (data >> 3)));
  SET_PIN_FAST(currentConfig.data4, (0x1 & (data >> 4)));
  SET_PIN_FAST(currentConfig.data5, (0x1 & (data >> 5)));

  //set 2
  SET_PIN_FAST(currentConfig.data6, (0x1 & (data >> 6)));
  SET_PIN_FAST(currentConfig.data7, (0x1 & (data >> 7)));
  SET_PIN_FAST(currentConfig.data8, (0x1 & (data >> 8)));
  SET_PIN_FAST(currentConfig.data9, (0x1 & (data >> 9)));
  SET_PIN_FAST(currentConfig.data10, (0x1 & (data >> 10)));
  SET_PIN_FAST(currentConfig.data11, (0x1 & (data >> 11)));
  SET_PIN_FAST(currentConfig.data12, (0x1 & (data >> 12)));

  //set 3
  SET_PIN_FAST(currentConfig.data13, (0x1 & (data >> 13)));
  SET_PIN_FAST(currentConfig.data14, (0x1 & (data >> 14)));
  SET_PIN_FAST(currentConfig.data15, (0x1 & (data >> 15)));
}

inline static void getRawData(uint16_t * data){
  *data = 0;
  //set 1
  *data |= READ_PIN(currentConfig.data0) << 0;
  *data |= READ_PIN(currentConfig.data1) << 1;
  *data |= READ_PIN(currentConfig.data2) << 2;
  *data |= READ_PIN(currentConfig.data3) << 3;
  *data |= READ_PIN(currentConfig.data4) << 4;
  *data |= READ_PIN(currentConfig.data5) << 5;

  //set 2
  *data |= READ_PIN(currentConfig.data6) << 6;
  *data |= READ_PIN(currentConfig.data7) << 7;
  *data |= READ_PIN(currentConfig.data8) << 8;
  *data |= READ_PIN(currentConfig.data9) << 9;
  *data |= READ_PIN(currentConfig.data10) << 10;
  *data |= READ_PIN(currentConfig.data11) << 11;
  *data |= READ_PIN(currentConfig.data12) << 12;

  //set 3
  *data |= READ_PIN(currentConfig.data13) << 13;
  *data |= READ_PIN(currentConfig.data14) << 14;
  *data |= READ_PIN(currentConfig.data15) << 15;
}

// control order DC, LCD_CS, LCD_RST
inline static void setRawControl(uint8_t ctrl){
  //control pins
  SET_PIN(currentConfig.lcdDC, (0x1 & (ctrl >> 0)));
  SET_PIN(currentConfig.lcdCS, (0x1 & (ctrl >> 1)));
  SET_PIN(currentConfig.lcdRST, (0x1 & (ctrl >> 2)));
}

inline static void strobe(Pin p){
  SET_PIN_FAST(p,0);
  asm("nop \n");
  SET_PIN_FAST(p,1);
}

inline static void setRaw(uint8_t control, uint16_t data){
  setRawData(data);
  setRawControl(control);
}

#define NOP_CONTROL_PINS 0x5 // RST, WR, RD high

#define WRITE_CONTROL_PINS_REG 0x4 // all low but RST and RD
inline static void writeCmd(uint16_t data, uint8_t param){
  setRaw(WRITE_CONTROL_PINS_REG | param, data);
  strobe(currentConfig.lcdWR);
  setRaw(NOP_CONTROL_PINS,0x0);
}


#define WRITE_CONTROL_PINS_LCD 0x5 // RST , RD , DC high
inline static void prepDisplayForWrite(){
  writeCmd(LCD_DISPLAY_DATA,0);
  setRawControl(WRITE_CONTROL_PINS_LCD);
}

/*******************************************************/
/******* END OF THE BIG BLOCK OF IO ABSTRACTION ********/
/*******************************************************/


inline static void writeRegister(uint16_t reg, uint16_t val){
  writeCmd(reg, 0);// select register
  writeCmd(val, 1);// write new value
}

inline static void writeDisplay(uint16_t data){
  SET_PIN_FAST(currentConfig.lcdWR,0);
  setRawData(data);
  SET_PIN_FAST(currentConfig.lcdWR,1);
}

inline static void skipPixel(){
  strobe(currentConfig.lcdRD);
}

inline static void readDisplay(uint16_t * data){
  changeDataPinDirection(PIO_IN);
  SET_PIN(currentConfig.lcdRD,0);
  nanoSleep(250);//data access time
  getRawData(data);
  SET_PIN(currentConfig.lcdRD,1);
  changeDataPinDirection(PIO_OUT);
}

inline static void setWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
  writeRegister(LCD_RAM_ADDR_X, x);
  writeRegister(LCD_RAM_ADDR_Y, y);
  writeRegister(LCD_HORZ_RAM_ADDR,(0xFF00 & ((y+h - 1)<<8)) | (0x00FF & (y)));
  writeRegister(LCD_VERT_RAM_START_ADDR,0x01FF & (x));
  writeRegister(LCD_VERT_RAM_END_ADDR, 0x01FF & (x + w - 1));
}

inline static void setWindowDefault(){
  //POR values from SSD1289 data sheet
  writeRegister(LCD_RAM_ADDR_X,0x0);
  writeRegister(LCD_RAM_ADDR_Y,0x0);
  writeRegister(LCD_HORZ_RAM_ADDR, 0xEF00);
  writeRegister(LCD_VERT_RAM_START_ADDR,0x0);
  writeRegister(LCD_VERT_RAM_END_ADDR, 0x13F);
}

void initializeLCD(LCDConfig lcdConf){
  currentConfig = lcdConf;
  powerOnLCD();
}

void initializeLCDDefault(){
  //control
  currentConfig.lcdDC = toPin(PIO_BANK_C,PIO_PC23,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.lcdWR = toPin(PIO_BANK_C,PIO_PC24,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.lcdRD = toPin(PIO_BANK_C,PIO_PC25,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.lcdCS = toPin(PIO_BANK_C,PIO_PC26,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.lcdRST = toPin(PIO_BANK_B,PIO_PB25,(uint32_t*)&REG_PIOB_SODR);

  //data
  currentConfig.data0 = toPin(PIO_BANK_C,PIO_PC5,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.data1 = toPin(PIO_BANK_C,PIO_PC7,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.data2 = toPin(PIO_BANK_C,PIO_PC9,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.data3 = toPin(PIO_BANK_A,PIO_PA20,(uint32_t*)&REG_PIOA_SODR);
  currentConfig.data4 = toPin(PIO_BANK_C,PIO_PC4,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.data5 = toPin(PIO_BANK_C,PIO_PC6,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.data6 = toPin(PIO_BANK_C,PIO_PC8,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.data7 = toPin(PIO_BANK_A,PIO_PA19,(uint32_t*)&REG_PIOA_SODR);

  currentConfig.data8 = toPin(PIO_BANK_C,PIO_PC16,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.data9 = toPin(PIO_BANK_C,PIO_PC14,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.data10 = toPin(PIO_BANK_C,PIO_PC12,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.data11 = toPin(PIO_BANK_B,PIO_PB14,(uint32_t*)&REG_PIOB_SODR);
  currentConfig.data12 = toPin(PIO_BANK_C,PIO_PC17,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.data13 = toPin(PIO_BANK_C,PIO_PC15,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.data14 = toPin(PIO_BANK_C,PIO_PC13,(uint32_t*)&REG_PIOC_SODR);
  currentConfig.data15 = toPin(PIO_BANK_B,PIO_PB21,(uint32_t*)&REG_PIOB_SODR);

  enableAllIOBanks();
  enablePin(PIO_BANK_C, PIO_PC23 | PIO_PC24 | PIO_PC25 | PIO_PC26 | PIO_PC7  |
                        PIO_PC29 | PIO_PC4  | PIO_PC6  | PIO_PC8  | PIO_PC16 | PIO_PC9 |
                        PIO_PC12 | PIO_PC17 | PIO_PC15 | PIO_PC13 | PIO_PC5 | PIO_PC14 , PIO_OUT);
  enablePin(PIO_BANK_A, PIO_PA20 | PIO_PA19, PIO_OUT);
  enablePin(PIO_BANK_B, PIO_PB25 | PIO_PB14 | PIO_PB21, PIO_OUT);

  powerOnLCD();
}

void powerOnLCD(){
  //set WR and RD to off
  SET_PIN(currentConfig.lcdWR,1);
  SET_PIN(currentConfig.lcdRD,1);

  //reset display
  SET_PIN(currentConfig.lcdRST,0);
  sleep(1);
  SET_PIN(currentConfig.lcdRST,1);

  //carry out the power on sequence as described in SSD1289 documentation (page 71)
  writeRegister(LCD_DISPLAY_CONTROL, 0x21);
  writeRegister(LCD_OSCILLATOR, 0x1);
  writeRegister(LCD_DISPLAY_CONTROL, 0x23);
  writeRegister(LCD_SLEEP_MODE, 0x0);
  sleep(30);
  writeRegister(LCD_DISPLAY_CONTROL, 0x133);
  writeRegister(LCD_ENTRY_MODE, 0x6000);
  writeRegister(LCD_DRIVE_WAVEFORM, 0x1000);

  //change to landscape view
  writeRegister(LCD_DRIVER_OUT_C,0x293F);
  writeRegister(LCD_ENTRY_MODE,0x6038);

  //clear screen
  clearDisplay(0x0);
}

void clearDisplay(uint16_t color){
  prepDisplayForWrite();
  for(int y = 0; y < SSD1289_HEIGHT; y ++){
    for(int x = 0; x < SSD1289_WIDTH; x ++){
      writeDisplay(color);
    }
  }
}

void setClipRegion(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
  clippingRec.x = x;
  clippingRec.y = y;
  clippingRec.w = w;
  clippingRec.h = h;
}

void clearClipRegion(){
  clippingRec.x = 0;
  clippingRec.y = 0;
  clippingRec.w = 320;
  clippingRec.h = 240;
}



void __drawSetup(Bitmap * b, int xI, int yI, float scale, uint16_t * xOff, uint16_t * yOff, uint16_t * scaleWidth, uint16_t * scaleHeight);

void drawRectangle(Rectangle * r, uint16_t color){
  uint16_t xOffset =0;
  uint16_t yOffset =0;
  uint16_t scaleWidth = 0;
  uint16_t scaleHeight = 0;
  Bitmap fake;
  fake.w = r->w;
  fake.h = r->h;

  __drawSetup(&fake, r->x, r->y, 1.0, &xOffset, &yOffset, &scaleWidth, &scaleHeight);

  for(uint16_t y = yOffset; y < scaleHeight; y ++){
    for(uint16_t x = xOffset; x < scaleWidth; x ++){
      writeDisplay(color);
    }
  }
}

//bitmap helper functions
void __drawBitmap(Bitmap * b, int xI, int yI, float scale);
void __drawBitmapL(Bitmap * b, int xI, int yI, float scale);
void drawBitmap(Bitmap * b, int x, int y){
  __drawBitmap(b, x, y, 1);
}

void drawBitmapNNScale(Bitmap * b, int x, int y, float scale){
  __drawBitmap(b, x, y, scale);
}

void drawBitmapLScale(Bitmap * b, int x, int y, float scale){
  __drawBitmapL(b, x, y, scale);
}

void __drawBitmapL(Bitmap * b, int xI, int yI, float scale){
  uint16_t xOffset =0;
  uint16_t yOffset =0;
  uint16_t scaleWidth = 0;
  uint16_t scaleHeight = 0;

  __drawSetup(b, xI, yI, scale, &xOffset, &yOffset, &scaleWidth, &scaleHeight);

  for(float y =yOffset; y < scaleHeight; y ++){
    for(float x = xOffset; x < scaleWidth; x ++){
      float diffY = y/scale - (int)(y/scale);

      float diffX = ((int)(y/scale)*b->w) + x/scale - (int)(((int)(y/scale)*b->w) + x/scale);
      uint16_t * c1 = &(b->rgb[(int)(((int)(y/scale)*b->w) + x/scale)]);


      if(((int)(x/scale) + 1) < b->w && ((int)(y/scale) + 1) < b->h){
        uint16_t * c2 = &(b->rgb[(int)(((int)(y/scale)*b->w) + (x+1)/scale)]);
        uint16_t * c3 = &(b->rgb[(int)(((int)((y+1)/scale)*b->w) + x/scale)]);
        uint16_t * c4 = &(b->rgb[(int)(((int)((y+1)/scale)*b->w) + (x+1)/scale)]);
        uint16_t color = 0x0;
        color |= (uint16_t)(((1.0f - diffX)*(0x001F & *c1) + diffX*(0x001F & *c2))*(1.0f - diffY) + diffY*((1.0f - diffX)*(0x001F & *c3) + diffX*(0x001F & *c4))) & 0x001F; // B
        color |= (uint16_t)(((1.0f - diffX)*(0x07E0 & *c1) + diffX*(0x07E0 & *c2))*(1.0f - diffY) + diffY*((1.0f - diffX)*(0x07E0 & *c3) + diffX*(0x07E0 & *c4))) & 0x07E0; // G
        color |= (uint16_t)(((1.0f - diffX)*(0xF800 & *c1) + diffX*(0xF800 & *c2))*(1.0f - diffY) + diffY*((1.0f - diffX)*(0xF800 & *c3) + diffX*(0xF800 & *c4))) & 0xF800; // R
        writeDisplay(color);
      }
      else if(((int)(x/scale) + 1) < b->w){
        uint16_t * c2 = &(b->rgb[(int)(((int)(y/scale)*b->w) + (x+1)/scale)]);
        uint16_t color = 0x0;
        color |= (uint16_t)((1.0f - diffX)*(0x001F & *c1) + diffX*(0x001F & *c2)) & 0x001F; // B
        color |= (uint16_t)((1.0f - diffX)*(0x07E0 & *c1) + diffX*(0x07E0 & *c2)) & 0x07E0; // G
        color |= (uint16_t)((1.0f - diffX)*(0xF800 & *c1) + diffX*(0xF800 & *c2)) & 0xF800; // R
        writeDisplay(color);
      }
      else {
        writeDisplay(*c1);
      }
    }
  }

  setWindowDefault();
}

void __drawBitmap(Bitmap * b, int xI, int yI, float scale){
  uint16_t xOffset =0;
  uint16_t yOffset =0;
  uint16_t scaleWidth = 0;
  uint16_t scaleHeight = 0;

  __drawSetup(b, xI, yI, scale, &xOffset, &yOffset, &scaleWidth, &scaleHeight);

  for(uint16_t y =yOffset; y < scaleHeight; y ++){
    for(uint16_t x = xOffset; x < scaleWidth; x ++){
      writeDisplay(b->rgb[(int)((((int)(y/scale))*b->w) + (int)x/scale)]);
    }
  }

  setWindowDefault();
}

void __drawSetup(Bitmap * b, int xI, int yI, float scale, uint16_t * xOff, uint16_t * yOff, uint16_t * scaleWidth, uint16_t * scaleHeight){
  *xOff = 0;
  *yOff = 0;

  if(xI > (clippingRec.x + clippingRec.w) || yI > (clippingRec.y + clippingRec.h)){
    *scaleWidth =0;
    *scaleHeight = 0;
    //early abort
    return;
  }

  if(xI < clippingRec.x){
    *xOff = -(xI - clippingRec.x);
  }
  if(yI < clippingRec.y){
    *yOff = -(yI - clippingRec.y);
  }

  *scaleWidth = b->w*scale;
  *scaleHeight = b->h*scale;

  if(xI + *scaleWidth > clippingRec.x + clippingRec.w){
    *scaleWidth -= (xI + *scaleWidth) - (clippingRec.x + clippingRec.w);
  }
  if(yI + *scaleHeight > clippingRec.y + clippingRec.h){
    *scaleHeight -= (yI + *scaleHeight) - (clippingRec.y + clippingRec.h);
  }

  setWindow(xI + *xOff, yI + *yOff, *scaleWidth - *xOff, *scaleHeight - *yOff);
  prepDisplayForWrite();
}
