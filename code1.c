#include "stm32f4xx.h"
//#include "usart.h"
#include "delay.h"
#iclude "sys.h"
#define ok 1

#define bool int
#define true 1
#define false 0

#define r_led(n) ((n==0)?(GPIOF->BSRRL|=1<<9):(GPIOF->BSRRH|=1<<9))
#define g_led(n) ((n==0)?(GPIOF->BSRRL|=1<<10):(GPIOF->BSRRH|=1<<10))

int set_sys_clock(void);
void Init_TIM5(u32 arr,u16 psc);
void Init_NVIC(void);
void delay_ms(u16 nms);

bool falg;
u8 TIM5CH1_CAPTURE_STA=0; //0x80标记捕获到低电平，0x40标记捕获到高电平
u32 TIM5CH1_CAPTURE_VAL;

void Init_led_gpio()
{
  RCC->AHB1ENR|=1<<5; //开时钟使能
  GPIOF->MODER|=1<<18;
  GPIOF->MODER|=1<<20;	
  GPIOF->OTYPER&=0<<9;
  GPIOF->OTYPER&=0<<10;
  GPIOF->OSPEEDR|=3<<18;
  GPIOF->OSPEEDR|=3<<20; 	//设置9、10号引脚输出速度为50MHz
  GPIOF->PUPDR|=2<<18;
  GPIOF->PUPDR|=2<<20;//设置9、10下拉。	
 }


 void Init_cap_gpio()
 {
  RCC->AHB1ENR|=1<<0;
  GPIOA->MODER|=2<<0;
  GPIOA->OTYPER|=0<<0;
  GPIOA->PUPDR|=2<<0;
  GPIOA->AFR[0]|=2<<0; 
  GPIOA->OSPEEDR|=3<<0;	 
 }
 
int main(void) 
{
	long long temp=0;
 set_sys_clock();
 Init_led_gpio();
 Init_cap_gpio();
 uart_init(84,115200);
 delay_init(8);
 Init_TIM5(0xffffffff,83);
 Init_NVIC();
 
 	r_led(0);
	g_led(0);
	
  while(1)
  {
	if((TIM5CH1_CAPTURE_STA)&(0X80)) //成功捕获了一个高电平脉宽
	 {
	  temp=TIM5CH1_CAPTURE_STA&0x3f;
		 temp*=0xffffffff;
		temp+=TIM5CH1_CAPTURE_VAL;
		temp/=1000000;
		TIM5CH1_CAPTURE_STA=0;
	   }
 	printf("%lld\n",temp);
	  
	  delay_ms(9000);
	   if(falg) r_led(1);
	   else r_led(0);
     //if(TIM5CH1_CAPTURE_STA&0X40) ;	   
    	   
    }
   
}

int set_sys_clock(void)
{
	u32 retry=0;
	u8 status=0;
    RCC->CR|=1<<16; //使能HSE时钟
	while((RCC->CR&(1<<17)==0)&&(retry < 0X1FFFFFFF)) retry++; //等待HSE起振，如果等待太久那么就返回错误码。
	if(retry == 0X1FFFFFFF) status = 1; 
	else
	{
	 //RCC->APB1ENR|=1<<28;  //使能允许进入待机模式
		PWR->CR|=3<<14;	  //平横功耗和性能
		RCC->CFGR|=(0<<4)|(5<<10)|(4<<13);
		RCC->CR&=0<<24; //关闭主PLL
		RCC->PLLCFGR|=1<<22; //设定PLL时钟源（设定之前要保证PLL处于关闭状态）
		RCC->CR|=1<<24; //打开主PLL
		while((RCC->CR|(0<<25))==0); //等待主PLL启动完毕
		FLASH->ACR|=(1<<8)|(1<<9)|(1<<10); 
		FLASH->ACR|=5<<0; //等待五个系统周期，保证FLASH设置完成
		RCC->CFGR&=~(3<<0); //重新设定之前小心的清零
		RCC->CFGR|=2<<0; //选择PLL作为系统时钟
	    while((RCC->CFGR&(2<<2))!=(2<<2)); //等待系统时钟切换完毕		
	 } 
	return status;
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



void TIM5_IRQHandler(void)
{
 u16 tre;
  tre=TIM5->SR;
  
if((TIM5CH1_CAPTURE_STA&0X80)==0) //如果捕获到高电平且还没捕获到下一个高电平，需要测高电平脉宽	
{ 
	
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
	  falg=!falg;
		if((TIM5CH1_CAPTURE_STA)&(0X40)) //高电平捕获完毕
		{
			 g_led(0);
			 TIM5CH1_CAPTURE_STA=0;
			 TIM5CH1_CAPTURE_VAL=TIM5->CCR1;
			 TIM5CH1_CAPTURE_STA|=0X80; //标记捕获到了低电平电平
			 
			 TIM5->CCER|=1<<0; //捕获通道使能
			 TIM5->DIER|=1<<0; //允许更新中断
			 TIM5->CCER&=~(1<<1); //准备捕获上升沿
	    }
		else
		{			
			TIM5CH1_CAPTURE_VAL=0; 
			TIM5CH1_CAPTURE_STA=0;
			
			

			 TIM5->CR1&=~(1<<0); //使能计数器，防止误计数
			  TIM5->CNT=0; //将计数器清零
			  TIM5->DIER|=1<<0; //允许更新中断
			 TIM5->CCER|=1<<1; //准备捕获下降沿
			 TIM5->CR1|=0X01; //开启定时器使能，开始计数 
			
			TIM5CH1_CAPTURE_STA|=0X40; //标记捕获到了高电平
		 }	   	 
	 }
 }
  TIM5->SR=0;		
 }

 
 
