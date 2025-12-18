/*
 * RPIREQ.c
 *
 *  Created on: Dec 1, 2025
 *      Author: hugof
 */

//////////////////////////////////////// INCLUDES
#include "interface_stm32_raspberry.h"
#include "bmp280.h"
#include "usart.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

//////////////////////////////////////// DEFINES
#define RX_BUF_SIZE 32

//////////////////////////////////////// VARIABLES
extern uint8_t uart1_rx;
extern char command[16];
extern uint8_t cmd_index;

//////////////////////////////////////// FUNCTIONS

void interface_stm32_raspberry_process_command(char *cmd)
{
	int32_t temp_raw_100;
	uint32_t press_raw_100;
	int32_t temp_compensate_100;
	uint32_t press_compensate_100;

    if (strcmp(cmd, "GET_T") == 0)
    {
        if (bmp280_read_temp_press_int(&temp_raw_100, &press_raw_100, &temp_compensate_100, &press_compensate_100) == HAL_OK)
        {
            char msg[25];
            // Format demandé : T=+12.50_C sur 10 caractères
            snprintf(msg, sizeof(msg), "T=%ld.%02ld C", temp_compensate_100 / 100, temp_compensate_100 % 100);
            HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
        }
    }
    else if (strcmp(cmd, "GET_P") == 0)
    {
        if (bmp280_read_temp_press_int(&temp_raw_100, &press_raw_100, &temp_compensate_100, &press_compensate_100) == HAL_OK)
        {
            char msg[25];
            // Format : P=102300Pa (Pa = pression en Pa)
            snprintf(msg, sizeof(msg), "P=%lu.%02lu hPa", press_compensate_100 / 100, press_compensate_100 % 100);
            HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
        }
    }
    else
    {
        char *err = "CMD_ERR";
        HAL_UART_Transmit(&huart1, (uint8_t*)err, strlen(err), HAL_MAX_DELAY);
        HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        // Echo immédiat pour voir ce qu'on tape dans minicom
        HAL_UART_Transmit(&huart1, &uart1_rx, 1, HAL_MAX_DELAY);

        if (uart1_rx != '\n' && uart1_rx != '\r')
        {
            if (cmd_index < sizeof(command) - 1)
            {
                command[cmd_index++] = uart1_rx;
            }
        }
        else
        {
            command[cmd_index] = '\0';

            // On traite la commande
            interface_stm32_raspberry_process_command(command);

            // IMPORTANT : vider le buffer !  Sinon on ne peut pas faire plusieurs requêtes
            memset(command, 0, sizeof(command));

            cmd_index = 0;
        }

        HAL_UART_Receive_IT(&huart1, &uart1_rx, 1);
    }
}
