#pragma once
#include "base_message.h"
using namespace std;
namespace hx_net
{
	class Timer_message:public base_message
	{
	public:
		Timer_message(session_ptr pSession,DeviceInfo &devInfo);
		~Timer_message();
	public:
        int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
		int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
		bool IsStandardCommand();
		void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
		void GetAllCmd(CommandAttribute &cmdAll);
	protected:
		int TFS_001Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
	private:
		session_ptr m_pSession;//关联连接对象
		DeviceInfo           &d_devInfo;//设备信息
		
	};
}

