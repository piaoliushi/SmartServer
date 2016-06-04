#include "MsgHandleAgent.h"
#include "MsgHandleAgentImpl.h"
#include "snmp_pp.h"
#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif
namespace hx_net
{
    MsgHandleAgent::MsgHandleAgent(session_ptr conPtr,boost::asio::io_service& io_service,DeviceInfo &devInfo)
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

    bool MsgHandleAgent::Init(pDevicePropertyExPtr devProperty)
	{
		if(m_msgImpl==NULL)
			return false;
        return m_msgImpl->Init(devProperty);
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
    int  MsgHandleAgent::check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number)
	{
		if(m_msgImpl==NULL)
			return -1;
        return m_msgImpl->check_msg_header(data,nDataLen,cmdType,number);
	}
	int  MsgHandleAgent::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		if(m_msgImpl==NULL)
			return -1;
		return m_msgImpl->decode_msg_body(data,data_ptr,nDataLen);
	}

    int  MsgHandleAgent::decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target)
    {
        if(m_msgImpl==NULL)
            return -1;
        return m_msgImpl->decode_msg_body(snmp,data_ptr,target);
    }

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

    void MsgHandleAgent::GetAllCmd(CommandAttribute &cmdAll)
    {
        if(m_msgImpl==NULL)
            return;
        return m_msgImpl->GetAllCmd(cmdAll);
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

    void MsgHandleAgent::exec_task_now(int icmdType,string sUser,e_ErrorCode &eErrCode)
    {
        if(m_msgImpl==NULL)
            return ;
        return m_msgImpl->exec_task_now(icmdType,sUser,eErrCode);
    }

    void MsgHandleAgent::start_task_timeout_timer()
    {
        if(m_msgImpl==NULL)
            return ;
        return m_msgImpl->start_task_timeout_timer();
    }

    //获得运行状态
    int MsgHandleAgent::get_run_state()
    {
        if(m_msgImpl==NULL)
            return dev_unknown;
        return m_msgImpl->get_run_state();
    }

     void MsgHandleAgent::reset_run_state()
     {
         if(m_msgImpl==NULL)
             return;
         return m_msgImpl->reset_run_state();
     }

     //添加新告警
     bool  MsgHandleAgent::add_new_alarm(string sPrgName,int alarmId,int nState,time_t  startTime)
     {
         if(m_msgImpl==NULL)
             return false;
         return m_msgImpl->add_new_alarm(sPrgName,alarmId,nState, startTime);
     }
     bool  MsgHandleAgent::add_new_data(string sIp,DevMonitorDataPtr &mapData)
     {
         if(m_msgImpl==NULL)
             return false;
         return m_msgImpl->add_new_data(sIp,mapData);
     }
}
