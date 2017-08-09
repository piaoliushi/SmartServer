#ifndef HARRISTRANSMMIT_H
#define HARRISTRANSMMIT_H
#pragma once
#include "Transmmiter.h"
#include "../Tsmt_message.h"
namespace hx_net{
          class CHarrisTransmmit:public Transmmiter
          {
          public:
              CHarrisTransmmit(Tsmt_message_ptr ptsmessage,int subprotocol,int addresscode);
              int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
              int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
              int decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target,int& runstate);
              bool IsStandardCommand();
              void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit){};
              void GetAllCmd(CommandAttribute &cmdAll);
              void harris_Callback(int reason, Snmp *session,Pdu &pdu, SnmpTarget &target);
              bool exec_cmd(Snmp *snmp, int cmdtype, CTarget *target);
          protected:
              void initOid();
          private:
              int OnHarrisAm50(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);

              int get_snmp(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target);
              void harris_decode(int reason, Snmp *session,Pdu &pdu, SnmpTarget &target);
          private:
              int m_subprotocol;
              int m_addresscode;
              Pdu query_pdu;
              DevMonitorDataPtr curdata_ptr;
           //   Pdu snmp_pdu;
              boost::recursive_mutex data_mutex;
              Tsmt_message_ptr m_pmessage;
              map<string,int> map_Oid;
          };
}
#endif // HARRISTRANSMMIT_H
