using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SROHMI
{
    class AIUI
    {
        private SerialPort sp1;
        public delegate void RecivedMessageEventHandler(string message); //定义委托
        public event RecivedMessageEventHandler RecivedMessageEvent = null;//定义事件
        #region 构造函数
        public AIUI()
        {
            sp1 = new SerialPort();
            //串口参数赋值
            sp1.PortName = "com4";
            sp1.BaudRate = 115200;     //波特率115200
            sp1.DataBits = 8;
            sp1.Parity = Parity.None;
            sp1.StopBits = StopBits.One;

            //判断是否开启，没开启直接打开
            if (!sp1.IsOpen)
            {
                try
                {
                    sp1.Open();

                }
                catch
                {
                    MessageBox.Show("串口打开失败");
                }

                sp1.DataReceived += new SerialDataReceivedEventHandler(sp_DataReceive);
            }
        }
        #endregion
        #region 串口接收及处理
        /*串口接收*/
        private void sp_DataReceive(object sender, SerialDataReceivedEventArgs e)
        {
            Thread.Sleep(100);
            int n = sp1.BytesToRead;
            byte[] buf = new byte[n];
            sp1.Read(buf, 0, n);

            Thread thr = new Thread(new ParameterizedThreadStart(HandleData));
            thr.Start(buf);
        }
        /*接收处理*/
        private void HandleData(object o)
        {
            byte[] buff = o as byte[];
            byte[] sendbuf = { 0xA5, 0x01, 0xFF, 0x04, 0x00, 0x00, 0x00, 0xA5, 0x00, 0x00, 0x00, 0x00 };

            if (null == buff || buff.Length < 4)
            {
                return;
            }

            if (0xA5 == buff[0] && 0x01 == buff[1])
            {
                sendbuf[5] = buff[5];
                sendbuf[6] = buff[6];
                byte sum = 0;
                for (int i = 0; i < sendbuf.Length; i++)
                {

                    sum += sendbuf[i];
                }
                sendbuf[sendbuf.Length - 1] = (byte)((byte)((byte)~sum + 1) & 0xff);

                sp1.Write(sendbuf, 0, sendbuf.Length);

                switch (buff[2])
                {
                    /*握手请求*/
                    case 0x01:
                        break;
                    /*WiFi配置*/
                    case 0x02: break;
                    /*AIUI配置*/
                    case 0x03: break;
                    /*AIUI消息*/
                    case 0x04:
                        byte[] zip = new byte[buff.Length - 8];
                        for (int i = 0; i < zip.Length; i++)
                        { zip[i] = buff[7 + i]; }
                        using (MemoryStream dms = new MemoryStream())            //dms保存解压之后的流
                        {
                            using (MemoryStream cms = new MemoryStream(zip))     //cms是用来转换的流
                            {
                                using (GZipStream gzip = new GZipStream(cms, CompressionMode.Decompress))
                                {
                                    byte[] bytes = new byte[1024];
                                    int len = 0;
                                    //读取压缩流，同时会被解压
                                    while ((len = gzip.Read(bytes, 0, bytes.Length)) > 0)
                                    {
                                        dms.Write(bytes, 0, len);
                                    }
                                }
                            }
                            string str = Encoding.UTF8.GetString(dms.ToArray());
                            try
                            {

                                JObject obj = JObject.Parse(str);
                                JToken j1 = obj["type"];

                                switch (j1.ToString())
                                {
                                    case "wifi_status":
                                        JToken ssid = obj["content"]["ssid"];
                                        JToken stat = obj["content"]["connected"];
                                        if (stat.ToString() == "True")
                                             MessageBox.Show("当前设备连入了" + ssid.ToString());
                                        else
                                            MessageBox.Show("当前设备没有连接wifi");
                                        break;
                                    case "aiui_event":
                                        JToken j2 = obj["content"]["eventType"];
                                        switch (j2.ToString())
                                        {
                                            /*服务状态事件*/
                                            case "3":
                                                JToken stats = obj["content"]["arg1"];
                                                if (stats.ToString() == "2") 
                                                RecivedMessage("模块空闲");
                                                if (stats.ToString() == "3")
                                                RecivedMessage("模块就绪");
                                                break;
                                            /*听写结果事件*/
                                            case "1":
                                                try
                                                {
                                                    JToken result = obj["content"]["result"]["intent"]["text"];
                                                    string re = result.ToString();
                                                    try
                                                    {
                                                        JToken answer = obj["content"]["result"]["intent"]["answer"];
                                                        string sa = answer.ToString();
                                                    }
                                                    catch
                                                    {
                                                        try
                                                        {
                                                            JToken state = obj["content"]["result"]["intent"]["state"];
                                                            string st = state.ToString();
                                                        }
                                                        catch
                                                        {
                                                            RecivedMessage("无回答");
                                                        }

                                                    }
                                                }
                                                catch
                                                {

                                                }
                                                break;
                                        }
                                        break;
                                }

                            }
                            catch
                            {

                            }
                        }
                        break;
                    /*主控消息*/
                    case 0x05: break;
                    /*确认消息*/
                    case 0xff:
                        break;
                }
            }
        }
        #endregion
        Random r1 = new Random();
        /*发送主控消息*/
        public void Send_Commond(string str)
        {
            byte[] cbytes = null;
            using (MemoryStream cms = new MemoryStream())
            {
                using (GZipStream gzip = new GZipStream(cms, CompressionMode.Compress))
                {
                    byte[] bytes = Encoding.UTF8.GetBytes(str);
                    gzip.Write(bytes, 0, bytes.Length);
                }
                cbytes = cms.ToArray();                              //Json压缩后的数组
            }

            byte[] sendbuf = new byte[cbytes.Length + 8];
            sendbuf[0] = 0xA5;
            sendbuf[1] = 0x01;
            sendbuf[2] = 0x05;
            sendbuf[3] = (byte)(sendbuf.Length - 8);
            sendbuf[4] = 0;
            sendbuf[5] = (byte)r1.Next(0, 255);
            sendbuf[6] = (byte)r1.Next(0, 255);
            for (int i = 0; i < cbytes.Length; i++)
            {
                sendbuf[7 + i] = cbytes[i];
            }
            byte sum = 0;
            for (int i = 0; i < sendbuf.Length; i++)
            {

                sum += sendbuf[i];
            }
            sendbuf[sendbuf.Length - 1] = (byte)((byte)((byte)~sum + 1) & 0xff);
            try
            {
                sp1.Write(sendbuf, 0, sendbuf.Length);
            }
            catch { }
            
        }
       #region Wifi名称密码设置
        public void WIFI_Set(string name1, string password1,int jiami)
        {

            byte[] name = null;
            byte[] password = null;
            name = Encoding.UTF8.GetBytes(name1);
            password = Encoding.UTF8.GetBytes(password1);

            byte[] sendbuf = new byte[12 + name.Length + password.Length];
            sendbuf[0] = 0xA5;
            sendbuf[1] = 0x01;
            sendbuf[2] = 0x02;

            sendbuf[3] = (byte)(sendbuf.Length - 8);
            sendbuf[4] = 0;

            sendbuf[5] = (byte)(r1.Next(0, 255));
            sendbuf[6] = 0;

            sendbuf[7] = 0;

            sendbuf[8] = (byte)jiami;

            sendbuf[9] = (byte)name.Length;
            sendbuf[10] = (byte)password.Length;

            for (int i = 0; i < name.Length; i++)
            {
                sendbuf[11 + i] = name[i];
            }
            for (int i = 0; i < password.Length; i++)
            {
                sendbuf[11 + name.Length + i] = password[i];
            }
            byte sum = 0;
            for (int i = 0; i < sendbuf.Length; i++)
            {

                sum += sendbuf[i];
            }
            sendbuf[sendbuf.Length - 1] = (byte)((byte)((byte)~sum + 1) & 0xff);
            sp1.Write(sendbuf, 0, sendbuf.Length);

        }
        #endregion
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
