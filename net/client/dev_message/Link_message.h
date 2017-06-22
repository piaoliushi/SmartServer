#pragma once
#include "base_message.h"
#include "./snmp_pp/snmp_pp.h"
using namespace std;

//class Pdu;
//class SnmpTarget;
namespace hx_net
{
class Link_message:public base_message
{
public:
    Link_message(session_ptr pSession,DeviceInfo &devInfo);
    ~Link_message(void);
public:
    int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    int decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target);
    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
    void GetAllCmd(CommandAttribute &cmdAll);
    void Link_Callback(int reason, Snmp *session,Pdu &pdu, SnmpTarget &target);
protected:
    int  parse_SatelliteReceive_data(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target);
    void parse_Satellite_data_(Pdu &pdu, SnmpTarget &target);
    int  parse_TestReceive_data(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target);
    void parse_Testllite_data_(Pdu &pdu, SnmpTarget &target);

    int  parse_SingAptReceive_data(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target);
    void parse_SingApt_data(Pdu &pdu, SnmpTarget &target);
    void parse_DmpSwitch_data(Pdu &pdu, SnmpTarget &target);
    void parse_AsiApt_data(Pdu &pdu, SnmpTarget &target);
    void parse_weile_avsp_decorder_data(Pdu &pdu, SnmpTarget &target);

    void initOid();
    void init_SingApt_Oid();
    void init_Dmp_Oid();
    void init_AsiApt_Oid();
    void init_weile_avsp_decoder_Oid();
private:
     Pdu query_pdu;
    session_ptr m_pSession;//关联连接对象
    DeviceInfo           &d_devInfo;//设备信息
    boost::recursive_mutex          data_mutex;
    map<string,int> map_Oid;
    DevMonitorDataPtr      d_curData_ptr;
    //boost::shared_ptr<TaskQueue<DevMonitorDataPtr > > d_task_queue_ptr;//任务队列

};
}
