#ifndef CDTRANSMMITER_H
#define CDTRANSMMITER_H
#include "Transmmiter.h"
namespace hx_net{
	class CDtransmmiter:public Transmmiter
	{
	public:
		CDtransmmiter(int subprotocol,int addresscode);
		~CDtransmmiter();

        int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
		int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
		bool IsStandardCommand();
		void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
		void GetAllCmd(CommandAttribute &cmdAll);
	private:
		int OnXG_247Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        int OnKT_digData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        int OnCG_dig1KwData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        void amp_data(unsigned char *data,DevMonitorDataPtr data_ptr,int ampnum);

        int KaiTengData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        int KT_1Kw_813_Data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
        void amp_813_data(unsigned char *data,DevMonitorDataPtr data_ptr,int ampnum);
        int Kt_50WAmp(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate);
	private:
		int m_subprotocol;
		int m_addresscode;
	};
}
#endif // CDTRANSMMITER_H
