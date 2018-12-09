#include "Envir_message.h"
#include"../../../utility.h"
#include "LocalConfig.h"
namespace hx_net
{

    Envir_message::Envir_message(session_ptr pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo)
        :d_devInfo(devInfo)
        ,io_service_(io_service)
	{
        m_pSession = boost::dynamic_pointer_cast<device_session>(pSession);
        if(IsStandardCommand())
            d_curData_ptr = DevMonitorDataPtr(new Data);
	}

	Envir_message::~Envir_message(void)
	{
	}

    int Envir_message::check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number)
	{
		switch(d_devInfo.nDevProtocol)
		{
		case WS2032:
			{
				switch (d_devInfo.nSubProtocol)
				{
				case WS2032_A:
					{
						if(nDataLen<2)
							return RE_HEADERROR;
						else
						{
							if (data[0] == 0x55 && data[1] == d_devInfo.iAddressCode)
							{
								return 0;
							}
							else
							{
								unsigned char cDes[2]={0};
								cDes[0]=0x55;
								cDes[1]=d_devInfo.iAddressCode;
								return kmp(data,nDataLen,cDes,2);
							}
						}
					}
					break;
				case THB11RS:
					{
						if(nDataLen<3)
							return RE_HEADERROR;
						else
						{
							if(data[0] == d_devInfo.iAddressCode && data[1] == 0x04)
							{
								return 0;
							}
							else
							{
								unsigned char cDes[2]={0};
								cDes[0] = d_devInfo.iAddressCode;
								cDes[1] = 0x04;
								return kmp(data,nDataLen,cDes,2);
							}
						}
                    }
					break;
				case KD40_IO:
					{
						if(nDataLen<1)
							return RE_HEADERROR;
						else
						{
							if(data[0] == d_devInfo.iAddressCode)
								return 0;
							else
							{
								unsigned char cDes[1]={0x01};
								cDes[0] = d_devInfo.iAddressCode;
								return kmp(data,nDataLen,cDes,1);
							}
						}
					}
					break;
				case C2000_M21A:
					{
						if(data[0] == 0x00 && data[1] == 0x01 && data[2] == 0x00 && data[3] == 0x00 )
							return 0;
						else
						{
							unsigned char cDes[4]={0x00};
							cDes[1] = 0x01;
							return kmp(data,nDataLen,cDes,4);
						}
					}
					break;
				case AC_103_CTR:
					{
						if(data[0]!=d_devInfo.iAddressCode)
							return RE_HEADERROR;
						else
							return (data[5]+2);
					}
					break;
				case FRT_X06A:
					{
						if(data[0]!=0x01 || data[8]!=0x02)
							return RE_HEADERROR;
						return int(((data[6]<<8)|data[7])+4);
					}
					break;
                case C2000_A2_8020:
                    {
                        if(data[0]!=d_devInfo.iAddressCode)
                            return RE_HEADERROR;
                        if(data[1]==0x0F || data[1]==0x05)
                            return 5;
                        if(data[1]==0x02 || data[1]==0x01 )
                            return 3;

                        return RE_HEADERROR;
                    }
                    break;
                case C2000_SDD8020_BB3:
                {
                    if(data[0]!=0x00 || data[1]!=0x01)
                        return RE_HEADERROR;
                    return data[5];
                }
                    break;
                case NT511_AD:
                {
                    if(data[0]!=d_devInfo.iAddressCode || data[1]!=0x03)
                        return RE_HEADERROR;
                    return data[2]+2;
                }
                default:
                    return RE_NOPROTOCOL;
				}
			}
			break;
		}
		
		return RE_UNKNOWDEV;
	}
    int Envir_message::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode)
	{
        if(data_ptr!=NULL)
            d_curData_ptr = data_ptr;
		switch(d_devInfo.nDevProtocol)
		{
		case WS2032:
			{
				switch (d_devInfo.nSubProtocol)
				{
				case WS2032_A:
					{
                    return EnvironWS302(data,d_curData_ptr,nDataLen,iaddcode);
					}
					break;
				case THB11RS:
                    return EnvironTHB11RS(data,d_curData_ptr,nDataLen,iaddcode);
				case KD40_IO:
                    return KD40RData(data,d_curData_ptr,nDataLen,iaddcode);
				case C2000_M21A:
                    return C2000_M21A_Data(data,d_curData_ptr,nDataLen,iaddcode);
				case AC_103_CTR:
					{
						
					}
					break;
				case FRT_X06A:
					{
                        int iresult = FRT_X06A_Data(data,d_curData_ptr,nDataLen,iaddcode);
                        m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                        return iresult;
					}
					break;
                case C2000_A2_8020:
                {
                    int iresult=C2000_A2_Data(data,d_curData_ptr,nDataLen,iaddcode);
                    m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                    return iresult;
                }
                case C2000_SDD8020_BB3:
                {
                    int iresult = C2000_BB3_Data(data,d_curData_ptr,nDataLen,iaddcode);
                    m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                    return iresult;
                }
                case NT511_AD:
                {
                    int iresult = NT511_AD_Data(data,d_curData_ptr,nDataLen,iaddcode);
                    m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                    return iresult;
                }
                default:
					return RE_NOPROTOCOL;
				}
			}
			break;
		}
		return RE_UNKNOWDEV;
	}

    int  Envir_message::parse_AC103CTR_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode)
	{
        iaddcode = d_devInfo.iAddressCode;
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
        switch (d_devInfo.nSubProtocol)
		{
		case WS2032_A:
			break;
		case AC_103_CTR:
		case FRT_X06A:
        case C2000_A2_8020:
        case C2000_SDD8020_BB3:
        case NT511_AD:
            return true;


		}
		return false;
	}
	
	void Envir_message::GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit)
	{
	}

	void Envir_message::GetAllCmd( CommandAttribute &cmdAll )
	{
		switch(d_devInfo.nDevProtocol){
		case WS2032:
			{
				switch(d_devInfo.nSubProtocol)
				{
				case WS2032_A:
					{
						CommandUnit tmUnit;
						tmUnit.ackLen = 13;
						tmUnit.commandLen = 8;
						tmUnit.commandId[0] = 0x55;
						tmUnit.commandId[1] = d_devInfo.iAddressCode;
						tmUnit.commandId[2] = 0x00;
						tmUnit.commandId[3] = 0x01;
						tmUnit.commandId[4] = 0x01;
						tmUnit.commandId[5] = 0x00;
						tmUnit.commandId[6] = tmUnit.commandId[1]+tmUnit.commandId[2]+tmUnit.commandId[3]+tmUnit.commandId[4]+tmUnit.commandId[5];
						tmUnit.commandId[7] = 0xAA;
						cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
					}
					break;
				case THB11RS:
					{
						CommandUnit tmUnit;
						tmUnit.ackLen = 9;
						tmUnit.commandLen = 8;
						tmUnit.commandId[0] = d_devInfo.iAddressCode;
						tmUnit.commandId[1] = 0x04;
						tmUnit.commandId[2] = 0x00;
						tmUnit.commandId[3] = 0x00;
						tmUnit.commandId[4] = 0x00;
						tmUnit.commandId[5] = 0x02;
						unsigned short uscrc = CRC16_A001(tmUnit.commandId,6);
						tmUnit.commandId[6] = (uscrc&0x00FF);
						tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
						cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
					}
					break;
				case KD40_IO:
					{
                        unsigned char KD40_REQUIRY[] = {0x01,0x03,0x06,0x40,0x00,0x04,0x45,0x55};
						CommandUnit tmUnit;
						tmUnit.ackLen = 13;
						tmUnit.commandLen = 8;
						tmUnit.commandId[0] = d_devInfo.iAddressCode;
						tmUnit.commandId[1] = 0x03;
						tmUnit.commandId[2] = 0x06;
						tmUnit.commandId[3] = 0x40;
						tmUnit.commandId[4] = 0x00;
						tmUnit.commandId[5] = 0x04;
						unsigned short uscrc = CRC16_A001(tmUnit.commandId,6);
						tmUnit.commandId[6] = (uscrc&0x00FF);
						tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
						cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
					}
					break;
				case C2000_M21A:
					{
						CommandUnit tmUnit;
						tmUnit.commandId[0] = 0x00;
						tmUnit.commandId[1] = 0x01;
						tmUnit.commandId[2] = 0x00;
						tmUnit.commandId[3] = 0x00;
						tmUnit.commandId[4] = 0x00;
						tmUnit.commandId[5] = 0x06;
						tmUnit.commandId[6] = 0x01;
						tmUnit.commandId[7] = 0x03;
						tmUnit.commandId[8] = 0x00;
						tmUnit.commandId[9] = 0x1A;
						tmUnit.commandId[10] = 0x00;
						tmUnit.commandId[11] = 0x10;
						tmUnit.commandLen = 12;
						tmUnit.ackLen = 41;
						cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
					}
					break;
				case FRT_X06A:
					{
						CommandUnit tmUnit;
						tmUnit.commandId[0] = 0x01;
						tmUnit.commandId[1] = (d_devInfo.iAddressCode&0xFF00)>>8;
						tmUnit.commandId[2] = (d_devInfo.iAddressCode&0x00FF);
						tmUnit.commandId[3] = 0x00;
						tmUnit.commandId[4] = 0x00;
						tmUnit.commandId[5] = 0x07;
						tmUnit.commandId[6] = 0x00;
						tmUnit.commandId[7] = 0x00;
						tmUnit.commandId[8] = 0x02;
						tmUnit.commandId[9] = 0x03;
						unsigned short crcret = CRC16_CCITT(tmUnit.commandId,10);
						tmUnit.commandId[10] = (crcret&0xFF00)>>8;
						tmUnit.commandId[11] = (crcret&0x00FF);
						tmUnit.commandId[12] = 0x04;
						tmUnit.commandLen = 13;
						tmUnit.ackLen = 9;
						cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
					}
					break;
                case C2000_A2_8020:
                {
                    CommandUnit tmUnit;
                    tmUnit.ackLen = 3;
                    tmUnit.commandLen = 8;
                    tmUnit.commandId[0] = d_devInfo.iAddressCode;
                    tmUnit.commandId[1] = 0x02;
                    tmUnit.commandId[2] = 0x00;
                    tmUnit.commandId[3] = 0xC8;
                    tmUnit.commandId[4] = 0x00;
                    tmUnit.commandId[5] = 0x08;
                    unsigned short uscrc = CRC16_A001(tmUnit.commandId,6);
                    tmUnit.commandId[6] = (uscrc&0x00FF);
                    tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
                    cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);

                    tmUnit.commandId[1] = 0x01;
                    tmUnit.commandId[3] = 0x64;
                    tmUnit.commandId[5] = 0x02;
                    uscrc = CRC16_A001(tmUnit.commandId,6);
                    tmUnit.commandId[6] = (uscrc&0x00FF);
                    tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
                    cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);


//                    tmUnit.commandId[1] = 0x0f;
//                    tmUnit.commandId[2] = 0x00;
//                    tmUnit.commandId[3] = 0x64;
//                    tmUnit.commandId[4] = 0x00;
//                    tmUnit.commandId[5] = 0x02;
//                    tmUnit.commandId[6] = 0x01;
//                    tmUnit.commandId[7] = 0x01;
//                    uscrc = CRC16_A001(tmUnit.commandId,8);
//                    tmUnit.commandId[8] = (uscrc&0x00FF);
//                    tmUnit.commandId[9] = ((uscrc & 0xFF00)>>8);
//                    tmUnit.commandLen = 10;
//                    cmdAll.mapCommand[MSG_DEV_TURNON_OPR].push_back(tmUnit);

//                    tmUnit.commandId[7] = 0x02;
//                    uscrc = CRC16_A001(tmUnit.commandId,8);
//                    tmUnit.commandId[8] = (uscrc&0x00FF);
//                    tmUnit.commandId[9] = ((uscrc & 0xFF00)>>8);
//                    tmUnit.commandLen = 10;

                    tmUnit.commandId[1] = 0x05;
                    tmUnit.commandId[2] = 0x00;
                    tmUnit.commandId[3] = 0x64;
                    tmUnit.commandId[4] = 0xFF;
                    tmUnit.commandId[5] = 0x00;
                    uscrc = CRC16_A001(tmUnit.commandId,6);
                    tmUnit.commandId[6] = (uscrc&0x00FF);
                    tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
                    cmdAll.mapCommand[MSG_DEV_TURNON_OPR].push_back(tmUnit);
                    tmUnit.commandId[3] = 0x65;
                    uscrc = CRC16_A001(tmUnit.commandId,6);
                    tmUnit.commandId[6] = (uscrc&0x00FF);
                    tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
                    cmdAll.mapCommand[MSG_DEV_TURNON_OPR].push_back(tmUnit);

                    tmUnit.commandId[3] = 0x64;
                    tmUnit.commandId[4] = 0x00;
                    uscrc = CRC16_A001(tmUnit.commandId,6);
                    tmUnit.commandId[6] = (uscrc&0x00FF);
                    tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
                    cmdAll.mapCommand[MSG_DEV_TURNOFF_OPR].push_back(tmUnit);
                    tmUnit.commandId[3] = 0x65;
                    uscrc = CRC16_A001(tmUnit.commandId,6);
                    tmUnit.commandId[6] = (uscrc&0x00FF);
                    tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);


                    cmdAll.mapCommand[MSG_DEV_TURNOFF_OPR].push_back(tmUnit);

                }
                    break;
                case C2000_SDD8020_BB3:
                {
                    //000100000006FF0200C80008
                    CommandUnit tmUnit;
                    tmUnit.commandId[0] = 0x00;
                    tmUnit.commandId[1] = 0x01;
                    tmUnit.commandId[2] = 0x00;
                    tmUnit.commandId[3] = 0x00;
                    tmUnit.commandId[4] = 0x00;
                    tmUnit.commandId[5] = 0x06;
                    tmUnit.commandId[6] = 0xFF;
                    tmUnit.commandId[7] = 0x02;
                    tmUnit.commandId[8] = 0x00;
                    tmUnit.commandId[9] = 0xC8;
                    tmUnit.commandId[10] = 0x00;
                    tmUnit.commandId[11] = 0x08;
                    tmUnit.ackLen = 6;
                    tmUnit.commandLen = 12;
                    cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
                    //000100000006FF0100640002
                    tmUnit.commandId[7] = 0x01;
                    tmUnit.commandId[9] = 0x64;
                    tmUnit.commandId[11] = 0x02;
                    cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
                    //000100000008FF0F006400020101
                    /* tmUnit.commandId[5] = 0x08;
                                tmUnit.commandId[7] = 0x0F;
                                tmUnit.commandId[9] = 0x64;
                                tmUnit.commandId[11] = 0x02;
                                tmUnit.commandId[12] = 0x01;
                                tmUnit.commandId[13] = 0x01;
                                tmUnit.commandLen = 14;
                                cmdAll.mapCommand[MSG_DEV_TURNON_OPR].push_back(tmUnit);

                                tmUnit.commandId[13] = 0x02;
                                cmdAll.turnoffComm.push_back(tmUnit);
                                cmdAll.mapCommand[MSG_DEV_TURNOFF_OPR].push_back(tmUnit);*/
                    tmUnit.commandId[7] = 0x05;
                    tmUnit.commandId[9] = 0x64;
                    tmUnit.commandId[10] = 0xFF;
                    tmUnit.commandId[11] = 0x00;
                    cmdAll.mapCommand[MSG_DEV_TURNON_OPR].push_back(tmUnit);
                    tmUnit.commandId[9] = 0x65;
                    cmdAll.mapCommand[MSG_DEV_TURNON_OPR].push_back(tmUnit);
                    tmUnit.commandId[9] = 0x64;
                    tmUnit.commandId[10] = 0x00;
                    cmdAll.mapCommand[MSG_DEV_TURNOFF_OPR].push_back(tmUnit);
                    tmUnit.commandId[9] = 0x65;
                    cmdAll.mapCommand[MSG_DEV_TURNOFF_OPR].push_back(tmUnit);

                }
                    break;
                case NT511_AD:{
                    CommandUnit tmUnit;
                    tmUnit.ackLen = 3;
                    tmUnit.commandLen = 8;
                    tmUnit.commandId[0] = d_devInfo.iAddressCode;
                    tmUnit.commandId[1] = 0x03;
                    tmUnit.commandId[2] = 0x00;
                    tmUnit.commandId[3] = 0x00;
                    tmUnit.commandId[4] = 0x00;
                    tmUnit.commandId[5] = 0x08;
                    unsigned short uscrc = CRC16_A001(tmUnit.commandId,6);
                    tmUnit.commandId[6] = (uscrc&0x00FF);
                    tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
                    cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
                }
                    break;
                }

        }
            break;
		}
	}

    int Envir_message::EnvironWS302( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode )
	{
		DataInfo dainfo;
		dainfo.bType = false;
		dainfo.fValue = float(((data[5]<<8)|data[6])/10.0);
		data_ptr->mValues[0] = dainfo;
		dainfo.fValue = float(((data[7]<<8)|data[8])/10.0);
		data_ptr->mValues[1] = dainfo;
        iaddcode = data[1];
		return RE_SUCCESS;
	}

    int Envir_message::EnvironTHB11RS( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode )
	{
		DataInfo dainfo;
		dainfo.bType = false;
		dainfo.fValue = float(((data[3]<<8)|data[4])/10.0);
		data_ptr->mValues[0] = dainfo;
		dainfo.fValue = float(((data[5]<<8)|data[6])/10.0);
		data_ptr->mValues[1] = dainfo;
        iaddcode = data[0];
		return RE_SUCCESS;
	}

    int Envir_message::KD40RData( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode )
	{
		int index = 0;
		for (int i=0;i<4;++i)
		{
			DataInfo dainfo;
			dainfo.bType = true;
            dainfo.fValue = (float)(((data[3+2*i]<<8)|data[4+2*i]));
            if(dainfo.fValue>0)
                dainfo.sValue="1";
            else
                dainfo.sValue="0";
            data_ptr->mValues[index] = dainfo;

            index++;
		}
        iaddcode = data[0];
		return RE_SUCCESS;
	}

    int Envir_message::C2000_M21A_Data( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode )
	{
		int index = 0;
		unsigned char bdata1,bdata2,bdata3,bdata4;
		float fdat = 0.0;
		DataInfo dainfo;
		dainfo.bType = false;
		for(int i=0;i<8;++i)
		{
			/*if((12+i*4)>=nDataLen)
				return ;*/
			bdata1 = data[9 + 4*i];//通道号码必须从9开始
			bdata2 = data[10 + 4*i];
			bdata3 = data[11 + 4*i];
			bdata4 = data[12 + 4*i];
			*(((char*)(&fdat) + 0)) = bdata4;
			*(((char*)(&fdat) + 1)) = bdata3;
			*(((char*)(&fdat) + 2)) = bdata2;
			*(((char*)(&fdat) + 3)) = bdata1;
			dainfo.fValue = (float)((fdat*1000-4)*6.25);
			data_ptr->mValues[index++] = dainfo;
		}
        iaddcode = d_devInfo.iAddressCode;
		return RE_SUCCESS;
	}

    int Envir_message::FRT_X06A_Data( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode )
	{
		int count = data[0];//监测个数
		int data_index = 1; //解析位置
		DataInfo dainfo;
        iaddcode = d_devInfo.iAddressCode;
		for(int i=0;i<count;i++)
		{
			int channum = (data[data_index]<<8)|data[data_index+1];
			switch(channum)
			{
			case 0x0002:
				{
					int index=0;
					for(int j=0;j<6;j++)
					{
						dainfo.bType = false;
						dainfo.fValue = data[data_index+2+j];
						data_ptr->mValues[index++] = dainfo;
					}
					data_ptr->mValues[0].fValue +=2000;
					data_index+=8;
				}
				break;
			case 0x0006:
				{
					dainfo.bType = false;
					dainfo.fValue = data[data_index+2]*0.1;
					data_ptr->mValues[6] = dainfo;
					data_index+=3;
				}
				break;
			case 0x0007:
				{
					dainfo.bType = true;
					dainfo.fValue = data[data_index+2];
					data_ptr->mValues[7] = dainfo;
					data_index+=3;
				}
				break;
			case 0x0100:
				{
					dainfo.bType = false;
					dainfo.fValue = ((data[data_index+2]<<8)|data[data_index+3])*0.1;
					data_ptr->mValues[8] = dainfo;
					data_index+=4;
				}
				break;
			case 0x0080:
				{
					dainfo.bType = false;
					dainfo.fValue = ((data[data_index+2]<<8)|data[data_index+3])*0.1;
					data_ptr->mValues[9] = dainfo;
					data_index+=4;
				}
				break;
			case 0x0081:
				{
					dainfo.bType = false;
					dainfo.fValue = ((data[data_index+2]<<8)|data[data_index+3])*0.1;
					data_ptr->mValues[10] = dainfo;
					data_index+=4;
				}
				break;
			case 0x00A1:
				{
					dainfo.bType = false;
					dainfo.fValue = ((data[data_index+2]<<8)|data[data_index+3])*0.1;
					data_ptr->mValues[11] = dainfo;
					data_index+=4;
				}
				break;
			case 0x00A2:
				{
					dainfo.bType = false;
					dainfo.fValue = ((data[data_index+2]<<8)|data[data_index+3])*0.1;
					data_ptr->mValues[12] = dainfo;
					data_index+=4;
				}
				break;
			case 0x0020:
				{
					dainfo.bType = false;
					dainfo.fValue = ((data[data_index+2]<<8)|data[data_index+3])*0.1;
					data_ptr->mValues[13] = dainfo;
					data_index+=4;
				}
				break;
			case 0x0040:
				{
					dainfo.bType = false;
					dainfo.fValue = ((data[data_index+2]<<8)|data[data_index+3])*0.1;
					data_ptr->mValues[14] = dainfo;
					data_index+=4;
				}
				break;
			case 0x0060:
				{
					dainfo.bType = false;
					dainfo.fValue = ((data[data_index+2]<<8)|data[data_index+3])*0.1;
					data_ptr->mValues[15] = dainfo;
					data_index+=4;
				}
				break;
			case 0x0082:
				{
					dainfo.bType = false;
					dainfo.fValue = ((data[data_index+2]<<8)|data[data_index+3])*0.1;
					data_ptr->mValues[16] = dainfo;
					data_index+=4;
				}
				break;
			case 0x0083:
				{
					dainfo.bType = false;
					dainfo.fValue = ((data[data_index+2]<<8)|data[data_index+3])*0.1;
					data_ptr->mValues[17] = dainfo;
					data_index+=4;
				}
				break;
			case 0x0084:
				data_index+=4;
				break;
			case 0x0085:
				data_index+=4;
				break;
			default:
				data_index+=4;
				break;
			}
		}
		return RE_SUCCESS;
	}


    int Envir_message::C2000_A2_Data(unsigned char *data, DevMonitorDataPtr data_ptr,
                                     int nDataLen, int &iaddcode)
    {
        iaddcode = data[0];
        if(data[1]==0x02)
        {
            unsigned char bdata1;
            bdata1 = data[3];
            DataInfo dainfo;
            dainfo.bType = true;
            for(int i=0;i<8;++i)
            {
                dainfo.fValue =  Getbit(bdata1,i);
                data_ptr->mValues[i] = dainfo;
            }
        }else if(data[1]==0x01)
        {
            unsigned char bdata1;
            bdata1 = data[3];
            DataInfo dainfo;
            dainfo.bType = true;
            for(int i=0;i<2;++i)
            {
                dainfo.fValue =  Getbit(bdata1,i);
                data_ptr->mValues[i+8] = dainfo;
            }
        }
        return RE_SUCCESS;
    }

    int Envir_message::C2000_BB3_Data(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
    {
        iaddcode = d_devInfo.iAddressCode;
        if(data[7]==0x02)
        {
            unsigned char bdata1;
            bdata1 = data[9];
            DataInfo dainfo;
            dainfo.bType = true;
            for(int i=0;i<8;++i)
            {
                dainfo.fValue =  Getbit(bdata1,i);
                data_ptr->mValues[i] = dainfo;
            }
        }
        else if(data[1]==0x01)
        {
            unsigned char bdata1;
            bdata1 = data[9];
            DataInfo dainfo;
            dainfo.bType = true;
            for(int i=0;i<2;++i)
            {
                dainfo.fValue =  Getbit(bdata1,i);
                data_ptr->mValues[i+8] = dainfo;
            }
        }
        return RE_SUCCESS;
    }

    //执行任务
    void Envir_message::exec_task_now(int icmdType,string sUser,e_ErrorCode &eErrCode,map<int,string> &mapParam,int nMode,
                                     bool bSnmp,Snmp *snmp,CTarget *target)
    {

        eErrCode = EC_OK;
        switch(d_devInfo.nSubProtocol){
        case C2000_SDD8020_BB3:
        case C2000_A2_8020:
        {
            switch (icmdType) {
            case MSG_TRANSMITTER_TURNOFF_OPR:
            case MSG_DEV_TURNOFF_OPR:{
            if(m_pSession!=NULL)

                m_pSession->send_cmd_to_dev(d_devInfo.sDevNum,MSG_DEV_TURNOFF_OPR,0,eErrCode);

            }
                break;
            default:
                break;
            }
        }
        break;
        default:
            break;
        }

         m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
    }

    //执行通用命令
    void Envir_message::exec_general_task(int icmdType,string sUser,devCommdMsgPtr lpParam,e_ErrorCode &eErrCode)
    {
        eErrCode = EC_OK;
        switch (icmdType) {
        case MSG_DEV_TURNOFF_OPR:{
            if(m_pSession!=NULL){
                eErrCode = EC_OK;
                int param_2 = atoi(lpParam->cparams(0).sparamvalue().c_str());
                cout<<"send MSG_DEV_TURNOFF_OPR---"<<"param:"<<param_2<<endl;
                m_pSession->send_cmd_to_dev(d_devInfo.sDevNum,MSG_DEV_TURNOFF_OPR,param_2,eErrCode);
            }
        }
            break;
        default:
            break;
        }
        m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
    }

    //执行联动命令
    void Envir_message::exec_action_task_now(map<int,vector<ActionParam> > &param,int actionType,string sUser,e_ErrorCode &eErrCode)
    {
        switch(actionType){
        case ACTP_SOUND_LIGHT_ALARM:{
            //联动声光告警只做开设备动作（打开声光告警）
            if(m_pSession!=NULL){
                eErrCode = EC_OK;
                if(param.size()>=2){
                    int param_2 = atoi(param[1][0].strParamValue.c_str());
                    boost::asio::deadline_timer delay_send_timer(io_service_, boost::posix_time::milliseconds(20));
                    delay_send_timer.wait();
                    m_pSession->send_cmd_to_dev(d_devInfo.sDevNum,MSG_DEV_TURNON_OPR,param_2,eErrCode);
                }
            }
        }
            break;
        }
    }

    int Envir_message::NT511_AD_Data(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
    {
        iaddcode = data[0];
        DataInfo dainfo;
        dainfo.bType = false;
        for(int i=0;i<4;++i)
        {
            dainfo.fValue = data[3+2*i]*256+data[4+2*i]-20;
            data_ptr->mValues[i] = dainfo;
        }
        dainfo.bType = true;
        for(int j=0;j<4;++j)
        {
            dainfo.fValue = Getbit(data[12+2*j],1);
            data_ptr->mValues[4+6*j] = dainfo;
            dainfo.fValue = Getbit(data[12+2*j],2);
            data_ptr->mValues[5+6*j] = dainfo;
            dainfo.fValue = Getbit(data[12+2*j],3);
            data_ptr->mValues[6+6*j] = dainfo;
            dainfo.fValue = Getbit(data[12+2*j],4);
            data_ptr->mValues[7+6*j] = dainfo;
            dainfo.fValue = Getbit(data[12+2*j],6);
            data_ptr->mValues[8+6*j] = dainfo;
            dainfo.fValue = Getbit(data[12+2*j],7);
            data_ptr->mValues[9+6*j] = dainfo;
        }


        //cout<<"wendu="<<data_ptr->mValues[0].fValue<<"--"<<data_ptr->mValues[1].fValue<<"--"<<data_ptr->mValues[2].fValue<<endl;

         return RE_SUCCESS;
    }

}
