#include "usart.h"
#include "dma.h"




 

/*
 * ��ʼ��IO ����1
 * bound:������
 */
void uart_init( u32 bound )
{
	/* GPIO�˿����� */
	GPIO_InitTypeDef	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE );     /* ʹ��USART1��GPIOAʱ�� */
	USART_DeInit( USART1 );                                                             /*��λ����1 */
	/* USART1_TX   PA9 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;                                           /* PA9 */
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;                                  		/*����������� */
	GPIO_Init( GPIOA, &GPIO_InitStructure );                                            /* ��ʼ��PA9 */

	/* USART1_RX	  PA10 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                                          /* PA10 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;                               /* �������� */
	GPIO_Init( GPIOA, &GPIO_InitStructure );                                            /* ��ʼ��PA10 */

	/* USART ��ʼ������ */
	USART_InitStructure.USART_BaudRate = bound;                                         /* һ������Ϊ9600; */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;                         /* �ֳ�Ϊ8λ���ݸ�ʽ */
	USART_InitStructure.USART_StopBits = USART_StopBits_1;                              /* һ��ֹͣλ */
	USART_InitStructure.USART_Parity = USART_Parity_No;                                 /* ����żУ��λ */
	USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;     /* ��Ӳ������������ */
//#if EN_USART1_RX
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                     /* �շ�ģʽ */
//#else
	//USART_InitStructure.USART_Mode = USART_Mode_Tx;
//#endif
	USART_Init( USART1, &USART_InitStructure );                                         /* ��ʼ������ */

//#if EN_USART1_RX                                                                      /* ���ʹ���˽��� */
	/* Usart1 NVIC ���� */

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 2;                          /* ��ռ���ȼ�2 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;                                  /* �����ȼ�2 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                                     /* IRQͨ��ʹ�� */
	NVIC_Init( &NVIC_InitStructure );                                                   /* ����ָ���Ĳ�����ʼ��VIC�Ĵ��� */

//	USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );                                    /* �����ж� */
	USART_ITConfig( USART1, USART_IT_IDLE, ENABLE );            											  /* ���տ����ж� */
////#endif
	USART_Cmd( USART1, ENABLE );                                                        /* ʹ�ܴ��� */
	
	USART_DMACmd( USART1, USART_DMAReq_Rx, ENABLE );																		/* ʹ�ܴ���1DMA���� */
	
	MYDMA_Config( DMA1_Channel5, (u32) & USART1->DR, (u32) USART1_RX_BUF, USART1_REC_LEN );	/* DMA1 Channel5 ����*/
}


//#if EN_USART1_RX                                           			/* ���ʹ���˽��� */

/*
 * ����1�жϷ������
 * ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
 */
u8 USART1_RX_BUF[USART1_REC_LEN]; /* ���ջ���,���USART1_REC_LEN���ֽ�. */
/*
 * ����״̬
 * bit15��	������ɱ�־
 * bit14��	���յ�0x0d
 * bit13~0��	���յ�����Ч�ֽ���Ŀ
 */
u16 USART1_RX_STA = 0;           /* ����״̬��� */

void USART1_IRQHandler( void )                                   		/* ����2�жϷ������ */
{
	u8 temp;

	if ( USART_GetITStatus( USART1, USART_IT_IDLE ) != RESET )				/* ���տ����ж� */
	{
		DMA_Cmd( DMA1_Channel5, DISABLE );
		DMA_ClearFlag( DMA1_FLAG_GL5 );

		temp = USART1_REC_LEN - DMA_GetCurrDataCounter( DMA1_Channel5 );
		USART1_RX_STA	= temp | 0x8000;
		
		DMA_SetCurrDataCounter( DMA1_Channel5, USART1_REC_LEN );
		DMA_Cmd( DMA1_Channel5, ENABLE );

		temp	= USART1->SR;
		temp	= USART1->DR;																						/* ���������������־λ */
		USART_ClearITPendingBit(USART1, USART_IT_IDLE);
	}
}


////#endif


void uart_SendByte( u8 data )
{
	while ( USART_GetFlagStatus( USART1, USART_FLAG_TC ) != SET );
	USART_SendData( USART1, data );
}

void uart_SendString( char *str , u8 length )
{
	while(length--)
	{
		uart_SendByte( *str++ );
	}
}

