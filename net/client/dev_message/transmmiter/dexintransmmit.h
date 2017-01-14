#ifndef DEXINTRANSMMIT_H
#define DEXINTRANSMMIT_H
#pragma once
#include "Transmmiter.h"
namespace hx_net{
          class DeXinTransmmit:public Transmmiter
          {
          public:
              DeXinTransmmit(int subprotocol,int addresscode);
              ~DeXinTransmmit();

              int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
              int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
              bool IsStandardCommand();
              void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
              void GetAllCmd(CommandAttribute &cmdAll);
          private:
              int OnDigTVData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
              void UnitData(unsigned char *data,DevMonitorDataPtr data_ptr,int iparam,int ipalen,int &imodnum,int& runstate);
          private:
              int m_subprotocol;
              int m_addresscode;
          };
}
#endif // DEXINTRANSMMIT_H
