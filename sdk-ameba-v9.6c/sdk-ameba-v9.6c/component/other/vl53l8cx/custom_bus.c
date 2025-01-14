/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : custom_bus.c
  * @brief          : source file for the BSP BUS IO driver
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
*/
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "custom_bus.h"
#include "i2c_api.h"

#include "FreeRTOS.h"
#include "task.h"



//PF1 --- SCL
//PF2 --- SDA


i2c_t   i2cmaster;


#define VL53L8CX_SCL    PF_1 
#define VL53L8CX_SDA    PF_2 


int BSP_I2C4_Init(void)
{
  int ret = 0;

  /* i2c init */
  i2c_init(&i2cmaster, VL53L8CX_SDA, VL53L8CX_SCL);

  /* set i2c freq */
  i2c_frequency(&i2cmaster, 100000);

  return ret;
}

/**
  * @brief  DeInitialize I2C HAL.
  * @retval BSP status
  */
int BSP_I2C4_DeInit(void)
{
  int ret = 0;

  i2c_reset(&i2cmaster);

  return ret;
}

/**
  * @brief  Check whether the I2C bus is ready.
  * @param DevAddr : I2C device address
  * @param Trials : Check trials number
  * @retval BSP status
  */
int BSP_I2C4_IsReady(unsigned short DevAddr, int Trials)
{
  int ret = 0;
 
  return ret;
}

/**
  * @brief  Write a value in a register of the device through BUS.
  * @param  DevAddr Device address on Bus.
  * @param  Reg    The target register address to write
  * @param  pData  Pointer to data buffer to write
  * @param  Length Data Length
  * @retval BSP status
  */

int BSP_I2C4_WriteReg(unsigned short DevAddr, unsigned short Reg, unsigned char *pData, unsigned short Length)
{
  int ret = 0;

  // i2c_write_reg(&i2cmaster, Reg, pData, Length, 1);
  
  return ret;
}

/**
  * @brief  Read a register of the device through BUS
  * @param  DevAddr Device address on Bus.
  * @param  Reg    The target register address to read
  * @param  pData  Pointer to data buffer to read
  * @param  Length Data Length
  * @retval BSP status
  */
int  BSP_I2C4_ReadReg(unsigned short DevAddr, unsigned short Reg, unsigned char *pData, unsigned short Length)
{
   int ret = 0;

  // i2c_read_reg(&i2cmaster, Reg ,pData , Length, 1);
   return ret;
}

/**
  * @brief  Write a value in a register of the device through BUS.
  * @param  DevAddr Device address on Bus.
  * @param  Reg    The target register address to write

  * @param  pData  Pointer to data buffer to write
  * @param  Length Data Length
  * @retval BSP statu
  */

int BSP_I2C4_WriteReg16(unsigned short DevAddr, unsigned short Reg, unsigned char *pData, unsigned short Length)
{
   int ret = 0;

   i2c_write_reg(&i2cmaster, Reg, pData, Length, 1);
  
   return ret;
}

/**
  * @brief  Read registers through a bus (16 bits)
  * @param  DevAddr: Device address on BUS
  * @param  Reg: The target register address to read
  * @param  Length Data Length
  * @retval BSP status
  */
int  BSP_I2C4_ReadReg16(unsigned short DevAddr, unsigned short Reg, unsigned char *pData, unsigned short Length)
{
    int ret = 0;

    i2c_read_reg(&i2cmaster, Reg ,pData , Length, 1);

    return ret;
}

/**
  * @brief  Send an amount width data through bus (Simplex)
  * @param  DevAddr: Device address on Bus.
  * @param  pData: Data pointer
  * @param  Length: Data length
  * @retval BSP status
  */
int BSP_I2C4_Send(unsigned short DevAddr, unsigned char *pData, unsigned short Length) {

  int ret = 0;

  return ret;
}

/**
  * @brief  Receive an amount of data through a bus (Simplex)
  * @param  DevAddr: Device address on Bus.
  * @param  pData: Data pointer
  * @param  Length: Data length
  * @retval BSP status
  */
int BSP_I2C4_Recv(unsigned short DevAddr, unsigned char *pData, unsigned short Length) {

  int ret = 0;
  
  return ret;
}


/**
  * @brief Register Default BSP I2C4 Bus Msp Callbacks
  * @retval BSP status
  */
int BSP_I2C4_RegisterDefaultMspCallbacks (void)
{
  return 0;
}

/**
  * @brief BSP I2C4 Bus Msp Callback registering
  * @param Callbacks     pointer to I2C4 MspInit/MspDeInit callback functions
  * @retval BSP status
  */

int BSP_GetTick(void) {
  
  return xTaskGetTickCount();

  return 0;
}


