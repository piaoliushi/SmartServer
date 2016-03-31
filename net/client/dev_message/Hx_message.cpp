#include "Hx_message.h"
namespace hx_net
{


    Hx_message::Hx_message(net_session *pSession)
		:m_pSession(pSession)
	{
	}

	Hx_message::~Hx_message(void)
	{
	}

	void Hx_message::SetSubPro(int subprotocol)
	{
		m_Subprotocol = (HxSubPrototcol)subprotocol;
	}

	bool Hx_message::IsStandardCommand()
	{
		switch(m_Subprotocol)
		{
		case HUIXIN_DATA_MGR:
		case HUIXIN_6300:
		case HUIXIN_9020:
		case HUIXIN_740P:
		case HUIXIN_730P:
		case HUIXIN_761:
			return true;
		}
		return false;
	}
	
	void Hx_message::GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit)
	{
		switch(m_Subprotocol)
		{
		case HUIXIN_6300:
			{
				cmdUnit.commandLen = 14;
				cmdUnit.commandId[0] = 0xAA;
				cmdUnit.commandId[1] = 0x63;
				cmdUnit.commandId[2] = 0x11;
				cmdUnit.commandId[3] = ((m_DevAddr&0xFF00)>>8);
				cmdUnit.commandId[4] = (m_DevAddr&0x00FF);
				cmdUnit.commandId[5] = 0x00;
				cmdUnit.commandId[6] = 0x07;
				cmdUnit.commandId[7] = 0x00;
				cmdUnit.commandId[8] = 0x00;
				cmdUnit.commandId[9] = 0x00;
				cmdUnit.commandId[10] = 0x00;
				cmdUnit.commandId[11] = 0x00;
				cmdUnit.commandId[12] = 0x00;
				cmdUnit.commandId[13] = 0x55;
				int nParam = atoi(lpParam->cparams(0).sparamvalue().c_str());
				if(nParam==0)//+
				{
					cmdUnit.commandId[2] = 0x33;
					cmdUnit.commandId[7] = 0x01;
					cmdUnit.commandId[12] = 0x01;
				}
				else if(nParam==1)//-
				{
					cmdUnit.commandId[2] = 0x33;
					cmdUnit.commandId[7] = 0x00;
					cmdUnit.commandId[12] = 0x00;
				}

			}
			break;
		case HUIXIN_9020:
			{

				cmdUnit.commandLen = 14;
				cmdUnit.commandId[0] = 0xAA;
				cmdUnit.commandId[1] = 0x92;
				cmdUnit.commandId[2] = 0x66;
				cmdUnit.commandId[3] = ((m_DevAddr&0xFF00)>>8);
				cmdUnit.commandId[4] = (m_DevAddr&0x00FF);
				cmdUnit.commandId[5] = 0x00;
				
				cmdUnit.commandId[6] = 0x07; 
				cmdUnit.commandId[7] = 0x00;
				int nParam = atoi(lpParam->cparams(0).sparamvalue().c_str());
				if(nParam<6&& nParam>=0)
					cmdUnit.commandId[7] = nParam;
				cmdUnit.commandId[8] = 0x01;
				cmdUnit.commandId[9] = 0x00;
				cmdUnit.commandId[10] = 0x00;
				cmdUnit.commandId[11] = 0x00;
				cmdUnit.commandId[12] = (cmdUnit.commandId[7]+cmdUnit.commandId[8])&0xff;
				cmdUnit.commandId[13] = 0x55;
			}
			break;
		case HUIXIN_0804D:
			{
				cmdUnit.commandLen = 6;
				cmdUnit.commandId[0] = 0xAA;
				cmdUnit.commandId[1] = 0x22;
				cmdUnit.commandId[2] = atoi(lpParam->cparams(0).sparamvalue().c_str());//输入信号通道号0-7
				cmdUnit.commandId[3] = atoi(lpParam->cparams(1).sparamvalue().c_str());//输出信号通道号0-3
				cmdUnit.commandId[4] = cmdUnit.commandId[1]^cmdUnit.commandId[2]^cmdUnit.commandId[3];
				cmdUnit.commandId[5] = 0x55;
			}
			break;
		case HUIXIN_761://切换通道
			{
				//int channelId = atoi(lpParam->cparams(0).sparamvalue().c_str());
			}
			break;

		}
		return;
	}

	int  Hx_message::PreHandleMsg()
	{
		if(m_ParamPtr==NULL || m_pSession==NULL)
			return -1;
		switch(m_Subprotocol)
		{
		case HUIXIN_DATA_MGR:
			{
				if(m_ParamPtr->antenaS!=dev_unknown)//存在关联设备
				{
					dev_run_state current_antenna_state = m_pSession->get_child_run_state(m_ParamPtr->sAntennaId);
					if(m_ParamPtr->antenaS!=current_antenna_state)
					{
						//广播设备状态到在线客户端
						string oldDevId;
						if(current_antenna_state==antenna_host){
							m_pSession->clear_dev_alarm(m_ParamPtr->sHostId);
							m_pSession->clear_dev_state(m_ParamPtr->sHostId);//非天线所在发射机，清除报警
							m_pSession->set_child_run_state(m_ParamPtr->sHostId,dev_shutted_down);
							oldDevId = m_ParamPtr->sHostId;
						}
						else if(current_antenna_state==antenna_backup){
							m_pSession->clear_dev_alarm(m_ParamPtr->sBackupId);
							m_pSession->clear_dev_state(m_ParamPtr->sBackupId);//非天线所在发射机，清除报警
							m_pSession->set_child_run_state(m_ParamPtr->sBackupId,dev_shutted_down);
							oldDevId = m_ParamPtr->sBackupId;
						}

						m_pSession->set_child_run_state(m_ParamPtr->sAntennaId,m_ParamPtr->antenaS);
						if(!oldDevId.empty())
						{
							DevMonitorDataPtr zeroData_ptr(new Data);
							m_pSession->send_data_to_client(oldDevId,zeroData_ptr);
						}

					}

				}
			}
			break;
		case HUIXIN_6300:
			break;
		}
		return -1;
	}

	void Hx_message::input_params(const vector<string> &vParam)
	{
		switch(m_Subprotocol)
		{
		case HUIXIN_DATA_MGR:
			{
				if(vParam.size()==4)
				{
					if(m_ParamPtr==NULL)
						m_ParamPtr = hxDataMgrParamPtr(new HxDataMgrParam);
					m_ParamPtr->sHostId=vParam[0];
					m_ParamPtr->sBackupId=vParam[1];
					m_ParamPtr->sAntennaId=vParam[2];
					m_ParamPtr->sStationId = vParam[3];
				}
			}
			break;
		case HUIXIN_6300:
			break;
		}
		return;
	}

	int Hx_message::check_msg_header(unsigned char *data,int nDataLen)
	{
		switch(m_Subprotocol)
		{
		case HUIXIN_DATA_MGR:
			{
				if(data[0]!=0x7E && data[1]!=0x30)
					return -1;
				else
					return ((data[5]<<8)|data[4]);//计算消息体长度
			}
		case HUIXIN_6300:
			{
				if(data[0]!=0xAA || data[1]!=0x63)
					return -1;
				else
					return int((data[5]<<8)|data[6]);
			}

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
				return int((data[5]<<8)|data[6]);
			break;
		case HUIXIN_730P:
			if(data[0]!=0x7E || data[2]!=0x22)
				return -1;
			else
				return int((data[6]<<8)|data[5]);
			break;
		case HUIXIN_761:
			{
				DevUploadHeadPtr msg_ = (DevUploadHeadPtr)(&data[0]);
				if(msg_->startCode != 0x7e)
					return -1;
				if(msg_->msgBodyLen<=0 || msg_->msgBodyLen>MaxBodySize)
					return -1;
				return msg_->msgBodyLen;
			}
			break;
		case HUIXIN_0804D:
			{
				if(data[0]==0xAA && data[1]==0x33)
				{	
					return 0;
				}
			}
			break;
		}
		return -1;

	}

	int Hx_message::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		switch(m_Subprotocol)
		{
		case HUIXIN_DATA_MGR:
			return parse_DATA_MGR_data(data,data_ptr,nDataLen);
		case HUIXIN_6300:
		case HUIXIN_9020:
		case HUIXIN_740P:
			return parse_740P_data(data,data_ptr,nDataLen);
		case HUIXIN_761:
			return parse_761_data(data,data_ptr,nDataLen);
		case HUIXIN_760:
            return -1;//Agent()->HxPaseData(data_ptr.get(),(LPBYTE)(data),nDataLen);
		case HUIXIN_730P:
			return parse_730P_data(data,data_ptr,nDataLen);
		}
		return -1;
	}



	//解析汇鑫数据管理器数据,根据返回值决定是否需要进行数据解析
	int Hx_message::parse_DATA_MGR_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		if(nDataLen<6)
			return -1;

		int nCmdType = data[7];
		string sDevId;
		m_ParamPtr->antenaS =dev_unknown;
		sDevId.assign((const char*)(&(data[8])),8);
		switch(nCmdType)//查询回复数据
		{

		case 0x11://查询数据回复
			{
                //data_ptr->sId = sDevId;//保存当前设备id
                //if(Agent()->HxPaseData(data_ptr.get(),(LPBYTE)(data+16),nDataLen-31)<0)
				{
					//LOG(ERROR)<<"查询回复数据解析出错--Data:";
					if(nDataLen>0 &&nDataLen<1024)
					{
						string outputStr;
						CharStr2HexStr(data,outputStr,nDataLen);
						//LOG(ERROR)<<"查询回复数据解析出错--Data:"<<outputStr;
					}
					return -1;
				}
				if(data[nDataLen-13]==0x01)
					m_ParamPtr->antenaS = antenna_backup;
				else if(data[nDataLen-14]==0x01)
					m_ParamPtr->antenaS = antenna_host;
				else if(data[nDataLen-13]==0x00 && data[nDataLen-14]==0x00)
					m_ParamPtr->antenaS = dev_detecting;
				return 0;
			}
		case 0x77://开关机指令回复
			{
				if(nDataLen!=19)
				{
					//LOG(ERROR)<<"开关机控制回复数据长度出错--!!!";
					if(nDataLen>0 &&nDataLen<1024)
					{
						string outputStr;
						CharStr2HexStr(data,outputStr,nDataLen);
						//LOG(ERROR)<<"开关机控制回复数据长度出错--Data:"<<outputStr;
					}
					return -1;
				}
				int nResult = data[17];
				e_MsgType tmMsgType;
				if(data[16]==0x00)
					tmMsgType = MSG_TRANSMITTER_TURNOFF_ACK;
				else
					tmMsgType = MSG_TRANSMITTER_TURNON_ACK;
				m_pSession->excute_result_notify(sDevId,TRANSMITTER,tmMsgType,nResult);
				//LOG(ERROR)<<"开关机控制下位机回复结果data[17]="<<nResult;
				return 1;
			}
			break;
		case 0xb0://天线（主->备）指令回复
			{
				if(nDataLen!=18)
				{
					//LOG(ERROR)<<"天线(主->备)控制回复数据长度出错--!!!";
					if(nDataLen>0 &&nDataLen<1024)
					{
						string outputStr;
						CharStr2HexStr(data,outputStr,nDataLen);
						//LOG(ERROR)<<"天线(主->备)控制回复数据长度出错--Data:"<<outputStr;
					}
					return -1;
				}
				int nResult = data[16];
				e_MsgType tmMsgType;
				tmMsgType = MSG_ANTENNA_BTOH_ACK;
				if(nResult==0x01)
					m_pSession->excute_result_notify(sDevId,ANTENNA,tmMsgType,nResult);
				else if(nResult==0x03)
					m_pSession->excute_result_notify(sDevId,ANTENNA,tmMsgType,0x02);
				else if(nResult==0x02)
					m_pSession->excute_result_notify(sDevId,ANTENNA,tmMsgType,0x03);
				else if(nResult==0x04)
					m_pSession->excute_result_notify(sDevId,ANTENNA,tmMsgType,0x04);
				//LOG(ERROR)<<"开关机控制下位机回复结果data[16]="<<nResult;
				return 1;
			}
			break;
		case 0xb1://天线（备->主）指令回复
			{
				if(nDataLen!=18)
				{
					//LOG(ERROR)<<"天线(备->主)控制回复数据长度出错--!!!";
					string outputStr;
					if(nDataLen>0 &&nDataLen<1024)
					{
						CharStr2HexStr(data,outputStr,nDataLen);
						//LOG(ERROR)<<"天线(备->主)控制回复数据长度出错--Data:"<<outputStr;
					}
					return -1;
				}
				int nResult = data[16];
				e_MsgType tmMsgType;
				tmMsgType = MSG_ANTENNA_HTOB_ACK;
				if(nResult==0x01)
					m_pSession->excute_result_notify(sDevId,ANTENNA,tmMsgType,nResult);
				else if(nResult==0x03)
					m_pSession->excute_result_notify(sDevId,ANTENNA,tmMsgType,0x02);
				else if(nResult==0x02)
					m_pSession->excute_result_notify(sDevId,ANTENNA,tmMsgType,0x03);
				else if(nResult==0x04)
					m_pSession->excute_result_notify(sDevId,ANTENNA,tmMsgType,0x04);
				//LOG(ERROR)<<"开关机控制下位机回复结果data[16]="<<nResult;
				return 1;
			}
			break;
		default:
			cout<<"??????command:"<<nCmdType<<endl;

		}
		return -1;
	}

	int  Hx_message::getChannelCount()
	{
		switch(m_Subprotocol)
		{
		case HUIXIN_DATA_MGR:
			return 0;
		case HUIXIN_6300:
		case HUIXIN_9020:
		case HUIXIN_740P:
		case HUIXIN_761:
		case HUIXIN_760:
		case HUIXIN_730P:
			return 6;
		}
		return 0;
	}

	bool Hx_message::isMonitorChannel(int nChnnel,DevMonitorDataPtr curDataPtr)
	{
		switch(m_Subprotocol)
		{
		case HUIXIN_9020:
			{
				switch(nChnnel)
				{
				case 1:
                    if (curDataPtr->mValues[2].fValue==1.0f)
						return true;
					break;
				case 2:
                    if (curDataPtr->mValues[5].fValue==1.0f)
						return true;
					break;
				case 3:
                    if (curDataPtr->mValues[8].fValue==1.0f)
						return true;
					break;
				case 4:
                    if (curDataPtr->mValues[11].fValue==1.0f)
						return true;
					break;
				case 5:
                    if (curDataPtr->mValues[14].fValue==1.0f)
						return true;
					break;
				case 6:
                    if (curDataPtr->mValues[17].fValue==1.0f)
						return true;
					break;

				}
				return false;
			}
			break;
		}

		return true;
	}

	//该监控量是否属于该通道
	bool Hx_message::isBelongChannel(int nChnnel,int monitorItemId)
	{
		switch(m_Subprotocol)
		{
		case HUIXIN_740P:
			{
				switch(nChnnel)
				{
				case 1:
					if (monitorItemId>=0 && monitorItemId<6)
						return true;
					break;
				case 2:
					if (monitorItemId>=6 && monitorItemId<12)
						return true;
					break;
				case 3:
					if (monitorItemId>=12 && monitorItemId<18)
						return true;
					break;
				case 4:
					if (monitorItemId>=18 && monitorItemId<24)
						return true;
					break;
				case 5:
					if (monitorItemId>=24 && monitorItemId<30)
						return true;
					break;
				case 6:
					if (monitorItemId>=30 && monitorItemId<36)
						return true;
					break;

				}
			}
			break;
		case HUIXIN_761:
		case HUIXIN_760:
		case HUIXIN_730P:
			{
				switch(nChnnel)
				{
				case 1:
					if (monitorItemId>=0 && monitorItemId<9)
							return true;
					break;
				case 2:
					if (monitorItemId>=9 && monitorItemId<18)
						return true;
					break;
				case 3:
					if (monitorItemId>=18 && monitorItemId<27)
						return true;
					break;
				case 4:
					if (monitorItemId>=27 && monitorItemId<36)
						return true;
					break;
				case 5:
					if (monitorItemId>=36 && monitorItemId<45)
						return true;
					break;
				case 6:
					if (monitorItemId>=45 && monitorItemId<54)
						return true;
					break;
					
				}
			}
			break;
		case HUIXIN_9020:
			{
				switch(nChnnel)
				{
				case 1:
					if (monitorItemId>=0 && monitorItemId<2)
						return true;
					break;
				case 2:
					if (monitorItemId>=2 && monitorItemId<5)
						return true;
					break;
				case 3:
					if (monitorItemId>=5 && monitorItemId<8)
						return true;
					break;
				case 4:
					if (monitorItemId>=8 && monitorItemId<11)
						return true;
					break;
				case 5:
					if (monitorItemId>=11 && monitorItemId<14)
						return true;
					break;
				case 6:
					if (monitorItemId>=14 && monitorItemId<17)
						return true;
					break;

				}
			}
		}

		return false;
	}
	
	//当前项是否在报警
	bool Hx_message::itemIsAlarm(int nAlarmS,int itemId,dev_alarm_state &alarm_state)
	{
		if(nAlarmS==1)
		{
			if(m_mapItemAlarmRecord.find(itemId)!=m_mapItemAlarmRecord.end())
			{
				if(m_mapItemAlarmRecord[itemId].first == 1)
				{
					if(m_mapItemAlarmRecord[itemId].second == 1)
					{
						alarm_state = lower_alarm;
						m_mapItemAlarmRecord[itemId].second++;
						return true;
					}
				}
				else
				{
					//第一次数据低于下限
					m_mapItemAlarmRecord[itemId].first = 1;
					m_mapItemAlarmRecord[itemId].second = 1;
				}
			}
			else
			{
				//无报警则添加该报警项
				m_mapItemAlarmRecord[itemId].first=1;
				m_mapItemAlarmRecord[itemId].second=1;
			}
		}else
		{
			//查找此报警是否已经存在
			if(m_mapItemAlarmRecord.find(itemId)!=m_mapItemAlarmRecord.end())
			{
				m_mapItemAlarmRecord.erase(itemId);
				alarm_state = resume_normal;
			}
		}
		return false;
	}

	//判断监测量是否报警
	bool Hx_message::ItemValueIsAlarm(DevMonitorDataPtr curDataPtr,int monitorItemId,dev_alarm_state &curState)
	{
		switch(m_Subprotocol)
		{
		case HUIXIN_761:
			{
				if(m_mapAlarm.find(monitorItemId)==m_mapAlarm.end())
					return false;
				else
				{
					int alarmId = m_mapAlarm[monitorItemId];
					return itemIsAlarm(alarmId,monitorItemId,curState);
				}
			}
			break;
		case HUIXIN_9020:
			break;
		}
		return false;
	}

	int Hx_message::parse_761_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		DevUploadHeadPtr msg_ = (DevUploadHeadPtr)(&data[0]);
		if(msg_->msgBodyLen<1 || data[nDataLen-1]!=0x55)
			return -1;
		switch(msg_->commandCode)
		{
		case 0x22://监测数据
			{
                int nResult;// = On761Data(data_ptr,data,nDataLen);//Agent()->HxPaseData(data_ptr.get(),data,nDataLen);
                if(nResult==0 && data_ptr->mValues.size()>0)
				{
					if(m_pSession)
						m_pSession->start_handler_data(data_ptr,false);
					return 0;
				}else
					return -1;
			}
			break;
		case 0x11://音频数据
			{
				//unchar_ptr curAudioData(new vector<unsigned char>);
				//curAudioData->assign(data[8],data[nDataLen-1]);
				boost::uint8_t uChannel = data[7];
				if(m_pSession)
					m_pSession->start_handler_mp3_data_ex(uChannel,&(data[8]),nDataLen-9);
					//m_pSession->start_handler_mp3_data_ex(uChannel,curAudioData);
				return 0;
			}
			break;
		case 0x33://设置频点
			break;
		case 0x44:
			break;
		case 0x66:
			break;
		case 0xEE://登陆
			break;
		}
		return -1;
	}

	int Hx_message::parse_730P_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		switch(data[2])
		{
		case 0x22:
            int nResult;// = Agent()->HxPaseData(data_ptr.get(),data,nDataLen);
            if(nResult==0 && data_ptr->mValues.size()>0)
			{
				if(m_pSession)
					m_pSession->start_handler_data(data_ptr);
				return 0;
			}else
				return -1;
			break;
		}

		return -1;
	}

	int Hx_message::parse_740P_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		switch(data[2])
		{
		case 0x11:
            int nResult;// = Agent()->HxPaseData(data_ptr.get(),data,nDataLen);
            if(nResult==0 && data_ptr->mValues.size()>0)
			{
				if(m_pSession)
					m_pSession->start_handler_data(data_ptr);
				return 0;
			}else
				return -1;
			break;
		}

		return -1;
	}

    /*int Hx_message::On761Data( DevMonitorDataPtr dataBuf,LPBYTE lpBuffer,DWORD dwCount )
	{
		int cmdid = lpBuffer[2];
		if(cmdid == 0x22)
		{
			int num=(((lpBuffer[6]<<8)|lpBuffer[5])-8)/11;
			for(int i=0;i<num;++i)
			{
				int index = lpBuffer[7+11*i]-1;
				if(index*11+15>dwCount)//检查数据越界
					return -1;
				dataBuf->datainfoBuf[index*9].bType = false;
				if(lpBuffer[8+11*i]==1)
					dataBuf->datainfoBuf[index*9].fValue = (lpBuffer[10+11*i]<<8|(lpBuffer[9+11*i]))*0.01;
				else
					dataBuf->datainfoBuf[index*9].fValue = (lpBuffer[10+11*i]<<8|(lpBuffer[9+11*i]));
				dataBuf->datainfoBuf[index*9+1].bType = false;
				dataBuf->datainfoBuf[index*9+1].fValue = (lpBuffer[8+11*i]);
				dataBuf->datainfoBuf[index*9+2].bType = false;
				dataBuf->datainfoBuf[index*9+2].fValue = (lpBuffer[11+11*i]);
				dataBuf->datainfoBuf[index*9+3].bType = false;
				dataBuf->datainfoBuf[index*9+3].fValue = (lpBuffer[12+11*i]);
				dataBuf->datainfoBuf[index*9+4].bType = true;
				dataBuf->datainfoBuf[index*9+4].fValue = (lpBuffer[13+11*i])==1? 0:1;
				dataBuf->datainfoBuf[index*9+5].bType = false;
				dataBuf->datainfoBuf[index*9+5].fValue = (lpBuffer[14+11*i]);
				dataBuf->datainfoBuf[index*9+6].bType = false;
				dataBuf->datainfoBuf[index*9+6].fValue = (lpBuffer[15+11*i]);
				dataBuf->datainfoBuf[index*9+7].bType = false;
				dataBuf->datainfoBuf[index*9+7].fValue = (lpBuffer[16+11*i]);
				dataBuf->datainfoBuf[index*9+8].bType = false;
				dataBuf->datainfoBuf[index*9+8].fValue = (lpBuffer[17+11*i]);
			}
			for(int j=0;j<6;++j)//6个通道报警信息
			{
				dataBuf->datainfoBuf[54+3*j].bType = true;
				dataBuf->datainfoBuf[55+3*j].bType = true;
				dataBuf->datainfoBuf[56+3*j].bType = true;
				dataBuf->datainfoBuf[54+3*j].fValue = Getbit(lpBuffer[73+j],1);
				dataBuf->datainfoBuf[55+3*j].fValue = Getbit(lpBuffer[73+j],2);
				dataBuf->datainfoBuf[56+3*j].fValue = Getbit(lpBuffer[73+j],0);
				
				m_mapAlarm[7+9*j] = (Getbit(lpBuffer[73+j],1)==1)?1:0;
				m_mapAlarm[8+9*j] = (Getbit(lpBuffer[73+j],2)==1)?1:0;
				m_mapAlarm[2+9*j] = (Getbit(lpBuffer[73+j],0)==1)?1:0;

			}
			for(int k=0;k<6;++k)
			{
				dataBuf->datainfoBuf[72+k].bType = true;
				dataBuf->datainfoBuf[72+k].fValue = Getbit(lpBuffer[79],k);
			}
			dataBuf->Length = 78;
		}
		
		return 0;
    }*/

	
}
