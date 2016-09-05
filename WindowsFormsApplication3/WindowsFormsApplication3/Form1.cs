using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using System.Threading;
using System.Windows.Forms.DataVisualization.Charting;

namespace WindowsFormsApplication3
{
    public partial class Form1 : Form
    {
        BackgroundWorker PortCheckBW = new BackgroundWorker();
        BackgroundWorker GetProtBufBW = new BackgroundWorker();
        BackgroundWorker DrwBW = new BackgroundWorker();
        SerialPort SerialPort = new SerialPort();
        StringBuilder builder = new StringBuilder();
        MotorParam MotorParam;
        
        string[] TIMx = new string[] { "TIM1", "TIM2", "TIM3", "TIM4", "TIM5", "TIM6", "TIM7", "TIM8" };
        string[] GPIOx = new string[] { "GPIOA", "GPIOB", "GPIOC", "GPIOD", "GPIOE", "GPIOF", "GPIOG" };
        string[] channel = new string[] { "0","1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14","15","all" };
        string[] MotorNum = new string[] { "0","1","2","3"};
        System.Timers.Timer t = new System.Timers.Timer(1);

        #region  全局变量
        float m;
        bool ComFlag = false;
        float timee = 0, risetime = 0;
        bool IsDraw = true;
        #endregion

        #region 界面事件
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            PortParam();
            motormodel();
            SetMotorParam();
            PortCheckBW.DoWork += new DoWorkEventHandler(PortParam_name);
            PortCheckBW.RunWorkerAsync();
        }

        void PortParam_name(object sender, EventArgs e)
        {
            int i = 0, j = 0;
            while (true)
            {
                Thread.Sleep(100);
                this.Invoke((EventHandler)(delegate
                {
                    foreach (var item in SerialPort.GetPortNames())
                    {
                        i++;
                        if (!(SelectItemBoxPortName.Items.Contains(item)))
                        {
                            SelectItemBoxPortName.Items.Add(item);
                            MotorSelectItemBox.Items.Add(item);
                        }
                    }
                    foreach (var item in SelectItemBoxPortName.Items)
                    {
                        j++;
                    }
                    if (i != j)
                    {
                        i = 0;
                        j = 0;
                        SelectItemBoxPortName.Items.Clear();
                        MotorSelectItemBox.Items.Clear();
                    }
                }));
            }
        }

        void GetSerialPortBuf(object sender, EventArgs e)
        {
            while (ComFlag)
            {
                Thread.Sleep(100);
                int n = SerialPort.BytesToRead;
                byte[] buf = new byte[n];
                String str1 = string.Empty;
                str1.Remove(0, str1.Length);
                SerialPort.Read(buf, 0, n);
                builder.Remove(0, builder.Length);
                String str = Encoding.UTF8.GetString(buf);
                if (radioButton1.Checked)
                {
                    {
                        this.Invoke((EventHandler)(delegate
                        {
                            foreach (byte b in buf)
                            {   //b.ToString()将当前byte对象的值转换为它的等效字符串表示
                                builder.Append(b.ToString("X2") + " ");  //"X2"为C#中的字符串格式控制符。X为 十六进制，2为位数控制

                            }
                            this.ShowText.AppendText(builder.ToString());
                        }));
                    }

                }

                if (radioButton2.Checked)
                {
                    this.Invoke(new Action(() =>
                    {
                        this.ShowText.AppendText(str);
                    }));
                }
            }
        }

        void Drw(object sender, EventArgs e)
        {
            //chart1.ChartAreas[0].AxisX.ScaleView.Size = 10;
            //chart1.ChartAreas[0].AxisX.Maximum = 100;
            while (IsDraw)
            {
                Thread.Sleep(100);
                int n = SerialPort.BytesToRead;
                byte[] buf = new byte[n];
                SerialPort.Read(buf, 0, n);
                unPack(buf, n);
                this.Invoke((EventHandler)(delegate
                {
                    chart1.ChartAreas[0].AxisX.ScrollBar.IsPositionedInside = true;
                    chart1.ChartAreas[0].AxisX.ScrollBar.Enabled = true;
                    this.chart1.Series[0].ChartType = SeriesChartType.Spline;
                    chart1.ChartAreas[0].AxisY.Maximum = 1;
                    chart1.ChartAreas[0].AxisX.Maximum = 50;
                    chart1.ChartAreas[0].AxisX.Interval = 1;
                    chart1.Series[0].Points.AddXY(timee, m);
                    this.textBox9.AppendText(Convert.ToString(m) + "  ");
                }));

            }

        }

        private void ConnectSerialPort(object sender, EventArgs e)
        {
            try
            {
                SerialPort.PortName = SelectItemBoxPortName.SelectedItem.ToString();
                SerialPort.BaudRate = Convert.ToInt32(SelectItemBoxBaudRate.SelectedItem.ToString());
                SerialPort.Parity = SelectItemBoxParity.SelectedItem.ToString() == "0" ? Parity.None : (SelectItemBoxParity.SelectedItem.ToString() == "1") ? Parity.Odd : Parity.Even;
                SerialPort.DataBits = Convert.ToInt16(SetectItemBoxDataBits.SelectedItem.ToString());
                SerialPort.StopBits = SelectItemBoxStopBits.SelectedItem.ToString() == "1" ? StopBits.One : (SelectItemBoxStopBits.SelectedItem.ToString() == "1.5" ? StopBits.OnePointFive : (SelectItemBoxStopBits.SelectedValue.ToString() == "2" ? StopBits.Two : StopBits.None));
                SerialPort.NewLine = "UU";
                SerialPort.Open();
                ComFlag = true;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
            GetProtBufBW.DoWork += new DoWorkEventHandler(GetSerialPortBuf);
            GetProtBufBW.RunWorkerAsync();
        }

        private void SerialPortSend(object sender, EventArgs e)
        {
            SendMessage();
        }

        private void SetMotorConfig(object sender, EventArgs e)
        {
            int i = 0;
            MotorParam = new MotorParam() { };

            foreach (string b in MotorNum)
            {
                if (b == comboBox8.SelectedItem.ToString())
                {
                    MotorParam.motornum = i;
                    i = 0;
                    break;
                }
                i++;
            }
            foreach (string b in TIMx)
            {
                if (b == comboBox7.SelectedItem.ToString())
                {
                    MotorParam.TIM2Samp = i;
                    i = 0;
                    break;
                }
                i++;
            }
            foreach (string b in TIMx)
            {
                if (b == comboBox6.SelectedItem.ToString())
                {
                    MotorParam.TIM2PWM = i;
                    i = 0;
                    break;
                }
                i++;
            }
            foreach (string b in GPIOx)
            {

                if (b == comboBox36.SelectedItem.ToString())
                {
                    MotorParam.dir_IO_domain = i;
                    i = 0;
                    break;
                }
                i++;
            }
            foreach (string b in GPIOx)
            {

                if (b == comboBox32.SelectedItem.ToString())
                {
                    MotorParam.brake_IO_domain = i;
                    i = 0;
                    break;
                }
                i++;
            }
            foreach (string b in GPIOx)
            {

                if (b == comboBox24.SelectedItem.ToString())
                {
                    MotorParam.encode_IO1_domain = i;
                    i = 0;
                    break;
                }
                i++;
            }
            foreach (string b in GPIOx)
            {

                if (b == comboBox20.SelectedItem.ToString())
                {
                    MotorParam.encode_IO2_domain = i;
                    i = 0;
                    break;
                }
                i++;
            }
            foreach (string b in GPIOx)
            {

                if (b == comboBox11.SelectedItem.ToString())
                {
                    MotorParam.PWM_IO_domain = i;
                    i = 0;
                    break;
                }
                i++;
            }

            foreach (string b in channel)
            {

                if (b == comboBox35.SelectedItem.ToString())
                {
                    MotorParam.dir_IO_channel = i;
                    i = 0;
                    break;
                }
                i++;
            }
            foreach (string b in channel)
            {

                if (b == comboBox31.SelectedItem.ToString())
                {
                    MotorParam.brake_IO_channel = i;
                    i = 0;
                    break;
                }
                i++;
            }
            foreach (string b in channel)
            {

                if (b == comboBox23.SelectedItem.ToString())
                {
                    MotorParam.encode_IO1_channel = i;
                    i = 0;
                    break;
                }
                i++;
            }
            foreach (string b in channel)
            {
                if (b == comboBox19.SelectedItem.ToString())
                {
                    MotorParam.encode_IO2_channel = i;
                    i = 0;
                    break;
                }
                i++;
            }
            foreach (string b in channel)
            {

                if (b == comboBox12.SelectedItem.ToString())
                {
                    MotorParam.PWM_IO_channel = i;
                    i = 0;
                    break;
                }
                i++;
            }
            MotorParam.PWM_channel = Int16.Parse(comboBox15.Text);
            List<byte> btList = new List<byte>
            {
                0x0f,0x00,0x00,0x24,0x00,0x00,0x00,
                //(byte)(MotorParam.encode_line& 0xff),  //5
                //(byte)((MotorParam.encode_line>>8)& 0xff),  //6

                (byte)(MotorParam.TIM2Samp&0xff), //7

                (byte)(MotorParam.TIM2PWM&0xff),  //8

                (byte)(MotorParam.dir_IO_domain& 0xff),  //9

                (byte)(MotorParam.brake_IO_domain&0xff),  //10

                (byte)(MotorParam.encode_IO1_domain&0xff), //11

                (byte)(MotorParam.encode_IO2_domain&0xff),  //12

                (byte)(MotorParam.PWM_IO_domain&0xff),   //13
                 
                (byte)(MotorParam.dir_IO_channel& 0xff),  //14

                (byte)(MotorParam.brake_IO_channel&0xff),  //15

                (byte)(MotorParam.encode_IO1_channel&0xff),  //16

                (byte)(MotorParam.encode_IO2_channel&0xff),   //17

                (byte)(MotorParam.PWM_IO_channel&0xff),   //18

                (byte)(MotorParam.motornum&0xff),    //19

                (byte)(MotorParam.PWM_channel&0xff),  //20
            };
            PackageList(btList);
            SerialPort.Write(btList.ToArray(), 0, btList.Count);
        }



        private void SetContorlParam(object sender, EventArgs e)
        {
            int i = 0;
            speed_PID speedPID = new speed_PID() { };
            speedPID.encode_line = Int16.Parse(textBox1.Text);
            speedPID.Diameter = Int16.Parse(textBox21.Text);
            speedPID.Kp = Int16.Parse(textBox22.Text);
            speedPID.PI = Int16.Parse(textBox13.Text);
            speedPID.Kd = Int16.Parse(textBox14.Text);
            speedPID.integLimit = Int16.Parse(textBox27.Text);
            speedPID.targetSpeed = Int16.Parse(textBox18.Text);
            foreach (string b in MotorNum)
            {
                if (b == comboBox10.SelectedItem.ToString())
                {
                    speedPID.motornum = i;
                    i = 0;
                    break;
                }
                i++;
            }
            List<byte> btList = new List<byte>
            {
                0x0f,0x00,0x00,0x25,0x00,
                (byte)(speedPID.Kp),
                (byte)(speedPID.Kp>>8),
                (byte)(speedPID.PI),
                (byte)(speedPID.PI>>8),
                (byte)(speedPID.Kd),
                (byte)(speedPID.Kd>>8),
                (byte)(speedPID.integLimit),
                (byte)(speedPID.integLimit>>8),
                (byte)(speedPID.targetSpeed),//13
                (byte)(speedPID.targetSpeed>>8),
                (byte)(speedPID.motornum&0xff),
                (byte)(speedPID.encode_line),//16
                (byte)(speedPID.encode_line>>8),
                (byte)(speedPID.Diameter),//18
                (byte)(speedPID.Diameter>>8),
            };
            PackageList(btList);
            SerialPort.Write(btList.ToArray(), 0, btList.Count);
        }

        private void LeadMotorConfig(object sender, EventArgs e)
        {
            foreach (string b in MotorModelBox.Items)
            {
                if (MotorModelBox.SelectedItem.ToString() == "lungu1")
                {
                    comboBox15.SelectedIndex = 0;
                    comboBox6.SelectedIndex = 2;
                    comboBox7.SelectedIndex = 3;
                    comboBox36.SelectedIndex = 5;
                    comboBox32.SelectedIndex = 5;
                    comboBox24.SelectedIndex = 1;
                    comboBox20.SelectedIndex = 1;
                    comboBox11.SelectedIndex = 1;
                    comboBox35.SelectedIndex = 2;
                    comboBox31.SelectedIndex = 3;
                    comboBox23.SelectedIndex = 6;
                    comboBox19.SelectedIndex = 6;
                    comboBox12.SelectedIndex = 1;
                    //comboBox8.SelectedIndex = 0;
                    //comboBox10.SelectedIndex = 0;
                }
                if (MotorModelBox.SelectedItem.ToString() == "lungu2")
                {
                    comboBox15.SelectedIndex = 1;
                    comboBox6.SelectedIndex = 2;
                    comboBox7.SelectedIndex = 1;
                    comboBox36.SelectedIndex = 5;
                    comboBox32.SelectedIndex = 5;
                    comboBox24.SelectedIndex = 0;
                    comboBox20.SelectedIndex = 1;
                    comboBox11.SelectedIndex = 1;
                    comboBox35.SelectedIndex = 4;
                    comboBox31.SelectedIndex = 5;
                    comboBox23.SelectedIndex = 0;
                    comboBox19.SelectedIndex = 6;
                    comboBox12.SelectedIndex = 0;
                }
            }
        }

        private void EnDrw(object sender, EventArgs e)
        {
            //chart1.Series.Clear();
            IsDraw = true;
            t.Elapsed += new System.Timers.ElapsedEventHandler(theout);
            t.AutoReset = true;
            t.Enabled = true;
            DrwBW.DoWork += new DoWorkEventHandler(Drw);
            DrwBW.RunWorkerAsync();
        }

        public void theout(object source, System.Timers.ElapsedEventArgs e)
        {
            timee += (float)0.01;
            if ((m > 0 && m < 0.4) || (m > 0.6))
            {
                risetime += 1;
            }
            if (m == 0) risetime = 0;
        }

        private void ConnectMotorConfigSerialPort(object sender, EventArgs e)
        {
            try
            {
                SerialPort.PortName = MotorSelectItemBox.SelectedItem.ToString();
                SerialPort.BaudRate = 115200;
                SerialPort.Parity = Parity.None;
                SerialPort.DataBits = 8;
                SerialPort.StopBits = StopBits.One;
                SerialPort.NewLine = "UU";
                SerialPort.Open();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void CloseDrw(object sender, EventArgs e)
        {
            IsDraw = false;
        }
        #endregion

        #region 方法
        void PortParam()
        {
            SelectItemBoxBaudRate.Items.Add("1200");
            SelectItemBoxBaudRate.Items.Add("2400");
            SelectItemBoxBaudRate.Items.Add("4800");
            SelectItemBoxBaudRate.Items.Add("9600");
            SelectItemBoxBaudRate.Items.Add("19200");
            SelectItemBoxBaudRate.Items.Add("38400");
            SelectItemBoxBaudRate.Items.Add("115200");
            SetectItemBoxDataBits.Items.Add("5");//数据位
            SetectItemBoxDataBits.Items.Add("6");
            SetectItemBoxDataBits.Items.Add("7");
            SetectItemBoxDataBits.Items.Add("8");
            SelectItemBoxParity.Items.Add("0");
            SelectItemBoxParity.Items.Add("1");
            SelectItemBoxParity.Items.Add("2");
            SelectItemBoxStopBits.Items.Add("1");
            SelectItemBoxStopBits.Items.Add("1.5");
            SelectItemBoxStopBits.Items.Add("2");

            SelectItemBoxBaudRate.SelectedIndex = 6;
            SetectItemBoxDataBits.SelectedIndex = 3;
            SelectItemBoxParity.SelectedIndex = 0;
            SelectItemBoxStopBits.SelectedIndex = 0;
        }

        void SetMotorParam()
        {
            foreach (string a in TIMx)
            {
                comboBox7.Items.Add(a);
                comboBox6.Items.Add(a);
            }
            comboBox7.SelectedValue = null;
            foreach (string a in GPIOx)
            {
                comboBox36.Items.Add(a);
                comboBox32.Items.Add(a);
                comboBox24.Items.Add(a);
                comboBox20.Items.Add(a);
                comboBox11.Items.Add(a);
            }
            foreach (string a in channel)
            {
                comboBox35.Items.Add(a);
                comboBox31.Items.Add(a);
                comboBox23.Items.Add(a);
                comboBox19.Items.Add(a);
                comboBox12.Items.Add(a);
            }
            foreach (string a in MotorNum)
            {
                comboBox8.Items.Add(a);
                comboBox10.Items.Add(a);
            }

            comboBox15.Items.Add("1");
            comboBox15.Items.Add("2");
            comboBox15.Items.Add("3");
            comboBox15.Items.Add("4");

            //comboBox15.SelectedIndex = 0;
            //comboBox6.SelectedIndex = 0;
            //comboBox7.SelectedIndex = 0;
            //comboBox36.SelectedIndex = 0;
            //comboBox32.SelectedIndex = 0;
            //comboBox24.SelectedIndex = 0;
            //comboBox20.SelectedIndex = 0;
            //comboBox11.SelectedIndex = 0;
            //comboBox35.SelectedIndex = 0;
            //comboBox31.SelectedIndex = 0;
            //comboBox23.SelectedIndex = 0;
            //comboBox19.SelectedIndex = 0;
            //comboBox12.SelectedIndex = 0;
            //comboBox8.SelectedIndex = 0;
            //comboBox10.SelectedIndex = 0;
        }

        void motormodel()
        {
            MotorModelBox.Items.Add("lungu1");
            MotorModelBox.Items.Add("lungu2");
        }
        
        void SendMessage()
        {
            try
            {
                if (radioButton4.Checked)
                {
                    string a = textBox3.Text.Trim();
                    string[] b = a.Split(' ');  //将字符串中的元素按‘ ’（空格）分离出来。
                    byte[] sendmesssge = new byte[b.Length];
                    int s = b.Length;
                    for (int i = 0; i < s; i++)
                    {
                        sendmesssge[i] = Convert.ToByte(b[i]);
                    }
                    SerialPort.Write(sendmesssge, 0, s);
                }
                else
                {
                    string sendm = textBox3.Text.Trim();
                    SerialPort.Write(sendm);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        void PackageList(List<byte> btList)
        {
            int checkSum = 0;
            foreach (byte item in btList)
            {
                checkSum += item;
            }
            checkSum &= 0xff;
            btList.Add((byte)checkSum);
            List<int> CTRLIndexList = new List<int>();
            for (int i = 0; i < btList.Count; i++)
            {
                if (btList[i] == 0x55 || btList[i] == 0xA5 || btList[i] == 0xAA)
                {
                    CTRLIndexList.Add(i);
                }
            }

            for (int i = 0; i < CTRLIndexList.Count; i++)
            {
                btList.Insert(CTRLIndexList[i] + i, 0xa5);
            }
            btList.Insert(0, 0xaa);
            btList.Insert(0, 0xaa);
            btList.Add(0x55);
            btList.Add(0x55);
        }

        void unPack(byte[] Dlist, int n)
        {
            int k = 0, i = 0 ;
            byte[] shuju = new byte[4];
            for (k = 0; k < n - 1; k++)
            {
                if ((Dlist[k] == 0xAA) && (Dlist[k + 1] == 0xAA))
                    if (k + 10 < n)
                    {
                        //for (i = 0; i < 4; i++)
                        //{
                        //    shuju[i] = Dlist[k + 7 +i];
                        //}
                        shuju[0] = Dlist[k + 7];
                        shuju[1] = Dlist[k + 8];
                        shuju[2] = Dlist[k + 9];
                        shuju[3] = Dlist[k + 10];
                        m = BitConverter.ToSingle(shuju,0);
                    }
                // m = ((float)(Dlist[k + 7] | Dlist[k + 8] << 8)); // / 1000;
               
            }
        }
        #endregion

        #region  暂时未用的控件
        private void groupBox7_Enter(object sender, EventArgs e)
        {

        }
        private void groupBox2_Enter(object sender, EventArgs e)
        {

        }
        private void button9_Click(object sender, EventArgs e)
        {

        }
        private void textBox25_TextChanged(object sender, EventArgs e)
        {

        }
        private void textBox2_TextChanged(object sender, EventArgs e)
        {

        }

        private void CloseSerialBtn_Click(object sender, EventArgs e)
        {
            try
            {
                if (SerialPort.IsOpen == true)
                {
                    ComFlag = false;
                    SerialPort.Close();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void comboBox7_SelectedIndexChanged(object sender, EventArgs e)
        {

        }
        #endregion 

    }
}
