#include "connect_handler.h"
#include "../../LocalConfig.h"
#include "../../database/DataBaseOperation.h"

using namespace db;
namespace hx_net
{
	connect_handler::connect_handler(boost::asio::io_service& io_service, 
        TaskQueue<msgPointer>& taskwork,LocalServer &srv)
        :net_session(io_service)
		,h_b_timer_(io_service)
#ifdef USE_CLIENT_STRAND
		, strand_(io_service)
#endif 
		,taskwork_(taskwork)
		,receive_msg_ptr_(new message)
		,srv_(srv)
		,usr_state_(usr_offline)
	{

	}

	connect_handler::~connect_handler(void)
	{
	}

	//关闭连接，并清理自己
	void connect_handler::close_handler()
	{
		h_b_timer_.cancel();
		close_i();
		srv_.remove_connection_handler(shared_from_this());
	}

	//启动心跳定时器
	void connect_handler::start_hb_timer()
	{
        h_b_timer_.expires_from_now(boost::posix_time::seconds(60));
		h_b_timer_.async_wait(strand_.wrap(
			boost::bind(&connect_handler::handle_timeout,this,
			boost::asio::placeholders::error))); 
	}

	//心跳超时
	void connect_handler::handle_timeout(const boost::system::error_code& e)
	{
		if (!e)
		{
			std::time_t now = time(0);
            if (now - get_last_hb_time() >180)//超时
			{
 				close_handler();
                //cout<<"heartbeat timeout!"<<endl;
				return;
			}			
			else
			{
				start_hb_timer();
			}
				
		}
		
	}

	//获得最后心跳时间
	std::time_t connect_handler::get_last_hb_time()
	{
        boost::recursive_mutex::scoped_lock lock(hb_mutex_);
		return last_hb_time_;
	}

	//设置最后心跳时间
	void connect_handler::set_last_hb_time(std::time_t t)
	{
        boost::recursive_mutex::scoped_lock lock(hb_mutex_);
		last_hb_time_ = t;
	}

// 	bool connect_handler::is_online()
// 	{
// 		if(get_state()==usr_online)
// 			return true;
// 		return false;
// 	}
	//回复心跳请求
	int connect_handler::heart_beat_rsp()
	{
		std::time_t now = time(0);
		set_last_hb_time(now);
		googleMsgPtr nullMsg;
		if(!sendMessage(MSG_HEARTBEAT_ACK,nullMsg))
		{
            //std::cout << "send heartbeat Ack error"<< std::endl;
			return -1;
		}

        //cout<<"send heart beat ack successful!"<<endl;
		return 0;
		
	}

	void connect_handler::login_user_ack(string sUser,string sPassword,msgPointer &pMsg)
	{
		loginAckMsgPtr sloginAck(new LoginAck);
		srv_.user_login(shared_from_this(),sUser,sPassword,*sloginAck);
		sendMessage(MSG_LOGIN_ACK,sloginAck);
	}

	void connect_handler::logout_user_ack(string sUser,string sPassword,msgPointer &pMsg)
	{
		logoutAckMsgPtr slogOutAck(new LogoutAck);
		srv_.user_logout(shared_from_this(),sUser,sPassword,*slogOutAck);
		sendMessage(MSG_LOGOUT_ACK,slogOutAck);
	}

    //交接班(ack消息共用，类型区分)
    void connect_handler::handover_ack(string soldUser,string sNewUser,string sNewPassword,msgPointer &pMsg)
    {
        loginAckMsgPtr sloginAck(new LoginAck);
        HandOverReq rhandover;
        rhandover.ParseFromArray(pMsg->body(),pMsg->bodySize());
        string sContents = rhandover.scontents().c_str();
        srv_.user_handover(shared_from_this(),soldUser,sNewUser,sNewPassword,sContents,*sloginAck);
        sendMessage(MSG_HANDOVER_ACK,sloginAck);
    }

    //签到,签退(ack)
    void connect_handler::user_sign_in_out_ack(int bIn,string sUser,string sPassword)
    {
        signInOutAckMsgPtr signinAck(new SignInOutAck);
        srv_.user_signin_out(shared_from_this(),sUser,sPassword,signinAck,bIn);
        sendMessage(MSG_SIGN_IN_OUT_ACK,signinAck);
    }
    //值班日志
    void  connect_handler::user_duty_log(string sUserId,const string &sContent,int nType)
    {
        dutyLogAckMsgPtr dutyAck(new DutyLogAck);
        //std::string sstationid = GetInst(LocalConfig).local_station_id();
        if(!GetInst(DataBaseOperation).AddDutyLog(sUserId,sContent,nType))
            dutyAck->set_eresult(EC_FAILED);
        else
            dutyAck->set_eresult(EC_OK);
        sendMessage(MSG_DUTY_LOG_ACK,dutyAck);
    }


	//开始接收消息头
	void connect_handler::start_read_head()
	{
		receive_msg_ptr_->setsession(shared_from_this());
		boost::asio::async_read(socket(), boost::asio::buffer(
			receive_msg_ptr_->data(),HeadSize),
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

	//消息头接收回调
	void connect_handler::handle_read_head(const boost::system::error_code& error,
		size_t bytes_transferred)
	{
		if (!error && receive_msg_ptr_->decode_header())
		{
			//如果已经是完整消息（无参消息)
			if(receive_msg_ptr_->is_full_message())
			{
				// 将数据提交到工作队列.
				taskwork_.SubmitTask(receive_msg_ptr_);
				// 继续读取下一个数据包头.
				receive_msg_ptr_ = msgPointer(new message);
				//receive_msg_ptr_->reset_head_size();//重设消息长度
				//重新请求数据头
				start_read_head();

			}
			else//继续请求消息体
			{
				// 请求数据包.
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
			std::cout << boost::system::system_error(error).what() << std::endl;
			close_handler();// 出错，清理handler
		}
	}


	void connect_handler::handle_read_body(const boost::system::error_code& error, 
		size_t bytes_transferred)
	{
		if (!error && receive_msg_ptr_->check_body(bytes_transferred))
		{
			// 将数据提交到工作队列.
			taskwork_.SubmitTask(receive_msg_ptr_);
			// 继续读取下一个数据包头.
			receive_msg_ptr_ = msgPointer(new message);
			start_read_head();
		}
		else
		{
            if(receive_msg_ptr_->is_flash_check_message()){
				sendFlashCheckMsg();
			}
            close_handler();
		}
	}

	void connect_handler::sendFlashCheckMsg()
	{
		char checkMsg[100]=   
			"<cross-domain-policy> "     
			"<allow-access-from domain=\"*\" to-ports=\"*\"/>"     
			"</cross-domain-policy> ";//套接字策略文件   
		boost::asio::async_write
			(socket(),
			boost::asio::buffer(checkMsg,strlen(checkMsg)+1),
            boost::bind(&net_session::handle_write,shared_from_this(),
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred)
			);
	}
	
	int connect_handler::putq(msgPointer msgPtr)
	{
// 		boost::mutex::scoped_lock lock(msg_q_mutex_);
// 		send_msg_q_.push_back(msgPtr);
// 		if (!send_msg_q_.empty())
// 			start_write();
		return 0;
	}

	void connect_handler::start_write(msgPointer msgPtr)
	{
		boost::asio::async_write
			(socket(),
			boost::asio::buffer(&(msgPtr->data()[0]),msgPtr->length()),
            boost::bind(&net_session::handle_write,shared_from_this(),
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred)
			);
	}

	//发送google消息
	bool connect_handler::sendMessage(e_MsgType _type,googleMsgPtr gMsgPtr)
	{
		msgPointer msgPtr(new message);
		if(msgPtr->encode_google_message(_type,gMsgPtr))
		{
			start_write(msgPtr);
			return true;
		}
		return false;
	}

	void connect_handler::handle_write(const boost::system::error_code& error,size_t bytes_transferred)
	{
		if (!error)
		{
// 			boost::mutex::scoped_lock lock(msg_q_mutex_);
// 			send_msg_q_.pop_front();

			//若在线则继续发送队列消息
			//if (get_state() == usr_online)
// 			{
// 
// 				if (!send_msg_q_.empty())
// 					start_write();
// 				return;
// 			}
		}
		else
		{
			close_handler();// 出错，清理handler
		}
	}
}

