#include "Mydelay.h"

static u8	fac_us	= 0;          /* usÑÓÊ±±¶³ËÊý */
static u16	fac_ms	= 0;        /* msÑÓÊ±±¶³ËÊý */
#ifdef OS_CRITICAL_METHOD       /* Èç¹ûOS_CRITICAL_METHOD¶¨ÒåÁË,ËµÃ÷Ê¹ÓÃucosIIÁË. */
/* systickÖÐ¶Ï·þÎñº¯Êý,Ê¹ÓÃucosÊ±ÓÃµ½ */
void SysTick_Handler( void )
{
	OSIntEnter();                 /* ½øÈëÖÐ¶Ï */
	OSTimeTick();                 /* µ÷ÓÃucosµÄÊ±ÖÓ·þÎñ³ÌÐò */
	OSIntExit();                  /* ´¥·¢ÈÎÎñÇÐ»»ÈíÖÐ¶Ï */
}

#endif

/*
 * ³õÊ¼»¯ÑÓ³Ùº¯Êý
 * µ±Ê¹ÓÃucosµÄÊ±ºò,´Ëº¯Êý»á³õÊ¼»¯ucosµÄÊ±ÖÓ½ÚÅÄ
 * SYSTICKµÄÊ±ÖÓ¹Ì¶¨ÎªHCLKÊ±ÖÓµÄ1/8
 * SYSCLK:ÏµÍ³Ê±ÖÓ
 */
void delay_init()
{
#ifdef OS_CRITICAL_METHOD                                 /* Èç¹ûOS_CRITICAL_METHOD¶¨ÒåÁË,ËµÃ÷Ê¹ÓÃucosIIÁË. */
	u32 reload;
#endif
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK_Div8 ); /*Ñ¡ÔñÍâ²¿Ê±ÖÓ  HCLK/8 */
	fac_us = SystemCoreClock / 8000000;                     /* ÎªÏµÍ³Ê±ÖÓµÄ1/8 */

#ifdef OS_CRITICAL_METHOD                                 /* Èç¹ûOS_CRITICAL_METHOD¶¨ÒåÁË,ËµÃ÷Ê¹ÓÃucosIIÁË. */
	reload	= SystemCoreClock / 8000000;                    /* Ã¿ÃëÖÓµÄ¼ÆÊý´ÎÊý µ¥Î»ÎªK */
	reload	*= 1000000 / OS_TICKS_PER_SEC;                  /* ¸ù¾ÝOS_TICKS_PER_SECÉè¶¨Òç³öÊ±¼ä */
                                                          /* reloadÎª24Î»¼Ä´æÆ÷,×î´óÖµ:16777216,ÔÚ72MÏÂ,Ô¼ºÏ1.86s×óÓÒ */
	fac_ms		= 1000 / OS_TICKS_PER_SEC;                    /* ´ú±íucos¿ÉÒÔÑÓÊ±µÄ×îÉÙµ¥Î» */
	SysTick->CTRL	|= SysTick_CTRL_TICKINT_Msk;              /* ¿ªÆôSYSTICKÖÐ¶Ï */
	SysTick->LOAD	= reload;                                 /* Ã¿1/OS_TICKS_PER_SECÃëÖÐ¶ÏÒ»´Î */
	SysTick->CTRL	|= SysTick_CTRL_ENABLE_Msk;               /* ¿ªÆôSYSTICK */
#else
	fac_ms = (u16) fac_us * 1000;                           /* ·ÇucosÏÂ,´ú±íÃ¿¸ömsÐèÒªµÄsystickÊ±ÖÓÊý */
#endif
}


#ifdef OS_CRITICAL_METHOD                                 /* Ê¹ÓÃÁËucos */
/*
 * ÑÓÊ±nus
 * nusÎªÒªÑÓÊ±µÄusÊý.
 */
void delay_us( u32 nus )
{
	u32	ticks;
	u32	told, tnow, tcnt = 0;
	u32	reload = SysTick->LOAD;                             /* LOADµÄÖµ */
	ticks	= nus * fac_us;                                   /* ÐèÒªµÄ½ÚÅÄÊý */
	tcnt	= 0;
	told	= SysTick->VAL;                                   /* ¸Õ½øÈëÊ±µÄ¼ÆÊýÆ÷Öµ */
	while ( 1 )
	{
		tnow = SysTick->VAL;
		if ( tnow != told )
		{
			if ( tnow < told )
				tcnt += told - tnow;              /* ÕâÀï×¢ÒâÒ»ÏÂSYSTICKÊÇÒ»¸öµÝ¼õµÄ¼ÆÊýÆ÷¾Í¿ÉÒÔÁË. */
			else tcnt += reload - tnow + told;
			told = tnow;
			if ( tcnt >= ticks )
				break;                            /* Ê±¼ä³¬¹ý/µÈÓÚÒªÑÓ³ÙµÄÊ±¼ä,ÔòÍË³ö. */
		}
	}
}


/*
 * ÑÓÊ±nms
 * nms:ÒªÑÓÊ±µÄmsÊý
 */
void delay_ms( u16 nms )
{
	if ( OSRunning == TRUE )                /* Èç¹ûosÒÑ¾­ÔÚÅÜÁË */
	{
		if ( nms >= fac_ms )                  /* ÑÓÊ±µÄÊ±¼ä´óÓÚucosµÄ×îÉÙÊ±¼äÖÜÆÚ */
		{
			OSTimeDly( nms / fac_ms );          /* ucosÑÓÊ± */
		}
		nms %= fac_ms;                        /* ucosÒÑ¾­ÎÞ·¨Ìá¹©ÕâÃ´Ð¡µÄÑÓÊ±ÁË,²ÉÓÃÆÕÍ¨·½Ê½ÑÓÊ± */
	}
	delay_us( (u32) (nms * 1000) );         /* ÆÕÍ¨·½Ê½ÑÓÊ±,´ËÊ±ucosÎÞ·¨Æô¶¯µ÷¶È. */
}


#else /*²»ÓÃucosÊ± */
/*
 * ÑÓÊ±nus
 * nusÎªÒªÑÓÊ±µÄusÊý.
 */
void delay_us( u32 nus )
{
	u32 temp;
	SysTick->LOAD	= nus * fac_us;                   /* Ê±¼ä¼ÓÔØ */
	SysTick->VAL	= 0x00;                           /* Çå¿Õ¼ÆÊýÆ÷ */
	SysTick->CTRL	|= SysTick_CTRL_ENABLE_Msk;       /* ¿ªÊ¼µ¹Êý */
	do
	{
		temp = SysTick->CTRL;
	}
	while ( temp & 0x01 && !(temp & (1 << 16) ) );  /* µÈ´ýÊ±¼äµ½´ï */
	SysTick->CTRL	&= ~SysTick_CTRL_ENABLE_Msk;      /* ¹Ø±Õ¼ÆÊýÆ÷ */
	SysTick->VAL	= 0X00;                           /* Çå¿Õ¼ÆÊýÆ÷ */
}


/*
 * ÑÓÊ±nms
 * ×¢ÒânmsµÄ·¶Î§
 * SysTick->LOADÎª24Î»¼Ä´æÆ÷,ËùÒÔ,×î´óÑÓÊ±Îª:
 * nms<=0xffffff*8*1000/SYSCLK
 * SYSCLKµ¥Î»ÎªHz,nmsµ¥Î»Îªms
 * ¶Ô72MÌõ¼þÏÂ,nms<=1864
 */
void delay_ms( u16 nms )
{
	u32 temp;
	SysTick->LOAD	= (u32) nms * fac_ms;             /* Ê±¼ä¼ÓÔØ(SysTick->LOADÎª24bit) */
	SysTick->VAL	= 0x00;                           /* Çå¿Õ¼ÆÊýÆ÷ */
	SysTick->CTRL	|= SysTick_CTRL_ENABLE_Msk;       /* ¿ªÊ¼µ¹Êý */
	do
	{
		temp = SysTick->CTRL;
	}
	while ( temp & 0x01 && !(temp & (1 << 16) ) );  /* µÈ´ýÊ±¼äµ½´ï */
	SysTick->CTRL	&= ~SysTick_CTRL_ENABLE_Msk;      /* ¹Ø±Õ¼ÆÊýÆ÷ */
	SysTick->VAL	= 0X00;                           /* Çå¿Õ¼ÆÊýÆ÷ */
}

/*
 * ÑÓÊ±ns
 * ns:0~256
 */
void delay( u8 ns )
{
	while ( ns-- )
	{
		delay_ms( 1000 );
	}
}


#endif
