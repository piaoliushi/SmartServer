#include "GmeTransmmit.h"

namespace hx_net{

	GmeTransmmit::GmeTransmmit(int subprotocol,int addresscode)
        :Transmmiter()
		,m_subprotocol(subprotocol)
		,m_addresscode(addresscode)
	{

    }

	GmeTransmmit::~GmeTransmmit()
	{

	}

    int GmeTransmmit::check_msg_header( unsigned char *data,int nDataLen,CmdType cmdType,int number)
	{
		switch(m_subprotocol)
		{
		case BEIJING_JIZHAO_FM:
		case BEIJING_JIZHAO_DIGTV:
		case BEIJING_JIZHAO_SIMTV:
		case GME_SIMTV1014A:
		case GME_SIMTV_MWAVE:
		case GME_FEN_MWAVE3KW:
		case GME_MMAVE1033_3KW:
		case GME_SIMTV:
			{
				time_t tNow = time(0);
                tm *tmNow = localtime(&tNow);
				if((data[0]==m_addresscode || data[0]==0xff) && 
                    data[1]>=0 && data[2]<=Dec2Hex(tmNow->tm_mon+1) &&  data[2]>0 &&
					data[3]<=0x31 && data[3]>0 && data[4]<0x24 && 
					data[4]>=0 && data[5]<0x60 && data[5]>=0 && 
					data[6]<0x60 && data[6]>=0)
				{
                    return RE_SUCCESS;
				}
				else
				{//查找日期+时间头，若找到返回第一个字节位置

					unsigned char cDes[3]={0};
                    cDes[0]=Dec2Hex((tmNow->tm_year+1900)%100);
                    cDes[1]=Dec2Hex(tmNow->tm_mon+1);
                    cDes[2]=Dec2Hex(tmNow->tm_mday);
					int nPos = kmp(data,nDataLen,cDes,3);
					if(nPos>0)
						return nPos-1;
					else
						return nPos;
				}
			}
			break;
		case GME_1F33E:
		case GME_DIGTV:
		case GME_DIGTV_6:
        case GME_CDR_1KW:
			{


				if(nDataLen<2)
                    return RE_HEADERROR;
                if(data[0]==0xEA)
				{//整机功率，则计算数据体长度
					int niDataLen, HiByte,LoByte;
					HiByte=data[2];
					LoByte=data[3];
					niDataLen=HiByte<<8|LoByte;

                    return (niDataLen+1);
				}
				else
				{
                    return RE_HEADERROR;
				}

			}
			break;
		}
		return RE_NOPROTOCOL;
	}

	int GmeTransmmit::decode_msg_body( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate )
	{
        switch(m_subprotocol)
        {
        case BEIJING_JIZHAO_FM:
            return GmeFMData(data,data_ptr,nDataLen,runstate);
        case BEIJING_JIZHAO_SIMTV:
            return GmeSIMData(data,data_ptr,nDataLen,runstate);
        case GME_SIMTV1014A:
            return GmeSIM1014AData(data,data_ptr,nDataLen,runstate);
        case GME_1F33E:
            return Gme1F33E(data,data_ptr,nDataLen,runstate);
        case GME_DIGTV:
            return GmeDigtv(data,data_ptr,nDataLen,runstate);
        case GME_DIGTV_6:
            return GmeDigtv6(data,data_ptr,nDataLen,runstate);
        case GME_CDR_1KW:
            return GmeCDR1Kw(data,data_ptr,nDataLen,runstate);
        }
        return RE_NOPROTOCOL;
	}

	bool GmeTransmmit::IsStandardCommand()
	{
		switch(m_subprotocol)
		{
		case GME_1F33E:
		case GME_DIGTV:
		case GME_DIGTV_6:
        case GME_CDR_1KW:
			return true;
		}
		return false;
	}

	void GmeTransmmit::GetSignalCommand( devCommdMsgPtr lpParam,CommandUnit &cmdUnit )
	{

	}

	void GmeTransmmit::GetAllCmd( CommandAttribute &cmdAll )
	{
		switch(m_subprotocol)
		{
		case BEIJING_JIZHAO_FM:
		case BEIJING_JIZHAO_DIGTV:
		case BEIJING_JIZHAO_SIMTV:
		case GME_SIMTV1014A:
		case GME_SIMTV_MWAVE:
		case GME_FEN_MWAVE3KW:
		case GME_MMAVE1033_3KW:
		case GME_SIMTV:
			{
				CommandUnit tmUnit;
				tmUnit.commandId[0] = m_addresscode;
				tmUnit.commandId[1] = 0x27;
				tmUnit.commandId[2] = 0x00;
				tmUnit.commandId[3] = 0x00;
				tmUnit.commandId[4] = tmUnit.commandId[0]^tmUnit.commandId[1];
				tmUnit.commandLen = 5;
                tmUnit.ackLen = 1026;
				vector<CommandUnit> vtUnit;
				vtUnit.push_back(tmUnit);
				cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
			
				tmUnit.commandId[1] = 0x3C;
				tmUnit.commandId[4] = tmUnit.commandId[0]^tmUnit.commandId[1];
				tmUnit.ackLen = 0;
				vector<CommandUnit> vtTurnOnUnit;
				vtTurnOnUnit.push_back(tmUnit);
				cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtTurnOnUnit;

				tmUnit.commandId[1] = 0x3E;
				tmUnit.commandId[4] = tmUnit.commandId[0]^tmUnit.commandId[1];
				vector<CommandUnit> vtTurnOffUnit;
				vtTurnOffUnit.push_back(tmUnit);
				cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnOffUnit;
			}
			break;
		case GME_1F33E:
		case GME_DIGTV:
		case GME_DIGTV_6:
        case GME_CDR_1KW:
			{
				CommandUnit tmUnit;
				tmUnit.commandId[0] = 0xD5;
				tmUnit.commandId[1] = 0x02;
				tmUnit.commandId[2] = 0x00;
				tmUnit.commandId[3] = 0x00;
				tmUnit.commandId[4] = 0x02;
				tmUnit.commandLen = 5;
				tmUnit.ackLen = 4;
				vector<CommandUnit> vtUnit;
				vtUnit.push_back(tmUnit);
				cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;

				tmUnit.commandId[1] = 0x85;
				tmUnit.commandId[4] = 0x85;
                tmUnit.ackLen = 4;
				vector<CommandUnit> vtTurnOnUnit;
				vtTurnOnUnit.push_back(tmUnit);
				cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtTurnOnUnit;

				tmUnit.commandId[1] = 0x8A;
				tmUnit.commandId[4] = 0x8A;
				vector<CommandUnit> vtTurnOffUnit;
				vtTurnOffUnit.push_back(tmUnit);
				cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnOffUnit;
			}
			break;
		}
	}

	int GmeTransmmit::GmeFMData( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate )
	{
		int indexpos =0; //存放第一个变量的位置
		int HiByte;  //高字节 
		int LoByte;  //低字节
		int tmp;
		float ftmp;
		if(data[0]==m_addresscode)
			data = data+1;
		DataInfo dtinfo;
		dtinfo.bType = false;
		HiByte = data[632];
		LoByte = data[633];
		tmp = ((HiByte <<8 ) | LoByte); 
		dtinfo.fValue = tmp * (float) 0.01;
		data_ptr->mValues[indexpos++] = dtinfo;
		HiByte = data[634];
		LoByte = data[635];
		tmp = ((HiByte <<8 ) | LoByte);
		dtinfo.fValue  = tmp * (float)0.1; 
		data_ptr->mValues[indexpos++] = dtinfo;
		HiByte = data[648];
		LoByte = data[649];
		tmp = ((HiByte <<8 ) | LoByte);
		dtinfo.fValue = tmp * (float)0.01;
		data_ptr->mValues[indexpos++] = dtinfo;
        unsigned char byt = data[6];
		dtinfo.bType = true;
		for(int i=0; i<5; i++)
		{   
			tmp = Getbit(byt,i);  //在线状态 <------->  无音频自动切换   

			dtinfo.fValue  =(float)((tmp == 0) ? 1:0);
			data_ptr->mValues[indexpos++] = dtinfo;
		}
		//RVR激励器1数据
		byt = data[0x16];
		for(int i =0 ;i <6;i++)
		{
			

			tmp =Getbit(byt,i);

			if(i ==3 || i == 4)// General ,Unlock采用的电平与其他不同
				dtinfo.fValue =(float)((tmp == 0) ? 1:0);  // 0 代表故障， 1 代表正常
			else
				dtinfo.fValue =(float)((tmp == 0) ? 0:1);
			data_ptr->mValues[indexpos++] = dtinfo;
		}
		byt = data[0x17];
		for(int i =0 ;i <7;i++)
		{
			tmp =Getbit(byt,i);
			if(i == 0)  //激励器音频 1:有，0：无
				dtinfo.fValue =(float)((tmp == 1) ? 1:0);
			else
				dtinfo.fValue =(float)((tmp == 0) ? 1:0);
			data_ptr->mValues[indexpos++] = dtinfo;
		}
		//调频发射机器RVR模拟量的存储顺序与常规相反
		dtinfo.bType = false;
		for(int i=0 ;i < 7;i++)    //左声道输入电平   --------  反射功率
		{
			LoByte = data[24+i*2];
			HiByte = data[24+i*2+1];
			int tmp = ( (HiByte *256) + LoByte);
			float ftmp = (float)tmp;
			if(i == 2)
				ftmp =(float)tmp *(float)0.01;  //功率设定值, 输出功率
			else if(i == 5 )
				ftmp = (float)tmp * (float)0.1;
			else if(i == 4)
				ftmp =(float)tmp * (float)0.01; //频率
			else if(i == 6)
				ftmp =(float)tmp * (float)0.01;
			dtinfo.fValue  = ftmp;
			data_ptr->mValues[indexpos++] = dtinfo;
		}
		//激励器2数据
		byt = data[0x26];   //工作状态 onAir <------->SWR
		dtinfo.bType = true;
		for(int i =0 ;i <6;i++)
		{
			tmp =Getbit(byt,i);
			if(i == 3 || i == 4)// General ,Unlock采用的电平与其他不同
				dtinfo.fValue = (float)((tmp == 0) ? 0:1);  // 0 代表故障， 1 代表正常
			else
				dtinfo.fValue = (float)((tmp == 0) ? 1:0);
			data_ptr->mValues[indexpos++] = dtinfo;
		}
		byt = data[0x27]; // EX1ST2, 参数设置   ,用0代表有，用1代表无
		for(int i =0 ;i <7;i++)
		{
			tmp =Getbit(byt,i);
			if(i == 0)  //激励器音频 1:有，0：无
				dtinfo.fValue = (float)((tmp == 1) ? 1:0);
			else
				dtinfo.fValue = (float)((tmp == 0) ? 1:0);
			data_ptr->mValues[indexpos++] = dtinfo;
		}
		dtinfo.bType = false;
		for(int i=0 ;i < 7;i++)    //左声道输入电平   --------  反射功率
		{
			LoByte = data[0x28+i*2];//0x18 + i];
			HiByte = data[0x28+i*2+1];//0x18 + i+1];
			//int tmp = ( (HiByte <<8 ) | LoByte);
			int tmp = ( (HiByte *256) + LoByte);
			float ftmp = (float)tmp;
			if(i == 2)
				ftmp =tmp *(float)0.01;  //功率设定值, 输出功率
			else if(i == 5 )
				ftmp = (float)(tmp * 0.1);
			else if(i == 4)
				ftmp =tmp * (float)0.01; //频率
			else if(i == 6)
				ftmp =tmp * (float)0.01;
			dtinfo.fValue = ftmp;
			data_ptr->mValues[indexpos++] = dtinfo;
		}
		dtinfo.bType = false;
		// 功放数据,每个功率占有17B
		for(int i =0 ;i <20 ;i++)  //最多只有20个功放， 在功放中，我们只取输入和输出功率
		{
			HiByte = data[256 + 17 *i +6];//取功放输入高字节
			LoByte = data[256 + 17 *i +6 +1]; //取功放输入低字节
			int tmp = ((HiByte <<8 ) | LoByte);
			float ftmp = (float)tmp;
			ftmp *= (float)0.01;
			dtinfo.fValue = ftmp;
			data_ptr->mValues[indexpos++] = dtinfo;
			HiByte = data[256 + 17 *i +8];//取功放输出高字节
			LoByte = data[256 + 17 *i +8 +1]; //取功放输出低字节
			tmp = ((HiByte <<8 ) | LoByte);
			dtinfo.fValue =(float)tmp;
			data_ptr->mValues[indexpos++] = dtinfo;
		}
		//电源及冷却系统,基地址为 596
		byt = data[596];
		dtinfo.bType = true;
		for(int i=1;i <4; i++)
		{
			tmp = Getbit(byt,i);
			dtinfo.fValue = (float)((tmp == 0) ? 1:0);
			data_ptr->mValues[indexpos++] = dtinfo;
		}
		dtinfo.bType = false;
		//开关电源电流
		HiByte = data[597];
		LoByte = data[598];
		tmp = ((HiByte <<8 ) | LoByte);
		ftmp = (float)tmp; 
		dtinfo.fValue = ftmp;
		//电源过热 ---- 交流欠压
		byt = data[612];
		dtinfo.bType = true;
		for(int i=7; i>4;i--)
		{
			tmp = Getbit(byt,i);
			dtinfo.fValue = (float)((tmp == 0) ? 1:0);
			data_ptr->mValues[indexpos++] = dtinfo;
		}
        return RE_SUCCESS;
	}

	int GmeTransmmit::GmeSIMData( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate )
	{

		int indexpos =0; //存放第一个变量的位置
		int HiByte;  //高字节 
		int LoByte;  //低字节
		int tmp;
		float ftmp;
		if(data[0]==m_addresscode)
			data = data+1;
		DataInfo dtinfo;
		dtinfo.bType = false;
		HiByte = data[6];
		LoByte = data[7];
		tmp = ((HiByte <<8 ) | LoByte);
		dtinfo.fValue = ((float)tmp) *(float)(0.01);
		data_ptr->mValues[indexpos++] = dtinfo;
		float outpwr = (float)tmp;
		HiByte = data[8];
		LoByte = data[9];
		tmp = ((HiByte <<8 ) | LoByte);
		ftmp = tmp *(float)0.01;
		float refpwr =0;
		dtinfo.fValue = ((ftmp -1)/(ftmp +1))*((ftmp -1)/(ftmp +1)) * outpwr * (float)10;
		data_ptr->mValues[indexpos++] = dtinfo;

		dtinfo.fValue = ftmp;
		data_ptr->mValues[indexpos++] = dtinfo;

		LoByte = data[31];
		dtinfo.bType = true;
		for(int i = 2;i <8; i++)   //功放电源状态 ---- 电源及风机状况
		{
			tmp =Getbit(LoByte,i);
			if(i ==3 || i == 6 || i== 7)
			{
				dtinfo.fValue = (float)((tmp == 1) ? 0:1);
			}
			else
				dtinfo.fValue = (float)(tmp);
			data_ptr->mValues[indexpos++] = dtinfo;
		}
		LoByte = data[106];
		for(int i =7; i>=0; i--)
		{
			tmp =Getbit(LoByte,i);
			dtinfo.fValue = (float)((tmp == 1) ? 0:1);
			data_ptr->mValues[indexpos++] = dtinfo;
		}
		dtinfo.bType = false;
		for(int i= 0;i<5;i++)
		{
			HiByte = data[107 + 2 *i];
			LoByte = data[107 + 2*i +1];
			tmp = ((HiByte <<8 ) | LoByte);
			if(i == 1)
				dtinfo.fValue = (float)tmp;
			else 
				dtinfo.fValue = ftmp *(float)0.1;
			data_ptr->mValues[indexpos++] = dtinfo;
		}
	
		dtinfo.bType = true;
		LoByte = data[117];
		for(int i =7; i>=0; i--)
		{
			tmp =Getbit(LoByte,i);
			dtinfo.fValue = (float)((tmp == 1) ? 0:1);
			data_ptr->mValues[indexpos++] = dtinfo;
		}
		dtinfo.bType = false;
		for(int i= 0;i<5;i++)
		{
			HiByte = data[118 + 2 *i];
			LoByte = data[118 + 2*i +1];
			tmp = ((HiByte <<8 ) | LoByte);
			if(i == 1)
				dtinfo.fValue = (float)tmp;
			else 
				dtinfo.fValue = ftmp *(float)0.1;
			data_ptr->mValues[indexpos++] = dtinfo;
		}
		int gnum=0;
		gnum = (int)(nDataLen-128)/32;
		if(gnum>20)
			gnum = 20;
		for(int i =0 ;i <gnum ;i++)
		{
			HiByte = data[128 + 32 *i +6*2];//取功放输入高字节
			LoByte = data[128 + 32 *i +6*2 +1]; //取功放输入低字节
			int tmp = ((HiByte <<8 ) | LoByte);
			dtinfo.fValue = 0.01*tmp;
			data_ptr->mValues[indexpos++] = dtinfo;
			HiByte = data[128 + 32 *i +7*2];//取功放输出高字节
			LoByte = data[128 + 32 *i +7*2+1]; //取功放输出低字节
			tmp = ((HiByte <<8 ) | LoByte);
			dtinfo.fValue = (float)tmp;
			data_ptr->mValues[indexpos++] = dtinfo;
		}
		for(int k=0;k<12;k++)
		{
			HiByte = data[32 + 2*k];
			LoByte = data[32 + 2*k +1];
			tmp = ((HiByte <<8 ) | LoByte);
			dtinfo.fValue = (float)tmp;
			data_ptr->mValues[indexpos++] = dtinfo;
		}
        return RE_SUCCESS;
    }

    int GmeTransmmit::GmeSIM1014AData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        int indexpos =0; //存放第一个变量的位置
        int HiByte;  //高字节
        int LoByte;  //低字节
        int tmp;
        float ftmp;
        if(data[0]==m_addresscode)
            data = data+1;
        DataInfo dtinfo;
        dtinfo.bType = false;
        HiByte = data[676];//GME 1014A 格式例外
        LoByte = data[677];
        tmp = HiByte*256+LoByte;
        dtinfo.fValue = ((float)tmp) *(float)(0.01);
        data_ptr->mValues[indexpos++] = dtinfo;
        float outpwr = (float)tmp;
        HiByte = data[678];
        LoByte = data[679];
        ftmp = (HiByte*256+LoByte) *0.01;
        float refpwr =0;
        if(ftmp > 1.0)//驻波比小于1
        {
            refpwr = (float)(((ftmp -1))/(ftmp +1) * data_ptr->mValues[0].fValue *30);
        }else
            refpwr =0;
        dtinfo.fValue = refpwr;
        data_ptr->mValues[indexpos++] = dtinfo;
        dtinfo.fValue = ftmp;
        data_ptr->mValues[indexpos++] = dtinfo;
        LoByte = data[31];
        dtinfo.bType = true;
        for(int i = 2;i <8; i++)
        {
            dtinfo.fValue = Getbit(LoByte,i)==1 ? 0:1;
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        LoByte = data[106];
        for(int i=7;i>=0;--i)
        {
            dtinfo.fValue = Getbit(LoByte,i)==1 ? 0:1;
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        dtinfo.bType = false;
        for(int i=0;i<5;++i)
        {
            HiByte = data[107+2*i];
            LoByte = data[108+2*i];
            if(i==1)
            {
                ftmp = HiByte*256+LoByte;
            }
            else
                ftmp = (HiByte*256+LoByte)*0.1;
            dtinfo.fValue = ftmp;
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        dtinfo.bType = true;
        LoByte = data[117];
        for(int i=7;i>=0;--i)
        {
            dtinfo.fValue = Getbit(LoByte,i)==1 ? 0:1;
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        dtinfo.bType = false;
        for(int i=0;i<5;++i)
        {
            HiByte = data[118+2*i];
            LoByte = data[119+2*i];
            if(i==1)
            {
                ftmp = HiByte*256+LoByte;
            }
            else
                ftmp = (HiByte*256+LoByte)*0.1;
            dtinfo.fValue = ftmp;
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        int gnum=0;
        gnum = (int)(nDataLen-128)/32;
        for(int i =0 ;i <gnum ;i++)
        {
            HiByte = data[128 + 32 *i +6*2];//取功放输入高字节
            LoByte = data[128 + 32 *i +6*2 +1]; //取功放输入低字节
            int tmp = ((HiByte <<8 ) | LoByte);
            float ftmp = (float)tmp;
            ftmp *= (float)0.01;
            dtinfo.fValue = ftmp;
            data_ptr->mValues[indexpos++] = dtinfo;
            HiByte = data[128 + 32 *i +7*2];//取功放输出高字节
            LoByte = data[128 + 32 *i +7*2+1]; //取功放输出低字节
            tmp = ((HiByte <<8 ) | LoByte);
            dtinfo.fValue = tmp;
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        for(int k=0;k<12;k++)
        {
            HiByte = data[32 + 2*k];
            LoByte = data[32 + 2*k +1];

            tmp = ((HiByte <<8 ) | LoByte);

            if(tmp >52)
                tmp =48;
            dtinfo.fValue = tmp;
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        return RE_SUCCESS;
    }

    int GmeTransmmit::Gme1F33E(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        if(data[1]!=0x02)
            return RE_CMDACK;
        int iLastNum = nDataLen-4;
        data+=4;
        int iModType;
        DataInfo dtinfo;
        dtinfo.bType = false;
        while(iLastNum>4){
            if(data[0]==0xFC)
                iModType = data[1];
            else
            {
                bool bFind=false;
                for(int i=0;i<(iLastNum-4);i++)
                {
                    if(data[i]==0xFC)
                    {
                        iModType = data[i+1];
                        data+=i;
                        bFind = true;
                        break;
                    }
                }
                if(!bFind)
                    return RE_HEADERROR;
            }
            switch(iModType){
            case 0x02:
            {

                dtinfo.fValue = float(((data[4]<<8)|data[5])*0.001);
                data_ptr->mValues[0] = dtinfo;
                dtinfo.fValue = float((data[6]<<8)|data[7]);
                data_ptr->mValues[1] = dtinfo;
                dtinfo.fValue = float(((data[8]<<8)|data[9])*0.01);
                data_ptr->mValues[2] = dtinfo;
                data+=(data[3]+4);
                iLastNum-=(data[3]+4);
            }
                break;
            case 0x03:
            {
                dtinfo.bType = true;
                if(data[4]==0x21){
                    dtinfo.fValue=0;//通电
                    runstate = 0;
                }
                else{
                    dtinfo.fValue=1;
                    runstate = 1;
                }
                data_ptr->mValues[3] = dtinfo;
                dtinfo.bType = false;
                dtinfo.fValue = float(((data[4]<<8)|data[5])*0.1);
                data_ptr->mValues[4] = dtinfo;
                dtinfo.fValue = float(((data[6]<<8)|data[7])*0.1);
                data_ptr->mValues[5] = dtinfo;
                dtinfo.fValue = float(((data[8]<<8)|data[9])*0.1);
                data_ptr->mValues[6] = dtinfo;
                dtinfo.fValue = 0;
                data_ptr->mValues[7] = dtinfo;
                data+=(data[3]+2);//实际长度少了2个字节，应该是9个字节，实际只有7个字节
                iLastNum-=(data[3]+2);
            }
                break;
            case 0x04://激励数据
            {
                if(data[2]==0x01)//激励1数据
                {
                    dtinfo.fValue = float(((data[4]<<8)|data[5])*0.1);
                    data_ptr->mValues[8] = dtinfo;
                    dtinfo.fValue = float(((data[7]<<8)|data[8])*0.1);
                    data_ptr->mValues[9] = dtinfo;
                    dtinfo.bType = true;
                    dtinfo.fValue = data[9];
                    data_ptr->mValues[10] = dtinfo;
                    dtinfo.bType = false;
                    dtinfo.fValue = float(data[10]+data[11]*0.01+data[12]*0.001);
                    data_ptr->mValues[11] = dtinfo;
                    dtinfo.fValue = float(data[13]+data[14]*0.1);
                    data_ptr->mValues[12] = dtinfo;
                    dtinfo.fValue = float(data[15]+data[16]*0.1);
                    data_ptr->mValues[13] = dtinfo;
                    int iMinus;//数据符号
                    iMinus = data[18]>>7;
                    if(iMinus==0)
                        dtinfo.fValue = float((data[18]&0x7F)*0.5);
                    else
                        dtinfo.fValue = float(-1*(data[18]&0x7F)*0.5);
                    data_ptr->mValues[14] = dtinfo;
                    iMinus = data[19]>>7;
                    if(iMinus==0)
                        dtinfo.fValue = float((data[19]&0x7F)*0.5);
                    else
                        dtinfo.fValue = float(-1*(data[19]&0x7F)*0.5);
                    data_ptr->mValues[15] = dtinfo;
                    dtinfo.bType = true;
                    dtinfo.fValue = data[28];
                    data_ptr->mValues[16] = dtinfo;
                    dtinfo.bType = false;
                    if(data[29]==0x00)
                        dtinfo.fValue = 0x00;
                    else if(data[29]==0x02)
                        dtinfo.fValue = 50;
                    else if(data[29]==0x03)
                        dtinfo.fValue = 75;
                    data_ptr->mValues[17] = dtinfo;
                    dtinfo.bType = true;
                    dtinfo.fValue = (data[30]==0x01 ? 0:1);
                    data_ptr->mValues[18] = dtinfo;
                    dtinfo.bType = false;
                    iMinus = data[31]>>7;
                    if(iMinus==0)
                        dtinfo.fValue = float((data[31]&0x7F)*0.2);
                    else
                        dtinfo.fValue = float(-1*(data[31]&0x7F)*0.2);
                    data_ptr->mValues[19] = dtinfo;

                    iMinus = data[32]>>7;
                    if(iMinus==0)
                        dtinfo.fValue = float((data[32]&0x7F)*0.2);
                    else
                        dtinfo.fValue = float(-1*(data[32]&0x7F)*0.2);
                    data_ptr->mValues[20] = dtinfo;
                    iMinus = data[33]>>7;
                    if(iMinus==0)
                        dtinfo.fValue = float((data[33]&0x7F)*0.2);
                    else
                        dtinfo.fValue = float(-1*(data[33]&0x7F)*0.2);
                    data_ptr->mValues[21] = dtinfo;
                    dtinfo.bType = true;
                    dtinfo.fValue = data[34];
                    data_ptr->mValues[22] = dtinfo;
                    dtinfo.bType = false;
                    dtinfo.fValue = float((data[39]<<8)|data[40]);
                    data_ptr->mValues[23] = dtinfo;
                    dtinfo.fValue = float(data[43]+data[44]*0.001);
                    data_ptr->mValues[24] = dtinfo;
                    dtinfo.fValue = float(data[50]+data[51]*0.001);
                    data_ptr->mValues[25] = dtinfo;
                    dtinfo.fValue = float(data[52]+data[53]*0.001);
                    data_ptr->mValues[26] = dtinfo;
                    dtinfo.bType = true;
                    iMinus = data[54]&0x01;
                    dtinfo.fValue=(iMinus==0x01 ? 0:1);//b0
                    data_ptr->mValues[27] = dtinfo;
                    dtinfo.fValue = ((data[54]&0x02)>>1);//b1
                    data_ptr->mValues[28] = dtinfo;
                    dtinfo.fValue = ((data[54]&0x04)>>2);//b2
                    data_ptr->mValues[29] = dtinfo;
                    dtinfo.fValue = ((data[54]&0x10)>>4);//b4
                    data_ptr->mValues[30] = dtinfo;
                    dtinfo.fValue = ((data[54]&0x20)>>5);//b5
                    data_ptr->mValues[31] = dtinfo;
                    dtinfo.fValue = ((data[54]&0x80)>>7);//b7
                    data_ptr->mValues[32] = dtinfo;
                    //	data+=(data[3]+4);
                    //	iLastNum-=(data[3]+4);
                }
                else if(data[2]==0x02)//激励2
                {
                    dtinfo.fValue = float(((data[4]<<8)|data[5])*0.1);
                    data_ptr->mValues[33] = dtinfo;
                    dtinfo.fValue = float(((data[7]<<8)|data[8])*0.1);
                    data_ptr->mValues[34] = dtinfo;
                    dtinfo.bType = true;
                    dtinfo.fValue = data[9];
                    data_ptr->mValues[35] = dtinfo;
                    dtinfo.bType = false;
                    dtinfo.fValue = float(data[10]+data[11]*0.01+data[12]*0.001);
                    data_ptr->mValues[36] = dtinfo;
                    dtinfo.fValue = float(data[13]+data[14]*0.1);
                    data_ptr->mValues[37] = dtinfo;
                    dtinfo.fValue = float(data[15]+data[16]*0.1);
                    data_ptr->mValues[38] = dtinfo;
                    int iMinus;//数据符号
                    iMinus = data[18]>>7;
                    if(iMinus==0)
                        dtinfo.fValue = float((data[18]&0x7F)*0.5);
                    else
                        dtinfo.fValue = float(-1*(data[18]&0x7F)*0.5);
                    data_ptr->mValues[39] = dtinfo;
                    iMinus = data[19]>>7;
                    if(iMinus==0)
                        dtinfo.fValue = float((data[19]&0x7F)*0.5);
                    else
                        dtinfo.fValue = float(-1*(data[19]&0x7F)*0.5);
                    data_ptr->mValues[40] = dtinfo;
                    dtinfo.bType = true;
                    dtinfo.fValue = data[28];
                    data_ptr->mValues[41] = dtinfo;
                    dtinfo.bType = false;
                    if(data[29]==0x00)
                        dtinfo.fValue = 0x00;
                    else if(data[29]==0x02)
                        dtinfo.fValue = 50;
                    else if(data[29]==0x03)
                        dtinfo.fValue = 75;
                    data_ptr->mValues[42] = dtinfo;
                    dtinfo.bType = true;
                    dtinfo.fValue = (data[30]==0x01 ? 0:1);
                    data_ptr->mValues[43] = dtinfo;
                    dtinfo.bType = false;
                    iMinus = data[31]>>7;
                    if(iMinus==0)
                        dtinfo.fValue = float((data[31]&0x7F)*0.2);
                    else
                        dtinfo.fValue = float(-1*(data[31]&0x7F)*0.2);
                    data_ptr->mValues[44] = dtinfo;

                    iMinus = data[32]>>7;
                    if(iMinus==0)
                        dtinfo.fValue = float((data[32]&0x7F)*0.2);
                    else
                        dtinfo.fValue = float(-1*(data[32]&0x7F)*0.2);
                    data_ptr->mValues[45] = dtinfo;
                    iMinus = data[33]>>7;
                    if(iMinus==0)
                        dtinfo.fValue = float((data[33]&0x7F)*0.2);
                    else
                        dtinfo.fValue = float(-1*(data[33]&0x7F)*0.2);
                    data_ptr->mValues[46] = dtinfo;
                    dtinfo.bType = true;
                    dtinfo.fValue = data[34];
                    data_ptr->mValues[47] = dtinfo;
                    dtinfo.bType = false;
                    dtinfo.fValue = float((data[39]<<8)|data[40]);
                    data_ptr->mValues[48] = dtinfo;
                    dtinfo.fValue = float(data[43]+data[44]*0.001);
                    data_ptr->mValues[49] = dtinfo;
                    dtinfo.fValue = float(data[50]+data[51]*0.001);
                    data_ptr->mValues[50] = dtinfo;
                    dtinfo.fValue = float(data[52]+data[53]*0.001);
                    data_ptr->mValues[51] = dtinfo;
                    dtinfo.bType = true;
                    iMinus = data[54]&0x01;
                    dtinfo.fValue=(iMinus==0x01 ? 0:1);//b0
                    data_ptr->mValues[52] = dtinfo;
                    dtinfo.fValue = ((data[54]&0x02)>>1);//b1
                    data_ptr->mValues[53] = dtinfo;
                    dtinfo.fValue = ((data[54]&0x04)>>2);//b2
                    data_ptr->mValues[54] = dtinfo;
                    dtinfo.fValue = ((data[54]&0x10)>>4);//b4
                    data_ptr->mValues[55] = dtinfo;
                    dtinfo.fValue = ((data[54]&0x20)>>5);//b5
                    data_ptr->mValues[56] = dtinfo;
                    dtinfo.fValue = ((data[54]&0x80)>>7);//b7
                    data_ptr->mValues[57] = dtinfo;

                }
                data+=(data[3]+4);
                iLastNum-=(data[3]+4);
            }

                break;
            case 0x07://功放数据
            {
                if(data[2]==0x01)//功放1
                {
                    dtinfo.bType = false;
                    dtinfo.fValue = float(((data[4]<<8)|data[5])*0.1);
                    data_ptr->mValues[58] = dtinfo;
                    dtinfo.fValue = float(((data[6]<<8)|data[7])*0.1);
                    data_ptr->mValues[59] = dtinfo;
                    dtinfo.fValue = float(((data[8]<<8)|data[9])*0.1);
                    data_ptr->mValues[60] = dtinfo;
                    dtinfo.fValue = float(((data[18]<<8)|data[19]));
                    data_ptr->mValues[61] = dtinfo;
                    dtinfo.bType = true;
                    dtinfo.fValue = float((data[23]&0x02)>>1);
                    data_ptr->mValues[62] = dtinfo;
                    dtinfo.fValue = float((data[23]&0x04)>>2);
                    data_ptr->mValues[63] = dtinfo;
                    dtinfo.fValue = float((data[23]&0x10)>>4);
                    data_ptr->mValues[64] = dtinfo;
                }
                else if(data[2]==0x02)//功放2
                {
                    dtinfo.bType = false;
                    dtinfo.fValue = float(((data[4]<<8)|data[5])*0.1);
                    data_ptr->mValues[65] = dtinfo;
                    dtinfo.fValue = float(((data[6]<<8)|data[7])*0.1);
                    data_ptr->mValues[66] = dtinfo;
                    dtinfo.fValue = float(((data[8]<<8)|data[9])*0.1);
                    data_ptr->mValues[67] = dtinfo;
                    dtinfo.fValue = float(((data[18]<<8)|data[19]));
                    data_ptr->mValues[68] = dtinfo;
                    dtinfo.bType = true;
                    dtinfo.fValue = float((data[23]&0x02)>>1);
                    data_ptr->mValues[69] = dtinfo;
                    dtinfo.fValue = float((data[23]&0x04)>>2);
                    data_ptr->mValues[70] = dtinfo;
                    dtinfo.fValue = float((data[23]&0x10)>>4);
                    data_ptr->mValues[71] = dtinfo;
                }
                data+=(data[3]+4);
                iLastNum-=(data[3]+4);
            }
                break;
            default:
                data+=(data[3]+4);
                iLastNum-=(data[3]+4);
                break;
            }
        }
        return RE_SUCCESS;
    }

    int GmeTransmmit::GmeDigtv(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        int iLastNum = nDataLen-4;//剩余解析字节数
        data+=4;
        int iModType;//数据模块类型
        DataInfo dtinfo;
        dtinfo.bType = false;
        while(iLastNum>4)
        {
            if(data[0]==0xFC)
                iModType = data[1];
            else
            {
                bool bFind=false;
                for(int i=0;i<(iLastNum-4);i++)
                {
                    if(data[i]==0xFC)
                    {
                        iModType = data[i+1];
                        data+=i;
                        bFind = true;
                        break;
                    }
                }
                if(!bFind)
                   break;
            }
            switch(iModType)
            {
            case 0x02:
            {
                dtinfo.fValue = float(((data[4]<<8)|data[5])*0.001);
                data_ptr->mValues[0] = dtinfo;
                dtinfo.fValue = float((data[6]<<8)|data[7]);
                data_ptr->mValues[1] = dtinfo;
                dtinfo.fValue = float(((data[8]<<8)|data[9])*0.01);
                data_ptr->mValues[2] = dtinfo;
                data+=(data[3]+4);
                iLastNum-=(data[3]+4);
            }
                break;
            case 0x03:
            {
                dtinfo.bType = true;
                if(data[4]==0x21){
                    dtinfo.fValue=0;//通电
                    runstate = 0;
                }
                else{
                    dtinfo.fValue=1;
                    runstate = 1;
                }
                data_ptr->mValues[3] = dtinfo;
                data+=5;//实际长度少了2个字节，应该是9个字节，实际只有7个字节
                iLastNum-=5;
            }
                break;
            case 0x04://激励数据
            {
                int index = -1;
                if(data[2]==0x01)//激励1数据
                {
                    index = 4;
                }
                else if(data[2]==0x02)//激励2
                {
                    index = 38;
                }
                if(index<=0)
                {
                    data+=(data[3]+4);
                    iLastNum-=(data[3]+4);
                    break;
                }
                for(int i=0;i<8;++i)
                {
                    dtinfo.bType = true;
                    dtinfo.fValue = Getbit(data[4],i);
                    data_ptr->mValues[index++] = dtinfo;
                }
                dtinfo.bType = false;
                dtinfo.fValue = (data[5]<<8|data[6])*0.1;
                data_ptr->mValues[index++] = dtinfo;
                for(int i=3;i<8;++i)
                {
                    dtinfo.bType = true;
                    dtinfo.fValue = Getbit(data[7],i);
                    data_ptr->mValues[index++] = dtinfo;
                }
                dtinfo.bType = false;
                dtinfo.fValue = (data[11]<<8|data[12])*0.001;
                data_ptr->mValues[index++] = dtinfo;
                dtinfo.bType = false;
                dtinfo.fValue = (data[13]<<8|data[14])*0.001;
                data_ptr->mValues[index++] = dtinfo;
                for(int i=0;i<6;++i)
                {
                    dtinfo.fValue = (data[15+i*2]<<8|data[16+i*2])*0.001;
                    data_ptr->mValues[index++] = dtinfo;
                }
                dtinfo.bType = true;
                for(int i=0;i<8;++i)
                {
                    dtinfo.fValue = Getbit(data[61],i);
                    data_ptr->mValues[index++] = dtinfo;
                }
                for(int i=4;i<8;++i)
                {
                    dtinfo.fValue = Getbit(data[62],i);
                    data_ptr->mValues[index++] = dtinfo;
                }
                iLastNum-=(data[3]+4);
                data+=(data[3]+4);
            }

                break;
            case 0x07://功放数据
            {
                int index = -1;
                switch(data[2])
                {
                case 0x01:
                    index = 72;
                    break;
                case 0x02:
                    index = 87;
                    break;
                case 0x03:
                    index = 102;
                    break;
                case 0x04:
                    index = 117;
                    break;
                default:
                    break;
                }
                if(index>71)
                {
                    dtinfo.bType = false;
                    for(int i=0;i<9;++i)
                    {
                        dtinfo.fValue = (data[4+i*2]<<8|data[5+i*2])*0.1;
                        data_ptr->mValues[index++] = dtinfo;
                    }
                    dtinfo.bType = true;
                    for(int i=5;i>1;--i)
                    {
                        dtinfo.fValue = Getbit(data[22],i);
                        data_ptr->mValues[index++] = dtinfo;
                    }
                    dtinfo.fValue = ((data[22]<<8|data[23])>>5&0x1F);
                    data_ptr->mValues[index++] = dtinfo;
                    dtinfo.fValue = (data[23]&0x1F);
                    data_ptr->mValues[index++] = dtinfo;
                }
                iLastNum-=(data[3]+8);
                data+=(data[3]+8);
            }
                break;
            default:
                iLastNum-=(data[3]+4);
                data+=(data[3]+4);
                break;
            }
        }
        if(data_ptr->mValues[3].fValue==1)
        {
            data_ptr->mValues[0].fValue = 0;
            data_ptr->mValues[1].fValue = 0;
        }
        return RE_SUCCESS;
    }

    int GmeTransmmit::GmeDigtv6(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        int iLastNum = nDataLen-4;//剩余解析字节数
        data+=4;
        int iModType;//数据模块类型
        DataInfo dtinfo;
        dtinfo.bType = false;
        while(iLastNum>4)
        {
            if(data[0]==0xFC)
                iModType = data[1];
            else
            {
                bool bFind=false;
                for(int i=0;i<(iLastNum-4);i++)
                {
                    if(data[i]==0xFC)
                    {
                        iModType = data[i+1];
                        data+=i;
                        bFind = true;
                        break;
                    }
                }
                if(!bFind)
                    break;
            }
            switch(iModType)
            {
            case 0x02:
            {
                dtinfo.fValue = float(((data[4]<<8)|data[5])*0.001);
                data_ptr->mValues[0] = dtinfo;
                dtinfo.fValue = float((data[6]<<8)|data[7]);
                data_ptr->mValues[1] = dtinfo;
                dtinfo.fValue = float(((data[8]<<8)|data[9])*0.01);
                data_ptr->mValues[2] = dtinfo;
                data+=(data[3]+4);
                iLastNum-=(data[3]+4);
            }
                break;
            case 0x03:
            {
                dtinfo.bType = true;
                if(data[4]==0x21){
                    dtinfo.fValue=0;//通电
                    runstate = 0;
                }
                else{
                    dtinfo.fValue=1;
                    runstate = 1;
                }
                data_ptr->mValues[3] = dtinfo;
                data+=5;//实际长度少了2个字节，应该是9个字节，实际只有7个字节
                iLastNum-=5;
            }
                break;
            case 0x04://激励数据
            {
                int index = -1;
                if(data[2]==0x01)//激励1数据
                {
                    index = 4;
                }
                else if(data[2]==0x02)//激励2
                {
                    index = 38;
                }
                if(index<=0)
                {
                    data+=(data[3]+4);
                    iLastNum-=(data[3]+4);
                    break;
                }
                for(int i=0;i<8;++i)
                {
                    dtinfo.bType = true;
                    dtinfo.fValue = Getbit(data[4],i);
                    data_ptr->mValues[index++] = dtinfo;
                }
                dtinfo.bType = false;
                dtinfo.fValue = (data[5]<<8|data[6])*0.1;
                data_ptr->mValues[index++] = dtinfo;
                for(int i=3;i<8;++i)
                {
                    dtinfo.bType = true;
                    dtinfo.fValue = Getbit(data[7],i);
                    data_ptr->mValues[index++] = dtinfo;
                }
                dtinfo.bType = false;
                dtinfo.fValue = (data[11]<<8|data[12])*0.001;
                data_ptr->mValues[index++] = dtinfo;
                dtinfo.bType = false;
                dtinfo.fValue = (data[13]<<8|data[14])*0.001;
                data_ptr->mValues[index++] = dtinfo;
                for(int i=0;i<6;++i)
                {
                    dtinfo.fValue = (data[15+i*2]<<8|data[16+i*2])*0.001;
                    data_ptr->mValues[index++] = dtinfo;
                }
                dtinfo.bType = true;
                for(int i=0;i<8;++i)
                {
                    dtinfo.fValue = Getbit(data[61],i);
                    data_ptr->mValues[index++] = dtinfo;
                }
                for(int i=4;i<8;++i)
                {
                    dtinfo.fValue = Getbit(data[62],i);
                    data_ptr->mValues[index++] = dtinfo;
                }
                iLastNum-=(data[3]+4);
                data+=(data[3]+4);
            }

                break;
            case 0x07://功放数据
            {
                int index = -1;
                switch(data[2])
                {
                case 0x01:
                    index = 72;
                    break;
                case 0x02:
                    index = 88;
                    break;
                case 0x03:
                    index = 104;
                    break;
                case 0x04:
                    index = 120;
                    break;
                default:
                    break;
                }
                if(index>71)
                {
                    dtinfo.bType = false;
                    for(int i=0;i<6;++i)
                    {
                        dtinfo.fValue = (data[4+i*2]<<8|data[5+i*2])*0.1;
                        data_ptr->mValues[index++] = dtinfo;
                    }
                    dtinfo.fValue = (data[18]<<8|data[19])*0.1;
                    data_ptr->mValues[index++] = dtinfo;
                    dtinfo.fValue = (data[20]<<8|data[21]);
                    data_ptr->mValues[index++] = dtinfo;
                    dtinfo.fValue = (data[22]<<8|data[23])*0.1;
                    data_ptr->mValues[index++] = dtinfo;
                    dtinfo.fValue = (data[24]<<8|data[25])*0.1;
                    data_ptr->mValues[index++] = dtinfo;
                    dtinfo.bType = true;
                    for(int i=5;i>1;--i)
                    {
                        dtinfo.fValue = Getbit(data[26],i);
                        data_ptr->mValues[index++] = dtinfo;
                    }
                    dtinfo.fValue = ((data[26]<<8|data[27])>>5&0x1F);
                    data_ptr->mValues[index++] = dtinfo;
                    dtinfo.fValue = (data[28]&0x1F);
                    data_ptr->mValues[index++] = dtinfo;
                }
                iLastNum-=(data[3]+8);
                data+=(data[3]+8);
            }
                break;
            default:
                iLastNum-=(data[3]+4);
                data+=(data[3]+4);
                break;
            }
        }
        if(data_ptr->mValues[3].fValue==1)
        {
            data_ptr->mValues[0].fValue = 0;
            data_ptr->mValues[1].fValue = 0;
        }
        return RE_SUCCESS;
    }

    int GmeTransmmit::GmeCDR1Kw(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
        {
            int iLastNum = nDataLen-4;//剩余解析字节数
            data+=4;
            int iModType;//数据模块类型
            DataInfo dtinfo;
            dtinfo.bType = false;
            while(iLastNum>4)
            {
                if(data[0]==0xFC)
                    iModType = data[1];
                else
                {
                    bool bFind=false;
                    for(int i=0;i<(iLastNum-4);i++)
                    {
                        if(data[i]==0xFC)
                        {
                            iModType = data[i+1];
                            data+=i;
                            bFind = true;
                            break;
                        }
                    }
                    if(!bFind)
                        break;
                }
                switch(iModType){
                case 0x02:
                {

                    dtinfo.fValue = float(((data[4]<<8)|data[5])*0.001);
                    data_ptr->mValues[0] = dtinfo;
                    dtinfo.fValue = float((data[6]<<8)|data[7]);
                    data_ptr->mValues[1] = dtinfo;
                    dtinfo.fValue = float(((data[8]<<8)|data[9])*0.01);
                    data_ptr->mValues[2] = dtinfo;
                    data+=(data[3]+4);
                    iLastNum-=(data[3]+4);
                }
                    break;
                case 0x03:
                {
                    dtinfo.bType = true;
                    if(data[4]==0x21){
                        dtinfo.fValue=0;//通电
                        runstate = 0;
                    }
                    else{
                        dtinfo.fValue=1;
                        runstate = 1;
                    }
                    data_ptr->mValues[3] = dtinfo;
                    dtinfo.bType = false;
                    dtinfo.fValue = float(((data[5]<<8)|data[6])*0.1);
                    data_ptr->mValues[4] = dtinfo;
                    dtinfo.fValue = float(((data[7]<<8)|data[8])*0.1);
                    data_ptr->mValues[5] = dtinfo;
                    dtinfo.fValue = float(((data[9]<<8)|data[10])*0.1);
                    data_ptr->mValues[6] = dtinfo;
                    dtinfo.fValue = dtinfo.fValue = float(((data[11]<<8)|data[12])*0.1);;
                    data_ptr->mValues[7] = dtinfo;
                    data+=(data[3]+4);
                    iLastNum-=(data[3]+4);
                }
                    break;
                case 0x07://功放数据
                {
                    int index = -1;
                    switch(data[2])
                    {
                    case 0x01:
                        index = 8;
                        break;
                    case 0x02:
                        index = 20;
                        break;
                    default:
                        break;
                    }
                    if(index>=8)
                    {
                        dtinfo.bType = false;
                        for(int i=0;i<6;++i)
                        {
                            dtinfo.fValue = float((data[4+2*i]*256+data[5+2*i])*0.1);
                            data_ptr->mValues[index++] = dtinfo;
                        }
                        dtinfo.fValue = float((data[16]*256+data[17]));
                        data_ptr->mValues[index++] = dtinfo;
                        dtinfo.fValue = float((data[18]*256+data[19]));
                        data_ptr->mValues[index++] = dtinfo;
                        dtinfo.fValue = float((data[20]*256+data[21])*0.1);
                        data_ptr->mValues[index++] = dtinfo;
                        dtinfo.bType = true;
                        for(int i=0;i<3;++i)
                        {
                            dtinfo.fValue = Getbit(data[22],i);
                            data_ptr->mValues[index++] = dtinfo;
                        }
                    }
                    data+=(data[3]+4);
                    iLastNum-=(data[3]+4);
                }
                    break;
                }
            }
            if(data_ptr->mValues[3].fValue==1)
            {
                data_ptr->mValues[0].fValue = 0;
                data_ptr->mValues[1].fValue = 0;
            }
            return RE_SUCCESS;
        }
}
