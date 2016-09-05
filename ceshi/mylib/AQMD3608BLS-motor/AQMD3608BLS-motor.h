#ifndef __USART3_H
#define __USART3_H
#include "stm32f10x.h" 
#include "stdio.h"
#include "dma.h"
#include "Mydelay.h"

#define USART3_REC_LEN	100                     		/* ×î´ó½ÓÊÕ»º´æ×Ö½ÚÊý */
#define USART3_RX_EN		1                     		  /* 0,²»½ÓÊÕ;1,½ÓÊÕ. */

extern u8	USART3_RX_BUF[USART3_REC_LEN];   	 				/* ½ÓÊÕ»º³å,×î´óUSART3_REC_LEN×Ö½Ú */
extern u16	USART3_RX_STA;                          /* ½ÓÊÕÊý¾Ý×´Ì¬ */

void usart3_init( u32 bound );                      /* ´®¿Ú3³õÊ¼»¯ */
void uart3_SendString( u8 *str, u8 length );
void Modbus_Send(uint8_t *Message , uint16_t Lenth);
uint8_t Modbus_WriteSgl(uint8_t Slave, uint16_t Adr, uint16_t Data, uint8_t Re, uint8_t Retry);
uint8_t	Modbus_Check(uint8_t *p ,uint16_t l);
uint16_t CRC16(uint8_t *puchMsg , uint16_t usDataLen);
void Modbus_Send(uint8_t *Message , uint16_t Lenth);

#endif
