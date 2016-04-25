#pragma once
#include "base_message.h"
using namespace std;

namespace hx_net
{
class Envir_message:public base_message
{
public:
    Envir_message(session_ptr pSession,DeviceInfo &devInfo);
    ~Envir_message(void);
public:
    int  check_msg_header(unsigned char *data,int nDataLen);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
	void GetAllCmd(CommandAttribute &cmdAll);
protected:
    int  parse_AC103CTR_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
	int EnvironWS302(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
	int EnvironTHB11RS(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
	int KD40RData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
	int C2000_M21A_Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
	int FRT_X06A_Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
	int On992Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
private:
    session_ptr m_pSession;//关联连接对象
    DeviceInfo           &d_devInfo;//设备信息
};
}
