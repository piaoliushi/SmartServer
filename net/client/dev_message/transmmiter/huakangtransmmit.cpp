#include "huakangtransmmit.h"
namespace hx_net
{

HuaKangtransmmit::HuaKangtransmmit(int subprotocol,int addresscode)
    :Transmmiter()
    ,m_subprotocol(subprotocol)
    ,m_addresscode(addresscode)
{
}

int HuaKangtransmmit::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
{
    switch(m_subprotocol)
    {
    case HK_FM_1KW:
    {
        if(data[0]==m_addresscode)// && data[1]==0x03)
            return data[2]+2;
        else
            return RE_HEADERROR;
    }
    default:
        return RE_NOPROTOCOL;
    }
    return RE_NOPROTOCOL;
}

int HuaKangtransmmit::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    switch(m_subprotocol)
    {
    case HK_FM_1KW:
        return Fm1KwData(data,data_ptr,nDataLen,runstate);
    default:
        return RE_NOPROTOCOL;
    }
    return RE_NOPROTOCOL;
}

bool HuaKangtransmmit::IsStandardCommand()
{
    switch(m_subprotocol)
    {
    case HK_FM_1KW:
        return true;
    default:
        return false;
    }
    return false;
}

void HuaKangtransmmit::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
{

}

void HuaKangtransmmit::GetAllCmd(CommandAttribute &cmdAll)
{
    CommandUnit tmUnit;
    switch(m_subprotocol)
    {
    case HK_FM_1KW:
    {
        tmUnit.commandLen = 8;
        tmUnit.ackLen = 3;
        tmUnit.commandId[0] = m_addresscode;
        tmUnit.commandId[1] = 0x03;
        tmUnit.commandId[2] = 0x00;
        tmUnit.commandId[3] = 0x00;
        tmUnit.commandId[4] = 0x00;
        tmUnit.commandId[5] = 0x38;
        unsigned short crcret = CRC16_A001(tmUnit.commandId,6);
        tmUnit.commandId[6] = (crcret&0x00FF);
        tmUnit.commandId[7] = ((crcret & 0xFF00)>>8);
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
    }
        break;
    default:
        return ;
    }
}

int HuaKangtransmmit::Fm1KwData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    if(data[1]!=0x03)
        return RE_CMDACK;
    DataInfo dtinfo;
    dtinfo.bType = false;
    dtinfo.fValue = (data[27]*256+data[28])*0.0001;
    data_ptr->mValues[0] = dtinfo;
    dtinfo.fValue = (data[29]*256+data[30])*0.1;
    data_ptr->mValues[1] = dtinfo;
    if(data_ptr->mValues[0].fValue*1000>data_ptr->mValues[1].fValue)
    {
        dtinfo.fValue =  sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
    }
    else
    {
        dtinfo.fValue = 0;
    }
    data_ptr->mValues[2] = dtinfo;
    dtinfo.fValue = (data[3]*256+data[4])*0.01;
    data_ptr->mValues[3] = dtinfo;
    dtinfo.fValue = (data[5]*256+data[6]);
    data_ptr->mValues[4] = dtinfo;
    dtinfo.fValue = (data[13]*256+data[14]);
    data_ptr->mValues[5] = dtinfo;
    dtinfo.fValue = (data[31]*256+data[32])*0.1;
    data_ptr->mValues[6] = dtinfo;
    dtinfo.fValue = (data[33]*256+data[34])*0.1;
    data_ptr->mValues[7] = dtinfo;
    dtinfo.fValue = (data[35]*256+data[36])*0.1;
    data_ptr->mValues[8] = dtinfo;
    return RE_SUCCESS;
}

}
