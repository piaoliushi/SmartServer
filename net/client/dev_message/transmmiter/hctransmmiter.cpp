#include "hctransmmiter.h"
namespace hx_net
{
    HcTransmmiter::HcTransmmiter(int subprotocol,int addresscode)
        :Transmmiter()
        ,m_subprotocol(subprotocol)
        ,m_addresscode(addresscode)
    {
    }

    int HcTransmmiter::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
    {
        switch(m_subprotocol)
        {
        case HangChun_HCF5KW:
        {
            if(nDataLen<5)
                return -1;
            if(data[0] == 0x3a && data[1] == 0x01 && data[2] == 0x30 && data[3] == 0x30 && data[4] == 0x30)
                return 0;
            else
            {
                unsigned char cDes[5]={0};
                cDes[0]=0x3a;
                cDes[0]=0x01;
                cDes[0]=0x30;
                cDes[0]=0x30;
                cDes[0]=0x30;
                return kmp(data,nDataLen,cDes,5);
            }
        }
        case HC_CDR_100_300W:
        case HC_CDR_1KW:
        case HC_CDR_3KW:
        case HC_CDR_10KW:
        {
            if(data[0]==0x7E && data[1]==0xE7 && data[2] == m_addresscode)
                return data[4]+5;
            else
                return RE_HEADERROR;
        }
        default:
            return RE_NOPROTOCOL;
        }
        return RE_NOPROTOCOL;
    }

    int HcTransmmiter::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        switch(m_subprotocol)
        {
        case HangChun_HCF5KW:
            return OnHangChunHCF5KW(data,data_ptr,nDataLen,runstate);
        case HC_CDR_100_300W:
            return HcCdr100_300W(data,data_ptr,nDataLen,runstate);
        case HC_CDR_1KW:
            return HcCdr1KW(data,data_ptr,nDataLen,runstate);
        case HC_CDR_3KW:
            return HcCdr3KW(data,data_ptr,nDataLen,runstate);
        case HC_CDR_10KW:
            return HcCdr10KW(data,data_ptr,nDataLen,runstate);
        default:
            return RE_NOPROTOCOL;
        }
        return RE_NOPROTOCOL;
    }

    bool HcTransmmiter::IsStandardCommand()
    {
        switch(m_subprotocol)
        {
        case HC_CDR_100_300W:
        case HC_CDR_1KW:
        case HC_CDR_3KW:
        case HC_CDR_10KW:
            return true;
        default:
            break;
        }
        return false;
    }

    void HcTransmmiter::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
    {
        switch(m_subprotocol)
        {
        default:
            return;
        }
        return;
    }

    void HcTransmmiter::GetAllCmd(CommandAttribute &cmdAll)
    {//7E E7 01 07 00 11 01 ED A5 C6 E8 AA
        CommandUnit tmUnit;
        switch(m_subprotocol)
        {
        case HangChun_HCF5KW:
        {
            tmUnit.commandLen = 1;
            tmUnit.ackLen = 0;
            tmUnit.commandId[0] = 0x55;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
        case HC_CDR_100_300W:
        case HC_CDR_1KW:
        {
            tmUnit.ackLen = 8;
            tmUnit.commandLen = 12;
            tmUnit.commandId[0] = 0x7E;
            tmUnit.commandId[1] = 0xE7;
            tmUnit.commandId[2] = m_addresscode;
            tmUnit.commandId[3] = 0x07;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x11;
            tmUnit.commandId[6] = 0x01;
            tmUnit.commandId[7] = 0xED;
            tmUnit.commandId[8] = 0xA5;
            tmUnit.commandId[9] = 0xC6;
            tmUnit.commandId[10] = 0xE8;
            tmUnit.commandId[11] = 0xAA;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[5] = 0x20;
            tmUnit.commandId[7] = 0x2C;
            tmUnit.commandId[8] = 0x35;
            tmUnit.commandId[9] = 0x37;
            tmUnit.commandId[10] = 0x0D;
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
            tmUnit.commandId[5] = 0x21;
            tmUnit.commandId[7] = 0xF0;
            tmUnit.commandId[8] = 0x58;
            tmUnit.commandId[9] = 0xAD;
            tmUnit.commandId[10] = 0xBA;
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
        }
            break;
        case HC_CDR_3KW:
        {
            tmUnit.ackLen = 8;
            tmUnit.commandLen = 12;
            tmUnit.commandId[0] = 0x7E;
            tmUnit.commandId[1] = 0xE7;
            tmUnit.commandId[2] = m_addresscode;
            tmUnit.commandId[3] = 0x07;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x11;
            tmUnit.commandId[6] = 0x01;
            tmUnit.commandId[7] = 0xED;
            tmUnit.commandId[8] = 0xA5;
            tmUnit.commandId[9] = 0xC6;
            tmUnit.commandId[10] = 0xE8;
            tmUnit.commandId[11] = 0xAA;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[5] = 0x12;
            tmUnit.commandId[6] = 0x01;
            tmUnit.commandId[7] = 0x8D;
            tmUnit.commandId[8] = 0xD2;
            tmUnit.commandId[9] = 0x74;
            tmUnit.commandId[10] = 0x86;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[6] = 0x02;
            tmUnit.commandId[7] = 0x8F;
            tmUnit.commandId[8] = 0xBB;
            tmUnit.commandId[9] = 0x81;
            tmUnit.commandId[10] = 0x0F;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[6] = 0x03;
            tmUnit.commandId[7] = 0x8E;
            tmUnit.commandId[8] = 0x63;
            tmUnit.commandId[9] = 0x2D;
            tmUnit.commandId[10] = 0x88;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[5] = 0x20;
            tmUnit.commandId[7] = 0x2C;
            tmUnit.commandId[8] = 0x35;
            tmUnit.commandId[9] = 0x37;
            tmUnit.commandId[10] = 0x0D;
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
            tmUnit.commandId[5] = 0x21;
            tmUnit.commandId[7] = 0xF0;
            tmUnit.commandId[8] = 0x58;
            tmUnit.commandId[9] = 0xAD;
            tmUnit.commandId[10] = 0xBA;
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
        }
            break;
        case HC_CDR_10KW:
        {
            tmUnit.ackLen = 8;
            tmUnit.commandLen = 12;
            tmUnit.commandId[0] = 0x7E;
            tmUnit.commandId[1] = 0xE7;
            tmUnit.commandId[2] = m_addresscode;
            tmUnit.commandId[3] = 0x07;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x11;
            tmUnit.commandId[6] = 0x01;
            tmUnit.commandId[7] = 0xED;
            tmUnit.commandId[8] = 0xA5;
            tmUnit.commandId[9] = 0xC6;
            tmUnit.commandId[10] = 0xE8;
            tmUnit.commandId[11] = 0xAA;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[5] = 0x12;
            tmUnit.commandId[6] = 0x01;
            tmUnit.commandId[7] = 0x8D;
            tmUnit.commandId[8] = 0xD2;
            tmUnit.commandId[9] = 0x74;
            tmUnit.commandId[10] = 0x86;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[6] = 0x02;
            tmUnit.commandId[7] = 0x8F;
            tmUnit.commandId[8] = 0xBB;
            tmUnit.commandId[9] = 0x81;
            tmUnit.commandId[10] = 0x0F;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[6] = 0x03;
            tmUnit.commandId[7] = 0x8E;
            tmUnit.commandId[8] = 0x63;
            tmUnit.commandId[9] = 0x2D;
            tmUnit.commandId[10] = 0x88;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[6] = 0x04;
            tmUnit.commandId[7] = 0x8B;
            tmUnit.commandId[8] = 0x68;
            tmUnit.commandId[9] = 0x6A;
            tmUnit.commandId[10] = 0x1D;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[6] = 0x05;
            tmUnit.commandId[7] = 0x8A;
            tmUnit.commandId[8] = 0xB0;
            tmUnit.commandId[9] = 0xC6;
            tmUnit.commandId[10] = 0x9A;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[6] = 0x06;
            tmUnit.commandId[7] = 0x88;
            tmUnit.commandId[8] = 0xD9;
            tmUnit.commandId[9] = 0x33;
            tmUnit.commandId[10] = 0x13;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[5] = 0x20;
            tmUnit.commandId[7] = 0x2C;
            tmUnit.commandId[8] = 0x35;
            tmUnit.commandId[9] = 0x37;
            tmUnit.commandId[10] = 0x0D;
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
            tmUnit.commandId[5] = 0x21;
            tmUnit.commandId[7] = 0xF0;
            tmUnit.commandId[8] = 0x58;
            tmUnit.commandId[9] = 0xAD;
            tmUnit.commandId[10] = 0xBA;
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
        }
            break;
        default:
            return;
        }
        return;
    }

    int HcTransmmiter::OnHangChunHCF5KW(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        int indexpos =0;
        DataInfo dtinfo;
        dtinfo.bType = false;
        dtinfo.fValue = AsciiToInt(data[10])+AsciiToInt(data[11])*0.1+AsciiToInt(data[12])*0.01;
        data_ptr->mValues[indexpos++] = dtinfo;
        dtinfo.fValue = AsciiToInt(data[10])*10+AsciiToInt(data[11]);
        data_ptr->mValues[indexpos++] = dtinfo;
        if(data_ptr->mValues[0].fValue*1000>data_ptr->mValues[1].fValue)
        {
            dtinfo.fValue = sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
        }
        else
        {
            dtinfo.fValue = 0;
        }
        data_ptr->mValues[indexpos++] = dtinfo;
        dtinfo.fValue = AsciiToInt(data[15])*100+AsciiToInt(data[16])*10+AsciiToInt(data[17]);
        data_ptr->mValues[indexpos++] = dtinfo;
        dtinfo.fValue = AsciiToInt(data[18])*100+AsciiToInt(data[19])*10+AsciiToInt(data[20]);
        data_ptr->mValues[indexpos++] = dtinfo;
        for(int i=0;i<3;++i)
        {
            dtinfo.fValue = AsciiToInt(data[21+2*i])*10+AsciiToInt(data[22+2*i]);
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        for(int i=0;i<5;++i)
        {
            dtinfo.fValue = AsciiToInt(data[27+4*i])*1000+AsciiToInt(data[28+4*i])*100+AsciiToInt(data[29+4*i])*10+AsciiToInt(data[30+4*i]);
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        return RE_SUCCESS;
    }

    int HcTransmmiter::HcCdr100_300W(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        if(data[5]!=0x11)
            return RE_CMDACK;
        if(data[7]!=0x00)
            return RE_HEADERROR;
        int baseindex=8;
        int indexpos =0;
        DataInfo dtinfo;
        dtinfo.bType = false;
        dtinfo.fValue = (data[baseindex]*256+data[baseindex+1])*0.0001;
        data_ptr->mValues[indexpos++] = dtinfo;
        dtinfo.fValue = (data[baseindex+2]*256+data[baseindex+3])*0.1;
        data_ptr->mValues[indexpos++] = dtinfo;
        if(data_ptr->mValues[0].fValue*1000>data_ptr->mValues[1].fValue)
        {
            dtinfo.fValue = sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
        }
        else
            dtinfo.fValue = 0;
        data_ptr->mValues[indexpos++] = dtinfo;
        if(Getbit(data[baseindex+4],7)==1)
            dtinfo.fValue =  (data[baseindex+4]&0x7F)*(-1);
        else
            dtinfo.fValue =  data[baseindex+4];
        data_ptr->mValues[indexpos++] = dtinfo;
        for(int i=0;i<3;++i)
        {
            dtinfo.fValue = (data[baseindex+5+2*i]*256+data[baseindex+6+2*i])*0.1;
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        int isign=0.1;
        if(Getbit(data[baseindex+11],7)==1)
            isign = -0.1;
        dtinfo.fValue = ((data[baseindex+11]&0x7F)*256+data[baseindex+12])*isign;
        data_ptr->mValues[indexpos++] = dtinfo;
        int sta1 = data[baseindex+21]*256+data[baseindex+22];
        dtinfo.bType = true;
        dtinfo.fValue = (sta1&0x07)==0x01 ? 0:1;
        runstate = (sta1&0x07)==0x01 ? 0:1;
        data_ptr->mValues[indexpos++] = dtinfo;
        for(int i=0;i<6;++i)
        {
            dtinfo.fValue = ((sta1>>(3+2*i))&0x03)==0x00 ? 0:1;
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        sta1 = data[baseindex+23]*256+data[baseindex+24];
        dtinfo.fValue = Getbit(sta1,0);
        data_ptr->mValues[indexpos++] = dtinfo;
        dtinfo.fValue = Getbit(sta1,1);
        data_ptr->mValues[indexpos++] = dtinfo;
        return RE_SUCCESS;
    }

    int HcTransmmiter::HcCdr1KW(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        if(data[5]!=0x11)
            return RE_CMDACK;
        if(data[7]!=0x00)
            return RE_HEADERROR;
        int baseindex=8;
        int indexpos =0;
        DataInfo dtinfo;
        dtinfo.bType = false;
        dtinfo.fValue = (data[baseindex]*256+data[baseindex+1])*0.0001;
        data_ptr->mValues[indexpos++] = dtinfo;
        dtinfo.fValue = (data[baseindex+2]*256+data[baseindex+3])*0.1;
        data_ptr->mValues[indexpos++] = dtinfo;
        if(data_ptr->mValues[0].fValue*1000>data_ptr->mValues[1].fValue)
        {
            dtinfo.fValue = sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
        }
        else
            dtinfo.fValue = 0;
        data_ptr->mValues[indexpos++] = dtinfo;
        if(Getbit(data[baseindex+4],7)==1)
            dtinfo.fValue =  (data[baseindex+4]&0x7F)*(-1);
        else
            dtinfo.fValue =  data[baseindex+4];
        data_ptr->mValues[indexpos++] = dtinfo;
        for(int i=0;i<7;++i)
        {
            dtinfo.fValue = (data[baseindex+5+2*i]*256+data[baseindex+6+2*i])*0.1;
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        int isign=0.1;
        if(Getbit(data[baseindex+19],7)==1)
            isign = -0.1;
        dtinfo.fValue = ((data[baseindex+19]&0x7F)*256+data[baseindex+20])*isign;
        data_ptr->mValues[indexpos++] = dtinfo;
        int sta1 = data[baseindex+21]*256+data[baseindex+22];
        dtinfo.bType = true;
        dtinfo.fValue = (sta1&0x07)==0x01 ? 0:1;
        runstate = (sta1&0x07)==0x01 ? 0:1;
        data_ptr->mValues[indexpos++] = dtinfo;
        for(int i=0;i<6;++i)
        {
            dtinfo.fValue = ((sta1>>(3+2*i))&0x03)==0x00 ? 0:1;
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        sta1 = data[baseindex+23]*256+data[baseindex+24];
        for(int i=0;i<5;++i)
        {
            dtinfo.fValue = ((sta1>>(2*i))&0x03)==0x00 ? 0:1;
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        dtinfo.fValue = Getbit(sta1,10);
        data_ptr->mValues[indexpos++] = dtinfo;
        dtinfo.fValue = Getbit(sta1,11);
        data_ptr->mValues[indexpos++] = dtinfo;
        return RE_SUCCESS;
    }

    int HcTransmmiter::HcCdr3KW(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        if(data[5]!=0x11 && data[5]!=0x12)
            return RE_CMDACK;
        if(data[7]!=0x00)
            return RE_HEADERROR;
        if(data[5]==0x11)
        {
            int baseindex=8;
            int indexpos =0;
            DataInfo dtinfo;
            dtinfo.bType = false;
            dtinfo.fValue = (data[baseindex]*256+data[baseindex+1])*0.0001;
            data_ptr->mValues[indexpos++] = dtinfo;
            dtinfo.fValue = (data[baseindex+2]*256+data[baseindex+3])*0.1;
            data_ptr->mValues[indexpos++] = dtinfo;
            if(data_ptr->mValues[0].fValue*1000>data_ptr->mValues[1].fValue)
            {
                dtinfo.fValue = sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
            }
            else
                dtinfo.fValue = 0;
            data_ptr->mValues[indexpos++] = dtinfo;
            if(Getbit(data[baseindex+4],7)==1)
                dtinfo.fValue =  (data[baseindex+4]&0x7F)*(-1);
            else
                dtinfo.fValue =  data[baseindex+4];
            data_ptr->mValues[indexpos++] = dtinfo;
            for(int i=0;i<3;++i)
            {
                dtinfo.fValue = (data[baseindex+5+2*i]*256+data[baseindex+6+2*i]);
                data_ptr->mValues[indexpos++] = dtinfo;
            }
            dtinfo.fValue = (data[baseindex+11]*256+data[baseindex+12])*0.1;
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        else
        {
            int baseindex=8;
            int indexpos =8;
            int ampnum = data[6];
            if(ampnum>0 && ampnum<4)
            {
                indexpos += 12*(ampnum-1);
                DataInfo dtinfo;
                dtinfo.bType = false;
                dtinfo.fValue = (data[baseindex]*256+data[baseindex+1])*0.1;
                data_ptr->mValues[indexpos++] = dtinfo;
                dtinfo.fValue = (data[baseindex+2]*256+data[baseindex+3])*0.1;
                int isign=0.1;
                if(Getbit(data[baseindex+4],7)==1)
                    isign = -0.1;
                dtinfo.fValue = ((data[baseindex+4]&0x7F)*256+data[baseindex+5])*isign;
                data_ptr->mValues[indexpos++] = dtinfo;
                if(Getbit(data[baseindex+6],7)==1)
                    dtinfo.fValue =  (data[baseindex+6]&0x7F)*(-1);
                else
                    dtinfo.fValue =  data[baseindex+6];
                for(int i=0;i<8;++i)
                {
                    dtinfo.fValue = (data[baseindex+7+2*i]*256+data[baseindex+8+2*i])*0.1;
                    data_ptr->mValues[indexpos++] = dtinfo;
                }
            }
        }
        return RE_SUCCESS;
    }

    int HcTransmmiter::HcCdr10KW(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        if(data[5]!=0x11 && data[5]!=0x12)
            return RE_CMDACK;
        if(data[7]!=0x00)
            return RE_HEADERROR;
        if(data[5]==0x11)
        {
            int baseindex=8;
            int indexpos =0;
            DataInfo dtinfo;
            dtinfo.bType = false;
            dtinfo.fValue = (data[baseindex]*256+data[baseindex+1])*0.0001;
            data_ptr->mValues[indexpos++] = dtinfo;
            dtinfo.fValue = (data[baseindex+2]*256+data[baseindex+3])*0.1;
            data_ptr->mValues[indexpos++] = dtinfo;
            if(data_ptr->mValues[0].fValue*1000>data_ptr->mValues[1].fValue)
            {
                dtinfo.fValue = sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
            }
            else
                dtinfo.fValue = 0;
            data_ptr->mValues[indexpos++] = dtinfo;
            if(Getbit(data[baseindex+4],7)==1)
                dtinfo.fValue =  (data[baseindex+4]&0x7F)*(-1);
            else
                dtinfo.fValue =  data[baseindex+4];
            data_ptr->mValues[indexpos++] = dtinfo;
            for(int i=0;i<3;++i)
            {
                dtinfo.fValue = (data[baseindex+5+2*i]*256+data[baseindex+6+2*i]);
                data_ptr->mValues[indexpos++] = dtinfo;
            }
            dtinfo.fValue = (data[baseindex+11]*256+data[baseindex+12])*0.1;
            data_ptr->mValues[indexpos++] = dtinfo;
        }
        else
        {
            int baseindex=8;
            int indexpos =8;
            int ampnum = data[6];
            if(ampnum>0 && ampnum<7)
            {
                indexpos += 12*(ampnum-1);
                DataInfo dtinfo;
                dtinfo.bType = false;
                dtinfo.fValue = (data[baseindex]*256+data[baseindex+1])*0.1;
                data_ptr->mValues[indexpos++] = dtinfo;
                dtinfo.fValue = (data[baseindex+2]*256+data[baseindex+3])*0.1;
                int isign=0.1;
                if(Getbit(data[baseindex+4],7)==1)
                    isign = -0.1;
                dtinfo.fValue = ((data[baseindex+4]&0x7F)*256+data[baseindex+5])*isign;
                data_ptr->mValues[indexpos++] = dtinfo;
                if(Getbit(data[baseindex+6],7)==1)
                    dtinfo.fValue =  (data[baseindex+6]&0x7F)*(-1);
                else
                    dtinfo.fValue =  data[baseindex+6];
                for(int i=0;i<8;++i)
                {
                    dtinfo.fValue = (data[baseindex+7+2*i]*256+data[baseindex+8+2*i])*0.1;
                    data_ptr->mValues[indexpos++] = dtinfo;
                }
            }
        }
        return RE_SUCCESS;
    }
}
