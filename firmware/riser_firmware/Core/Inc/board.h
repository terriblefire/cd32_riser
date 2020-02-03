/*
 * board.h
 *
 *  Created on: Jan 30, 2020
 *      Author: stephen
 */

#ifndef INC_BOARD_H_
#define INC_BOARD_H_

#define CD32_RISER_REV0

#ifdef CD32_RISER_REV0
#define KBD_GPIO_Port GPIOB
#define KBD_CLOCK_Pin GPIO_PIN_3
#define KBD_DATA_Pin GPIO_PIN_5
#define KBD_RESET_Pin GPIO_PIN_2
#endif


#endif /* INC_BOARD_H_ */
