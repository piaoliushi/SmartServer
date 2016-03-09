#pragma once
#include <map>
#include "device_message.h"
#include "../share_ptr_object_define.h"
#include "include.h"
using namespace std;

namespace net
{
	class session;
	class MsgHandleAgentImpl;
	class MsgHandleAgent
	{
	public:
		MsgHandleAgent(session *conPtr,boost::asio::io_service& io_service);
		~MsgHandleAgent(void);
	public:
		bool Init(int protocol,int Subprotocol,int DevCode,map<int,double> &itemRatio);
		bool Finit();
		int start();
		int stop();
		bool is_auto_run();
		int  check_msg_header(unsigned char *data,int nDataLen);
		int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
		void input_params(const vector<string> &vParam);
        //HDevAgentPtr DevAgent();
		int  PreHandleMsg();
		bool IsStandardCommand();
		void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
		int  getChannelCount();
		bool isBelongChannel(int nChnnel,int monitorItemId);
		bool isMonitorChannel(int nChnnel,DevMonitorDataPtr curDataPtr);
		bool ItemValueIsAlarm(DevMonitorDataPtr curDataPtr,int monitorItemId,dev_alarm_state &curState);

		bool isRegister();
		void getRegisterCommand(CommandUnit &cmdUnit);
	private:
		MsgHandleAgentImpl *m_msgImpl;
	};
	typedef boost::shared_ptr<MsgHandleAgent>        HMsgHandlePtr;
}
