#pragma once
#include "../device_session.h"
#include "../device_message.h"
using namespace std;

namespace net
{
	#define  Getbit(x,n)  ((x>>n) & 0x01)
	class base_message
	{
	public:
		base_message(void);
		virtual ~base_message(void);
	public:
		virtual void SetSubPro(int subprotocol){};
		virtual int check_msg_header(unsigned char *data,int nDataLen){return -1;};
		virtual int decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen){return -1;};
		virtual int PreHandleMsg(){return -1;}
		virtual void input_params(const vector<string> &vParam){};
		virtual bool IsStandardCommand(){return false;}
		virtual void SetAddr(int addr){};
		virtual void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit){};
		virtual bool isRegister(){return true;}
		virtual void getRegisterCommand(CommandUnit &cmdUnit){};
		virtual int start(){return -1;}
		virtual int stop(){return -1;}
		virtual	bool is_auto_run(){return false;}

		virtual int  getChannelCount(){return 0;}
		virtual bool isBelongChannel(int nChnnel,int monitorItemId){return false;}
		virtual bool isMonitorChannel(int nChnnel,DevMonitorDataPtr curDataPtr){return true;}
		virtual bool ItemValueIsAlarm(DevMonitorDataPtr curDataPtr,int monitorItemId,dev_alarm_state &curState){return false;}
        //HDevAgentPtr Agent(){return dev_agent_;}
		unsigned long CRC16_A001( unsigned char * ptr, int len );
	public:
        static void Char2Hex(unsigned char ch, char* szHex);
		static void CharStr2HexStr(unsigned char const* pucCharStr, string &pszHexStr, int iSize);
	private:
        //HDevAgentPtr       dev_agent_;//协议解析和数据验证对象

	};
}
