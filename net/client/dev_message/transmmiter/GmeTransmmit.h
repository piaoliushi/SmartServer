#pragma once
#include "Transmmiter.h"
namespace hx_net{
	class GmeTransmmit:public Transmmiter
	{
	public:
		GmeTransmmit(int subprotocol,int addresscode);
		~GmeTransmmit();
		int  check_msg_header(unsigned char *data,int nDataLen);
		int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
		bool IsStandardCommand();
		void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
		void GetAllCmd(CommandAttribute &cmdAll);
	private:
		int GmeFMData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
		int GmeSIMData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
	private:
		int m_subprotocol;
		int m_addresscode;
	};
}