using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using CCWin;
namespace SROHMI
{
    public partial class Function3 : Form
    {
        private int picture_num = 0;
        public Function3()
        {
            InitializeComponent();

        }

        private void label1_Click(object sender, EventArgs e)
        {
            this.Close();
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
        private void pictureBox2_Click(object sender, EventArgs e)
        {
            this.Close();
        }

       

       
        
    }
}
