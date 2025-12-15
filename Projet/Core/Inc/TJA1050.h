/*
 * TJA1050.h
 *
 *  Created on: Dec 10, 2025
 *      Author: hugof
 */

#ifndef INC_TJA1050_H_
#define INC_TJA1050_H_

#include "main.h"

void motor_command_send(int8_t angle_cmd);
void motor_test_loop(void);

#endif /* INC_TJA1050_H_ */
