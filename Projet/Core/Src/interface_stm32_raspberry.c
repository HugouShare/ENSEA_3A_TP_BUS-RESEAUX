/*
 * RPIREQ.c
 *
 *  Created on: Dec 1, 2025
 *      Author: hugof
 */

//////////////////////////////////////// INCLUDES
#include "interface_stm32_raspberry.h"
#include "usart.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

//////////////////////////////////////// DEFINES
#define RX_BUF_SIZE 32

//////////////////////////////////////// VARIABLES
uint32_t pressure = 102300;
float temperature = 12.50;
float K = 12.34;
float angle = 125.7;

extern uint8_t cmd_received;
extern uint8_t rx_buf[RX_BUF_SIZE];

//////////////////////////////////////// FUNCTIONS
void send_temperature()
{
    char msg[32];
    sprintf(msg, "T=%+06.2f_C", temperature);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, 32, HAL_MAX_DELAY);
}

void send_pressure()
{
    char msg[32];
    sprintf(msg, "P=%06dPa", pressure);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, 32, HAL_MAX_DELAY);
}

void send_K()
{
    char msg[32];
    sprintf(msg, "K=%08.5f", K);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, 32, HAL_MAX_DELAY);
}

void send_angle()
{
    char msg[32];
    sprintf(msg, "A=%08.4f", angle);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, 32, HAL_MAX_DELAY);
}

void interface_stm32_raspberry_process_command()
{
    if (!cmd_received) return;
    cmd_received = 0;

    if (strncmp((char*)rx_buf, "GET_T", 5) == 0) {
        send_temperature();
    }
    else if (strncmp((char*)rx_buf, "GET_P", 5) == 0) {
        send_pressure();
    }
    else if (strncmp((char*)rx_buf, "GET_A", 5) == 0) {
        send_angle();
    }
    else if (strncmp((char*)rx_buf, "GET_K", 5) == 0) {
        send_K();
    }
    else if (strncmp((char*)rx_buf, "SET_K=", 6) == 0) {
        int val = atoi((char*)&rx_buf[6]);
        K = val / 100.0f;   // stocke le coefficient

        HAL_UART_Transmit(&huart1, (uint8_t*)"SET_K=OK", 8, HAL_MAX_DELAY);
    }
}
