
#include "spi.h"
#include "board.h"
#include "stm32f1xx_hal.h"

void spi8_a(uint8_t keycode, uint8_t period)
{
	for (int i = 0; i < 8; i++)
		{
			// data line is inverted
			if (keycode & 0x80)
			{
				// a logic 1 is low in hardware
				HAL_GPIO_WritePin(KBD_GPIO_Port, KBD_DATA_Pin, GPIO_PIN_SET);
			}
			else
			{
				// a logic 0 is high in hardware
				HAL_GPIO_WritePin(KBD_GPIO_Port, KBD_DATA_Pin, GPIO_PIN_RESET);
			}
			keycode <<= 1;
			udelay(period);
			/* pulse the clock */
			HAL_GPIO_WritePin(KBD_GPIO_Port, KBD_CLOCK_Pin, GPIO_PIN_SET ); // Clear KBD_CLOCK pin
			udelay(period);
			HAL_GPIO_WritePin(KBD_GPIO_Port, KBD_CLOCK_Pin, GPIO_PIN_RESET); // Set KBD_CLOCK pin
			udelay(period);
		}

		HAL_GPIO_WritePin(KBD_GPIO_Port, KBD_DATA_Pin, GPIO_PIN_SET);
}

void spi8(uint8_t keycode, uint8_t period)
{
	for (int i = 0; i < 8; i++)
		{
			// data line is inverted
			if (keycode & 0x80)
			{
				// a logic 1 is low in hardware
				HAL_GPIO_WritePin(KBD_GPIO_Port, KBD_DATA_Pin, GPIO_PIN_RESET);
			}
			else
			{
				// a logic 0 is high in hardware
				HAL_GPIO_WritePin(KBD_GPIO_Port, KBD_DATA_Pin, GPIO_PIN_SET);
			}
			keycode <<= 1;
			udelay(period);
			/* pulse the clock */
			HAL_GPIO_WritePin(KBD_GPIO_Port, KBD_CLOCK_Pin, GPIO_PIN_RESET); // Clear KBD_CLOCK pin
			udelay(period);
			HAL_GPIO_WritePin(KBD_GPIO_Port, KBD_CLOCK_Pin, GPIO_PIN_SET); // Set KBD_CLOCK pin
			udelay(period);
		}

		HAL_GPIO_WritePin(KBD_GPIO_Port, KBD_DATA_Pin, GPIO_PIN_SET); // Set KBD_DATA pin
}
