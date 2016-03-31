#ifndef DEV_CLIENT_H
#define DEV_CLIENT_H

#pragma once

#include "../io_service_pool.h"
#include "../../qnotifyhandler.h"
//#include <urdl/read_stream.hpp>
#include "http_request_session.h"
using boost::asio::ip::tcp;

namespace hx_net
{
	class DevClient
	{
	public:
		struct DevKey
		{
			DevKey(string _stId,string _devId)
			{
				stationId =  _stId;
				devId = _devId;
			}
			bool operator == (const DevKey &other)const
			{
				if(other.stationId == stationId
					&& other.devId == devId)
					return true;
				return false;
			}

			bool operator < (const DevKey &other)const
			{
				if( stationId<other.stationId )
					return true;
				else if(stationId==other.stationId)
				{
					return devId<other.devId;
				}
				return false;
			}

			string stationId;
			string devId;
		};

		//初始化一个客户端，该客户端维护一个任务队列，并创建一个io_service
		DevClient(TaskQueue<msgPointer>& taskwork,size_t io_service_pool_size=2);
		~DevClient();
	public:
		void run();//启动io_service pool
		void stop();
		void connect_all();
		void disconnect_all();
		
		//通用命令执行
		e_ErrorCode excute_command(int cmdType,devCommdMsgPtr lpParam);

		//获取转换模块在线数
		int get_modle_online_count();

		//获得设备连接
		con_state get_dev_net_state(string sStationId,string sDevid);
		//获得设备运行状态
		dev_run_state get_dev_run_state(string sStationId,string sDevid);
		//获得设备运行状态
        void get_dev_alarm_state(string sStationId,string sDevid,map<int,std::pair<int,tm> >& cellAlarm);
		//获得设备类型
		bool dev_base_info(string sStationId,DevBaseInfo& devInfo,string sDevid="local");
		//----------------------relay server-------------------------------------------------//
		//连接上级服务器
		void connect_relay_server();
		//连接告警决策服务器
		void connect_alarm_server();
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
		//向上提交查岗结果
		void commit_check_working_result(checkWorkingNotifyMsgPtr pcheckWorkResult);
		//---------------------------end-----------------------------------------------------//
		//是否是通过上级平台直连下级平台设备
		bool is_direct_connect_device(string sStationId,string sDevNumber);
    protected:

	private:
		TaskQueue<msgPointer>&         taskwork_;
		boost::recursive_mutex         device_pool_mutex_;
		std::map<DevKey,session_ptr>   device_pool_;
		io_service_pool                io_service_pool_;

//---------------relay server---------------------------------------//
		boost::recursive_mutex         relay_svc_session_mutex_;//级联服务器连接器互斥量
		session_ptr                    relay_server_seesion_;   //级联服务连接对象
//---------------alarm server---------------------------------------//
		boost::recursive_mutex         alarm_svc_session_mutex_;//告警服务器连接器互斥量
		session_ptr                    alarm_server_seesion_;   //告警服务连接对象


        http_request_session_ptr      http_request_session_ptr_;
	};
}

#endif
