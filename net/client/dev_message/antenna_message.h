#pragma once
#include "../../../DataType.h"
#include "base_message.h"
using namespace std;
namespace hx_net {
class Antenna_message : public base_message
{
public:
    Antenna_message(session_ptr pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo);
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
    bool dev_can_excute_cmd();
    void excute_task_cmd(e_ErrorCode &eErrCode,int &nExcutResult);
    void exec_task_now(int icmdType,string sUser,e_ErrorCode &eErrCode,map<int,string> &mapParam,int nMode=0,
                               bool bSnmp=false,Snmp *snmp=NULL,CTarget *target=NULL);

    void switch_antenna_pos(e_ErrorCode &eErrCode,int &nExcutResult);
    bool can_switch_antenna();
    void get_relate_dev_info(string &sStationId,string &sDevId,string &sAttenaId);
protected:
    int   parse_HX_981(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    bool  cmd_excute_is_ok();
    void  restart_task_timeout_timer();
    void  schedules_task_time_out(const boost::system::error_code& error);
private:
    dev_session_ptr  m_pSession;
    boost::asio::deadline_timer     task_timeout_timer_;

    boost::recursive_mutex    can_switch_mutex_;
    bool   can_switch_;
    DeviceInfo           &d_devInfo;
    DevMonitorDataPtr d_curData_ptr;
    boost::recursive_mutex          run_state_mutex_;
    int dev_run_state_;
    string                 d_cur_user_;
    int                    d_cur_task_;
    DeviceInfo            *d_relate_host_tsmt_ptr_;
    DeviceInfo            *d_relate_backup_tsmt_ptr_;
    bool                   d_antenna_Agent_;
    bool                   d_onekeyopen_996;
    bool                   d_onekeyopen_soft;
};
}
