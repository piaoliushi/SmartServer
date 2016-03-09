#include "Envir_message.h"

namespace net
{

	Envir_message::Envir_message(session *pSession)
		:m_pSession(pSession)
	{
	}

	Envir_message::~Envir_message(void)
	{
	}

	void Envir_message::SetSubPro(int subprotocol)
	{
		m_Subprotocol = (SgSubProtocol)subprotocol;
	}

	int Envir_message::check_msg_header(unsigned char *data,int nDataLen)
	{
		switch (m_Subprotocol)
		{
		case WS2032_A:
			break;
		case AC_103_CTR:
			{
				if(data[0]!=m_DevAddr)
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
                //Agent()->HxPaseData(data_ptr.get(),(LPBYTE)(data),nDataLen);
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
        /*switch (m_Subprotocol)
		{
		case WS2032_A:
			break;
		case AC_103_CTR:
            {//智能空调命令组织
				if(lpParam->cparams_size()<1)
					return;
				int nChannel = atoi(lpParam->cparams(0).sparamvalue().c_str());
				BYTE AC_103_CONTRL[] = {0x01,0x8B,0x00,0x00,0x00,0x02,0x01,0x10,0xFF,0xFF};
				cmdUnit.commandLen = sizeof(AC_103_CONTRL);
				switch(nChannel)
				{
                    case 0://模式切换(0--7)[0:制冷,1:制热,2:送风,3:抽湿,4:自动,5:模式+1,6:+,7:-]
					{
						if(lpParam->cparams_size()!=2)
							return;
						AC_103_CONTRL[6] = 0x06;
						//AC_103_CONTRL[7] = 0x01;
						unsigned long dResult;
						int modId = atoi(lpParam->cparams(1).sparamvalue().c_str());
						//for(int i=0;i<8;++i)
						{
							AC_103_CONTRL[7]=modId+1;
							dResult = CRC16_A001(AC_103_CONTRL,8);
							AC_103_CONTRL[8] = (dResult&0xFF00)>>8;
							AC_103_CONTRL[9] = (dResult&0x00FF);
							memcpy(cmdUnit.commandId,AC_103_CONTRL,cmdUnit.commandLen);
						}
					}
					break;
				case 1://送风(强,中,弱)
					{
						//if(lpParam->cparams_size()!=2)
						//	return;
						AC_103_CONTRL[6] = 0x03;
						AC_103_CONTRL[7] = 0x01;
						unsigned long dResult;
						//for(int i=0;i<3;++i)
						{
							int modId = atoi(lpParam->cparams(1).sparamvalue().c_str());
							//AC_103_CONTRL[7]+=modId;
							dResult = CRC16_A001(AC_103_CONTRL,8);
							AC_103_CONTRL[8] = (dResult&0xFF00)>>8;
							AC_103_CONTRL[9] = (dResult&0x00FF);
							memcpy(cmdUnit.commandId,AC_103_CONTRL,cmdUnit.commandLen);
						}
					}
					break;
				case 2://抽湿
					{
						AC_103_CONTRL[6] = 0x04;
						AC_103_CONTRL[7] = 0x01;
						unsigned long dResult = CRC16_A001(AC_103_CONTRL,8);
						AC_103_CONTRL[8] = (dResult&0xFF00)>>8;
						AC_103_CONTRL[9] = (dResult&0x00FF);
						memcpy(cmdUnit.commandId,AC_103_CONTRL,cmdUnit.commandLen);
					}
					break;
				case 3://自动
					{
						AC_103_CONTRL[6] = 0x05;
						unsigned long dResult = CRC16_A001(AC_103_CONTRL,8);
						AC_103_CONTRL[8] = (dResult&0xFF00)>>8;
						AC_103_CONTRL[9] = (dResult&0x00FF);
						memcpy(cmdUnit.commandId,AC_103_CONTRL,cmdUnit.commandLen);
					}
					break;
				case 4://开机
					{
						BYTE AC_103_OPEN[]={0x01,0x8B,0x00,0x00,0x00,0x02,0x00,0x01,0xFF,0xFF};//打开
						AC_103_OPEN[0] = m_DevAddr;
						unsigned long dResult = CRC16_A001(AC_103_OPEN,8);
						AC_103_OPEN[8] = (dResult&0xFF00)>>8;
						AC_103_OPEN[9] = (dResult&0x00FF);
						cmdUnit.commandLen = sizeof(AC_103_OPEN);
						cmdUnit.ackLen = 6;
						memcpy(cmdUnit.commandId,AC_103_OPEN,cmdUnit.commandLen);
					}
					break;
				case 5://关机
					{
						BYTE AC_103_CLOSE[]={0x01,0x8B,0x00,0x00,0x00,0x02,0x00,0x00,0xFF,0xFF};//关闭
						AC_103_CLOSE[0] = m_DevAddr;
						unsigned long dResult = CRC16_A001(AC_103_CLOSE,8);
						AC_103_CLOSE[8] = (dResult&0xFF00)>>8;
						AC_103_CLOSE[9] = (dResult&0x00FF);
						cmdUnit.commandLen = sizeof(AC_103_CLOSE);
						cmdUnit.ackLen = 6;
						memcpy(cmdUnit.commandId,AC_103_CLOSE,cmdUnit.commandLen);
					}
					break;

                }
			}
			return;
        }*/
	}
}
