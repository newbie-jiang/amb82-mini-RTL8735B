/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : custom_bus.h
  * @brief          : header file for the BSP BUS IO driver
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CUSTOM_BUS_H
#define CUSTOM_BUS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "custom_conf.h"
#include "custom_errno.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup CUSTOM
  * @{
  */

/** @defgroup CUSTOM_BUS CUSTOM BUS
  * @{
  */

/** @defgroup CUSTOM_BUS_Exported_Constants CUSTOM BUS Exported Constants
  * @{
  */

#define BUS_I2C4_INSTANCE I2C4
#define BUS_I2C4_SCL_GPIO_CLK_ENABLE() __HAL_RCC_GPIOH_CLK_ENABLE()
#define BUS_I2C4_SCL_GPIO_CLK_DISABLE() __HAL_RCC_GPIOH_CLK_DISABLE()
#define BUS_I2C4_SCL_GPIO_PIN GPIO_PIN_11
#define BUS_I2C4_SCL_GPIO_AF GPIO_AF4_I2C4
#define BUS_I2C4_SCL_GPIO_PORT GPIOH
#define BUS_I2C4_SDA_GPIO_CLK_ENABLE() __HAL_RCC_GPIOH_CLK_ENABLE()
#define BUS_I2C4_SDA_GPIO_CLK_DISABLE() __HAL_RCC_GPIOH_CLK_DISABLE()
#define BUS_I2C4_SDA_GPIO_PIN GPIO_PIN_12
#define BUS_I2C4_SDA_GPIO_AF GPIO_AF4_I2C4
#define BUS_I2C4_SDA_GPIO_PORT GPIOH

#ifndef BUS_I2C4_POLL_TIMEOUT
   #define BUS_I2C4_POLL_TIMEOUT                0x1000U
#endif
/* I2C4 Frequency in Hz  */
#ifndef BUS_I2C4_FREQUENCY
   #define BUS_I2C4_FREQUENCY  1000000U /* Frequency of I2Cn = 100 KHz*/
#endif




/* BUS IO driver over I2C Peripheral */
// HAL_StatusTypeDef MX_I2C4_Init(I2C_HandleTypeDef* hi2c);
int BSP_I2C4_Init(void);
int BSP_I2C4_DeInit(void);
int BSP_I2C4_IsReady(unsigned short DevAddr, int Trials);
int BSP_I2C4_WriteReg(unsigned short Addr, unsigned short Reg, unsigned char *pData, unsigned short Length);
int BSP_I2C4_ReadReg(unsigned short Addr, unsigned short Reg, unsigned char *pData, unsigned short Length);
int BSP_I2C4_WriteReg16(unsigned short Addr, unsigned short Reg, unsigned char *pData, unsigned short Length);
int BSP_I2C4_ReadReg16(unsigned short Addr, unsigned short Reg, unsigned char *pData, unsigned short Length);
int BSP_I2C4_Send(unsigned short DevAddr, unsigned char *pData, unsigned short Length);
int BSP_I2C4_Recv(unsigned short DevAddr, unsigned char *pData, unsigned short Length);
int BSP_I2C4_SendRecv(unsigned short DevAddr, unsigned char *pTxdata, unsigned char *pRxdata, unsigned short Length);




#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1U)

#endif /* (USE_HAL_I2C_REGISTER_CALLBACKS == 1U) */

 int BSP_GetTick(void);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
#ifdef __cplusplus
}
#endif

#endif /* CUSTOM_BUS_H */

