#ifndef MINIIMU_H
#define MINIIMU_H

#include "stm32f10x.h"

void ReadDataPacket(void);
void IMU_Data_Explain(void);
void InitIMU(void);
void InitIMUUSART(void);
void InitIMUUSARTGPIO(void);
void InitIMUUSART2_NVIC(void);



extern u16 IMURXData;
extern u16 IMURXDataBuf[11];
extern u16 Z_Angle,X_Angle,Y_Angle;
extern u8 IMUcounter;
extern u16 Zrotate_Ang;
extern u8 RX_Data_flg;

#endif
