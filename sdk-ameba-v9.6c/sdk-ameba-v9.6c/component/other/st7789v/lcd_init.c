#include "lcd_init.h"
#include "task.h"
#include "spi_api.h"

//使用硬件spi时使能
#define    USE_HARDWARE_SPI  

gpio_t  gpio_scl;
gpio_t  gpio_sda;  //miso
gpio_t  gpio_cs;
gpio_t  gpio_rs;   //dc 
gpio_t  gpio_rst;



#define    SPI_SCL_PIN     PF_6
#define    SPI_SDA_PIN     PF_7
#define    SPI_CS_PIN      PF_8
#define    SPI_RS_PIN      PD_18
#define    SPI_RST_PIN     PD_17

#define    SPI_MISO_PIN     PF_5

spi_t  spi_master;


void HAL_Delay(uint32_t ms)
{
	vTaskDelay(ms);
}

 
void LCD_GPIO_Init(void)
{
	gpio_init(&gpio_rs, SPI_RS_PIN);
	gpio_dir(&gpio_rs, PIN_OUTPUT);        // Direction: Output
	gpio_mode(&gpio_rs, PullNone);         // No pull
    gpio_write(&gpio_rs,0);    

    gpio_init(&gpio_rst, SPI_RST_PIN);
	gpio_dir(&gpio_rst, PIN_OUTPUT);        // Direction: Output
	gpio_mode(&gpio_rst, PullNone);         // No pull
    gpio_write(&gpio_rst,0);    


   #ifdef USE_HARDWARE_SPI
  
	spi_init(&spi_master, SPI_SDA_PIN, SPI_MISO_PIN, SPI_SCL_PIN, SPI_CS_PIN);
     
	spi_format(&spi_master, 8, 2, 0);     
	
	spi_frequency(&spi_master, 10000000); // 10Mhz   
    spi_enable(&spi_master);
    
   #else

    gpio_init(&gpio_scl, SPI_SCL_PIN);
	gpio_dir(&gpio_scl, PIN_OUTPUT);        // Direction: Output
	gpio_mode(&gpio_scl, PullNone);         // No pull
    gpio_write(&gpio_scl,0);    

    gpio_init(&gpio_sda, SPI_SDA_PIN);
	gpio_dir(&gpio_sda, PIN_OUTPUT);        // Direction: Output
	gpio_mode(&gpio_scl, PullNone);         // No pull
    gpio_write(&gpio_scl,0);    

    gpio_init(&gpio_cs, SPI_CS_PIN);
	gpio_dir(&gpio_cs, PIN_OUTPUT);        // Direction: Output
	gpio_mode(&gpio_cs, PullNone);         // No pull
    gpio_write(&gpio_cs,0);    


    gpio_write(&gpio_scl,1);    //此处默认必须拉高，不然无法显示

   #endif 
}
 
 
/******************************************************************************
      函数说明：LCD串行数据写入函数
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus(u8 dat) 
{	
	u8 i;
	LCD_CS_Clr();
	for(i=0;i<8;i++)
	{			  
		LCD_SCLK_Clr();
		if(dat&0x80)
		{
		   LCD_MOSI_Set();
		}
		else
		{
		   LCD_MOSI_Clr();
		}
		LCD_SCLK_Set();
		dat<<=1;
	}	
	LCD_CS_Set();
}
 
 
/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(u8 dat)
{
	#ifdef USE_HARDWARE_SPI
    spi_master_write(&spi_master,dat);
	#else
    LCD_Writ_Bus(dat);
	#endif
	
}
 
 
/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(u16 dat)
{
    #ifdef USE_HARDWARE_SPI

    spi_master_write(&spi_master,dat>>8);
    spi_master_write(&spi_master,dat);

	#else

	LCD_Writ_Bus(dat>>8);
	LCD_Writ_Bus(dat);

	#endif
}
 
 
/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(u8 dat)
{
	LCD_DC_Clr();//写命令

    #ifdef USE_HARDWARE_SPI
    spi_master_write(&spi_master,dat);
	#else
    LCD_Writ_Bus(dat);
	#endif

	LCD_DC_Set();//写数据
}
 
 
/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2)
{
	if(USE_HORIZONTAL==0)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==1)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+80);
		LCD_WR_DATA(y2+80);
		LCD_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==2)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
	}
	else
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+80);
		LCD_WR_DATA(x2+80);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
	}

       
}
 
// void LCD_Init(void)
// {
// 	LCD_GPIO_Init();//初始化GPIO
	
// 	LCD_RES_Clr();//复位
// 	HAL_Delay(100);
// 	LCD_RES_Set();
// 	HAL_Delay(100);
	
// 	//************* Start Initial Sequence **********//
// 	LCD_WR_REG(0x11); //Sleep out 
// 	HAL_Delay(120);              //Delay 120ms 
// 	//************* Start Initial Sequence **********// 
// 	LCD_WR_REG(0x36);
// 	// if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);
// 	// else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
// 	// else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
// 	// else LCD_WR_DATA8(0xA0);
// 	LCD_WR_DATA8(0x60);
 
// 	LCD_WR_REG(0x3A);     
// 	LCD_WR_DATA8(0x05);     //rgb565
//     //ADD
// 	LCD_WR_REG(0xB0);
// 	LCD_WR_DATA8(0x00);   
// 	LCD_WR_DATA8(0xE0);   
 
// 	LCD_WR_REG(0xB2);     
// 	LCD_WR_DATA8(0x0C);   
// 	LCD_WR_DATA8(0x0C);   
// 	LCD_WR_DATA8(0x00);   
// 	LCD_WR_DATA8(0x33);   
// 	LCD_WR_DATA8(0x33);   
 
// 	LCD_WR_REG(0xB7);     
// 	LCD_WR_DATA8(0x35);   
 
// 	LCD_WR_REG(0xBB);     
// 	LCD_WR_DATA8(0x2B);   //2b
 
// 	LCD_WR_REG(0xC0);     
// 	LCD_WR_DATA8(0x2C);   
 
// 	LCD_WR_REG(0xC2);     
// 	LCD_WR_DATA8(0x01);   
 
// 	LCD_WR_REG(0xC3);     
// 	LCD_WR_DATA8(0x11);   
 
// 	LCD_WR_REG(0xC4);     
// 	LCD_WR_DATA8(0x20);   //VDV, 0x20:0v
 
// 	LCD_WR_REG(0xC6);     
// 	LCD_WR_DATA8(0x0F);   //0x0F:60Hz13:60Hz   
 
// 	LCD_WR_REG(0xD0);     
// 	LCD_WR_DATA8(0xA4);   
// 	LCD_WR_DATA8(0xA1);   
 
// 	LCD_WR_REG(0xD6);     
// 	LCD_WR_DATA8(0xA1);   //sleep in后，gate输出为GND
 
// 	LCD_WR_REG(0xE0);     
// 	LCD_WR_DATA8(0xD0);   
// 	LCD_WR_DATA8(0x00);   
// 	LCD_WR_DATA8(0x05);   
// 	LCD_WR_DATA8(0x0E);   
// 	LCD_WR_DATA8(0x15);   
// 	LCD_WR_DATA8(0x0D);   
// 	LCD_WR_DATA8(0x37);   
// 	LCD_WR_DATA8(0x43);   
// 	LCD_WR_DATA8(0x47);   
// 	LCD_WR_DATA8(0x09);   
// 	LCD_WR_DATA8(0x15);   
// 	LCD_WR_DATA8(0x12);   
// 	LCD_WR_DATA8(0x16);   
// 	LCD_WR_DATA8(0x19);   
 
// 	LCD_WR_REG(0xE1);     
// 	LCD_WR_DATA8(0xD0);   
// 	LCD_WR_DATA8(0x00);   
// 	LCD_WR_DATA8(0x05);   
// 	LCD_WR_DATA8(0x0D);   
// 	LCD_WR_DATA8(0x0C);   
// 	LCD_WR_DATA8(0x06);   
// 	LCD_WR_DATA8(0x2D);   
// 	LCD_WR_DATA8(0x44);   
// 	LCD_WR_DATA8(0x40);   
// 	LCD_WR_DATA8(0x0E);   
// 	LCD_WR_DATA8(0x1C);   
// 	LCD_WR_DATA8(0x18);   
// 	LCD_WR_DATA8(0x16);   
// 	LCD_WR_DATA8(0x19);   
 
// 	LCD_WR_REG(0xE4);     
// 	LCD_WR_DATA8(0x1D);   //使用240根gate  (N+1)*8
// 	LCD_WR_DATA8(0x00);   //设定gate起点位置
// 	LCD_WR_DATA8(0x00);   //当gate没有用完时，bit4(TMG)设为0
 
// 	LCD_WR_REG(0x21);     
 
// 	LCD_WR_REG(0x29);  

//     
// 	LCD_WR_REG(0x2A);
// } 



void LCD_Init(void)
{
	LCD_GPIO_Init();//初始化GPIO

	HAL_Delay(300); //延迟等待初始化完成

    LCD_RES_Set();
	HAL_Delay(10);
	LCD_RES_Clr();//复位
	HAL_Delay(10);
	LCD_RES_Set();
	HAL_Delay(120);
	
	//************* Start Initial Sequence **********//
	LCD_WR_REG(0x11); //Sleep out 
	HAL_Delay(120);              //Delay 120ms 
	//************* Start Initial Sequence **********// 
	LCD_WR_REG(0x36);

	// LCD_WR_DATA8(0x00);
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
	// else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x60);
	else LCD_WR_DATA8(0xA0);
	
 
	LCD_WR_REG(0x3A);     
	LCD_WR_DATA8(0x05);     //rgb565
    //ADD
	LCD_WR_REG(0xB0);
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0xE0);   
 
	LCD_WR_REG(0xB2);     
	LCD_WR_DATA8(0x0C);   
	LCD_WR_DATA8(0x0C);   
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0x33);   
	LCD_WR_DATA8(0x33);   
 
	LCD_WR_REG(0xB7);     
	LCD_WR_DATA8(0x35);   
 
	LCD_WR_REG(0xBB);     
	LCD_WR_DATA8(0x2B);   //2b
 
	LCD_WR_REG(0xC0);     
	LCD_WR_DATA8(0x2C);   
 
	LCD_WR_REG(0xC2);     
	LCD_WR_DATA8(0x01);   
 
	LCD_WR_REG(0xC3);     
	LCD_WR_DATA8(0x11);   
 
	LCD_WR_REG(0xC4);     
	LCD_WR_DATA8(0x20);   //VDV, 0x20:0v
 
	LCD_WR_REG(0xC6);     
	LCD_WR_DATA8(0x0F);   //0x0F:60Hz13:60Hz   
 
	LCD_WR_REG(0xD0);     
	LCD_WR_DATA8(0xA4);   
	LCD_WR_DATA8(0xA1);   
 
	// LCD_WR_REG(0xD6);     
	// LCD_WR_DATA8(0xA1);   //sleep in后，gate输出为GND
 
	LCD_WR_REG(0xE0);     
	LCD_WR_DATA8(0xD0);   
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x0E);   
	LCD_WR_DATA8(0x15);   
	LCD_WR_DATA8(0x0D);   
	LCD_WR_DATA8(0x37);   
	LCD_WR_DATA8(0x43);   
	LCD_WR_DATA8(0x47);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x15);   
	LCD_WR_DATA8(0x12);   
	LCD_WR_DATA8(0x16);   
	LCD_WR_DATA8(0x19);   
 
	LCD_WR_REG(0xE1);     
	LCD_WR_DATA8(0xD0);   
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x0D);   
	LCD_WR_DATA8(0x0C);   
	LCD_WR_DATA8(0x06);   
	LCD_WR_DATA8(0x2D);   
	LCD_WR_DATA8(0x44);   
	LCD_WR_DATA8(0x40);   
	LCD_WR_DATA8(0x0E);   
	LCD_WR_DATA8(0x1C);   
	LCD_WR_DATA8(0x18);   
	LCD_WR_DATA8(0x16);   
	LCD_WR_DATA8(0x19);   


	LCD_WR_REG(0xE7);     
	LCD_WR_DATA8(0x00);   

	LCD_WR_REG(0x29);  
	LCD_WR_REG(0x2C);  
 
	// LCD_WR_REG(0xE4);     
	// LCD_WR_DATA8(0x1D);   //使用240根gate  (N+1)*8
	// LCD_WR_DATA8(0x00);   //设定gate起点位置
	// LCD_WR_DATA8(0x00);   //当gate没有用完时，bit4(TMG)设为0
 
	// LCD_WR_REG(0x21);     
 
	// LCD_WR_REG(0x29);  

    // //
	// LCD_WR_REG(0x2A);
} 
 
 
 
 
 
 
 