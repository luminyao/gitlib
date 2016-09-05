#include "miniIMU.h"

//void ReadDataPacket()
//{
//		if (IMURXData == 0X55 )    //��⵽��ͷ׼������11������
//		{
//			IMUcounter = 0;   //����׼������11������
//			IMURXDataBuf[0] = 0X55;
//		}
//		if ( IMURXDataBuf[0] == 0X55 && IMUcounter != 0 )
//		{
//			IMURXDataBuf[IMUcounter] = IMURXData;	
//		}
//		IMUcounter++;
//		if ( IMUcounter == 12 )
//		{
//			IMURXDataBuf[0] = 0;   //���֡ͷ
//			IMUcounter = 0;
//		}
//}

void IMU_Data_Explain()  //IMU���ݽ���
{
	if ( RX_Data_flg == 1 ) //�������ʱ�����Խ������ݰ���
	{
		GPIO_ResetBits(GPIOD,GPIO_Pin_13);
		switch( IMURXDataBuf[1] )
		{
			//case 0X51:
	
			case 0X52:
				Zrotate_Ang = ( ( IMURXDataBuf[7]<<8 ) | IMURXDataBuf[6] ) / 32768.0 * 2000;
				break;
			case 0X53:
				X_Angle = ( ( IMURXDataBuf[3]<<8 ) | IMURXDataBuf[2] ) / 32768.0 * 180;
				Y_Angle = ( ( IMURXDataBuf[5]<<8 ) | IMURXDataBuf[4] ) / 32768.0 * 180;
				Z_Angle = ( ( IMURXDataBuf[7]<<8 ) | IMURXDataBuf[6] ) / 32768.0 * 180;
				break;
			default:  break;
		}
	}
}

void USART2_IRQHandler()
{
	GPIO_SetBits(GPIOD,GPIO_Pin_13);
	//if ( USART_GetITStatus(USART2,USART_IT_RXNE)==SET )  //����ȡ�Ĵ����Ƿ�������
	//{
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
		IMURXData = USART2->DR;
	//}
	if (IMURXData == 0X55 )    //��⵽��ͷ׼������11������
	{
		RX_Data_flg = 0;  //�Ӱ���ʼ���������ݽ��
		IMUcounter = 0;   //����׼������11������
		IMURXDataBuf[0] = 0X55;
	}
	if ( IMURXDataBuf[0] == 0X55 && IMUcounter != 0 )
	{
		IMURXDataBuf[IMUcounter] = IMURXData;	
	}
	IMUcounter++;
	if ( IMUcounter == 11 )
	{
		RX_Data_flg = 1;  //�Ӱ�����ˣ����Խ��������
		IMURXDataBuf[0] = 0;   //���֡ͷ
		IMUcounter = 0;
	}
	//USART2->SR = 0;   //��������жϱ�־λ
}


void InitIMU()
{
	InitIMUUSART();       //��ʼ��IMU����ΪUSART2
	InitIMUUSARTGPIO();   //��ʼ��USART2��GPIO�ڣ�TX--PA2��RX--PA3
	InitIMUUSART2_NVIC();
}


void InitIMUUSART()   //��ʼ��IMU����ΪUSART2
{
	USART_InitTypeDef  InitIMUUstruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	
	InitIMUUstruct.USART_BaudRate=115200;
	InitIMUUstruct.USART_WordLength=USART_WordLength_8b;
	InitIMUUstruct.USART_StopBits=USART_StopBits_1;
	InitIMUUstruct.USART_Parity=USART_Parity_No;
	InitIMUUstruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	InitIMUUstruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_Init(USART2,&InitIMUUstruct);
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);   //ʹ�ܽ����ж�							
	USART_Cmd(USART2, ENABLE); 
}

void InitIMUUSARTGPIO( )  //��ʼ��USART2��GPIO�ڣ�TX--PA2��RX--PA3
{
	GPIO_InitTypeDef GPIO_InitStructure;					//����һ��GPIO�ṹ�����

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	
															//ʹ�ܸ����˿�ʱ�ӣ���Ҫ������

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 				//���ô��ڽ��ն˿ڹҽӵ�9�˿�
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	   		//���ù��������©
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   	//���ö˿��ٶ�Ϊ50M
  	GPIO_Init(GPIOA, &GPIO_InitStructure);				   	//���ݲ�����ʼ��GPIOA�Ĵ���	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//��������(��λ״̬);	   				
  	GPIO_Init(GPIOA, &GPIO_InitStructure);				   	//���ݲ�����ʼ��GPIOA�Ĵ���
}

void InitIMUUSART2_NVIC()
{
	NVIC_InitTypeDef USART2_NVIC;
	#ifdef  VECT_TAB_RAM  							//���������ַѡ��
	
		NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);  	//��0x20000000��ַ��Ϊ���������ַ(RAM)
	#else  
	
		NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0); //��0x08000000��ַ��Ϊ���������ַ(FLASH)  
	#endif
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	USART2_NVIC.NVIC_IRQChannel= USART2_IRQChannel;
	USART2_NVIC.NVIC_IRQChannelPreemptionPriority=1;
	USART2_NVIC.NVIC_IRQChannelSubPriority=1;
	USART2_NVIC.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&USART2_NVIC);
}
