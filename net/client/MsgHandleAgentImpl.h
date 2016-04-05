#pragma once
#include "MsgHandleAgentImpl.h"
//#include "othdev_session.h"
#include "./dev_message/base_message.h"

namespace hx_net
{
	class MsgHandleAgentImpl
	{
	public:
        MsgHandleAgentImpl(net_session *pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo);
		~MsgHandleAgentImpl(void);
	public:
        bool Init();
		bool Finit();
		int start();
		int stop();
		bool is_auto_run();
		int PreHandleMsg();
		int  check_msg_header(unsigned char *data,int nDataLen);
		int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
		void input_params(const vector<string> &vParam);
		bool IsStandardCommand();
		void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
		int getChannelCount();
		bool isBelongChannel(int nChnnel,int monitorItemId);
		bool isMonitorChannel(int nChnnel,DevMonitorDataPtr curDataPtr);
		bool ItemValueIsAlarm(DevMonitorDataPtr curDataPtr,int monitorItemId,dev_alarm_state &curState);

		bool isRegister();
		void getRegisterCommand(CommandUnit &cmdUnit);
        int  cur_dev_state();
        void exec_task_now(int icmdType,int nResult);
	public:
		base_message *m_pbaseMsg;
        net_session      *m_pSessionPtr;
		boost::asio::io_service    &m_io_service;
        DeviceInfo     &m_devInfo;
	};
}
