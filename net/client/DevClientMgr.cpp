//--------------------------------------------------------------------------------//
//说明：该类实现设备连接，数据分析管理，其中包括主动连接本地设备，以及连接下级服务器取得
//      设备数据。
//作者：刘堃
//日期：2012-10-10

#include "DevClientMgr.h"
#include "client_work.h"
#include <boost/network/protocol/http.hpp>
namespace hx_net
{
	DevClientMgr::DevClientMgr()
        : _devclientptr(new DevClient(2))//创建一个服务
	{
		_listenthreadptr.reset(new boost::thread(boost::bind(&DevClientMgr::RunNetListen, this)));
	}

	DevClientMgr::~DevClientMgr(void)
	{
		disconnect();
		_devclientptr->stop();
        _listenthreadptr->join();
	}

	void DevClientMgr::connect(std::string ip,unsigned short port)
	{
		if (_devclientptr)
			_devclientptr->connect_all();
	}


	void DevClientMgr::disconnect()
	{
		if (_devclientptr)
			_devclientptr->disconnect_all();

	}

	void DevClientMgr::RunNetListen()
	{
		if (_devclientptr)
			_devclientptr->run();
	}


	//通用命令执行
	e_ErrorCode DevClientMgr::excute_command(int cmdType,devCommdMsgPtr lpParam)
	{
		if (_devclientptr)
			return _devclientptr->excute_command(cmdType,lpParam);
		return EC_OBJECT_NULL;
	}

	int DevClientMgr::get_modle_online_count()
	{
		if(_devclientptr)
			return _devclientptr->get_modle_online_count();
		return 0;
	}

	//获得设备连接
	con_state DevClientMgr::get_dev_net_state(string sStationId,string sDevid)
	{
		if (_devclientptr)
			return _devclientptr->get_dev_net_state(sStationId,sDevid);
		return con_disconnected;
	}
	//获得设备运行状态
	dev_run_state DevClientMgr::get_dev_run_state(string sStationId,string sDevid)
	{
		if (_devclientptr)
			return _devclientptr->get_dev_run_state(sStationId,sDevid);
		return dev_unknown;
	}
	//获得设备运行状态
    void DevClientMgr::get_dev_alarm_state(string sStationId,string sDevid,map<int,map<int,CurItemAlarmInfo> >& cellAlarm)
	{
		if (_devclientptr)
			_devclientptr->get_dev_alarm_state(sStationId,sDevid,cellAlarm);
	}

	//获得设备基本信息
	bool DevClientMgr::dev_base_info(string sStationId,DevBaseInfo& devInfo,string sdevId)
	{
		if (_devclientptr)
			return _devclientptr->dev_base_info(sStationId,devInfo,sdevId);
		return false;
	}

    //开始执行控制任务
    e_ErrorCode DevClientMgr::start_exec_task(string sDevId,string sUser,int cmdType)
    {
        if(_devclientptr)
            return _devclientptr->start_exec_task(sDevId,sUser,cmdType);
        return EC_OBJECT_NULL;
    }
    //更新运行图
   e_ErrorCode DevClientMgr::update_monitor_time(string sDevId,map<int,vector<Monitoring_Scheduler> >& monitorScheduler,
                             vector<Command_Scheduler> &cmmdScheduler)
   {
       if(_devclientptr)
           return _devclientptr->update_monitor_time(sDevId,monitorScheduler,cmmdScheduler);
       return EC_OBJECT_NULL;
   }
   //更新告警配置
  e_ErrorCode DevClientMgr::update_dev_alarm_config(string sDevId,DeviceInfo &devInfo)
  {
      if(_devclientptr)
          return _devclientptr->update_dev_alarm_config(sDevId,devInfo);
      return EC_OBJECT_NULL;
  }

  //上报http消息
  e_ErrorCode   DevClientMgr::response_http_msg(string sUrl,string &sContent,string sRqstType)
  {
      if(_devclientptr)
          return _devclientptr->response_http_msg(sUrl,sContent,sRqstType);
      return EC_OBJECT_NULL;
  }

  //发送短信
  e_ErrorCode DevClientMgr::SendSMSContent(vector<string> &PhoneNumber, string AlarmContent)
   {
       if(_devclientptr)
           return _devclientptr->SendSMSContent(PhoneNumber,AlarmContent);
        return EC_OBJECT_NULL;
   }

  //发送联动命令
  e_ErrorCode  DevClientMgr::SendActionCommand(const string &sDevId,string sUser,int actionType)
  {
      if(_devclientptr)
          return _devclientptr->SendActionCommand(sDevId,sUser,actionType);
       return EC_OBJECT_NULL;
  }

}

