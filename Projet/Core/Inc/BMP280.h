/*
 * BMP280.h
 *
 *  Created on: Nov 24, 2025
 *      Author: hugof
 */

#ifndef INC_BMP280_H_
#define INC_BMP280_H_

//////////////////////////////////////// INCLUDES
#include "main.h"
#include "stdio.h"

//////////////////////////////////////// DEFINES
#define BMP280_I2C_ADDR  (0x77 << 1)
#define BMP280_ID 0xD0
#define BMP280_CTRL_MEAS 0xF4
#define BMP280_CONFIG 0x57 // mode normal, Pressure oversampling x16, Temperature oversampling x2
#define BMP280_CALIBRATION 0x88
#define BMP280_PRESS_MSB 0xF7 // from 0xF7 to 0xFC => respectively pressure and temperature data values

//////////////////////////////////////// DATASHEET FUNCTIONS
int32_t BMP280_compensate_T_int32(int32_t adc_T);
uint32_t BMP280_compensate_P_int32(int32_t adc_P);

//////////////////////////////////////// PERSONAL FUNCTIONS
HAL_StatusTypeDef BMP280_WriteReg(uint8_t reg, uint8_t value);
HAL_StatusTypeDef BMP280_ReadReg(uint8_t reg, uint8_t *value);
HAL_StatusTypeDef BMP280_ReadMulti(uint8_t reg, uint8_t *buf, uint16_t len);
void BMP280_Init(void);
void BMP280_Calibration(void);
void BMP280_ReadRawData(int32_t *raw_temp, int32_t *raw_press);

#endif /* INC_BMP280_H_ */
