#ifndef __USART3_H
#define __USART3_H
#include "stdio.h"
#include "time.h"
#include "stm32f10x.h" 
#include "dma.h"

#define USART3_REC_LEN	100                     		/* �����ջ����ֽ��� */
#define USART3_RX_EN		1                     		  /* 0,������;1,����. */

extern u8	USART3_RX_BUF[USART3_REC_LEN];   	 				/* ���ջ���,���USART3_REC_LEN�ֽ� */
extern u16	USART3_RX_STA;                          /* ��������״̬ */

void usart3_init( u32 bound );                      /* ����3��ʼ�� */
void TIM7_Int_Init( u16 arr, u16 psc );
void uart3_SendString( u8 *str, u8 length );

#endif

