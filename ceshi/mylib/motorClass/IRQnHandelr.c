#include <motorClass.h>

void TIM5_IRQn_deal()
{
	TIM5->SR = 0;
}

void TIM4_IRQn_deal()
{
	
	TIM4->SR = 0;
}

void TIM3_IRQn_deal()
{
	
	TIM3->SR = 0;
}

void TIM2_IRQn_deal()
{
	
	TIM2->SR = 0; 
}

void TIM1_IRQn_deal()
{
	
	TIM1->SR = 0;
}
