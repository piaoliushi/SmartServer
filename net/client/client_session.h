#ifndef  CLIENT_SESSION
#define  CLIENT_SESSION
#pragma once
#include "../taskqueue.h"
#include "../message.h"
#include "../net_session.h"
#include "../../qnotifyhandler.h"
using boost::asio::ip::tcp;
namespace hx_net
{	
    class client_session :public net_session
	{
	public:


		client_session(boost::asio::io_service& io_service, TaskQueue<msgPointer>& taskwork);
		virtual ~client_session(void);
		//设置自动重连间隔
		void set_reconnect_flag(bool bflag,int interval=10);
		//是否需要自动重连
		bool need_reconnect();
		//开始连接
		void connect(std::string hostname,unsigned short port,bool bReconnect=false);
		//完全退出，停止重连，清理socket，复位初始数据
		void disconnect();
		//登陆
		void  login_user(string sUser,string sPassword);
		//注销
		void  logout_user(string sUser,string sPassword);
		//登陆响应
		void login_user_ack(string sUser,string sPassword,msgPointer &pMsg);
		//注销响应
		void logout_user_ack(string sUser,string sPassword,msgPointer &pMsg);
		//心跳响应
		int   heart_beat_rsp();
		//开始接收数据头
		void  start_read_head();
		//是否已连接
		bool       is_connected();
		//是否正在连接...
		bool       is_connecting();
		//是否已断开
		bool       is_disconnected();
		//发送消息
		bool sendMessage(e_MsgType _type,googleMsgPtr gMsgPtr);
	protected:	
		//获取连接状态
		con_state  get_con_state();
		//设置连接状态
		void       set_con_state(con_state s);
		//关闭socket资源，清理与本次连接关联的所有资源
		void close_all();
		//投递发送消息
		//int  putq(msgPointer msgPtr);
		//清理发送队列
		void clearDeque();
		//开始发送数据
		void start_write(msgPointer msgPtr);	
		//启动心跳定时器
		void  start_hb_timer();
		//心跳超时
		void heartbeat_timeout(const boost::system::error_code& error);
		//获得最后心跳时间
		std::time_t get_last_hb_time();
		//设置最后心跳时间
		void set_last_hb_time(std::time_t t);
		//启动5秒循环连接定时器（在timeout之前定时循环连接）
		void start_connect_timer(unsigned long nSeconds=5);
		//重连回调
		void connect_timer_event(const boost::system::error_code& error);
		//启动连接超时定时器
		void start_timeout_timer(unsigned long nSeconds);
		//连接超时回调
		void connect_timeout(const boost::system::error_code& error);
		//上级查岗
		void  check_station_working(checkWorkingReqMsgPtr pcheckWork);
	public:
		void handle_connected(const boost::system::error_code& error);
 		void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred);
 		void handle_read_body(const boost::system::error_code& error, size_t bytes_transferred);
 		void handle_write(const boost::system::error_code& error,size_t bytes_transferred);
		
	private:
		bool                            stop_flag_;
 		TaskQueue<msgPointer>&          taskwork_;
		tcp::resolver                   resolver_;
		tcp::endpoint                   endpoint_;         //记录端点信息
 		msgPointer                      receive_msg_ptr_;


		boost::asio::deadline_timer     h_b_timer_;//心跳超时定时器
		boost::mutex                    hb_mutex_;
		std::time_t                     last_hb_time_;
		
		boost::mutex                    msg_q_mutex_;
		std::deque<msgPointer>          send_msg_q_;

		boost::recursive_mutex          con_state_mutex_;
		con_state                       con_state_;

		boost::recursive_mutex          re_connect_mutex_;
 		bool                            reconnect_flag_;     //自动重连标志

 		int                             deadline_interval_;  //重连间隔
		boost::asio::deadline_timer     reconnect_timer_;    //循环重连定时器
		boost::asio::deadline_timer     timeout_timer_;      //连接超时定时器
 		
#ifdef USE_STRAND
		boost::asio::io_service::strand strand_;           //消息头与消息体同步
#endif 
		//QNotifyHandler                  &notify_;//消息通知对象引用

	};
	
}
#endif
