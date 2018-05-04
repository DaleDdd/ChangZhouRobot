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
    public partial class Wifiset : Form
    {
        private Function6 f1;
        public Wifiset(Function6 fun6)
        {
            f1 = fun6;
            InitializeComponent();
            comboBox1.SelectedIndex = 2;
        }
 

        private void button1_Click_1(object sender, EventArgs e)
        {
            if (textBox1.Text == "" && textBox2.Text == "")
            {
                MessageBox.Show("请输入WIFI信息");
            }
            else
            {
                f1.wifiset(textBox2.Text, textBox1.Text,comboBox1.SelectedIndex);

                this.Close();
            }

        }

        private void button2_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}
