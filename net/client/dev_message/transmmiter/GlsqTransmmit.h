#ifndef GLSQTRANSMMIT_H
#define GLSQTRANSMMIT_H
#pragma once
#include "Transmmiter.h"
namespace hx_net{
    class GlsqTransmmit:public Transmmiter
    {
    public:
      GlsqTransmmit(int subprotocol,int addresscode);
      ~GlsqTransmmit();

      int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
      int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
      bool IsStandardCommand();
      void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
      void GetAllCmd(CommandAttribute &cmdAll);
    private:
      int Glsq_T_Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
      int GetExcitData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int excitnum);
      int GetPowerampData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int pampnum);
    private:
        int m_subprotocol;
        int m_addresscode;
     };
}
#endif // GLSQTRANSMMIT_H
