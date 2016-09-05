#include "usart2.h"
#include "dma.h"

///* ���ʹ��ucos,����������ͷ�ļ�����. */
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"     /* ucos ʹ�� */
#endif

/* �������´���,֧��printf����,������Ҫѡ��use MicroLIB */
#if 1
#pragma import(__use_no_semihosting)
/* ��׼����Ҫ��֧�ֺ��� */
struct __FILE
{
	int handle;
};

FILE __stdout;
/* ����_sys_exit()�Ա���ʹ�ð�����ģʽ */
_sys_exit( int x )
{
	x = x;
}
/* �ض���fputc���� */
int fputc( int ch, FILE *f )
{
	while ( (USART2->SR & 0X40) == 0 );    /* ѭ������,ֱ��������� */
	USART2->DR = (u8) ch;
	return(ch);
}

#endif

void uart2_init( u32 bound )
{
	USART_InitTypeDef	USART_InitStructure;
	GPIO_InitTypeDef	GPIO_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );        		/* ʹ��GPIOAʱ�� */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, ENABLE );       		/* ʹ��USART2ʱ�� */

	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_2;
	GPIO_Init( GPIOA, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_3;
	GPIO_Init( GPIOA, &GPIO_InitStructure );

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
	/* Configure USART2 */
	USART_Init( USART2, &USART_InitStructure );

	/* Usart2 NVIC ���� */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 2;    		/* ��ռ���ȼ� */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;								/* �����ȼ� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;										/* IRQͨ��ʹ�� */
	NVIC_Init( &NVIC_InitStructure );																	/* ����ָ���Ĳ�����ʼ��VIC�Ĵ��� */

	USART_ITConfig( USART2, USART_IT_IDLE, ENABLE );            		  /* ���տ����ж� */

	USART_Cmd( USART2, ENABLE );																			/* ʹ�ܴ��� */

	USART_DMACmd( USART2, USART_DMAReq_Rx, ENABLE );									/* ʹ�ܴ���2DMA���� */

	MYDMA_Config( DMA1_Channel6, (u32) & USART2->DR, (u32) USART2_RX_BUF, USART2_REC_LEN );		/* DMA1 Channel6 ����*/
}


//#if EN_USART2_RX																										/* ���ʹ���˽��� */

/*
 * ����2�жϷ������
 * ע��,��ȡUSART2x->SR�ܱ���Ī������Ĵ���
 */
u8 USART2_RX_BUF[USART2_REC_LEN];																 		/* ���ջ���,���USART2_REC_LEN���ֽ�. */
/*
 * ����״̬
 * bit15��	������ɱ�־
 * bit14��	���յ�0x0d
 * bit13~0��	���յ�����Ч�ֽ���Ŀ
 */
u16 USART2_RX_STA = 0;																							/* ����״̬��� */

void USART2_IRQHandler( void )                                   		/* ����2�жϷ������ */
{
	u8 temp;

	if ( USART_GetITStatus( USART2, USART_IT_IDLE ) != RESET )				/* ���տ����ж� */
	{
		DMA_Cmd( DMA1_Channel6, DISABLE );

		temp = USART2_REC_LEN - DMA_GetCurrDataCounter( DMA1_Channel6 );
		USART2_RX_STA	= temp | 0x8000;
		
		DMA_SetCurrDataCounter( DMA1_Channel6, USART2_REC_LEN );
		DMA_Cmd( DMA1_Channel6, ENABLE );

		temp	= USART2->SR;
		temp	= USART2->DR;			/*	���������������־λ����USART_ClearITPendingBit(USART2, USART_IT_IDLE);û�� */
	}
}

//#endif


void uart2_SendByte( u8 data )
{
	while ( USART_GetFlagStatus( USART2, USART_FLAG_TC ) != SET );
	USART_SendData( USART2, data );
}

void uart2_SendString( u8 *str, u8 length )
{
	while ( length-- )
	{
		uart2_SendByte( *str++ );
	}
}

