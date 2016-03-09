#ifndef CLIENT_WORK
#define CLIENT_WORK
#pragma once
#include "include.h"
#include "../message.h"
#include "../Worker.h"
#include "../taskqueue.h"
#include "../session.h"
 #include "../../protocol/protocol.pb.h"
 #include "../config.h"
 #include "../../database/dbmanager.h"
#include "../SvcMgr.h"
#include "../../LocalConfig.h"
//#include "connect_handler.h"
#include <QString>
using namespace net;
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
			psession->heart_beat_rsp();
			break;
		case MSG_HEARTBEAT_ACK:
			psession->heart_beat_rsp();
			break;
		case MSG_LOGIN_REQ://客户端及服务器注册请求
			{
				LoginReq rlogin;
				rlogin.ParseFromArray(task->body(),task->bodySize());
				string sUser = GetInst(LocalConfig).local_station_id();
				string sUsr = sUser;
                string sPsw = rlogin.susrpsw();//QString::fromUtf8(rlogin.susrpsw().c_str()).toLocal8Bit();
				psession->login_user_ack(sUsr,sPsw,task);
			}
			break;
		case MSG_LOGOUT_REQ://客户端及服务器注销请求
			{
				LogoutReq rlogout;
				rlogout.ParseFromArray(task->body(),task->bodySize());
				psession->logout_user_ack(rlogout.susrname(),rlogout.susrpsw(),task);
			}
			break;
		case MSG_LOGIN_ACK://服务注册响应,分析并保存下级台站状态信息
			{//不验证用户名，密码
				string sUser = GetInst(LocalConfig).local_station_id();
				psession->login_user_ack(sUser,"childserver",task);
			}
			break;
		case MSG_LOGOUT_ACK://服务退出响应
			{//不验证用户名，密码
				string sUser = GetInst(LocalConfig).local_station_id();
				psession->logout_user_ack(sUser,"childserver",task);
			}
			break;
		case MSG_TRANSMITTER_TURNON_OPR://发射机开启(默认高功率开机)
		case MSG_TRANSMITTER_MIDDLE_POWER_TURNON_OPR://中功率开机
		case MSG_TRANSMITTER_LOW_POWER_TURNON_OPR://低功率开机
			break;
		case MSG_TRANSMITTER_TURNOFF_OPR://发射机关闭
			{
// 				DeviceCommandMsg commandmsg_;
// 				commandmsg_.ParseFromArray(task->body(),task->bodySize());
// 				string sUsr = QString::fromUtf8(commandmsg_.soperuser().c_str()).toLocal8Bit();
// 				e_ErrorCode nReult = GetInst(SvcMgr).turn_off_transmitter(commandmsg_.sstationid(),
// 											commandmsg_.sdevid(),sUsr);
			}
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
		case MSG_CHECK_WORKING_REQ://上级查岗
			{
				checkWorkingReqMsgPtr checkWorkPtr = checkWorkingReqMsgPtr(new CheckStationWorkingReq);
				checkWorkPtr->ParseFromArray(task->body(),task->bodySize());
				psession->check_station_working(checkWorkPtr);
			}
			break;
		}
		
		return true;
	}
};
#endif
