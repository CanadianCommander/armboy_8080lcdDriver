#include "lcd8080.h"
#include "config.h"
#include "hardware/hardware.h"

/** LCD REGISTERS **/
#define LCD_DISPLAY_CONTROL 0x7
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
}

void powerOnLCD(){

}
