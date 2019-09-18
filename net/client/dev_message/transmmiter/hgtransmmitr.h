#ifndef HGTRANSMMITR_H
#define HGTRANSMMITR_H
#pragma once
#include "Transmmiter.h"
namespace hx_net{
class HgTransmmitr:public Transmmiter
{
public:
    HgTransmmitr(int subprotocol,int addresscode);
    int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
    void GetAllCmd(CommandAttribute &cmdAll);
private:
    int Hg100KwData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
    int Hg50KwData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
    int Hg10KwData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
    int Hg10KwexData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
    int Hg1Kw4Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
private:
    int m_subprotocol;
    int m_addresscode;
};
}
#endif // HGTRANSMMITR_H
