#pragma once
#include "base_message.h"
using namespace std;

namespace hx_net
{
class Envir_message:public base_message
{
public:
    Envir_message(session_ptr pSession,DeviceInfo &devInfo);
    ~Envir_message(void);
public:
    int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
	void GetAllCmd(CommandAttribute &cmdAll);

    //执行任务
    void exec_task_now(int icmdType,string sUser,e_ErrorCode &eErrCode,
                                     bool bSnmp,Snmp *snmp,CTarget *target);
    //执行通用命令
    void exec_general_task(int icmdType,string sUser,devCommdMsgPtr lpParam,e_ErrorCode &eErrCode);
    //执行联动命令
    void exec_action_task_now(map<int,vector<ActionParam> > &param,int actionType,string sUser,e_ErrorCode &eErrCode);
protected:
    int  parse_AC103CTR_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    int EnvironWS302(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    int EnvironTHB11RS(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    int KD40RData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    int C2000_M21A_Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    int FRT_X06A_Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    int C2000_A2_Data(unsigned char *data, DevMonitorDataPtr data_ptr,
                                     int nDataLen, int &iaddcode);
    int C2000_BB3_Data(unsigned char *data, DevMonitorDataPtr data_ptr,
                       int nDataLen, int &iaddcode);

    int NT511_AD_Data(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen,
                      int &iaddcode);
private:
    session_ptr           m_pSession;//关联连接对象
    DeviceInfo           &d_devInfo;//设备信息
    DevMonitorDataPtr     d_curData_ptr;
    time_t                d_OprStartTime;//提交控制命令开始时间
};
}
