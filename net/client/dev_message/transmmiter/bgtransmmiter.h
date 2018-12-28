#ifndef BGTRANSMMITER_H
#define BGTRANSMMITER_H
#pragma once
#include "Transmmiter.h"
namespace hx_net
{
    class BgTransmmiter:public Transmmiter
    {
    public:
        BgTransmmiter(int subprotocol,int addresscode);
        int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
        int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        bool IsStandardCommand();
        void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
        void GetAllCmd(CommandAttribute &cmdAll);
    private:
        int BG300Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        int BGAm1KwData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        int OnBeiguangFM1KW(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        int BeiGuangFm3KWData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        int BeiGuangFm5KWData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        int BeiGuangFm10KWData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        int Beiguang618bData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        int BeiguangCDR1KwData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
    private:
        int m_subprotocol;
        int m_addresscode;
    };
}
#endif // BGTRANSMMITER_H
