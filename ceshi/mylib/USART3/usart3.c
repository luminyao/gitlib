#include "usart3.h"


/*
 * 初始化IO 串口3
 * bound:波特率
 */
void usart3_init( u32 bound )
{
	NVIC_InitTypeDef	NVIC_InitStructure;
	GPIO_InitTypeDef	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );                                 /* 使能GPIOB时钟 */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3, ENABLE );                                /* 使能USART3时钟 */

	USART_DeInit( USART3 );                                                                 /*复位串口3 */

	/* USART3_TX   PB10 */
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_10;																							/* PB10 */
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF_PP;                                    		/*复用推挽输出 */
	GPIO_Init( GPIOB, &GPIO_InitStructure );                                                /* 初始化PB10 */

	/* USART3_RX	  PB11 */
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;                                		/* 浮空输入 */
	GPIO_Init( GPIOB, &GPIO_InitStructure );                                                /* 初始化PB11 */

	USART_InitStructure.USART_BaudRate = bound;																							/* 波特率 */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;                 					  /* 字长为8位数据格式 */
	USART_InitStructure.USART_StopBits = USART_StopBits_2;                    					  	/* 两个停止位 */
	USART_InitStructure.USART_Parity = USART_Parity_No;                     							  /* 无奇偶校验位 */
	USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;         /* 无硬件数据流控制 */
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                         /* 收发模式 */

	USART_Init( USART3, &USART_InitStructure );                                             /* 初始化串口3 */

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;                              /* 抢占优先级1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;                                 			/* 子优先级1 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;																					/* IRQ通道使能 */
	NVIC_Init( &NVIC_InitStructure );                                                       /* 根据指定的参数初始化VIC寄存器 */

	USART_ITConfig( USART3, USART_IT_IDLE, ENABLE );            		 												/* 接收空闲中断 */
	
	USART_Cmd( USART3, ENABLE );																														/* 使能串口 */

	USART_DMACmd( USART3, USART_DMAReq_Rx, ENABLE );																				/* 使能串口3DMA接收 */

	MYDMA_Config( DMA1_Channel3, (u32) & USART3->DR, (u32) USART3_RX_BUF, USART3_REC_LEN );	/* DMA1 Channel3 配置*/
}


#ifdef USART3_RX_EN                                     /* 如果使能了接收 */

/*
 * 通过判断接收连续2个字符之间的时间差不大于100ms来决定是不是一次连续的数据.
 * 如果2个字符接收间隔超过100ms,则认为不是1次连续数据.也就是超过100ms没有接收到
 * 任何数据,则表示此次接收完毕.
 * 接收到的数据状态
 * [15]:0,没有接收到数据;1,接收到了一批数据.
 * [14:0]:接收到的数据长度
 */
 
u8 USART3_RX_BUF[USART3_REC_LEN];																/* 接收缓冲,最大USART_REC_LEN个字节. */
u16 USART3_RX_STA = 0;

void USART3_IRQHandler( void )                                   		/* 串口2中断服务程序 */
{
	u8 temp;
#ifdef OS_TICKS_PER_SEC                                        			/* 如果时钟节拍数定义了,说明要使用ucosII了. */
	OSIntEnter();
#endif
	if ( USART_GetITStatus( USART3, USART_IT_IDLE ) != RESET )				/* 接收空闲中断 */
	{
		if( (USART3_RX_STA & 0x8000) == 0 )
		{
			DMA_Cmd( DMA1_Channel3, DISABLE );

			temp = USART3_REC_LEN - DMA_GetCurrDataCounter( DMA1_Channel3 );
			USART3_RX_STA	= temp | 0x8000;

			DMA_SetCurrDataCounter( DMA1_Channel3, USART3_REC_LEN );
			DMA_Cmd( DMA1_Channel3, ENABLE );

			temp	= USART3->SR;
			temp	= USART3->DR;			/*	这两句用于清除标志位，而USART_ClearITPendingBit(USART3, USART_IT_IDLE);没用 */
		}
	}
#ifdef OS_TICKS_PER_SEC  	 	/* 如果时钟节拍数定义了,说明要使用ucosII了. */
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

