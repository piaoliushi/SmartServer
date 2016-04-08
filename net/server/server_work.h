#ifndef SERVER_WORK
#define SERVER_WORK
#pragma once
#include "include.h"
#include "../message.h"
#include "../Worker.h"
#include "../taskqueue.h"
#include "../net_session.h"
#include "../../protocol/protocol.pb.h"
#include "../config.h"
//#include "../../database/dbmanager.h"
#include "../SvcMgr.h"
//#include "connect_handler.h"
#include <QString>
using namespace hx_net;
class server_work:public Worker<msgPointer>
{
public:
	server_work(TaskQueue<msgPointer>& _taskqueue,std::size_t _maxthreads = 4) :
	  Worker<msgPointer>(_taskqueue, _maxthreads){}

	~server_work(void){};
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
		case MSG_LOGIN_REQ://客户端及服务器注册请求
			{
				LoginReq rlogin;
				rlogin.ParseFromArray(task->body(),task->bodySize());
                string sUsr = rlogin.susrname();
                string sPsw = rlogin.susrpsw();
				psession->login_user_ack(sUsr,sPsw,task);
			}
			break;
		case MSG_HANDOVER_REQ://交接班请求
			{
				HandOverReq rhandover;
				rhandover.ParseFromArray(task->body(),task->bodySize());
                string sOldUsr = rhandover.scurusrname();
                string sNewUsr = rhandover.snewusrname();
                string sNewPsw = rhandover.snewusrpsw();
				psession->handover_ack(sOldUsr,sNewUsr,sNewPsw,task);
			}
			break;
		case MSG_SIGN_IN_OUT_REQ://签到请求
			{
				SignInOutReq signInoutReq;
				signInoutReq.ParseFromArray(task->body(),task->bodySize());
				int nType = signInoutReq.issignin();
                string sUser = signInoutReq.ssigninname();
                string sPsw = signInoutReq.ssigninpsw();
				psession->user_sign_in_out_ack(nType,sUser,sPsw);
			}
			break;
		case MSG_DUTY_LOG_REQ://值班日志写入boost::dynamic_pointer_cast
			{
				DutyLogReq dutyReq;
				dutyReq.ParseFromArray(task->body(),task->bodySize());
                string sUserId = dutyReq.scurusrnumber();
                string sContent = dutyReq.scontent();
				int    nType = dutyReq.ntype();
				psession->user_duty_log(sUserId,sContent,nType);
			}
			break;
		case MSG_CHECK_WORKING_REQ://查岗请求
			{
				checkWorkingReqMsgPtr checkWorkPtr = checkWorkingReqMsgPtr(new CheckStationWorkingReq);
				checkWorkPtr->ParseFromArray(task->body(),task->bodySize());
				psession->check_station_working(checkWorkPtr);
			}
			break;
		case MSG_CHECK_WORKING_NOTIFY://查岗通知
			{
				checkWorkingNotifyMsgPtr checkWorkResultPtr = checkWorkingNotifyMsgPtr(new CheackStationWorkingNotify);
				checkWorkResultPtr->ParseFromArray(task->body(),task->bodySize());
				psession->check_station_result_notify(checkWorkResultPtr);
			}
			break;
		case MSG_LOGOUT_REQ://客户端及服务器注册请求
			{
				LogoutReq rlogout;
				rlogout.ParseFromArray(task->body(),task->bodySize());
				psession->logout_user_ack(rlogout.susrname(),rlogout.susrpsw(),task);
			}
			break;
		case MSG_LOGIN_ACK://服务注册响应,分析并保存下级台站状态信息
			psession->login_user_ack("HigherServer","123456",task);
			break;
		case MSG_LOGOUT_ACK://服务退出响应不验证用户名，密码
			psession->logout_user_ack("HigherServer","123456",task);
			break;
		case MSG_TRANSMITTER_TURNON_OPR://发射机开启(默认高功率开机)
		case MSG_TRANSMITTER_MIDDLE_POWER_TURNON_OPR://中功率开机
		case MSG_TRANSMITTER_LOW_POWER_TURNON_OPR://低功率开机
		case MSG_TRANSMITTER_TURNOFF_OPR://发射机关闭
		case MSG_ANTENNA_HTOB_OPR:
		case MSG_ANTENNA_BTOH_OPR:
		case MSG_TRANSMITTER_RISE_POWER_OPR://发射机升功率
		case MSG_TRANSMITTER_REDUCE_POWER_OPR://发射机降功率
        case MSG_SET_FREQUENCY_OPR:
        case MSG_SEARCH_FREQUENCY_OPR:
        case MSG_0401_SWITCH_OPR:
        case MSG_CONTROL_MOD_SWITCH_OPR:
        case MSG_ADJUST_TIME_SET_OPR:
        case MSG_GENERAL_COMMAND_OPR:
        case MSG_SWITCH_AUDIO_CHANNEL_OPR://切换音频通道
        case MSG_DEV_RESET_OPR:
        {
            devCommdMsgPtr commandmsg_(new DeviceCommandMsg);
            commandmsg_->ParseFromArray(task->body(),task->bodySize());
            string sUsr = commandmsg_->soperuser();
            e_ErrorCode nReult = GetInst(SvcMgr).start_exec_task(commandmsg_->sdevid(),sUsr,MsgPtr->type);
            devCommdRsltPtr ackMsg(new DeviceCommandResultNotify);
            ackMsg->set_sstationid(commandmsg_->sstationid());
            ackMsg->set_sdevid(commandmsg_->sdevid());
            ackMsg->set_sdevname(commandmsg_->sdevname());
            ackMsg->set_edevtype(commandmsg_->edevtype());
            ackMsg->set_eerrorid(nReult);
            ackMsg->set_soperuser(commandmsg_->soperuser());
            psession->sendMessage((e_MsgType)MsgPtr->type,ackMsg);
        }
        break;
		case MSG_DEV_REALTIME_DATA_NOTIFY://下级台站实时设备数据
			psession->dev_data_notify(task);
			break;
		case MSG_DEV_NET_STATE_NOTIFY://下级台站设备连接状态
			psession->dev_netstate_update(task);
			break;
		case MSG_DEV_WORK_STATE_NOTIFY://下级台站设备工作状态
			psession->dev_workstate_update(task);
			break;
		case MSG_DEV_ALARM_STATE_NOTIFY://下级台站设备报警状态
			psession->dev_alarmstate_update(task);
			break;

		}
		
		return true;
	}
};
#endif
