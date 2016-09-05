#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "stm32f10x.h" 
#define USART1_REC_LEN	100                     /* 定义最大接收字节数 100 */
#define EN_USART1_RX	1                      		/* 使能（1）/禁止（0）串口1接收 */

extern u8	USART1_RX_BUF[USART1_REC_LEN];        /* 接收缓冲,最大USART1_REC_LEN个字节.末字节为换行符 */
extern u16	USART1_RX_STA;                    	/* 接收状态标记 */

void uart_init( u32 bound );
void uart_SendByte( u8 data );
void uart_SendString( char *str, u8 length );

#endif

