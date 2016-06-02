#pragma once
#include "base_message.h"
using namespace std;

namespace hx_net
{
class Media_message:public base_message
{
public:
    Media_message(session_ptr pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo);
    ~Media_message(void);
public:
    int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
	void GetAllCmd(CommandAttribute &cmdAll);
protected:
    int  parse_AC103CTR_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
private:
    session_ptr         m_pSession;//关联连接对象
    DeviceInfo           &d_devInfo;//设备信息
    boost::asio::deadline_timer     task_timeout_timer_;//控制任务执行定时器
};
}
