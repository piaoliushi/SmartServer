#ifndef DEV_UPLOAD_HANDLER_X
#define DEV_UPLOAD_HANDLER_X

#pragma once

#include "../session.h"
#include "../taskqueue.h"
#include "dev_upload_message.h"
#include "server.h"
#include "../share_ptr_object_define.h"
using boost::asio::ip::tcp;
namespace net
{
	class dev_upload_handler_X:public session
	{
	public:	
		dev_upload_handler_X(boost::asio::io_service& io_service, 
			TaskQueue<msgPointer>& taskwork,server& srv);

		virtual ~dev_upload_handler_X(void);
		//开始接收消息头
		void start_read_head();	
		//发送消息到客户端
		bool sendMessage(e_MsgType _type,googleMsgPtr gMsgPtr);
        //启动心跳定时器
		void start_hb_timer();
        //获得最后心跳时间
		std::time_t get_last_hb_time();
        //设置最后心跳时间
		void set_last_hb_time(std::time_t t);
        //心跳回复
		int  heart_beat_rsp();
        //操作超时
		void handle_timeout(const boost::system::error_code& e);
        //关闭当前连接
		void close_handler();
		//关闭
		//void close_i();
        //处理普通监测数据
		void handler_data(DevMonitorDataPtr curDataPtr);
		//处理mp3数据
		void handler_mp3_data(boost::uint8_t uChannel,unchar_ptr curDataPtr);
        //处理设备回复命令响应数据
		bool handler_command_data(boost::uint8_t cmdType,uploadMsgPtr pMsg);
        //设备基本信息
//		void dev_base_info(DevBaseInfo& devInfo,string iId);
        //设置连接状态
// 		void set_con_state(con_state curState);
// 	    //获取连接状态
// 		con_state       get_con_state();
//         //获得运行状态
// 		dev_run_state   get_run_state();
// 		//设置运行状态
// 		void set_run_state(dev_run_state curState);
// 		//获得报警状态
// 		void get_alarm_state(map<int,std::pair<int,tm>>& cellAlarm);

		//切换音频通道
		bool switch_audio_channel(string sUser,int sNewChannel,e_ErrorCode &opResult);

		//执行通用指令
		bool excute_command(int cmdType,devCommdMsgPtr lpParam,e_ErrorCode &opResult);

	protected:
		//是否到了保存时间
		bool is_need_save_data();
		//保存报警数据
		void save_alarm_data(DevAlarmStatus *pDevAlarm);
		//更新报警时间
		void update_alarm_end_time(DevAlarmStatus *pDevAlarm);
		//提交任务
		void task_count_increase();
		//任务递减
		void task_count_decrease();
		//当前线程池任务数
		int  task_count();
		//等待任务结束
		void wait_task_end();
		//清除所有报警标志
		void clear_all_alarm();
		//是否在监测时间段
		bool is_monitor_time();
		//判断是否保存当前记录
		void save_monitor_record(DevMonitorDataPtr curDataPtr);
		//检测报警状态
		void detect_alarm_state(DevMonitorDataPtr curDataPtr,bool bMonitor);
		//当前项是否在报警
		bool ItemValueIsAlarm(float fValue,DevParamerMonitorItem &ItemInfo,
							  dev_alarm_state &alarm_state);
		//插入发送队列(暂未使用)
		int  putq(msgPointer msgPtr);
		//发送数据
		void start_write(msgPointer msgPtr);

	public:
		virtual void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred);
		virtual void handle_read_body(const boost::system::error_code& error, size_t bytes_transferred);
		virtual void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
	private:
		TaskQueue<msgPointer>&          taskwork_;
		HDevAgent                       dev_agent_;//协议解析和数据验证对象
		uploadMsgPtr                    receive_msg_ptr_;//接收缓冲区
		DevParamerInfo                  cur_upload_dev_info_;//当前上传设备信息
		boost::recursive_mutex          con_state_mutex_;
		con_state                       dev_con_state_;//设备当前连接状态
		boost::recursive_mutex          run_state_mutex_;
		dev_run_state                   dev_run_state_;//设备运行状态
		server&                         srv_;//服务管理对象
		boost::asio::deadline_timer     h_b_timer_;//心跳超时定时器
		boost::mutex                    hb_mutex_;
		std::time_t                     last_hb_time_;
		boost::mutex                    state_mutex_;
		usr_state                       usr_state_;
		time_t                          tmLastSaveTime;   //监测数据最后保存时间
		boost::mutex                    task_mutex_;
		int                             task_count_;
		boost::condition                task_end_conditon_;


		boost::recursive_mutex               alarm_state_mutex;    
        map<int,std::pair<int,tm> >           mapItemAlarmStartTime;//报警项报警开始时间
        map<int,std::pair<int,unsigned int> > mapItemAlarmRecord;

#ifdef USE_STRAND
		boost::asio::io_service::strand strand_;           //消息头与消息体同步
#endif 
	};
}

#endif
