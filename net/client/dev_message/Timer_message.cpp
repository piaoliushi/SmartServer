#include "Timer_message.h"
#include"../../../utility.h"
namespace hx_net
{
	Timer_message::Timer_message(session_ptr pSession,DeviceInfo &devInfo)
		:base_message()
		,m_pSession(pSession)
		,d_devInfo(devInfo)
	{

	}

	Timer_message::~Timer_message()
	{

	}

    int Timer_message::check_msg_header( unsigned char *data,int nDataLen,CmdType cmdType,int number )
	{
		switch(d_devInfo.nSubProtocol)
		{
		case TFS_001:
			{
				if(data[0]==0x47 && data[1]==0x42)
				{
					return 0;
				}
				else
				{
					unsigned char cDes[2]={0};
					cDes[0]=0x47;
					cDes[1] = 0x42;
					return kmp(data,nDataLen,cDes,2);
				}
			}
			break;
		}
		return -1;
	}

    int Timer_message::decode_msg_body( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode )
	{
		switch(d_devInfo.nSubProtocol)
		{
		case TFS_001:
            return TFS_001Data(data,data_ptr,nDataLen,iaddcode);
			break;
		}
		return -1;
	}

	bool Timer_message::IsStandardCommand()
	{
		switch(d_devInfo.nSubProtocol)
		{
		case TFS_001:
			return false;
		}
		return false;
	}

	void Timer_message::GetSignalCommand( devCommdMsgPtr lpParam,CommandUnit &cmdUnit )
	{

	}

	void Timer_message::GetAllCmd( CommandAttribute &cmdAll )
	{
		switch(d_devInfo.nSubProtocol)
		{
		case TFS_001:
			{
				CommandUnit tmUnit;
				tmUnit.ackLen = 11;
				tmUnit.commandLen = 0;
				cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
			}
			break;
		}
	}

    int Timer_message::TFS_001Data( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode )
	{
        iaddcode = d_devInfo.iAddressCode;
		int classid = data[2];
		int index = 0,loopnum=6;
		if(classid==0x01)
		{
			index = 6;
			loopnum = 7;
		}
		if(nDataLen<(loopnum+4))
			return -1;
		DataInfo dainfo;
		dainfo.bType = false;
		for(int i=0;i<loopnum;++i)
		{
			dainfo.fValue = data[i+3];
			data_ptr->mValues[index++] = dainfo;
		}
		return 0;
	}
}

