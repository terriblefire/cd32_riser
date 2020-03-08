/**
  ******************************************************************************
  * @file    usbh_hid_gamepad.h
  * @author  MCD Application Team
  * @brief   This file contains all the prototypes for the usbh_hid_gamepad.c
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      http://www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive  ----------------------------------------------*/
#ifndef __USBH_HID_gamepad_H
#define __USBH_HID_gamepad_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid.h"
#include "stdlib.h"

/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_HID_CLASS
  * @{
  */

/** @defgroup USBH_HID_gamepad
  * @brief This file is the Header file for usbh_hid_gamepad.c
  * @{
  */


/** @defgroup USBH_HID_gamepad_Exported_Types
  * @{
  */

/**
  * @}
  */

/** @defgroup USBH_HID_gamepad_Exported_Defines
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_HID_gamepad_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_HID_gamepad_Exported_Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_HID_gamepad_Exported_FunctionsPrototype
  * @{
  */
USBH_StatusTypeDef USBH_HID_GamepadInit(USBH_HandleTypeDef *phost);
uint8_t *USBH_HID_GetGamepadInfo(USBH_HandleTypeDef *phost);
static uint16_t collect_bits(uint8_t *p, uint16_t offset, uint8_t size, int is_signed);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBH_HID_gamepad_H */

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
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
