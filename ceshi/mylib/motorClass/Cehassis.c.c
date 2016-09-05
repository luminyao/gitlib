#include "motorClass.h"

void Correct_Airth(MovBaseClass *Mov,float Linear,float Angular);
void Ring_Airth(MovBaseClass *Mov,float V1,float V2);

MovBaseClass *new_MovBase(MovBaseParam *_MovBaseParam)
{
	MovBaseClass *m;
	m = (MovBaseClass *)malloc(sizeof(MovBaseClass));
	m->m[0] = _MovBaseParam->m[0];
	m->m[1] = _MovBaseParam->m[1];
  
	m->WheelBetw = _MovBaseParam->WheelBetw;
	
	m->MovBaseContr = _MovBaseContr;
//	free(_MovBaseParam);
//	_MovBaseParam = NULL;
	return m;
}

static void _MovBaseContr(MovBaseClass *MovBase,float Linear,float Angular)
{	
		u8 n1,n2;
		static float VR,VL;
		Correct_Airth(MovBase,Linear,Angular);
	  VR = Linear + ( Angular * MovBase->WheelBetw )/20000.0 + MovBase->Lin_Compen;
		VL = Linear - ( Angular * MovBase->WheelBetw )/20000.0;
		Ring_Airth(MovBase,VR,VL);
		if ( VR > 0 )
		{
			n1 = 1;
		}else if ( VR < 0 )
		{
			n1 = 0;
			VR = (-1)*VR;
		}else if ( VR == 0 )
		{
			n2 = 1;
		}			
		MovBase->m[0]->motor_MovControl(MovBase->m[0],n1,n2);	
		if ( VL> 0 )
		{
			n1 = 1;
		}else if ( VL < 0 )
		{
			n1 = 0;
			VL = (-1)*VL;
		}else if ( VL == 0 )
		{
			n2 = 1;
		}	
		MovBase->m[1]->motor_MovControl(MovBase->m[1],n1,n2);		
		MovBase->m[0]->Speed_Contor(MovBase->m[0],VR,MovBase->m[0]->Kp,MovBase->m[0]->Ki,MovBase->m[0]->Kd);
		MovBase->m[1]->Speed_Contor(MovBase->m[1],VL,MovBase->m[1]->Kp,MovBase->m[1]->Ki,MovBase->m[1]->Kd);
		Get_SpeedandOdometry(MovBase);
}


void Get_SpeedandOdometry(MovBaseClass *Mov)
{
	float	offset_motor1 = 0, offset_motor2 = 0;
	static float pulse_motor1_last = 0,pulse_motor2_last = 0;
	
	Mov->Liner = (Mov->m[0]->fback_R_speed + Mov->m[1]->fback_R_speed) / 2;
	Mov->Angular = (Mov->m[0]->fback_R_speed - Mov->m[1]->fback_R_speed) / Mov->WheelBetw;
	
	offset_motor1 = (float) (((Mov->m[0]->motormile - pulse_motor1_last) / Mov->m[0]->Wheel_pulse_circle) * Mov->m[0]->Wheel_cir);
	offset_motor2 = (float) (((Mov->m[1]->motormile - pulse_motor2_last) / Mov->m[0]->Wheel_pulse_circle) * Mov->m[0]->Wheel_cir);
	
	pulse_motor1_last = Mov->m[0]->motormile - 0.5;
	pulse_motor2_last = Mov->m[1]->motormile - 0.5;
	if ( offset_motor1 == 0.5 )
	{
		offset_motor1 = 0;
	}
	if ( offset_motor2 == 0.5 )
	{
		offset_motor2 = 0;
	}
	CalOdometer(Mov,offset_motor1,offset_motor2);
}

void CalOdometer(MovBaseClass *Mov,float offset_motor1,float offset_motor2)
{
	float	Det_D;                                          /* 间隔时间内的扇形的弧长 */
	float	Det_th;                                         /* 间隔时间内的扇形的弧度(单位：rad) */

	if ( offset_motor2 != offset_motor1 )
	{
		Det_th	= ((offset_motor2 - offset_motor1)) / Mov->WheelBetw;
		Det_D	= ((offset_motor2 + offset_motor1)) / 2;  			/*以弧长来近似弦长*/
	}
	else		/* 防止因除数为0发生错误 */
	{
		Det_th	= 0.0;
		Det_D	= offset_motor1;
	}
	
	Mov->Theta_Odometry += Det_th;                                                                     	/* 使用偏角累加来获得偏航角 */
	Mov->X_Odometry += cos(Mov->Theta_Odometry) * Det_D;
	Mov->Y_Odometry += sin(Mov->Theta_Odometry) * Det_D;
	
	if(Mov->Theta_Odometry < 0)	Mov->Theta_Odometry += 2 * 3.1415926;
	else if(Mov->Theta_Odometry >= 2 * 3.1415926)	Mov->Theta_Odometry -= 2 * 3.1415926;
	
	Mov->Theta += Det_th; 
	if(Mov->Theta < 0)	Mov->Theta += 2 * (3.1415926);
	else if(Mov->Theta >= 2 * (3.1415926))	Mov->Theta -= 2 * (3.1415926);

	Mov->Y_Refer += sin(Mov->Theta) * Det_D;																																/* 使用偏角累加来获得偏航角 */
}


void Correct_Airth(MovBaseClass *Mov,float Linear,float Angular)   //运动校准算法
{
//	static int16_t	offset_motor1 = 0, offset_motor2 = 0;
//	static int16_t pulse_motor1_last = 0,pulse_motor2_last = 0,e;
	if ( Angular != 0 )
	{
		Mov->Theta = 0;
//		offset_motor1 = 0;
//		offset_motor2 = 0
	}
//	offset_motor1 += ( Mov->m[0]->motormile - pulse_motor1_last );
//	offset_motor2 += ( Mov->m[1]->motormile - pulse_motor2_last );
//	
//	pulse_motor1_last = Mov->m[0]->motormile;
//	pulse_motor2_last = Mov->m[1]->motormile;
	
//	e = offset_motor1 - offset_motor1;
	if ( Linear != 0 )
	{
		Mov->Lin_Compen = Mov->Y_Refer * 1;
	}
	else
	{
		Mov->Lin_Compen = 0;
		Mov->Y_Refer = 0;
	}
	if ( Mov->Lin_Compen > 0.08 ) Mov->Lin_Compen = 0.08;
	if ( Mov->Lin_Compen < -0.08 ) Mov->Lin_Compen = -0.08;
}


void Ring_Airth(MovBaseClass *Mov,float V1,float V2)
{ 
	static float m,X,Y,k , VK1,VK2;
	int8_t dir = 0;
	if (abs(V1) > abs(V2))
		dir = -1;
	if (abs(V1) < abs(V2))
		dir = 1;
	if ( (V1 != V2)&&((V1 != VK1)||(V2 != VK2)))  //转弯动作
	{
		m = ((abs(V2) + abs(V1)) * Mov->WheelBetw)/2*abs(V1 - V2);
		X = Mov->X_Odometry + m * sin(Mov->Theta_Odometry);
		Y = Mov->Y_Odometry + m * cos(Mov->Theta_Odometry);
	}
	k = (Mov->X_Odometry - X)*(Mov->X_Odometry - X) + (Mov->Y_Odometry - Y) * (Mov->Y_Odometry - Y);
	if ( V1 == V2 )
	{
		k = m;
	}
	VK1 = V1;
	VK2 = V2;
	
	Mov->Lin_Compen = (dir * (sqrt(k) - m)) * 0.5;
	
	if ( Mov->Lin_Compen > 0.08 ) Mov->Lin_Compen = 0.08;
	if ( Mov->Lin_Compen < -0.08 ) Mov->Lin_Compen = -0.08;
}
