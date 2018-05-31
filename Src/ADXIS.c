/** 
* @brief        ADIS16470���׹��ܿ�(����STM32��HAL��)
* @details  Ϊ�������ṩ��������,����ͨ�������ȡ�����ǵ�������ٶ�,���ٶ��뵱ǰ��̬�� \n
*						STM32HAL�⿪����ʹ��: \n
*						��Cube����������һ��SPI�豸Ϊȫ˫��,ע�����CPOLΪHIGH,CHPA=2,�����Ҫʹ��BurstRead��Ҫʹ������С��1Mb/s,����ʹ������С��2Mb/s \n
*						��������һ��SPI�⻹��Ҫ����һ��GPIO_OutputΪSPI�豸Ƭѡ��,����������labelΪSPI_NSS,��ʽΪ��©(����)��� \n
* 					���stm32�ͺŸ���ADXIS.h�ڵ�ͷ�ļ�Ϊ���̶�Ӧ��stm32ͷ�ļ�,�ڱ��ļ����޸�ADX_SPIΪ����ѡ�õ�SPI��� \n
*						������Ҫ�ȵ���ADX_Init(void)�󷽿�ʹ��Burst_read��Single_Handle�������� \n
*						��STM32HAL��: \n
*						����ʵ�ֺ궨��NSS_SET��NSS_RESET Ƭѡ������/���ͺ��� \n
*						����ʵ�� HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout)����,��check����Ϊunsigned char����ͬ��
* @author      WMD
* @date     2018��3��27��15:43:21
* @version  1.1
* @par Copyright (c):  
*       WMD 
* @par ��־
*/  
#include "ADXIS.h"
#include "spi.h"
#include "string.h"
#define NSS_SET HAL_GPIO_WritePin(SPI_NSS_GPIO_Port,SPI_NSS_Pin,GPIO_PIN_SET)
#define NSS_RESET HAL_GPIO_WritePin(SPI_NSS_GPIO_Port,SPI_NSS_Pin,GPIO_PIN_RESET)
#define ADX_SPI hspi1 //ʹ���ĸ�spi
ADX_t imu;//���������ݽṹ��
GyroData_t GyroData;
/** 
* @brief �ÿ�ʹ��Ĭ��ģʽ,������ԭʼ���ݸ���Ƶ��2000Hz
*/

static int16_t Send_Cmd=0x6800;//Burst Readָ��
static void sb_delay(volatile uint32_t t)
{
	while(t--);
}
//�������ݵĴ���
void ADX_Init(void)//Ϊ��֤spi���ݵ�burst read ���ڽ����� �ȴ�����ȡ
{
	ADX_Write_Reg(0x68,0x80);//����ָ��
	ADX_Write_Reg(0x69,0x00);
	HAL_Delay(400);//�ȴ��������
	
	ADX_Write_Reg(0x5C,0x03);//�������ð�����˹�˲����ȼ�
	ADX_Write_Reg(0x5D,0x00);

	ADX_Write_Reg(0x64,0x03);//�趨��ֵ�˲���ֵΪ4 ��ʱ�ǶȲ��������Ƶ��Ϊ2000/(3+1)=500Hz
	ADX_Write_Reg(0x65,0x00);
}
static uint16_t ADX_flame_TandR(uint16_t trans)
{
	NSS_RESET;
	uint16_t result;
	static HAL_StatusTypeDef state;
	state=HAL_SPI_TransmitReceive(&ADX_SPI,(uint8_t*)&trans,(uint8_t*)&result,1,0xff);
	if(state!=HAL_OK)
	{
		while(1);
	}
	NSS_SET;
	sb_delay(150);
	return result;
}
//��ȡ�Ĵ����ĺ���,��ΪSPI�ķ�ʽ ��������ȡ����Ч������,���Խ���������ȡ
/*
@parameter:
	addr_Reg ����ȡ�ļĴ�������,Rx_point
	Rx_point ��ȡ������ݷŵ�λ��
	Reg_num	 ����ȡ�ļĴ�������
@return:
	0 �ɹ�
	1 ����  //emmmm������ʱ��û��ʵ�ִ�����
*/
int8_t ADX_Read_Reg(uint8_t* addr_Reg,uint16_t* Rx_point,uint8_t Reg_num)
{
	uint16_t Tx_tmp=0,Rx_tmp=0;
	
	//���ݵ�һ֡ ֻ������
	Tx_tmp=addr_Reg[0]<<8;
	Rx_tmp=ADX_flame_TandR(Tx_tmp);
	for(uint8_t i=1;i<Reg_num;i++)//+1����Ϊspi��һ֡�ӳ�
	{
		Tx_tmp=addr_Reg[i]<<8;//׼������֡�ĸ�ʽ
		Rx_tmp=ADX_flame_TandR(Tx_tmp);
		Rx_point[i-1]=Rx_tmp;
	}
	//�������һ֡ ֻ�Ӳ���
	Tx_tmp=0;
	Rx_point[Reg_num-1]=ADX_flame_TandR(Tx_tmp);
	
	return 0;
}
/** 
* @brief  ��ADIS16470�Ĵ����ڲ�д��������
* @param[in]   addr д�ĵ�ַ
* @param[in]   value д��ֵ
* @retval  0  �ɹ� 
* @retval  -1   ����(δʵ��)
* @par ��־ 
*
*/
int8_t ADX_Write_Reg(uint8_t addr,uint8_t value)
{
	addr|=0x80;//д���ݵ�����
	uint16_t Tx_tmp=(addr<<8) | value;
	ADX_flame_TandR(Tx_tmp);
	return 0;
}
/** 
* @brief  ����BurstReadһ���Ի��������ٶ�����ٶ�,16λ����
* @param[in]   none
* @par ��־ 
*
*/
int8_t ADX_BurstRead()
{
	NSS_RESET;
	static uint8_t* u8point=(uint8_t*)&imu;
	int16_t parity=0;
	HAL_StatusTypeDef check=HAL_OK;
	uint16_t tmpRx=0;
	static uint16_t tmpTx[10]={0};
	check|=HAL_SPI_TransmitReceive(&ADX_SPI,(uint8_t*)&Send_Cmd,(uint8_t*)&tmpRx,1,0xff);//����ָ��,��16λ����������,�ʲ���������
	//ע�⵽�����ֲ���ADXIS16470�ķ��͸�ʽ�ǵ͵�ַ��ǰ �ߵ�ַ�ں� ����stm32��С��ģʽ ���Բ���Ҫ����λ����
	if(check!=HAL_OK)while(1);
	sb_delay(100);
	check|=HAL_SPI_TransmitReceive(&ADX_SPI,(uint8_t*)&tmpTx,u8point,10,0xff);//����20�ֽڵ����ݷ���imu�ṹ����
	if(check!=HAL_OK)while(1);
	NSS_SET;
	sb_delay(150);
	//��������У��
	for(uint8_t i=0;i<9*2;i++)
	{
		parity+=u8point[i];
	}
	if(parity==imu.Checknum && !check)return 0;
	else 
	{
		memset(&imu,0x00,sizeof(imu));
		return -1;//У��ʧ��
	}
}
/** 
* @brief  ���ö��Ĵ����ķ�ʽ���������������ٶ���������ٶ���������̬��,32λ����
* @param[in]   void
* @par ��־ 
*
*/
void ADX_Single_Handle(void)
{
	//���ڱ�����һ�εĽǶ�ֵ ���¼ӻ�ȥ
	float lastx=GyroData.anglex;
	float lasty=GyroData.angley;
	float lastz=GyroData.anglez;
	
	static uint8_t addr[]=
	{ 0x04,0x06,//Gyro_X
		0x08,0x0A,//Gyro_Y
		0x0C,0x0E,//Gyro_Z
		0x10,0x12,//Acc_X
		0x14,0x16,//Acc_Y
		0x18,0x1A,//Acc_Z
		0x24,0x26,//Delta_X ע�������������2ms�ڵĽǶȲ�
		0x28,0x2A,//Delta_Y
		0x2C,0x2E,//Delta_Z
	};
	static uint16_t data[sizeof(addr)];
	int32_t* point32=(int32_t*)data;//���ڽ�����int16_t���ͺϳ�int32_t����
	float* Gyro_float=(float*)&GyroData;//���ڽ�int32_t����ת����float����
	ADX_Read_Reg(addr,data,sizeof(addr));
	//52428.8f
	uint8_t i;
	for(i=0;i<3;i++)//Gyro 0.1 ��/s=2^16LSB
	{
		*(Gyro_float+i)=*(point32+i)/655360.0f;
	}
	for(;i<6;i++)//Acc 1.25 m/s^2=2^16LSB
	{
		*(Gyro_float+i)=*(point32+i)/52428.8f;
	}
	for(;i<9;i++)//Angle 2160 ��=2^31LSB
	{
		*(Gyro_float+i)=*(point32+i)/994205.4f;
	}
	//�ӻؽǶ�ֵ 
	GyroData.anglex+=lastx;
	GyroData.angley+=lasty;
	GyroData.anglez+=lastz;
}
/** 
* @brief  �������ǽ�����ƫУ׼
* @note		�ú��������ڿ�ʼʱִ��,ִ��ʱ��������ò�Ҫ�ƶ� ��Ϊ�����ǿ��Ա���У׼ֵ,����У׼��һ����ÿ������ʱ�����
* @par ��־ 
*
*/
void Self_Calibration(void)
{
	int32_t RawBiasData[6]={0};//�洢�����Ƶĵ�ǰ���ֵ
	uint8_t addr=0x40;//XG_BIAS_LOW�ĵ�ַ ��Ϊ�����ĵ�ַ�������� ����ȡ�����ʼ
	uint8_t* writedata=(uint8_t*)RawBiasData;//��д���ڴ���׵�ַ ��addrһ�����
	for(uint8_t i=0;i<24;i++)//24��6��У׼ֵ*4 ��Ϊһ��У׼ֵ��4���ֽڵ� ������ִ��һ�ν�������
	{
		ADX_Write_Reg(addr+i,writedata[i]);
	}
	uint16_t count=0;
	uint32_t timestamp;
	GyroData_t GyroInt={0};//���ڻ����õ���ʱ�ṹ��
	while(count<5000)//У׼10��
	{
		ADX_Single_Handle();
		timestamp=HAL_GetTick();
		while(HAL_GetTick()-timestamp<2);
		count++;
	}
	GyroInt.anglex=GyroData.anglex;
	GyroInt.angley=GyroData.angley;
	GyroInt.anglez=GyroData.anglez;
	//ִ�е�����˵�����ݲɼ�����
	float* pointf=(float*)&GyroInt;//�����ַ ���ڴ�Gyro_Data�л�ȡ����
	uint8_t i=0;
	for(;i<3;i++)
	{
		RawBiasData[i]=-(*(pointf+i+6)) *131072.0f/2.0f;//����������ת����int32_t ע����ʱ��У׼ֵȡ���ǽǶ�ֵ 5s�ĽǶȱ仯/5�͵��ڽ��ٶ��� ���Դ˴�����5
	}
	//ִ�е����� ����У׼����׼����� ׼��д��
	for(uint8_t i=0;i<12;i++)//12��3��У׼ֵ*4 ��Ϊһ��У׼ֵ��4���ֽڵ�
	{
		ADX_Write_Reg(addr+i,writedata[i]);
	}
}