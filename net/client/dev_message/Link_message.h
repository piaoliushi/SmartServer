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
    void GetSignalCommand(map<int, string> mapParam, CommandUnit &cmdUnit);
    void GetAllCmd(CommandAttribute &cmdAll);
    void Link_Callback(int reason, Snmp *session,Pdu &pdu, SnmpTarget &target);

    void exec_task_now(int icmdType,string sUser,e_ErrorCode &eErrCode,map<int,string> &mapParam,int nMode=0,
                                       bool bSnmp=false,Snmp *snmp=NULL,CTarget *target=NULL);
    void exec_general_task(int icmdType,string sUser,devCommdMsgPtr lpParam,
                                           e_ErrorCode &eErrCode);

    int getBelongChannelIdFromMonitorItem(int monitorItemId);
    void Link_TimeoutCallback(bool bState);
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
    void parse_weile_avsp_apt_data(Pdu &pdu, SnmpTarget &target);


    void  GetResultData(DevMonitorDataPtr data_ptr);

    void initOid();
    void init_SingApt_Oid();
    void init_Dmp_Oid();
    void init_AsiApt_Oid();
    void init_smsx_asi_decoder_Oid();
    void init_weile_avsp_decoder_Oid();
    void init_avsApt_Oid();
    void init_smsx_asi_Oid();
    void init_normal_snmp_oid();

    int decode_0401AV(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode,int &curChannel);
    int decode_0401DA(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode,int &curChannel);
    int decode_0401DABS(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode,int &curChannel);
    int decode_0214DA(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode);
    int decode_0401SP(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode,int &curChannel);
    int decode_JC5103(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode,int &curChannel);
    int decode_6300(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode);
    int decode_6300_AD(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode);
    int decode_9020(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode);
    int decode_0401AP(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode,int &curChannel);
private:
    void GetSwitchCmd(devCommdMsgPtr lpParam,CommandUnit &cmdUnit,int &channelId);
    void GetControlModCmd(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
    void GetControlModCmdByModId(int modId, CommandUnit &cmdUnit);
    void GetSwitchCmdByChannel(int channelId, CommandUnit &cmdUnit);

    void DAS_5103_ACK(unsigned char *data);
private:
    Pdu query_pdu;
    dev_session_ptr        m_pSession;//关联连接对象
    DeviceInfo            &d_devInfo;//设备信息
    //boost::recursive_mutex          data_mutex;
    map<string,int>        map_Oid;
    DevMonitorDataPtr      d_curData_ptr;

    string                 d_cur_user_;//当前用户
    int                    d_cur_task_;//当前任务

    boost::recursive_mutex          switch_channel_mutex;
    int curRoute_;//当前在哪一路（针对切换器）
    //boost::shared_ptr<TaskQueue<DevMonitorDataPtr > > d_task_queue_ptr;//任务队列

};
}
