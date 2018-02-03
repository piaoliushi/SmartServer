#pragma once
#include "../../../DataType.h"
#include "base_message.h"
using namespace std;
namespace hx_net {
class Antenna_message : public base_message
{
public:
    Antenna_message(session_ptr pSession,DeviceInfo &devInfo);
    ~Antenna_message();
public:
    int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
    void GetAllCmd(CommandAttribute &cmdAll);
    bool is_running();
    bool is_shut_down();
    bool is_detecting();
    int  get_run_state();
    void reset_run_state();
    void set_run_state(int curState);
    void excute_task_cmd(e_ErrorCode &eErrCode,int &nExcutResult);
    //执行命令
    void exec_task_now(int icmdType,string sUser,e_ErrorCode &eErrCode,map<int,string> &mapParam,
                               bool bSnmp=false,Snmp *snmp=NULL,CTarget *target=NULL);

    void switch_antenna_pos(e_ErrorCode &eErrCode,int &nExcutResult);
    //设置天线位置，只有

private:
    int   parse_HX_981(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    bool  cmd_excute_is_ok();
private:
    dev_session_ptr  m_pSession;//关联连接对象
    DeviceInfo           &d_devInfo;//设备信息
    DevMonitorDataPtr d_curData_ptr;
    boost::recursive_mutex          run_state_mutex_;//设备运行状态互斥量
    int dev_run_state_;
    string                 d_cur_user_;//当前用户
    int                    d_cur_task_;//当前任务
};
}
