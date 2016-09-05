#ifndef INF_H
#define INF_H
#include "stm32f10x.h"

void init_infGPIO(void);
void Line_Follow(void);
uint8_t LineFlow_dec(void);
extern int16_t pwmr,pwml;
#endif


