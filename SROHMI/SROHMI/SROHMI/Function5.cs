using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using CCWin;
using System.IO;
using System.Threading;

namespace SROHMI
{
    public partial class Function5 : Form
    {
        send send_buf;
        stop close_tim;
        public sendai send_aiui;
        public delegate void Opengif();
        byte[] Send_Buf = new byte[6];
        public Function5()
        {      
            InitializeComponent();
        }
        public Function5(send s,stop s1,sendai a1)
        {
            send_buf = s;
            close_tim = s1;
            send_aiui = a1;
            InitializeComponent();
        }

        private void label1_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void pictureBox2_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void Function5_Load(object sender, EventArgs e)
        {
            //string path = @"素材\A7-机器人互动 - 修改1(1).png";
            //pictureBox4.Enabled = false;
            //if (File.Exists(path))
            //{
            //    this.BackgroundImage = Image.FromFile(@"素材\A7-机器人互动 - 修改1(1).png");
            //}
        }

        //private void pictureBox3_Click(object sender, EventArgs e)
        //{
        //    pictureBox3.Enabled = false;
        //    pictureBox4.Enabled = true;
        //    string path = @"素材\A7-机器人互动 - 修改2(1).png";
        //    if (File.Exists(path))
        //    {
        //        this.BackgroundImage = Image.FromFile(@"素材\A7-机器人互动 - 修改2(1).png");
        //    }
        //}

        //private void pictureBox4_Click(object sender, EventArgs e)
        //{
        //    pictureBox3.Enabled = true;
        //    pictureBox4.Enabled = false;
        //    string path = @"素材\A7-机器人互动 - 修改1(1).png";
        //    if (File.Exists(path))
        //    {
        //        this.BackgroundImage = Image.FromFile(@"素材\A7-机器人互动 - 修改1(1).png");
        //    }
        //}

        
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
      

        private void pictureBox5_Click(object sender, EventArgs e)
        {
            string str = Commond.RESET_WAKE_MESSAGE.Replace("MY_MSG_TYPE", "7");
            send_aiui(str);
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
            if (this.InvokeRequired)
            {
                Opengif delegateMethod = new Opengif(this.Close_gif);
                this.Invoke(delegateMethod);
            }
            else
            {
                gif.Visible = false;
            }
        }
        //无回答图片显示
        public void gif_open()
        {
            Open_gif();
            Thread.Sleep(2000);
            Close_gif();
        }
       public void xianshi()
        {
            string path = @"素材\圆圈.gif";
            if (File.Exists(path))
            {
               pictureBox5.Image = Image.FromFile(@"素材\圆圈.gif");
            }
        }
        public void sleep()
        {
            string path = @"素材\点我对话.png";
            if (File.Exists(path))
            {
                pictureBox5.Image = Image.FromFile(@"素材\点我对话.png");
            }
        }
    }
}
