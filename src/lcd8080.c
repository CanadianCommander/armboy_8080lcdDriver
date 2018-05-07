#include "lcd8080.h"
#include "config.h"
#include "hardware/hardware.h"
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
#define LCD_FRAME_FREQ 0x25
/*******************/

static LCDConfig currentConfig;

static uint16_t prevData =0;
/*******************************************************/
/************** BIG BLOCK OF IO ABSTRACTION ************/
/*******************************************************/

//normal set pin has way to much branching. NEED SPEED!
#define SET_PIN_FAST(p,l) *(p.setReg + (0x1 & ~l)) |= p.pin

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

inline static void setRaw(uint8_t control,uint16_t data){
  setRawData(data);
  setRawControl(control);
}

#define NOP_CONTROL_PINS 0x5 // RST, WR, RD high

#define WRITE_CONTROL_PINS_REG 0x4 // all low but RST and RD
inline static void writeCmd(uint16_t data, uint8_t param){
  setRaw(WRITE_CONTROL_PINS_REG | param,data);
  strobe(currentConfig.lcdWR);
  setRaw(NOP_CONTROL_PINS,0x0);
}

#define READ_CONTROL_PINS_REG 0x4 // RST, WR high
inline static void readCmd(uint16_t * data, uint8_t param){
  setRaw(READ_CONTROL_PINS_REG | param,0x0);
  strobe(currentConfig.lcdRD);
  strobe(currentConfig.lcdRD);
  getRawData(data);
  setRaw(NOP_CONTROL_PINS,0x0);
}


#define WRITE_CONTROL_PINS_LCD 0x5 // RST , RD , DC high
inline static void prepDisplayForWrite(){
  setRawControl(WRITE_CONTROL_PINS_LCD);
}

inline static void writeDisplay(uint16_t data){
  setRawData(data);
  strobe(currentConfig.lcdWR);
}

#define READ_CONTROL_PINS_LCD 0x5 // RST, WR, DC high
inline static void readDisplay(uint16_t * data){
  setRaw(READ_CONTROL_PINS_LCD,0x0);
  SET_PIN(currentConfig.lcdRD,0);
  nanoSleep(1000);//data access time
  getRawData(data);
  SET_PIN(currentConfig.lcdRD,1);
  setRaw(NOP_CONTROL_PINS,0x0);
}
/*******************************************************/
/******* END OF THE BIG BLOCK OF IO ABSTRACTION ********/
/*******************************************************/


inline static void writeRegister(uint16_t reg, uint16_t val){
  writeCmd(reg,0);// select register
  writeCmd(val,1);// write new value
}

inline static void readRegister(uint16_t reg, uint16_t * val){
  uint16_t d;
  setRaw(0x4,reg);
  strobe(currentConfig.lcdWR);
  setRaw(0x5,0x0);
  SET_PIN(currentConfig.lcdRD,0);
  nanoSleep(1000);
  getRawData(&d);
  SET_PIN(currentConfig.lcdRD,1);
  printf("d is: %x \n",d);
  SET_PIN(currentConfig.lcdRD,0);
  nanoSleep(1000);
  getRawData(&d);
  SET_PIN(currentConfig.lcdRD,1);
  printf("d is: %x \n",d);
  SET_PIN(currentConfig.lcdRD,0);
  nanoSleep(1000);
  getRawData(&d);
  SET_PIN(currentConfig.lcdRD,1);
  printf("d is: %x \n",d);
  //writeCmd(reg,0); // sel reg
  //readCmd(val,1);// read
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
  SET_PIN(currentConfig.lcdWR,1);
  SET_PIN(currentConfig.lcdRD,1);

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

  //landscape ???
  writeRegister(LCD_DRIVER_OUT_C,0x293F);
  writeRegister(LCD_ENTRY_MODE,0x6038);

  uint32_t cTime;
  uint32_t t2;
  uint32_t center =1;
  uint32_t mov = 10;
  uint32_t radii = 25;
  int dx;
  int dy,dy2,dy3;
  writeCmd(0x22,0);
  prepDisplayForWrite();
  for(int z =0; z < 20000; z++){
    if(center >= 320 || center <= 0){
      mov = -mov;
    }
    center += mov;
    cTime = getTime();
    for(int y =0; y<240;y++){
      for(int x=0; x < 320; x+=2){
        dx = x - center;
        dy = y - 120;
        dy2 = y - 40;
        dy3 = y - 200;
        if((dx*dx + dy*dy)/10 < radii){
          writeDisplay(0xF800);
          writeDisplay(0xF800);
        }
        else if((dx*dx + dy2*dy2)/10 < radii){
          writeDisplay(0x001F);
          writeDisplay(0x001F);
        }
        else if((dx*dx + dy3*dy3)/10 < radii){
          writeDisplay(0x7E0);
          writeDisplay(0x7E0);
        }
        else {
          writeDisplay(0x0000);
          writeDisplay(0x0000);
        }
      }
    }
    if(z % 300 == 0){
      t2 = getTime() - cTime;
      printf("Frame Time: %dms\n",t2);
    }
  }

  uint16_t d;
  readRegister(0x0, &d);
}
