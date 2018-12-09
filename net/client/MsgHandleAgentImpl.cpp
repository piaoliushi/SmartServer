#include "MsgHandleAgentImpl.h"
#include "./dev_message/Envir_message.h"
//#include "./dev_message/Hx_message.h"
#include "./dev_message/Tsmt_message.h"
#include "./dev_message/Electric_message.h"
#include "./dev_message/Timer_message.h"
#include "./dev_message/Link_message.h"
#include "./dev_message/Media_message.h"
#include "./dev_message/antenna_message.h"

namespace hx_net
{
    MsgHandleAgentImpl::MsgHandleAgentImpl(session_ptr pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo)
		:m_pSessionPtr(pSession)
		,m_pbaseMsg(0)
		,m_io_service(io_service)
        ,m_devInfo(devInfo)
	{
	}

	MsgHandleAgentImpl::~MsgHandleAgentImpl(void)
	{
	}

    bool MsgHandleAgentImpl::Init(pDevicePropertyExPtr devProperty)
	{
        switch(m_devInfo.iDevType)
		{
        case DEVICE_TRANSMITTER:{
                m_pbaseMsg = new Tsmt_message(m_pSessionPtr,m_io_service,m_devInfo,devProperty);
             }
            break;
        case DEVICE_TEMP:
        case DEVICE_SMOKE:
        case DEVICE_WATER:
        case DEVICE_AIR:
            m_pbaseMsg = new Envir_message(m_pSessionPtr,m_io_service,m_devInfo);
			break;
        case DEVICE_GPS:
            m_pbaseMsg = new Timer_message(m_pSessionPtr,m_devInfo);
            break;
        case DEVICE_GS_RECIVE://卫星接收机
        case DEVICE_MW://微波接收机
        case DEVICE_TR://光收发器
        case DEVICE_ENCODER://编码器
        case DEVICE_MUX://复用器
        case DEVICE_MO://调制器
        case DEVICE_SWITCH://切换器
        case DEVICE_ADAPTER://适配器
            m_pbaseMsg = new Link_message(m_pSessionPtr,m_devInfo);
            break;
        case DEVICE_MEDIA:	{
                m_pbaseMsg = new Media_message(m_pSessionPtr,m_io_service,m_devInfo,devProperty);
			}
			break;
        case DEVICE_UPS:
        case DEVICE_ELEC:{
                m_pbaseMsg = new Electric_message(m_pSessionPtr,m_io_service,m_devInfo);
            } break;

        case DEVICE_ANTENNA:{
                m_pbaseMsg = new Antenna_message(m_pSessionPtr,m_io_service,m_devInfo);
        } break;
		default:
			return false;
		}
		return true;
	}

	bool MsgHandleAgentImpl::Finit()
	{
		if(m_pbaseMsg != NULL)
		{
			delete m_pbaseMsg;
			m_pbaseMsg=NULL;
		}
		return true;
	}

	int MsgHandleAgentImpl::start()
	{
		if(m_pbaseMsg==NULL)
			return -1;
		return m_pbaseMsg->start();
	}
	int MsgHandleAgentImpl::stop()
	{
		if(m_pbaseMsg==NULL)
			return -1;
		return m_pbaseMsg->stop();
	}

	bool MsgHandleAgentImpl::is_auto_run()
	{
		if(m_pbaseMsg==NULL)
			return false;
		return m_pbaseMsg->is_auto_run();
	}

    void MsgHandleAgentImpl::GetAllCmd(CommandAttribute &cmdAll)
    {
        if(m_pbaseMsg==NULL)
            return ;
        return m_pbaseMsg->GetAllCmd(cmdAll);
    }

    int  MsgHandleAgentImpl::check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number)
	{
		if(m_pbaseMsg==NULL)
			return -1;
        return m_pbaseMsg->check_msg_header(data,nDataLen,cmdType,number);
	}
    int  MsgHandleAgentImpl::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode)
	{
		if(m_pbaseMsg==NULL)
			return -1;
        return m_pbaseMsg->decode_msg_body(data,data_ptr,nDataLen,iaddcode);
	}

    int  MsgHandleAgentImpl::decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target)
    {
        if(m_pbaseMsg==NULL)
            return -1;
        return m_pbaseMsg->decode_msg_body(snmp,data_ptr,target);
    }

    //http消息解析
    int  MsgHandleAgentImpl::decode_http_msg(const string &data,DevMonitorDataPtr data_ptr,CmdType cmdType,int number)
    {
        if(m_pbaseMsg==NULL)
            return -1;
        return m_pbaseMsg->decode_http_msg(data,data_ptr,cmdType,number);
    }

    /*HDevAgentPtr MsgHandleAgentImpl::DevAgent()
	{
		if(m_pbaseMsg==NULL)
			return HDevAgentPtr();
		return m_pbaseMsg->Agent();
    }*/

	void MsgHandleAgentImpl::input_params(const vector<string> &vParam)
	{
		if(m_pbaseMsg==NULL)
			return;
		return m_pbaseMsg->input_params(vParam);
	}

	int MsgHandleAgentImpl::PreHandleMsg()
	{
		if(m_pbaseMsg==NULL)
			return -1;
		return m_pbaseMsg->PreHandleMsg();
	}

	bool MsgHandleAgentImpl::IsStandardCommand()
	{
		if(m_pbaseMsg==NULL)
			return false;
		return m_pbaseMsg->IsStandardCommand();
	}

	void MsgHandleAgentImpl::GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit)
	{
		if(m_pbaseMsg==NULL)
			return;
		return m_pbaseMsg->GetSignalCommand(lpParam,cmdUnit);
	}

    void MsgHandleAgentImpl::GetSignalCommand(int cmmType,int nIndex,CommandUnit &cmdUnit)
    {
        if(m_pbaseMsg==NULL)
            return;
        return m_pbaseMsg->GetSignalCommand(cmmType,nIndex,cmdUnit);
    }

	int MsgHandleAgentImpl::getChannelCount()
	{
		if(m_pbaseMsg==NULL)
			return -1;
		return m_pbaseMsg->getChannelCount();
	}

	bool MsgHandleAgentImpl::isBelongChannel(int nChnnel,int monitorItemId)
	{
		if(m_pbaseMsg==NULL)
			return false;
		return m_pbaseMsg->isBelongChannel(nChnnel,monitorItemId);
	}

	bool MsgHandleAgentImpl::ItemValueIsAlarm(DevMonitorDataPtr curDataPtr,
		                                      int monitorItemId,dev_alarm_state &curState)
	{
		if(m_pbaseMsg==NULL)
			return false;
		return m_pbaseMsg->ItemValueIsAlarm(curDataPtr,monitorItemId,curState);
	}

	bool MsgHandleAgentImpl::isMonitorChannel(int nChnnel,DevMonitorDataPtr curDataPtr)
	{
		if(m_pbaseMsg==NULL)
			return false;
		return m_pbaseMsg->isMonitorChannel(nChnnel,curDataPtr);
	}

	bool MsgHandleAgentImpl::isRegister()
	{
		if(m_pbaseMsg==NULL)
			return false;
		return m_pbaseMsg->isRegister();
	}
	void MsgHandleAgentImpl::getRegisterCommand(CommandUnit &cmdUnit)
	{
		if(m_pbaseMsg==NULL)
			return ;
		return m_pbaseMsg->getRegisterCommand(cmdUnit);
	}

    int  MsgHandleAgentImpl::cur_dev_state()
    {
        if(m_pbaseMsg==NULL)
            return -1;
        return m_pbaseMsg->cur_dev_state();
    }

    void MsgHandleAgentImpl::exec_task_now(int icmdType,string sUser,e_ErrorCode &eErrCode,map<int,string> &mapParam,int nMode,
                                           bool bSnmp,Snmp *snmp,CTarget *target)
    {
        if(m_pbaseMsg==NULL){
            eErrCode = EC_MSG_HANDEL_OBJ_NULL;
            return ;
        }
        return m_pbaseMsg->exec_task_now(icmdType,sUser,eErrCode,mapParam,nMode,bSnmp,snmp,target);
    }

    //执行通用指令
    void MsgHandleAgentImpl::exec_general_task(int icmdType,string sUser,devCommdMsgPtr lpParam,
                                           e_ErrorCode &eErrCode){
        if(m_pbaseMsg==NULL){
            eErrCode = EC_MSG_HANDEL_OBJ_NULL;
            return ;
        }
        return m_pbaseMsg->exec_general_task(icmdType,sUser,lpParam,eErrCode);
    }

    //执行联动命令
    void MsgHandleAgentImpl::exec_action_task_now(map<int,vector<ActionParam> > &param,int actionType,string sUser,e_ErrorCode &eErrCode)
    {
        if(m_pbaseMsg==NULL){
            eErrCode = EC_MSG_HANDEL_OBJ_NULL;
            return ;
        }
        return m_pbaseMsg->exec_action_task_now(param,actionType,sUser,eErrCode);
    }

    void MsgHandleAgentImpl::start_task_timeout_timer()
    {
        if(m_pbaseMsg==NULL)
            return ;
        return m_pbaseMsg->start_task_timeout_timer();
    }

    //获得运行状态
    int MsgHandleAgentImpl::get_run_state()
    {
        if(m_pbaseMsg==NULL)
            return dev_unknown;
        return m_pbaseMsg->get_run_state();
    }

    //设置运行状态
    void MsgHandleAgentImpl::set_run_state(int curState)
    {
        if(m_pbaseMsg==NULL)
            return;
        return m_pbaseMsg->set_run_state(curState);
    }

    void MsgHandleAgentImpl::reset_run_state()
    {
        if(m_pbaseMsg==NULL)
            return;
        return m_pbaseMsg->reset_run_state();
    }

    bool MsgHandleAgentImpl::dev_can_excute_cmd()
    {
        if(m_pbaseMsg==NULL)
            return false;
        return m_pbaseMsg->dev_can_excute_cmd();
    }

    //是否需要清除告警
    bool  MsgHandleAgentImpl::is_need_clear_alarm()
    {
        if(m_pbaseMsg==NULL)
            return false;
        return m_pbaseMsg->is_need_clear_alarm();
    }

    //添加新告警
    bool  MsgHandleAgentImpl::add_new_alarm(string sPrgName,int alarmId,int nState,time_t  startTime)
    {
        if(m_pbaseMsg==NULL)
            return false;
        return m_pbaseMsg->add_new_alarm(sPrgName,alarmId,nState, startTime);
    }

    bool  MsgHandleAgentImpl::add_new_data(string sIp,int nChannel,DevMonitorDataPtr &mapData)
    {
        if(m_pbaseMsg==NULL)
            return false;
        return m_pbaseMsg->add_new_data(sIp,nChannel,mapData);
    }

    //获取关联机信息
    void  MsgHandleAgentImpl::get_relate_dev_info(string &sStationId,string &sDevId,string &sAttenaId)
    {
        if(m_pbaseMsg==NULL)
            return ;
        return m_pbaseMsg->get_relate_dev_info(sStationId,sDevId,sAttenaId);
    }

    //是否是天线代理
    bool MsgHandleAgentImpl::is_anttena_agent()
    {
        if(m_pbaseMsg==NULL)
            return false;
        return m_pbaseMsg->is_anttena_agent();
    }

    //是否是软件一键开机
    bool MsgHandleAgentImpl::is_soft_onekey_open()
    {
        if(m_pbaseMsg==NULL)
            return false;
        return m_pbaseMsg->is_soft_onekey_open();
    }
    //是否是996一键开机
    bool MsgHandleAgentImpl::is_996_onekey_open()
    {
        if(m_pbaseMsg==NULL)
            return false;
        return m_pbaseMsg->is_996_onekey_open();
    }
}

