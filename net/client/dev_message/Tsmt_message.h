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
    int  check_msg_header(unsigned char *data,int nDataLen);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
    int   cur_dev_state();
    void exec_task_now(int icmdType,string sUser,e_ErrorCode &eErrCode);
    //启动定时控制
    void start_task_timeout_timer();
    //设备运行状态
    bool is_running();
    bool is_shut_down();
    bool is_detecting();
    //获得运行状态
    int get_run_state();
    //设置运行状态
    void set_run_state(int curState);
    void reset_run_state();


protected:
    //任务超时回调
    void schedules_task_time_out(const boost::system::error_code& error);
    //检测运行状态
    void detect_run_state(DevMonitorDataPtr curDataPtr);
    void exec_trunon_task_(int nType);
    void exec_trunoff_task_();
     void  excute_task_cmd();
     void  GetResultData(DevMonitorDataPtr data_ptr);
    void   CreateObject();
    bool  cmd_excute_is_ok();
private:
    int      m_Subprotocol;//设备子协议号
    int                 m_mainprotocol;//主协议编号
    boost::asio::deadline_timer     task_timeout_timer_;//控制任务执行定时器
    dev_session_ptr m_pSession;//关联连接对象
    DeviceInfo     &d_devInfo;//设备信息
    pDevicePropertyExPtr d_devProperty;
    boost::recursive_mutex          run_state_mutex_;//设备运行状态互斥量
    int                    dev_run_state_;//设备运行状态
    int                    d_run_count_;//运行状态计数
    int                    d_shutdown_count_;//关机状态计数
    string               d_cur_user_;//当前用户
    int                    d_cur_task_;//当前任务
    DeviceInfo                *d_relate_tsmt_ptr_;//关联发射机
    DeviceInfo                *d_relate_antenna_ptr_;//关联天线
    int                   d_Host_;//主机标志(-1:未绑定主备属性)
    bool                d_relate_Agent_;//主机代理标志
    bool                d_antenna_Agent_;//天线代理标志
    time_t             d_OprStartTime;//提交控制命令开始时间
    Transmmiter   *m_ptransmmit;

};
}
