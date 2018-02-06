#pragma once
#include "../../../DataType.h"
#include "base_message.h"
#include "./104/iec104.h"
using namespace std;

namespace hx_net
{
class Electric_message:public base_message
{
public:
    Electric_message(session_ptr pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo);
    ~Electric_message(void);
public:
    void SetProtocol(int mainprotocol,int subprotocol);
    int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    bool IsStandardCommand();
    bool isRegister();
    bool is_auto_run();
    int  start();
    int  stop();
    void getRegisterCommand(CommandUnit &cmdUnit);
    void start_test_send_timer(unsigned long nSeconds=50);
    void stop_test_send_timer();
	void GetAllCmd(CommandAttribute &cmdAll);
    void GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit);
    void GetSignalCommand(int nChannel, CommandUnit &cmdUnit);

    void exec_general_task(int icmdType, string sUser, devCommdMsgPtr lpParam, e_ErrorCode &eErrCode);

    void exec_task_now(int icmdType, string sUser, e_ErrorCode &eErrCode,map<int,string> &mapParam,
                       bool bSnmp, Snmp *snmp, CTarget *target);
    //执行联动命令
    void exec_action_task_now(map<int,vector<ActionParam> > &param,int actionType,string sUser,e_ErrorCode &eErrCode);
protected:
    void test104data();
    void  GetResultData(DevMonitorDataPtr data_ptr);
    int parse_104_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    int iecsock_iframe_recv(struct iec_buf *buf,DevMonitorDataPtr data_ptr);
    int iecsock_sframe_recv(struct iechdr *h);
    int iecsock_uframe_recv(struct iechdr *h);
    void iecsock_uframe_send(enum uframe_func func);
    void iecsock_sframe_send();
    void iecsock_iframe_send();
    void test_time_event(const boost::system::error_code& error);
    int decode_Eda9033A(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    int decode_SPM33(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    int decode_KSTUPS(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    int decode_EM400(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    int decode_EA66(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode);
    int decode_AcrNetEm(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode);
    int decode_Elecctr(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode);
    int decode_Acr_PZ(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode);
    int decode_Acr_Ard2L(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode);
    int decode_ST_C6_20Ks(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode);
    int decode_DSE_7320(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode);
private:
    int              m_Subprotocol;//设备子协议号
    int              m_mainprotocol;//主协议编号
    dev_session_ptr  m_pSession;//关联连接对象

    int              m_DevAddr;//设备地址

    boost::asio::deadline_timer     d_test_send_timer_;//连接定时器
    boost::asio::deadline_timer     d_interrogation_send_timer_;//总召唤定时发送定时器
    int      m_test_send_count;//测试帧发送计数
    unsigned char		d_stopdt;	/* monitor direction 0=active 1=inactive */
    unsigned char		d_testfr;	/* test function 1=active 0=inactive */
    unsigned short      d_num_r;//接收序列号
    unsigned short      d_num_s;//发送序列号

    DeviceInfo          &d_devInfo;//设备信息

	int m_Uo,m_Io;
	float m_Ubb,m_Ibb;
    DevMonitorDataPtr d_curData_ptr;
};
}
