/*
 * RPIREQ.h
 *
 *  Created on: Dec 1, 2025
 *      Author: hugof
 */

#ifndef INC_INTERFACE_STM32_RASPBERRY_H_
#define INC_INTERFACE_STM32_RASPBERRY_H_

//////////////////////////////////////// INCLUDES
#include "main.h"
#include "usart.h"

//////////////////////////////////////// FUNCTIONS
void send_temperature();
void send_pressure();
void send_K();
void send_angle();
void interface_stm32_raspberry_process_command();

#endif /* INC_INTERFACE_STM32_RASPBERRY_H_ */
