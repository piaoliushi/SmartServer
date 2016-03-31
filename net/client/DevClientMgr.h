#ifndef DEV_CLIENT_MGR_H
#define DEV_CLIENT_MGR_H

#pragma once

#include "../io_service_pool.h"
#include <boost/shared_ptr.hpp>
//UserWork
#include "DevClient.h"
//#include "SvcClient.h"
//#include "../../DataType.h"
//#define     http_client;
using namespace boost::network::http;

//typedef  http::client   hx_http_client;

class client_work;
class client;
namespace hx_net
{
	
	class DevClientMgr
	{
	public:
		DevClientMgr();
		~DevClientMgr();

		void connect(std::string ip = "127.0.0.1",unsigned short port=5000);

		void disconnect();

		//通用命令执行
		e_ErrorCode excute_command(int cmdType,devCommdMsgPtr lpParam);
		//向上提交查岗结果
		void commit_check_working_result(checkWorkingNotifyMsgPtr pcheckWorkResult);

		//是否是通过上级平台直连下级平台设备
		bool is_direct_connect_device(string sStationId,string sDevNumber);

		void RunNetListen();

		void RunTasks();

		int get_modle_online_count();

		//获得设备连接
		con_state get_dev_net_state(string sStationId,string sDevid);
		//获得设备运行状态
		dev_run_state get_dev_run_state(string sStationId,string sDevid);
		//获得设备运行状态
        void get_dev_alarm_state(string sStationId,string sDevid,map<int,std::pair<int,tm> >& cellAlarm);
		//获得设备基本信息
		bool dev_base_info(string sStationId,DevBaseInfo& devInfo,string sdevId="local");


		//----------------------relay server-------------------------------------------------//
		//发送设备数据通知
		void send_dev_data(string sStationid,string sDevid,devDataNfyMsgPtr &dataPtr);
		//发送设备网络状态数据
		void send_dev_net_state_data(string sStationid,string sDevid,devNetNfyMsgPtr &netPtr);
		//发送设备运行状态数据
		void send_dev_work_state_data(string sStationid,string sDevid,devWorkNfyMsgPtr &workPtr);
		//发送设备报警状态数据
		void send_dev_alarm_state_data(string sStationid,string sDevid,devAlarmNfyMsgPtr &alarmPtr);
		//发送控制执行结果通知
		void send_command_execute_result(string sStationid,string sDevid,e_MsgType nMsgType,devCommdRsltPtr &commdRsltPtr);
		//---------------------------end-----------------------------------------------------//
	private:
		boost::shared_ptr<boost::thread> _listenthreadptr;//网络监听线程
		boost::shared_ptr<boost::thread> _workthreadptr;//工作线程
		boost::shared_ptr<TaskQueue<msgPointer> > _taskqueueptr;//任务队列
		boost::shared_ptr<client_work> _workerptr;//当前用户任务UserWork
		boost::shared_ptr<DevClient> _devclientptr;//设备服务对象
	};
}
#endif
