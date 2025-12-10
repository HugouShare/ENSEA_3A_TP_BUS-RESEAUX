/*
 * RPIREQ.h
 *
 *  Created on: Dec 1, 2025
 *      Author: hugof
 */

#ifndef INC_RPIREQ_H_
#define INC_RPIREQ_H_

//////////////////////////////////////// INCLUDES
#include "main.h"
#include "usart.h"

//////////////////////////////////////// FUNCTIONS
void send_temperature();
void send_pressure();
void send_K();
void send_angle();
void process_command();

#endif /* INC_RPIREQ_H_ */
