#include "MsgHandleAgent.h"
#include "MsgHandleAgentImpl.h"
namespace hx_net
{
    MsgHandleAgent::MsgHandleAgent(net_session *conPtr,boost::asio::io_service& io_service,DeviceInfo &devInfo)
        :m_msgImpl( new MsgHandleAgentImpl(conPtr,io_service,devInfo))
	{
	}

	MsgHandleAgent::~MsgHandleAgent(void)
	{
		if(m_msgImpl!=NULL)
		{
			m_msgImpl->Finit();
			delete m_msgImpl;
			m_msgImpl = NULL;
		}
	}

    bool MsgHandleAgent::Init()
	{
		if(m_msgImpl==NULL)
			return false;
        return m_msgImpl->Init();
	}

	bool MsgHandleAgent::Finit()
	{
		if(m_msgImpl==NULL)
			return true;
		m_msgImpl->Finit();
		delete m_msgImpl;
		m_msgImpl = NULL;
		return true;
	}

	int MsgHandleAgent::start()
	{
		if(m_msgImpl==NULL)
			return -1;
		return m_msgImpl->start();
	}
	int MsgHandleAgent::stop()
	{
		if(m_msgImpl==NULL)
			return -1;
		return m_msgImpl->stop();
	}

	bool MsgHandleAgent::is_auto_run()
	{
		if(m_msgImpl==NULL)
			return false;
		return m_msgImpl->is_auto_run();
	}
	int  MsgHandleAgent::check_msg_header(unsigned char *data,int nDataLen)
	{
		if(m_msgImpl==NULL)
			return -1;
		return m_msgImpl->check_msg_header(data,nDataLen);
	}
	int  MsgHandleAgent::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		if(m_msgImpl==NULL)
			return -1;
		return m_msgImpl->decode_msg_body(data,data_ptr,nDataLen);
	}

    /*HDevAgentPtr MsgHandleAgent::DevAgent()
	{
		if(m_msgImpl==NULL)
			return HDevAgentPtr();
		return m_msgImpl->DevAgent();
    }*/

	void  MsgHandleAgent::input_params(const vector<string> &vParam)
	{
		if(m_msgImpl==NULL)
			return ;
		return m_msgImpl->input_params(vParam);
	}

	int  MsgHandleAgent::PreHandleMsg()
	{
		if(m_msgImpl==NULL)
			return -1;
		return m_msgImpl->PreHandleMsg();
	}

	bool MsgHandleAgent::IsStandardCommand()
	{
		if(m_msgImpl==NULL)
			return false;
		return m_msgImpl->IsStandardCommand();
	}

	void MsgHandleAgent::GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit)
	{
		if(m_msgImpl==NULL)
			return;
		return m_msgImpl->GetSignalCommand(lpParam,cmdUnit);
	}

	int MsgHandleAgent::getChannelCount()
	{
		if(m_msgImpl==NULL)
			return -1;
		return m_msgImpl->getChannelCount();
	}

	bool MsgHandleAgent::isBelongChannel(int nChnnel,int monitorItemId)
	{
		if(m_msgImpl==NULL)
			return false;
		return m_msgImpl->isBelongChannel(nChnnel,monitorItemId);
	}

	bool MsgHandleAgent::ItemValueIsAlarm(DevMonitorDataPtr curDataPtr,int monitorItemId,dev_alarm_state &curState)
	{
		if(m_msgImpl==NULL)
			return false;
		return m_msgImpl->ItemValueIsAlarm(curDataPtr,monitorItemId,curState);
	}

	bool MsgHandleAgent::isMonitorChannel(int nChnnel,DevMonitorDataPtr curDataPtr)
	{
		if(m_msgImpl==NULL)
			return false;
		return m_msgImpl->isMonitorChannel(nChnnel,curDataPtr);
	}

	bool MsgHandleAgent::isRegister()
	{
		if(m_msgImpl==NULL)
			return false;
		return m_msgImpl->isRegister();
	}
	void MsgHandleAgent::getRegisterCommand(CommandUnit &cmdUnit)
	{
		if(m_msgImpl==NULL)
			return ;
		return m_msgImpl->getRegisterCommand(cmdUnit);
	}

    int  MsgHandleAgent::cur_dev_state()
    {
        if(m_msgImpl==NULL)
            return -1;
        return m_msgImpl->cur_dev_state();
    }

    void MsgHandleAgent::exec_task_now(int icmdType,int nResult)
    {
        if(m_msgImpl==NULL)
            return ;
        return m_msgImpl->exec_task_now(icmdType,nResult);
    }
}
