#ifndef GSBRTRANSMMIT_H
#define GSBRTRANSMMIT_H
#pragma once
#include "Transmmiter.h"
//#include <boost/thread/recursive_mutex.hpp>
//class Pdu;
//class SnmpTarget;
namespace hx_net{
         class GsbrTransmmit:public Transmmiter
         {
          public:
               GsbrTransmmit(int subprotocol,int addresscode);
               ~GsbrTransmmit();
               int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
               int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
               int decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target,int& runstate);
               bool IsStandardCommand();
               void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
               void GetAllCmd(CommandAttribute &cmdAll);
               void gsbr_Callback(int reason, Snmp *session,Pdu &pdu, SnmpTarget &target);
           private:
               int get_snmp(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target);
           private:
               int m_subprotocol;
               int m_addresscode;
               DevMonitorDataPtr curdata_ptr;
               Pdu snmp_pdu;
               boost::recursive_mutex data_mutex;
         };
}
#endif // GSBRTRANSMMIT_H
