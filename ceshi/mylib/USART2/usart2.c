#include "usart2.h"
#include "dma.h"

///* 如果使用ucos,则包括下面的头文件即可. */
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"     /* ucos 使用 */
#endif

/* 加入以下代码,支持printf函数,而不需要选择use MicroLIB */
#if 1
#pragma import(__use_no_semihosting)
/* 标准库需要的支持函数 */
struct __FILE
{
	int handle;
};

FILE __stdout;
/* 定义_sys_exit()以避免使用半主机模式 */
_sys_exit( int x )
{
	x = x;
}
/* 重定义fputc函数 */
int fputc( int ch, FILE *f )
{
	while ( (USART2->SR & 0X40) == 0 );    /* 循环发送,直到发送完毕 */
	USART2->DR = (u8) ch;
	return(ch);
}

#endif

void uart2_init( u32 bound )
{
	USART_InitTypeDef	USART_InitStructure;
	GPIO_InitTypeDef	GPIO_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );        		/* 使能GPIOA时钟 */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, ENABLE );       		/* 使能USART2时钟 */

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

	/* Usart2 NVIC 配置 */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 2;    		/* 抢占优先级 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;								/* 子优先级 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;										/* IRQ通道使能 */
	NVIC_Init( &NVIC_InitStructure );																	/* 根据指定的参数初始化VIC寄存器 */

	USART_ITConfig( USART2, USART_IT_IDLE, ENABLE );            		  /* 接收空闲中断 */

	USART_Cmd( USART2, ENABLE );																			/* 使能串口 */

	USART_DMACmd( USART2, USART_DMAReq_Rx, ENABLE );									/* 使能串口2DMA接收 */

	MYDMA_Config( DMA1_Channel6, (u32) & USART2->DR, (u32) USART2_RX_BUF, USART2_REC_LEN );		/* DMA1 Channel6 配置*/
}


//#if EN_USART2_RX																										/* 如果使能了接收 */

/*
 * 串口2中断服务程序
 * 注意,读取USART2x->SR能避免莫名其妙的错误
 */
u8 USART2_RX_BUF[USART2_REC_LEN];																 		/* 接收缓冲,最大USART2_REC_LEN个字节. */
/*
 * 接收状态
 * bit15，	接收完成标志
 * bit14，	接收到0x0d
 * bit13~0，	接收到的有效字节数目
 */
u16 USART2_RX_STA = 0;																							/* 接收状态标记 */

void USART2_IRQHandler( void )                                   		/* 串口2中断服务程序 */
{
	u8 temp;

	if ( USART_GetITStatus( USART2, USART_IT_IDLE ) != RESET )				/* 接收空闲中断 */
	{
		DMA_Cmd( DMA1_Channel6, DISABLE );

		temp = USART2_REC_LEN - DMA_GetCurrDataCounter( DMA1_Channel6 );
		USART2_RX_STA	= temp | 0x8000;
		
		DMA_SetCurrDataCounter( DMA1_Channel6, USART2_REC_LEN );
		DMA_Cmd( DMA1_Channel6, ENABLE );

		temp	= USART2->SR;
		temp	= USART2->DR;			/*	这两句用于清除标志位，而USART_ClearITPendingBit(USART2, USART_IT_IDLE);没用 */
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

