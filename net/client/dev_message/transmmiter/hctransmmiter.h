#ifndef HCTRANSMMITER_H
#define HCTRANSMMITER_H
#pragma once
#include "Transmmiter.h"
namespace hx_net
{
     class HcTransmmiter:public Transmmiter
     {
     public:
           HcTransmmiter(int subprotocol,int addresscode);
           int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
           int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
           bool IsStandardCommand();
           void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
           void GetAllCmd(CommandAttribute &cmdAll);
     private:
           int OnHangChunHCF5KW(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
           int HcCdr100_300W(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
           int HcCdr1KW(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
           int HcCdr3KW(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
           int HcCdr10KW(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
     private:
         int m_subprotocol;
         int m_addresscode;
     };
}
#endif // HCTRANSMMITER_H
