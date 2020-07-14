#include "SvcMgr.h"
#include "../net/client/DevClientMgr.h"
//#include "./database/dbmanager.h"
#include <QMessageBox>
#include "LocalConfig.h"
namespace hx_net
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

		svc_state_ = DEVSVC_STOPPED;
		return true;
	}


	QNotifyHandler* SvcMgr::get_notify()
	{
		return notify_handler_ptr_;
	}

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
    void SvcMgr::send_monitor_data_to_client(string sStationid,string sDevid,devDataNfyMsgPtr &dataPtrToClient)
	{
		if(m_pClientMgrPtr)
			m_pClientMgrPtr->SendMonitorData(sStationid,sDevid,dataPtrToClient);
	}

	//发送设备网络连接状态信息到客户端（通知本地客户端，同时上传数据给上级台站）
	void SvcMgr::send_dev_net_state_to_client(string sStationid,string sDevid,devNetNfyMsgPtr &netPtr)
	{
		if(m_pClientMgrPtr)
			m_pClientMgrPtr->SendDevNetStateData(sStationid,sDevid,netPtr);
	}

	//发送设备运行状态信息到客户端（通知本地客户端，同时上传数据给上级台站）
	void SvcMgr::send_dev_work_state_to_client(string sStationid,string sDevid,devWorkNfyMsgPtr &workPtr)
	{
		if(m_pClientMgrPtr)
			m_pClientMgrPtr->SendDevWorkStateData(sStationid,sDevid,workPtr);
	}
	//发送设备报警状态信息到客户端（通知本地客户端，同时上传数据给上级台站）
	void SvcMgr::send_dev_alarm_state_to_client(string sStationid,string sDevid,devAlarmNfyMsgPtr &alarmPtr)
	{
		if(m_pClientMgrPtr)
			m_pClientMgrPtr->SendDevAlarmStateData(sStationid,sDevid,alarmPtr);
	}
	//发送控制执行结果通知（通知本地客户端，同时上传数据给上级台站）
	void SvcMgr::send_command_execute_result(string sStationid,string sDevid,e_MsgType nMsgType,devCommdRsltPtr &commdRsltPtr)
	{
		if(m_pClientMgrPtr)
			m_pClientMgrPtr->SendCommandExecuteResult(sStationid,sDevid,nMsgType,commdRsltPtr);
	}
	//获得设备连接
	con_state SvcMgr::get_dev_net_state(string sStationId,string sDevid)
	{
		if(m_pDevMgrPtr)
			return m_pDevMgrPtr->get_dev_net_state(sStationId,sDevid);
		return con_disconnected;
	}

    //获得设备数据返回状态
    con_state SvcMgr::get_data_return_state(string sStationId,string sDevid)
    {
        if(m_pDevMgrPtr)
            return m_pDevMgrPtr->get_data_return_state(sStationId,sDevid);
        return con_disconnected;
    }

	//获得设备运行状态
	dev_run_state SvcMgr::get_dev_run_state(string sStationId,string sDevid)
	{
		if(m_pDevMgrPtr)
			return m_pDevMgrPtr->get_dev_run_state(sStationId,sDevid);
		return dev_unknown;
	}

    //设置设备运行状态
    void SvcMgr::set_dev_run_state(string sStationId,string sDevid,int nState)
    {
        if(m_pDevMgrPtr)
            return m_pDevMgrPtr->set_dev_run_state(sStationId,sDevid,nState);
    }

    //获得设备是否允许控制
    bool SvcMgr::dev_can_excute_cmd(string sStationId,string sDevid)
    {
        if(m_pDevMgrPtr)
            return m_pDevMgrPtr->dev_can_excute_cmd(sStationId,sDevid);
        return false;
    }

    //获取设备当前命令执行状态
    int SvcMgr::get_dev_opr_state(string sStationId,string sDevid)
    {
        if(m_pDevMgrPtr)
            return m_pDevMgrPtr->get_dev_opr_state(sStationId,sDevid);
        return -1;
    }

	//获得设备运行状态
    void SvcMgr::get_dev_alarm_state(string sStationId,string sDevid,map<int,map<int,CurItemAlarmInfo> >& cellAlarm)
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
            return m_pDevMgrPtr->dev_base_info(sStationId,devInfo,sdevId);
		}
		return false;
	}

	//执行通用指令
    e_ErrorCode SvcMgr::excute_command(string sDevId,int cmdType,string sUser,devCommdMsgPtr lpParam)
	{
		if(m_pDevMgrPtr)
		{
            e_ErrorCode eResult = m_pDevMgrPtr->excute_command(sDevId,cmdType,sUser,lpParam);
            return eResult;
		}
		return EC_OBJECT_NULL;
	}

    e_ErrorCode SvcMgr::start_exec_task(string sDevId,string sUser,int cmdType,map<int,string> &mapParam,int nMode)
    {
        if(m_pDevMgrPtr)
            return m_pDevMgrPtr->start_exec_task(sDevId,sUser,cmdType,mapParam,nMode);
        return EC_OBJECT_NULL;
    }

    //更新运行图
   e_ErrorCode SvcMgr::update_monitor_time(string  sDevId,map<int,vector<Monitoring_Scheduler> >& monitorScheduler,
                             vector<Command_Scheduler> &cmmdScheduler)
   {
       if(m_pDevMgrPtr)
           return m_pDevMgrPtr->update_monitor_time(sDevId,monitorScheduler,cmmdScheduler);
       return EC_OBJECT_NULL;
   }

   //更新告警配置
  e_ErrorCode SvcMgr::update_dev_alarm_config(string sDevId,DeviceInfo &devInfo)
  {
      if(m_pDevMgrPtr)
          return m_pDevMgrPtr->update_dev_alarm_config(sDevId,devInfo);
      return EC_OBJECT_NULL;
  }

  //上报http消息
  e_ErrorCode   SvcMgr::response_http_msg(string sUrl,string &sContent,string sRqstType)
  {
      if(m_pDevMgrPtr)
          return m_pDevMgrPtr->response_http_msg(sUrl,sContent,sRqstType);
      return EC_OBJECT_NULL;
  }

  //发送短信
  e_ErrorCode    SvcMgr::SendSMSContent(vector<string> &PhoneNumber,string AlarmContent){
      if(m_pDevMgrPtr){

          m_pDevMgrPtr->SendSMSContent(PhoneNumber,AlarmContent);
      }
      return EC_OBJECT_NULL;
  }

  e_ErrorCode  SvcMgr::SendActionCommand(map<int,vector<ActionParam> > &param,string sUser,int actionType){
      if(m_pDevMgrPtr)
          m_pDevMgrPtr->SendActionCommand(param,sUser,actionType);
      return EC_OBJECT_NULL;
  }
}
