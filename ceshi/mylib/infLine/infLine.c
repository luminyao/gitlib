#include "infLine.h"
#include "sys.h"
extern SETPara set;

void init_infGPIO()
{
	GPIO_InitTypeDef led;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	led.GPIO_Pin=GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; 
	led.GPIO_Speed=GPIO_Speed_50MHz;
	led.GPIO_Mode=GPIO_Mode_IPD;
	GPIO_Init(GPIOB,&led);
}

uint16_t check_inf()
{
	uint16_t infmod;
	if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)==1)
	{
		infmod |= 0x1000;
	}else
	{
		infmod &= 0x0111;
	}
	if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6)==1)
	{
		infmod |= 0x0100;
	}else
	{
		infmod &= 0x1011;
	}
	if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5)==1)
	{
		infmod |= 0x0010;
	}else
	{
		infmod &= 0x1101;
	}
	if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)==1)
	{
		infmod |= 0x0001;
	}else
	{
		infmod &= 0x1110;
	}
	
	return infmod;
}

uint8_t LineFlow_dec(void)
{   //红外有反射光(未碰到黑线)为1,右电机0x01,左电机0x02。让模块正对自己,7,5红外在右侧，6,4红外在左侧
	uint16_t infmod = 0;
	static uint16_t stopmod = 0,stop = 0,clc;
	static float weigth = 120;
	infmod = check_inf();
//	switch (infmod)
//	{ //左转为正角速度，右转为负角速度
//		case 0x0111: if ( weigth < 400 ) weigth = 220;break;  //左侧红外
//	  case 0x0011: if ( weigth < 400 ) weigth = 160;break;
//		case 0x1011: if ( weigth < 400 ) weigth = 140;break;	//左侧红外
//		
//		case 0x1110: if ( weigth > 0 ) weigth = 20;break;    //右侧红外
//	  case 0x1100: if ( weigth > 0 ) weigth = 80;break;
//		case 0x1101: if ( weigth > 0 ) weigth = 100;break;		//左侧红外
//		case 0x0110: return 1;
//		case 0x0000: weigth = 120;stopmod |= 0x0001;break;
//	}
//	switch (infmod)
//	{ //左转为正角速度，右转为负角速度
//		case 0x0111: if ( weigth < 300 ) weigth += 0.6;break;  //左侧红外
//	  case 0x0011: if ( weigth < 300 ) weigth += 0.3;break;
//		case 0x1011: if ( weigth < 300 ) weigth += 0.08;break;	//左侧红外
//		
//		case 0x1110: if ( weigth > 0 ) weigth -= 0.6;break;    //右侧红外
//	  case 0x1100: if ( weigth > 0 ) weigth -= 0.3;break;
//		case 0x1101: if ( weigth > 0 ) weigth -= 0.08;break;		//左侧红外
//		
//		case 0x0110: weigth = 120; return 1;
//		case 0x0000: weigth = 120;stopmod |= 0x0001;break;
//	}
	switch (infmod)
	{ //左转为正角速度，右转为负角速度
		case 0x0111: pwmr = 70;pwml = 0;break;  //左侧红外
	  case 0x0011: pwmr = 70;pwml = 0;break;
		case 0x1011: pwmr = 50;pwml = 0;break;	//左侧红外
		
		case 0x1110: pwml = -70;pwmr = 0;break;    //右侧红外
	  case 0x1100: pwml = -70;pwmr = 0;break;
		case 0x1101: pwml = -50;pwmr = 0;break;		//左侧红外
		case 0x0110: return 1;
		case 0x0000: pwmr = 0;pwml = 0;stopmod |= 0x0001;break;
	}
	if ( stopmod == 0x0101 )
	{
		clc = 0;
		stopmod = 0;
		stop = 1;
	}
	if ( clc == 10 )
	{
		stop = 1;
	}
	if (infmod == 0x1111)
	{
		weigth = 120;
		if ( stopmod & 0x0001 )
		{
			clc++;
			stopmod = stopmod<<8;
		}
		pwml = -50;
		pwmr = 50;
	}
	if (stop != 1)
	{
//		set.linear = 110.0;
//		set.Angular = ( 120 - weigth ) * 20; 
		//send_concom3();
	}
	else
	{
		send_concom5();
		delay_ms(2);
	}
	return 	0;
}
