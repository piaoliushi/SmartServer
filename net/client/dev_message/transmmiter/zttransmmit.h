#ifndef ZTTRANSMMIT_H
#define ZTTRANSMMIT_H
#pragma once
#include "Transmmiter.h"
namespace hx_net
{
    class ZtTransmmit:public Transmmiter
    {
    public:
        ZtTransmmit(int subprotocol,int addresscode);
        int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
        int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        bool IsStandardCommand();
        void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
        void GetAllCmd(CommandAttribute &cmdAll);
    private:
        int ZtAm10KwData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        int ZtAm1KwData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
    private:
        int m_subprotocol;
        int m_addresscode;
    };
}
#endif // ZTTRANSMMIT_H
