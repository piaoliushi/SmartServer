#ifndef CONNECT_HANDLER
#define CONNECT_HANDLER

#pragma once

#include "../net_session.h"
#include "../taskqueue.h"
#include "../message.h"
#include "local_server.h"
using boost::asio::ip::tcp;
namespace hx_net
{
    class connect_handler:public net_session
	{
	public:	
		connect_handler(boost::asio::io_service& io_service, 
            TaskQueue<msgPointer>& taskwork,LocalServer& srv);

		virtual ~connect_handler(void);

		void start_read_head();	
		
		usr_state get_state();

		void set_state(usr_state s);

        bool sendMessage(e_MsgType _type,googleMsgPtr gMsgPtr);

		void start_hb_timer();

		std::time_t get_last_hb_time();

		void set_last_hb_time(std::time_t t);

		int  heart_beat_rsp();

		void login_user_ack(string sUser,string sPassword,msgPointer &pMsg);

		void logout_user_ack(string sUser,string sPassword,msgPointer &pMsg);

        void handover_ack(string soldUser,string sNewUser,string sNewPassword,msgPointer &pMsg);

        void user_duty_log(string sUserId,const string &sContent,int nType);


		void handle_timeout(const boost::system::error_code& e);

		int  putq(msgPointer msgPtr);

		void start_write(msgPointer msgPtr);

		void close_handler();

		void sendFlashCheckMsg();

	public:
		virtual void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred);
		virtual void handle_read_body(const boost::system::error_code& error, size_t bytes_transferred);
		virtual void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
	private:
		TaskQueue<msgPointer>&          taskwork_;
		msgPointer                      receive_msg_ptr_;
        LocalServer&                         srv_;//服务管理对象
		boost::asio::deadline_timer     h_b_timer_;//心跳超时定时器
        boost::recursive_mutex                    hb_mutex_;
		std::time_t                     last_hb_time_;
        boost::recursive_mutex                    state_mutex_;
		usr_state                       usr_state_;
#ifdef USE_CLIENT_STRAND
		boost::asio::io_service::strand strand_;           //消息头与消息体同步
#endif 
    };
    typedef boost::shared_ptr<hx_net::connect_handler> connect_handler_ptr;
}

#endif
