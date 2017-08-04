#ifndef ZCTRANSMMIT_H
#define ZCTRANSMMIT_H
#include "Transmmiter.h"
namespace hx_net{
class ZcTransmmit:public Transmmiter
{
    typedef struct
    {
        unsigned char uHeadFlag[2]; //固定的命令头 10 02
        int uSendID;
        int uRecvID;
        unsigned char uFunc;
        unsigned char uSubFunc;
        unsigned short uDatalen;
    }AckHeader,*pAckHeader;
public:
    ZcTransmmit(int subprotocol,int addresscode);

    int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
    void GetAllCmd(CommandAttribute &cmdAll);
private:
    int Zhc618F_Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
private:
    int m_subprotocol;
    int m_addresscode;
};

#endif // ZCTRANSMMIT_H
}
