#include "usart.h"
#include "dma.h"




 

/*
 * 初始化IO 串口1
 * bound:波特率
 */
void uart_init( u32 bound )
{
	/* GPIO端口设置 */
	GPIO_InitTypeDef	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE );     /* 使能USART1，GPIOA时钟 */
	USART_DeInit( USART1 );                                                             /*复位串口1 */
	/* USART1_TX   PA9 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;                                           /* PA9 */
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;                                  		/*复用推挽输出 */
	GPIO_Init( GPIOA, &GPIO_InitStructure );                                            /* 初始化PA9 */

	/* USART1_RX	  PA10 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                                          /* PA10 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;                               /* 浮空输入 */
	GPIO_Init( GPIOA, &GPIO_InitStructure );                                            /* 初始化PA10 */

	/* USART 初始化设置 */
	USART_InitStructure.USART_BaudRate = bound;                                         /* 一般设置为9600; */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;                         /* 字长为8位数据格式 */
	USART_InitStructure.USART_StopBits = USART_StopBits_1;                              /* 一个停止位 */
	USART_InitStructure.USART_Parity = USART_Parity_No;                                 /* 无奇偶校验位 */
	USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;     /* 无硬件数据流控制 */
//#if EN_USART1_RX
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                     /* 收发模式 */
//#else
	//USART_InitStructure.USART_Mode = USART_Mode_Tx;
//#endif
	USART_Init( USART1, &USART_InitStructure );                                         /* 初始化串口 */

//#if EN_USART1_RX                                                                      /* 如果使能了接收 */
	/* Usart1 NVIC 配置 */

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 2;                          /* 抢占优先级2 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;                                  /* 子优先级2 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                                     /* IRQ通道使能 */
	NVIC_Init( &NVIC_InitStructure );                                                   /* 根据指定的参数初始化VIC寄存器 */

//	USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );                                    /* 接收中断 */
	USART_ITConfig( USART1, USART_IT_IDLE, ENABLE );            											  /* 接收空闲中断 */
////#endif
	USART_Cmd( USART1, ENABLE );                                                        /* 使能串口 */
	
	USART_DMACmd( USART1, USART_DMAReq_Rx, ENABLE );																		/* 使能串口1DMA接收 */
	
	MYDMA_Config( DMA1_Channel5, (u32) & USART1->DR, (u32) USART1_RX_BUF, USART1_REC_LEN );	/* DMA1 Channel5 配置*/
}


//#if EN_USART1_RX                                           			/* 如果使能了接收 */

/*
 * 串口1中断服务程序
 * 注意,读取USARTx->SR能避免莫名其妙的错误
 */
u8 USART1_RX_BUF[USART1_REC_LEN]; /* 接收缓冲,最大USART1_REC_LEN个字节. */
/*
 * 接收状态
 * bit15，	接收完成标志
 * bit14，	接收到0x0d
 * bit13~0，	接收到的有效字节数目
 */
u16 USART1_RX_STA = 0;           /* 接收状态标记 */

void USART1_IRQHandler( void )                                   		/* 串口2中断服务程序 */
{
	u8 temp;

	if ( USART_GetITStatus( USART1, USART_IT_IDLE ) != RESET )				/* 接收空闲中断 */
	{
		DMA_Cmd( DMA1_Channel5, DISABLE );
		DMA_ClearFlag( DMA1_FLAG_GL5 );

		temp = USART1_REC_LEN - DMA_GetCurrDataCounter( DMA1_Channel5 );
		USART1_RX_STA	= temp | 0x8000;
		
		DMA_SetCurrDataCounter( DMA1_Channel5, USART1_REC_LEN );
		DMA_Cmd( DMA1_Channel5, ENABLE );

		temp	= USART1->SR;
		temp	= USART1->DR;																						/* 这两句用于清除标志位 */
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

