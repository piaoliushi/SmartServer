#pragma once
#include "base_message.h"
#include "./104/iec104.h"
//using namespace std;

namespace hx_net
{
class Electric_message:public base_message
{
public:
    Electric_message(session_ptr pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo);
    ~Electric_message(void);
public:
    void SetProtocol(int mainprotocol,int subprotocol);
    int    check_msg_header(unsigned char *data,int nDataLen);
    int    decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
    bool IsStandardCommand();
    bool isRegister();
    bool is_auto_run(){return true;}
    int start();
    int stop();
    void getRegisterCommand(CommandUnit &cmdUnit);
    void start_test_send_timer(unsigned long nSeconds=50);
    void stop_test_send_timer();
	void GetAllCmd(CommandAttribute &cmdAll);
protected:
    void test104data();
    int parse_104_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
    int iecsock_iframe_recv(struct iec_buf *buf,DevMonitorDataPtr data_ptr);
    int iecsock_sframe_recv(struct iechdr *h);
    int iecsock_uframe_recv(struct iechdr *h);
    void iecsock_uframe_send(enum uframe_func func);
    void iecsock_sframe_send();
    void iecsock_iframe_send();
    void test_time_event(const boost::system::error_code& error);
	int decode_Eda9033A(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
private:
    int              m_Subprotocol;//设备子协议号
    int                          m_mainprotocol;//主协议编号
    session_ptr  m_pSession;//关联连接对象
    int      m_DevAddr;//设备地址
    bool  m_Register;
    boost::asio::deadline_timer     d_test_send_timer_;//连接定时器
    boost::asio::deadline_timer     d_interrogation_send_timer_;//总召唤定时发送定时器
    int      m_test_send_count;//测试帧发送计数
    unsigned char		d_stopdt;	/* monitor direction 0=active 1=inactive */
    unsigned char		d_testfr;	/* test function 1=active 0=inactive */
    unsigned short     d_num_r;//接收序列号
    unsigned short     d_num_s;//发送序列号

    DeviceInfo           &d_devInfo;//设备信息

	int m_Uo,m_Io;
	float m_Ubb,m_Ibb;
};
}
