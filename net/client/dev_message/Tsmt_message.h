#pragma once
#include "../../../DataType.h"
#include "base_message.h"
using namespace std;

namespace hx_net
{
class Transmmiter;
class Tsmt_message:public base_message
{
public:
    Tsmt_message(session_ptr  pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo,
                 pDevicePropertyExPtr devProperty);
    ~Tsmt_message(void);
public:
    void SetProtocol(int mainprotocol,int subprotocol);
    void GetAllCmd(CommandAttribute &cmdAll);
    int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    int  decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target);
    //http消息解析
    int  decode_http_msg(const string &data,DevMonitorDataPtr data_ptr,CmdType cmdType,int number);

    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
    void GetSignalCommand(int cmmType,int nIndex,CommandUnit &cmdUnit);
    int  cur_dev_state();
    void exec_task_now(int icmdType,string sUser,e_ErrorCode &eErrCode,map<int,string> &mapParam,
                       bool bSnmp=false,Snmp *snmp=NULL,CTarget *target=NULL);
    //执行联动命令
    void exec_action_task_now(map<int,vector<ActionParam> > &param,int actionType,
                                                string sUser,e_ErrorCode &eErrCode);
    //启动定时控制
    void start_task_timeout_timer();
    void start_step_task_timeout_timer();
    //设备运行状态
    bool is_running();
    bool is_shut_down();
    bool is_detecting();
    //获得运行状态
    int get_run_state();
    //设置运行状态
    void set_run_state(int curState);
    void reset_run_state();
    void aysnc_data(DevMonitorDataPtr curDataPtr);

    //是否需要清除告警
    bool  is_need_clear_alarm();

    void exec_general_task(int icmdType, string sUser, devCommdMsgPtr lpParam, e_ErrorCode &eErrCode);

protected:
    //任务超时回调
    void  schedules_task_time_out(const boost::system::error_code& error);
    void  schedules_step_task_time_out(const boost::system::error_code& error);
    //检测运行状态
    void  detect_run_state(DevMonitorDataPtr curDataPtr);
    void  exec_trunon_task_(int nType,e_ErrorCode &eErrCode,int &nExcutResult);
    void  exec_trunoff_task_(e_ErrorCode &eErrCode,int &nExcutResult);
    void  exec_other_task_(e_ErrorCode &eErrCode);
    void  exec_step_trunon_task_(e_ErrorCode &eErrCode,int &nExcutResult);
    void  exec_step_trunoff_task_(e_ErrorCode &eErrCode,int &nExcutResult);
    void  excute_task_cmd(e_ErrorCode &eErrCode,int &nExcutResult);
    void  GetResultData(DevMonitorDataPtr data_ptr);
    void  CreateObject();
    bool  cmd_excute_is_ok();
    void  check_device_alarm(int nAlarmType);

    //获取分几开关当前步骤超时设置
    int getsteptimeout();
    bool is_step_task_exeok();
    bool is_step_all_exeok();

private:
    int                             m_Subprotocol;//设备子协议号
    int                             m_mainprotocol;//主协议编号
    boost::asio::deadline_timer     task_timeout_timer_;//控制任务执行定时器
    dev_session_ptr                 m_pSession;//关联连接对象
    DeviceInfo                      &d_devInfo;//设备信息
    pDevicePropertyExPtr            d_devProperty;
    boost::recursive_mutex          run_state_mutex_;//设备运行状态互斥量
    int                    dev_run_state_;//设备运行状态
    int                    d_run_count_;//运行状态计数
    int                    d_shutdown_count_;//关机状态计数
    string                 d_cur_user_;//当前用户
    int                    d_cur_task_;//当前任务
    map<int,string>        d_cur_task_param_;//当前任务参数
    DeviceInfo            *d_relate_tsmt_ptr_;//关联发射机
    DeviceInfo            *d_relate_antenna_ptr_;//关联天线
    int                    d_Host_;//主机标志(-1:未绑定主备属性//0->主机，1->备机)
    bool                   d_relate_Agent_;//主机代理标志
    bool                   d_antenna_Agent_;//天线代理标志
    bool                   d_onekeyopen_996;//996一键开机标志
    bool                   d_onekeyopen_soft;//通用软件一键开机标志
    time_t                 d_OprStartTime;//提交控制命令开始时间
    Transmmiter           *d_ptransmmit;
    DevMonitorDataPtr      d_curData_ptr;
    Snmp                  *d_cur_snmp;
    bool                   d_bUse_snmp;
    CTarget               *d_cur_target;
    int                   d_curStep;//分步骤开关机当前步骤号
    StepCommandAttribute  d_step_oc_cmd;
    DevMonitorDataPtr      d_checkData_ptr;
};
typedef boost::shared_ptr<hx_net::Tsmt_message>  Tsmt_message_ptr;
typedef boost::weak_ptr<hx_net::Tsmt_message>    Tsmt_message_weak_ptr;
}
