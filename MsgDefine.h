#ifndef MSG_DEFINE_H
#define MSG_DEFINE_H

//#include "include.h"
#include <boost/cstdint.hpp>

#define HeadSize    16              //不包括数据开始字节.
#define UPLOAD_MSG_HEAD_SIZE 7      //设备上传数据头
#define MaxBodySize 1024000         //

#pragma pack(push, 1)	           //这里,保存了该对齐方式,1字节对齐.
typedef struct _tagHead
{
	boost::uint32_t type;          //封包类型（消息类型）
	boost::uint32_t packsize;      //封包大小,包含该头大小,数据包大小计算方法: datasize = packsize - packheadsize.
	boost::uint32_t sourcetype;    //0:客户端，1:设备监测服务，2:流媒体服务
	boost::uint32_t reserve1;      //保留1.
}* packHeadPtr, packHead;
#pragma pack(pop)	               //如果保存了对齐方式,用这个.

#pragma pack(push, 1)	           //这里,保存了该对齐方式,1字节对齐.
typedef struct _tagDevUploadHead
{
	boost::uint8_t  startCode;
	boost::uint8_t  productCode;
	boost::uint8_t  commandCode;
	boost::uint16_t devAddress;
	boost::uint16_t msgBodyLen;

} *DevUploadHeadPtr,DevUploadHead;
#pragma pack(pop)	               //如果保存了对齐方式,用这个.
//message type define
enum e_MsgType
{
    MSG_DEVICE_QUERY = 0,//查询
	MSG_LOGIN_REQ     = 1, // 登录
	MSG_LOGIN_ACK	  = 2,
	MSG_HEARTBEAT_REQ = 3, // 心跳
	MSG_HEARTBEAT_ACK = 4,
	MSG_LOGOUT_REQ	  = 5, // 注销
	MSG_LOGOUT_ACK	  = 6,
	MSG_CONFIG_REQ    = 7, // 配置请求
	MSG_CONFIG_ACK    = 8,


	MSG_DEV_REALTIME_DATA_NOTIFY = 9, //设备实时数据通知
	MSG_DEV_NET_STATE_NOTIFY     = 10,//设备网络连接状态通知
	MSG_DEV_WORK_STATE_NOTIFY    = 11,//设备工作状态通知
	MSG_DEV_ALARM_STATE_NOTIFY   = 12,//设备报警状态通知

	MSG_DEV_TURNON_OPR   = 13,//设备开启(只用于非发射机设备)
	MSG_DEV_TURNOFF_OPR  = 14,//设备关闭(只用于非发射机设备)

    MSG_TRANSMITTER_TURNON_OPR              = 15,//发射机开(默认高功率开)
	MSG_TRANSMITTER_MIDDLE_POWER_TURNON_OPR = 16,//发射机中功率开机
	MSG_TRANSMITTER_LOW_POWER_TURNON_OPR    = 17,//发射机低功率开机
	MSG_TRANSMITTER_TURNON_ACK       = 18,

	MSG_TRANSMITTER_TURNOFF_OPR      = 19,//发射机关
	MSG_TRANSMITTER_TURNOFF_ACK      = 20,//发射机关

	MSG_TRANSMITTER_RISE_POWER_OPR   = 21,//升功率
	MSG_TRANSMITTER_RISE_POWER_ACK   = 22,//升功率

	MSG_TRANSMITTER_REDUCE_POWER_OPR = 23,//降功率
	MSG_TRANSMITTER_REDUCE_POWER_ACK = 24,//降功率

	MSG_ANTENNA_HTOB_OPR  = 25,//天线切换(主->备)
	MSG_ANTENNA_HTOB_ACK  = 26,//天线切换(主->备)

	MSG_ANTENNA_BTOH_OPR  = 27,//天线切换(备->主)
	MSG_ANTENNA_BTOH_ACK  = 28,//天线切换(主->备)

	MSG_CHECK_WORKING_NOTIFY     = 29,//查岗通知

	MSG_SWITCH_AUDIO_CHANNEL_OPR = 30,//切换音频通道
	MSG_SWITCH_AUDIO_CHANNEL_ACK = 31,//切换音频通道响应

	MSG_SET_FREQUENCY_OPR = 32,//设置频率
	MSG_SET_FREQUENCY_ACK = 33,//设置频率响应

	MSG_SEARCH_FREQUENCY_OPR = 34,//搜索频率
	MSG_SEARCH_FREQUENCY_ACK = 35,

	MSG_STOP_PLAY_AUDIO = 36,//停止声音播放(客户端命令)

	MSG_0401_SWITCH_OPR = 37,//0401DA通道切换
	MSG_0401_SWITCH_ACK = 38,

	MSG_CONTROL_MOD_SWITCH_OPR = 39,//手/自动模式切换
	MSG_CONTROL_MOD_SWITCH_ACK = 40,

	MSG_0401_ALARM_SWITCH_TIME_SET_OPR = 41,//0401DA报警切换时间设置
	MSG_0401_ALARM_SWITCH_TIME_SET_ACK = 42,//

	MSG_ADJUST_TIME_SET_OPR = 43,//0401DA报警切换时间设置
	MSG_ADJUST_TIME_SET_ACK = 44,//

	MSG_GENERAL_COMMAND_OPR = 45,
	MSG_GENERAL_COMMAND_ACK = 46,

    MSG_HANDOVER_REQ      = 47,//交接班请求
    MSG_HANDOVER_ACK      = 48,//交接班回复

	MSG_SIGN_IN_OUT_REQ   = 49,//签到请求
	MSG_SIGN_IN_OUT_ACK   = 50,//签到回复

	MSG_DUTY_LOG_REQ      = 51,//值班日志
	MSG_DUTY_LOG_ACK      = 52,//值班日志响应

	MSG_CHECK_WORKING_REQ = 53,//查岗请求
	MSG_CHECK_WORKING_ACK = 54,//查岗回复

    MSG_DEV_RESET_OPR     = 55,//复位指令
    MSG_DEV_RESET_ACK     = 56,//复位响应

    MSG_REMIND_NOTIFY     = 57,//提醒通知
    MSG_REMIND_NOTIFY_ACK = 58,//提醒响应
};

enum {
    VALUE_TYPE_STRING  = 0,
    VALUE_TYPE_DOUBLE  = 1,
    VALUE_TYPE_BOOL    = 2,
    VALUE_TYPE_INT     = 3,
};

#endif
