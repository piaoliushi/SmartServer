#include "Envir_message.h"

namespace hx_net
{

    Envir_message::Envir_message(net_session *pSession,DeviceInfo &devInfo)
		:m_pSession(pSession)
        ,d_devInfo(devInfo)
	{
	}

	Envir_message::~Envir_message(void)
	{
	}

    void Envir_message::SetProtocol(int mainprotocol,int subprotocol)
	{
        m_Subprotocol = subprotocol;
        m_mainprotocol =  mainprotocol;
	}

	int Envir_message::check_msg_header(unsigned char *data,int nDataLen)
	{
		switch (m_Subprotocol)
		{
		case WS2032_A:
			break;
		case AC_103_CTR:
			{
                if(data[0]!=d_devInfo.iAddressCode)
					return -1;
				else
					return (data[5]+2);
			}
			break;
		case FRT_X06A:
			{
				if(data[0]!=0x01 || data[8]!=0x02)
					return -1;
				return int(((data[6]<<8)|data[7])+4);
			}
			break;
		}
		return -1;
	}
	int Envir_message::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		switch (m_Subprotocol)
		{
		case WS2032_A:
			break;
		case AC_103_CTR:
			return parse_AC103CTR_data(data,data_ptr,nDataLen);
		case FRT_X06A:
            return -1;//Agent()->HxPaseData(data_ptr.get(),(LPBYTE)(data)+9,nDataLen-9);
		}
		return -1;
	}

	int  Envir_message::parse_AC103CTR_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		if(nDataLen<6)
			return -1;
		switch(data[1])
		{
		case 0x01://设备整机状态回复
			{
				return 0;
			}
			break;
		case 0x8b:
			return 0;
		}
		return -1;
	}


	bool Envir_message::IsStandardCommand()
	{
		switch (m_Subprotocol)
		{
		case WS2032_A:
			break;
		case AC_103_CTR:
		case FRT_X06A:
			return true;
		}
		return false;
	}
	
	void Envir_message::GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit)
	{
	}
}
