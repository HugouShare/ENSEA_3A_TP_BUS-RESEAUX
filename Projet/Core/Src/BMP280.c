/*
 * BMP280.c
 *
 *  Created on: Nov 24, 2025
 *      Author: hugof
 */

//////////////////////////////////////// INCLUDES
#include "BMP280.h"
#include "i2c.h"

//////////////////////////////////////// PERSONAL FUNCTIONS
HAL_StatusTypeDef BMP280_WriteReg(uint8_t reg, uint8_t value)
{
	uint8_t buf[2];
	buf[0] = reg;
	buf[1] = value;

	return HAL_I2C_Master_Transmit(&hi2c1, BMP280_I2C_ADDR, buf, 2, HAL_MAX_DELAY);
}

HAL_StatusTypeDef BMP280_ReadReg(uint8_t reg, uint8_t *value)
{
	// Envoyer l'adresse du registre
	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, BMP280_I2C_ADDR, &reg, 1, HAL_MAX_DELAY);
	if (status != HAL_OK)
	{
		return status;
	}

	// Lire la donnée
	return HAL_I2C_Master_Receive(&hi2c1, BMP280_I2C_ADDR, value, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef BMP280_ReadMulti(uint8_t reg, uint8_t *buf, uint16_t len)
{
	HAL_StatusTypeDef status;

	// Envoyer l'adresse du registre de départ
	status = HAL_I2C_Master_Transmit(&hi2c1, BMP280_I2C_ADDR, &reg, 1, HAL_MAX_DELAY);
	if (status != HAL_OK)
	{
		return status;
	}

	// Lire les octets suivants
	return HAL_I2C_Master_Receive(&hi2c1, BMP280_I2C_ADDR, buf, len, HAL_MAX_DELAY);
}

void BMP280_Init(void)
{
	uint8_t id;

	// Lecture de l'ID du device (0xD0 → doit retourner 0x58)
	BMP280_ReadReg(0xD0, &id);
	if (id != 0x58)
	{
		// Capteur non détecté
		while(1);
	}
	printf("l'ID du BMP280 est : %u",id);

	// Configuration du device
	BMP280_WriteReg(BMP280_CTRL_MEAS, BMP280_CONFIG );
}

uint8_t calibration_values [26];
void BMP280_Calibration(void)
{
	BMP280_ReadMulti(BMP280_CALIBRATION, calibration_values, 26);
}

void BMP280_ReadRawData(int32_t *raw_temp, int32_t *raw_press)
{
	uint8_t data[6];

	BMP280_ReadMulti(BMP280_PRESS_MSB, data, 6);

	*raw_press = (int32_t)((data[0] << 12) | (data[1] << 4) | (data[2] >> 4));
	*raw_temp  = (int32_t)((data[3] << 12) | (data[4] << 4) | (data[5] >> 4));
}

//////////////////////////////////////// DATASHEET FUNCTIONS
int32_t t_fine;

int32_t BMP280_compensate_T_int32(int32_t adc_T)
{
	/*
	 * Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
	 * t_fine carries fine temperature as global value
	 */

	uint16_t dig_T1 = (uint16_t)((calibration_values[2]<<8)|(calibration_values[1]));
	int16_t dig_T2 = (int16_t)((calibration_values[4]<<8)|(calibration_values[3]));
	int16_t dig_T3 = (int16_t)((calibration_values[6]<<8)|(calibration_values[5]));

	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

uint32_t BMP280_compensate_P_int32(int32_t adc_P)
{
	/*
	 * Returns pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
	 */

	uint16_t dig_P1 = (uint16_t)((calibration_values[8]<<8)|(calibration_values[7]));
	int16_t dig_P2 = (int16_t)((calibration_values[10]<<8)|(calibration_values[9]));
	int16_t dig_P3 = (int16_t)((calibration_values[12]<<8)|(calibration_values[11]));
	int16_t dig_P4 = (int16_t)((calibration_values[14]<<8)|(calibration_values[13]));
	int16_t dig_P5 = (int16_t)((calibration_values[16]<<8)|(calibration_values[15]));
	int16_t dig_P6 = (int16_t)((calibration_values[18]<<8)|(calibration_values[17]));
	int16_t dig_P7 = (int16_t)((calibration_values[20]<<8)|(calibration_values[19]));
	int16_t dig_P8 = (int16_t)((calibration_values[22]<<8)|(calibration_values[21]));
	int16_t dig_P9 = (int16_t)((calibration_values[24]<<8)|(calibration_values[23]));

	int32_t var1, var2;
	uint32_t p;
	var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)dig_P6);
	var2 = var2 + ((var1*((int32_t)dig_P5))<<1);
	var2 = (var2>>2)+(((int32_t)dig_P4)<<16);
	var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)dig_P2) * var1)>>1))>>18;
	var1 =((((32768+var1))*((int32_t)dig_P1))>>15);
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
	if (p < 0x80000000)
	{
		p = (p << 1) / ((uint32_t)var1);
	}
	else
	{
		p = (p / (uint32_t)var1) * 2;
	}
	var1 = (((int32_t)dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((int32_t)(p>>2)) * ((int32_t)dig_P8))>>13;
	p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));
	return p;
}
