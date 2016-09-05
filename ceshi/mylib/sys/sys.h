#ifndef SYS_H
#define SYS_H
#include <motorClass.h>
#include "Mydelay.h"
#include "usart.h"
#include <stdio.h>
#include <stdlib.h>

#define bool int
#define true 1
#define false 0

#define r_led(n) ((n==0)?(GPIOF->BSRRL|=1<<9):(GPIOF->BSRRH|=1<<9))
#define g_led(n) ((n==0)?(GPIOF->BSRRL|=1<<10):(GPIOF->BSRRH|=1<<10))

int set_sys_clock(void);
void Init_TIM5(u32 arr,u16 psc);
void Init_NVIC(void);
void Init_cap_gpio(void);


void send_concom3(int16_t m);
void send_concom5(void);

//typedef struct 
//{
// float linear;
// float Angular; 
//}SETPara;

#endif
