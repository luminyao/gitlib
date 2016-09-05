#ifndef __IRQ__H
#define __IRQ__H
#include "stm32f10x.h" 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define bool int
#define true 1
#define false 0


void TIM5_IRQn_deal(void);
void TIM4_IRQn_deal(void);
void TIM3_IRQn_deal(void);
void TIM2_IRQn_deal(void);
void TIM1_IRQn_deal(void);
#endif

#define GPIO(domain,channel)\
				GPIO##domain,\
				GPIO_Pin_##channel


#define CONTRACT_IO_INIT_1(Gtype,domain,channel)\
        Gtype.ContrAct1_IO_domain = GPIO##domain;\
				Gtype.ContrAct1_IO_channel = GPIO_Pin_##channel;

#define CONTRACT_IO_INIT_2(Gtype,domain,channel)\
        Gtype.ContrAct2_IO_domain = GPIO##domain;\
				Gtype.ContrAct2_IO_channel = GPIO_Pin_##channel;
/* 默认参数 */
#define	MOTORPARAM(motorStruct) \
				motorStruct.PwmVoltage =0; \
				motorStruct.PWMTIM_OCPolarity = TIM_OCPolarity_High; \
				motorStruct.PWMTIM_ClockDivision =  TIM_CKD_DIV1;	 \
				motorStruct.PWMarr = 1000-1; \
				motorStruct.PWMpsc = 0; \
				motorStruct.PWMTIM_OCMode = TIM_OCMode_PWM1; \
				motorStruct.PWMTIM_CounterMode = TIM_CounterMode_Up

typedef struct _motorClass motorClass;
typedef struct  _motorParam  motorParam;
typedef struct _MovBaseClass MovBaseClass;
typedef struct _MovBaseParam MovBaseParam;

motorClass  *new_motor(motorParam *_motor);
MovBaseClass *new_MovBase(MovBaseParam *_MovBaseParam);

static void _Speed_Contor(motorClass *motor,float target_R_speed,float Kp, float Pi,float Kd);
static void _motor_MovControl(motorClass *d,u8 n1,u8 n2);
static void _MovBaseContr(MovBaseClass *MovBase,float Linear,float Angular);
static void Samp(motorClass *motor);

void CalOdometer(MovBaseClass *Mov,float offset_motor1,float offset_motor2);
void Get_SpeedandOdometry(MovBaseClass *Mov);




static void (*TIM_Callback2)(u8 a);
static void (*TIM_Callback3)(u8 a);
static void (*TIM_Callback4)(u8 a);
static void (*TIM_Callback5)(u8 a);

static void (*_TIM1_IRQn_deal)(void);
static void (*_TIM2_IRQn_deal)(void);
static void (*_TIM3_IRQn_deal)(void);
static void (*_TIM4_IRQn_deal)(void);
static void (*_TIM5_IRQn_deal)(void);


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
);

MovBaseClass * BuildMovBase(motorClass * m1,motorClass * m2,float WH);




struct _MovBaseClass
{
	motorClass * m[2];
	float WheelBetw;   //轮间距
	float Liner;
	float Angular;
	float X_Odometry;
	float Theta_Odometry;   
	float Y_Odometry;
	
	float Y_Refer;
	float Theta;
	
	float Lin_Compen;
	void (*MovBaseContr)(MovBaseClass *MovBase,float Linear,float Angular);
};

struct _MovBaseParam
{
	motorClass * m[2];
	float WheelBetw;   //轮间距
};


struct _motorClass   //电机运行时过程变量结构体
{	
	GPIO_TypeDef * ContrAct1_IO_domain;
	u16 ContrAct1_IO_channel;			//控制引脚1

	GPIO_TypeDef * ContrAct2_IO_domain;
	u16 ContrAct2_IO_channel;			//控制引脚2
		
	u16 PWMOCSwitch;    	
	
	float PiSum;						//积分和		
	float Kdago;						//微分差
  u16 motormile;					//电机里程
	u16 motormileK;
	float fback_R_speed;			//反馈速度
	u16 Overflow_Mark;
	u8  CAPTURE_STA;
	u8  StopMark;
	u16 UnitMileTime;
	int16_t trSpeed;
	float Kp;
	float Ki;
	float Kd;
	int16_t Wheel_cir;
	int16_t Wheel_pulse_circle;
	bool Orthornot;
	uint16_t MotorTime;
	uint16_t MotorTimeK;
	
	TIM_TypeDef *PWMTIMx;
	TIM_TypeDef *PulseTIMx;
	
	void (*Speed_Contor)(motorClass *d,float target_R_speed,float Kp, float Pi,float Kd); 
	void (*motor_MovControl)(motorClass *d,u8 n1,u8 n2);
	/*.........
	各类算法；
	........*/
};



struct  _motorParam   //新建电机参数结构体
{ 
	GPIO_TypeDef * ContrAct1_IO_domain;
	u16 ContrAct1_IO_channel;

	GPIO_TypeDef * ContrAct2_IO_domain;
	u16 ContrAct2_IO_channel;

	GPIO_TypeDef * encode_IO_domain_1;  
	u16  encode_IO_channel_1;

	GPIO_TypeDef * encode_IO_domain_2;  
	u16  encode_IO_channel_2;
	  					
/*******************PWM***********************************/	
	TIM_TypeDef *PWMTIMx;
	GPIO_TypeDef * PWM_IO_domain;
	u16 PWM_IO_channel;
	int16_t Wheel_cir;
	int16_t Wheel_pulse_circle;
	bool Orthornot;
	
	u16 PwmVoltage;
	u16 PWMTIM_OCMode;
//	u16 PWMTIM_Channel;
	u16 PWMTIM_Pulse;
	u16 PWMTIM_OCPolarity;
	u16 PWMOCSwitch;	
	u16 PWMarr;
	u16 PWMpsc;
	u8  PWMTIM_ClockDivision;
	u16 PWMTIM_CounterMode;
	u32 PWMTIM_GPIO_Remap;
/******************************************************/	
	TIM_TypeDef *PulseTIMx;  //编码时钟 
};	






static void _Speed_Contor(motorClass *motor,float target_R_speed,float Kp, float Pi,float Kd)
{
	float e,d;
	float det;
	if ( motor->Orthornot )
	{
		Samp(motor);
	}else 
	if ((!motor->Orthornot) && (motor->UnitMileTime != 0))
	{
		motor->fback_R_speed = ( motor->Wheel_cir / motor->Wheel_pulse_circle ) / ((float) motor->UnitMileTime);   //单位m/s
	}else if ( motor->UnitMileTime == 0 )
	{
		motor->fback_R_speed = 0;
	}
	if ( target_R_speed < 0 ) target_R_speed = target_R_speed *(-1);
	e = target_R_speed - motor->fback_R_speed;
	d = motor->Kdago - motor->fback_R_speed; 

	if ( motor->PulseTIMx->CNT > 700 )
	{
		motor->UnitMileTime = 0;
	  motor->fback_R_speed = 0;
	}
//	if( motor->PiSum < 100 )  
	{
		motor->PiSum +=  e ;
	}
	det = e * Kp + Pi * motor->PiSum  + d * Kd;
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


static void _motor_MovControl(motorClass *d,u8 n1,u8 n2)
{
	if(n1 == 0)	   
	{
	  GPIO_ResetBits(d->ContrAct1_IO_domain, d->ContrAct1_IO_channel);
	}
	else if(n1 == 1)	
	{
	  GPIO_SetBits(d->ContrAct1_IO_domain, d->ContrAct1_IO_channel); 
	}	
	if(n2 == 0)	   
	{
	  GPIO_ResetBits(d->ContrAct2_IO_domain, d->ContrAct2_IO_channel);
	}
	else if(n2 == 1)	
	{
	  GPIO_SetBits(d->ContrAct2_IO_domain, d->ContrAct2_IO_channel); 
	}	
	if ( d->trSpeed < 0 )
	{
		GPIO_ResetBits(d->ContrAct1_IO_domain, d->ContrAct1_IO_channel);
	}else if ( d->trSpeed > 0 )
	{
		GPIO_SetBits(d->ContrAct1_IO_domain, d->ContrAct1_IO_channel); 
	}
	if ( d->trSpeed  == 0 )
	{
		GPIO_ResetBits(d->ContrAct2_IO_domain, d->ContrAct2_IO_channel);
	}
}

static void Samp(motorClass *motor)
{
	float time;
	if (motor->PulseTIMx->SR&0x01) //溢出
	{
		motor->CAPTURE_STA++;
		motor->PulseTIMx->SR = 0;
	}else
	{
		motor->motormile = (0xffff) * motor->CAPTURE_STA + motor->PulseTIMx->CNT;
	}
  if (motor->PWMTIMx->CNT - motor->MotorTimeK >= 0)
	{
		motor->MotorTime += motor->PWMTIMx->CNT - motor->MotorTimeK;
	}
	else
	{
		motor->MotorTime += motor->PWMTIMx->ARR - motor->MotorTimeK + motor->PWMTIMx->CNT;
	}
	time = (float) motor->MotorTime / 65530;
	motor->MotorTimeK = motor->PWMTIMx->CNT;
	motor->fback_R_speed = (( motor->motormile - motor->motormileK ) *( motor->Wheel_cir / motor->Wheel_pulse_circle ))/time;
	motor->motormileK = motor->motormile;
}
