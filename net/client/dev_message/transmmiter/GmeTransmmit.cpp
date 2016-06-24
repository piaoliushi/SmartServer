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
                //char xxx[1024];
                //memset(xxx,0,1024);
                //memcpy(xxx,data,1023);
               // int yyy = Dec2Hex(tmNow->tm_mon) ;
				if((data[0]==m_addresscode || data[0]==0xff) && 
                    data[1]>=0 && data[2]<=Dec2Hex(tmNow->tm_mon+1) &&  data[2]>0 &&
					data[3]<=0x31 && data[3]>0 && data[4]<0x24 && 
					data[4]>=0 && data[5]<0x60 && data[5]>=0 && 
					data[6]<0x60 && data[6]>=0)
				{
					return 0;
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
			{


				if(nDataLen<2)
					return -1;
				if(data[0]==0xEA && data[1]==0x02)
				{//整机功率，则计算数据体长度
					int niDataLen, HiByte,LoByte;
					HiByte=data[2];
					LoByte=data[3];
					niDataLen=HiByte<<8|LoByte;
					if(nDataLen==nDataLen+5)
						return 0;
					return (niDataLen+5-nDataLen);
				}
				else
				{
					unsigned char cDes[2]={0};
					//cDes[0]= m_DevAddr;
					cDes[0]=0xEA;
					cDes[1]=0x02;
					int nPos = kmp(data,nDataLen,cDes,3);
					if(nPos>0)
						return nPos-1;
					else
						return nPos;
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
        }
        return -1;
	}

	bool GmeTransmmit::IsStandardCommand()
	{
		switch(m_subprotocol)
		{
		case GME_1F33E:
		case GME_DIGTV:
		case GME_DIGTV_6:
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
				tmUnit.ackLen = 5;
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
		return 0;
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
		return 0;
	}
}
