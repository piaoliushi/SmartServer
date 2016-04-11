#ifndef  __NTP_SESSION_
#define __NTP_SESSION_
#pragma once

#include "../../net_session.h"
#include "../../../DataTypeDefine.h"
using boost::asio::io_service;
using boost::asio::ip::tcp;

namespace hx_net
{
class ntp_session;
typedef boost::shared_ptr<ntp_session>  ntp_session_ptr;
typedef boost::weak_ptr<ntp_session>    ntp_session_weak_ptr;
    class ntp_session:public boost::enable_shared_from_this<net_session>
	{
	public:
        ntp_session(boost::asio::io_service& io_service);
        ~ntp_session();
		//udp连接
		void udp_connect(std::string hostname,unsigned short port);
		//断开连接
		void disconnect();
		//发送消息
		bool sendRawMessage(unsigned char * data_,int nDatalen);
	protected:
		void start_read_head(int msgLen);//开始接收头
		void start_read_body(int msgLen);//开始接收体
		void start_read(int msgLen);
		void start_query_timer(unsigned long nSeconds=2000);
		void start_connect_timer(unsigned long nSeconds=3);
		void start_timeout_timer(unsigned long nSeconds=10);
		void start_write(unsigned char* commStr,int commLen);
	public:	
		void handle_connected(const boost::system::error_code& error);
        void handle_udp_read(const boost::system::error_code& error,size_t bytes_transferred);//udp接收回调
		virtual void handle_write(const boost::system::error_code& error,size_t bytes_transferred);
	private:
		udp::resolver                    uresolver_;
		udp::endpoint                   uendpoint_;
#ifdef USE_STRAND
		io_service::strand              strand_;   //消息头与消息体同步
#endif
        boost::asio::io_service&          io_service_;

	};
}
#endif
