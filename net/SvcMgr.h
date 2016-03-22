#ifndef SVC_MGR_H
#define SVC_MGR_H

#pragma once
#include "../qnotifyhandler.h"
#include "./client/DevClientMgr.h"
#include"./server/ServerMgr.h"
//#include "../database/dbmanager.h"
#include "include.h"
//#include "RecordAgent.h"
namespace net
{
	//typedef boost::shared_ptr<RecordAgent>  pRecordAgentPtr;
	class SvcMgr
	{
		enum svc_state
		{
			DEVSVC_RUNNING, //服务正在运行
			DEVSVC_STOPPED, //服务已停止
			DEVSVC_STARTING,//服务正在启动
			DEVSVC_STOPPING,//服务正在停止
		};
	public:
		SvcMgr(void);
		~SvcMgr(void);
	public:

		//启动服务器
		bool Start();
		//停止服务器
		bool Stop();
		//是否已启动
		bool IsStarted();
		//是否正在启动
		bool IsStarting();

		QNotifyHandler* get_notify();

		//pRecordAgentPtr record_agent_ptr();

		void set_notify(QNotifyHandler* pNotify);
        //获得已运行天线数
		//int get_antenna_running_count();
		//获得串口服务器在线数
		int get_modle_online_count();
		//获取设备基本信息
		bool dev_base_info(string sStationId,DevBaseInfo& devInfo,string sdevId="local");
		//获得设备连接状态
		con_state get_dev_net_state(string sStationId,string sDevid);
		//获得设备运行状态
		dev_run_state get_dev_run_state(string sStationId,string sDevid);
		//获得设备运行状态
        void get_dev_alarm_state(string sStationId,string sDevid,map<int,std::pair<int,tm> >& cellAlarm);
		
		//发送设备数据通知
		void send_monitor_data_to_client(string sStationid,string sDevid,devDataNfyMsgPtr &dataPtrToClient,devDataNfyMsgPtr &dataPtrToSvr);
		//发送设备网络连接状态信息到客户端
		void send_dev_net_state_to_client(string sStationid,string sDevid,devNetNfyMsgPtr &netPtr);
		//发送设备运行状态信息到客户端
		void send_dev_work_state_to_client(string sStationid,string sDevid,devWorkNfyMsgPtr &workPtr);
		//发送设备报警状态信息到客户端
		void send_dev_alarm_state_to_client(string sStationid,string sDevid,devAlarmNfyMsgPtr &alarmPtr);
		//发送控制执行结果通知
		void send_command_execute_result(string sStationid,string sDevid,e_MsgType nMsgType,devCommdRsltPtr &commdRsltPtr);
										 
		//void BroadcastMessage(e_MsgType _type,googleMsgPtr gMsgPtr);//google::protobuf::Message *
		//获得子台站设备状态信息
		loginAckMsgPtr  get_child_station_dev_status();
		//执行通用指令
		e_ErrorCode excute_command(int cmdType,devCommdMsgPtr lpParam);
		//上级查岗
		void check_station_working(checkWorkingReqMsgPtr pcheckWork);
		//向上提交查岗结果
		void commit_check_working_result(checkWorkingNotifyMsgPtr pcheckWorkResult);
		//是否是通过上级平台直连下级平台设备
		bool is_direct_connect_device(string sStationId,string sDevNumber);
	private:
		svc_state svc_state_;
		boost::shared_ptr<DevClientMgr> m_pDevMgrPtr;//设备(被动连接)管理对象
		boost::shared_ptr<ServerMgr> m_pClientMgrPtr;//客户端服务、上传服务、上传设备管理对象
		
		//pRecordAgentPtr m_pRecordAgentPtr;//录制服务代理对象
		QNotifyHandler* notify_handler_ptr_;
	};
}
#endif 
