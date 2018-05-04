using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SROHMI
{
    //声明一个委托，用来关闭主窗体定时器
    public delegate void stop();
    
    public partial class video : Form
    {
        public delegate void RecivedMessageEventHandler(string message); //定义委托
        public event RecivedMessageEventHandler RecivedMessageEvent = null;//定义事件
        stop tim_stop,tim_start;
        public video()
        {
            InitializeComponent();

        }
        public video(stop tim,stop tim1)
        {
            InitializeComponent();
            tim_stop = tim;
            tim_start = tim1;
        }

        private void video_Load(object sender, EventArgs e)
        {
            axWindowsMediaPlayer1.Location = new Point(0, 0);
           // axWindowsMediaPlayer1.Size = this.Size;
            axWindowsMediaPlayer1.uiMode = "none";      //视频播放器界面去除各类按钮
            axWindowsMediaPlayer1.settings.volume = 100;
            axWindowsMediaPlayer1.URL = @"素材\宣传视频\案例展示.mp4";            //视频播放路径
            axWindowsMediaPlayer1.Ctlcontrols.play();   //播放视频
        }

        private void axWindowsMediaPlayer1_StatusChange(object sender, EventArgs e)     //播放结束后循环播放
        {
            axWindowsMediaPlayer1.Ctlcontrols.play();
        }

       

        private void axWindowsMediaPlayer1_ClickEvent(object sender, AxWMPLib._WMPOCXEvents_ClickEvent e)
        {
           RecivedMessage("点击");
        }

        private void video_FormClosing(object sender, FormClosingEventArgs e)
        {
            axWindowsMediaPlayer1.close();
        }

        /// <summary>
        /// 触发事件的函数
        /// </summary>
        /// <param name="message"></param>
        private void RecivedMessage(string message)
        {
            if (RecivedMessageEvent != null)
            {
                RecivedMessageEvent(message);
            }
        }

    }
}
