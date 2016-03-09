#include "SvcMgr.h"
#include "../net/client/DevClientMgr.h"
#include "./database/dbmanager.h"
#include <QMessageBox>
#include "LocalConfig.h"
namespace net
{
	SvcMgr::SvcMgr(void)
		:notify_handler_ptr_(NULL)
		,svc_state_(DEVSVC_STOPPED)
	{

	}

	SvcMgr::~SvcMgr(void)
	{
	}

	bool SvcMgr::IsStarted()
	{
		if(svc_state_==DEVSVC_RUNNING)
			return true;
		return false;
	}

	bool SvcMgr::Start()
	{

		if(IsStarted())
			return true;

		if(!m_pClientMgrPtr)
			m_pClientMgrPtr = boost::shared_ptr<ServerMgr>(new ServerMgr(GetInst(LocalConfig).local_port()));

		if(!m_pDevMgrPtr)
			m_pDevMgrPtr = boost::shared_ptr<DevClientMgr>(new DevClientMgr); 

        svc_state_=DEVSVC_STARTING;
		//启动boost线程池，尺寸2
		GetInst(boost::threadpool::pool).size_controller().resize(2);
		//连接设备
		m_pDevMgrPtr->connect();

		svc_state_ = DEVSVC_RUNNING;
		return true;
	}

	bool SvcMgr::Stop()
	{
		if(!IsStarted())
			return false;

		//清理设备连接服务
		m_pDevMgrPtr.reset();
		//先停止客户端服务器
	    m_pClientMgrPtr->stop_server();	
		//清理客户端服务
		m_pClientMgrPtr.reset();

// 		m_pRecordAgentPtr->unInitAgent();
// 		m_pRecordAgentPtr.reset();
		
		svc_state_ = DEVSVC_STOPPED;
		return true;
	}


	QNotifyHandler* SvcMgr::get_notify()
	{
		return notify_handler_ptr_;
	}

// 	pRecordAgentPtr SvcMgr::record_agent_ptr()
// 	{
// 		return m_pRecordAgentPtr;
// 	}

	void SvcMgr::set_notify(QNotifyHandler* pNotify)
	{
		notify_handler_ptr_=pNotify;
	}

	int SvcMgr::get_modle_online_count()
	{
		if(m_pDevMgrPtr)
			return m_pDevMgrPtr->get_modle_online_count();
		return 0;
	}

	//发送设备数据通知（通知本地客户端，同时上传数据给上级平台）
	void SvcMgr::send_monitor_data_to_client(string sStationid,string sDevid,devDataNfyMsgPtr &dataPtrToClient,
	                                                                         devDataNfyMsgPtr &dataPtrToSvr)
	{
		if(m_pClientMgrPtr)
			m_pClientMgrPtr->SendMonitorData(sStationid,sDevid,dataPtrToClient);
		if(m_pDevMgrPtr)
			m_pDevMgrPtr->send_dev_data(sStationid,sDevid,dataPtrToSvr);
	}

	//发送设备网络连接状态信息到客户端（通知本地客户端，同时上传数据给上级台站）
	void SvcMgr::send_dev_net_state_to_client(string sStationid,string sDevid,devNetNfyMsgPtr &netPtr)
	{
		if(m_pClientMgrPtr)
			m_pClientMgrPtr->SendDevNetStateData(sStationid,sDevid,netPtr);
		if(m_pDevMgrPtr)
			m_pDevMgrPtr->send_dev_net_state_data(sStationid,sDevid,netPtr);
	}

	//发送设备运行状态信息到客户端（通知本地客户端，同时上传数据给上级台站）
	void SvcMgr::send_dev_work_state_to_client(string sStationid,string sDevid,devWorkNfyMsgPtr &workPtr)
	{
		if(m_pClientMgrPtr)
			m_pClientMgrPtr->SendDevWorkStateData(sStationid,sDevid,workPtr);
		if(m_pDevMgrPtr)
			m_pDevMgrPtr->send_dev_work_state_data(sStationid,sDevid,workPtr);
	}
	//发送设备报警状态信息到客户端（通知本地客户端，同时上传数据给上级台站）
	void SvcMgr::send_dev_alarm_state_to_client(string sStationid,string sDevid,devAlarmNfyMsgPtr &alarmPtr)
	{
		if(m_pClientMgrPtr)
			m_pClientMgrPtr->SendDevAlarmStateData(sStationid,sDevid,alarmPtr);
		if(m_pDevMgrPtr)
			m_pDevMgrPtr->send_dev_alarm_state_data(sStationid,sDevid,alarmPtr);
	}
	//发送控制执行结果通知（通知本地客户端，同时上传数据给上级台站）
	void SvcMgr::send_command_execute_result(string sStationid,string sDevid,e_MsgType nMsgType,devCommdRsltPtr &commdRsltPtr)
	{
		if(m_pClientMgrPtr)
			m_pClientMgrPtr->SendCommandExecuteResult(sStationid,sDevid,nMsgType,commdRsltPtr);
		if(m_pDevMgrPtr)
			m_pDevMgrPtr->send_command_execute_result(sStationid,sDevid,nMsgType,commdRsltPtr);
	}
	//获得设备连接
	con_state SvcMgr::get_dev_net_state(string sStationId,string sDevid)
	{
		if(m_pDevMgrPtr)
			return m_pDevMgrPtr->get_dev_net_state(sStationId,sDevid);
		return con_disconnected;
	}
	//获得设备运行状态
	dev_run_state SvcMgr::get_dev_run_state(string sStationId,string sDevid)
	{
		if(m_pDevMgrPtr)
			return m_pDevMgrPtr->get_dev_run_state(sStationId,sDevid);
		return dev_unknown;
	}
	//获得设备运行状态
    void SvcMgr::get_dev_alarm_state(string sStationId,string sDevid,map<int,std::pair<int,tm> >& cellAlarm)
	{
		if(m_pDevMgrPtr)
			m_pDevMgrPtr->get_dev_alarm_state(sStationId,sDevid,cellAlarm);
	}

	//获得设备基本信息
	bool SvcMgr::dev_base_info(string sStationId,DevBaseInfo& devInfo,string sdevId)
	{
		bool bRtValue = false;
		if (m_pDevMgrPtr)
		{
			bRtValue = m_pDevMgrPtr->dev_base_info(sStationId,devInfo,sdevId);
			if(bRtValue==true)
				return true;
		}
		return false;
	}

	//获得子台站设备状态信息
	loginAckMsgPtr  SvcMgr::get_child_station_dev_status()
	{
		if(m_pClientMgrPtr)
			return m_pClientMgrPtr->get_child_station_dev_status();
		return loginAckMsgPtr();
	}

	//执行通用指令
	e_ErrorCode SvcMgr::excute_command(int cmdType,devCommdMsgPtr lpParam)
	{
		if(m_pDevMgrPtr)
		{
			e_ErrorCode eResult = m_pDevMgrPtr->excute_command(cmdType,lpParam);
// 			if(eResult==EC_DEVICE_NOT_FOUND||eResult==EC_OBJECT_NULL)
// 			{
// 				if (m_pClientMgrPtr)
// 					return m_pClientMgrPtr->excute_command(cmdType,lpParam);
// 			}else
				return eResult;
		}
		return EC_OBJECT_NULL;
	}
	//上级查岗
	void SvcMgr::check_station_working(checkWorkingReqMsgPtr pcheckWork)
	{
		if (m_pClientMgrPtr)
			m_pClientMgrPtr->check_station_working(pcheckWork);
		return;
	}
	//向上提交查岗结果
	void SvcMgr::commit_check_working_result(checkWorkingNotifyMsgPtr pcheckWorkResult)
	{
		if(m_pDevMgrPtr)
			m_pDevMgrPtr->commit_check_working_result(pcheckWorkResult);
		return;
	}

	//是否是通过上级平台直连下级平台设备
	bool SvcMgr::is_direct_connect_device(string sStationId,string sDevNumber)
	{
		if(m_pDevMgrPtr)
			return m_pDevMgrPtr->is_direct_connect_device(sStationId,sDevNumber);
		return false;
	}
}
