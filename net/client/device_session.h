#ifndef  __DEVICE_SESSION_
#define __DEVICE_SESSION_
#pragma once

#include "../taskqueue.h"
#include "../message.h"
#include "device_message.h"
#include "../net_session.h"
#include "../../DataType.h"
#include "../../DataTypeDefine.h"
#include "MsgHandleAgent.h"
#include "http_request_session.h"
#include "parse_ass_device.h"
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
class Snmp;
class CTarget;

namespace hx_net
{
    class device_session:public net_session
    {
        friend class Tsmt_message;
    public:
        device_session(boost::asio::io_service& io_service,
            ModleInfo & modinfo,http_request_session_ptr &httpPtr);
        ~device_session();
        Dev_Type dev_type(){return DEV_OTHER;}
        //初始化设备配置
        void init_session_config();
        //设备基本信息
        void dev_base_info(DevBaseInfo& devInfo,string iId="local");
        //该连接是否包含此设备id
        bool is_contain_dev(string sDevId);
        //连接
        void connect();
        //开始连接
        void connect(std::string hostname,unsigned short port,bool bReconnect=false);
        //udp连接
        void udp_connect(std::string hostname,unsigned short port);
        //agent 连接，适用于http,snmp
        void agent_connect(std::string hostname,unsigned short port);
        //获取同步网络数据，使用http,snmp
        void get_sync_net_data();
        //断开连接
        void disconnect();
        //是否已建立连接
        bool is_connected();//string sDevId=""
        //是否正在连接
        bool is_connecting();
        //是否已断开
        bool is_disconnected(string sDevId="");
        //获得协议转换器连接状态
        con_state       get_con_state();
        //获得设备运行状态（默认连接正常则运行正常）
        dev_run_state   get_run_state(string sDevId);
        //获得报警状态
        void get_alarm_state(string sDevId,map<int,map<int,CurItemAlarmInfo> >& cellAlarm);
        //执行通用指令
        bool excute_command(int cmdType,devCommdMsgPtr lpParam,e_ErrorCode &opResult);
        //清除所有报警标志
        void clear_all_alarm();
        //清除单设备报警
        void clear_dev_alarm(string sDevId);
        //清理设备未设置的告警项
        void clear_dev_item_alarm(string sDevId,int nitemId);
        //发送消息
        bool sendRawMessage(unsigned char * data_,int nDatalen);
        //启动定时控制
        void start_task_schedules_timer();
        //发送命令到设备
        void send_cmd_to_dev(string sDevId,int cmdType,int childId);
        //更新运行图
        void update_monitor_time(string devId,map<int,vector<Monitoring_Scheduler> >& monitorScheduler,
                                 vector<Command_Scheduler> &cmmdScheduler);
        //更新告警配置
        void  update_dev_alarm_config(string sDevId,DeviceInfo &devInfo);

        //开始处理监测数据
        void start_handler_data(string sDevId,DevMonitorDataPtr curDataPtr,bool bCheckAlarm=true);
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
        void check_alarm_state(string sDevId,DevMonitorDataPtr curDataPtr,bool bMonitor);

        void save_monitor_record(string sDevId,DevMonitorDataPtr curDataPtrconst,const map<int,DeviceMonitorItem> &mapMonitorItem);

        bool is_need_save_data(string sDevId);
        //判断当前时间是否需要上传
        bool is_need_report_data(string sDevId);

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

        bool  excute_general_command(int cmdType,devCommdMsgPtr lpParam,e_ErrorCode &opResult);


        //-------2016-3-30------------------------//
        void  parse_item_alarm(string devId,float fValue,DeviceMonitorItem &ItemInfo,bool &bAlarmNow);
        void  record_alarm_and_notify(string &devId,float fValue,const float &fLimitValue,bool bMod,
                                        DeviceMonitorItem &ItemInfo,CurItemAlarmInfo &curAlarm);
        //设备断线告警（博汇）
        void send_device_data_state_notify(string sDevId,bool bHaveData);

        //打开串口
        void   open_com();

        int  con_mod();
    protected:
        void handle_connected(const boost::system::error_code& error);
        void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred);//通用消息头（分消息head，body）
        void handle_read_body(const boost::system::error_code& error, size_t bytes_transferred);//通用消息体
        void handle_udp_read(const boost::system::error_code& error,size_t bytes_transferred);//udp接收回调
        void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
        void schedules_task_time_out(const boost::system::error_code& error);
        virtual void handle_write(const boost::system::error_code& error,size_t bytes_transferred);
       //开始执行任务
        bool start_exec_task(string sDevId,string sUser,e_ErrorCode &opResult,int cmdType);
        void notify_client_execute_result(string sDevId,string devName,string user,int cmdType, tm *pCurTime,bool bNtfFlash,int eResult);
        void set_opr_state(string sdevId,dev_opr_state curState);
        dev_opr_state get_opr_state(string sdevId);
    private:
        tcp::resolver                     resolver_;
        udp::resolver                    uresolver_;

        tcp::endpoint                     endpoint_;
        udp::endpoint                    uendpoint_;
        boost::recursive_mutex      con_state_mutex_;
        con_state                          othdev_con_state_;
        othdevMsgPtr                    receive_msg_ptr_;
        boost::asio::deadline_timer     connect_timer_;//连接定时器
        boost::asio::deadline_timer     timeout_timer_;//连接超时定时器
        boost::asio::deadline_timer     query_timer_;//查询定时器
        boost::asio::deadline_timer     schedules_task_timer_;//控制任务执行定时器

        size_t                          query_timeout_count_;//查询命令执行超时次数
        size_t                          cur_msg_q_id_;//当前发送的消息序号

        boost::recursive_mutex          data_deal_mutex;
        boost::recursive_mutex          alarm_state_mutex;
        //devid<itemid<iLimittype,info> > >
        map<string ,map<int,map<int,CurItemAlarmInfo> > > mapItemAlarm;//设备监控量告警信息

        CurItemAlarmInfo  netAlarm;//通讯异常告警
        map<string,time_t>                               tmLastSaveTime;
        map<string,time_t>                               tmLastSendHttpTime;
        map<string,pair<CommandAttrPtr,HMsgHandlePtr> >   dev_agent_and_com;//add by lk 2013-11-26
        string                                           cur_dev_id_;//当前查询设备id

        ModleInfo                           &modleInfos_;
        boost::mutex                        task_mutex_;
        int									task_count_;
        boost::condition                    task_end_conditon_;

        map<string,pDevicePropertyExPtr>    run_config_ptr;//moxa下设备配置
        pMoxaPropertyExPtr                  moxa_config_ptr;//moxa配置

#ifdef USE_STRAND
        io_service::strand              strand_;   //消息头与消息体同步
#endif

        http_request_session_ptr   &http_ptr_;

        boost::recursive_mutex                    opr_state_mutex_;
        map<string,dev_opr_state>              dev_opr_state_;//设备控制命令发送状态
        boost::asio::io_service&                   io_service_;

        boost::recursive_mutex                    update_time_schedule_mutex_;//更新运行图锁
        boost::recursive_mutex                    update_cmd_schedule_mutex_;//更新运行图锁
        boost::recursive_mutex                    update_alarm_config_mutex_;//更新告警配置锁

        Snmp    *snmp_ptr_;
        CTarget *target_ptr_;

        boost::shared_ptr<boost::asio::serial_port> pSerialPort_ptr_;//串口操作
        bool all_dev_is_use_;

        map<string,parse_ass_dev_ptr>   map_dev_ass_parse_ptr_;//
    };
    typedef boost::shared_ptr<hx_net::device_session> dev_session_ptr;
    typedef boost::weak_ptr<hx_net::device_session>    dev_session_weak_ptr;
}


#endif
