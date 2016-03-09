#pragma once
#include "../../../DataType.h"
#include "base_message.h"
using namespace std;

namespace net
{
	class Eda_message:public base_message
	{
	public:
		Eda_message(session *pSession=NULL);
		~Eda_message(void);
	public:
		void SetSubPro(int subprotocol);
		int  check_msg_header(unsigned char *data,int nDataLen);
		int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
		bool IsStandardCommand();
		void SetAddr(int addr){m_DevAddr = addr;};
		void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
	protected:
		//int  parse_9033_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen);
	private:
		SgSubProtocol m_Subprotocol;//设备子协议号
		session *m_pSession;//关联连接对象
		int      m_DevAddr;//设备地址
	};
}
