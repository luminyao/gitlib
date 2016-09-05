#include "stm32f10x.h" 
#include <stdio.h>
#include <stdlib.h>

#ifndef __IRQ__H
#define __IRQ__H
void TIM5_IRQn_deal(void);
void TIM4_IRQn_deal(void);
void TIM3_IRQn_deal(void);
void TIM2_IRQn_deal(void);
void TIM1_IRQn_deal(void);
#endif

#define DIRECT_IO_INIT_1(Gtype,domain,channel)\
         	Gtype.direct1_IO_domain = GPIO##domain;\
	Gtype.direct1_IO_channel = GPIO_Pin_##channel;

#define DIRECT_IO_INIT_2(Gtype,domain,channel)\
         	Gtype.direct2_IO_domain = GPIO##domain;\
	Gtype.direct2_IO_channel = GPIO_Pin_##channel;

#define BRAVE_IO_INIT(Gtype,domain,channel)\
         	Gtype.brave_IO_domain = GPIO##domain;\
	Gtype.brave_IO_channel = GPIO_Pin_##channel;

#define	MOTORPARAM(motorStruct) \
	motorStruct.PwmVoltage =0; \
	motorStruct.PWMTIM_OCPolarity = TIM_OCPolarity_High; \
	motorStruct.PWMTIM_ClockDivision =  TIM_CKD_DIV1;	 \
	motorStruct.ClockTIMarr = 7200; \
	motorStruct.ClockTIMpsc = 71; \
	motorStruct.PWMarr = 1000-1; \
	motorStruct.PWMpsc = 0; \
	GPIO_DeInit(GPIOA); \
	GPIO_DeInit(GPIOB); \
	GPIO_DeInit(GPIOC); \
	GPIO_DeInit(GPIOD); \
	GPIO_DeInit(GPIOE); \
	motorStruct.PWMTIM_OCMode = TIM_OCMode_PWM1; \
	motorStruct.PWMTIM_CounterMode = TIM_CounterMode_Up


typedef struct _motorClass motorClass;
typedef struct _MotorBrush_ Brush_motor;
typedef struct _MotorBrushless_  Brushless_motor;
typedef struct  _motorParam  motorParam;

motorClass  *new_brush_motor(motorParam *_motor);
motorClass  *new2_brush_motor(motorParam *_motor);
static void _Speedsamp(motorClass *motor,u16 samptime);
static void _Speed_Contor(motorClass *motor,u16 target_R_speed,float Kp, float Pi,float Kd);

static void _Brushless_motorReverse(motorClass* d,u8 dir);
static void _Brushless_motor_ResetBrave(motorClass *d);
static void _Brushless_motor_brake(motorClass *d);

static void _Brush_motor_Reversal(motorClass *motor);
static void _Brush_motor_Foreward(motorClass *motor);
static void _Brush_motor_Brave(motorClass *motor);


static void (*TIM_Callback2)(u8 a);
static void (*TIM_Callback3)(u8 a);
static void (*TIM_Callback4)(u8 a);
static void (*TIM_Callback5)(u8 a);

static void (*_TIM1_IRQn_deal)(void);
static void (*_TIM2_IRQn_deal)(void);
static void (*_TIM3_IRQn_deal)(void);
static void (*_TIM4_IRQn_deal)(void);
static void (*_TIM5_IRQn_deal)(void);

struct _motorClass
{
	GPIO_TypeDef * direct1_IO_domain;
	u16 direct1_IO_channel;

	GPIO_TypeDef * direct2_IO_domain;
	u16 direct2_IO_channel;

	GPIO_TypeDef * brave_IO_domain;
	u16 brave_IO_channel; 
		
	u16 PWMOCSwitch;    
	u16 motortime;			
	u8 	dir; 			
	float PiSum;			
	float Kdago;			
	
	u16 fback_R_speed;
	
	TIM_TypeDef *PWMTIMx;
	TIM_TypeDef *PulseTIMx;
	
	void (*Speedsamp)(motorClass *d,u16 samptime);
	void (*Init_TIMtoPWM)(motorParam *PWM);
	void (*Init_PWMGPIO)(u32 GPIO_Remap,GPIO_TypeDef * PWM_IO_domain,u16 PWM_IO_channel);
	void (*Init_TIMtoClock)(motorParam *TIM);
	void (*Init_TIMtoPulseCount)(motorParam *d);
	void (*InitMotor)(motorParam *d);	
	
	void (*Speed_Contor)(motorClass *d,u16 target_R_speed,float Kp, float Pi,float Kd);
	void (*Brush_motor_Reversal)(motorClass *d);
	void (*Brush_motor_Brake)(motorClass *d);	
	void (*Brush_motor_Foreward)(motorClass *d);

	void (*Brushless_motor_brake)(motorClass *d);
	void (*Brushless_motor_Reverse)(motorClass *d,u8 dir);
	void (*Brushless_motor_ResetBrave)(motorClass *d); 
};


struct  _motorParam
{
	GPIO_TypeDef * direct1_IO_domain;
	u16 direct1_IO_channel;

	GPIO_TypeDef * direct2_IO_domain;
	u16 direct2_IO_channel;

	GPIO_TypeDef * brave_IO_domain;
	u16 brave_IO_channel;  

	GPIO_TypeDef * encode_IO_domain_1;  
	u16  encode_IO_channel_1;

	GPIO_TypeDef * encode_IO_domain_2;  
	u16  encode_IO_channel_2;
	
//	GPIO_TypeDef * Encoder_IO_domain_1;
//	u16 Encoder_IO_channel_1; 
//	
//	GPIO_TypeDef * Encoder_IO_domain_2;
//	u16 Encoder_IO_channel_2; 
   
	u16 motortime;			

	u8 	dir; 			
	float PiSum;			
	float Kdago;			
	float target_R_speed;		
	u16 fback_R_speed;	
			
/*******************PWM***********************************/	
	TIM_TypeDef *PWMTIMx;
	GPIO_TypeDef * PWM_IO_domain;
	u16 PWM_IO_channel;
	
	u16 PwmVoltage;
	u16 PWMTIM_OCMode;
	u16 PWMTIM_Channel;
	u16 PWMTIM_Pulse;
	u16 PWMTIM_OCPolarity;
	u16 PWMOCSwitch;	
	u16 PWMarr;
	u16 PWMpsc;
	u8  PWMTIM_ClockDivision;
	u16 PWMTIM_CounterMode;
	u32 PWMTIM_GPIO_Remap;
/******************************************************/	
	TIM_TypeDef *PulseTIMx;
	GPIO_TypeDef * PulseTIM_IO_domain;
	u16 PulseTIM_IO_channel;
/*****************************************************/	
	TIM_TypeDef *ClockTIMx;
	u16 ClockTIMarr;
	u16 ClockTIMpsc; 
};	


static void _InitMotor(motorParam *motor)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOF | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = motor->direct1_IO_channel; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(motor->direct1_IO_domain, &GPIO_InitStructure);  

	GPIO_InitStructure.GPIO_Pin = motor->direct2_IO_channel; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(motor->direct2_IO_domain, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = motor->brave_IO_channel; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(motor->brave_IO_domain, &GPIO_InitStructure);
}

static void _Init_TIMtoClock(motorParam *ClockTIM)
{
	TIM_TimeBaseInitTypeDef TIM2ClockStruct;
	NVIC_InitTypeDef TIM2Clock_NVICStruct;
	if (ClockTIM->ClockTIMx == TIM2)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
		TIM2Clock_NVICStruct.NVIC_IRQChannel = TIM2_IRQn;
	}
	if (ClockTIM->ClockTIMx == TIM3)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
		TIM2Clock_NVICStruct.NVIC_IRQChannel = TIM3_IRQn;
	}
	if (ClockTIM->ClockTIMx == TIM4)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
		TIM2Clock_NVICStruct.NVIC_IRQChannel = TIM4_IRQn;
	}
	if (ClockTIM->ClockTIMx == TIM5)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);
		TIM2Clock_NVICStruct.NVIC_IRQChannel = TIM5_IRQn;
	} 

	TIM_DeInit(ClockTIM->ClockTIMx);
	TIM2ClockStruct.TIM_Period=ClockTIM->ClockTIMarr;   
	TIM2ClockStruct.TIM_Prescaler=ClockTIM->ClockTIMpsc;
	TIM2ClockStruct.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM2ClockStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInit(ClockTIM->ClockTIMx,&TIM2ClockStruct);
	TIM_ClearFlag(ClockTIM->ClockTIMx,TIM_FLAG_Update);
	TIM_ARRPreloadConfig(ClockTIM->ClockTIMx, DISABLE);
	TIM_ITConfig(ClockTIM->ClockTIMx,TIM_IT_Update,ENABLE);
	TIM_Cmd(ClockTIM->ClockTIMx,ENABLE);

	//TIM2Clock_NVICStruct.NVIC_IRQChannel=TIM->TIM_IRQn;
	TIM2Clock_NVICStruct.NVIC_IRQChannelPreemptionPriority=1;
	TIM2Clock_NVICStruct.NVIC_IRQChannelSubPriority=2;
	TIM2Clock_NVICStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&TIM2Clock_NVICStruct);
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
	//TIM_DeInit(PWMTIM->PWMTIMx);
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
	GPIO_InitTypeDef GPIO_InitStructure;
	//TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOF | RCC_APB2Periph_AFIO, ENABLE);
  if (d->PulseTIM_IO_channel != NULL)
	{
		GPIO_InitStructure.GPIO_Pin = d->PulseTIM_IO_channel;  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(d->PulseTIM_IO_domain, &GPIO_InitStructure);
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
	if (d->PulseTIMx == TIM5)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	} 
	
//	TIM_DeInit(d->TIMx); 
//	TIM_TimeBaseStructure.TIM_Period = 1; 
//	TIM_TimeBaseStructure.TIM_Prescaler = 0xffff; 
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0; 
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
//	TIM_TimeBaseInit(d->TIMx, &TIM_TimeBaseStructure); // Time base configuration 
//	TIM_ETRClockMode2Config(d->TIMx, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0); 
//	TIM_SetCounter(d->TIMx, 0); 
//	TIM_ClearITPendingBit(d->TIMx, TIM_IT_Update); 
//	TIM_ITConfig(d->TIMx,TIM_IT_Update, ENABLE); 
//	TIM_Cmd(d->TIMx, ENABLE);
	d->PulseTIMx->PSC = 0; 
	d->PulseTIMx->ARR = 0xffff; 

	d->PulseTIMx->SMCR |= 1<<15; 
	d->PulseTIMx->SMCR |= 0<<14;
	d->PulseTIMx->SMCR |= 0<<12;
  d->PulseTIMx->SMCR |= 0<<8; 
	d->PulseTIMx->SMCR |= 7<<4; 

	d->PulseTIMx->SMCR |= 7<<0; 
	d->PulseTIMx->DIER |= 1<<6; 
	d->PulseTIMx->DIER |= 1<<0; 

	d->PulseTIMx->CR1 |= 0<<4;	    
	d->PulseTIMx->CR1 |= 1<<0; 
	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure); 
}

static void _Brush_motor_Brave(motorClass *motor)
{
	GPIO_ResetBits(motor->direct1_IO_domain, motor->direct1_IO_channel);
	GPIO_ResetBits(motor->direct2_IO_domain, motor->direct2_IO_channel);
}

static void _Brush_motor_Foreward(motorClass *motor)
{
	GPIO_SetBits(motor->direct1_IO_domain, motor->direct1_IO_channel);
	GPIO_ResetBits(motor->direct2_IO_domain, motor->direct2_IO_channel);
}

static void _Brush_motor_Reversal(motorClass *motor)
{
	GPIO_ResetBits(motor->direct1_IO_domain, motor->direct1_IO_channel);
	GPIO_SetBits(motor->direct2_IO_domain, motor->direct2_IO_channel);
}

static void _Brushless_motor_brake(motorClass *d)
{
	GPIO_ResetBits(d->brave_IO_domain, d->brave_IO_channel);	
}

static void _Brushless_motor_ResetBrave(motorClass *d)
{
  GPIO_SetBits(d->brave_IO_domain, d->brave_IO_channel);   	 
}

static void _Brushless_motorReverse(motorClass* d,u8 dir)
{
	if(dir == 0)	   //0??
	{
	  GPIO_ResetBits(d->direct1_IO_domain, d->direct1_IO_channel);
		d->dir = 0;
	}
	else if(dir == 1)	//1??
	{
	  GPIO_SetBits(d->direct1_IO_domain, d->direct1_IO_channel); 
		d->dir = 1;
	}	
}


static void _Speed_Contor(motorClass *motor,u16 target_R_speed,float Kp, float Pi,float Kd)
{
	float e,d;
	float det;
	e = target_R_speed - motor->fback_R_speed;
	d = motor->Kdago - motor->fback_R_speed;  
	if( motor->PiSum < 100 )  
	{
		motor->PiSum += e ;
	}
	det = e * Kp + Pi * motor->PiSum + d * Kd;
	if ( det < 0 )
	{
		det = 0;
	}
	switch(motor->PWMOCSwitch)    
	{
		case 0x0001:
			motor->PWMTIMx->CCR1 = det;
			break;
		case 0x0010:
			motor->PWMTIMx->CCR2 = det;
			break;
		case 0x0100:
			motor->PWMTIMx->CCR3 = det;
			break;
		case 0x1000:
			motor->PWMTIMx->CCR4 = det;
			break;		
	}
	motor->Kdago = motor->fback_R_speed;  
}


static void _Speedsamp(motorClass *motor,u16 samptime)
{
	if( motor->motortime > samptime ) 	
	{
		motor->fback_R_speed = 0;
	 	motor->motortime = 0;
		motor->fback_R_speed = motor->PulseTIMx->CNT;  
		motor->PulseTIMx->CNT = 0; 
	}
}


struct _MotorBrush_     
{
	motorClass mt;  		
};

struct _MotorBrushless_ 
{
	motorClass mt;   
};
