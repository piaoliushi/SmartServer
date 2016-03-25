//------------------------------------------------------------------------------------------------//
//
//
//
//
//
//------------------------------------------------------------------------------------------------//
#include "client_session.h"
//#include "SvcClientMgr.h"
#include "../../LocalConfig.h"
#include "../SvcMgr.h"
//#include <glog/logging.h>
namespace hx_net
{
	client_session::client_session(boost::asio::io_service& io_service, 
		TaskQueue<msgPointer>& taskwork)
        :net_session(io_service)
#ifdef USE_CLIENT_STRAND
		, strand_(io_service)
#endif 
		,h_b_timer_(io_service)
		,resolver_(io_service)
 		,taskwork_(taskwork)
		,timeout_timer_(io_service)
 		,receive_msg_ptr_(new message)
 		,deadline_interval_(20)//每120秒进行一次循环重连尝试
		,con_state_(con_disconnected)
		,reconnect_flag_(false)
		,reconnect_timer_(io_service)
		,stop_flag_(false)
	{

	}

	client_session::~client_session(void)
	{
	}

	//设置自动重连与重连间隔
	void client_session::set_reconnect_flag(bool bflag,int interval)
	{
		boost::recursive_mutex::scoped_lock lock(re_connect_mutex_);
		reconnect_flag_ = bflag;
		deadline_interval_ = interval;
	}
	//是否需要自动重连
	bool client_session::need_reconnect()
	{
		boost::recursive_mutex::scoped_lock lock(re_connect_mutex_);
		return reconnect_flag_;
	}
	//启动重连定时器
	void client_session::start_connect_timer(unsigned long nSeconds)
	{
		if(!need_reconnect())
			return;
		set_con_state(con_connecting);//设置正在重连标志
		reconnect_timer_.expires_from_now(boost::posix_time::seconds(nSeconds));
		reconnect_timer_.async_wait(boost::bind(&client_session::connect_timer_event,
			this,boost::asio::placeholders::error));
	}

	//启动重连定时器，并启动超时定时器
	void client_session::connect_timer_event(const boost::system::error_code& error)
	{
		//只有当前状态是正在连接才执行超时。。。
		if(!is_connecting())
			return;
		if(error!= boost::asio::error::operation_aborted)
		{
			socket().async_connect(endpoint_,boost::bind(&client_session::handle_connected,
				this,boost::asio::placeholders::error)); 
			
			std::cout<<"reconnect request..."<<std::endl;
			start_timeout_timer(deadline_interval_);//启动超时定时器
		}
	}

	//启动超时定时器
	void client_session::start_timeout_timer(unsigned long nSeconds)
	{
		timeout_timer_.expires_from_now(boost::posix_time::seconds(nSeconds));
		timeout_timer_.async_wait(boost::bind(&client_session::connect_timeout,
			this,boost::asio::placeholders::error));
	}

	//开始连接
	void client_session::connect(std::string hostname,unsigned short port,bool bReconnect)
	{
		if(is_connected())
		{
			std::string id = str(boost::format("%1%:%2%")%hostname%port);
			std::cout<<id<<"is connected!!"<<std::endl;
			return;
		}

		stop_flag_ = false;
		set_reconnect_flag(bReconnect);

		boost::system::error_code ec;     
		tcp::resolver::query query(hostname, boost::lexical_cast<std::string, unsigned short>(port));     
		tcp::resolver::iterator iter = resolver_.resolve(query, ec);  

		if(iter != tcp::resolver::iterator())
		{
			endpoint_ = (*iter).endpoint();
			socket().async_connect(endpoint_,boost::bind(&client_session::handle_connected,
				this,boost::asio::placeholders::error)); 
			set_con_state(con_connecting);//正在连接
			start_timeout_timer(deadline_interval_);//启动超时重连定时器
		}

	}

	void client_session::handle_connected(const boost::system::error_code& error) 
	{
		if(stop_flag_)
			return ;

		timeout_timer_.cancel();//关闭重连超时定时器

		if(!error)
		{
			set_con_state(con_connected);
			std::time_t now = time(0);
			set_last_hb_time(now);
			start_hb_timer();
			start_read_head();//开始请求数据头
		}
		else
		{
			close_i();

			if(need_reconnect())
				start_connect_timer();//启动重连尝试
		}
	}


	//连接超时
	void client_session::connect_timeout(const boost::system::error_code& error)
	{
		//只有当前状态是正在连接才执行超时。。。
		if(!is_connecting())
			return;
		//超时了
		if(error!= boost::asio::error::operation_aborted)
		{
			std::cout << "reconnect--->timeout"<< std::endl;

			//如果非自动连接则发送超时通知
			if(need_reconnect())
				start_connect_timer();//启动重连尝试
		}
		else
			std::cout << "timeout timer to be cancel"<< std::endl;
	}

	//关闭socket，清理连接资源
	void client_session::close_all()
	{
		set_con_state(con_disconnected);
		close_i();
		reconnect_timer_.cancel();
		timeout_timer_.cancel();
		h_b_timer_.cancel();
		clearDeque();
	}

	//断开连接，设置停止标志，仅用于完全退出系统
	void client_session::disconnect()
	{
		stop_flag_=true;//设置停止标志
		close_all();
	}
	//清理发送队列
	void client_session::clearDeque()
	{
		boost::mutex::scoped_lock lock(msg_q_mutex_);
		if(!send_msg_q_.empty())
			send_msg_q_.clear();
	}

	//启动心跳定时器
	void client_session::start_hb_timer()
	{
		h_b_timer_.expires_from_now(boost::posix_time::seconds(60));
		h_b_timer_.async_wait(
			boost::bind(&client_session::heartbeat_timeout,this,
			boost::asio::placeholders::error)); //strand_.wrap()
	}

	//心跳检测
	void client_session::heartbeat_timeout(const boost::system::error_code& error)
	{
		if (!is_connected())
			return ;
		if (!error)
		{
			std::time_t now = time(0);
			//判断服务器心跳回复消息是否超时
			//超时：断开与服务器的连接
			if (now - get_last_hb_time()>180)
			{
				close_all();
				start_connect_timer();//启动重连尝试
				cout<<"heartbeat timeout!"<<endl;
				return;
			}
			else
			{
				googleMsgPtr nullMsg;
				if(!sendMessage(MSG_HEARTBEAT_REQ,nullMsg))
					std::cout << "send heartbeat requst error"<< std::endl;
				cout<<"send heartbeat requst successful!"<<endl;
				start_hb_timer();
			}
		}
	}

	//获得最后心跳时间
	std::time_t client_session::get_last_hb_time()
	{
		boost::mutex::scoped_lock lock(hb_mutex_);
		return last_hb_time_;
	}

	//设置最后心跳时间
	void client_session::set_last_hb_time(std::time_t t)
	{
		boost::mutex::scoped_lock lock(hb_mutex_);
		last_hb_time_ = t;
	}

	//收到心跳响应，复位计时
	int client_session::heart_beat_rsp()
	{
		std::time_t now = time(0);
		set_last_hb_time(now);
		cout<<"heart beat ack"<<endl;
		return 0;
	}
	//用户登陆
	void  client_session::login_user(string sUser,string sPassword)
	{
		//LoginReq sloginReq;
		loginReqMsgPtr sloginReq(new LoginReq);
		sloginReq->set_susrname(sUser);
		sloginReq->set_susrpsw(sPassword);
		sendMessage(MSG_LOGIN_REQ,sloginReq);
	}
	//用户注销
	void  client_session::logout_user(string sUser,string sPassword)
	{
		//LogoutReq slogoutReq;
		logoutReqMsgPtr slogoutReq(new LogoutReq);
		slogoutReq->set_susrname(QString::fromLocal8Bit(sUser.c_str()).toUtf8().data());
		slogoutReq->set_susrpsw(QString::fromLocal8Bit(sPassword.c_str()).toUtf8().data());
		sendMessage(MSG_LOGOUT_REQ,slogoutReq);
	}

	//登陆响应
	//将本级服务所有本地设备状态信息及本级以下上传的设备
	//状态信息一并打包提交给上级服务
	void client_session::login_user_ack(string sUser,string sPassword,msgPointer &pMsg)
	{
		//此处增加服务权限配置时修改。。。。
		if(sPassword=="childserver")
		{
			string sServerId = GetInst(LocalConfig).local_dev_server_number();
			loginAckMsgPtr curStatusPtr = GetInst(SvcMgr).get_child_station_dev_status();
			curStatusPtr->set_usrname(sUser);
			curStatusPtr->set_usrnumber(sServerId);
			sendMessage(MSG_LOGIN_ACK,curStatusPtr);
		}
	}

	//注销响应
	void client_session::logout_user_ack(string sUser,string sPassword,msgPointer &pMsg)
	{

	}
	//上级查岗
	void  client_session::check_station_working(checkWorkingReqMsgPtr pcheckWork)
	{
		GetInst(SvcMgr).check_station_working(pcheckWork);
	}
	//获取连接状态
	con_state client_session::get_con_state()
	{
		boost::recursive_mutex::scoped_lock lock(con_state_mutex_);
		return con_state_;
	}
	//设置连接状态
	void  client_session::set_con_state(con_state s)
	{
		boost::recursive_mutex::scoped_lock lock(con_state_mutex_);
		if(con_state_ != s)
		{
			con_state_ = s;
			//notify_.OnConnect(endpoint_.address().to_string(),endpoint_.port(),con_state_);
		}

	}
	//是否已连接
	bool  client_session::is_connected()
	{
		if(get_con_state()==con_connected)
			return true;
		return false;
	}
	//是否正在连接
	bool  client_session::is_connecting()
	{
		if(get_con_state()==con_connecting)
			return true;
		return false;
	}
	//是否已断开
	bool  client_session::is_disconnected()
	{
		if(get_con_state()==con_disconnected)
			return true;
		return false;
	}

	//开始请求数据头
	void client_session::start_read_head()
	{
		receive_msg_ptr_->setsession(shared_from_this());
		boost::asio::async_read(socket(), boost::asio::buffer(receive_msg_ptr_->data(),
			HeadSize),
#ifdef USE_STRAND
			strand_.wrap(
#endif
            boost::bind(&net_session::handle_read_head, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
#ifdef USE_STRAND
			)
#endif
			);
	}
	//数据头请求回调
	void client_session::handle_read_head(const boost::system::error_code& error, size_t bytes_transferred)
	{
		if (!is_connected())
			return ;

		if (!error && receive_msg_ptr_->decode_header())
		{
			//如果已经是完整消息（无参消息)
			if(receive_msg_ptr_->is_full_message())
			{
				// 将数据提交到工作队列.
				taskwork_.SubmitTask(receive_msg_ptr_);
				// 继续读取下一个数据包头.
				receive_msg_ptr_ = msgPointer(new message);
				start_read_head();//重新请求数据头
			}
			else
			{
				//继续请求消息体
				boost::asio::async_read(socket(), boost::asio::buffer(
					receive_msg_ptr_->body(), receive_msg_ptr_->body_length()),
#ifdef USE_STRAND
					strand_.wrap
					(
#endif // USE_STRAND
                    boost::bind(&net_session::handle_read_body, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred)
#ifdef USE_STRAND
					)
#endif // USE_STRAND
					);
			}

		}
		else
		{
			//LOG(ERROR)<<"读取message header失败"<<boost::system::system_error(error).what();
			close_all();//清理痕迹
			start_connect_timer();//启动重连定时器
		}
	}

	void client_session::handle_read_body(const boost::system::error_code& error, size_t bytes_transferred)
	{
		if (!is_connected())
			return ;

		if (!error && receive_msg_ptr_->check_body(bytes_transferred))
		{
			// 将数据提交到工作队列.
			taskwork_.SubmitTask(receive_msg_ptr_);
			// 继续读取下一个数据包头.
			receive_msg_ptr_ = msgPointer(new message);
			start_read_head();//开始请求数据头
		}
		else
		{
			//LOG(ERROR)<<"读取message body失败"<<boost::system::system_error(error).what();
			close_all();//清理痕迹
			start_connect_timer();//启动重连定时器
		}
	}

	//加入发送消息队列
// 	int client_session::putq(msgPointer msgPtr)
// 	{
// 		boost::mutex::scoped_lock lock(msg_q_mutex_);
// 		send_msg_q_.push_back(msgPtr);
// 
// 		if (!send_msg_q_.empty())
// 			start_write();
// 		return 0;
// 	}

	//开始异步请求写数据
	void client_session::start_write(msgPointer msgPtr)
	{
		boost::asio::async_write
			(
			socket(),
			boost::asio::buffer(&(msgPtr->data()[0]),msgPtr->length()),
            boost::bind(&net_session::handle_write,shared_from_this(),
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred)
			);
	}
	
	//发送消息
	bool client_session::sendMessage(e_MsgType _type,googleMsgPtr gMsgPtr)
	{
		if (!is_connected())
			return false;

		msgPointer msgPtr(new message);
		if(msgPtr->encode_google_message(_type,gMsgPtr))
		{
			start_write(msgPtr);
			return true;
		}
		else
		{
			//LOG(ERROR)<<"编码google message error";
		}
		return false;
	}

	void client_session::handle_write(const boost::system::error_code& error,std::size_t bytes_transferred)
	{
		if (!is_connected())
			return ;
		if (!error)
		{
			return;
		}
		else
		{
			close_all();//清理痕迹
			start_connect_timer();//启动重连定时器
		}
	}

	
}
