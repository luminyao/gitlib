#ifndef __USART3_H
#define __USART3_H
#include "stdio.h"
#include "time.h"
#include "stm32f10x.h" 
#include "dma.h"

#define USART3_REC_LEN	100                     		/* 最大接收缓存字节数 */
#define USART3_RX_EN		1                     		  /* 0,不接收;1,接收. */

extern u8	USART3_RX_BUF[USART3_REC_LEN];   	 				/* 接收缓冲,最大USART3_REC_LEN字节 */
extern u16	USART3_RX_STA;                          /* 接收数据状态 */

void usart3_init( u32 bound );                      /* 串口3初始化 */
void TIM7_Int_Init( u16 arr, u16 psc );
void uart3_SendString( u8 *str, u8 length );

#endif

