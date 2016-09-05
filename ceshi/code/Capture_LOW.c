#include "stm32f10x.h"
#include "sys.h"

extern  u8 TIM5CH1_CAPTURE_STA; //0x80标记捕获到低电平，0x40标记捕获到高电平
extern u32 TIM5CH1_CAPTURE_VAL;
extern motorClass *motor[2];
 void Init_cap_gpio()
 {
	GPIO_InitTypeDef	GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOA,&GPIO_InitStructure);
 }
 

void Init_TIM5(u32 arr,u16 psc)
{
 RCC->APB1ENR|=1<<3;
   TIM5->ARR=arr;
   TIM5->PSC=psc;	
   
  //TIM5->CCMR1|=1<<11;
	//TIM5->CR1|=0<<4;
	TIM5->CCMR1|=1<<0;  //CC1配置为输入，IC1映射到TI1
	TIM5->CCMR1|=0<<4;  //捕获不滤波，每次事件都捕获
	TIM5->CCMR1|=0<<10;
	
	//TIM5->CCER|=0<<3;
	TIM5->CCER|=0<<1; //上升沿捕获
	TIM5->CCER|=1<<0; //捕获通道使能
	
	TIM5->EGR=1<<0; //初始化计数器并生成寄存器更新事件（预分频器也将清零）
  TIM5->DIER|=1<<1; //使能通道一中断
	TIM5->DIER|=1<<0; //允许更新中断
	TIM5->CR1|=0x01;    	//使能计数器
}

void Init_NVIC()
{
	
	NVIC_InitTypeDef NVIC_InitStructure;
	#ifdef  VECT_TAB_RAM  							//向量表基地址选择

	  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);  	//将0x20000000地址作为向量表基地址(RAM)
	#else  

	  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0); //将0x08000000地址作为向量表基地址(FLASH)  
	#endif
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}



void TIM5_IRQn_deal(void)
{
 u16 tre;
 tre=TIM5->SR;

//if((TIM5CH1_CAPTURE_STA&0X80)==0) //如果捕获到高电平且还没捕获到下一个高电平，需要测高电平脉宽	
//{ 
	
  if(tre&0x01) //计数器已经溢出了
   {
	   
     if(TIM5CH1_CAPTURE_STA&0X40) //检查是否位高电平
	 {
       if((TIM5CH1_CAPTURE_STA&0X3F)==0x3F) //高电平太长（我觉得没必要）	  
	   {
	    TIM5CH1_CAPTURE_VAL=0xffffffff;
		   TIM5CH1_CAPTURE_STA|=0X80; //不要计高电平脉宽了，准备捕获下一个高电平  
	    }
	   else TIM5CH1_CAPTURE_STA++;
     	 }		 
     }
  
  if(tre&0x02) //捕获到了什么
  {
		   motor[0]->motormile2++;
			 TIM5CH1_CAPTURE_STA=0;
			 TIM5CH1_CAPTURE_VAL=TIM5->CCR1;
			 //TIM5->CNT=0;
			 TIM5CH1_CAPTURE_STA|=0X80; //标记捕获到了上升沿
		   
//	  falg=!falg;
		if((TIM5CH1_CAPTURE_STA)&(0X40)) //高电平捕获完毕
		{
			 TIM5CH1_CAPTURE_STA=0;
			 TIM5CH1_CAPTURE_VAL=TIM5->CCR1;
			 TIM5->CNT=0;
			 TIM5CH1_CAPTURE_STA|=0X80; //标记捕获到了上升沿
			 
//			 TIM5->CCER|=1<<0; //捕获通道使能
			 TIM5->DIER|=1<<0; //允许更新中断
			 TIM5->CCER&=~(1<<1); //准备捕获上升沿
	  }
		else
		{			
			TIM5CH1_CAPTURE_VAL=0; 
			TIM5CH1_CAPTURE_STA=0;
	
//			 TIM5->CR1&=~(1<<0); //使能计数器，防止误计数
			  TIM5->CNT=0; //将计数器清零
			 TIM5->DIER|=1<<0; //允许更新中断
			 TIM5->CCER|=1<<1; //准备捕获下降沿
			 TIM5->CR1|=0X01; //开启定时器使能，开始计数 	
			TIM5CH1_CAPTURE_STA|=0X40; //标记捕获到了上升沿
		 }	   	 
//	 //}
 }
  TIM5->SR=0;		
 }

 
 
