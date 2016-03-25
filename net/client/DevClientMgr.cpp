//--------------------------------------------------------------------------------//
//说明：该类实现设备连接，数据分析管理，其中包括主动连接本地设备，以及连接下级服务器取得
//      设备数据。
//作者：刘堃
//日期：2012-10-10

#include "DevClientMgr.h"
#include "client_work.h"
//#include "../../DataType.h"
//#include <glog/logging.h>
#include <boost/network/protocol/http.hpp>
//using namespace boost::network::http;
//typedef  boost::shared_ptr<client>  hx_http_client_ptr;
namespace hx_net
{
	DevClientMgr::DevClientMgr()
		: _taskqueueptr(new TaskQueue<msgPointer>)//创建一个任务队列
		, _workerptr(new client_work((*_taskqueueptr.get()),2))//创建一个用户任务
		, _devclientptr(new DevClient((*_taskqueueptr.get()),2))//创建一个服务
	{
        http::client::request  request("http://192.168.1.47/AuthenService.asmx/Test");
        std::string  content = "hello word!";

		_listenthreadptr.reset(new boost::thread(boost::bind(&DevClientMgr::RunNetListen, this)));
		_workthreadptr.reset(new boost::thread(boost::bind(&DevClientMgr::RunTasks, this)));
	}

	DevClientMgr::~DevClientMgr(void)
	{
		disconnect();
		_workerptr->stop();
		_devclientptr->stop();
		_taskqueueptr->ExitNotifyAll();	
		
		_listenthreadptr->join();
		//LOG(ERROR)<<"设备管理服务已停止！！";
		_workthreadptr->join();
		//LOG(ERROR)<<"设备工作者线程已停止！！";

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

	void DevClientMgr::RunTasks()
	{
		if (_workerptr)
			_workerptr->run();//创建工作线程用于从任务队列中取出任务
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
    void DevClientMgr::get_dev_alarm_state(string sStationId,string sDevid,map<int,std::pair<int,tm> >& cellAlarm)
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

	//----------------------relay server-------------------------------------------------//
	//发送设备数据通知
	void DevClientMgr::send_dev_data(string sStationid,string sDevid,devDataNfyMsgPtr &dataPtr)
	{
		if (_devclientptr)
			_devclientptr->send_dev_data(sStationid,sDevid,dataPtr);
	}

	//发送设备网络状态数据
	void DevClientMgr::send_dev_net_state_data(string sStationid,string sDevid,devNetNfyMsgPtr &netPtr)
	{
		if (_devclientptr)
			_devclientptr->send_dev_net_state_data(sStationid,sDevid,netPtr);
	}
		
	//发送设备运行状态数据
	void DevClientMgr::send_dev_work_state_data(string sStationid,string sDevid,devWorkNfyMsgPtr &workPtr)
	{
		if (_devclientptr)
			_devclientptr->send_dev_work_state_data(sStationid,sDevid,workPtr);
	}
	//发送设备报警状态数据
	void DevClientMgr::send_dev_alarm_state_data(string sStationid,string sDevid,devAlarmNfyMsgPtr &alarmPtr)
	{
		if (_devclientptr)
			_devclientptr->send_dev_alarm_state_data(sStationid,sDevid,alarmPtr);
	}
	//发送控制执行结果通知
	void DevClientMgr::send_command_execute_result(string sStationid,string sDevid,e_MsgType nMsgType,devCommdRsltPtr &commdRsltPtr)
	{
		if (_devclientptr)
			_devclientptr->send_command_execute_result(sStationid,sDevid,nMsgType,commdRsltPtr);
	}

	//向上提交查岗结果
	void DevClientMgr::commit_check_working_result(checkWorkingNotifyMsgPtr pcheckWorkResult)
	{
		if (_devclientptr)
			_devclientptr->commit_check_working_result(pcheckWorkResult);
	}
	//------------------------------------------------end-----------------------------------------------------//
	//是否是通过上级平台直连下级平台设备
	bool DevClientMgr::is_direct_connect_device(string sStationId,string sDevNumber)
	{
		if(_devclientptr)
			return _devclientptr->is_direct_connect_device(sStationId,sDevNumber);
		return false;
	}
}

