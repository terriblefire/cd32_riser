/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

#define USBH_DEBUG_LEVEL 9


#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "stm32f1xx_hal.h"
#include "usbh_hid.h"
#include "usb_host.h"
#include "usbh_def.h"
#include "syscall.h"
#include "debug.h"
#include "stm32f1xx_it.h"
#include "amiga.h"

/* External functions */
extern void MX_USB_HOST_Process(void);

/* Local functions prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI3_Init(void);
static void MX_UART5_Init(void);
void MX_USB_HOST_Process(void);


/* Local variables */
static int debuglevel = DBG_INFO;

static const char *fwBuild = "v0.1rc";
RTC_HandleTypeDef hrtc;
SPI_HandleTypeDef hspi3;
UART_HandleTypeDef huart5;


static void banner(void)
{
	printf("\r\n\r\n" ANSI_BLUE "RETROBITLAB AMIGA USB KEYBOARD ADAPTER" ANSI_RESET "\r\n");
	printf(ANSI_BLUE "-=* STM32F401 BASED BOARD HANDLER  *=-" ANSI_RESET "\r\n");
	printf(ANSI_YELLOW);
	printf("FWVER: %s", fwBuild);
	printf(ANSI_RESET "\r\n");
	printf("\r\n\n");
}

static void led_light(int state)
{
	int tpval = GPIO_PIN_RESET;

	if (!!state)
	{
		tpval = GPIO_PIN_SET;
	}
	else
	{
		tpval = GPIO_PIN_RESET;
	}
	HAL_GPIO_WritePin(TP1_GPIO_Port, TP1_Pin, tpval);
}

static void led_toggle(void)
{
	static int tpval = 0;
	if (tpval == 0)
	{
		tpval = 1;
	}
	else
	{
		tpval = 0;
	}
	led_light(tpval);
}

#define USB_REPORT_RETRY    (6)

static void usb_keyboard_led(USBH_HandleTypeDef *usbhost, keyboard_led_t ld)
{
	keyboard_led_t led = ld;
	USBH_StatusTypeDef status;
	int retrygood = USB_REPORT_RETRY;
	DBG_N("Called: %p -- Led: 0x%02x\r\n", usbhost, led);
	if (usbhost != NULL)
	{
		for (;;)
		{
			status = USBH_HID_SetReport(usbhost, 0x02, 0x00, &led, 1);
			DBG_N("[%d] USB Status: %d\r\n", retrygood, status);
			if (status == USBH_OK)
				retrygood--;
			if (retrygood <= 0)
				break;
		}
	}
	DBG_N("Exit\r\n");
}

static void usb_keyboard_led_init(USBH_HandleTypeDef * usbhost)
{
	keyboard_led_t led = CAPS_LOCK_LED | NUM_LOCK_LED | SCROLL_LOCK_LED;
	int n;
	DBG_N("Called: led: 0x%02x\r\n", led);
	if (usbhost != NULL)
	{
		for (n = 0; n < 2; n++)
		{
			DBG_V("LEDS ON\r\n");
			usb_keyboard_led(usbhost, led);
			mdelay(250);
			DBG_V("LEDS OFF\r\n");
			usb_keyboard_led(usbhost, 0);
			mdelay(125);
		}
		/* Default leds off */
		DBG_V("LEDS OFF\r\n");
		usb_keyboard_led(usbhost, 0);
	}
	DBG_N("Exit\r\n");
}

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
	ApplicationTypeDef aState = APPLICATION_DISCONNECT;
	int timerOneShot = 1;
	int resetTimer = 0;
	led_status_t stat;
	static keyboard_led_t keyboard_led = 0;
	int do_led = 0;
	USBH_HandleTypeDef * usbhost = NULL;
	int keyboard_ready = 0;
	int mouse_ready = 1;
	int count = 0;

	_write_ready(SYSCALL_NOTREADY, &huart5);

	/* MCU Configuration----------------------------------------------------------*/
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_RTC_Init();
	MX_SPI3_Init();
	MX_UART5_Init();

	/* Initialize DEBUG UART */

	_write_ready(SYSCALL_READY, &huart5);
	/* Initialize GPIO for Amiga and assert the nRESET Line */
	amikb_gpio_init();

	banner();

	/* Initialize USB HOST OTG FS */
	MX_USB_HOST_Init();

	/* Infinite loop */
	timer_start();
	/* Now intialize Amiga Pinouts and sync the keyboard */
	amikb_startup();
	amikb_ready(0);

	for (;;)
	{
		MX_USB_HOST_Process();
		aState = USBH_ApplicationState();
		// Se risulta connessa la tastiera USB
		if (aState == APPLICATION_READY)
		{
			DBG_N("APPLICATION READY\r\n");
			usbhost = USBH_GetHost();
			if (usbhost != NULL)
			{
				HID_TypeTypeDef device_type = USBH_HID_GetDeviceType(usbhost);

				if (device_type== HID_KEYBOARD)
				{
					if (!timerOneShot)
					{
						timerOneShot = !timerOneShot;
						DBG_V("#### KEYBOARD CONNECTED ####\r\n");
						timer_start();
					}

					if ( !keyboard_ready )
					{
						DBG_V("### BOARD LED ON ### WAIT 500msec FOR LEDS\r\n");
						led_light(0);
						if (timer_elapsed(500))
						{
							DBG_V("### KEYBOARD LED TOGGLE ###\r\n");
							usb_keyboard_led_init(usbhost);
							keyboard_ready = 1;
							keyboard_led = 0;
						}
					}

					// Get data from keyboard!
					if (USBH_Keybd(usbhost) == 0)
					{
						// Send all received keycode to Amiga
						stat = amikb_process(USBH_GetScanCode());
						// ...and manage the keyboard led
						switch (stat)
						{
							case LED_CAPS_LOCK_OFF:
								DBG_V("CAPS LOCK LED OFF\r\n");
								keyboard_led &= ~CAPS_LOCK_LED;
								do_led = 1;
								break;
							case LED_CAPS_LOCK_ON:
								DBG_V("CAPS LOCK LED ON\r\n");
								keyboard_led |= CAPS_LOCK_LED;
								do_led = 1;
								break;
							case LED_NUM_LOCK_OFF:
								DBG_V("NUM LOCK LED OFF\r\n");
								keyboard_led &= ~NUM_LOCK_LED;
								do_led = 1;
								break;
							case LED_NUM_LOCK_ON:
								DBG_V("NUM LOCK LED ON\r\n");
								keyboard_led |= NUM_LOCK_LED;
								do_led = 1;
								break;
							case LED_SCROLL_LOCK_OFF:
								DBG_V("SCROLL LOCK LED OFF\r\n");
								keyboard_led &= ~SCROLL_LOCK_LED;
								do_led = 1;
								break;
							case LED_SCROLL_LOCK_ON:
								DBG_V("SCROLL LOCK LED ON\r\n");
								keyboard_led |= SCROLL_LOCK_LED;
								do_led = 1;
								break;
							case LED_RESET_BLINK:
								DBG_V("RESET OCCURS FROM AMIGA SIDE\r\n");
								usb_keyboard_led_init(usbhost);
								do_led = 0;
								break;
							default:
							case NO_LED:
								DBG_V("NO ACTION FOR USB REPORT\r\n");
								do_led = 0;
								break;
						}
						// ...and let the led management to be done!
						if (do_led)
						{
							DBG_N("USB ASK FOR REPORT: LED: 0x%02x\r\n", keyboard_led);
							usb_keyboard_led(usbhost, keyboard_led);
						}
					}
					else
					{
						// In IDLE mode, check if there are some
						// RESET request on the CLOCK line.
						// Any EXTERNAL Amiga keyboard will assert low
						// the clock line for more than 500msec to
						// obtain the SYSTEM RESET REQUEST, so do we.
						if (amikb_reset_check())
						{
							// Is it the first time in IDLE state?
							if (!resetTimer)
							{
								// Yes! Startup the timer
								resetTimer = 1;
								timer_start();
							}
							// In reset state
							if (timer_elapsed(500))
							{
								DBG_N("Now it's time for a RESET!\r\n");
								resetTimer = 0;
								amikb_reset();
								amikb_startup();
							}
						}
						else
						{
							// If not in CLOCK LOW MODE, reset the
							// timer for next time...
							resetTimer = 0;
						}
					}
				}
				else if (device_type == HID_MOUSE)
				{
					if (!timerOneShot)
					{
						timerOneShot = !timerOneShot;
						DBG_V("#### MOUSE CONNECTED ####\r\n");
						timer_start();
					}

					mouse_ready = 1;

					if (USBH_mouse(usbhost) == 0)
					{
						amim_process(USBH_GetMouseDelta());

					}

				}
				else
				{
					keyboard_ready = 0;
					// Quick blink on device-not-supported
					DBG_N("#### HID Device NOT SUPPORTED ####\r\n");
					if (!timerOneShot)
					{
						timerOneShot = !timerOneShot;
						timer_start();
					}
					if (timer_elapsed(100))
					{
						DBG_N("UNKNOWN USB DEVICE\r\n");
						led_toggle();
						
						timer_start();
						if (count++ > 10)
						{
							DBG_I("Waiting REAL USB Keyboard - Amiga Is Back!\r\n");
							count = 0;
						}
					}
				}
			}
			else
			{
				// Pretty quick blink on no-hid-device-plugged
				DBG_N("NO HID DEVICE FOUND.\r\n");
				keyboard_ready = 0;
				mouse_ready = 0;

				if (!timerOneShot)
				{
					timerOneShot = !timerOneShot;
					timer_start();
				}
				if (timer_elapsed(250))
				{
					DBG_N("NO HID DEVICE FOUND\r\n");
					led_toggle();
					amikb_notify("NO HID Device. Please Connect - Amiga Is Back!\n");
					timer_start();
					if (count++ > 10)
					{
						DBG_I("Waiting USB HID Keyboard - Amiga Is Back!\r\n");
						count = 0;
					}
				}
			}
		}
		else
		{
			keyboard_ready = 0;
			DBG_N("APPLICATION %d\r\n", aState);
			// On first run, we start a timer and every 1/2 second we
			// toggle LED Pin
			if (timerOneShot)
			{
				DBG_N("UNCONNECTED USB HID KEYBOARD. PLEASE CONNECT\r\n");
				timer_start();
				timerOneShot = 0;
			}
			// slow blink on no device connected
			if (timer_elapsed(500))
			{
				DBG_N("WAIT INSERT USB KEYBOARD\r\n");
				led_toggle();
				amikb_notify("Waiting USB Keyboard - Amiga Is Back!\n");
				timer_start();
				if (count++ > 10)
				{
					DBG_I("Waiting USB Keyboard - Amiga Is Back!\r\n");
					count = 0;
				}
			}
		}
		amikb_ready(keyboard_ready);
	}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.Prediv1Source = RCC_PREDIV1_SOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USB;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV3;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the Systick interrupt time 
  */
  __HAL_RCC_PLLI2S_ENABLE();
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
    
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date 
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 0x1;
  DateToUpdate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 9600;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
