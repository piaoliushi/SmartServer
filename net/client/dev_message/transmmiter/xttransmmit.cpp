#include "xttransmmit.h"
namespace hx_net{

XtTransmmit::XtTransmmit(int subprotocol,int addresscode)
    :Transmmiter()
    ,m_subprotocol(subprotocol)
    ,m_addresscode(addresscode)
{
}

int XtTransmmit::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
{
    switch(m_subprotocol)
    {
    case XT_DM:{
        if(data[0] == 0x7E && data[1]==0xE7)
            return pHeader[2];
        return -1;
    }
        break;
    }
    return RE_NOPROTOCOL;
}

int XtTransmmit::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    switch(m_subprotocol)
    {
    case XT_DM:
        return xtDMData(data,data_ptr,nDataLen,runstate);
    }
    return RE_NOPROTOCOL;
}

bool XtTransmmit::IsStandardCommand()
{
    switch(m_subprotocol)
    {
    case XT_DM:
        return true;
    }
    return false;
}

void XtTransmmit::GetAllCmd(CommandAttribute &cmdAll)
{
    switch(m_subprotocol)
    {
    case XT_DM:
    {
        CommandUnit tmUnit;
        tmUnit.commandLen = 7;
        tmUnit.ackLen = 3;
        tmUnit.commandId[0] = 0x7E;
        tmUnit.commandId[1] = 0xE7;
        tmUnit.commandId[2] = 0x04;
        tmUnit.commandId[3] = (m_addresscode&0xFF);
        tmUnit.commandId[4] = 0x03;
        tmUnit.commandId[5] = 0x00;
        tmUnit.commandId[6] = 0x9E;

        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.commandId[4] = 0x0C;
        tmUnit.commandId[6] = 0xFF;
        cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
        tmUnit.commandId[4] = 0x0A;
        tmUnit.commandId[6] = 0xFF;
        cmdAll.mapCommand[MSG_TRANSMITTER_MIDDLE_POWER_TURNON_OPR].push_back(tmUnit);
        tmUnit.commandId[4] = 0x0B;
        tmUnit.commandId[6] = 0xFF;
        cmdAll.mapCommand[MSG_TRANSMITTER_LOW_POWER_TURNON_OPR].push_back(tmUnit);
        tmUnit.commandId[4] = 0x09;
        tmUnit.commandId[6] = 0x94;
        cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
        tmUnit.commandId[1] = 0x0F;
        tmUnit.commandId[8] = 0x92;
        cmdAll.mapCommand[MSG_DEV_RESET_OPR].push_back(tmUnit);
    }
        break;
    }
}

int XtTransmmit::xtDMData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    if(data[4]!=0x03)
            return RE_CMDACK;
    DataInfo dtinfo;
    dtinfo.bType = false;
    dtinfo.fValue = (float)(data[11]*data[11]/43302.000000);
    data_ptr->mValues[0] = dtinfo;
    dtinfo.fValue = (float)(data[12]*data[12]/43302.000000);
    data_ptr->mValues[1] = dtinfo;
    dtinfo.fValue = (float)(data[15]/255.000000);
    data_ptr->mValues[2] = dtinfo;
    dtinfo.fValue = (float)(data[13]/255.000000);
    data_ptr->mValues[3] = dtinfo;
    dtinfo.fValue = (float)(data[14]/255.000000);
    data_ptr->mValues[4] = dtinfo;
    dtinfo.fValue = (float)(data[16]/255.000000);
    data_ptr->mValues[5] = dtinfo;
    dtinfo.fValue = (float)(data[17]/255.000000);
    data_ptr->mValues[6] = dtinfo;
    dtinfo.fValue = (float)(data[18]/255.000000);
    data_ptr->mValues[7] = dtinfo;
    dtinfo.fValue = (float)(data[19]/255.000000);
    data_ptr->mValues[8] = dtinfo;
    dtinfo.fValue = (float)(data[20]/255.000000);
    data_ptr->mValues[9] = dtinfo;
    dtinfo.fValue = (float)(data[21]/255.000000);
    data_ptr->mValues[10] = dtinfo;
    dtinfo.fValue = (float)(data[22]/255.000000);
    data_ptr->mValues[11] = dtinfo;
    dtinfo.fValue = (data[23]<<8)|(data[24]);
    data_ptr->mValues[12] = dtinfo;
    int baseindex = 13;
    dtinfo.bType = true;
    for(int i=0;i<8;++i)
    {
        dtinfo.fValue = Getbit(data[5],i)==0 ? 1:0;
        data_ptr->mValues[baseindex++] = dtinfo;
    }
    for(int i=0;i<8;++i)
    {
        if(i!=1 && i!=3)
        {
            dtinfo.fValue = Getbit(data[6],i)==0 ? 1:0;
            data_ptr->mValues[baseindex++] = dtinfo;
        }
    }
    for(int i=0;i<4;++i)
    {
        dtinfo.fValue = Getbit(data[7],i)==0 ? 1:0;
        data_ptr->mValues[baseindex++] = dtinfo;
    }
    for(int i=0;i<8;++i)
    {
        if(i!=5)
        {
            dtinfo.fValue = Getbit(data[8],i)==0 ? 1:0;
            data_ptr->mValues[baseindex++] = dtinfo;
        }
    }
    for(int i=0;i<3;++i)
    {
        dtinfo.fValue = Getbit(data[9],i)==0 ? 1:0;
        data_ptr->mValues[baseindex++] = dtinfo;
    }
    for(int i=0;i<8;++i)
    {
        dtinfo.fValue = Getbit(data[10],i)==0 ? 1:0;
        data_ptr->mValues[baseindex++] = dtinfo;
    }
    return RE_SUCCESS;
}

}
