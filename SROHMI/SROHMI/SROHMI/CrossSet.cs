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
    public partial class CrossSet : Form
    {
        private Function6 f1;
        public CrossSet(Function6 f)
        {
            f1 = f;
            InitializeComponent();
            
        }
        TextBox[] tex = new TextBox[10];
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
            for (int i = 0; i < 10; i++)
            {
                tex[i].Text = st[i].ToString();
            }
        }
        private void button1_Click(object sender, EventArgs e)
        {
            f1.close_tim();
            byte[] send_buf = new byte[16];
            send_buf[0] = 0x01;
            send_buf[1] = 0x16;
            send_buf[2] = 0;
            send_buf[3] = 15;
            send_buf[4] = 0;
            send_buf[5] = 10;
            for (int i = 0; i < 10; i++)
            {
                send_buf[6 + i] = (byte)int.Parse(tex[i].Text);
            }
            f1.send_buf(send_buf, 15);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}
