#pragma once
#include "MsgHandleAgentImpl.h"
//#include "othdev_session.h"
#include "./dev_message/base_message.h"

namespace net
{
	class MsgHandleAgentImpl
	{
	public:
		MsgHandleAgentImpl(session *pSession,boost::asio::io_service& io_service);
		~MsgHandleAgentImpl(void);
	public:
		bool Init(Protocol protocol,int Subprotocol,int DevCode,map<int,double> &itemRatio);
		bool Finit();
		int start();
		int stop();
		bool is_auto_run();
		int PreHandleMsg();
		int  check_msg_header(unsigned char *data,int nDataLen);
		int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
		void input_params(const vector<string> &vParam);
        //HDevAgentPtr DevAgent();
		bool IsStandardCommand();
		void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
		int getChannelCount();
		bool isBelongChannel(int nChnnel,int monitorItemId);
		bool isMonitorChannel(int nChnnel,DevMonitorDataPtr curDataPtr);
		bool ItemValueIsAlarm(DevMonitorDataPtr curDataPtr,int monitorItemId,dev_alarm_state &curState);

		bool isRegister();
		void getRegisterCommand(CommandUnit &cmdUnit);
	public:
		Protocol      m_CurProtocol;
		base_message *m_pbaseMsg;
		session      *m_pSessionPtr;
		boost::asio::io_service    &m_io_service;
	};
}
