#include "CDtransmmiter.h"
namespace hx_net{
	CDtransmmiter::CDtransmmiter(int subprotocol,int addresscode)
        :Transmmiter()
        ,m_subprotocol(subprotocol)
		,m_addresscode(addresscode)
	{

	}

	CDtransmmiter::~CDtransmmiter()
	{

	}

    int CDtransmmiter::check_msg_header( unsigned char *data,int nDataLen,CmdType cmdType,int number)
	{
		switch(m_subprotocol)
		{
        case CHENGDU_KAITENG_TV10KW:
        case CHENGDU_KAITENG:
        case CHENGDU_KAITENG_KFS_II:
        {
            if(data[0]==0x7e && data[12]==m_addresscode && data[13]==0x86)
                return RE_SUCCESS;
            else
            {
                unsigned char cDes[14]={0};
                cDes[0]=0x7E;
                cDes[1]=m_addresscode;
                cDes[2]=0x81;
                cDes[3]=0x31;
                cDes[4]=0xCC;
                cDes[5]=0x7E;
                cDes[6]=m_addresscode;
                cDes[7]=0x86;
                cDes[8]=0x04;
                cDes[9]=0x32;
                cDes[10]=0xCC;
                cDes[11]=0x7E;
                cDes[12]=m_addresscode;
                cDes[13]=0x86;
                return kmp(data,nDataLen,cDes,14);
            }
        }
		case CHENGDU_XINGUANG_247:
			{
                if(data[0]!=0x01 || data[1] != 0x03){

                    return RE_HEADERROR;
                }
                else if(data[2]==0x07)
                {
                    return 125;
                }
				return (((data[3]<<8)|data[4])+2);
			}
        case CHENGDU_KT_DIG:
        {
            if(data[0]==0xAA && data[1]==0xFF && data[2]==m_addresscode)
                return RE_SUCCESS;
            else
            {
                unsigned char cDes[3]={0};
                cDes[0]=0xAA;
                cDes[1]=0xFF;
                cDes[2]=m_addresscode;
                return kmp(data,nDataLen,cDes,3);
            }
        }
        case CHENGDU_CHENGGUANG_DIG:
        {
            if(data[0]==m_addresscode &&data[1]==0x10)
                return RE_SUCCESS;
            else
            {
                unsigned char cDes[2]={0};
                cDes[0] = m_addresscode;
                cDes[1] = 0x10;
                return kmp(data,nDataLen,cDes,2);
            }
        }
		default:
            return RE_NOPROTOCOL;
		}
		return -1;
	}

	bool CDtransmmiter::IsStandardCommand()
	{
		switch(m_subprotocol)
		{
		case CHENGDU_XINGUANG_247:
			return true;
		}
		return false;
	}

	void CDtransmmiter::GetSignalCommand( devCommdMsgPtr lpParam,CommandUnit &cmdUnit )
	{

	}

	int CDtransmmiter::decode_msg_body( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate )
	{
		switch(m_subprotocol)
		{
		case CHENGDU_KAITENG:{
            return KaiTengData(data,data_ptr,nDataLen,runstate);
        }
			break;
		case CHENGDU_KAITENG_300W:
			break;
		case CHENGDU_CHENGGUANG:
			break;
		case CHENGDU_KT_DIG:
            return OnKT_digData(data,data_ptr,nDataLen,runstate);
		case CHENGDU_KAITENG_TV10KW:
			break;
		case CHENGDU_XINGUANG:
			break;
		case CHENGDU_XINGUANG_247:
            return OnXG_247Data(data,data_ptr,nDataLen,runstate);
        case CHENGDU_CHENGGUANG_DIG:
            return OnCG_dig1KwData(data,data_ptr,nDataLen,runstate);
        case CHENGDU_KAITENG_KFS_II:
            return KT_1Kw_813_Data(data,data_ptr,nDataLen,runstate);
		}
        return RE_NOPROTOCOL;
	}

	void CDtransmmiter::GetAllCmd( CommandAttribute &cmdAll )
	{
		switch(m_subprotocol)
		{
        case CHENGDU_KAITENG_TV10KW:
        case CHENGDU_KAITENG:{
            CommandUnit tmUnit;
            tmUnit.commandLen=5;
            tmUnit.ackLen = 249;
            tmUnit.commandId[0] = 0x7E;
            tmUnit.commandId[1] = m_addresscode;
            tmUnit.commandId[2] = 0x06;

            tmUnit.commandId[3] = tmUnit.commandId[1]^tmUnit.commandId[2];
            tmUnit.commandId[4] = 0xCC;
            vector<CommandUnit> vtUnit;
            vtUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
            tmUnit.commandId[2] = 0x03;
            tmUnit.commandId[3] = tmUnit.commandId[1]^tmUnit.commandId[2];
            vector<CommandUnit> vtTurnOnUnit;
            vtTurnOnUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtTurnOnUnit;
            tmUnit.commandId[2] = 0x04;
            tmUnit.commandId[3] = tmUnit.commandId[1]^tmUnit.commandId[2];
            vector<CommandUnit> vtTurnOffUnit;
            vtTurnOffUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnOffUnit;
        }
			break;
        case CHENGDU_KAITENG_KFS_II:
        {
            CommandUnit tmUnit;
            tmUnit.commandLen=5;
            tmUnit.ackLen = 249;
            tmUnit.commandId[0] = 0x7E;
            tmUnit.commandId[1] = m_addresscode;
            tmUnit.commandId[2] = 0x06;

            tmUnit.commandId[3] = tmUnit.commandId[1]^tmUnit.commandId[2];
            tmUnit.commandId[4] = 0xCC;
            vector<CommandUnit> vtUnit;
            vtUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
            tmUnit.commandId[2] = 0x03;
            tmUnit.commandId[3] = tmUnit.commandId[1]^tmUnit.commandId[2];
            vector<CommandUnit> vtTurnOnUnit;
            vtTurnOnUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtTurnOnUnit;
            tmUnit.commandId[2] = 0x04;
            tmUnit.commandId[3] = tmUnit.commandId[1]^tmUnit.commandId[2];
            vector<CommandUnit> vtTurnOffUnit;
            vtTurnOffUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnOffUnit;
        }
                    break;
		case CHENGDU_KAITENG_300W:
			break;
		case CHENGDU_CHENGGUANG:
			break;
        case CHENGDU_KT_DIG:{
            CommandUnit tmUnit;
            tmUnit.ackLen = 705;
            tmUnit.commandLen=10;
            tmUnit.commandId[0] = 0xAA;
            tmUnit.commandId[1] = 0xFF;
            tmUnit.commandId[2] = m_addresscode;
            tmUnit.commandId[3] = 0xFF;
            tmUnit.commandId[4] = 0x0A;
            tmUnit.commandId[5] = 0x00;
            tmUnit.commandId[6] = 0x52;
            unsigned short crcret = CalcCRC16_KT(tmUnit.commandId,7);
            tmUnit.commandId[7] = (crcret&0x00FF);
            tmUnit.commandId[8] = ((crcret & 0xFF00)>>8);
            tmUnit.commandId[9] = 0x55;
            vector<CommandUnit> vtUnit;
            vtUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
            tmUnit.ackLen = 0;
            tmUnit.commandId[6] = 0x60;
            crcret = CalcCRC16_KT(tmUnit.commandId,7);
            tmUnit.commandId[7] = (crcret&0x00FF);
            tmUnit.commandId[8] = ((crcret & 0xFF00)>>8);
            vector<CommandUnit> vtTurnOnUnit;
            vtTurnOnUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtTurnOnUnit;
            tmUnit.commandId[6] = 0x61;
            crcret = CalcCRC16_KT(tmUnit.commandId,7);
            tmUnit.commandId[7] = (crcret&0x00FF);
            tmUnit.commandId[8] = ((crcret & 0xFF00)>>8);
            vector<CommandUnit> vtTurnOffUnit;
            vtTurnOffUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnOffUnit;
        }
			break;
		case CHENGDU_XINGUANG:
		case CHENGDU_XINGUANG_247:
			{
				CommandUnit tmUnit;
				tmUnit.ackLen = 5;
				tmUnit.commandLen=0;
				vector<CommandUnit> vtUnit;
				vtUnit.push_back(tmUnit);
				cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
				//cmdAll.queryComm.push_back(tmUnit);
				tmUnit.commandId[0] = 0x01;
				tmUnit.commandId[1] = 0x03;
				tmUnit.commandId[2] = 0x04;
				tmUnit.commandId[3] = 0x00;
				tmUnit.commandId[4] = 0x41;
				tmUnit.commandId[5] = 0x00;
				tmUnit.commandId[6] = 0x01;
				tmUnit.commandId[7] = 0x01;	
				tmUnit.commandLen = 8;
				vector<CommandUnit> vtTurnOnUnit;
				vtTurnOnUnit.push_back(tmUnit);
				cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtTurnOnUnit;
			//	cmdAll.turnonComm.push_back(tmUnit);
				tmUnit.commandId[7] = 0x02;
				vector<CommandUnit> vtTurnOffUnit;
				vtTurnOffUnit.push_back(tmUnit);
				cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnOffUnit;
			//	cmdAll.turnoffComm.push_back(tmUnit);
			}
			break;
        case CHENGDU_CHENGGUANG_DIG:
           {
                CommandUnit tmUnit;
                tmUnit.ackLen = 8;
                tmUnit.commandLen=2;
                tmUnit.commandId[0] = m_addresscode;
                tmUnit.commandId[1] = 0x10;
                vector<CommandUnit> vtUnit;
                vtUnit.push_back(tmUnit);
                tmUnit.commandId[1] = 0x17;
                vtUnit.push_back(tmUnit);
               /* tmUnit.commandId[1] = 0x01;
                tmUnit.ackLen = 24;
                vtUnit.push_back(tmUnit);
                tmUnit.commandId[1] = 0x02;
                vtUnit.push_back(tmUnit);
                tmUnit.commandId[1] = 0x03;
                vtUnit.push_back(tmUnit);
                tmUnit.commandId[1] = 0x04;
                vtUnit.push_back(tmUnit);
                tmUnit.commandId[1] = 0x05;
                vtUnit.push_back(tmUnit);
                tmUnit.commandId[1] = 0x06;
                vtUnit.push_back(tmUnit);*/
                cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
                tmUnit.ackLen = 2;
                tmUnit.commandLen=3;
                tmUnit.commandId[1] = 0x11;
                tmUnit.commandId[2] = (tmUnit.commandId[1]+tmUnit.commandId[0])&0xFF;
                vector<CommandUnit> vtTurnonUnit;
                vtTurnonUnit.push_back(tmUnit);
                cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtTurnonUnit;
                tmUnit.commandId[1] = 0x22;
                tmUnit.commandId[2] = (tmUnit.commandId[1]+tmUnit.commandId[0])&0xFF;
                vector<CommandUnit> vtTurnoffUnit;
                vtTurnoffUnit.push_back(tmUnit);
                cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnoffUnit;
           }
            break;
		}
	}

	int CDtransmmiter::OnXG_247Data( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate )
	{
		if(data[2]!=0x03)
		{
            return RE_CMDACK;
		}
		int curpos=5;
		int iparamtype=0;
		int iparamlen = 0;
		
		while((curpos+2)<nDataLen)
		{
			iparamtype = ((data[curpos]<<8)|data[curpos+1]);
			iparamlen = ((data[curpos+2]<<8)|data[curpos+3]);
			int ivalue=0;
			for(int i=0;i<iparamlen;++i)
			{
				ivalue = ((ivalue<<8)|data[curpos+4+i]);
			}
			curpos = curpos+4+iparamlen;

			DataInfo dainfo;
			switch (iparamtype)
			{
            case 0x0041:{
                    //if(ivalue==0x01)
                    //    runstate = 0;
                    //else
                    //	runstate = 1;
				}
				break;
            case 0x0501:{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[3] = dainfo;
				}
				break;
            case 0x0502:{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[4] = dainfo;
				}
				break;
			case 0x0503:
				data_ptr->mValues[5] = dainfo;
				break;
            case 0x0504:{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[6] = dainfo;
				}
				break;
            case 0x0505:{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[7] = dainfo;
				}
				break;
            case 0x0506:{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[8] = dainfo;
				}
				break;
            case 0x0507:{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[9] = dainfo;
				}
				break;
            case 0x0508:{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[10] = dainfo;
				}
				break;

            case 0x0509:{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[11] = dainfo;
				}
				break;
            case 0x050A:{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[12] = dainfo;
				}
				break;
            case 0x050B:{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[13] = dainfo;
				}
				break;
            case 0x050C:{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[14] = dainfo;
				}
				break;
			case 0x050D:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[15] = dainfo;
				}
				break;
			case 0x050E:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[16] = dainfo;
				}
				break;
			case 0x050F:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[17] = dainfo;
				}
				break;
			case 0x0510:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[18] = dainfo;
				}
				break;
			case 0x0511:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[19] = dainfo;
				}
				break;
			case 0x0512:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[20] = dainfo;
				}
				break;
			case 0x0513:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[21] = dainfo;
				}
				break;
			case 0x0514:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[22] = dainfo;
				}
				break;
			case 0x0515:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[23] = dainfo;
				}
				break;
			case 0x0516:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[24] = dainfo;
				}
				break;
			case 0x0517:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[25] = dainfo;
				}
				break;
			case 0x0518:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[26] = dainfo;
				}
				break;
			case 0x0519:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[27] = dainfo;
				}
				break;
			case 0x051A:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[28] = dainfo;
				}
				break;
			case 0x051B:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[29] = dainfo;
				}
				break;
			case 0x051C:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[30] = dainfo;
				}
				break;
			case 0x051D:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[31] = dainfo;
				}
				break;
			case 0x051E:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[32] = dainfo;
				}
				break;
			case 0x051F:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[33] = dainfo;
				}
				break;
			case 0x0520:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[34] = dainfo;
				}
				break;
			case 0x0521:
				{
					dainfo.bType = false;
					dainfo.fValue = float(ivalue*0.001);
					data_ptr->mValues[0] = dainfo;
				}
				break;
			case 0x0522:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[1] = dainfo;
				}
				break;
			case 0x0523:
				{
					dainfo.bType = false;
					dainfo.fValue = float(ivalue*0.1);
					data_ptr->mValues[35] = dainfo;
				}
				break;
			case 0x0524:
				{
					dainfo.bType = false;
					dainfo.fValue = float(ivalue*0.1);
					data_ptr->mValues[36] = dainfo;
				}
				break;
			case 0x0525:
				{
					dainfo.bType = false;
					dainfo.fValue = float(ivalue*0.1);
					data_ptr->mValues[37] = dainfo;
				}
				break;
			case 0x0526:
				{
					dainfo.bType = false;
					dainfo.fValue = ivalue;
					data_ptr->mValues[38] = dainfo;
				}
				break;
			default:break;
			}
		}
		map<int,DataInfo>::iterator iterPW = data_ptr->mValues.find(0);
		map<int,DataInfo>::iterator iterRF = data_ptr->mValues.find(1);
		DataInfo zbb;
		zbb.bType = false;
		zbb.fValue = 1.0;
		if(iterPW!=data_ptr->mValues.end() && iterRF!=data_ptr->mValues.end())
		{
			zbb.fValue = ((*iterPW).second.fValue*1000+(*iterRF).second.fValue)/((*iterPW).second.fValue*1000-(*iterRF).second.fValue);
		}
		data_ptr->mValues[2] = zbb;
        return 0;
    }

    int CDtransmmiter::OnKT_digData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        unsigned char dataType;
        int  dataLen;
        int indexpos;
        bool bhavedata=true;
        do{
            dataType = data[3];
            dataLen = (data[5]<<8)|data[4];
            DataInfo dainfo;
            switch(dataType){
              case 0x01:
              {
                indexpos = 0;
                dainfo.bType = false;
                for(int i=0;i<15;i++){
                    dainfo.fValue = (float)((data[34+i*4]<<8)|data[33+i*4]);
                    data_ptr->mValues[indexpos++] = dainfo;
                    if(i==1){
                        if(data_ptr.get()->mValues[0].fValue!=data_ptr.get()->mValues[1].fValue){
                            dainfo.fValue = (data_ptr.get()->mValues[0].fValue+data_ptr.get()->mValues[1].fValue)/(data_ptr.get()->mValues[0].fValue-data_ptr.get()->mValues[1].fValue);
                        }
                        else{
                            dainfo.fValue = 1.0;
                        }
                        data_ptr->mValues[1] = dainfo;
                    }
                }

                for(int i=0;i<3;++i)
                {
                    dainfo.fValue = 0;
                    data_ptr->mValues[indexpos++] = dainfo;
                }
                dainfo.bType = true;
                for(int i=0;i<8;++i){
                    dainfo.fValue = data[9+i*3]==0 ? 1:0;
                    data_ptr->mValues[indexpos++] = dainfo;
                }
                data_ptr.get()->mValues[0].fValue = data_ptr.get()->mValues[0].fValue*0.001;
               }
                break;
              case 0x04:{
                indexpos = 27;
                dainfo.bType = true;
                for(int i=0;i<8;++i){
                    dainfo.fValue = data[9+i*3]==0 ? 1:0;
                    data_ptr->mValues[indexpos++] = dainfo;
                }
                dainfo.bType = false;
                for(int i=0;i<16;++i){
                    dainfo.fValue = ((data[34+i*4]<<8)|data[33+i*4])*0.1;
                    data_ptr->mValues[indexpos++] = dainfo;
                }
              }
                break;
              case 0x02:{
                indexpos=51;
                dainfo.bType = true;
                dainfo.fValue = data[33]==0? 1:0;
                data_ptr->mValues[indexpos++] = dainfo;
                dainfo.fValue = data[39]==0? 1:0;
                data_ptr->mValues[indexpos++] = dainfo;
                dainfo.fValue = data[42]==0? 1:0;
                data_ptr->mValues[indexpos++] = dainfo;
                dainfo.fValue = data[45]==0? 1:0;
                data_ptr->mValues[indexpos++] = dainfo;
                dainfo.fValue = data[48]==0? 1:0;
                data_ptr->mValues[indexpos++] = dainfo;
                dainfo.bType = false;
                dainfo.fValue = data[51];
                data_ptr->mValues[indexpos++] = dainfo;
                dainfo.fValue = 0;
                data_ptr->mValues[indexpos++] = dainfo;
                dainfo.fValue = ((data[85]<<8)|data[84])*0.1;
                data_ptr->mValues[indexpos++] = dainfo;
                dainfo.fValue = ((data[113]<<8)|data[112])*0.1;
                data_ptr->mValues[indexpos++] = dainfo;
                for(int i=0;i<3;++i){
                    dainfo.fValue = ((data[125+i*4]<<8)|data[124+i*4])*0.1;
                    data_ptr->mValues[indexpos++] = dainfo;
                }
              }
                break;
              case 0x05:{
                indexpos=63;
                dainfo.bType = true;
                dainfo.fValue = data[9]==0? 1:0;
                data_ptr->mValues[indexpos++] = dainfo;
                dainfo.fValue = data[12]==0? 1:0;
                data_ptr->mValues[indexpos++] = dainfo;
                dainfo.bType = false;
                for(int i=0;i<13;++i){
                    dainfo.fValue = ((data[16+i*4]<<8)|data[15+i*4]);
                    data_ptr->mValues[indexpos++] = dainfo;
                }
              }
            }
            data=data+dataLen;
            if(data[0]!=0xAA && data[1]!=0xFF)
                bhavedata=FALSE;
        }while (bhavedata);
        return 0;
    }

    int CDtransmmiter::OnCG_dig1KwData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        if(nDataLen<8)
            return RE_CMDACK;
        while(nDataLen>=8)
        {
            int unitIndex = data[1];
            DataInfo dainfo;
            switch(unitIndex)
            {
              case 0x10:
              {
                dainfo.bType = false;
                dainfo.fValue = ((data[2]<<8)|data[3])*0.001;
                float pwr = ((data[2]<<8)|data[3]);
                data_ptr->mValues[0] = dainfo;
                dainfo.fValue = ((data[4]<<8)|data[5]);
                data_ptr->mValues[1] = dainfo;
                float ref = dainfo.fValue;
                if(pwr>ref)
                {
                    dainfo.fValue = sqrt((pwr+ref)/(pwr-ref));
                }
                else
                    dainfo.fValue = 1;
                data_ptr->mValues[2] = dainfo;
                nDataLen -= 8;
                data += 8;
              }
                break;
              case 0x17:
              {
                dainfo.bType = false;
                dainfo.fValue = data[2];
                data_ptr->mValues[3] = dainfo;
                dainfo.bType = true;
                for(int i=0;i<8;++i)
                {
                    dainfo.fValue = Getbit(data[3],i);
                    data_ptr->mValues[4+i] = dainfo;
                }
                dainfo.fValue = Getbit(data[4],0);
                data_ptr->mValues[12] = dainfo;
                dainfo.fValue = Getbit(data[4],1);
                data_ptr->mValues[13] = dainfo;
                runstate = Getbit(data[4],1);
                for(int i=3;i<8;++i)
                {
                   dainfo.fValue = Getbit(data[4],i);
                   data_ptr->mValues[11+i] = dainfo;
                }
                nDataLen -= 8;
                data += 8;
              }
                break;
            case 0x01:{
                amp_data(data,data_ptr,0);
                nDataLen -= 24;
                data += 24;
            }
                break;
            case 0x02:{
                amp_data(data,data_ptr,1);
                nDataLen -= 24;
                data += 24;
            }
                break;
            case 0x03:{
                amp_data(data,data_ptr,2);
                nDataLen -= 24;
                data += 24;
            }
                break;
            case 0x04:{
                amp_data(data,data_ptr,3);
                nDataLen -= 24;
                data += 24;
            }
                break;
            case 0x05:{
                amp_data(data,data_ptr,4);
                nDataLen -= 24;
                data += 24;
            }
                break;
            case 0x06:{
                amp_data(data,data_ptr,5);
                nDataLen -= 24;
                data += 24;
            }
                break;
            case 0x99:
            {
                nDataLen -= 2;
                data += 2;
            }
                break;
            default:
                nDataLen -= 2;
                data += 2;
                break;
            }
        }
        return 0;
    }

    void CDtransmmiter::amp_data(unsigned char *data, DevMonitorDataPtr data_ptr, int ampnum)
    {
        int index = 19+16*ampnum;
        DataInfo dainfo;
        dainfo.bType = false;
        dainfo.fValue = data[2]*0.01;
        data_ptr->mValues[index++] = dainfo;
        dainfo.fValue = (data[3]<<8)|data[4];
        data_ptr->mValues[index++] = dainfo;
        dainfo.fValue = (data[5]<<8)|data[6];
        data_ptr->mValues[index++] = dainfo;
        dainfo.fValue = ((data[7]<<8)|data[8])*0.1;
        data_ptr->mValues[index++] = dainfo;
        for(int i=0;i<11;++i)
        {
            dainfo.fValue = data[9+i]*0.1;
            data_ptr->mValues[index++] = dainfo;
        }
        dainfo.fValue = ((data[20]<<8)|data[21])*0.1;
        data_ptr->mValues[index++] = dainfo;
    }

    int CDtransmmiter::KaiTengData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        float fwd =float((256*data[15] + data[16])*0.001);
        float refpwd  = float(256*data[17] + data[18]);
        DataInfo dainfo;
        dainfo.bType = false;
        dainfo.fValue = fwd;
        data_ptr->mValues[0] = dainfo;
        dainfo.fValue = refpwd;
        data_ptr->mValues[1] = dainfo;
        if(fwd*1000>refpwd)
        {
            dainfo.fValue = sqrt((fwd*1000+refpwd)/(fwd*1000-refpwd));
        }
        else
        {
            dainfo.fValue = 1.0;
        }
        data_ptr->mValues[2] = dainfo;
        dainfo.fValue = float(256*data[19] + data[20]);
        data_ptr->mValues[3] = dainfo;
        dainfo.fValue = float(256*data[21] + data[22]);
        data_ptr->mValues[4] = dainfo;
        dainfo.fValue = float(256*data[23] + data[24]);
        data_ptr->mValues[5] = dainfo;
        dainfo.fValue = float((256*data[25] + data[26])*0.01);
        data_ptr->mValues[6] = dainfo;
        dainfo.fValue = float((256*data[27] + data[28])*0.01);
        data_ptr->mValues[7] = dainfo;
        dainfo.bType = true;
        for(int i=0;i<8;++i)
        {
            dainfo.fValue = Getbit(data[29],i);
            data_ptr->mValues[24+i] = dainfo;
        }
        int istartpos = 33;
        int pasum, li;
        //根据数据长度计算功放数目
        pasum = (nDataLen - 50)/38;
        for(li=0; li<pasum; li++)
        {
            //功放总功率P0（2字节），低位再前，高位在后，单位W；

            dainfo.bType = false;
            dainfo.fValue = float((256*data[istartpos+4+li*38]+data[istartpos+5+li*38]));
            data_ptr->mValues[8+2*li] = dainfo;

            //功放总反射功率Pr（2字节），同上，单位W；
            dainfo.fValue = float((256*data[istartpos+6+li*38]+data[istartpos+7+li*38]));
            data_ptr->mValues[9+2*li] = dainfo;


            //凯腾发射机状态量从24开始
            dainfo.bType = true;
            dainfo.fValue = Getbit(data[istartpos + 28 + li*38],1);
            data_ptr->mValues[32+2*li] = dainfo;
            dainfo.fValue = Getbit(data[istartpos + 28 + li*38],2);
            data_ptr->mValues[33+2*li] = dainfo;
        }
        return 0;
    }

    int CDtransmmiter::KT_1Kw_813_Data(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        unsigned char cDes[3]={0};
        cDes[0]=0x7E;
        cDes[1]=m_addresscode;
        cDes[2]=0x86;
        int nDatapos = kmp(data,nDataLen,cDes,3);//第一次找到模块数量位置
        data = data+nDatapos+3;
        nDataLen = nDataLen-nDatapos-3;
       /* nDatapos = kmp(data,nDataLen,cDes,3);//第二次找到主控单元数据位置
        data = data+nDatapos;
        nDataLen = nDataLen-nDatapos;*/
        DataInfo dainfo;
        int nModenum=-1;
        while(nDataLen>19)
        {
            nDatapos = kmp(data,nDataLen,cDes,3);
            if(nDatapos<0)
                break;
            data = data+nDatapos+3;
            nDataLen = nDataLen-nDatapos-3;
            nModenum = data[0];
            switch (nModenum) {
            case 0x00:{
                float fpwr = (data[2]*256+data[1])*0.001;
                float fref = data[4]*256+data[3];
                dainfo.bType = false;
                dainfo.fValue = fpwr;
                data_ptr->mValues[0] = dainfo;
                dainfo.fValue = fref;
                data_ptr->mValues[1] = dainfo;
                if(fpwr*1000>fref)
                {
                    dainfo.fValue = sqrt((fpwr*1000+fref)/(fpwr*1000-fref));
                }
                else
                    dainfo.fValue = 1.0;
                data_ptr->mValues[2] = dainfo;
                for(int i=0;i<3;++i)
                {
                    dainfo.fValue = data[6+i*2]*256+data[5+i*2];
                    data_ptr->mValues[3+i] = dainfo;
                }
                dainfo.fValue = (data[12]*256+data[11])*0.01;
                data_ptr->mValues[6] = dainfo;
                dainfo.fValue = (data[14]*256+data[13])*0.01;
                data_ptr->mValues[7] = dainfo;
                runstate = Getbit(data[15],0);
                dainfo.bType = true;
                for(int i=0;i<8;++i)
                {
                    dainfo.fValue = Getbit(data[15],i);
                    data_ptr->mValues[8+i] = dainfo;
                }
                data=data+17;
                nDataLen = nDataLen-17;
            }
                break;
            case 0x0A:{
                dainfo.bType = false;
                for(int i=0;i<10;++i)
                {
                    dainfo.fValue = (data[2+2*i]*256+data[1+2*i])*0.1;
                    data_ptr->mValues[16+i] = dainfo;
                }
                dainfo.bType = true;
                for(int i=0;i<5;++i)
                {
                    dainfo.fValue = Getbit(data[21],i);
                    data_ptr->mValues[26+i] = dainfo;
                }
                data=data+32;
                nDataLen = nDataLen-32;
            }
                break;
            case 0x64:{
                dainfo.bType = true;
                for(int i=0;i<3;++i)
                {
                    dainfo.fValue = Getbit(data[1],i);
                    data_ptr->mValues[31+i] = dainfo;
                }
                dainfo.bType = false;
                int nFre = data[3]*100+data[4]*10+data[5];
                float fFre = data[6]*0.1+data[7]*0.01+data[8]*0.001+data[9]*0.0001+data[10]*0.00001+data[11]*0.000001;
                dainfo.fValue = nFre+fFre;
                data_ptr->mValues[34] = dainfo;
                dainfo.fValue = (data[13]*256+data[12])*0.1;
                data_ptr->mValues[35] = dainfo;
                dainfo.fValue = (data[15]*256+data[14])*0.1;
                data_ptr->mValues[36] = dainfo;
                dainfo.fValue = data[16];
                data_ptr->mValues[37] = dainfo;
                dainfo.fValue = data[17];
                data_ptr->mValues[38] = dainfo;
                data=data+20;
                nDataLen = nDataLen-20;
            }
                break;
            case 0x0B:{
                amp_813_data(data,data_ptr,0);
                data=data+32;
                nDataLen = nDataLen-32;
            }
                break;
            case 0x0C:{
                amp_813_data(data,data_ptr,1);
                data=data+32;
                nDataLen = nDataLen-32;
            }
                break;
            case 0x0D:{
                amp_813_data(data,data_ptr,2);
                data=data+32;
                nDataLen = nDataLen-32;
            }
                break;
            case 0x0E:{
                amp_813_data(data,data_ptr,3);
                data=data+32;
                nDataLen = nDataLen-32;
            }
                break;
            default:
            {
                data=data+3;
                nDataLen = nDataLen-3;
            }
                break;
            }
        }
        return 0;
    }

    void CDtransmmiter::amp_813_data(unsigned char *data, DevMonitorDataPtr data_ptr, int ampnum)
    {
        DataInfo dainfo;
        dainfo.bType = false;
        for(int i=0;i<15;++i)
        {//17
            dainfo.fValue = (data[2+2*i]*256+data[1+2*i])*0.1;
            data_ptr->mValues[39+i+17*ampnum] = dainfo;
        }
        dainfo.bType = true;
        for(int i=0;i<2;++i)
        {
            dainfo.fValue = Getbit(data[31],i);
            data_ptr->mValues[54+i+17*ampnum] = dainfo;
        }
    }
}
