#include "AhhxTransmmit.h"

namespace hx_net{

    AhhxTransmmit::AhhxTransmmit(int subprotocol,int addresscode)
        :Transmmiter()
		,m_subprotocol(subprotocol)
		,m_addresscode(addresscode)
	{

	}

    AhhxTransmmit::~AhhxTransmmit()
	{

	}

    int AhhxTransmmit::check_msg_header( unsigned char *data,int nDataLen,CmdType cmdType,int number)
	{
		switch(m_subprotocol)
		{
        case HUIXIN_993:
        case HUIXIN_JWT:
        case HUIXIN_HN:
        case HUIXIN_992:
        case HUIXIN_993_EX:{
            if(data[0]==0x55 && data[2]==0xF3)
            {
                if(data[1]< 0x99 )
                    return 0;
            }
            else
            {
                unsigned char cDes[3]={0};
                cDes[0]=0x55;
                cDes[1] = 0x65;
                cDes[2] = 0xF3;
                return kmp(data,nDataLen,cDes,3);
            }
			}
			break;
        case ANHUI_994:
            {
            if(data[0]==0x55 && data[2]==0xF3)
            {
                if(data[1]< 0x99 )
                    return 0;
            }
            else
            {
                unsigned char cDes[3]={0};
                cDes[0]=0x55;
                cDes[1] = 0x35;
                cDes[2] = 0xF3;      
                return kmp(data,nDataLen,cDes,3);
            }
            }
            break;
        case HUIXIN_0804D:
                {
                    if(data[0]==0xAA) //&& pHeader[1]==0x33
                    {
                        return 0;
                    }
                    else
                    {//查找日期+时间头，若找到返回第一个字节位置
                        unsigned char cDes[2]={0};
                        cDes[0]=0xAA;
                        cDes[1]=0x33;
                        return kmp(data,nDataLen,cDes,2);
                    }
                }
                break;
            case HUIXIN_0401_AV:
                {
                    if(data[0]==0xAA && data[1]==0x41)
                        return 0;
                    else
                    {
                        unsigned char cDes[2]={0};
                        cDes[0]=0xAA;
                        cDes[1]=0x41;
                        return kmp(data,nDataLen,cDes,2);
                    }
                }
                break;
            case HUIXIN_0401_SKY:
                {
                    if(data[0]==0xAA && data[1]==0x43)
                        return 0;
                    else
                    {
                        unsigned char cDes[2]={0};
                        cDes[0]=0xAA;
                        cDes[1]=0x43;
                        return kmp(data,nDataLen,cDes,2);
                    }
                }
                break;
            case HUIXIN_0401_DABS:
            case HUIXIN_0401_DA:
                {
                    if(data[0]==0xAA && data[1]==0x45)
                        return 0;
                    else
                    {
                        unsigned char cDes[2]={0};
                        cDes[0]=0xAA;
                        cDes[1]=0x45;
                        return kmp(data,nDataLen,cDes,2);
                    }
                }
                break;
            case HUIXIN_DATA_MGR:
                {
                    if(data[0]!=0x7E || data[1]!=0x30)
                        return -1;
                    else
                    {
                        return int((data[5]<<8)|data[4]);
                    }
                }
                break;
            case HUIXIN_6300:
                if(data[0]!=0xAA || data[1]!=0x63)
                    return -1;
                else
                {
                    return int((data[5]<<8)|data[6]);
                }
                break;
            case HUIXIN_9020:
                if(data[0]!=0xAA || data[1]!=0x92)
                    return -1;
                else
                {
                    return int((data[5]<<8)|data[6]);
                }
                break;
            case HUIXIN_740P:
                if(data[0]!=0x7E || data[1]!=0x74)
                    return -1;
                else
                {
                    return int((data[5]<<8)|data[6]);
                }
                break;
            case HUIXIN_0214:
                {
                    if(data[0]!=0x7E || data[1]!=0x40)
                        return -1;
                    else
                    {
                        return int((data[6]<<8)|data[5]);
                    }
                }
                break;
        }
		return RE_NOPROTOCOL;
	}

    int AhhxTransmmit::decode_msg_body( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate )
	{
		switch(m_subprotocol)
		{
        case HUIXIN_993:
           return Md993Data(data,data_ptr,nDataLen,runstate);
		}
		return -1;
	}

    bool AhhxTransmmit::IsStandardCommand()
	{
		switch(m_subprotocol)
		{
        case HUIXIN_DATA_MGR:
        case HUIXIN_6300:
        case HUIXIN_9020:
        case HUIXIN_740P:
        case HUIXIN_730P:
        case HUIXIN_761:
        case HUIXIN_0214:
            return true;
		}
		return false;
	}

    void AhhxTransmmit::GetSignalCommand( devCommdMsgPtr lpParam,CommandUnit &cmdUnit )
	{

	}

    void AhhxTransmmit::GetAllCmd( CommandAttribute &cmdAll )
	{
		switch(m_subprotocol)
		{
        case HUIXIN_993:
        case ANHUI_994:
        case HUIXIN_JWT:
        case HUIXIN_HN:
        {
            CommandUnit tmUnit;
            tmUnit.commandLen = 5;
            if(m_subprotocol == ANHUI_994)
                tmUnit.ackLen = 56;
            else
                tmUnit.ackLen = 104;
            tmUnit.commandId[0]=0x55;
            tmUnit.commandId[1]=0x02;
            tmUnit.commandId[2]=0xF3;
            tmUnit.commandId[3]=0xF3;
            tmUnit.commandId[4]=0x00;
            vector<CommandUnit> vtUnit;
            vtUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;

            tmUnit.commandLen = 7;
            tmUnit.commandId[0]=0x55;
            tmUnit.commandId[1]=0x03;
            tmUnit.commandId[2]=0xF2;
            tmUnit.commandId[3]=0x00;
            tmUnit.commandId[4]=0x01;
            tmUnit.commandId[5]=0xF3;
            tmUnit.commandId[6]=0x00;
            vector<CommandUnit> vtHTurnOnUnit;
            vtHTurnOnUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtHTurnOnUnit;
            tmUnit.commandId[3]=0x02;
            tmUnit.commandId[5]=0xF1;
            vector<CommandUnit> vtMTurnOnUnit;
            vtMTurnOnUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_TRANSMITTER_MIDDLE_POWER_TURNON_OPR] = vtMTurnOnUnit;
            tmUnit.commandId[3]=0x03;
            tmUnit.commandId[5]=0xF0;
            vector<CommandUnit> vtLTurnOnUnit;
            vtLTurnOnUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_TRANSMITTER_LOW_POWER_TURNON_OPR] = vtLTurnOnUnit;
            tmUnit.commandId[3]=0x01;
            tmUnit.commandId[5]=0xF2;
            vector<CommandUnit> vtTurnOffUnit;
            vtTurnOffUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnOffUnit;

            tmUnit.commandId[3]=0x04;
            tmUnit.commandId[5]=0xF7;
            vector<CommandUnit> vtUpUnit;
            vtUpUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_TRANSMITTER_RISE_POWER_OPR] = vtUpUnit;
            tmUnit.commandId[3]=0x05;
            tmUnit.commandId[5]=0xF6;
            vector<CommandUnit> vtDwUnit;
            vtDwUnit.push_back(tmUnit);
             cmdAll.mapCommand[MSG_TRANSMITTER_REDUCE_POWER_OPR] = vtDwUnit;
        }
            break;
        }
    }
    int AhhxTransmmit::Md993Data(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        float fdat=0;//
        float fAnalogData[25];
        for(int i=0;i<24;++i)
        {
            *(((char*)(&fdat) + 0)) = data[3+4*i];
            *(((char*)(&fdat) + 1)) = data[4+4*i];
            *(((char*)(&fdat) + 2)) = data[5+4*i];
            *(((char*)(&fdat) + 3)) = data[6+4*i];
            if(i<2)
            {
                fAnalogData[i] = fdat;
            }
            else
                fAnalogData[i+1] = fdat;
        }

        fAnalogData[2]  = 	0;

        DataInfo dtinfo;
        dtinfo.bType = false;
        for(int index=0;index<25;++index)
        {
            dtinfo.fValue = fAnalogData[index];
            data_ptr->mValues[index] = dtinfo;
        }
        dtinfo.bType = true;
        for(int j=0;j<3;++j)
        {
            for(int i=0;i<8;++i)
            {
                dtinfo.fValue = Getbit(data[99+j],i);
                data_ptr->mValues[25+j*8+i] = dtinfo;
            }
        }
        return 0;
    }
}
