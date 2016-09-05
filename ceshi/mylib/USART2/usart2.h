#ifndef __USART2_H
#define __USART2_H 
#include "stm32f10x.h" 

#define USART2_REC_LEN  	100  	//�����������ֽ���
#define EN_USART2_RX 			1			//ʹ�ܣ�1��/��ֹ��0������1����
	  	
extern u8  USART2_RX_BUF[USART2_REC_LEN]; //���ջ���,���USART2_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART2_RX_STA;         				//����״̬���

//����봮���жϽ��գ��벻Ҫע�����º궨��
void uart2_init(u32 bound);
void uart2_SendByte( u8 data );
void uart2_SendString( u8 *str , u8 length );

#endif


