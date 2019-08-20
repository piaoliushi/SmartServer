#ifndef SGTRANSMMIT_H
#define SGTRANSMMIT_H
#pragma once
#include "Transmmiter.h"

namespace hx_net{
      class SgTransmmit:public Transmmiter
      {
      public:
          SgTransmmit(int subprotocol,int addresscode);

          int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
          int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
          bool IsStandardCommand();
          void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
          void GetAllCmd(CommandAttribute &cmdAll);
      private:
          int SX762_5KWData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
          int RY_3KwPdmData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
          int ShanGuangFM1_300W(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate);
          int RY_1KwPdmData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
          int SX_10KwAmData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
      private:
          int m_subprotocol;
          int m_addresscode;
      };
}
#endif // SGTRANSMMIT_H
