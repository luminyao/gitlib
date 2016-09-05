#include "stm32f10x.h"
#include "sys.h"

extern  u8 TIM5CH1_CAPTURE_STA; //0x80��ǲ��񵽵͵�ƽ��0x40��ǲ��񵽸ߵ�ƽ
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
	TIM5->CCMR1|=1<<0;  //CC1����Ϊ���룬IC1ӳ�䵽TI1
	TIM5->CCMR1|=0<<4;  //�����˲���ÿ���¼�������
	TIM5->CCMR1|=0<<10;
	
	//TIM5->CCER|=0<<3;
	TIM5->CCER|=0<<1; //�����ز���
	TIM5->CCER|=1<<0; //����ͨ��ʹ��
	
	TIM5->EGR=1<<0; //��ʼ�������������ɼĴ��������¼���Ԥ��Ƶ��Ҳ�����㣩
  TIM5->DIER|=1<<1; //ʹ��ͨ��һ�ж�
	TIM5->DIER|=1<<0; //��������ж�
	TIM5->CR1|=0x01;    	//ʹ�ܼ�����
}

void Init_NVIC()
{
	
	NVIC_InitTypeDef NVIC_InitStructure;
	#ifdef  VECT_TAB_RAM  							//���������ַѡ��

	  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);  	//��0x20000000��ַ��Ϊ���������ַ(RAM)
	#else  

	  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0); //��0x08000000��ַ��Ϊ���������ַ(FLASH)  
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

//if((TIM5CH1_CAPTURE_STA&0X80)==0) //������񵽸ߵ�ƽ�һ�û������һ���ߵ�ƽ����Ҫ��ߵ�ƽ����	
//{ 
	
  if(tre&0x01) //�������Ѿ������
   {
	   
     if(TIM5CH1_CAPTURE_STA&0X40) //����Ƿ�λ�ߵ�ƽ
	 {
       if((TIM5CH1_CAPTURE_STA&0X3F)==0x3F) //�ߵ�ƽ̫�����Ҿ���û��Ҫ��	  
	   {
	    TIM5CH1_CAPTURE_VAL=0xffffffff;
		   TIM5CH1_CAPTURE_STA|=0X80; //��Ҫ�Ƹߵ�ƽ�����ˣ�׼��������һ���ߵ�ƽ  
	    }
	   else TIM5CH1_CAPTURE_STA++;
     	 }		 
     }
  
  if(tre&0x02) //������ʲô
  {
		   motor[0]->motormile2++;
			 TIM5CH1_CAPTURE_STA=0;
			 TIM5CH1_CAPTURE_VAL=TIM5->CCR1;
			 //TIM5->CNT=0;
			 TIM5CH1_CAPTURE_STA|=0X80; //��ǲ�����������
		   
//	  falg=!falg;
		if((TIM5CH1_CAPTURE_STA)&(0X40)) //�ߵ�ƽ�������
		{
			 TIM5CH1_CAPTURE_STA=0;
			 TIM5CH1_CAPTURE_VAL=TIM5->CCR1;
			 TIM5->CNT=0;
			 TIM5CH1_CAPTURE_STA|=0X80; //��ǲ�����������
			 
//			 TIM5->CCER|=1<<0; //����ͨ��ʹ��
			 TIM5->DIER|=1<<0; //��������ж�
			 TIM5->CCER&=~(1<<1); //׼������������
	  }
		else
		{			
			TIM5CH1_CAPTURE_VAL=0; 
			TIM5CH1_CAPTURE_STA=0;
	
//			 TIM5->CR1&=~(1<<0); //ʹ�ܼ���������ֹ�����
			  TIM5->CNT=0; //������������
			 TIM5->DIER|=1<<0; //��������ж�
			 TIM5->CCER|=1<<1; //׼�������½���
			 TIM5->CR1|=0X01; //������ʱ��ʹ�ܣ���ʼ���� 	
			TIM5CH1_CAPTURE_STA|=0X40; //��ǲ�����������
		 }	   	 
//	 //}
 }
  TIM5->SR=0;		
 }

 
 
