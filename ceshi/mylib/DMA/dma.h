#ifndef __DMA_H
#define	__DMA_H	   
//#include "sys.h"
//#include "timer.h"
//#include "usart.h"

#include "stm32f10x.h" 
#include <stdio.h>
#include <stdlib.h>

void MYDMA_Config(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar,u16 cndtr);
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx,u16 cndtr);	//Ê¹ÄÜDMA1_CHx
void MYDMA_Init(void);	   
			 
#endif




