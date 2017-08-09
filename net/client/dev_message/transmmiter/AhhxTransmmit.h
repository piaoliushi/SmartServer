#pragma once
#include "Transmmiter.h"
namespace hx_net{
    class AhhxTransmmit:public Transmmiter
    {
	public:
        AhhxTransmmit(int subprotocol,int addresscode);
        ~AhhxTransmmit();
        int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
		int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
		bool IsStandardCommand();
		void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
		void GetAllCmd(CommandAttribute &cmdAll);
	private:
        int Md993Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        int Md996Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
	private:
		int m_subprotocol;
		int m_addresscode;
	};
}
