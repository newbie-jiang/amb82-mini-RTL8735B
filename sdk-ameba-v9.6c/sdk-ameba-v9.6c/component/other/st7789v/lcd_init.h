#ifndef __LCD_INIT_H
#define __LCD_INIT_H
 
#include "lcd.h"
 
#define USE_HORIZONTAL 0  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏
 
 
#define LCD_W 240
#define LCD_H 320
 
extern gpio_t  gpio_scl;
extern gpio_t  gpio_sda;  //miso
extern gpio_t  gpio_cs;
extern gpio_t  gpio_rs;   //dc 
extern gpio_t  gpio_rst;
 
//-----------------LCD端口定义---------------- 
 
#define LCD_SCLK_Clr() gpio_write(&gpio_scl,0)//SCL=SCLK
#define LCD_SCLK_Set() gpio_write(&gpio_scl,1)
 
#define LCD_MOSI_Clr() gpio_write(&gpio_sda,0)//SDA=MOSI
#define LCD_MOSI_Set() gpio_write(&gpio_sda,1)
 
#define LCD_RES_Clr()  gpio_write(&gpio_rst,0)//RES
#define LCD_RES_Set()  gpio_write(&gpio_rst,1)
 
#define LCD_DC_Clr()   gpio_write(&gpio_rs,0)//DC
#define LCD_DC_Set()   gpio_write(&gpio_rs,1)
 
#define LCD_CS_Clr()  gpio_write(&gpio_cs,0)//cs
#define LCD_CS_Set()  gpio_write(&gpio_cs,1)
 
// #define LCD_BLK_Clr()  gpio_write(&,0)//BLK
// #define LCD_BLK_Set()  gpio_write(GPIOB, GPIO_PIN_0,1)
 
void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(u8 dat);//模拟SPI时序
void LCD_WR_DATA8(u8 dat);//写入一个字节
void LCD_WR_DATA(u16 dat);//写入两个字节
void LCD_WR_REG(u8 dat);//写入一个指令
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);//设置坐标函数
void LCD_Init(void);//LCD初始化
#endif
 
 
 
 