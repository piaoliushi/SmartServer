#ifndef ASTRANSMMIT_H
#define ASTRANSMMIT_H
#pragma once
#include "Transmmiter.h"
#include "../Tsmt_message.h"
namespace hx_net{
class AsTransmmit:public Transmmiter
{
public:
    AsTransmmit(Tsmt_message_ptr ptsmessage,int subprotocol,int addresscode);
    int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
  //  int decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target,int& runstate);
    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
    void GetAllCmd(CommandAttribute &cmdAll);
private:
    int OnJiaHui_1Kw(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
    int JiaHui_TV_3Kw(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
private:
    int m_subprotocol;
    int m_addresscode;
    Tsmt_message_ptr m_pmessage;
};
}
#endif // ASTRANSMMIT_H
