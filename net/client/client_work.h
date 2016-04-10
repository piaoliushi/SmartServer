#ifndef CLIENT_WORK
#define CLIENT_WORK
#pragma once
#include "include.h"
#include "../message.h"
#include "../Worker.h"
#include "../taskqueue.h"
#include "../net_session.h"
 #include "../../protocol/protocol.pb.h"
 #include "../config.h"
#include "../SvcMgr.h"
#include "../../LocalConfig.h"
#include <QString>
using namespace hx_net;
class client_work:public Worker<msgPointer> 
{
public:
	client_work(TaskQueue<msgPointer>& _taskqueue,std::size_t _maxthreads = 4) :
	  Worker<msgPointer>(_taskqueue, _maxthreads){}

	~client_work(void){};
public:
	bool work(msgPointer& task)
	{
		if(task==NULL)
			return true;
		session_ptr psession;
		task->getsession(psession);
		if (!psession) 
			return true;

		packHeadPtr MsgPtr = task->msg();
		
		switch (MsgPtr->type)
		{
		case MSG_HEARTBEAT_REQ:
			break;
		case MSG_HEARTBEAT_ACK:
			break;
		case MSG_LOGIN_REQ://客户端及服务器注册请求
			break;
		case MSG_LOGOUT_REQ://客户端及服务器注销请求
			break;
		case MSG_TRANSMITTER_TURNON_OPR://发射机开启(默认高功率开机)
		case MSG_TRANSMITTER_MIDDLE_POWER_TURNON_OPR://中功率开机
		case MSG_TRANSMITTER_LOW_POWER_TURNON_OPR://低功率开机
		case MSG_TRANSMITTER_TURNOFF_OPR://发射机关闭
			break;
		case MSG_ANTENNA_HTOB_OPR:
		case MSG_ANTENNA_BTOH_OPR:
			break;
		case MSG_TRANSMITTER_RISE_POWER_OPR://发射机升功率
			break;
		case MSG_TRANSMITTER_REDUCE_POWER_OPR://发射机降功率
			break;
		case MSG_DEV_REALTIME_DATA_NOTIFY://下级台站发送来的实时设备数据
			break;
		}
		
		return true;
	}
};
#endif
