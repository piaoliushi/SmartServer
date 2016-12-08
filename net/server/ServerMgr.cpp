
#include "ServerMgr.h"
#include "LocalConfig.h"
#include "./http/RequestHandler.h"
#include "./http/RequestHandlerFactory.h"
#include "server_work.h"
struct web_handler {

    void operator()(const hx_http_server::request &request, hx_http_server::connection_ptr connection)
    {
        //request_handler_ptr request_handler_ = request_handler_factory::get_mutable_instance().create();
        //request_handler_ptr request_handler_(new request_handler());
        return request_handler_factory::get_mutable_instance().create()->start(request, connection);
    }
};

namespace hx_net
{
	ServerMgr::ServerMgr(int port)
		: _taskqueueptr(new TaskQueue<msgPointer>)//创建一个任务队列
		, _workerptr(new server_work((*_taskqueueptr.get())))//创建一个用户任务
        , _serverptr(new LocalServer(port, (*_taskqueueptr.get())))//创建一个服务
        ,_httpserverptr(NULL)
        ,_ws_serverptr(new websocket_server)
    {

        _listenthreadptr.reset(new boost::thread(boost::bind(&ServerMgr::RunNetListen, this)));
        _workthreadptr.reset(new boost::thread(boost::bind(&ServerMgr::RunTasks, this)));

        if(GetInst(LocalConfig).http_svc_use()==true){
            _web_handler = new web_handler;
            http::async_server<web_handler>::options options(*_web_handler);
            options.address(GetInst(LocalConfig).http_svc_ip())
                    .port(GetInst(LocalConfig).http_svc_port())
                    .io_service(boost::make_shared<boost::asio::io_service>())
                    .thread_pool(boost::make_shared<boost::network::utils::thread_pool>(2))
                    .reuse_address(true);
            _httpserverptr = new hx_http_server(options);
            _httpthreadptr.reset(new boost::thread(boost::bind(&ServerMgr::RunHttpServer,this)));

        }

        _ws_threadptr.reset(new boost::thread(boost::bind(&ServerMgr::RunWsServer, this)));
    }

	ServerMgr::~ServerMgr() 
	{
        if(_ws_serverptr){
            _ws_serverptr->stop();
            _ws_threadptr->join();
        }
        if(_httpserverptr){
            _httpserverptr->stop();
            _httpthreadptr->join();
        }
		_workerptr->stop();
		_taskqueueptr->ExitNotifyAll();
		_listenthreadptr->join();
		_workthreadptr->join();


	}

	void ServerMgr::RunNetListen()
	{
		if (_serverptr)
			_serverptr->run();
	}

	void ServerMgr::RunTasks()
	{
		if (_workerptr)
			_workerptr->run();
	}

    void ServerMgr::RunHttpServer()
    {
        if(_httpserverptr)
            _httpserverptr->run();
    }

    void ServerMgr::RunWsServer(){
        if(_ws_serverptr)
            _ws_serverptr->run(9002);
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
        if(_ws_serverptr)
            return _ws_serverptr->send_message(dataPtr);
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

	//停止服务
	void ServerMgr::stop_server()
	{
		if(_serverptr)
			_serverptr->stop();
	}
}
