#include "ShTransmmit.h"
#include "../../../../utility.h"
namespace hx_net{
    ShTransmmit::ShTransmmit(dev_session_ptr pSession,int subprotocol,int addresscode)
        :Transmmiter()
        ,m_subprotocol(subprotocol)
		,m_addresscode(addresscode)
        ,m_curMsgType(NO_CMDTYPE)
        ,m_curMsgId(0)
        ,m_pSession(pSession)
        //,m_sDevId(sDevId)
	{
	}

	ShTransmmit::~ShTransmmit()
	{

	}

    int ShTransmmit::check_msg_header( unsigned char *data,int nDataLen ,CmdType cmdType,int number)
	{
        m_curMsgType = cmdType;
        m_curMsgId = number;
		switch(m_subprotocol)
		{
		case All_Band_Pa:
			{
				if(data[0]==0xA5 && data[1]==0x5A)
                    return (((data[3]<<8)|data[2])-4);
				else
					return -1;
			}
		default:
			return -1;
		}
		return -1;
	}

    bool ShTransmmit::isLastQueryCmd()
    {
          if(m_curMsgId == m_maxMsgId-1)
              return true;
          return false;
    }

	bool ShTransmmit::IsStandardCommand()
	{
		switch(m_subprotocol)
		{
		case All_Band_Pa:
			return true;
		}
		return false;
	}

	void ShTransmmit::GetSignalCommand( devCommdMsgPtr lpParam,CommandUnit &cmdUnit )
	{

	}

	int ShTransmmit::decode_msg_body( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate )
	{

        int nResult=-1;
        switch(m_subprotocol)
        {
        case All_Band_Pa:{
            nResult = OnAllBandData(data,data_ptr,nDataLen,runstate);

            break;
        }
        }
        return nResult;
	}

     bool ShTransmmit::isMultiQueryCmd()
     {
         switch(m_subprotocol)
         {
         case All_Band_Pa:
             return true;
         }
         return false;
     }

	void ShTransmmit::GetAllCmd( CommandAttribute &cmdAll )
	{
		switch(m_subprotocol)
		{
		case All_Band_Pa:
			{
				CommandUnit tmUnit;
				tmUnit.ackLen = 4;
				tmUnit.commandLen=16;
                tmUnit.commandId[0] = 0xA5;
                tmUnit.commandId[1] = 0x5A;
				tmUnit.commandId[2] = 0x10;
				tmUnit.commandId[3] = 0x00;
				tmUnit.commandId[4] = 0x00;
				tmUnit.commandId[5] = 0x00;
				tmUnit.commandId[6] = 0x00;
				tmUnit.commandId[7] = 0x00;
				tmUnit.commandId[8] = 0x01;
				tmUnit.commandId[9] = 0x00;
				tmUnit.commandId[10] = 0x01;
				tmUnit.commandId[11] = 0x00;
				/*int isum=0;
				for(int i=0;i<12;++i)
				{
					isum+=tmUnit.commandId[i];
				}
				isum = isum&0xFFFF;
				tmUnit.commandId[12] = ((isum&0xFF00)>>8);
				tmUnit.commandId[13] = (isum&0xFF);*/
				tmUnit.commandId[12] = 0x11;
				tmUnit.commandId[13] = 0x01;
				
				tmUnit.commandId[14] = 0xEF;
				tmUnit.commandId[15] = 0xFE;
				vector<CommandUnit> vtUnit;
                vtUnit.push_back(tmUnit);
                /*tmUnit.commandId[10] = 0x02;
				tmUnit.commandId[12] = 0x12;
                vtUnit.push_back(tmUnit);*/
				tmUnit.commandId[10] = 0x20;
				tmUnit.commandId[12] = 0x30;
				vtUnit.push_back(tmUnit);
                tmUnit.commandId[10] = 0x03;
				tmUnit.commandId[12] = 0x13;
				vtUnit.push_back(tmUnit);
                /*tmUnit.commandId[10] = 0x04;
				tmUnit.commandId[12] = 0x14;
				vtUnit.push_back(tmUnit);
				tmUnit.commandId[10] = 0x05;
				tmUnit.commandId[12] = 0x15;
				vtUnit.push_back(tmUnit);
				tmUnit.commandId[10] = 0x10;
				tmUnit.commandId[12] = 0x20;
                vtUnit.push_back(tmUnit);*/
				cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
                m_maxMsgId = 3;
			}
			break;
		}
	}

	int ShTransmmit::OnAllBandData( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate )
	{
		int nDataType = data[10];
		switch(nDataType)
		{
		case 0x01://功放整体运行状态
			{
				int index = 0;
                int basebit=11;
				DataInfo dainfo;
				dainfo.bType = false;
				for(int i=0;i<9;++i)
				{
					int nValue=0;

                    for(int j=4;j>0;--j)
					{
                        int num = basebit+j+4*i;
                        nValue = ((nValue<<8)|data[num]);
					}
					dainfo.fValue = nValue*0.01;
                    if(i==0)
                        dainfo.fValue/=1000;
                    data_ptr->mValues[index++] = dainfo;
				}

			}
			break;
		case 0x02://功放整体告警状态
			{
				int index = 9;
				int basebit=12;
				DataInfo dainfo;
				dainfo.bType = true;
				for(int i=0;i<9;++i)
				{
					dainfo.fValue = data[basebit+i];
                    data_ptr->mValues[index++] = dainfo;
				}
			}
			break;
		case 0x03://预放运行状态
			{
				int index = 28;
				int basebit=11;
				DataInfo dainfo;
				dainfo.bType = false;
				for(int i=0;i<9;++i)
				{
					int nValue=0;
                    for(int j=4;j>0;--j)
					{
						nValue = ((nValue<<8)|data[basebit+j+4*i]);
					}
					dainfo.fValue = nValue*0.01;
                    data_ptr->mValues[index++] = dainfo;
				}
			}
			break;
		case 0x04://预放告警状态
			{
				int index = 39;
				int basebit=12;
				DataInfo dainfo;
				dainfo.bType = true;
				for(int i=0;i<6;++i)
				{
					dainfo.fValue = data[basebit+i];
                    data_ptr->mValues[index++] = dainfo;
				}
			}
			break;
		case 0x20://功放整体运行参数
			{
				int index = 18;
				int basebit=12;
				DataInfo dainfo;
				dainfo.bType = true;
				dainfo.fValue = data[basebit];
                data_ptr->mValues[index++] = dainfo;
				dainfo.bType = false;
				dainfo.fValue = data[basebit+1];
                data_ptr->mValues[index++] = dainfo;
				dainfo.fValue = ((data[basebit+3]<<8)|data[basebit+2]);
                data_ptr->mValues[index++] = dainfo;
				dainfo.fValue = data[basebit+4];
                data_ptr->mValues[index++] = dainfo;
				dainfo.fValue = data[basebit+5];
                data_ptr->mValues[index++] = dainfo;
				dainfo.fValue = data[basebit+6];
                data_ptr->mValues[index++] = dainfo;
				dainfo.fValue = data[basebit+7];
                data_ptr->mValues[index++] = dainfo;
				dainfo.fValue = ((data[basebit+9]<<8)|data[basebit+8]);
                data_ptr->mValues[index++] = dainfo;
				dainfo.fValue = data[basebit+10];
                data_ptr->mValues[index++] = dainfo;
			}
			break;
		case 0x05://后级功放模块1运行状态
            LastBandRunData(data,data_ptr,nDataLen,0);
			break;
		case 0x06://后级功放模块2运行状态
            LastBandRunData(data,data_ptr,nDataLen,1);
			break;
		case 0x07://后级功放模块3运行状态
            LastBandRunData(data,data_ptr,nDataLen,2);
			break;
		case 0x08://后级功放模块4运行状态
            LastBandRunData(data,data_ptr,nDataLen,3);
			break;
		case 0x09://后级功放模块5运行状态
            LastBandRunData(data,data_ptr,nDataLen,4);
			break;
		case 0x0A://后级功放模块6运行状态
            LastBandRunData(data,data_ptr,nDataLen,5);
			break;
		case 0x0B://后级功放模块7运行状态
            LastBandRunData(data,data_ptr,nDataLen,6);
			break;
		case 0x0C://后级功放模块8运行状态
            LastBandRunData(data,data_ptr,nDataLen,7);
			break;
		case 0x0D://后级功放模块9运行状态
            LastBandRunData(data,data_ptr,nDataLen,8);
			break;
		case 0x0E://后级功放模块10运行状态
            LastBandRunData(data,data_ptr,nDataLen,9);
			break;
		case 0x10://后级功放模块1告警状态
            LastBandSateData(data,data_ptr,nDataLen,0);
			break;
		case 0x11://后级功放模块2告警状态
            LastBandSateData(data,data_ptr,nDataLen,1);
			break;
		case 0x12://后级功放模块3告警状态
            LastBandSateData(data,data_ptr,nDataLen,2);
			break;
		case 0x13://后级功放模块4告警状态
            LastBandSateData(data,data_ptr,nDataLen,3);
			break;
		case 0x14://后级功放模块5告警状态
            LastBandSateData(data,data_ptr,nDataLen,4);
			break;
		case 0x15://后级功放模块6告警状态
            LastBandSateData(data,data_ptr,nDataLen,5);
			break;
		case 0x16://后级功放模块7告警状态
            LastBandSateData(data,data_ptr,nDataLen,6);
			break;
		case 0x17://后级功放模块8告警状态
            LastBandSateData(data,data_ptr,nDataLen,7);
			break;
		case 0x18://后级功放模块9告警状态
            LastBandSateData(data,data_ptr,nDataLen,8);
			break;
		case 0x19://后级功放模块10告警状态
            LastBandSateData(data,data_ptr,nDataLen,9);
			break;
		}
		return 0;
	}

    void ShTransmmit::LastBandRunData( unsigned char *data,DevMonitorDataPtr pBandData,int nDataLen,int nmodenum )
	{
		int index = 45+24*nmodenum;
		int basebit=12;
		DataInfo dainfo;
		dainfo.bType = false;
		for(int i=0;i<9;++i)
		{
			int nValue=0;
			for(int j=4;j>=0;--j)
			{
				nValue = ((nValue<<8)|data[basebit+j+4*i]);
			}
			dainfo.fValue = nValue*0.01;
			pBandData->mValues[index++] = dainfo;
		}
		basebit = 48;
		for(int j=0;j<3;++j)
		{
			dainfo.fValue = ((data[basebit+j+1]<<8)|data[basebit+j])*0.01;
			pBandData->mValues[index++] = dainfo;
		}
	}

    void ShTransmmit::LastBandSateData( unsigned char *data,DevMonitorDataPtr pBandData,int nDataLen,int nmodenum )
	{
		int index = 57+24*nmodenum;
		int basebit=12;
		DataInfo dainfo;
		dainfo.bType = true;
		for(int i=0;i<12;++i)
		{
			dainfo.fValue = data[basebit+i];
			pBandData->mValues[index++] = dainfo;
		}
	}
}
