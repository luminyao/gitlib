#include "miniIMU.h"

//void ReadDataPacket()
//{
//		if (IMURXData == 0X55 )    //检测到包头准备接收11个数据
//		{
//			IMUcounter = 0;   //做好准备接收11个数据
//			IMURXDataBuf[0] = 0X55;
//		}
//		if ( IMURXDataBuf[0] == 0X55 && IMUcounter != 0 )
//		{
//			IMURXDataBuf[IMUcounter] = IMURXData;	
//		}
//		IMUcounter++;
//		if ( IMUcounter == 12 )
//		{
//			IMURXDataBuf[0] = 0;   //清除帧头
//			IMUcounter = 0;
//		}
//}

void IMU_Data_Explain()  //IMU数据解释
{
	if ( RX_Data_flg == 1 ) //接收完成时，可以解释数据包了
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
	//if ( USART_GetITStatus(USART2,USART_IT_RXNE)==SET )  //检测读取寄存器是否有数据
	//{
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
		IMURXData = USART2->DR;
	//}
	if (IMURXData == 0X55 )    //检测到包头准备接收11个数据
	{
		RX_Data_flg = 0;  //接包开始，封锁数据解读
		IMUcounter = 0;   //做好准备接收11个数据
		IMURXDataBuf[0] = 0X55;
	}
	if ( IMURXDataBuf[0] == 0X55 && IMUcounter != 0 )
	{
		IMURXDataBuf[IMUcounter] = IMURXData;	
	}
	IMUcounter++;
	if ( IMUcounter == 11 )
	{
		RX_Data_flg = 1;  //接包完成了，可以解读数据了
		IMURXDataBuf[0] = 0;   //清除帧头
		IMUcounter = 0;
	}
	//USART2->SR = 0;   //清除接收中断标志位
}


void InitIMU()
{
	InitIMUUSART();       //初始化IMU串口为USART2
	InitIMUUSARTGPIO();   //初始化USART2的GPIO口，TX--PA2，RX--PA3
	InitIMUUSART2_NVIC();
}


void InitIMUUSART()   //初始化IMU串口为USART2
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
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);   //使能接收中断							
	USART_Cmd(USART2, ENABLE); 
}

void InitIMUUSARTGPIO( )  //初始化USART2的GPIO口，TX--PA2，RX--PA3
{
	GPIO_InitTypeDef GPIO_InitStructure;					//定义一个GPIO结构体变量

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	
															//使能各个端口时钟，重要！！！

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 				//配置串口接收端口挂接到9端口
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	   		//复用功能输出开漏
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   	//配置端口速度为50M
  	GPIO_Init(GPIOA, &GPIO_InitStructure);				   	//根据参数初始化GPIOA寄存器	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入(复位状态);	   				
  	GPIO_Init(GPIOA, &GPIO_InitStructure);				   	//根据参数初始化GPIOA寄存器
}

void InitIMUUSART2_NVIC()
{
	NVIC_InitTypeDef USART2_NVIC;
	#ifdef  VECT_TAB_RAM  							//向量表基地址选择
	
		NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);  	//将0x20000000地址作为向量表基地址(RAM)
	#else  
	
		NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0); //将0x08000000地址作为向量表基地址(FLASH)  
	#endif
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	USART2_NVIC.NVIC_IRQChannel= USART2_IRQChannel;
	USART2_NVIC.NVIC_IRQChannelPreemptionPriority=1;
	USART2_NVIC.NVIC_IRQChannelSubPriority=1;
	USART2_NVIC.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&USART2_NVIC);
}
