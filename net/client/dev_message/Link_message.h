#pragma once
#include "base_message.h"
using namespace std;

namespace hx_net
{
class Link_message:public base_message
{
public:
    Link_message(session_ptr pSession,DeviceInfo &devInfo);
    ~Link_message(void);
public:
    int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
    int decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target);
    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
	void GetAllCmd(CommandAttribute &cmdAll);
protected:
    int  parse_SatelliteReceive_data(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target);
private:
    session_ptr m_pSession;//关联连接对象
    DeviceInfo           &d_devInfo;//设备信息
};
}
