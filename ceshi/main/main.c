#include "sys.h"
#define USART1_REC_LEN	100                     /* 定义最大接收字节数 100 */
#include "usart2.h"
#include "string.h"
#include  <stdarg.h>
//SETPara set;
motorClass *motor[3];
motorParam SetParam;
int um[4] = {1,1,1,1};
static int motornum = 0;
int16_t tragetspeed;

void buildMotor(uint8_t *Message);
//板间串口通信使用USART1，测试板和PC通信使用USART2,主从蓝牙间的传输波特率为38400


#define ENMOTORTEST 0

void init_led()
{
	GPIO_InitTypeDef led;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOA,ENABLE);

	
	led.GPIO_Pin=GPIO_Pin_2; 
	led.GPIO_Speed=GPIO_Speed_50MHz;
	led.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA,&led);
}


void RCC_Configuration() 
{           
	RCC_DeInit();      
	RCC_HSEConfig(RCC_HSE_ON);         
	if(RCC_WaitForHSEStartUp()==SUCCESS)  
	{  
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);       
		FLASH_SetLatency(FLASH_Latency_2);        
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		RCC_PCLK2Config(RCC_HCLK_Div1);         
		RCC_PCLK1Config(RCC_HCLK_Div2);    
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);    
		RCC_PLLCmd(ENABLE);   
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY==RESET)) {}  
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);  
		while(RCC_GetSYSCLKSource()!=0x08){}       
	} 
}




void motor_test(motorParam *Setmotor)
{
	static u8 mo;
	motorParam motorStruct;
	MOTORPARAM(motorStruct);  //引入默认参数,将其他配置参设置为缺省值	
 
  motorStruct.ContrAct1_IO_domain = Setmotor->ContrAct1_IO_domain;
	motorStruct.ContrAct1_IO_channel = Setmotor->ContrAct1_IO_channel;
	motorStruct.ContrAct2_IO_domain = Setmotor->ContrAct2_IO_domain;
	motorStruct.ContrAct2_IO_channel = Setmotor->ContrAct2_IO_channel;
	/****************************************/
	/****************************************/
	/*pwm配置参数*/
	motorStruct.PWMTIMx = Setmotor->PWMTIMx;//TIM3; 
	motorStruct.PWMOCSwitch = Setmotor->PWMOCSwitch;
	motorStruct.PWM_IO_domain = Setmotor->PWM_IO_domain;//GPIOB;
	motorStruct.PWM_IO_channel = Setmotor->PWM_IO_channel;//GPIO_Pin_1; 
	motorStruct.PWMTIM_GPIO_Remap = 0;//Setmotor->PWMTIM_GPIO_Remap;//GPIO_Remap_TIM4;
	/****************************************/
	/*脉冲计数参数配置*/
	motorStruct.PulseTIMx = Setmotor->PulseTIMx;//TIM4;
	motorStruct.encode_IO_domain_1 = Setmotor->encode_IO_domain_1;//GPIOE;
	motorStruct.encode_IO_channel_1 = Setmotor->encode_IO_channel_1;//GPIO_Pin_0;
	
	motorStruct.encode_IO_domain_2 = 	NULL;//Setmotor->encode_IO_domain_2;
	motorStruct.encode_IO_channel_2 = NULL;//Setmotor->encode_IO_channel_2;
	/****************************************/
	motor[mo] = new_motor(&motorStruct);
	mo++;
	motornum++;
}

void delet_motor(motorParam *motor)
{
//	TIM_DeInit(motor->ClockTIMx);
	TIM_DeInit(motor->PulseTIMx);
	TIM_DeInit(motor->PWMTIMx);
	motor = NULL;
}

float X,Y,theta;
uint8_t DisposeCmd(uint8_t *RecieveText, uint8_t Length)
{
	uint8_t CheckSum=0, Message[128], i=0;
	if(RecieveText[0] != 0xAA && RecieveText[1] != 0xAA) return 0;
	if(RecieveText[Length-2] != 0x55 && RecieveText[Length-1] != 0x55) return 0;	
  
	RecieveText += 2;
	for(Length -= 5;Length>0;Length--)																				//去除帧头、控制符的数据存储在Message[]中
	{
		if(*RecieveText != 0xA5)
		{
			Message[i++] = *RecieveText;
			CheckSum += *(RecieveText++);
		}
		else
		{
			if(Length == 1)	return 0;
			
			Message[i++] = *(++RecieveText);
			CheckSum += *(RecieveText++);
			Length--;
		}
	}
	if ( Message[3] == 0x02 )
	{
		X = (int16_t) (Message[5] | Message[6]<<8);
		Y = (int16_t) (Message[7] |Message[8]<<8);
		theta = (int16_t) Message[9] | Message[10]<<8; 
	}
	if ( Message[3] == 0x24 )
	{
		buildMotor(Message);
		
		switch (Message[19])
		{
			case 1:if (um[0] == 1) {um[0] = 0; motor_test(&SetParam);}
			break;
			case 2:if (um[1] == 1) {um[1] = 0; motor_test(&SetParam);}
			break;
		  case 3:if (um[2] == 1) {um[2] = 0; motor_test(&SetParam);}
			break;
			case 4:if (um[3] == 1) {um[3] = 0; motor_test(&SetParam);}
			break;
		}
		//printf("2\r\n");
	}
	if ( Message[3] == 0x25 )
	{
		motor[Message[15]]->trSpeed = (int16_t) (Message[13] | Message[14]<<8);
		motor[Message[15]]->Kp = (float) (Message[5] | Message[6]<<8);
		motor[Message[15]]->Ki = (float) (Message[7] | Message[8]<<8);
		motor[Message[15]]->Wheel_pulse_circle = (int16_t)(Message[16] | Message[17]<<8);;
		motor[Message[15]]->Wheel_cir = (int16_t)(Message[18] | Message[19]<<8);
//		printf("M = %d",Message[15]);
	}
	return 1;
}

void buildMotor(uint8_t *Message)
{
	switch (Message[7])
	{
		case 0:
			SetParam.PulseTIMx = TIM1;
		  break;
		case 1:
			SetParam.PulseTIMx = TIM2;
		  break;
		case 2:
			SetParam.PulseTIMx = TIM3;
		  break;
		case 3:
			SetParam.PulseTIMx = TIM4;
		  break;
		case 4:
			SetParam.PulseTIMx = TIM5;
		  break;		
	}
	switch (Message[8])
	{
		case 0:
			SetParam.PWMTIMx = TIM1;
		  break;
		case 1:
			SetParam.PWMTIMx = TIM2;
		  break;
		case 2:
			SetParam.PWMTIMx = TIM3;
		  break;
		case 3:
			SetParam.PWMTIMx = TIM4;
		  break;
		case 4:
			SetParam.PWMTIMx = TIM5;
		  break;		
	}
	switch (Message[9])
	{
		case 0:
			SetParam.ContrAct1_IO_domain = GPIOA;
		  break;
		case 1:
			SetParam.ContrAct1_IO_domain = GPIOB;
		  break;
		case 2:
			SetParam.ContrAct1_IO_domain = GPIOC;
		  break;
		case 3:
			SetParam.ContrAct1_IO_domain = GPIOD;
		  break;
		case 4:
			SetParam.ContrAct1_IO_domain = GPIOE;
		  break;	
		case 5:
			SetParam.ContrAct1_IO_domain = GPIOF;
		  break;		
	}
	switch (Message[10])
	{
		case 0:
			SetParam.ContrAct2_IO_domain = GPIOA;
		  break;
		case 1:
			SetParam.ContrAct2_IO_domain = GPIOB;
		  break;
		case 2:
			SetParam.ContrAct2_IO_domain = GPIOC;
		  break;
		case 3:
			SetParam.ContrAct2_IO_domain = GPIOD;
		  break;
		case 4:
			SetParam.ContrAct2_IO_domain = GPIOE;
		  break;	
		case 5:
			SetParam.ContrAct2_IO_domain = GPIOF;
		  break;		
	}
	switch (Message[11])
	{
		case 0:
			SetParam.encode_IO_domain_1 = GPIOA;
		  break;
		case 1:
			SetParam.encode_IO_domain_1 = GPIOB;
		  break;
		case 2:
			SetParam.encode_IO_domain_1 = GPIOC;
		  break;
		case 3:
			SetParam.encode_IO_domain_1 = GPIOD;
		  break;
		case 4:
			SetParam.encode_IO_domain_1 = GPIOE;
		  break;		
		case 5:
			SetParam.encode_IO_domain_1 = GPIOF;
		  break;
	}
	switch (Message[12])
	{
		case 0:
			SetParam.encode_IO_domain_2 = GPIOA;
		  break;
		case 1:
			SetParam.encode_IO_domain_2 = GPIOB;
		  break;
		case 2:
			SetParam.encode_IO_domain_2 = GPIOC;
		  break;
		case 3:
			SetParam.encode_IO_domain_2 = GPIOD;
		  break;
		case 4:
			SetParam.encode_IO_domain_2 = GPIOE;
		  break;
		case 5:
			SetParam.encode_IO_domain_2 = GPIOF;
		  break;		
	}
	switch (Message[13])
	{
		case 0:
			SetParam.PWM_IO_domain = GPIOA;
		  break;
		case 1:
			SetParam.PWM_IO_domain = GPIOB;
		  break;
		case 2:
			SetParam.PWM_IO_domain = GPIOC;
		  break;
		case 3:
			SetParam.PWM_IO_domain = GPIOD;
		  break;
		case 4:
			SetParam.PWM_IO_domain = GPIOE;
		  break;
		case 5:
			SetParam.PWM_IO_domain = GPIOF;
		  break;		
	}
  switch (Message[14])
	{
		case 0:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_0;
		  break;
		case 1:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_1;
		  break;
		case 2:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_2;
		  break;
		case 3:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_3;
		  break;
		case 4:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_4;
		  break;
		case 5:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_5;
		  break;	
		case 6:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_6;
		  break;
		case 7:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_7;
		  break;
		case 8:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_8;
		  break;
		case 9:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_9;
		  break;
		case 10:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_10;
		  break;
		case 11:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_11;
		  break;
		case 12:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_12;
		  break;
		case 13:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_13;
		  break;
		case 14:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_14;
		  break;
		case 15:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_15;
		  break;	
		case 16:
			SetParam.ContrAct1_IO_channel = GPIO_Pin_All;
		  break;		
	}
	switch (Message[16])
	{
		case 0:
			SetParam.encode_IO_channel_1 = GPIO_Pin_0;
		  break;
		case 1:
			SetParam.encode_IO_channel_1 = GPIO_Pin_1;
		  break;
		case 2:
			SetParam.encode_IO_channel_1 = GPIO_Pin_2;
		  break;
		case 3:
			SetParam.encode_IO_channel_1 = GPIO_Pin_3;
		  break;
		case 4:
			SetParam.encode_IO_channel_1 = GPIO_Pin_4;
		  break;
		case 5:
			SetParam.encode_IO_channel_1 = GPIO_Pin_5;
		  break;	
		case 6:
			SetParam.encode_IO_channel_1 = GPIO_Pin_6;
		  break;
		case 7:
			SetParam.encode_IO_channel_1 = GPIO_Pin_7;
		  break;
		case 8:
			SetParam.encode_IO_channel_1 = GPIO_Pin_8;
		  break;
		case 9:
			SetParam.encode_IO_channel_1 = GPIO_Pin_9;
		  break;
		case 10:
			SetParam.encode_IO_channel_1 = GPIO_Pin_10;
		  break;
		case 11:
			SetParam.encode_IO_channel_1 = GPIO_Pin_11;
		  break;
		case 12:
			SetParam.encode_IO_channel_1 = GPIO_Pin_12;
		  break;
		case 13:
			SetParam.encode_IO_channel_1 = GPIO_Pin_13;
		  break;
		case 14:
			SetParam.encode_IO_channel_1 = GPIO_Pin_14;
		  break;
		case 15:
			SetParam.encode_IO_channel_1 = GPIO_Pin_15;
		  break;	
		case 16:
			SetParam.encode_IO_channel_1 = GPIO_Pin_All;
		  break;		
	}
	switch (Message[17])
	{
		case 0:
			SetParam.encode_IO_channel_2 = GPIO_Pin_0;
		  break;
		case 1:
			SetParam.encode_IO_channel_2 = GPIO_Pin_1;
		  break;
		case 2:
			SetParam.encode_IO_channel_2 = GPIO_Pin_2;
		  break;
		case 3:
			SetParam.encode_IO_channel_2 = GPIO_Pin_3;
		  break;
		case 4:
			SetParam.encode_IO_channel_2 = GPIO_Pin_4;
		  break;
		case 5:
			SetParam.encode_IO_channel_2 = GPIO_Pin_5;
		  break;	
		case 6:
			SetParam.encode_IO_channel_2 = GPIO_Pin_6;
		  break;
		case 7:
			SetParam.encode_IO_channel_2 = GPIO_Pin_7;
		  break;
		case 8:
			SetParam.encode_IO_channel_2 = GPIO_Pin_8;
		  break;
		case 9:
			SetParam.encode_IO_channel_2 = GPIO_Pin_9;
		  break;
		case 10:
			SetParam.encode_IO_channel_2 = GPIO_Pin_10;
		  break;
		case 11:
			SetParam.encode_IO_channel_2 = GPIO_Pin_11;
		  break;
		case 12:
			SetParam.encode_IO_channel_2 = GPIO_Pin_12;
		  break;
		case 13:
			SetParam.encode_IO_channel_2 = GPIO_Pin_13;
		  break;
		case 14:
			SetParam.encode_IO_channel_2 = GPIO_Pin_14;
		  break;
		case 15:
			SetParam.encode_IO_channel_2 = GPIO_Pin_15;
		  break;	
		case 16:
			SetParam.encode_IO_channel_2 = GPIO_Pin_All;
		  break;		
	}
	switch (Message[18])
	{
		case 0:
			SetParam.PWM_IO_channel = GPIO_Pin_0;
		  break;		
		case 1:
			SetParam.PWM_IO_channel = GPIO_Pin_1;
		  break;
		case 2:
			SetParam.PWM_IO_channel = GPIO_Pin_2;
		  break;
		case 3:
			SetParam.PWM_IO_channel = GPIO_Pin_3;
		  break;
		case 4:
			SetParam.PWM_IO_channel = GPIO_Pin_4;
		  break;
		case 5:
			SetParam.PWM_IO_channel = GPIO_Pin_5;
		  break;	
		case 6:
			SetParam.PWM_IO_channel = GPIO_Pin_6;
		  break;
		case 7:
			SetParam.PWM_IO_channel = GPIO_Pin_7;
		  break;
		case 8:
			SetParam.PWM_IO_channel = GPIO_Pin_8;
		  break;
		case 9:
			SetParam.PWM_IO_channel = GPIO_Pin_9;
		  break;
		case 10:
			SetParam.PWM_IO_channel = GPIO_Pin_10;
		  break;
		case 11:
			SetParam.PWM_IO_channel = GPIO_Pin_11;
		  break;
		case 12:
			SetParam.PWM_IO_channel = GPIO_Pin_12;
		  break;
		case 13:
			SetParam.PWM_IO_channel = GPIO_Pin_13;
		  break;
		case 14:
			SetParam.PWM_IO_channel = GPIO_Pin_14;
		  break;
		case 15:
			SetParam.PWM_IO_channel = GPIO_Pin_15;
		  break;	
		case 16:
			SetParam.PWM_IO_channel = GPIO_Pin_All;
		  break;		
	}
	switch (Message[20])
	{
		case 1:	
			SetParam.PWMOCSwitch = 0x1000;
			break;
		case 2:	
			SetParam.PWMOCSwitch = 0x0100;
			break;		
		case 3:	
			SetParam.PWMOCSwitch = 0x0010;
			break;
		case 4:	
			SetParam.PWMOCSwitch = 0x0001;
			break;			
	}
}


//void motor_test1()
//{
//	motorParam motorStruct;
//	MOTORPARAM(motorStruct);  //引入默认参数,将其他配置参设置为缺省值	
//	CONTRACT_IO_INIT_1(motorStruct,F,2);   
//	CONTRACT_IO_INIT_2(motorStruct,F,3);
//	/****************************************/
//	/*时钟配置参数*/
////	motorStruct.ClockTIMx = TIM5;
//	/****************************************/
//	/*pwm配置参数*/
//	motorStruct.PWMTIMx = TIM3; 
//	motorStruct.PWMOCSwitch = 0x1000;
//	motorStruct.PWM_IO_domain = GPIOB;
//	motorStruct.PWM_IO_channel = GPIO_Pin_1; 
//	motorStruct.PWMTIM_GPIO_Remap = 0;//GPIO_Remap_TIM4;
//	/****************************************/
//	/*脉冲计数参数配置*/
//	motorStruct.PulseTIMx = TIM4;
//	motorStruct.encode_IO_domain_1 = GPIOB;
//	motorStruct.encode_IO_channel_1 = GPIO_Pin_6;
//	/****************************************/
//	motor[0] = new_motor(&motorStruct);
//}

//void motor_test2()
//{
//	motorParam motorStruct;
//	MOTORPARAM(motorStruct);  //引入默认参数
//	CONTRACT_IO_INIT_1(motorStruct,F,4);   
//	CONTRACT_IO_INIT_2(motorStruct,F,5);
//	/****************************************/
//	/*时钟配置参数*/
////	motorStruct.ClockTIMx = TIM5;
//	/****************************************/
//	/*pwm配置参数*/
//	motorStruct.PWMTIMx = TIM3; 
//	motorStruct.PWMOCSwitch = 0x0100;
//	motorStruct.PWM_IO_domain = GPIOB;
//	motorStruct.PWM_IO_channel = GPIO_Pin_0; 
//	motorStruct.PWMTIM_GPIO_Remap = 0;//GPIO_Remap_TIM4;
//	/****************************************/
//	/*脉冲计数参数配置*/
//	motorStruct.PulseTIMx = TIM2;
//	motorStruct.encode_IO_domain_1 = GPIOA;
//	motorStruct.encode_IO_channel_1 = GPIO_Pin_0;
//	/****************************************/
//	motor[1] = new_motor(&motorStruct);
//}


void WritePackage(uint8_t *p,u8 l)
{
	uint16_t i, len=l+5;
	u8 *pBuf, text[128];
	uint8_t checksum = 0;

	pBuf = &text[0];
	*pBuf++ = 0XAA;
	*pBuf++ = 0XAA;
	
	for (i = 0; i < l; i++)
	{
		if ((p[i] == 0xA5) || (p[i] == 0XAA) || (p[i] == 0X55))
		{
			*pBuf++ = 0xA5;
			len++;
		}
		*pBuf++ = p[i];
		checksum += p[i];
	}
	
	if ((checksum == 0xA5) || (checksum == 0XAA) || (checksum == 0X55))
	{
		*pBuf++ = 0xA5;
		len++;
	}
	
	*pBuf++ = checksum;

	*pBuf++ = 0X55;
	*pBuf++ = 0X55;

	uart2_SendString(text,len);
	
}

int16_t Speed;

void send_concom(void)
{
	uint8_t text[20];
	float m ,value;
	char i, array[4];
	float* pf;
//	uint32_t* p;
//	p = (uint32_t *)text1;
	m = motor[0]->fback_R_speed;
				
	pf = &m;
	for ( i = 4;i > 0;i--)
	{
		array[i-1] = *((char*)pf + ( i - 1 ));
	}
	value = *((float*)array);
//	Speed = (int16_t)(motor[0]->fback_R_speed * 1000);
	text[0] = 0x06;
	text[1] = 0xff;
	text[2] = 0x00;
	text[3] = 0x01;
	text[4] = 0x00;
	/******数据区********/
	text[5] = array[0];
	text[6] = array[1];
	text[7] = array[2];
	text[8] = array[3];
	printf("%f \r\n",*((float*)array));
//	printf("%f \r\n",motor[0]->fback_R_speed);
//	WritePackage(text,7);
}


int m;
MovBaseClass *MO;
int main()
{	
	delay_init();
	init_led();
	uart_init(115200);
	uart2_init(115200);
	motor[0] = BuildMotor(GPIO(F,2),GPIO(F,3),TIM3,GPIO(B,1),0x1000,0,TIM4,GPIO(B,6),NULL,NULL);
//	motor[1]->Wheel_pulse_circle = 60;
//	motor[1]->Wheel_cir = 471;
//	motor[0] = BuildMotor(GPIO(F,4),GPIO(F,5),TIM3,GPIO(B,0),0x0100,0,TIM2,GPIO(A,0),NULL,NULL);
	motor[0]->Wheel_pulse_circle = 60;
	motor[0]->Wheel_cir = 471;
//	motor[0]->Kp =50;
//	motor[0]->Ki =2;
//	motor[0]->Kd =0;
//	motor[1]->Kp =50;
//	motor[1]->Ki =2;	
//	motor[1]->Kd =0;
	while (1)
	{
		u8 *p;
		u8 i;
		p	= USART2_RX_BUF;	
		
		DisposeCmd(p, 26);

//		for (i = 0;i < motornum;i++)
//		{
//			MO->MovBaseContr(MO,0.2,0);
//			motor[i]->motor_MovControl(motor[i],0,0);
//			motor[i]->Speed_Contor(motor[i],(float)motor[i]->trSpeed/10,motor[i]->Kp,motor[i]->Ki,motor[i]->Kd);
//		}
		
			motor[0]->motor_MovControl(motor[0],0,0);
			motor[0]->Speed_Contor(motor[0],0.5,50,2,0);
//		motor[0]->motor_MovControl(motor[0],0,0);
//		motor[0]->Speed_Contor(motor[0],(float)motor[0]->trSpeed/10,motor[0]->Kp,motor[0]->Ki,motor[0]->Kd);
//		motor[1]->motor_MovControl(motor[1],0,0);
//		motor[1]->Speed_Contor(motor[1],(float)motor[1]->trSpeed/10,motor[1]->Kp,motor[1]->Ki,motor[1]->Kd);
//		MO->MovBaseContr(MO,0.2,0);
			delay_ms(20);
//		printf("Speed1 = %f , mile1 = %d , Speed2 = %f , mile2 = %d \r\n",motor[0]->fback_R_speed,motor[0]->motormile,motor[1]->fback_R_speed,motor[1]->motormile);   //反馈速度单位，m/s
//		printf("Speed1 = %f  \r\n",(motor[0]->fback_R_speed - motor[1]->fback_R_speed));   //反馈速度单位，m/s
//		printf("trgSpeed = %f , Kp = %f\r\n",(float)motor[0]->trSpeed/10,motor[0]->Kp);
//			printf("%f\r\n",motor[0]->fback_R_speed);
			send_concom();		
	}
}






