#ifndef DATA_TYPE
#define DATA_TYPE

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#	pragma once
#endif

#include <string>
#include <vector>
#include <map>
#include "StructDef.h"

#include "DataTypeDefine.h"

using namespace std;


enum e_CmdExcuteResult
{
    CMD_RT_OK         = 0,//成功
    CMD_RT_FAILURE      = 1,//失败
    CMD_RT_GOING      = 2,//正在执行
    CMD_RT_GOING_LOOP = 3,//循环发送指令
    CMD_RT_FILURE_TIMEOUT = 4,//超时失败
    CMD_RT_GOING_DETECT_RUN_STATE = 5,//正在检测运行状态
    CMD_RT_GOING_DETECT_WAIT_ATTENA = 6,//正在等待天线到位
    CMD_RT_FAILURE_ATTENA_POS_ERROR = 7,//天线位置不对
    CMD_RT_FAILURE_NO_ALLOW_EXCUTE = 8,//禁止执行开关发射机操作（天线防抖等待）
    CMD_RT_FAILURE_NO_ALLOW_SWITCH_ATTENA = 9,//禁止倒天线（当前机器正在运行）
    CMD_RT_GOING_SWITCH_TO_HOST = 10,//切换器到主路
    CMD_RT_GOING_SWITCH_TO_BACKUP_ONE = 11,//切换器到备一路
    CMD_RT_GOING_SWITCH_TO_BACKUP_SECOND = 12,//切换器到备二路
    CMD_RT_GOING_SWITCH_TO_BACKUP_THRID = 13,//切换器到备三路
    CMD_RT_GOING_SWITCH_TO_BACKUP_FOURTH = 14,//切换器到备四路
    CMD_RT_SWITCH_CHANGE_HOST = 15,//信号置主路
    CMD_RT_SWITCH_CHANGE_BACKUP_ONE = 16,//信号置备一路
    CMD_RT_SWITCH_CHANGE_BACKUP_SECOND = 17,//信号置备二路
    CMD_RT_SWITCH_CHANGE_BACKUP_THRID = 18,//信号置备三路
    CMD_RT_SWITCH_CHANGE_BACKUP_FOURTH = 19,//信号置备四路

};

enum dev_alarm_state
{
	invalid_alarm =-1,
	upper_alarm   =0,//上限报警
	lower_alarm,     //下限报警
	state_alarm,     //状态量报警
	resume_normal,   //恢复正常
};

enum con_state 
{ 
    con_connected= 0,       //已连接
    con_disconnected ,      //已断开
    con_connecting,         //正在连接...
};

enum usr_state
{
	usr_offline=-1,    //离线
	usr_online,        //在线
};


enum dev_run_state
{
    dev_running   =0,//已开机
	dev_shutted_down,//已关机(备机)
	antenna_host,    //天线位置(主机)
	antenna_backup,  //天线位置(备机)
	dev_detecting,   //正在检测(正在移动)
	dev_unknown,     //未知
};

enum dev_opr_state
{
    dev_no_opr = -1,
    dev_opr_excuting,
    dev_opr_turn_on,                    //正在开机
    dev_opr_soft_onekey_turn_on, //正在服务器一键开机
    dev_opr_996_onekey_turn_on, //正在996一键开机
    dev_opr_auto_switch_backup,  //正在自动倒备机
    dev_opr_turn_off,                    //正在关机
    dev_opr_switch_attena,           //正在切换天线
    dev_opr_996_onekey_turn_off, //正在996关机
    dev_opr_other_task,// 正在执行其他任务
};

enum Dev_Type
{
    DEV_TRANSMITTER = 0,//发射机
    DEV_ANTENNA     ,   //天线
    DEV_OTHER       ,   //其他设备
    DEV_ELCPOWER    ,   //电力设备
    DEV_ENVIR       ,   //环境设备
    DEV_TRANSMITTER_AGENT,//发射机代理
    DEV_MEDIA       ,     //媒体设备
    DEV_NODEFIN     ,
};

//开机等级，适用于中波发射机，默认高功率
enum TurnOn_Type
{
	HIGH_POWER_ON =0,
	MIDDLE_POWER_ON ,
	LOW_POWER_ON,
};

//告警级别
enum Alarm_Level
{
	NORMAL =0,//直接通知客户端
	SMS    ,  //短信通知
	TELEPHONE,//电话通知
	SMSANDTEL,//短信加电话
};

//监控时间计划(非监控时间段不进行报警检测)----2012.3.15 added by lk
typedef struct 
{
	string  sGuid;                  //guid
	string  sStationNumber;         //台站编号
	string  sTransmitterNumber;     //发射机编号
	bool    bEnable;                //使能标志
	int     nMonitorDay;            //星期(监控)
	tm      tmStartTime;            //开始时间
	tm      tmCloseTime;            //结束时间
}MonitorScheduler;


struct DevicePropertyEx
{
        DevicePropertyEx()
		:query_timeout_count(6)//默认查询超时次数为3次(在查询间隔内发送两次查询指令无数据返回则断开当前网络连接)
        ,query_interval(3000)//2默认查询间隔2000耗秒
		,connect_timer_interval(3)//默认连接间隔3秒
		,connect_timeout_duration(10)//默认连接超时时长10秒
		,run_detect_max_count(2)//默认检测功率3次高于0值功率设定值则认为开机
		,shutdown_detect_max_count(2)//默认检测功率3次低于0值功率设定值则认为关机
        ,data_save_interval(300)//默认15分钟记录一次数据
		,alarm_detect_max_count(2)//默认报警检测次数为2次
		,cmd_timeout_interval(10)//10默认命令超时循环间隔为10秒钟
        ,cmd_excut_timeout_duration(60)//60默认命令发送超时时长为2分钟,超过此时间将不再尝试发送控制命令
		,multi_query_send_interval(30)//默认多查询指令交替间隔为30毫秒
		,zero_power_value(0.02)//默认0.02kw为关机零功率
		,u0_range_value(220)//默认电压量程（9033A）
		,i0_range_value(5)//默认电流量程(9033A)
		,ubb_ratio_value(1)//默认电压调整系数（9033A）
		,ibb_ratio_value(1)//默认电流调整系数(9033A)
        ,is_auto_switch_transmitter(false)//是否进行自动倒备
		,switch_power_lower_limit_value(0.02)//20w作为倒机门限
		,auto_switch_detect_max_times(2)//倒机条件检测次数
		,auto_switch_timeout_max_times(5)//倒机命令执行最大次数
        ,associate_transmit_number("")//关联发射机编号
        ,associate_use(false)//是否存在关联
        ,is_low_pressure_judge_(false)//是否进行低压判断
        ,low_pressure_limit_value_(100.00f)//低压判断门限值
        ,is_high_pressure_judge_(false)//是否进行高压判断
        ,high_pressure_limit_value_(200.00f)//高压判断门限值
        ,is_power_adjust_(false)//是否进行功率调整
		,power_limit_value_(0.50f)//默认0.5kw作为开机运行判定功率
        ,is_digital_item_adjust_(false)//是否进行数字量判断
        ,digital_detect_during_time_(10)//10秒钟检测时间
        ,is_step_open_close(false)//分步骤开关机
        ,iCheckRunIndex(0)//监测开关机的监控量
	{
		
	}


	int    query_interval;            //数据查询间隔
	int    query_timeout_count;       //查询命令超时发送次数
	int    connect_timer_interval;    //连接定时间隔
	int    connect_timeout_duration;  //连接超时时长
	int    run_detect_max_count;      //开机检测最大次数
	int    shutdown_detect_max_count; //关机检测最大次数
	int    data_save_interval;        //数据定时保存间隔
	int    alarm_detect_max_count;    //报警检测最大次数
	int    cmd_timeout_interval;      //命令定时器超时检测间隔
	int    cmd_excut_timeout_duration;//控制命令执行超时时间
	int    multi_query_send_interval; //复合指令发送延迟间隔
	double zero_power_value;          //零功率值（低于该值认为是关机）
	//double run_power_value;           //运行功率（高于该值认为已开机）
	bool   is_auto_switch_transmitter;   //是否自动倒备机
	double switch_power_lower_limit_value;//倒机门限功率
	int    auto_switch_detect_max_times;  //检测需自动倒备机条件次数
	int    auto_switch_timeout_max_times; //自动倒备机切换天线超时执行次数

	int u0_range_value;            //默认电压量程（9033A）
	int i0_range_value;            //默认电流量程(9033A)
	double ubb_ratio_value;           //默认电压调整系数（9033A）
	double ibb_ratio_value;           //默认电流调整系数(9033A)
	
    map<int,int>  cmd_ack_length_by_id;//查询命令响应长度

	string  associate_transmit_number;//关联主机id(针对无天线主备机配置)
	bool    associate_use;//关联主机启用

	bool    is_low_pressure_judge_;//是否进行低压判断
	double     low_pressure_limit_value_;//低压下限值
	bool    is_high_pressure_judge_;//是否进行高压判断
	double     high_pressure_limit_value_;//高压下限值
	bool    is_power_adjust_;//是否进行功率调整
	double     power_limit_value_;//功率门限值
	bool    is_digital_item_adjust_;//是否进行数字量判断
	int     digital_detect_during_time_;//数字量检测时长
    bool    is_step_open_close;//是否分多步开关机
    int     iCheckRunIndex;//判断开机的监控量序号 add by ws 2019-04-08

};
typedef boost::shared_ptr<DevicePropertyEx> pDevicePropertyExPtr;


struct MoxaPropertyEx
{
    MoxaPropertyEx():query_timeout_count(3)//默认查询超时次数为3次(在查询间隔内发送两次查询指令无数据返回则断开当前网络连接)
		,query_interval(2000)//2默认查询间隔2000耗秒1600
		,connect_timer_interval(3)//默认连接间隔3秒
		,connect_timeout_duration(10)//默认连接超时时长10秒
	{

	}
	int    query_interval;            //数据查询间隔
	int    query_timeout_count;       //查询命令超时发送次数
	int    connect_timer_interval;    //连接定时间隔
	int    connect_timeout_duration;  //连接超时时长
};
typedef boost::shared_ptr<MoxaPropertyEx> pMoxaPropertyExPtr;



struct DevSvcInfo
{
	string station_number;
	string dev_svc_number;
	string dev_svc_ip;
	int    dev_svc_port;  
};

typedef struct
{
	int    iAlarmLevel;    //报警级别
	string sPhoneNumber;   //电话号码
}SendMSInfo;


typedef struct
{
	string sPhone;       //呼叫电话号码
	string sVoice_text;  //通知内容,call_mode=0时为录音文件的绝对路径,=1是为tts要转换的文本
	string sSms_text;    //短信内容,notice_mode=0时被忽略
	int    iCall_count;  //语音呼叫计数,递增
	string sCall_result; //语音呼叫结果
	tm     tmCreateTime; //数据创建时间
	int    iTask_type;   //0为语音,1为短信,2为语音+短信
	int    iVoice_source;//0为录音文件,1为tts
	string sSms_result;  //短信通知结果
	int    iSms_count;   //短信发送次数
}CallTask;

typedef struct
{
	string sPrgNum;
	string sPrgName;
	int    iPrgType;
	string sFrequency;
	int    iMonit;
	vector<MonitorScheduler> vecMonitorScheduler;
	vector<MonitorScheduler> vecRecoringScheduler;
}PrgInfo;

const char CONST_STR_ITEM_UNIT[][8] = {"W","Kw","Mw"};

enum ActionType
{
    ACTP_SENDMSG = 0,//发送短信
    ACTP_TELE       ,//打电话
    ACTP_SEND_CLIENT,//通知客户端
    ACTP_OPEN_DEVICE,//开设备
    ACTP_CLOSE_DEVICE,//关设备
    ACTP_SWITCH_ANTTEN,//天线切换
    ACTP_SOUND_LIGHT_ALARM,//声光告警
};

#endif
