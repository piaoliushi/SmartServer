#ifndef LNGDTRANSMMITER_H
#define LNGDTRANSMMITER_H
#include "Transmmiter.h"
namespace hx_net{
	class LngdTransmmit:public Transmmiter
	{
	public:
        LngdTransmmit(int subprotocol,int addresscode);
        ~LngdTransmmit();

        int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
		int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
		bool IsStandardCommand();
		void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
		void GetAllCmd(CommandAttribute &cmdAll);
	private:
        int Ls68DbData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
	private:
		int m_subprotocol;
		int m_addresscode;
	};
}
#endif // LNGDTRANSMMITER_H
