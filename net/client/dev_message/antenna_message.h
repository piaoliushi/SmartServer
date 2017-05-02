#pragma once
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
private:
    int parse_HX_981(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
private:
    session_ptr  m_pSession;//关联连接对象
    DeviceInfo           &d_devInfo;//设备信息
    DevMonitorDataPtr d_curData_ptr;
};
}
