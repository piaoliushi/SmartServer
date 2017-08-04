#ifndef CKANGTETRANSMMIT_H
#define CKANGTETRANSMMIT_H
#include "Transmmiter.h"
namespace hx_net{
          class CKangteTransmmit:public Transmmiter
          {
          public:
              CKangteTransmmit(int subprotocol,int addresscode);
              int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);

              int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);

              int  decode_msg_body(const string &data,DevMonitorDataPtr data_ptr,CmdType cmdType,int number, int &runstate);

              bool IsStandardCommand();
              void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
              void GetSignalCommand(int cmmType,int nIndex,CommandUnit &cmdUnit);
              void GetAllCmd(CommandAttribute &cmdAll);
          private:
              int KTTvNetPortData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
              int KT_Http_NetPortData(const string &data,DevMonitorDataPtr data_ptr,
                                                        CmdType cmdType,int number, int &runstate);
          private:
              int m_subprotocol;
              int m_addresscode;
          };
}
#endif // CKANGTETRANSMMIT_H
