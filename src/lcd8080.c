#include "lcd8080.h"
#include "config.h"
#include "hardware/hardware.h"
#include <stdio.h>

/** LCD REGISTERS **/
#define LCD_OSCILLATOR 0x0
#define LCD_DRIVE_WAVEFORM 0x2
#define LCD_DISPLAY_CONTROL 0x7
#define LCD_SLEEP_MODE 0x10
#define LCD_ENTRY_MODE 0x11
/*******************/

static volatile LCDConfig currentConfig;

/*******************************************************/
/************** BIG BLOCK OF IO ABSTRACTION ************/
/*******************************************************/
// data order 1 -> 17 (ignoring nc pins). LOW 16 bits
static void setRawData(uint16_t data){
  //set 1
  SET_PIN(currentConfig.data1, (0x1 & (data >> 0)));
  SET_PIN(currentConfig.data2, (0x1 & (data >> 1)));
  SET_PIN(currentConfig.data3, (0x1 & (data >> 2)));
  SET_PIN(currentConfig.data4, (0x1 & (data >> 3)));
  SET_PIN(currentConfig.data5, (0x1 & (data >> 4)));

  //set 2
  SET_PIN(currentConfig.data6, (0x1 & (data >> 5)));
  SET_PIN(currentConfig.data7, (0x1 & (data >> 6)));
  SET_PIN(currentConfig.data8, (0x1 & (data >> 7)));
  SET_PIN(currentConfig.data10, (0x1 & (data >> 8)));
  SET_PIN(currentConfig.data11, (0x1 & (data >> 9)));
  SET_PIN(currentConfig.data12, (0x1 & (data >> 10)));

  //set 3
  SET_PIN(currentConfig.data13, (0x1 & (data >> 11)));
  SET_PIN(currentConfig.data14, (0x1 & (data >> 12)));
  SET_PIN(currentConfig.data15, (0x1 & (data >> 13)));
  SET_PIN(currentConfig.data16, (0x1 & (data >> 14)));
  SET_PIN(currentConfig.data17, (0x1 & (data >> 15)));
}

static void getRawData(uint16_t * data){
  *data = 0;
  //set 1
  *data |= READ_PIN(currentConfig.data1) << 0;
  *data |= READ_PIN(currentConfig.data2) << 1;
  *data |= READ_PIN(currentConfig.data3) << 2;
  *data |= READ_PIN(currentConfig.data4) << 3;
  *data |= READ_PIN(currentConfig.data5) << 4;

  //set 2
  *data |= READ_PIN(currentConfig.data6) << 5;
  *data |= READ_PIN(currentConfig.data7) << 6;
  *data |= READ_PIN(currentConfig.data8) << 7;
  *data |= READ_PIN(currentConfig.data10) << 8;
  *data |= READ_PIN(currentConfig.data11) << 9;
  *data |= READ_PIN(currentConfig.data12) << 10;

  //set 3
  *data |= READ_PIN(currentConfig.data13) << 11;
  *data |= READ_PIN(currentConfig.data14) << 12;
  *data |= READ_PIN(currentConfig.data15) << 13;
  *data |= READ_PIN(currentConfig.data16) << 14;
  *data |= READ_PIN(currentConfig.data17) << 15;
}

// control order DC, WR, RD, LCD_CS, LCD_RST
static void setRawControl(uint8_t ctrl){
  //control pins
  SET_PIN(currentConfig.lcdDC, (0x1 & (ctrl >> 0)));
  SET_PIN(currentConfig.lcdWR, (0x1 & (ctrl >> 1)));
  SET_PIN(currentConfig.lcdRD, (0x1 & (ctrl >> 2)));
  SET_PIN(currentConfig.lcdCS, (0x1 & (ctrl >> 3)));
  SET_PIN(currentConfig.lcdRST, (0x1 & (ctrl >> 4)));
}

static void setRaw(uint8_t control,uint16_t data){
  setRawData(data);
  setRawControl(control);
}

#define NOP_CONTROL_PINS 0x16 // RST, WR, RD high

#define WRITE_CONTROL_PINS_REG 0x14 // all low but RST and RD
static void writeCmd(uint16_t data){
  setRaw(WRITE_CONTROL_PINS_REG,data);
  nanoSleep(150);
  setRawControl(NOP_CONTROL_PINS);
}

#define WRITE_CONTROL_PINS_LCD 0x15 // RST , RD , DC high
static void writeDisplay(uint16_t data){
  setRaw(WRITE_CONTROL_PINS_LCD, data);
  nanoSleep(150);
  setRawControl(NOP_CONTROL_PINS);
}

#define READ_CONTROL_PINS_REG 0x12 // RST, WR high
static void readCmd(uint16_t * data){
  setRaw(READ_CONTROL_PINS_REG,0x0);
  nanoSleep(250);//data access time
  getRawData(data);
  setRawControl(NOP_CONTROL_PINS);
}

#define READ_CONTROL_PINS_LCD 0x13 // RST, WR, DC high
static void readDisplay(uint16_t * data){
  setRaw(READ_CONTROL_PINS_LCD,0x0);
  nanoSleep(250);//data access time
  getRawData(data);
  setRawControl(NOP_CONTROL_PINS);
}
/*******************************************************/
/******* END OF THE BIG BLOCK OF IO ABSTRACTION ********/
/*******************************************************/


static void writeRegister(uint16_t reg, uint16_t val){
  writeCmd(reg);// select register
  writeCmd(val);// write new value
}

static void readRegister(uint16_t reg, uint16_t * val){
  writeCmd(reg); // sel reg
  readCmd(val);// read
}

void initializeLCD(LCDConfig lcdConf){
  currentConfig = lcdConf;
  powerOnLCD();
}

void initializeLCDDefault(){
  //control
  currentConfig.lcdDC = toPin(PIO_BANK_C,PIO_PC23);
  currentConfig.lcdWR = toPin(PIO_BANK_C,PIO_PC24);
  currentConfig.lcdRD = toPin(PIO_BANK_C,PIO_PC25);
  currentConfig.lcdCS = toPin(PIO_BANK_C,PIO_PC26);
  currentConfig.lcdRST = toPin(PIO_BANK_B,PIO_PB25);

  //data
  currentConfig.data1 = toPin(PIO_BANK_C,PIO_PC7);
  currentConfig.data2 = toPin(PIO_BANK_C,PIO_PC9);
  currentConfig.data3 = toPin(PIO_BANK_A,PIO_PA20);
  currentConfig.data4 = toPin(PIO_BANK_C,PIO_PC4);
  currentConfig.data5 = toPin(PIO_BANK_C,PIO_PC6);
  currentConfig.data6 = toPin(PIO_BANK_C,PIO_PC8);
  currentConfig.data7 = toPin(PIO_BANK_A,PIO_PA19);

  currentConfig.data8 = toPin(PIO_BANK_C,PIO_PC16);
  currentConfig.data10 = toPin(PIO_BANK_C,PIO_PC12);
  currentConfig.data11 = toPin(PIO_BANK_B,PIO_PB14);
  currentConfig.data12 = toPin(PIO_BANK_C,PIO_PC17);
  currentConfig.data13 = toPin(PIO_BANK_C,PIO_PC15);
  currentConfig.data14 = toPin(PIO_BANK_C,PIO_PC13);
  currentConfig.data15 = toPin(PIO_BANK_B,PIO_PB21);

  enableAllIOBanks();
  enablePin(PIO_BANK_C, PIO_PC23 | PIO_PC24 | PIO_PC25 | PIO_PC26 | PIO_PC7  |
                        PIO_PC29 | PIO_PC4  | PIO_PC6  | PIO_PC8  | PIO_PC16 |
                        PIO_PC12 | PIO_PC17 | PIO_PC15 | PIO_PC13, PIO_OUT);
  enablePin(PIO_BANK_A, PIO_PA20 | PIO_PA19, PIO_OUT);
  enablePin(PIO_BANK_B, PIO_PB25 | PIO_PB14 | PIO_PB21, PIO_OUT);

  powerOnLCD();
}

void powerOnLCD(){
  //carry out the power on sequence as described in SSD1289 documentation (page 71)
  writeRegister(LCD_DISPLAY_CONTROL, 0x21);
  writeRegister(LCD_OSCILLATOR, 0x1);
  writeRegister(LCD_DISPLAY_CONTROL, 0x23);
  writeRegister(LCD_SLEEP_MODE, 0x0);
  sleep(30);
  writeRegister(LCD_DISPLAY_CONTROL, 0x33);
  writeRegister(LCD_ENTRY_MODE, 0x6000);
  writeRegister(LCD_DRIVE_WAVEFORM, 0x0);
}
