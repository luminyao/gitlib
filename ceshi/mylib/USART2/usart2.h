#ifndef __USART2_H
#define __USART2_H 
#include "stm32f10x.h" 

#define USART2_REC_LEN  	100  	//定义最大接收字节数
#define EN_USART2_RX 			1			//使能（1）/禁止（0）串口1接收
	  	
extern u8  USART2_RX_BUF[USART2_REC_LEN]; //接收缓冲,最大USART2_REC_LEN个字节.末字节为换行符 
extern u16 USART2_RX_STA;         				//接收状态标记

//如果想串口中断接收，请不要注释以下宏定义
void uart2_init(u32 bound);
void uart2_SendByte( u8 data );
void uart2_SendString( u8 *str , u8 length );

#endif


