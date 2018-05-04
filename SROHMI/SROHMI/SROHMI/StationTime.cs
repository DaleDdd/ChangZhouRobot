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
    public partial class StationTime : Form
    {
        private Function6 f1;
        public StationTime(Function6 f)
        {
            f1 = f;
            InitializeComponent();
        }
        TextBox[] tex = new TextBox[20];

        private void button2_Click(object sender, EventArgs e)
        {
            this.Close();
        }
        public void Set(byte[] st)
        {
            tex[0] = textBox1;
            tex[1] = textBox2;
            tex[2] = textBox3;
            tex[3] = textBox4;
            tex[4] = textBox5;
            tex[5] = textBox6;
            tex[6] = textBox7;
            tex[7] = textBox8;
            tex[8] = textBox9;
            tex[9] = textBox10;
            tex[10] = textBox11;
            tex[11] = textBox12;
            tex[12] = textBox13;
            tex[13] = textBox14;
            tex[14] = textBox15;
            tex[15] = textBox16;
            tex[16] = textBox17;
            tex[17] = textBox18;
            tex[18] = textBox19;
            tex[19] = textBox20;
            for (int i = 0; i < 20; i++)
            {
                tex[i].Text = st[i].ToString();
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            f1.close_tim();
            byte[] send_buf = new byte[26];
            send_buf[0] = 0x01;
            send_buf[1] = 0x16;
            send_buf[2] = 0;
            send_buf[3] = 14;
            send_buf[4] = 0;
            send_buf[5] = 20;
            for (int i = 0; i < 20; i++)
            {
                send_buf[6 + i] = (byte)int.Parse(tex[i].Text);
            }
            f1.send_buf(send_buf, 14);
        }
    }
}
