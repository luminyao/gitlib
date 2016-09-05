#include <motorClass.h>
static motorClass *mot[3]; 
static char Motor_message[10] = {0};
static int  Mp;
void (*_TIM_Callback_Samp)(u8 a);

static void _InitMotor(motorParam *motor)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOF | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = motor->ContrAct1_IO_channel; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(motor->ContrAct1_IO_domain, &GPIO_InitStructure);  

	GPIO_InitStructure.GPIO_Pin = motor->ContrAct2_IO_channel; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(motor->ContrAct2_IO_domain, &GPIO_InitStructure);
}


static void _Init_TIMtoPWM(motorParam *PWMTIM)
{
	TIM_OCInitTypeDef OCInitStruct;
	TIM_TimeBaseInitTypeDef TIMtoPWMStruct;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD |RCC_APB2Periph_GPIOF | RCC_APB2Periph_AFIO, ENABLE);
	
	if (PWMTIM->PWMTIMx == TIM2)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	}
	if (PWMTIM->PWMTIMx == TIM3)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	}
	if (PWMTIM->PWMTIMx == TIM4)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	}
	if (PWMTIM->PWMTIMx == TIM5)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);
	}
//	TIM_DeInit(PWMTIM->PWMTIMx);
	TIM_InternalClockConfig(PWMTIM->PWMTIMx);
	TIMtoPWMStruct.TIM_Period=1000-1;
	TIMtoPWMStruct.TIM_Prescaler=0;
	TIMtoPWMStruct.TIM_ClockDivision=PWMTIM->PWMTIM_ClockDivision;
	TIMtoPWMStruct.TIM_CounterMode=PWMTIM->PWMTIM_CounterMode;
	TIM_TimeBaseInit(PWMTIM->PWMTIMx,&TIMtoPWMStruct);
	TIM_ARRPreloadConfig(PWMTIM->PWMTIMx,ENABLE);
	TIM_Cmd(PWMTIM->PWMTIMx,ENABLE);
	
	TIM_OCStructInit(&OCInitStruct);
	OCInitStruct.TIM_OCMode= PWMTIM->PWMTIM_OCMode;//TIM_OCMode_PWM1;
	OCInitStruct.TIM_Pulse=PWMTIM->PwmVoltage;
	OCInitStruct.TIM_OCPolarity=TIM_OCPolarity_High;
	OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	if ((PWMTIM->PWMOCSwitch)&0x0001)
	{
		TIM_OC1Init(PWMTIM->PWMTIMx,&OCInitStruct);
		TIM_OC1PreloadConfig(PWMTIM->PWMTIMx,TIM_OCPreload_Enable);
	}
	if ((PWMTIM->PWMOCSwitch)&0x0010)
	{
		TIM_OC2Init(PWMTIM->PWMTIMx,&OCInitStruct);
		TIM_OC2PreloadConfig(PWMTIM->PWMTIMx,TIM_OCPreload_Enable);
	}
	if ((PWMTIM->PWMOCSwitch)&0x0100)
	{
		TIM_OC3Init(PWMTIM->PWMTIMx,&OCInitStruct);
		TIM_OC3PreloadConfig(PWMTIM->PWMTIMx,TIM_OCPreload_Enable);
	}
	if ((PWMTIM->PWMOCSwitch)&0x1000)
	{
		TIM_OC4Init(PWMTIM->PWMTIMx,&OCInitStruct);
		TIM_OC4PreloadConfig(PWMTIM->PWMTIMx,TIM_OCPreload_Enable);
	}
	TIM_CtrlPWMOutputs(PWMTIM->PWMTIMx,ENABLE);
	
	
	GPIO_InitStructure.GPIO_Pin = PWMTIM->PWM_IO_channel;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PWMTIM->PWM_IO_domain, &GPIO_InitStructure);
	if (PWMTIM->PWMTIM_GPIO_Remap != 0)
	{
		GPIO_PinRemapConfig(PWMTIM->PWMTIM_GPIO_Remap,ENABLE);
	} 
}


static void _Init_TIMtoPulseCount(motorParam *d) 
{ 
	u8 flag = 0;
	GPIO_InitTypeDef GPIO_InitStructure;
//  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOF | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOE, ENABLE);
  if (d->encode_IO_channel_1 != NULL )
	{
		flag++;
		GPIO_InitStructure.GPIO_Pin = d->encode_IO_channel_1;  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(d->encode_IO_domain_1, &GPIO_InitStructure);
	}
	if ( d->encode_IO_channel_2 != NULL )
	{
		flag++;
		GPIO_InitStructure.GPIO_Pin = d->encode_IO_channel_2;  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(d->encode_IO_domain_2, &GPIO_InitStructure);
	}
	if (d->PulseTIMx == TIM1)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = TIM1_TRG_COM_IRQn;
	}
	if (d->PulseTIMx == TIM2)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	}
	if (d->PulseTIMx == TIM3)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	}
	if (d->PulseTIMx == TIM4)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	}
//	if (d->PulseTIMx == TIM5)
//	{
//		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);
//		NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
//	} 
	
   TIM_DeInit(d->PulseTIMx); 
/*****************************************/	
	if (flag == 2)	
	{
		d->PulseTIMx->PSC = 0x0;
		d->PulseTIMx->ARR = 1;
		d->PulseTIMx->CCMR1 |= 1<<0;    
		d->PulseTIMx->CCMR1 |= 1<<8;    
		d->PulseTIMx->CCER |= 0<<1; 
		d->PulseTIMx->CCER |= 0<<0;
		d->PulseTIMx->CCER |= 0<<5;
		d->PulseTIMx->CCMR1 |= 3<<4;	
		d->PulseTIMx->SMCR |= 1<<0; 	
		d->PulseTIMx->CR1 |= 0X01; 
		d->Orthornot = true;
	}else if (flag == 1)
	{	
/*******/		
		d->PulseTIMx->PSC=65530;			
		d->PulseTIMx->ARR=0xffff;
		d->PulseTIMx->CCMR1|=1<<0;  
		d->PulseTIMx->CCMR1|=0<<4;  
		d->PulseTIMx->CCMR1|=0<<10;

		d->PulseTIMx->CCER|=0<<1; 
		d->PulseTIMx->CCER|=1<<0; 

		d->PulseTIMx->DIER|=1<<1; 
		d->PulseTIMx->DIER|=1<<0; 
		d->PulseTIMx->CR1 =0x00; 
		d->Orthornot = false;
	} 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure); 
}




static void _TIM_Callback_Samp2(u8 a)
{
	int i;
	u16 tre;
	
	for (i =0 ;i< Mp;i++)
	{
		if ( Motor_message[i] == a )
		{		
			tre=mot[i]->PulseTIMx->SR; 
			if(tre&0x01) 
			{   
				if(mot[i]->CAPTURE_STA&0X40) 
				{
//					if((mot[i]->CAPTURE_STA&0X3F)==0x3F)  
//					{
//						mot[i]->UnitMileTime=0xffff;
//						mot[i]->CAPTURE_STA|=0X80;   
//					}
//					else 
//					{
//						mot[i]->CAPTURE_STA++;
//					}
//				}		 
			}
			else
			{
				mot[i]->motormile++;
			}
			if(tre&0x02) 
			{
				if((mot[i]->CAPTURE_STA)&(0X40)) 
				{
					 mot[i]->CAPTURE_STA=0;
					 mot[i]->UnitMileTime=mot[i]->PulseTIMx->CNT;
					 mot[i]->CAPTURE_STA|=0X80; 
					 mot[i]->PulseTIMx->CR1&=~(1<<0); 
					 mot[i]->PulseTIMx->CNT=0;
					 mot[i]->PulseTIMx->CCER|=1<<0; 
					 mot[i]->PulseTIMx->DIER|=1<<0; 
					 mot[i]->PulseTIMx->CCER&=~(1<<1); 
					 mot[i]->PulseTIMx->CR1|=0X01; 
				}
				else
				{			
					mot[i]->UnitMileTime=TIM4->CNT; 
					mot[i]->CAPTURE_STA=0;
					mot[i]->PulseTIMx->CR1&=~(1<<0); 
					mot[i]->PulseTIMx->CNT=0; 
					mot[i]->PulseTIMx->DIER|=1<<0; 
					mot[i]->PulseTIMx->CCER|=1<<1; 
					mot[i]->PulseTIMx->CR1|=0X01; 
					mot[i]->CAPTURE_STA|=0X40; 
				}	
			} 
		}		
	}
	mot[i]->PulseTIMx->SR = 0;
 }
}


//static void _TIM_Callback_Samp1(u8 a)
//{
////	int i;
////	u16 tre;
////	for (i =0 ;i< Mp;i++)
////	{
////		if ( Motor_message[i] == a )
////		{
////				
////		}
////	}
//}

static void _waht_(u8 a)
{
	/**空函数**/
}

//static void Select_Interrupt_Handler(motorParam *_motor,u8 mo)
//{

//	if (_motor->ClockTIMx == TIM2)
//	{
//			Motor_message[mo] = 2;
//			TIM_Callback2 = _TIM_Callback_Samp;
//			if (TIM_Callback3 != _TIM_Callback_Samp)
//			{
//				TIM_Callback3 = _waht_;
//			}
//			if (TIM_Callback4 != _TIM_Callback_Samp)
//			{
//				TIM_Callback4 = _waht_;
//			}
//			if (TIM_Callback5 != _TIM_Callback_Samp)
//			{
//				TIM_Callback5 = _waht_;
//			}
//	}
//	if (_motor->ClockTIMx == TIM3)
//	{
//			Motor_message[mo] = 3;
//			TIM_Callback3 = _TIM_Callback_Samp;
//			if (TIM_Callback2 != _TIM_Callback_Samp)
//			{
//				TIM_Callback2 = _waht_;
//			}
//			if (TIM_Callback4 != _TIM_Callback_Samp)
//			{
//				TIM_Callback4 = _waht_;
//			}
//			if (TIM_Callback5 != _TIM_Callback_Samp)
//			{
//				TIM_Callback5 = _waht_;
//			}
//	}
//	if (_motor->ClockTIMx == TIM4)
//	{
//			Motor_message[mo] = 4;
//			TIM_Callback4 = _TIM_Callback_Samp;
//			if (TIM_Callback3 != _TIM_Callback_Samp)
//			{
//				TIM_Callback3 = _waht_;
//			}
//			if (TIM_Callback2 != _TIM_Callback_Samp)
//			{
//				TIM_Callback2 = _waht_;
//			}
//			if (TIM_Callback5 != _TIM_Callback_Samp)
//			{
//				TIM_Callback5 = _waht_;
//			}
//	}
//	if (_motor->ClockTIMx == TIM5)
//	{
//			Motor_message[mo] = 5;
//			TIM_Callback5 = _TIM_Callback_Samp;
//			if (TIM_Callback3 != _TIM_Callback_Samp)
//			{
//				TIM_Callback3 = _waht_;
//			}
//			if (TIM_Callback4 != _TIM_Callback_Samp)
//			{
//				TIM_Callback4 = _waht_;
//			}
//			if (TIM_Callback2 != _TIM_Callback_Samp)
//			{
//				TIM_Callback2 = _waht_;
//			}
//	}
//	_TIM1_IRQn_deal = TIM1_IRQn_deal ;
//	_TIM2_IRQn_deal = TIM2_IRQn_deal ;
//	_TIM3_IRQn_deal = TIM3_IRQn_deal ;
//	_TIM4_IRQn_deal = TIM4_IRQn_deal ;
//	_TIM5_IRQn_deal = TIM5_IRQn_deal ;
//}	

static void Select_Interrupt_Handler(motorParam *_motor,u8 mo)
{
	if ( !_motor->Orthornot )
	{
		_TIM_Callback_Samp = _waht_;
	}else if ( _motor->Orthornot )
	{
		_TIM_Callback_Samp = _TIM_Callback_Samp2;
	}
	if (_motor->PulseTIMx == TIM2)
	{
			Motor_message[mo] = 2;
			TIM_Callback2 = _TIM_Callback_Samp;
			if (TIM_Callback3 != _TIM_Callback_Samp)
			{
				TIM_Callback3 = _waht_;
			}
			if (TIM_Callback4 != _TIM_Callback_Samp)
			{
				TIM_Callback4 = _waht_;
			}
			if (TIM_Callback5 != _TIM_Callback_Samp)
			{
				TIM_Callback5 = _waht_;
			}
	}
	if (_motor->PulseTIMx == TIM3)
	{
			Motor_message[mo] = 3;
			TIM_Callback3 = _TIM_Callback_Samp;
			if (TIM_Callback2 != _TIM_Callback_Samp)
			{
				TIM_Callback2 = _waht_;
			}
			if (TIM_Callback4 != _TIM_Callback_Samp)
			{
				TIM_Callback4 = _waht_;
			}
			if (TIM_Callback5 != _TIM_Callback_Samp)
			{
				TIM_Callback5 = _waht_;
			}
	}
	if (_motor->PulseTIMx == TIM4)
	{
			Motor_message[mo] = 4;
			TIM_Callback4 = _TIM_Callback_Samp;
			if (TIM_Callback3 != _TIM_Callback_Samp)
			{
				TIM_Callback3 = _waht_;
			}
			if (TIM_Callback2 != _TIM_Callback_Samp)
			{
				TIM_Callback2 = _waht_;
			}
			if (TIM_Callback5 != _TIM_Callback_Samp)
			{
				TIM_Callback5 = _waht_;
			}
	}
	if (_motor->PulseTIMx == TIM5)
	{
			Motor_message[mo] = 5;
			TIM_Callback5 = _TIM_Callback_Samp;
			if (TIM_Callback3 != _TIM_Callback_Samp)
			{
				TIM_Callback3 = _waht_;
			}
			if (TIM_Callback4 != _TIM_Callback_Samp)
			{
				TIM_Callback4 = _waht_;
			}
			if (TIM_Callback2 != _TIM_Callback_Samp)
			{
				TIM_Callback2 = _waht_;
			}
	}
	_TIM1_IRQn_deal = TIM1_IRQn_deal ;
	_TIM2_IRQn_deal = TIM2_IRQn_deal ;
	_TIM3_IRQn_deal = TIM3_IRQn_deal ;
	_TIM4_IRQn_deal = TIM4_IRQn_deal ;
	_TIM5_IRQn_deal = TIM5_IRQn_deal ;
}	



motorClass *new_motor(motorParam *_motor)
{
	motorClass *m;
	motorClass *temp;
	static u8 mo = 0;
	m = (motorClass *)malloc(sizeof(motorClass));

	m->ContrAct1_IO_domain = _motor->ContrAct1_IO_domain;
	m->ContrAct1_IO_channel = _motor->ContrAct1_IO_channel;
	m->ContrAct2_IO_domain = _motor->ContrAct2_IO_domain;
	m->ContrAct2_IO_channel = _motor->ContrAct2_IO_channel;

	m->PWMTIMx = _motor->PWMTIMx; 
	m->PWMOCSwitch = _motor->PWMOCSwitch;
	m->PulseTIMx = _motor->PulseTIMx;

	m->Speed_Contor = _Speed_Contor;
	m->motor_MovControl = _motor_MovControl;
	
	_InitMotor(_motor);
	_Init_TIMtoPulseCount(_motor);
	_Init_TIMtoPWM(_motor);
	
	Select_Interrupt_Handler(_motor,mo);
	m->Orthornot = _motor->Orthornot;
	temp = m;
	mot[mo] = (motorClass *)temp;
	mo++;
	Mp = mo;
	free(_motor);
	_motor = NULL;   //6/19改
	return m;
}


void TIM2_IRQHandler(void)
{	
		TIM_Callback2(2);
	_TIM2_IRQn_deal();	

}

void TIM3_IRQHandler(void)
{	
		TIM_Callback3(3);
	_TIM3_IRQn_deal();	

}

void TIM4_IRQHandler(void)
{	
			TIM_Callback4(4);
		_TIM4_IRQn_deal();

}

void TIM5_IRQHandler(void)
{	
			TIM_Callback5(5);
		_TIM5_IRQn_deal();

}

void TIM1_TRG_COM_IRQHandler(void) //中断，清中断标志
{
	 _TIM1_IRQn_deal(); 			
}


motorClass * BuildMotor(
	GPIO_TypeDef * ContrAct1_IO_domain,    //控制引脚，方向控制引脚--GPIOx
	u16 ContrAct1_IO_channel,							 //控制引脚，方向控制引脚--GPIO_Pin_x

	GPIO_TypeDef * ContrAct2_IO_domain,		 //控制引脚，刹车控制引脚--GPIOx
	u16 ContrAct2_IO_channel,							 //控制引脚，刹车控制引脚--GPIO_Pin_x

	TIM_TypeDef * TIM2PWM,					//PWM产生时钟

	GPIO_TypeDef * PWM_IO_domain,   //PWM输出引脚--GPIOx
	u16 PWM_IO_Channel,							//PWM输出引脚--GPIO_Pin_x
	u16 PWMOCSwitch,								//PWM输出引脚对应的通道
	u32 PWMTIM_GPIO_Remap,					//是否需要映射

	TIM_TypeDef * TIM2Samp,					//采样时钟，包括速度采样和里程记录
	GPIO_TypeDef * encode_IO_domain_1,  //脉冲输入引脚--GPIOx
	u16  encode_IO_channel_1,						//脉冲输入引脚--GPIO_Pin_x

	GPIO_TypeDef * encode_IO_domain_2,  //脉冲输入引脚--GPIOx,不用则写NULL
	u16  encode_IO_channel_2						//脉冲输入引脚--GPIO_Pin_x，不用则写NULL
)
{
	motorParam motorStruct;
	MOTORPARAM(motorStruct);  //引入默认参数
  motorStruct.ContrAct1_IO_domain = ContrAct1_IO_domain;
	motorStruct.ContrAct1_IO_channel = ContrAct1_IO_channel;
	motorStruct.ContrAct2_IO_domain = ContrAct2_IO_domain;
	motorStruct.ContrAct2_IO_channel = ContrAct2_IO_channel;
	/****************************************/
	/*时钟配置参数*/
//	motorStruct.ClockTIMx = TIM5;
	/****************************************/
	/*pwm配置参数*/
	motorStruct.PWMTIMx = TIM2PWM; 
	motorStruct.PWMOCSwitch = PWMOCSwitch;
	motorStruct.PWM_IO_domain = PWM_IO_domain;
	motorStruct.PWM_IO_channel = PWM_IO_Channel; 
	motorStruct.PWMTIM_GPIO_Remap = PWMTIM_GPIO_Remap;//GPIO_Remap_TIM4;
	/****************************************/
	/*脉冲计数参数配置*/
	motorStruct.PulseTIMx = TIM2Samp;
	motorStruct.encode_IO_domain_1 = encode_IO_domain_1;
	motorStruct.encode_IO_channel_1 = encode_IO_channel_1;
	
	motorStruct.encode_IO_domain_2 = encode_IO_domain_2;
	motorStruct.encode_IO_channel_2 = encode_IO_channel_2;
	/****************************************/
	return new_motor(&motorStruct);
}

MovBaseClass * BuildMovBase(motorClass * m1,motorClass * m2,float WH)
{
	MovBaseParam MovBase;
	MovBase.m[0] = m1;
	MovBase.m[1] = m2;
	MovBase.WheelBetw = WH;
	return new_MovBase(&MovBase);
}






