#ifndef SERVER_MGR_H
#define SERVER_MGR_H

#pragma once

#include "../io_service_pool.h"
#include <boost/network/include/http/server.hpp>
#include <boost/network/uri.hpp>
#include "local_server.h"
#include "./http/CommonPrecomp.h"

class server_work;
namespace net
{
	
	class ServerMgr
	{
	public:
		ServerMgr(int port=5000);
		~ServerMgr();

		void RunNetListen();
		void RunTasks();
		//用户登录
		void Login(session_ptr ch_ptr,string usr,string psw,LoginAck &loginAck);
		//用户注销
		void Logout(session_ptr ch_ptr,string usr,string psw,LogoutAck &logoutAck);

		//发送设备数据通知
		void SendMonitorData(string sStationid,string sDevid,devDataNfyMsgPtr &dataPtr);

		//发送设备连接状态通知
		void SendDevNetStateData(string sStationid,string sDevid,devNetNfyMsgPtr &netPtr);

		//发送设备运行状态通知
		void SendDevWorkStateData(string sStationid,string sDevid,devWorkNfyMsgPtr &workPtr);

		//发送报警状态通知
		void SendDevAlarmStateData(string sStationid,string sDevid,devAlarmNfyMsgPtr &alarmPtr);

		//发送控制执行结果通知
		void SendCommandExecuteResult(string sStationid,string sDevid,e_MsgType nMsgType
			                                                            ,devCommdRsltPtr &commdRsltPtr);

		//获得子台站设备状态信息
		loginAckMsgPtr  get_child_station_dev_status();

		//停止
		void stop_server();

		//上级查岗
		void check_station_working(checkWorkingReqMsgPtr pcheckWork);

	private:
		boost::shared_ptr<boost::thread> _listenthreadptr;//网络监听线程
		boost::shared_ptr<boost::thread> _workthreadptr;//工作线程
		boost::shared_ptr<TaskQueue<msgPointer> > _taskqueueptr;//任务队列
		boost::shared_ptr<server_work> _workerptr;//当前用户任务
        boost::shared_ptr<LocalServer> _serverptr;//服务对象

		hx_http_server_ptr   _httpserverptr;//http服务对象
		test_handler        *_p_test_handler;
		utils::thread_pool   _thread_pool;
	};
}

#endif
