/*
 * TJA1050.c
 *
 *  Created on: Dec 10, 2025
 *      Author: hugof
 */

#include "tja1050.h"
#include "can.h"

CAN_TxHeaderTypeDef tx_header =
{
		.StdId = 0x61,
	    .TransmitGlobalTime = DISABLE,
	    .IDE = CAN_ID_STD,
	    .RTR = CAN_RTR_DATA,
		.DLC = 1
};

void motor_command_send(int8_t angle_cmd)
{

    uint32_t tx_mailbox;
    uint8_t tx_data[1];

    // Exemple : un seul octet contient la commande d’angle
    tx_data[0] = (uint8_t)angle_cmd;

    // Envoi de la trame CAN
    if (HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data, &tx_mailbox) != HAL_OK)
    {
        Error_Handler();
    }
}

void motor_test_loop(void)
{
    // Démarrage du module CAN
    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
        Error_Handler();
    }

    while (1)
    {
        // +90°
        motor_command_send(+90);
        HAL_Delay(1000);     // 1 seconde

        // -90°
        motor_command_send(-90);
        HAL_Delay(1000);     // 1 seconde
    }
}
