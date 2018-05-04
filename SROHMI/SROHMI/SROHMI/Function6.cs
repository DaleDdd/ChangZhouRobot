using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using CCWin;
using System.Threading;

namespace SROHMI
{
   
    public partial class Function6 : Form
    {
        private bool go_flag = false; 
        public send send_buf;
        public sendai send_aiui;
        public wifi wifiset;
        byte[] Send_Buf = new byte[6];
        public byte Speed_Set { get; set; } //速度设定值
        public stop close_tim,open_tim;
        public delegate void Opengif();
        StationSet stat;
        StationTime sttim;
        CrossSet cross;
        Send_commond send_commond;
        public Function6()
        {            
            InitializeComponent();
        }
        public Function6(send s,stop s1,stop s2,bool fl,sendai a1,wifi w1)
        {
            send_buf = s;
            send_aiui = a1;
            close_tim = s1;open_tim = s2;
            wifiset = w1;
            InitializeComponent();
            go_flag = fl;
            gif.Visible = false;
        }
        public void Open_gif()
        {
            if (this.InvokeRequired)
            {
                Opengif delegateMethod = new Opengif(this.Open_gif);
                this.Invoke(delegateMethod);
            }
            else
            {
                gif.Visible = true;
            }
        }
        public void Close_gif()
        {
            if (this.InvokeRequired)            //跨线程操作控件
            {
                Opengif delegateMethod = new Opengif(this.Close_gif);
                this.Invoke(delegateMethod);
            }
            else
            {
                gif.Visible = false;
            }
        }
        #region 防闪屏
        protected override CreateParams CreateParams        //防闪屏 

        {

            get

            {

                CreateParams cp = base.CreateParams;

                cp.ExStyle |= 0x02000000;

                return cp;

            }

        }
        #endregion
        private void Function6_Load(object sender, EventArgs e)
        {
            //if(go_flag==false)
            //{
            //    skinButton3.Enabled = false;
            //    skinButton4.Enabled = true;
            //}
            //else
            //{
            //    skinButton4.Enabled = false;
            //    skinButton3.Enabled = true;
            //}
        }
        public void crossset(byte []cro1)
        {
            cross.Set(cro1);
        }
        public void stim(byte[]st)
        {
            sttim.Set(st);
        }
        public void St(byte []st)
        {
            stat.Set(st);
        }
        public void Sp(byte speed)
        {

           // comboBox1.SelectedIndex = speed;
           switch(speed)
            {
                case 0:
                    comboBox1.SelectedIndex = 1;
                    break;
                case 1:
                    comboBox1.SelectedIndex = 2;
                    break;
                case 2:
                    comboBox1.SelectedIndex = 0;
                    break;
            }
           
        }
        public void km(byte km)
        {
            comboBox2.SelectedIndex = km;
        }
        public void go()
        {
            skinButton4.Enabled = false;
            skinButton3.Enabled = true;
        }
        public void nogo()
        {
            skinButton3.Enabled = false;
            skinButton4.Enabled = true;
        }

        private void skinButton4_Click(object sender, EventArgs e)
        {
            close_tim();
            Send_Buf[0] = 0x01;
            Send_Buf[1] = 0x06;
            Send_Buf[2] = 0x00;
            Send_Buf[3] = 0x01;
            Send_Buf[4] = 0x00;
            Send_Buf[5] = 0x01;
            send_buf(Send_Buf, 1);                          //启动
            Flag.Start = false;
            Thread thr = new Thread(new ParameterizedThreadStart(PollSend));
            thr.Start(Send_commond.Start);

        }

        private void skinButton3_Click(object sender, EventArgs e)
        {
            close_tim();
            Send_Buf[0] = 0x01;
            Send_Buf[1] = 0x06;
            Send_Buf[2] = 0x00;
            Send_Buf[3] = 0x01;
            Send_Buf[4] = 0x00;
            Send_Buf[5] = 0x02;
            send_buf(Send_Buf, 1);                          //停止
            Flag.End = false;
            Thread thr = new Thread(new ParameterizedThreadStart(PollSend));
            thr.Start(Send_commond.End);

        }

        private void label1_Click(object sender, EventArgs e)
        {
            open_tim();
            this.Close();
        }

        private void pictureBox2_Click(object sender, EventArgs e)
        {
            open_tim();
            this.Close();
        }
        #region 科大讯飞配置
        //wifi设置界面
        private void button3_Click(object sender, EventArgs e)
        {
            Wifiset wi = new Wifiset(this);
            wi.ShowDialog();
        }
        //wifi状态查询
        private void button4_Click(object sender, EventArgs e)
        {
            send_aiui(Commond.Wifi_Query);
        }
        //状态查询
        private void button5_Click(object sender, EventArgs e)
        {
            send_aiui(Commond.State_Query);
        }
        //开声音
        private void button6_Click(object sender, EventArgs e)
        {
            string str = Commond.Enable_Voice.Replace("MYVOICE", "true");
            send_aiui(str);
        }
        //关声音
        private void button7_Click(object sender, EventArgs e)
        {
            string str = Commond.Enable_Voice.Replace("MYVOICE", "false");
            send_aiui(str);
        }
        //手动唤醒
        private void button11_Click(object sender, EventArgs e)
        {
            string str = Commond.RESET_WAKE_MESSAGE.Replace("MY_MSG_TYPE", "7");
            send_aiui(str);
        }
        //重置唤醒
        private void button10_Click(object sender, EventArgs e)
        {
            string str = Commond.RESET_WAKE_MESSAGE.Replace("MY_MSG_TYPE", "8");
            send_aiui(str);
        }
        //当前状态显示
        public void set_state(string message)
        {
            aiui_state.Text = message;
        }
        //无回答图片显示
        public void gif_open()
        {
            Open_gif();
            Thread.Sleep(2000);
            Close_gif();
        }

        #endregion

        private void button1_Click(object sender, EventArgs e)
        {
            close_tim();
            stat = new StationSet(this);
            byte[] buf = new byte[6] {0x01,0x03,0x00,13,0x00,20 };
            send_buf(buf, 13);
            Flag.Stat_Set = false;        
            
            Thread thr = new Thread(new ParameterizedThreadStart(PollSend));
            thr.Start(Send_commond.Stat_Set);
            stat.ShowDialog();

        }

        private void button2_Click(object sender, EventArgs e)
        {
            close_tim();
            sttim = new StationTime(this);
            byte[] buf = new byte[6] { 0x01, 0x03, 0x00, 14, 0x00, 20 };
            send_buf(buf, 14);
            Flag.Stat_Tim = false;
            Thread thr = new Thread(new ParameterizedThreadStart(PollSend));
            thr.Start(Send_commond.Stat_Tim);
            sttim.ShowDialog();

        }

        private void skinButton2_Click(object sender, EventArgs e)
        {
            close_tim();
            Send_Buf[0] = 0x01;
            Send_Buf[1] = 0x06;
            Send_Buf[2] = 0x00;
            Send_Buf[3] = 0x04;
            Send_Buf[4] = 0x00;
           switch(comboBox1.SelectedIndex)
            {
                case 0: Send_Buf[5] = 0x02;
                    break;
                case 1:Send_Buf[5] = 0x00;
                    break;
                case 2:Send_Buf[5] = 0x01;
                    break;
            }
            send_buf(Send_Buf, 4);                          //发送速度设定值

            Thread.Sleep(500);
          
            Send_Buf[1] = 0x06;
            Send_Buf[2] = 0x00;
            Send_Buf[3] = 0x05;
            Send_Buf[4] = 0x00;
            switch (comboBox2.SelectedIndex)
            {
                case 0:
                    Send_Buf[5] = 0x00;
                    break;
                case 1:
                    Send_Buf[5] = 0x01;
                    break;
                case 2:
                    Send_Buf[5] = 0x02;
                    break;
                case 3:
                    Send_Buf[5] = 0x03;
                    break;
            }
            send_buf(Send_Buf, 5);                          //发送初始点转向设置

            //open_tim();
        }

        private void button8_Click(object sender, EventArgs e)
        {
            close_tim();
            cross = new CrossSet(this);
            byte[] buf = new byte[6] { 0x01, 0x03, 0x00, 15, 0x00, 10 };
            send_buf(buf, 15);
            Flag.Cross_Set = false;
            Thread thr = new Thread(new ParameterizedThreadStart(PollSend));
            thr.Start(Send_commond.Cross_Set);
            cross.ShowDialog();
        }

        /// <summary>
        /// 循环发送指令，在发送数据后未收到回应时每隔20ms循环发送，直到收到回应为止
        /// </summary>
        /// <param name="o"></param>
        private void PollSend(object o)
        {
            byte[] send_Poll_buff = new byte[6] { 0x01,0x00,0x00,0x00,0x00,0x00};
            byte commond = 0;
            switch((Send_commond)o)
            {
                case Send_commond.Start:
                    send_Poll_buff[1] = 0x06;
                    send_Poll_buff[3] = 0x01;
                    send_Poll_buff[5] = 0x01;
                    commond = 1;
                    break;
                case Send_commond.End:
                    send_Poll_buff[1] = 0x06;
                    send_Poll_buff[3] = 0x01;
                    send_Poll_buff[5] = 0x02;
                    commond = 1;
                    break;
                case Send_commond.Speed:break;
                case Send_commond.Stat_Set:
                    send_Poll_buff[1] = 0x03;
                    send_Poll_buff[3] = 13;
                    send_Poll_buff[5] = 20;
                    commond = 13;
                    break;
                case Send_commond.Stat_Tim:
                    send_Poll_buff[1] = 0x03;
                    send_Poll_buff[3] = 14;
                    send_Poll_buff[5] = 20;
                    commond = 14;
                    break;
                case Send_commond.Cross_Set:
                    send_Poll_buff[1] = 0x03;
                    send_Poll_buff[3] = 15;
                    send_Poll_buff[5] = 10;
                    commond = 15;
                    break;

            }
            while(true)
            {
                send_buf(send_Poll_buff, commond);
                Thread.Sleep(500);
                if ((Send_commond)o == Send_commond.Stat_Tim) { if (Flag.Stat_Tim == true) break; }
                if ((Send_commond)o == Send_commond.Stat_Set) { if (Flag.Stat_Set == true) break; }
                if ((Send_commond)o == Send_commond.Start) { if (Flag.Start == true) break; }
                if ((Send_commond)o == Send_commond.End) { if (Flag.End == true) break; }
                if ((Send_commond)o == Send_commond.Cross_Set) { if (Flag.Cross_Set == true) break; }
            }
        }
    }
}
