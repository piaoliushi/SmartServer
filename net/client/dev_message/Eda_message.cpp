#include "Eda_message.h"

namespace net
{

	Eda_message::Eda_message(session *pSession)
		:m_pSession(pSession)
	{
	}

	Eda_message::~Eda_message(void)
	{
	}

	void Eda_message::SetSubPro(int subprotocol)
	{
		m_Subprotocol = (SgSubProtocol)subprotocol;
	}

	int Eda_message::check_msg_header(unsigned char *data,int nDataLen)
	{
		switch (m_Subprotocol)
		{
		case ZXJY_BACK:
			{
				if(data[0]==0x00)
				{
					int nBodyLen = ((data[5]<<8)|(data[4]))*((data[7]<<8)|(data[6]));
					if(nDataLen == nBodyLen+8)
						return 0;
				}else
					return -1;
			}
		}
		return -1;
	}
	int Eda_message::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		switch (m_Subprotocol)
		{
		case ZXJY_BACK:
            return -1;//Agent()->HxPaseData(data_ptr.get(),(LPBYTE)(data),nDataLen);
		}
		return -1;
	}

	bool Eda_message::IsStandardCommand()
	{
		switch (m_Subprotocol)
		{
		case Eda9003_F:
			return false;
		case ZXJY_BACK:
			return true;
		}
		return false;
	}
	
	void Eda_message::GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit)
	{
		
	}
}
