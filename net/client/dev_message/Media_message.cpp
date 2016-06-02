#include "Media_message.h"
#include"../../../utility.h"
namespace hx_net
{

    Media_message::Media_message(session_ptr pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo)
		:m_pSession(pSession)
        ,d_devInfo(devInfo)
        ,task_timeout_timer_(io_service)
	{
	}

    Media_message::~Media_message(void)
	{
	}

    int Media_message::check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number)
	{
		return RE_UNKNOWDEV;
	}

    int Media_message::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		return RE_UNKNOWDEV;
	}

    int  Media_message::parse_AC103CTR_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
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


    bool Media_message::IsStandardCommand()
	{
        switch (d_devInfo.nSubProtocol)
		{
		case WS2032_A:
			break;
		case AC_103_CTR:
		case FRT_X06A:
			return true;
		}
		return false;
	}
	
    void Media_message::GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit)
	{
	}

    void Media_message::GetAllCmd( CommandAttribute &cmdAll )
	{
    }
}
