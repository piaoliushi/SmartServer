#ifndef CONNECT_SUBSVC_HANDLER
#define CONNECT_SUBSVC_HANDLER

#pragma once

#include "../net_session.h"
#include "../taskqueue.h"
#include "../message.h"
#include "local_server.h"
#include "../share_ptr_object_define.h"
using boost::asio::ip::tcp;
namespace hx_net
{
    class connect_subsvc_handler:public net_session
	{
	public:	
		connect_subsvc_handler(boost::asio::io_service& io_service, 
            TaskQueue<msgPointer>& taskwork,LocalServer& srv);

		virtual ~connect_subsvc_handler(void);

		void start_read_head();	
		
		//bool is_online();
		
		bool sendMessage(e_MsgType _type,googleMsgPtr gMsgPtr);

		void start_hb_timer();

		std::time_t get_last_hb_time();

		void set_last_hb_time(std::time_t t);

		int  heart_beat_rsp();

		void login_user(string sUser,string sPassword);

		void logout_user(string sUser,string sPassword);

		void login_user_ack(string sUser,string sPassword,msgPointer &pMsg);

		void logout_user_ack(string sUser,string sPassword,msgPointer &pMsg);

		void  dev_netstate_update(msgPointer &pMsg);
		bool  check_and_update_net_state(const DevNetStatus &netState);

		void  dev_workstate_update(msgPointer &pMsg);
		bool  check_and_update_work_state(const DevWorkStatus &workState);

		void  dev_alarmstate_update(msgPointer &pMsg);
		bool  check_and_update_alarm_state(string sDevNumber,const DevAlarmStatus &devAlarmState,
			                               const DevAlarmStatus_eCellAlarmMsg &cellAlarmState);

		void dev_data_notify(msgPointer &pMsg);

		void handle_timeout(const boost::system::error_code& e);

		int  putq(msgPointer msgPtr);

		void start_write(msgPointer msgPtr);

		void close_handler();

		loginAckMsgPtr  get_child_station_dev_status();

		void reset_netstate();//复位所有设备网络状态

		//查岗确认通知
		void  check_station_result_notify(checkWorkingNotifyMsgPtr pcheckWorkResult);

	protected:
		void  init_sava_data_time(const string &sDevId);
		bool is_need_save_data(string sDevId);//当前时间是否需要保存
		void handler_data(devDataNfyMsgPtr curData);
		//保存报警数据
		void save_alarm_data(DevAlarmStatus *pDevAlarm);
		//更新报警时间
		void update_alarm_end_time(DevAlarmStatus *pDevAlarm,const  string & sAlarmEndTm);
		//提交任务
		void task_count_increase();
		//任务递减
		void task_count_decrease();
		//任务数
		int  task_count();
		//等待任务结束
		void wait_task_end();

	public:
		virtual void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred);
		virtual void handle_read_body(const boost::system::error_code& error, size_t bytes_transferred);
		virtual void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
	private:
		TaskQueue<msgPointer>&          taskwork_;
		msgPointer                      receive_msg_ptr_;
        LocalServer&                         srv_;//服务管理对象
		boost::asio::deadline_timer     h_b_timer_;//心跳超时定时器
		boost::mutex                    hb_mutex_;
		std::time_t                     last_hb_time_;
		boost::mutex                    state_mutex_;
		usr_state                       usr_state_;
		boost::recursive_mutex        dev_data_save_mutex;
		map<string,time_t>              tmLastSaveTime;//amend by lk  2014-2-27(每设备一个)

		boost::recursive_mutex           child_station_s_mutex;
		loginAckMsgPtr						 child_station_dev_status_ptr;//下级台站设备状态信息
		loginAckMsgWeakPtr              child_station_dev_status_weakptr;

		boost::mutex                    task_mutex_;
		int                             task_count_;
		boost::condition                task_end_conditon_;
        map<string,map<int,std::pair<int,string> > >           mapStrItemAlarmStartTime;
#ifdef USE_STRAND
		boost::asio::io_service::strand strand_;           //消息头与消息体同步
#endif 
	};
}

#endif
