#include "LngdTransmmit.h"
#include "../../../../utility.h"
namespace hx_net{
	LngdTransmmit::LngdTransmmit(int subprotocol,int addresscode)
        :Transmmiter()
        ,m_subprotocol(subprotocol)
		,m_addresscode(addresscode)
	{

	}

	LngdTransmmit::~LngdTransmmit()
	{

	}

    int LngdTransmmit::check_msg_header( unsigned char *data,int nDataLen,CmdType cmdType,int number )
	{
		switch(m_subprotocol)
		{
		case LNSZ_169:
			{
				if(data[0]==0xFC && data[nDataLen-1]==0xFD)
					return 0;
				else
					return -1;
			}
		default:
			return -1;
		}
		return -1;
	}

	bool LngdTransmmit::IsStandardCommand()
	{
		switch(m_subprotocol)
		{
		case LNSZ_169:
			return false;
		}
		return false;
	}

	void LngdTransmmit::GetSignalCommand( devCommdMsgPtr lpParam,CommandUnit &cmdUnit )
	{

	}

	int LngdTransmmit::decode_msg_body( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate )
	{
		switch(m_subprotocol)
		{
		case LNSZ_HS:
			break;
		case LNSZ_10_5Kw:
			break;
		case LNSZ_169:
			break;
		}
		return -1;
	}

	void LngdTransmmit::GetAllCmd( CommandAttribute &cmdAll )
	{
		switch(m_subprotocol)
		{
		case LNSZ_HS:
			break;
		case LNSZ_10_5Kw:
			break;
		case LNSZ_169:
			{
				CommandUnit tmUnit;
				tmUnit.ackLen = 5;
				tmUnit.commandLen=7;
				tmUnit.commandId[0] = 0xFC;
				tmUnit.commandId[1] = 0x01;
				tmUnit.commandId[2] = 0x00;
				tmUnit.commandId[3] = 0x00;
				tmUnit.commandId[4] = 0x00;
				tmUnit.commandId[5] = 0x00;
				tmUnit.commandId[6] = 0xFD;
				vector<CommandUnit> vtUnit;
				vtUnit.push_back(tmUnit);
				tmUnit.commandId[2] = 0x01;
				vtUnit.push_back(tmUnit);
				tmUnit.commandId[2] = 0x02;
				vtUnit.push_back(tmUnit);
				tmUnit.commandId[2] = 0x03;
				vtUnit.push_back(tmUnit);
				cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
				
				tmUnit.commandId[1] = 0x02;
				tmUnit.commandId[2] = 0x00;
				tmUnit.ackLen = 0;
				vector<CommandUnit> vtTurnOnUnit;
				vtTurnOnUnit.push_back(tmUnit);
				cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtTurnOnUnit;
			
				tmUnit.commandId[2] = 0x01;
				vector<CommandUnit> vtTurnOffUnit;
				vtTurnOffUnit.push_back(tmUnit);
				cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnOffUnit;
				tmUnit.commandId[2] = 0x02;
				vector<CommandUnit> vtResetUnit;
				vtResetUnit.push_back(tmUnit);
		
				//添加复位命令

				//激励器切换
				tmUnit.commandId[1] = 0x03;
				tmUnit.commandId[2] = 0x20;
				
				tmUnit.commandId[1] = 0x04;
				tmUnit.commandId[2] = 0x01;
				vector<CommandUnit> vtUpUnit;
				vtUpUnit.push_back(tmUnit);
				cmdAll.mapCommand[MSG_TRANSMITTER_RISE_POWER_OPR] = vtUpUnit;
				tmUnit.commandId[2] = 0x02;
				vector<CommandUnit> vtDownUnit;
				vtDownUnit.push_back(tmUnit);
				cmdAll.mapCommand[MSG_TRANSMITTER_REDUCE_POWER_OPR] = vtDownUnit;
			}
			break;
		}
	}
}
