#ifndef SHTRANSMMITER_H
#define SHTRANSMMITER_H
#include "Transmmiter.h"
#include "../../device_session.h"
namespace hx_net{
	class ShTransmmit:public Transmmiter
	{
	public:
        ShTransmmit(dev_session_ptr pSession,int subprotocol,int addresscode);
		~ShTransmmit();

        int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
		int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate);
		bool IsStandardCommand();
		void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
		void GetAllCmd(CommandAttribute &cmdAll);
        bool isLastQueryCmd();
        bool isMultiQueryCmd();
	private:
        int OnAllBandData(unsigned char *data,DevMonitorDataPtr pBandData,int nDataLen,int& runstate);
        void LastBandRunData(unsigned char *data,DevMonitorDataPtr pBandData,int nDataLen,int nmodenum);
        void LastBandSateData(unsigned char *data,DevMonitorDataPtr pBandData,int nDataLen,int nmodenum);
	private:
		int m_subprotocol;
		int m_addresscode;
        //string m_sDevId;
        dev_session_ptr m_pSession;//关联连接对象
        //DevMonitorDataPtr &m_pBandData;
        int m_curMsgType;
        int m_curMsgId;
        int m_maxMsgId;
	};
}
#endif // CDTRANSMMITER_H
