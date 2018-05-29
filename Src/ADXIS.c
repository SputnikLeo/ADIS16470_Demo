//2018��3��27��15:43:21 WMD 
#include "ADXIS.h"
#include "spi.h"
#include "string.h"
#define NSS_SET HAL_GPIO_WritePin(SPI_NSS_GPIO_Port,SPI_NSS_Pin,GPIO_PIN_SET)
#define NSS_RESET HAL_GPIO_WritePin(SPI_NSS_GPIO_Port,SPI_NSS_Pin,GPIO_PIN_RESET)
//#define CHANGE(x) (((x)&0xff00)>>8) | (((x)&0x00ff)<<8) //��С�˻�ת
#define ADX_SPI hspi1 //ʹ���ĸ�spi
ADX_t imu;


static int16_t Send_Cmd=0x6800;//Burst Readָ��
static void sb_delay(volatile uint32_t t)
{
	while(t--);
}
//�������ݵĴ���
void ADX_Init(void)//Ϊ��֤spi���ݵ�burst read ���ڽ����� �ȴ�����ȡ
{
	ADX_Write_Reg(0x69,0x40);//����ָ��
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
	
//	Rx_tmp=ADX_flame_TandR(Tx_tmp);
//	sb_delay(100000);
//	Rx_tmp=ADX_flame_TandR(Tx_tmp);
//	sb_delay(100000);
//	Rx_point[0]=Rx_tmp;
	return 0;
}
int8_t ADX_Write_Reg(uint8_t addr,uint8_t value)
{
	addr|=0x80;//д���ݵ�����
	uint16_t Tx_tmp=(addr<<8) | value;
	ADX_flame_TandR(Tx_tmp);
	return 0;
}
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
		//memset(&imu,0x00,sizeof(imu));
		return -1;//У��ʧ��
	}
}

