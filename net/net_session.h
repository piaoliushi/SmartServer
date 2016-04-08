#ifndef NET_SESSION_H
#define NET_SESSION_H
#include "taskqueue.h"
#include "message.h"
//#include "./server/dev_upload_message.h"
#include "./client/device_message.h"
#include "share_ptr_object_define.h"
#include <string>
#include "../DataType.h"
#pragma once
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using namespace std;
namespace hx_net
{		
    class net_session;
    typedef boost::shared_ptr<net_session>  session_ptr;
    typedef boost::weak_ptr<net_session>    session_weak_ptr;

    class message;
    typedef boost::shared_ptr<message> msgPointer;
    class net_session: public boost::enable_shared_from_this<net_session>
	{
	public:

        net_session(boost::asio::io_service& io_service);
        virtual ~net_session(void);
		virtual Dev_Type dev_type(){return DEV_NODEFIN;}
        virtual void dev_base_info(DevBaseInfo& devInfo,string iId="local"){}
		virtual bool is_contain_dev(string sDevId){return false;}


		bool is_tcp();
		void set_tcp(bool bTcp);


		tcp::socket& socket();
        udp::socket& usocket();
        tcp::endpoint get_addr();
		udp::endpoint get_udp_addr();

		virtual con_state       get_con_state(){return con_disconnected;}
		virtual dev_run_state   get_run_state(){return dev_unknown;}
		virtual con_state       get_child_con_state(string sId){return con_disconnected;}
        virtual void            set_child_run_state(string sId,dev_run_state s){}
		virtual dev_run_state   get_child_run_state(string sId){return dev_unknown;}
        virtual void  get_alarm_state(map<int,std::pair<int,tm> >& cellAlarm){}
        virtual void  get_alarm_state(string sDevId,map<int,std::pair<int,tm> >& cellAlarm){}
        virtual void  clear_dev_alarm(string sDevId){}
        virtual void  clear_dev_state(string sDevId){}

        virtual void  set_con_state(con_state s){}
		virtual bool  is_connected(string sDevId=""){return false;}
        virtual void  reset_netstate(){}
		virtual bool  is_disconnected(string sDevId=""){return false;}

		virtual bool  is_running(string sDevId=""){return false;}
		virtual bool  is_shut_down(string sDevId=""){return false;}
		virtual int   heart_beat_rsp(){return -1;}
        virtual void  login_user(string sUser,string sPassword){}
        virtual void  logout_user(string sUser,string sPassword){}
        virtual void  login_user_ack(string sUser,string sPassword,msgPointer &pMsg){}
        virtual void  handover_ack(string sOldUser,string sNewUser,string sNewPassword,msgPointer &pMsg){}
        virtual void  user_sign_in_out(bool bIn,string sUser,string sPassword){}
        virtual void  user_sign_in_out_ack(int bIn,string sUser,string sPassword){}
        virtual void  user_duty_log(string sUserId,const string &sContent,int nType){}
        virtual void  check_station_working(checkWorkingReqMsgPtr pcheckWork){}
        virtual void  check_station_result_notify(checkWorkingNotifyMsgPtr pcheckWorkResult){}
        virtual void  logout_user_ack(string sUser,string sPassword,msgPointer &pMsg){}
        virtual void  dev_data_notify(msgPointer &pMsg){}
        virtual void  dev_netstate_update(msgPointer &pMsg){}
        virtual void  dev_workstate_update(msgPointer &pMsg){}
        virtual void  dev_alarmstate_update(msgPointer &pMsg){}
        virtual void  start_hb_timer(){}
        virtual void  start_read_head(){}

        virtual void  set_last_hb_time(std::time_t t){}
		virtual	bool  sendMessage(e_MsgType _type,googleMsgPtr gMsgPtr);
        virtual bool sendRawMessage(unsigned char * data_,int nDatalen){return false;}
		//打包发送实时多媒体数据（汇鑫760音频）
        virtual void send_monitor_data_message(string sStationid,string sDevid,int devType,
                                               unchar_ptr curData,DeviceMonitorItem &mapMonitorItem);
		//打包发送媒体数据
        virtual void send_monitor_data_message_ex(string sStationid,string sDevid,int devType,
                                unsigned char *curData,int nDataLen,DeviceMonitorItem &mapMonitorItem);
		//发送实时数据消息
        virtual void send_monitor_data_message(string sStationid,string sDevid,int devType,
                                               DevMonitorDataPtr curData,map<int,DeviceMonitorItem> &mapMonitorItem);
        virtual void send_data_to_client(string sDevid,DevMonitorDataPtr curData){}
		//打包发送设备连接状态消息
        virtual void send_net_state_message(string sStationid,string sDevid,string sDevName,int devType,
			                                con_state netState);
		//打包发送设备工作状态消息
        virtual void send_work_state_message(string sStationid,string sDevid,string sDevName,int devType,
			                                 dev_run_state runState);
		//打包发送设备报警状态消息
        virtual void send_alarm_state_message(string sStationid,string sDevid,string sDevName,
                                              int nCellId,string sCellName,int devType,int alarmState,
                                              string sStartTime,int alarmCount);
		//打包发送命令执行结果
        virtual void send_command_execute_result_message(string sStationid,string sDevid,int devType,string sDevName,
						                                 string sUsrName,e_MsgType nMsgType,e_ErrorCode eResult);
        virtual void excute_result_notify(string sDevId,int nDevType,e_MsgType oprType,int nResult){}
		//判断是否发短信与打电话
		void sendSmsAndCallPhone(int nAlarmLevel,string sContent);
		//开始处理监测数据
        virtual void start_handler_data(DevMonitorDataPtr curDataPtr,bool bCheckAlarm=true){}
		//开始处理mp3数据
        virtual void start_handler_mp3_data(boost::uint8_t nChannel,unchar_ptr curData){}
		//开始处理mp3数据扩展
        virtual void start_handler_mp3_data_ex(boost::uint8_t nChannel,unsigned char *curData,int nDataLen){}
		//-------------------------------------------transmitter--------------------------------//
		virtual void  handler_transmitter_data(DevMonitorDataPtr curDataPtr){};
		//-------------------------------------------client-------------------------------------//
        virtual void connect(std::string hostname,unsigned short port,bool bReconnect=false){}
        virtual void udp_connect(std::string hostname,unsigned short port){}

        virtual void disconnect(){}
		virtual void close_i();
        virtual void close_handler(){}
		//--------
        virtual loginAckMsgPtr  get_child_station_dev_status(){return loginAckMsgPtr();}

		//切换音频通道
        virtual bool excute_command(int cmdType,devCommdMsgPtr lpParam,e_ErrorCode &opResult){return false;}

        //发送命令到设备
        virtual void send_cmd_to_dev(string sDevId,int cmdType,int childId){}
        virtual bool start_exec_task(string sDevId,string sUser,e_ErrorCode &opResult,int cmdType){return false;}
	public:
		virtual void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred);
		virtual void handle_read_body(const boost::system::error_code& error, size_t bytes_transferred);
		virtual void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
	private:
		tcp::socket                     socket_;
		udp::socket                     udp_socket_;
		bool                            bTcp_;//是否为tcp连接
		boost::mutex          socket_mutex_;
    };

}

#endif
