#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "stm32f10x.h" 
#define USART1_REC_LEN	100                     /* �����������ֽ��� 100 */
#define EN_USART1_RX	1                      		/* ʹ�ܣ�1��/��ֹ��0������1���� */

extern u8	USART1_RX_BUF[USART1_REC_LEN];        /* ���ջ���,���USART1_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� */
extern u16	USART1_RX_STA;                    	/* ����״̬��� */

void uart_init( u32 bound );
void uart_SendByte( u8 data );
void uart_SendString( char *str, u8 length );

#endif

