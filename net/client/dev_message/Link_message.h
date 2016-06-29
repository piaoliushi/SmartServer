#pragma once
#include "base_message.h"
using namespace std;

class Pdu;
class SnmpTarget;
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
    void Link_Callback(int reason, Snmp *session,Pdu &pdu, SnmpTarget &target);
protected:
    int  parse_SatelliteReceive_data(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target);
    void parse_Satellite_data_(Pdu &pdu, SnmpTarget &target);

private:
    session_ptr m_pSession;//关联连接对象
    DeviceInfo           &d_devInfo;//设备信息
    boost::recursive_mutex          data_mutex;
    //DevMonitorDataPtr       d_data_ptr;
    boost::shared_ptr<TaskQueue<DevMonitorDataPtr > > d_task_queue_ptr;//任务队列
};
}
