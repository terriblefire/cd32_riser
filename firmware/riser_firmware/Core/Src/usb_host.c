/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file            : usb_host.c
  * @version         : v2.0_Cube
  * @brief           : This file implements the USB Host
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "debug.h"
#include "amiga.h"
#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_hid.h"

static int debuglevel = DBG_INFO;

/* USB Host core handle declaration */
USBH_HandleTypeDef hUsbHostFS;
ApplicationTypeDef Appli_state = APPLICATION_IDLE;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * user callback declaration
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * Init USB host library, add supported class and start the library
  * @retval None
  */
void MX_USB_HOST_Init(void)
{
  /* USER CODE BEGIN USB_HOST_Init_PreTreatment */
  
  /* USER CODE END USB_HOST_Init_PreTreatment */
  
  /* Init host Library, add supported class and start the library. */
  if (USBH_Init(&hUsbHostFS, USBH_UserProcess, HOST_FS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_RegisterClass(&hUsbHostFS, USBH_HID_CLASS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_Start(&hUsbHostFS) != USBH_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_HOST_Init_PostTreatment */
  
  /* USER CODE END USB_HOST_Init_PostTreatment */
}

/*
 * Background task
 */
void MX_USB_HOST_Process(void)
{
  /* USB Host Background task */
  USBH_Process(&hUsbHostFS);
}
/*
 * user callback definition
 */
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id)
{
  /* USER CODE BEGIN CALL_BACK_1 */
  switch(id)
  {
  case HOST_USER_SELECT_CONFIGURATION:
  break;

  case HOST_USER_DISCONNECTION:
  Appli_state = APPLICATION_DISCONNECT;
  break;

  case HOST_USER_CLASS_ACTIVE:
  Appli_state = APPLICATION_READY;
  break;

  case HOST_USER_CONNECTION:
  Appli_state = APPLICATION_START;
  break;

  default:
  break;
  }
  /* USER CODE END CALL_BACK_1 */
}

ApplicationTypeDef USBH_ApplicationState(void)
{
	return Appli_state;
}

USBH_HandleTypeDef * USBH_GetHost(void)
{
	return &hUsbHostFS;
}

static keyboard_code_t keycode;
static mouse_delta_t mouse_delta;

int USBH_Keybd(USBH_HandleTypeDef *phost)
{
	HID_KEYBD_Info_TypeDef *k_pinfo;

	DBG_N("Enter with phost %p\r\n", phost);

	k_pinfo = USBH_HID_GetKeybdInfo(phost);

	if (k_pinfo != NULL)
	{
		int i;
		DBG_V(  "lctrl = %d lshift = %d lalt   = %d\r\n"
				"lgui  = %d rctrl  = %d rshift = %d\r\n"
				"ralt  = %d rgui   = %d\r\n",
				k_pinfo->lctrl, k_pinfo->lshift, k_pinfo->lalt,
				k_pinfo->lgui, k_pinfo->rctrl, k_pinfo->rshift,
				k_pinfo->ralt, k_pinfo->rgui);
		keycode.lctrl = k_pinfo->lctrl;
		keycode.lshift = k_pinfo->lshift;
		keycode.lalt = k_pinfo->lalt;
		keycode.lgui = k_pinfo->lgui;
		keycode.rctrl = k_pinfo->rctrl;
		keycode.rshift = k_pinfo->rshift;
		keycode.ralt = k_pinfo->ralt;
		keycode.rgui = k_pinfo->rgui;
		for (i = 0; i < KEY_PRESSED_MAX; i++)
		{
			if (debuglevel >= DBG_VERBOSE) {
				printf("--- KEY: %d -- KeyCODE: 0x%02x\r\n", i, k_pinfo->keys[i]);
			}
			keycode.keys[i] = k_pinfo->keys[i];
		}
		if (debuglevel >= DBG_VERBOSE) printf("\n\r");
		return 0;
	}
	else
	{
		return -1; // Device NOT READY
	}
}

int USBH_mouse(USBH_HandleTypeDef *phost)
{
 	HID_MOUSE_Info_TypeDef *mouseInfo = USBH_HID_GetMouseInfo(&hUsbHostFS);

  mouse_delta.x = 0;
  mouse_delta.y = 0;
  mouse_delta.buttons = 0;

  if (mouseInfo != NULL)
  {
    mouse_delta.x = mouseInfo->x;
    mouse_delta.y = mouseInfo->y;
    mouse_delta.buttons = (mouseInfo->buttons[0] & 3);
  }   

  return 0;
}

mouse_delta_t * USBH_GetMouseDelta(void)
{
	return &mouse_delta;
}

keyboard_code_t * USBH_GetScanCode(void)
{
	return &keycode;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
