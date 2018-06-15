#include "rvrtransmmit.h"
namespace hx_net{

RvrTransmmit::RvrTransmmit(Tsmt_message_ptr ptsmessage,int subprotocol,int addresscode)
    :Transmmiter()
    ,m_subprotocol(subprotocol)
    ,m_addresscode(addresscode)
    ,m_pmessage(ptsmessage)
{
}

int RvrTransmmit::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
{
    switch(m_subprotocol)
    {
    case RVR_GENERAL:
    {
        if(data[0]==m_addresscode)
            return RE_SUCCESS;
        else
            return RE_HEADERROR;
    }
        break;
    case RSM_FM_OCBD:
    {
        if(data[0]==0x72)
            return RE_SUCCESS;
        else
            return RE_HEADERROR;
    }
    default:
        break;
    }
    return RE_NOPROTOCOL;
}

int RvrTransmmit::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    switch(m_subprotocol)
    {
    case RVR_GENERAL:
        return RvrGeneralData(data,data_ptr,nDataLen,runstate);
    case RSM_FM_OCBD:
        return RsmContrlBdData(data,data_ptr,nDataLen,runstate);
    default:
        break;
    }
    return RE_NOPROTOCOL;
}

bool RvrTransmmit::IsStandardCommand()
{
    switch(m_subprotocol)
    {
    case RVR_GENERAL:
        break;
    default:
        break;
    }
    return false;
}

void RvrTransmmit::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
{
    switch(m_subprotocol)
    {
    case RVR_GENERAL:
        break;
    default:
        break;
    }
}

void RvrTransmmit::GetAllCmd(CommandAttribute &cmdAll)
{
    switch(m_subprotocol)
    {
    case RVR_GENERAL:
    {
        CommandUnit tmUnit;
        tmUnit.commandLen = 1;
        tmUnit.ackLen = 1;
        tmUnit.commandId[0] = m_subprotocol;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.commandLen = 6;
        tmUnit.ackLen = 137;
        //3c 05 03 00 05 3f
        tmUnit.commandId[0] = 0x3C;
        tmUnit.commandId[1] = 0x05;
        tmUnit.commandId[2] = 0x03;
        tmUnit.commandId[3] = 0x00;
        tmUnit.commandId[4] = 0x05;
        tmUnit.commandId[5] = 0x3F;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);

    }
        break;
    case RSM_FM_OCBD:
    {
        CommandUnit tmUnit;
        tmUnit.commandLen = 1;
        tmUnit.ackLen = 13;
        tmUnit.commandId[0] = 0x72;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.commandId[0] = 0x66;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
    }
    default:
        break;
    }
}

int RvrTransmmit::RvrGeneralData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    int baseadd = 31;
    int indexpos = 0;
    DataInfo dtinfo;
    dtinfo.bType = false;
    dtinfo.fValue = (data[baseadd+1]*256+data[baseadd])*0.1;
    data_ptr->mValues[indexpos++] = dtinfo;
    dtinfo.fValue = (data[baseadd+3]*256+data[baseadd+2])*0.01;
    data_ptr->mValues[indexpos++] = dtinfo;
    if((data_ptr->mValues[0].fValue>data_ptr->mValues[1].fValue))
        dtinfo.fValue = sqrt((data_ptr->mValues[0].fValue+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue-data_ptr->mValues[1].fValue));
    else
        dtinfo.fValue = 0;
    data_ptr->mValues[indexpos++] = dtinfo;
    dtinfo.fValue = (data[baseadd+5]*256+data[baseadd+4])*0.05;
    data_ptr->mValues[indexpos++] = dtinfo;
    dtinfo.fValue = (data[baseadd+7]*256+data[baseadd+6])*0.01;
    data_ptr->mValues[indexpos++] = dtinfo;
    dtinfo.fValue = (data[baseadd+9]*256+data[baseadd+8])*0.1;
    data_ptr->mValues[indexpos++] = dtinfo;
    dtinfo.fValue = (data[baseadd+11]*256+data[baseadd+10])*0.01;
    data_ptr->mValues[indexpos++] = dtinfo;
    dtinfo.fValue = (data[baseadd+13]*256+data[baseadd+12])*0.05;
    data_ptr->mValues[indexpos++] = dtinfo;
    dtinfo.fValue = (data[baseadd+15]*256+data[baseadd+14])*0.1;
    data_ptr->mValues[indexpos++] = dtinfo;
    dtinfo.fValue = (data[baseadd+17]*256+data[baseadd+16])*0.1;
    data_ptr->mValues[indexpos++] = dtinfo;
    return RE_SUCCESS;
}

int RvrTransmmit::RsmContrlBdData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    char ResultData[6]={0};
    DataInfo dtinfo;
    dtinfo.bType = false;
    ResultData[0] = data[2];
    ResultData[1] = data[3];
    ResultData[2] = data[4];
    ResultData[3] = data[5];
    ResultData[4] = data[6];
    ResultData[5] = data[7];
    dtinfo.fValue = atof(ResultData);
    data_ptr->mValues[0] = dtinfo;
    ResultData[0] = data[15];
    ResultData[1] = data[16];
    ResultData[2] = data[17];
    ResultData[3] = data[18];
    ResultData[4] = data[19];
    ResultData[5] = data[20];
    dtinfo.fValue = atof(ResultData);
    data_ptr->mValues[1] = dtinfo;
    if(data_ptr->mValues[0].fValue*1000>data_ptr->mValues[1].fValue)
    {
        dtinfo.fValue = sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
    }
    else
    {
        dtinfo.fValue = 0;
    }
    data_ptr->mValues[2] = dtinfo;
    return RE_SUCCESS;
}

}
