using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SROHMI
{
    class Commond
    {
        public static string Wifi_Query = "{\"type\": \"status\",\"content\": {\"query\": \"wifi\"}}"; //wifi状态查询
        public static string State_Query = "{\"type\": \"aiui_msg\",\"content\": {\"msg_type\":1,\"arg1\":0,\"arg2\":0,\"params\":\"\"}}"; //查询服务状态
        public static string Enable_Voice = "{\"type\": \"voice\",\"content\": {\"enable_voice\":MYVOICE}}";//开关声音
        public static string START_TTS = "{\"type\": \"tts\",\"content\": {\"action\": \"start\",\"text\": \"******\"}}";//语音合成
        public static string RESET_WAKE_MESSAGE = "{\"type\": \"aiui_msg\",\"content\": {\"msg_type\": MY_MSG_TYPE,\"arg1\": 0,\"arg2\": 0,\"params\": \"\"}}";

    }
    public enum Send_commond               //发送的数据命令
    {
        Speed=0,                      //速度
        Stat_Set,                   //站点转向设置
        Stat_Tim,                   //站点停靠时间设置
        Start,                      //启动
        End,                         //停止
        Cross_Set                   //分叉点设置
    }
    class Flag
    {
        public static bool Start = false;
        public static bool End = false;
        public static bool Stat_Set = false;
        public static bool Stat_Tim = false;
        public static bool Cross_Set = false;
    }
}
