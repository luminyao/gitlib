using System;

namespace WindowsFormsApplication3
{
    [Serializable]
    public class MotorParam
    {
        public MotorParam() { }
        public int TIM2Samp;
        public int brake_IO_domain;
        public int brake_IO_channel;
        public int dir_IO_domain;
        public int dir_IO_channel;
        public int encode_IO1_domain;
        public int encode_IO1_channel;
        public int encode_IO2_domain;
        public int encode_IO2_channel;
        public int PWM_IO_domain;
        public int PWM_IO_channel;
        public Int16 PWM_channel;
        public int TIM2PWM;
        public int motornum;
    }
    [Serializable]
    public class speed_PID
    {
        public Int16 encode_line;
        public Int16 Diameter;
        public Int16 Kp;
        public Int16 PI;
        public Int16 Kd;
        public Int16 integLimit;
        public Int16 targetSpeed;
        public int motornum;
    }
}

