#include "astransmmit.h"
namespace hx_net{
AsTransmmit::AsTransmmit(Tsmt_message_ptr ptsmessage,int subprotocol,int addresscode)
    :Transmmiter()
    ,m_subprotocol(subprotocol)
    ,m_addresscode(addresscode)
    ,m_pmessage(ptsmessage)
{
}

int AsTransmmit::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
{

    switch(m_subprotocol)
    {
    case ANSHAN_JIAHUI:
    {
        if(data[0] == 0xa5 && data[1] == m_addresscode)
            return data[2]+2;
        else
            return RE_HEADERROR;
    }
    case ANSHAN_JIAHUI_3KW_TV:
    {
        if(data[0] == 0xa5 && (data[1] == 0x0c || data[1] == 0x0a || data[1] == 0x0e || data[1] == 0x00 || data[1] == 0x41))
            return 0;
        else
        {
            unsigned char cDes[1]={0};
            cDes[0]=0xa5;
            return kmp(data,nDataLen,cDes,1);
        }
    }
    default:
        break;
    }
    return RE_NOPROTOCOL;
}

int AsTransmmit::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    switch(m_subprotocol)
    {
    case ANSHAN_JIAHUI:
        return OnJiaHui_1Kw(data,data_ptr,nDataLen,runstate);
    case ANSHAN_JIAHUI_3KW_TV:
        return JiaHui_TV_3Kw(data,data_ptr,nDataLen,runstate);
    default:
        break;
    }
    return RE_NOPROTOCOL;
}

bool AsTransmmit::IsStandardCommand()
{
    switch(m_subprotocol)
    {
    case ANSHAN_JIAHUI:
        return true;
    }
    return false;
}

void AsTransmmit::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
{

}

void AsTransmmit::GetAllCmd(CommandAttribute &cmdAll)
{
    switch(m_subprotocol)
    {
    case ANSHAN_JIAHUI:
    {
        CommandUnit tmUnit;
        tmUnit.commandLen = 7;
        tmUnit.ackLen = 3;
        tmUnit.commandId[0] = 0x5A;
        tmUnit.commandId[1] = m_addresscode;
        tmUnit.commandId[2] = 0x02;
        tmUnit.commandId[3] = 0x3C;
        tmUnit.commandId[4] = 0x00;
        tmUnit.commandId[5] = 0x3E;
        tmUnit.commandId[6] = 0x0D;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.commandLen = 6;
        tmUnit.commandId[2] = 0x01;
        tmUnit.commandId[3] = 0x3D;
        tmUnit.commandId[4] = 0x3E;
        tmUnit.commandId[5] = 0x0D;
        cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
        tmUnit.commandId[3] = 0x4B;
        tmUnit.commandId[4] = 0x4C;
        cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
    }
        break;
    case ANSHAN_JIAHUI_3KW_TV:
    {
        CommandUnit tmUnit;
        tmUnit.commandLen = 4;
        tmUnit.ackLen = 18;
        tmUnit.commandId[0] = 0x42;
        tmUnit.commandId[1] = 0x0C;
        tmUnit.commandId[2] = 0x08;
        tmUnit.commandId[3] = 0xBB;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.ackLen = 0;
        tmUnit.commandId[1] = 0x41;
        cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
        cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
    }
        break;
    default:
        break;
    }
}

int AsTransmmit::OnJiaHui_1Kw(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    int indexpos =0;
    DataInfo dtinfo;
    dtinfo.bType = false;
    int res1 =BCD2OCT(data[7]);
    int res2 =BCD2OCT(data[6]);
    dtinfo.fValue = (res1*100+res2)*0.001;
    data_ptr->mValues[indexpos++] = dtinfo;
    res1 =BCD2OCT(data[9]);
    res2 =BCD2OCT(data[8]);
    dtinfo.fValue = (res1*100+res2);
    data_ptr->mValues[indexpos++] = dtinfo;
    dtinfo.fValue = 0;
    data_ptr->mValues[indexpos++] = dtinfo;
    res1 =BCD2OCT(data[11]);
    res2 =BCD2OCT(data[10]);
    dtinfo.fValue = (res1*100+res2)*0.1;
    dtinfo.bType = true;
    dtinfo.fValue = Getbit(data[5],7);
    data_ptr->mValues[indexpos++] = dtinfo;
    dtinfo.fValue = Getbit(data[5],3)==0 ? 1:0;
    data_ptr->mValues[indexpos++] = dtinfo;
    dtinfo.fValue = Getbit(data[5],2);
    data_ptr->mValues[indexpos++] = dtinfo;
    dtinfo.fValue = Getbit(data[5],1);
    data_ptr->mValues[indexpos++] = dtinfo;
    dtinfo.fValue = Getbit(data[5],0);
    data_ptr->mValues[indexpos++] = dtinfo;
    if(data_ptr->mValues[0].fValue*1000>data_ptr->mValues[1].fValue)
    {
        dtinfo.bType = false;
        dtinfo.fValue = sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
        data_ptr->mValues[2] = dtinfo;
    }
    return RE_SUCCESS;
}

int AsTransmmit::JiaHui_TV_3Kw(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    if(data[1]!=0x0C)
        return RE_CMDACK;
    DataInfo dtinfo;
    dtinfo.bType = false;
    dtinfo.fValue = (data[12]+data[13])/4095.00;
    data_ptr->mValues[0] = dtinfo;
    dtinfo.fValue = (data[14]+data[15])/4095.00;
    data_ptr->mValues[1] = dtinfo;
    dtinfo.fValue = 0.99;
    data_ptr->mValues[2] = dtinfo;
    dtinfo.fValue = (data[16]+data[17])/4095.00;
    data_ptr->mValues[3] = dtinfo;
    return RE_SUCCESS;
}
}
