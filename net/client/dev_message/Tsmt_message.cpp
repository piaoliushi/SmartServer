#include "Tsmt_message.h"

namespace hx_net
{

    Tsmt_message::Tsmt_message(net_session *pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo)
		:m_pSession(pSession)
        ,d_devInfo(devInfo)
        ,task_timeout_timer_(io_service)
	{
	}

    Tsmt_message::~Tsmt_message(void)
	{
	}

    void Tsmt_message::SetProtocol(int mainprotocol,int subprotocol)
    {
        m_Subprotocol =   subprotocol;
        m_mainprotocol =  mainprotocol;
    }

    int Tsmt_message::check_msg_header(unsigned char *data,int nDataLen)
	{
		return -1;
	}
    int Tsmt_message::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		return -1;
	}

    bool Tsmt_message::IsStandardCommand()
	{

		return false;
	}
	
    void Tsmt_message::GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit)
	{
		
	}

    int   Tsmt_message::cur_dev_state()
    {
            return -1;
    }

    void Tsmt_message::exec_task_now(int icmdType,int nResult)
    {

    }


    //启动任务定时器
    void Tsmt_message::start_task_timeout_timer()
    {
        task_timeout_timer_.expires_from_now(boost::posix_time::seconds(1));
        task_timeout_timer_.async_wait(boost::bind(&Tsmt_message::schedules_task_time_out,
            this,boost::asio::placeholders::error));
    }

    //任务超时回调
    void Tsmt_message::schedules_task_time_out(const boost::system::error_code& error)
    {

    }
}
