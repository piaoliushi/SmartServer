#include "ServerMgr.h"
#include "server_work.h"
#include "./http/RequestHandler.h"
#include "./http/RequestHandlerFactory.h"

struct test_handler {
    /* when there are many request at the same time, can put the request into a queue, then let another thread process it.
     * or maybe need to call http::client to connect to other server, and get response.
     */ 
    void operator()(const hx_http_server::request &request, hx_http_server::connection_ptr connection)
    {
        request_handler_ptr request_handler_ = request_handler_factory::get_mutable_instance().create();
        return request_handler_->start(request, connection);
    }
};

namespace net
{
	ServerMgr::ServerMgr(int port)
		: _taskqueueptr(new TaskQueue<msgPointer>)//创建一个任务队列
		, _workerptr(new server_work((*_taskqueueptr.get())))//创建一个用户任务
        , _serverptr(new LocalServer(port, (*_taskqueueptr.get())))//创建一个服务
		, _thread_pool(2)
	{
		_listenthreadptr.reset(new boost::thread(boost::bind(&ServerMgr::RunNetListen, this)));
		_workthreadptr.reset(new boost::thread(boost::bind(&ServerMgr::RunTasks, this)));
	}

	ServerMgr::~ServerMgr() 
	{
		_workerptr->stop();
		//_serverptr->stop();
		_taskqueueptr->ExitNotifyAll();
		_listenthreadptr->join();
		_workthreadptr->join();
	}

	void ServerMgr::RunNetListen()
	{
		if (_serverptr)
			_serverptr->run();

		_p_test_handler = new test_handler;
		std::string port = "8000";

		test_handler   handler;
		//hx_http_server xx("localhost",8000,handler,_thread_pool,http::_reuse_address = true);
		//_httpserverptr.reset(new hx_http_server("localhost", port, *_p_test_handler, _thread_pool,true));
		//hx_http_server xx;//("localhost", port, *_p_test_handler)
		//server instance(,
		//	http::_reuse_address = true);
		//instance.run();
	}

	void ServerMgr::RunTasks()
	{
		if (_workerptr)
			_workerptr->run();
	}
	//用户登录
	void ServerMgr::Login(session_ptr ch_ptr,string usr,string psw,LoginAck &loginAck)
	{
		if(_serverptr)
			return _serverptr->user_login(ch_ptr,usr,psw,loginAck);
		return;
	}

	//用户注销
	void ServerMgr::Logout(session_ptr ch_ptr,string usr,string psw,LogoutAck &logoutAck)
	{
		if(_serverptr)
			return _serverptr->user_logout(ch_ptr,usr,psw,logoutAck);
		return;
	}

	//发送设备数据通知
	void ServerMgr::SendMonitorData(string sStationid,string sDevid,devDataNfyMsgPtr &dataPtr)
	{
		if(_serverptr)
			return _serverptr->send_dev_data(sStationid,sDevid,dataPtr);
		return;
	}

	//发送设备连接状态通知
	void ServerMgr::SendDevNetStateData(string sStationid,string sDevid,devNetNfyMsgPtr &netPtr)
	{
		if(_serverptr)
			return _serverptr->send_dev_net_state_data(sStationid,sDevid,netPtr);
	}

	//发送设备运行状态通知
	void ServerMgr::SendDevWorkStateData(string sStationid,string sDevid,devWorkNfyMsgPtr &workPtr)
	{
		if(_serverptr)
			return _serverptr->send_dev_work_state_data(sStationid,sDevid,workPtr);
	}

	//发送设备报警状态通知
	void ServerMgr::SendDevAlarmStateData(string sStationid,string sDevid,devAlarmNfyMsgPtr &alarmPtr)
	{
		if(_serverptr)
			return _serverptr->send_dev_alarm_state_data(sStationid,sDevid,alarmPtr);
	}

	//发送控制执行结果通知
	void ServerMgr::SendCommandExecuteResult(string sStationid,string sDevid,e_MsgType nMsgType,devCommdRsltPtr &commdRsltPtr)
	{
		if(_serverptr)
			return _serverptr->send_command_execute_result(sStationid,sDevid,nMsgType,commdRsltPtr);
	}

	//获得子台站设备状态信息
	loginAckMsgPtr  ServerMgr::get_child_station_dev_status()
	{
		if(_serverptr)
			return _serverptr->get_child_station_dev_status();
		return loginAckMsgPtr();
	}
	//停止服务
	void ServerMgr::stop_server()
	{
		if(_serverptr)
			_serverptr->stop();
	}

	//上级查岗
	void ServerMgr::check_station_working(checkWorkingReqMsgPtr pcheckWork)
	{
		if(_serverptr)
			_serverptr->check_station_working(pcheckWork);
	}
}
