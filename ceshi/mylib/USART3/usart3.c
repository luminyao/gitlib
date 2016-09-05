#include "usart3.h"


/*
 * ��ʼ��IO ����3
 * bound:������
 */
void usart3_init( u32 bound )
{
	NVIC_InitTypeDef	NVIC_InitStructure;
	GPIO_InitTypeDef	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );                                 /* ʹ��GPIOBʱ�� */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3, ENABLE );                                /* ʹ��USART3ʱ�� */

	USART_DeInit( USART3 );                                                                 /*��λ����3 */

	/* USART3_TX   PB10 */
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_10;																							/* PB10 */
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF_PP;                                    		/*����������� */
	GPIO_Init( GPIOB, &GPIO_InitStructure );                                                /* ��ʼ��PB10 */

	/* USART3_RX	  PB11 */
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;                                		/* �������� */
	GPIO_Init( GPIOB, &GPIO_InitStructure );                                                /* ��ʼ��PB11 */

	USART_InitStructure.USART_BaudRate = bound;																							/* ������ */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;                 					  /* �ֳ�Ϊ8λ���ݸ�ʽ */
	USART_InitStructure.USART_StopBits = USART_StopBits_2;                    					  	/* ����ֹͣλ */
	USART_InitStructure.USART_Parity = USART_Parity_No;                     							  /* ����żУ��λ */
	USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;         /* ��Ӳ������������ */
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                         /* �շ�ģʽ */

	USART_Init( USART3, &USART_InitStructure );                                             /* ��ʼ������3 */

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;                              /* ��ռ���ȼ�1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;                                 			/* �����ȼ�1 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;																					/* IRQͨ��ʹ�� */
	NVIC_Init( &NVIC_InitStructure );                                                       /* ����ָ���Ĳ�����ʼ��VIC�Ĵ��� */

	USART_ITConfig( USART3, USART_IT_IDLE, ENABLE );            		 												/* ���տ����ж� */
	
	USART_Cmd( USART3, ENABLE );																														/* ʹ�ܴ��� */

	USART_DMACmd( USART3, USART_DMAReq_Rx, ENABLE );																				/* ʹ�ܴ���3DMA���� */

	MYDMA_Config( DMA1_Channel3, (u32) & USART3->DR, (u32) USART3_RX_BUF, USART3_REC_LEN );	/* DMA1 Channel3 ����*/
}


#ifdef USART3_RX_EN                                     /* ���ʹ���˽��� */

/*
 * ͨ���жϽ�������2���ַ�֮���ʱ������100ms�������ǲ���һ������������.
 * ���2���ַ����ռ������100ms,����Ϊ����1����������.Ҳ���ǳ���100msû�н��յ�
 * �κ�����,���ʾ�˴ν������.
 * ���յ�������״̬
 * [15]:0,û�н��յ�����;1,���յ���һ������.
 * [14:0]:���յ������ݳ���
 */
 
u8 USART3_RX_BUF[USART3_REC_LEN];																/* ���ջ���,���USART_REC_LEN���ֽ�. */
u16 USART3_RX_STA = 0;

void USART3_IRQHandler( void )                                   		/* ����2�жϷ������ */
{
	u8 temp;
#ifdef OS_TICKS_PER_SEC                                        			/* ���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��. */
	OSIntEnter();
#endif
	if ( USART_GetITStatus( USART3, USART_IT_IDLE ) != RESET )				/* ���տ����ж� */
	{
		if( (USART3_RX_STA & 0x8000) == 0 )
		{
			DMA_Cmd( DMA1_Channel3, DISABLE );

			temp = USART3_REC_LEN - DMA_GetCurrDataCounter( DMA1_Channel3 );
			USART3_RX_STA	= temp | 0x8000;

			DMA_SetCurrDataCounter( DMA1_Channel3, USART3_REC_LEN );
			DMA_Cmd( DMA1_Channel3, ENABLE );

			temp	= USART3->SR;
			temp	= USART3->DR;			/*	���������������־λ����USART_ClearITPendingBit(USART3, USART_IT_IDLE);û�� */
		}
	}
#ifdef OS_TICKS_PER_SEC  	 	/* ���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��. */
	OSIntExit();
#endif
}

#endif


void uart3_SendByte( u8 data )
{
	while ( USART_GetFlagStatus( USART3, USART_FLAG_TC ) != SET );
	USART_SendData( USART3, data );
}

void uart3_SendString( u8 *str , u8 length )
{
	while(length--)
	{
		uart3_SendByte( *str++ );
	}
}

