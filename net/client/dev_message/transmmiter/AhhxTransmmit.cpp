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
        case HUIXIN_993:{
            if(data[0]==0x55 && data[2]==0xF3)
            {
                if(data[1]< 0x99 )
                    return RE_SUCCESS;
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
                    return RE_SUCCESS;
            }
            else {
                unsigned char cDes[3]={0};
                cDes[0]=0x55;
                cDes[1] = 0x35;
                cDes[2] = 0xF3;
                return kmp(data,nDataLen,cDes,3);
            }
        }
            break;
        case HUIXIN_996:
        case HUIXIN_996_QBEX:
        case HUIXIN_996_HRRISEX:
        {
           if(data[0]!=0x7E || data[4]!=0x30)
                return RE_HEADERROR;
           else
                return int((data[3]<<8)|data[2])-1;
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
        case HUIXIN_996:
            return Md996Data(data,data_ptr,nDataLen,runstate);
        case HUIXIN_996_QBEX:
            return Md996QBData(data,data_ptr,nDataLen,runstate);
        case HUIXIN_996_HRRISEX:
            return Md996HrrisData(data,data_ptr,nDataLen,runstate);
		}
        return RE_NOPROTOCOL;
	}

    bool AhhxTransmmit::IsStandardCommand()
	{
		switch(m_subprotocol)
		{
        case HUIXIN_996:
        case HUIXIN_996_QBEX:
        case HUIXIN_996_HRRISEX:
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




            tmUnit.commandLen = 6;
            /*tmUnit.commandId[0]=0x55;
            tmUnit.commandId[1]=0x03;
            tmUnit.commandId[2]=0xF2;
            tmUnit.commandId[3]=0x00;//脉冲
            tmUnit.commandId[4]=0x01;
            tmUnit.commandId[5]=0xF3;
            tmUnit.commandId[6]=0x00;*/

            tmUnit.commandId[0]=0x55;
            tmUnit.commandId[1]=0x02;
            tmUnit.commandId[2]=0xFA;
            tmUnit.commandId[3]=0x01;//电平
            tmUnit.commandId[4]=0xFB;
            tmUnit.commandId[5]=0x00;




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


            tmUnit.commandId[1]=0x03;
            tmUnit.commandId[2]=0xFA;
            tmUnit.commandId[3]=0x00;
            tmUnit.commandId[4]=0xFA;//电平
            tmUnit.commandId[5]=0x00;

            //tmUnit.commandId[3]=0x01;
            //tmUnit.commandId[5]=0xF2;//脉冲

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
        case HUIXIN_996:
        {
            CommandUnit tmUnit;
            tmUnit.commandLen = 9;
            tmUnit.ackLen = 5;
            tmUnit.commandId[0] = 0x7E;
            tmUnit.commandId[1] = 0x11;
            tmUnit.commandId[2] = 0x05;
            tmUnit.commandId[3] = 0x00;
            tmUnit.commandId[4] = 0x30;
            tmUnit.commandId[5] = (m_addresscode&0x00FF);
            tmUnit.commandId[6] = ((m_addresscode&0xFF00)>>8);
            tmUnit.commandId[7] = 0x01;
            tmUnit.commandId[8] = 0x55;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandLen = 10;
            tmUnit.commandId[1] = 0x77;
            tmUnit.commandId[2] = 0x06;
            tmUnit.commandId[8] = 0x01;
            tmUnit.commandId[9] = 0x55;
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
            tmUnit.commandId[8] = 0x02;
            cmdAll.mapCommand[MSG_TRANSMITTER_MIDDLE_POWER_TURNON_OPR].push_back(tmUnit);
            tmUnit.commandId[8] = 0x03;
            cmdAll.mapCommand[MSG_TRANSMITTER_LOW_POWER_TURNON_OPR].push_back(tmUnit);
            tmUnit.commandId[8] = 0x00;
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
            tmUnit.commandId[1] = 0x31;
            tmUnit.commandId[8] = 0x01;
            cmdAll.mapCommand[MSG_TRANSMITTER_RISE_POWER_OPR].push_back(tmUnit);
            tmUnit.commandId[8] = 0x02;
            cmdAll.mapCommand[MSG_TRANSMITTER_REDUCE_POWER_OPR].push_back(tmUnit);
            tmUnit.commandId[8] = 0x03;
            cmdAll.mapCommand[MSG_DEV_RESET_OPR].push_back(tmUnit);


        }
            break;
        case HUIXIN_996_QBEX:
        case HUIXIN_996_HRRISEX:
           {
               CommandUnit tmUnit;
               tmUnit.commandLen = 9;
               tmUnit.ackLen = 5;
               tmUnit.commandId[0] = 0x7E;
               tmUnit.commandId[1] = 0x12;
               tmUnit.commandId[2] = 0x05;
               tmUnit.commandId[3] = 0x00;
               tmUnit.commandId[4] = 0x30;
               tmUnit.commandId[5] = (m_addresscode&0x00FF);
               tmUnit.commandId[6] = ((m_addresscode&0xFF00)>>8);
               tmUnit.commandId[7] = 0x01;
               tmUnit.commandId[8] = 0x55;
               cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
               tmUnit.commandLen = 10;
               tmUnit.commandId[1] = 0x77;
               tmUnit.commandId[2] = 0x06;
               tmUnit.commandId[8] = 0x01;
               tmUnit.commandId[9] = 0x55;
               cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
               tmUnit.commandId[8] = 0x02;
               cmdAll.mapCommand[MSG_TRANSMITTER_MIDDLE_POWER_TURNON_OPR].push_back(tmUnit);
               tmUnit.commandId[8] = 0x03;
               cmdAll.mapCommand[MSG_TRANSMITTER_LOW_POWER_TURNON_OPR].push_back(tmUnit);
               tmUnit.commandId[8] = 0x00;
               cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
               tmUnit.commandId[1] = 0x31;
               tmUnit.commandId[8] = 0x01;
               cmdAll.mapCommand[MSG_TRANSMITTER_RISE_POWER_OPR].push_back(tmUnit);
               tmUnit.commandId[8] = 0x02;
               cmdAll.mapCommand[MSG_TRANSMITTER_REDUCE_POWER_OPR].push_back(tmUnit);
               tmUnit.commandId[8] = 0x03;
               cmdAll.mapCommand[MSG_DEV_RESET_OPR].push_back(tmUnit);
           }
               break;
        }
    }
    int AhhxTransmmit::Md993Data(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        float fdat=0;
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

    int AhhxTransmmit::Md996Data(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {

        int nCmd = data[1];
        if(nCmd!=0x11)
            return RE_CMDACK;

        DataInfo dtinfo;
        dtinfo.bType = false;
        for(int i=0;i<50;++i){
            if(i==2)
               continue;
            else{
                dtinfo.fValue = data[8+2*i]*256+data[9+2*i];
                data_ptr->mValues[i] = dtinfo;
            }
        }
        dtinfo.bType = true;
        for(int i=0;i<50;++i){
            dtinfo.fValue = data[108+i];
            data_ptr->mValues[50+i] = dtinfo;
        }
        return RE_SUCCESS;
    }

    int AhhxTransmmit::Md996QBData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        int nCmd = data[1];
        if(nCmd!=0x12)
            return RE_CMDACK;
        DataInfo dtinfo;
        dtinfo.bType = false;
        for(int i=0;i<81;++i)
        {
            dtinfo.fValue = data[8+2*i]*256+data[9+2*i];
            data_ptr->mValues[i] = dtinfo;
        }
        dtinfo.bType = true;
        for(int j=0;j<9;++j)
        {
            dtinfo.fValue = data[408+j];
            data_ptr->mValues[81+j] = dtinfo;
        }
        return RE_SUCCESS;
    }





     int AhhxTransmmit::Md996HrrisData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        int nCmd = data[1];
        if(nCmd!=0x12)
            return RE_CMDACK;
        DataInfo dtinfo;
        dtinfo.bType = false;
        for(int i=0;i<176;++i)
        {
            dtinfo.fValue = data[8+2*i]*256+data[9+2*i];
            data_ptr->mValues[i] = dtinfo;
        }
        dtinfo.bType = true;
        for(int i=2;i<8;++i)
        {
            dtinfo.fValue = Getbit(data[408],i)==0 ? 1:0;

            data_ptr->mValues[174+i] = dtinfo;
        }
        for(int i=0;i<7;++i)
        {
            dtinfo.fValue = Getbit(data[409],i)==0 ? 1:0;
            data_ptr->mValues[182+i] = dtinfo;
        }
        for(int i=0;i<3;++i)
        {
            dtinfo.fValue = Getbit(data[410],i)==0 ? 1:0;
            data_ptr->mValues[189+i] = dtinfo;
        }
        for(int i=0;i<8;++i)
        {
            dtinfo.fValue = Getbit(data[411],i)==0 ? 1:0;
            data_ptr->mValues[192+i] = dtinfo;
        }
        for(int i=0;i<8;++i)
        {
            dtinfo.fValue = Getbit(data[412],i)==0 ? 1:0;
            data_ptr->mValues[200+i] = dtinfo;
        }
        for(int i=0;i<8;++i)
        {
            dtinfo.fValue = Getbit(data[413],i)==0 ? 1:0;
            data_ptr->mValues[208+i] = dtinfo;
        }
        for(int i=0;i<6;++i)
        {
            dtinfo.fValue = Getbit(data[414],i)==0 ? 1:0;
            data_ptr->mValues[216+i] = dtinfo;
        }
        dtinfo.fValue = Getbit(data[414],7)==0 ? 1:0;
        data_ptr->mValues[222] = dtinfo;
        for(int i=0;i<8;++i)
        {
            dtinfo.fValue = Getbit(data[415],i)==0 ? 1:0;
            data_ptr->mValues[223+i] = dtinfo;
        }
        for(int i=0;i<7;++i)
        {
            dtinfo.fValue = Getbit(data[416],i)==0 ? 1:0;
            data_ptr->mValues[231+i] = dtinfo;
        }
        for(int i=1;i<4;++i)
        {
            dtinfo.fValue = Getbit(data[417],i)==0 ? 1:0;
            data_ptr->mValues[237+i] = dtinfo;
        }
        for(int i=5;i<8;++i)
        {
            dtinfo.fValue = Getbit(data[417],i)==0 ? 1:0;
            data_ptr->mValues[236+i] = dtinfo;
        }
        for(int i=0;i<4;++i)
        {
            dtinfo.fValue = Getbit(data[418],i)==0 ? 1:0;
            data_ptr->mValues[244+i] = dtinfo;
        }
        dtinfo.fValue = Getbit(data[418],6)==0 ? 1:0;
        data_ptr->mValues[248] = dtinfo;
        dtinfo.fValue = Getbit(data[418],7)==0 ? 1:0;
        data_ptr->mValues[249] = dtinfo;
        for(int i=1;i<8;++i)
        {
            dtinfo.fValue = Getbit(data[419],i)==0 ? 1:0;
            data_ptr->mValues[249+i] = dtinfo;
        }

        for(int j=0;j<4;++j)
        {
            for(int i=0;i<8;++i)
            {
                dtinfo.fValue = Getbit(data[420+j*4],i)==0 ? 1:0;
                data_ptr->mValues[257+i+28*j] = dtinfo;
            }
            for(int i=0;i<8;++i)
            {
                dtinfo.fValue = Getbit(data[421+j*4],i)==0 ? 1:0;
                data_ptr->mValues[265+i+28*j] = dtinfo;
            }
            for(int i=0;i<8;++i)
            {
                dtinfo.fValue = Getbit(data[422+j*4],i)==0 ? 1:0;
                data_ptr->mValues[273+i+28*j] = dtinfo;
            }
            for(int i=0;i<3;++i)
            {
                dtinfo.fValue = Getbit(data[423+j*4],i)==0 ? 1:0;
                data_ptr->mValues[281+i+28*j] = dtinfo;
            }
            dtinfo.fValue = Getbit(data[423+j*4],5)==0 ? 1:0;
            data_ptr->mValues[284+28*j] = dtinfo;
        }
        for(int j=0;j<4;++j)
        {
            for(int i=0;i<4;++i)
            {
                dtinfo.fValue = Getbit(data[437+j*4],i)==0 ? 1:0;
                data_ptr->mValues[369+i+16*j] = dtinfo;
            }
            for(int i=3;i<7;++i)
            {
                dtinfo.fValue = Getbit(data[438+j*4],i)==0 ? 1:0;
                data_ptr->mValues[370+i+16*j] = dtinfo;
            }
            for(int i=0;i<8;++i)
            {
                dtinfo.fValue = Getbit(data[439+j*4],i)==0 ? 1:0;
                data_ptr->mValues[377+i+16*j] = dtinfo;
            }
        }
        return RE_SUCCESS;
    }
}
