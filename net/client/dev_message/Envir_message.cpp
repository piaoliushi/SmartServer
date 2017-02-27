#include "Envir_message.h"
#include"../../../utility.h"
namespace hx_net
{

    Envir_message::Envir_message(session_ptr pSession,DeviceInfo &devInfo)
		:m_pSession(pSession)
        ,d_devInfo(devInfo)
	{
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
				default:
					return RE_NOPROTOCOL;
				}
			}
			break;
		case HUIXIN:
			{
				switch (d_devInfo.nSubProtocol)
				{
				case HUIXIN_992:
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
							cDes[1] = 0x65;
							cDes[2] = 0xF3;
							return kmp(data,nDataLen,cDes,3);
						}
					}
					break;
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
		switch(d_devInfo.nDevProtocol)
		{
		case WS2032:
			{
				switch (d_devInfo.nSubProtocol)
				{
				case WS2032_A:
					{
                    return EnvironWS302(data,data_ptr,nDataLen,iaddcode);
					}
					break;
				case THB11RS:
                    return EnvironTHB11RS(data,data_ptr,nDataLen,iaddcode);
				case KD40_IO:
                    return KD40RData(data,data_ptr,nDataLen,iaddcode);
				case C2000_M21A:
                    return C2000_M21A_Data(data,data_ptr,nDataLen,iaddcode);
				case AC_103_CTR:
					{
						
					}
					break;
				case FRT_X06A:
					{
                        int iresult = FRT_X06A_Data(data,data_ptr,nDataLen,iaddcode);
                        m_pSession->start_handler_data(iaddcode,data_ptr);
                        return iresult;
					}
					break;
				default:
					return RE_NOPROTOCOL;
				}
			}
			break;
		case HUIXIN:
			{
				switch (d_devInfo.nSubProtocol)
				{
				case HUIXIN_992:
                    return On992Data(data,data_ptr,nDataLen,iaddcode);
					break;
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
				}
			}
			break;
		case HUIXIN:
			{
				switch (d_devInfo.nSubProtocol)
				{
				case HUIXIN_992:
					{
						CommandUnit tmUnit;
						tmUnit.commandId[0] = 0x55;
						tmUnit.commandId[1] = 0x02;
						tmUnit.commandId[2] = 0xF1;
						tmUnit.commandId[3] = 0xF1;
						tmUnit.commandId[4] = 0x00;
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

    int Envir_message::On992Data( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode )
	{
		unsigned char bdata1,bdata2,bdata3,bdata4;
		float fdat;
		int indexpos=0;
		DataInfo dainfo;
		dainfo.bType = false;
        iaddcode = d_devInfo.iAddressCode;
		for(int i=0;i<9;i++)//取温度
		{
			bdata1 = data[3 + 4*i];//通道号码必须从9开始
			bdata2 = data[4 + 4*i];
			bdata3 = data[5 + 4*i];
			bdata4 = data[6 + 4*i];

			*(((char*)(&fdat) + 0)) = bdata1;
			*(((char*)(&fdat) + 1)) = bdata2;
			*(((char*)(&fdat) + 2)) = bdata3;
			*(((char*)(&fdat) + 3)) = bdata4;
			dainfo.fValue = fdat;
			data_ptr->mValues[indexpos++] = dainfo;
		}
		bdata1 = data[39];
		dainfo.bType = true;
		for(int i =0; i<5;i++)//取火灾数据
		{
			dainfo.fValue = Getbit(bdata1,i)==0? 0:1;
			data_ptr->mValues[indexpos++] = dainfo;
		}
		bdata1 = data[40];
		for(int i =0; i<5;i++)//取水灾数据
		{
			dainfo.fValue = Getbit(bdata1,i)==0? 0:1;
			data_ptr->mValues[indexpos++] = dainfo;
		}
		return RE_SUCCESS;
	}
}
