#include "user_io.h"

#include "stm32f1xx_hal.h"

// mouse position storage for ps2 and minimig rate limitation
#define X 0
#define Y 1
#define MOUSE_FREQ 20   // 20 ms -> 50hz

static int16_t mouse_pos[2] = { 0, 0};
static uint8_t mouse_flags = 0;
static uint8_t joy0dat_low = 0;
static uint8_t joy0dat_high = 0;


typedef struct MOUSE_CMD {
   uint8_t cmd;
   int8_t x;
   int8_t y;
   uint8_t flags;
} __attribute__((packed)) MOUSE_CMD;

extern SPI_HandleTypeDef hspi3;

void spi_mouse_io()
{
	MOUSE_CMD packet;

	int8_t x_delta;
	int8_t y_delta;

	// ----- X axis -------
	if(mouse_pos[X] < -128)
	{
		x_delta = -128;
		mouse_pos[X] += 128;
	}
	else if(mouse_pos[X] > 127)
	{
		x_delta = 127;
		mouse_pos[X] -= 127;
	}
	else
	{
		x_delta = mouse_pos[X];
		mouse_pos[X] = 0;
	}

	// ----- Y axis -------
	if(mouse_pos[Y] < -128)
	{
		y_delta = -128;
		mouse_pos[Y] += 128;
	}
	else if(mouse_pos[Y] > 127)
	{
		y_delta = 127;
		mouse_pos[Y] -= 127;
	}
	else
	{
		y_delta = mouse_pos[Y];
		mouse_pos[Y] = 0;
	}

	joy0dat_low += y_delta;
	joy0dat_high += x_delta;

	packet.cmd = UIO_MOUSE;
	if (mouse_flags & 0x80)
	{
		packet.flags = mouse_flags & 0x03;
	}
	else
	{
		packet.flags = 0;
	}
	packet.x = joy0dat_low;
	packet.y = joy0dat_high;
	mouse_flags = 0;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET ); // NSS1 low
	HAL_SPI_Transmit(&hspi3, (uint8_t*)&packet, sizeof(MOUSE_CMD), 1000);
	while( hspi3.State == HAL_SPI_STATE_BUSY );  // wait xmission complete
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET ); // NSS1 high


}


void user_io_mouse(unsigned char b, int8_t x, int8_t y)
{
	// send mouse data as minimig expects it
    mouse_pos[X] += x;
    mouse_pos[Y] += y;
    mouse_flags |= 0x80 | (b&3);
}

void user_io_keyboard(int state)
{
	unsigned char keycode[2] = {0x05, state ? 0x81 : 0x01};

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET ); // NSS1 low
	HAL_SPI_Transmit(&hspi3, (uint8_t*)&keycode, 2, 1000);
	while( hspi3.State == HAL_SPI_STATE_BUSY );  // wait xmission complete
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET ); // NSS1 high
}


