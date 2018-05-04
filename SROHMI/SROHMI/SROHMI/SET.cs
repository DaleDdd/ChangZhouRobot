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
using System.IO;

namespace SROHMI
{
    //声明一个委托，用来发送数据
    public delegate void send(byte[] a,byte b);
    public delegate void sendai(string a);
    public delegate void wifi(string a, string b,int jia);
    public partial class SET : Form
    {
        byte[] Send_Buf = new byte[6];
        bool f1_flag = false;
        bool f2_flag = false;
        bool f3_flag = false;
        bool f4_flag = false;
        bool f5_flag = false;
        bool f6_flag = false;
        bool f7_flag = false;
        bool f8_flag = false;
        bool f9_flag = false;
        Function1 f1;
        Function2 f2;
        Function3 f3;
        Function4 f4;
        Function5 f5;
        Function6 f6;
        Function9 f9;
        send send_buf;
        stop close_tim,open_tim;
        Close closeform;
        sendai send_aiui;
        wifi wifiset;
        public bool go_flag { set; get; }
        public SET()
        {
            InitializeComponent();
            
            
        }
        public SET(send s,stop s1,stop s2,Close s3,bool fl,sendai a1,wifi w1)
        {
            InitializeComponent();
            send_buf = s;
            close_tim = s1;
            open_tim = s2;
            closeform = s3;
            go_flag=fl;
            send_aiui = a1;
            wifiset = w1;
            groupBox2.Enabled = Enabled;
            groupBox2.Visible = Enabled;

        }
        #region 打开6个窗体
        private void skinPictureBox1_Click(object sender, EventArgs e)
        {
            string path = @"素材\界面播放语音\学院简介语音";
            if (File.Exists(path))
            {
                using (StreamReader fs = new StreamReader(path, Encoding.GetEncoding("gb2312")))
                {
                    string sound_play = Commond.START_TTS.Replace("******", (fs.ReadToEnd()));
                    send_aiui(sound_play);
                }

            }
              f1_flag = true;
              f1 = new Function1();
              f1.Show();
            
                                          
        }

        private void skinPictureBox2_Click(object sender, EventArgs e)
        {
            string path = @"素材\界面播放语音\院系介绍语音";
            if (File.Exists(path))
            {
                using (StreamReader fs = new StreamReader(path, Encoding.GetEncoding("gb2312")))
                {
                    string sound_play = Commond.START_TTS.Replace("******", (fs.ReadToEnd()));
                    send_aiui(sound_play);
                }

            }
            //close_tim();
            //string str = Commond.START_TTS.Replace("******", "国信雅都大酒店是一个充满人文特色的酒店，酒店拥有各种丰富多彩的活动，等待您的光临");
            //send_aiui(str);
            f2_flag = true;
            f2 = new Function2();
            f2.Show();
            
            
        }

        private void skinPictureBox3_Click(object sender, EventArgs e)
        {
            string path = @"素材\界面播放语音\校园文化语音";
            if (File.Exists(path))
            {
                using (StreamReader fs = new StreamReader(path, Encoding.GetEncoding("gb2312")))
                {
                    string sound_play = Commond.START_TTS.Replace("******", (fs.ReadToEnd()));
                    send_aiui(sound_play);
                }

            }
            //close_tim();
            //string str = Commond.START_TTS.Replace("******", "本酒店拥有各类设备齐全的娱乐设施，必能令您身心舒畅");
            //send_aiui(str);
            f3_flag = true;
            f3 = new Function3();
            f3.Show();
          
           
        }

        private void skinPictureBox4_Click(object sender, EventArgs e)
        {
            string path = @"素材\界面播放语音\校园风采语音";
            if (File.Exists(path))
            {
                using (StreamReader fs = new StreamReader(path, Encoding.GetEncoding("gb2312")))
                {
                    string sound_play = Commond.START_TTS.Replace("******", (fs.ReadToEnd()));
                    send_aiui(sound_play);
                }

            }
            //close_tim();
            //string str = Commond.START_TTS.Replace("******", "酒店拥有世纪厅,多功能厅等大型宴会活动场所，可举办各类大型会议和宴请，更可为您度身打造各类婚宴，为您的人生之旅锦上添花");
            //send_aiui(str);
            f4_flag = true;
            f4 = new Function4();
            f4.Show();
        
            
        }

        private void skinPictureBox5_Click(object sender, EventArgs e)
        {

            //close_tim();
            string str = Commond.START_TTS.Replace("******", "你好，我的名字是叮咚叮咚，只要你叫一下我的名字或者按一下点我对话按钮就可以和我说话啦");
            send_aiui(str);
            f5_flag = true;
            f5 = new Function5(send_buf,close_tim,send_aiui);
            f5.Show();
            
            
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
        private void skinPictureBox6_Click(object sender, EventArgs e)
        {
            groupBox1.Visible = false;
            groupBox2.Visible = false;
            groupBox4.Location =new Point(529, 201);
            groupBox4.Visible = true;
            textBox1.Focus();



        }
        #endregion
        public void openbutton()
        {
            button5.Enabled = true;
        }
        public void closebutton()
        {
            button5.Enabled = false;
        }
        public void cro(byte []cro)
        {
            f6.crossset(cro);
        }
        public void stt(byte []st)
        {
            f6.stim(st);
        }
        public void st_s(byte[]st)
        {
            f6.St(st);
        }
        public void f6_s(byte sp)
        {
            f6.Sp(sp);
        }
        public void f6_k(byte km)
        {
            f6.km(km);
        }
       public void go()
        {
            f6.go();
        }
        public void nogo()
        {
            f6.nogo();
        }
        public void aiui_state(string state)
        {
            if (f6 == null)
                return;
            else
            {
                f6.set_state(state);                    /*在f6窗体上面显示相应的数值*/
            }                                                         

        }
        public void xianshigif()                        //在唤醒之后显示动态图
        {
            if (f5 == null)
                return;
            else
            {
                f5.xianshi();
            }
        }
        public void xianshisleep()
        {
            if (f5 == null)
                return;
            else
            {
                f5.sleep();
            }
        }
        public void aiui_noanswer()
        {
            if (f5 != null)                              //f5窗体存在
                f5.gif_open();
            if (f6 != null)
                f6.gif_open();                           //f6窗体存在
            
        }

        
        private void SET_Load(object sender, EventArgs e)
        {
            //this.BackgroundImage = Image.FromFile(@"素材\酒店.jpg");
            //using (StreamReader fs = new StreamReader(@"素材\界面文本\界面文本.txt", Encoding.GetEncoding("gb2312")))
            //{
            //    skinLabel1.Text = (fs.ReadToEnd());
            //}
        }

       

         /*机器人配置*/
        private void button3_Click(object sender, EventArgs e)
        {
            string pass = textBox1.Text;
            if (pass == "123")
            {
                close_tim();
                f6_flag = true;


                f6 = new Function6(send_buf, close_tim, open_tim, go_flag,send_aiui,wifiset);
                Send_Buf[0] = 0x01;
                Send_Buf[1] = 0x03;
                Send_Buf[2] = 0x00;
                Send_Buf[3] = 0x04;
                Send_Buf[4] = 0x00;
                Send_Buf[5] = 0x01;
                send_buf(Send_Buf, 4);          //请求速度值

                f6.Show();
                groupBox4.Visible = false;
                textBox1.Text = "";
                groupBox1.Visible = true;
                groupBox2.Visible = true;
            }
            else
            {
                MessageBox.Show("密码错误");
                textBox1.Text = "";
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            groupBox4.Visible = false;
            groupBox1.Visible = true;
            groupBox2.Visible = true;
            textBox1.Text = "";
        }

        private void skinPictureBox9_Click(object sender, EventArgs e)
        {
            f9_flag = true;
            f9 = new Function9();
            f9.Show();
        }

        private void label2_Click(object sender, EventArgs e)
        {
            closeform();
        }

        private void button5_Click(object sender, EventArgs e)
        {
            video v = new video();
            v.Show();
        }

        private void button5_Click_1(object sender, EventArgs e)
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

        //private void button5_Click_1(object sender, EventArgs e)
        //{
        //    close_tim();
        //    Send_Buf[0] = 0x01;
        //    Send_Buf[1] = 0x06;
        //    Send_Buf[2] = 0x00;
        //    Send_Buf[3] = 0x01;
        //    Send_Buf[4] = 0x00;
        //    Send_Buf[5] = 0x01;
        //    send_buf(Send_Buf, 1);                          //启动
        //}

        //private void button6_Click(object sender, EventArgs e)
        //{
        //    close_tim();
        //    Send_Buf[0] = 0x01;
        //    Send_Buf[1] = 0x06;
        //    Send_Buf[2] = 0x00;
        //    Send_Buf[3] = 0x01;
        //    Send_Buf[4] = 0x00;
        //    Send_Buf[5] = 0x02;
        //    send_buf(Send_Buf, 1);                          //停止
        //}

        /// <summary>
        /// 循环发送指令，在发送数据后未收到回应时每隔20ms循环发送，直到收到回应为止
        /// </summary>
        /// <param name="o"></param>
        private void PollSend(object o)
        {
            byte[] send_Poll_buff = new byte[6] { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 };
            byte commond = 0;
            switch ((Send_commond)o)
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
                case Send_commond.Speed: break;
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
            while (true)
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


        #region 关闭选项窗体时关闭其他打开的窗体
        private void SET_FormClosing(object sender, FormClosingEventArgs e)
        {
            if(f1_flag)
            {
                f1.Close();
            }
            if (f2_flag)
            {
                f2.Close();
            }
            if (f3_flag)
            {
                f3.Close();
            }
            if (f4_flag)
            {
                f4.Close();
            }
            if (f5_flag)
            {
                f5.Close();
            }
            if (f6_flag)
            {
                f6.Close();
            }
            if (f9_flag)
            {
                f9.Close();
            }

        }
        #endregion
    }
}
