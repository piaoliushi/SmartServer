#pragma once
#include "../../../DataType.h"
#include "base_message.h"
using namespace std;

namespace hx_net
{
class Tsmt_message:public base_message
{
public:
    Tsmt_message(net_session *pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo);
    ~Tsmt_message(void);
public:
    void SetProtocol(int mainprotocol,int subprotocol);
    int  check_msg_header(unsigned char *data,int nDataLen);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
    int   cur_dev_state();
    void exec_task_now(int icmdType,int nResult);
protected:
    //启动定时控制
    void start_task_timeout_timer();
    //任务超时回调
    void schedules_task_time_out(const boost::system::error_code& error);
private:
    int      m_Subprotocol;//设备子协议号
    int                 m_mainprotocol;//主协议编号
    boost::asio::deadline_timer     task_timeout_timer_;//控制任务执行定时器
    net_session *m_pSession;//关联连接对象
    DeviceInfo     &d_devInfo;//设备信息
};
}
