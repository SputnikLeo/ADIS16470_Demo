/** 
* @brief        ADIS16470����ͷ�ļ�
* @details  �ǵøĵ�14�е�ͷ�ļ�Ϊ�Լ��Ŀ��ļ�
* @author      WMD
* @date     date 
* @version  
* @par Copyright (c):  
*       WMD 
* @par ��־
*/  
#ifndef _ADXIS_H
#define _ADXIS_H

#include "stm32f4xx_hal.h"
typedef struct
{
	int16_t DIAG_STAT;
	int16_t X_GYRO;
	int16_t Y_GYRO;
	int16_t Z_GYRO;
	int16_t X_ACCL;
	int16_t Y_ACCL;
	int16_t Z_ACCL;
	int16_t TEMP;
	int16_t DATA_CNTR;
	int16_t Checknum;
}ADX_t;
typedef struct
{
	float wx;//���ٶ�
	float wy;
	float wz;
	float accx;//���ٶ�
	float accy;
	float accz;
	float anglex;//�Ƕ�
	float angley;
	float anglez;
}GyroData_t;
extern ADX_t imu;
extern GyroData_t GyroData;
int8_t ADX_BurstRead(void);
int8_t ADX_Read_Reg(uint8_t* addr_Reg,uint16_t* Rx_point,uint8_t Reg_num);
int8_t ADX_Write_Reg(uint8_t addr,uint8_t value);
void ADX_Init(void);//Ϊ��֤spi���ݵ�burst read ���ڽ����� �ȴ�����ȡ
void ADX_Single_Handle(void);//�ɼ�һ������
void Self_Calibration(void);
#endif
