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
                    return RE_SUCCESS;
				else
                    return RE_HEADERROR;
			}
        case LNSZ_HS:
            break;
        case LNSZ_10_5Kw:
            break;
        case LNSZ_68_BD:{
            if(data[0]==m_addresscode)
            {
                return data[2]+2;
            }
            else
            {
                return RE_HEADERROR;
            }
        }
            break;
        default:
            break;
		}
        return RE_NOPROTOCOL;
	}

	bool LngdTransmmit::IsStandardCommand()
	{
		switch(m_subprotocol)
		{
		case LNSZ_169:
			return false;
        case LNSZ_68_BD:
            return true;
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
        case LNSZ_68_BD:
            return Ls68DbData(data,data_ptr,nDataLen,runstate);
		}
        return RE_NOPROTOCOL;
	}

	void LngdTransmmit::GetAllCmd( CommandAttribute &cmdAll )
	{
		switch(m_subprotocol)
		{
        case LNSZ_HS:{
            CommandUnit tmUnit;
            tmUnit.commandLen = 17;
            tmUnit.ackLen = 0;
            tmUnit.commandId[0] = 0xFC;
            tmUnit.commandId[1] = 0x71;
            tmUnit.commandId[2] = 0x3A;
            tmUnit.commandId[3] = 0xD0;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x00;
            tmUnit.commandId[6] = 0x00;
            tmUnit.commandId[7] = 0x00;
            tmUnit.commandId[8] = 0x00;
            tmUnit.commandId[9] = 0x00;
            tmUnit.commandId[10] = 0x00;
            tmUnit.commandId[11] = 0x00;
            tmUnit.commandId[12] = 0xA0;
            tmUnit.commandId[13] = 0x69;
            tmUnit.commandId[14] = 0xCC;
            tmUnit.commandId[15] = 0xFD;
            tmUnit.commandId[16] = 0x00;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[2] = 0x55;
            tmUnit.commandId[3] = 0xA1;
            tmUnit.commandId[12] = 0xCF;
            tmUnit.commandId[13] = 0xB3;
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
            tmUnit.commandId[2] = 0x55;
            tmUnit.commandId[3] = 0xA2;
            tmUnit.commandId[12] = 0xCE;
            tmUnit.commandId[13] = 0xB2;
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
        }
            break;
        case LNSZ_10_5Kw:{
            CommandUnit tmUnit;
            tmUnit.commandLen = 7;
            tmUnit.ackLen = 0;
            tmUnit.commandId[0] = 0x12;
            tmUnit.commandId[1] = 0x01;
            tmUnit.commandId[2] = 0x04;
            tmUnit.commandId[3] = 0x00;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x00;
            tmUnit.commandId[6] = 0x01;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[0] = 0x13;
            tmUnit.commandId[1] = 0x80;
            tmUnit.commandId[2] = 0x04;
            tmUnit.commandId[3] = 0x00;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x00;
            tmUnit.commandId[6] = 0x01;
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
            tmUnit.commandId[6] = 0x02;
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
        }
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
        case LNSZ_68_BD:
        {
            CommandUnit tmUnit;
            tmUnit.commandLen = 8;
            tmUnit.ackLen = 3;
            tmUnit.commandId[0] = m_addresscode;
            tmUnit.commandId[1] = 0x04;
            tmUnit.commandId[2] = 0x00;
            tmUnit.commandId[3] = 0x00;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x08;
            unsigned short uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            //tmUnit.commandId[1] = 0x01;
            //uscrc = CRC16_A001(tmUnit.commandId,6);
           // tmUnit.commandId[6] = (uscrc&0x00FF);
            //tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            //cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        }
    }

    int LngdTransmmit::Ls68DbData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        DataInfo dtinfo;
        int cmdtype = data[1];
        if(cmdtype==0x04)
        {
            dtinfo.bType = false;
            dtinfo.fValue = data[3]*256+data[4];
            data_ptr->mValues[0] = dtinfo;
            dtinfo.fValue = data[5]*256+data[6];
            data_ptr->mValues[1] = dtinfo;
            if(data_ptr->mValues[0].fValue>data_ptr->mValues[1].fValue)
                dtinfo.fValue = sqrt((data_ptr->mValues[0].fValue+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue-data_ptr->mValues[1].fValue));
            else
                dtinfo.fValue = 0;
            data_ptr->mValues[2] = dtinfo;
            for(int i=0;i<6;++i)
            {
                dtinfo.fValue = data[7+2*i]*256+data[8+2*i];
                data_ptr->mValues[3+i] = dtinfo;
            }
        }
        else if(cmdtype==0x01)
        {
            dtinfo.bType = true;
            for(int i=0;i<8;++i)
            {
                dtinfo.fValue = Getbit(data[3],i);
                data_ptr->mValues[9+i] = dtinfo;
            }
        }
        else
        {
            return RE_CMDACK;
        }

        if(m_addresscode == 5){

            data_ptr->mValues[0].fValue = data_ptr->mValues[3].fValue;
            data_ptr->mValues[1].fValue = data_ptr->mValues[4].fValue;
            data_ptr->mValues[2].fValue=1.0f;

        }
        return RE_SUCCESS;
    }
}
