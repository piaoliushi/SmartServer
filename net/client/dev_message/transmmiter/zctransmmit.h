#ifndef ZCTRANSMMIT_H
#define ZCTRANSMMIT_H
#include "Transmmiter.h"
#include "../../device_session.h"
namespace hx_net{
class ZcTransmmit:public Transmmiter
{
public:
    ZcTransmmit(int subprotocol,int addresscode);

    int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
    void GetAllCmd(CommandAttribute &cmdAll);
private:
    int Zhc618F_Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
    int Zhc10KWTv_Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
    int Zhc3KWFm_Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
private:
    int m_subprotocol;
    int m_addresscode;  
};

#endif // ZCTRANSMMIT_H
}
