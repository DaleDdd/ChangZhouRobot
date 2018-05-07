using System;
using CCWin;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;
using System.Windows.Forms;
using System.IO;
using System.Threading;

namespace SROHMI
{
    public delegate void Close();
    public partial class Form1 : CCSkinMain
    {
        ModBus my = new ModBus();
        AIUI ai = new AIUI();
        public delegate void StartTim();
        System.Timers.Timer tim = new System.Timers.Timer();
        video vi;
        SET set;
        bool set_flag = false;              //set窗体打开标志位
        bool vi_flag = false;               //vi视频窗体打开标志位
        bool pp = false;
        byte[] Send_Buf = new byte[6];
        bool go_flag = false;
        public void CloseForm1()
        {
            this.Close();
        }           
        public void StartTimer2()
        {
            if(this.InvokeRequired)
            {
                StartTim delegateMethod = new StartTim(this.StartTimer2);
                this.Invoke(delegateMethod);
            }
            else
            {
                timer2.Start();
            }
        }
        public void StopTimer2()
        {
            if (this.InvokeRequired)
            {
                StartTim delegateMethod = new StartTim(this.StopTimer2);
                this.Invoke(delegateMethod);
            }
            else
            {
                timer2.Stop();
            }
        }
        public Form1()
        {
            InitializeComponent();
            Control.CheckForIllegalCrossThreadCalls = false;
           
            my.RecivedMessageEvent += new ModBus.RecivedMessageEventHandler(ReciveHandle);
            ai.RecivedMessageEvent += new AIUI.RecivedMessageEventHandler(AIUIHandle);
            tim.Elapsed += new System.Timers.ElapsedEventHandler(tim_Elapsed);
            tim.Interval =100;   //100ms进入一次
            tim.Start();
        }
        private bool huanxing_flag = false;
        private void ViHandle(string message)
        {
            if(message=="点击")
            {
                //set.Close();
                //set_flag = false;
                vi.Hide();              //先隐藏再关闭感官上运行更顺畅
                vi.Close();
                vi.Dispose();           //关闭视频窗体

                vi_flag = false;
                pp = false; 
                //timer1.Enabled = true;
                
            }
        }
        private void AIUIHandle(string message)
        {
             switch(message)
            {
                case "模块空闲":set.aiui_state("当前模块空闲");
                                set.xianshisleep();
                                huanxing_flag = false;
                                break;
                case "模块就绪":set.aiui_state("当前模块已经唤醒");
                                set.xianshigif();
                                    huanxing_flag = true;
                                break;
                case "无回答":  set.aiui_noanswer();
                                break;
                
            }
        }
        private int timsend_flag =0;
        private void tim_Elapsed(object sender, ElapsedEventArgs e)     //定时器中断，200ms一次
        {
            timsend_flag++;
            if(timsend_flag==8)
            {
                Send_Buf[0] = 0x01;
                Send_Buf[1] = 0x03;
                Send_Buf[2] = 0x00;
                Send_Buf[3] = 0x03;
                Send_Buf[4] = 0x00;
                Send_Buf[5] = 0x01;
                my.SendBuf(ref Send_Buf, 3);

                timsend_flag = 0;
            }
            else 
            {
                Send_Buf[0] = 0x01;
                Send_Buf[1] = 0x03;
                Send_Buf[2] = 0x00;
                Send_Buf[3] = 0x02;
                Send_Buf[4] = 0x00;
                Send_Buf[5] = 0x01;
                my.SendBuf(ref Send_Buf, 2);
            }
            

        }
        protected override CreateParams CreateParams

        {

            get

            {

                CreateParams cp = base.CreateParams;

                cp.ExStyle |= 0x02000000;

                return cp;

            }

        }
        private void ReciveHandle(string message)
        {
            byte[] send_ok = new byte[6] { 0x01, 0x06, 0, 14, 0, 0 };
            try
            {
                switch (message)
                {
                    case "从站03码接收错误":
                        //MessageBox.Show("从站03码接受错误");
                        Send_Buf[0] = 0x01;
                        Send_Buf[1] = 0x03;
                        Send_Buf[2] = 0x00;
                        Send_Buf[3] = 0x0B;
                        Send_Buf[4] = 0x00;
                        Send_Buf[5] = 0x01;
                        my.SendBuf(ref Send_Buf, 11);         //再次请求距离值

                        break;
                    case "从站06码接收错误":
                        MessageBox.Show("从站06码接收错误");
                        break;
                    case "接收到放音":
                        switch (my.Sound_num)
                        {
                            case 0: break;
                            case 1:
                                string str = Commond.START_TTS.Replace("******", "您好，我是苏州斯锐奇机器人有限公司生产的迎宾机器人百灵，很高兴为您服务");
                                Send_AIUI(str);
                                break;
                            case 2: break;
                            case 3:
                                string str3 = Commond.START_TTS.Replace("******", "欢迎光临");
                                Send_AIUI(str3);
                                break;
                            case 11:
                                string str11 = Commond.START_TTS.Replace("******", "你不要摸我的头了，会变笨的");
                                Send_AIUI(str11);
                                break;
                            case 12:
                                string str12 = Commond.START_TTS.Replace("******", "摸摸头我会更乖的哟");
                                Send_AIUI(str12);
                                break;

                            case 13:
                            case 14:
                            case 15:
                            case 16:
                            case 17:
                            case 18:
                            case 19:
                            case 20:
                            case 21:
                            case 22:
                            case 23:
                            case 24:
                            case 25:
                            case 26:
                            case 27:
                            case 28:
                            case 29:
                            case 30:
                            case 31:
                            case 32:
                                string path = @"素材\站点播报语音\站点" + (my.Sound_num - 12).ToString() + "播报语音.txt";
                                if (File.Exists(path))
                                {
                                    using (StreamReader fs = new StreamReader(path, Encoding.GetEncoding("gb2312")))
                                    {
                                        string sound_play = Commond.START_TTS.Replace("******", (fs.ReadToEnd()));
                                        Send_AIUI(sound_play);
                                    }

                                }
                                break;
                        }
                        break;
                    case "接收到障碍物状态":
                        if (my.Status == 2)                 //障碍物是人
                        {
                            if (vi_flag == true && set_flag == true)
                            {
                                set.Close();
                                set_flag = false;
                                vi.Hide();              //先隐藏再关闭感官上运行更顺畅
                                vi.Dispose();           //关闭视频窗体

                                timer1.Enabled = true;
                                vi_flag = false;
                                pp = false;
                            }
                        }
                        else
                        {
                            if (vi_flag == false)
                            {
                                vi_flag = true;

                                pp = true;

                            }
                        }
                        break;
                    case "接收到速度设定值":
                        set.f6_s(my.Speed_Set);

                        Thread.Sleep(100);
                        Send_Buf[0] = 0x01;
                        Send_Buf[1] = 0x03;
                        Send_Buf[2] = 0x00;
                        Send_Buf[3] = 0x05;
                        Send_Buf[4] = 0x00;
                        Send_Buf[5] = 0x01;
                        my.SendBuf(ref Send_Buf, 5);          //请求初始点转向值
                        break;
                    case "接收到初始点转向设定值":
                        set.f6_k(my.DIR_Set);
                        break;
                    case "接收到站点设定值":
                        Flag.Stat_Set = true;
                        set.st_s(my.station);
                        break;
                    case "接收到站点时间值":
                        Flag.Stat_Tim = true;
                        set.stt(my.statim);
                        break;
                    case "接收到分叉点设置值":
                        Flag.Cross_Set = true;
                        set.cro(my.crossset);
                        break;
                    case "速度参数设置成功":
                        //MessageBox.Show("参数设置成功");
                        //tim.Start();
                        //StartTimer2();
                        break;
                    case "距离参数设置成功":
                        MessageBox.Show("参数设置成功");
                        tim.Start();
                        StartTimer2();
                        break;
                    case "放音成功":
                        tim.Start();
                        StartTimer2();
                        break;
                    case "唤醒成功":
                        tim.Start();
                        StartTimer2();
                        break;
                    case "启动成功":
                        tim.Start();
                        StartTimer2();
                        Flag.Start = true;
                        //set.closebutton();
                        Console.WriteLine("启动成功");
                        //set.go();
                        //set.go_flag = true;
                        //go_flag = true;
                        break;
                    case "停止成功":
                        tim.Start();
                        StartTimer2();
                        Flag.End = true;
                        Console.WriteLine("停止成功");
                        //set.nogo();
                        //set.go_flag = false;
                        //go_flag = false;
                        break;
                    case "站点设置成功":
                        //tim.Start();
                        //StartTimer2();
                        MessageBox.Show("设置成功");
                        break;
                    case "接收到另一台车启动":
                        //带我参观按钮关闭，15分钟后打开
                        set.closebutton();
                        Thread.Sleep(900000);
                        set.openbutton();
                        break;



                }
            }
            catch { }
            
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            string path = @"素材\过场界面图片\过场界面.jpg";
            if (File.Exists(path))
            {
                skinPictureBox1.Image = Image.FromFile(@"素材\过场界面图片\过场界面.jpg");
            }
            using (StreamReader fs = new StreamReader(@"素材\界面文本\界面文本.txt", Encoding.GetEncoding("gb2312")))
            {
                skinLabel1.Text = (fs.ReadToEnd());
            }
        }

        #region 定义方法，委托用
        //声明一个方法，用来关闭定时器
        public void tim_close()
        {
            tim.Close();
            timer2.Stop();
        }
        public void tim_start()
        {
            tim.Start();
            StartTimer2();
        }
        public void Send_Data(byte[] Buff,byte add)
        {
            my.SendBuf(ref Buff, add);
        }
        public void Send_AIUI(string str)
        {
            ai.Send_Commond(str);
        }
        public void sp_write(string a1,string a2,int jia)
        {
            ai.WIFI_Set(a1, a2,jia);
        }
        #endregion
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            //当点击关闭的时候
            if(tim!=null)        //如果定时器没关
            {
                tim.Close();
            }
        }

        private void Form1_Click(object sender, EventArgs e)
        {
            if(set_flag==false)
            {
                set = new SET(Send_Data,tim_close,tim_start,CloseForm1,go_flag,Send_AIUI,sp_write);
                set.Show();
                set_flag = true; 
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (set_flag == false)
            {
                set = new SET(Send_Data, tim_close, tim_start,CloseForm1,go_flag,Send_AIUI,sp_write);
                set.Show();
                set_flag = true;
            }
           
        }
       
  

        private void timer2_Tick(object sender, EventArgs e)
        {
            if(pp==true)
            {
                vi = new video();
                vi.RecivedMessageEvent += new video.RecivedMessageEventHandler(ViHandle);
                vi.Show();
                pp = false;
            }
        }
    }
}
