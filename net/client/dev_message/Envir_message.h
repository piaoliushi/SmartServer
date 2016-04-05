#pragma once
#include "base_message.h"
using namespace std;

namespace hx_net
{
class Envir_message:public base_message
{
public:
    Envir_message(net_session *pSession,DeviceInfo &devInfo);
    ~Envir_message(void);
public:
    void SetProtocol(int mainprotocol,int subprotocol);
    int  check_msg_header(unsigned char *data,int nDataLen);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
protected:
    int  parse_AC103CTR_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
private:
    int              m_Subprotocol;//设备子协议号
    int              m_mainprotocol;//主协议编号
    net_session *m_pSession;//关联连接对象
    DeviceInfo           &d_devInfo;//设备信息
};
}
