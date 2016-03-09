#ifndef ERROR_CODE_H
#define ERROR_CODE_H

typedef enum e_MsgEnum
{
	CONNECT_SUCCESS       = 0,//网络连接成功
	CONNECT_TIMEOUT       = 1,//网络连接超时
	CONNECT_ABORT         = 2,//网络中断
	CONNECT_RECONNETING   = 3,//正在重新连接

	CONNECT_READ_HEAD_ERROR_ABORT =5,
	CONNECT_READ_BODY_ERROR_ABORT =6,

	RUN_STATE_UNKNOWN      =7,//状态未知
	RUN_STATE_SHUTDOWN     =8,//关闭
	RUN_STATE_RUNNING      =9,//正在运行

	RUN_ANTENNA_POS_BACKUP =10,//备机
	RUN_ANTENNA_POS_HOST   =11,//主机
	RUN_ANTENNA_MOVING     =12,//正在移动
}NET_EVENT_RESULT;



#endif