#ifndef DEV_CLIENT_MGR_H
#define DEV_CLIENT_MGR_H

#pragma once

#include "../io_service_pool.h"
#include <boost/shared_ptr.hpp>
#include "DevClient.h"

//using namespace boost::network::http;

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
        //下发控制指令
        e_ErrorCode start_exec_task(string sDevId,string sUser,int cmdType);

		void RunNetListen();

		int get_modle_online_count();

		//获得设备连接
		con_state get_dev_net_state(string sStationId,string sDevid);
		//获得设备运行状态
		dev_run_state get_dev_run_state(string sStationId,string sDevid);
		//获得设备运行状态
        void get_dev_alarm_state(string sStationId,string sDevid,map<int,map<int,CurItemAlarmInfo> >& cellAlarm);
		//获得设备基本信息
		bool dev_base_info(string sStationId,DevBaseInfo& devInfo,string sdevId="local");
        //更新运行图
       e_ErrorCode update_monitor_time(string &sDevId,map<int,vector<Monitoring_Scheduler> >& monitorScheduler,
                                 vector<Command_Scheduler> &cmmdScheduler);
       //更新告警配置
      e_ErrorCode update_dev_alarm_config(string &sDevId,DeviceInfo &devInfo);
	private:
		boost::shared_ptr<boost::thread> _listenthreadptr;//网络监听线程
        //boost::shared_ptr<boost::thread> _workthreadptr;//工作线程
        //boost::shared_ptr<client_work> _workerptr;//当前用户任务UserWork
        boost::shared_ptr<DevClient> _devclientptr;//设备服务对象
	};
}
#endif
