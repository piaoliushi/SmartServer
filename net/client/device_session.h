#ifndef  __DEVICE_SESSION_
#define __DEVICE_SESSION_
#pragma once

#include "../taskqueue.h"
#include "../message.h"
#include "device_message.h"
#include "../net_session.h"
#include "../../DataType.h"
#include "../../DataTypeDefine.h"
//#include "../../DevAgent.h"
#include "MsgHandleAgent.h"
using boost::asio::io_service;
using boost::asio::ip::tcp;

namespace hx_net
{
    class device_session:public net_session
	{
	public:
		device_session(boost::asio::io_service& io_service, 
			TaskQueue<msgPointer>& taskwork,ModleInfo & modinfo);
		~device_session();
		Dev_Type dev_type(){return DEV_OTHER;}
		void dev_base_info(DevBaseInfo& devInfo,string iId="local");
		//该连接是否包含此设备id
		bool is_contain_dev(string sDevId);
		//开始连接
		void connect(std::string hostname,unsigned short port,bool bReconnect=false);
		//udp连接
		void udp_connect(std::string hostname,unsigned short port);
		//断开连接
		void disconnect();
		//是否已建立连接
		bool is_connected(string sDevId="");
		//是否正在连接
		bool is_connecting();
		//是否已断开
		bool is_disconnected(string sDevId="");
		//获得协议转换器连接状态
		con_state       get_con_state();
		//获得设备运行状态（默认连接正常则运行正常）
		dev_run_state   get_run_state();
		//获得报警状态
        void get_alarm_state(string sDevId,map<int,std::pair<int,tm> >& cellAlarm);
		//执行通用指令
		bool excute_command(int cmdType,devCommdMsgPtr lpParam,e_ErrorCode &opResult);
		//清除所有报警标志
		void clear_all_alarm();
		//清除单设备报警
		void clear_dev_alarm(string sDevId);
		//发送消息
		bool sendRawMessage(unsigned char * data_,int nDatalen);
	protected:
		void start_read_head(int msgLen);//开始接收头
		void start_read_body(int msgLen);//开始接收体
		void start_read(int msgLen);
		void start_query_timer(unsigned long nSeconds=2000);
		void start_connect_timer(unsigned long nSeconds=3);
		void start_timeout_timer(unsigned long nSeconds=10);
		void set_con_state(con_state curState);
		void start_write(unsigned char* commStr,int commLen);
		void connect_timeout(const boost::system::error_code& error);
		void connect_time_event(const boost::system::error_code& error);
		void query_send_time_event(const boost::system::error_code& error);
		void  handler_data(string sDevId,DevMonitorDataPtr curDataPtr);
		void set_stop(bool bStop=true);
		bool is_stop();
        void close_all();
		//判断监测量是否报警
		bool ItemValueIsAlarm(string devId,float fValue,DevParamerMonitorItem &ItemInfo,
			                                            dev_alarm_state &alarm_state);
		void check_alarm_state(string sDevId,DevMonitorDataPtr curDataPtr,bool bMonitor);

		void save_monitor_record(string sDevId,DevMonitorDataPtr curDataPtr);

		bool is_need_save_data(string sDevId);

		string next_dev_id();

		//提交任务
		void task_count_increase();
		//任务递减
		void task_count_decrease();
		//任务数
		int  task_count();
		//等待任务结束
		void wait_task_end();
		//是否在监测时间段
		bool is_monitor_time(string sDevId);

		void sendSmsToUsers(int nLevel,string &sContent);

		bool excute_general_command(int cmdType,devCommdMsgPtr lpParam,e_ErrorCode &opResult);
	public:	
		void handle_connected(const boost::system::error_code& error);
		void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred);//通用消息头（分消息head，body）
		void handle_read_body(const boost::system::error_code& error, size_t bytes_transferred);//通用消息体
        void handle_udp_read(const boost::system::error_code& error,size_t bytes_transferred);//udp接收回调
		void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
		virtual void handle_write(const boost::system::error_code& error,size_t bytes_transferred);
		
	private:
		boost::mutex					stop_mutex_;
		bool								    stop_flag_;
		tcp::resolver                      resolver_;	
		udp::resolver                    uresolver_;

		tcp::endpoint                   endpoint_;
		udp::endpoint                   uendpoint_;
		boost::recursive_mutex          con_state_mutex_;
		con_state                       othdev_con_state_;
		othdevMsgPtr                    receive_msg_ptr_;
		boost::asio::deadline_timer     connect_timer_;//连接定时器
		boost::asio::deadline_timer     timeout_timer_;//连接超时定时器
		boost::asio::deadline_timer     query_timer_;//查询定时器   

		size_t                          command_timeout_count_;//命令执行超时次数
		size_t                          cur_msg_q_id_;//当前发送的消息序号
		TaskQueue<msgPointer>&          taskwork_;//任务队列应用(应修改为发射机特定任务)
		boost::recursive_mutex          data_deal_mutex;
		boost::recursive_mutex          alarm_state_mutex;
        map<string,map<int,std::pair<int,tm> > >           mapItemAlarmStartTime;//报警项报警开始时间
        map<string,map<int,std::pair<int,unsigned int> > > mapItemAlarmRecord;
		map<string,time_t>                               tmLastSaveTime;
        map<string,pair<CommandAttrPtr,HMsgHandlePtr> >   dev_agent_and_com;//add by lk 2013-11-26
		string                                           cur_dev_id_;//当前查询设备id

		ModleInfo                          &modleInfos;
		boost::mutex                        task_mutex_;
		int											task_count_;
		boost::condition                  task_end_conditon_;

		map<string,pTransmitterPropertyExPtr>       run_config_ptr;//moxa下设备配置
		pMoxaPropertyExPtr                          moxa_config_ptr;//moxa配置

#ifdef USE_STRAND
		io_service::strand              strand_;   //消息头与消息体同步
#endif
	};
}
#endif
