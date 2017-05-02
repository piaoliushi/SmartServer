#include "antenna_message.h"
namespace hx_net {
Antenna_message::Antenna_message(session_ptr pSession,DeviceInfo &devInfo)
    :base_message()
    ,d_devInfo(devInfo)
    ,m_pSession(pSession)
{
    if(IsStandardCommand())
        d_curData_ptr.reset(new Data);
}
Antenna_message::~Antenna_message()
{

}

int Antenna_message::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
{
    switch(d_devInfo.nDevProtocol)
    {
    case ANTENNA_CONTROL:{
        switch (d_devInfo.nSubProtocol) {
        case HX_MD981:
            if(data[0]==0xAA && data[1]==0x11 && data[11]==0xCC)
                return 0;
            else
                return -1;
        default:
            return RE_NOPROTOCOL;
        }
    }
    default:
        return RE_NOPROTOCOL;
    }
}

int Antenna_message::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    if(data_ptr!=NULL)
        d_curData_ptr = data_ptr;
    switch(d_devInfo.nDevProtocol)
    {
    case ANTENNA_CONTROL:{
        switch (d_devInfo.nSubProtocol) {
        case HX_MD981:
            return parse_HX_981(data,d_curData_ptr,nDataLen,iaddcode);
        default:
            return RE_NOPROTOCOL;
        }
    }
    default:
        return RE_NOPROTOCOL;
    }
}

bool Antenna_message::IsStandardCommand()
{
    switch (d_devInfo.nDevProtocol) {
    case ANTENNA_CONTROL:{
        switch(d_devInfo.nSubProtocol){
        default:
            return false;
        }
    }
    default:
        return false;
    }
}

void Antenna_message::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
{
    switch (d_devInfo.nDevProtocol) {
    case ANTENNA_CONTROL:
    {
        switch(d_devInfo.nSubProtocol){
        case HX_MD981:
        {
            if(lpParam->cparams_size()<1)
                break;
            //默认命令备机->主机
            cmdUnit.ackLen=0;
            cmdUnit.commandLen=11;
            cmdUnit.commandId[0]=0xAA;
            cmdUnit.commandId[1]=0x22;
            cmdUnit.commandId[2]=0x01;
            cmdUnit.commandId[3]=0x01;
            cmdUnit.commandId[4]=0x00;
            cmdUnit.commandId[5]=0x00;
            cmdUnit.commandId[6]=0x00;
            cmdUnit.commandId[7]=0x00;
            cmdUnit.commandId[8]=0x00;
            cmdUnit.commandId[9]=0x22;
            cmdUnit.commandId[10]=0xBB;
            if(atoi(lpParam->cparams(0).sparamvalue().c_str())==1)
            {
                cmdUnit.commandId[3]=0x00;
                cmdUnit.commandId[4]=0x01;
            }
        }
            break;
        default:
            break;
        }
    }
    default:
       break;
    }
}

void Antenna_message::GetAllCmd(CommandAttribute &cmdAll)
{
    switch (d_devInfo.nDevProtocol) {
    case ANTENNA_CONTROL:
    {
        switch(d_devInfo.nSubProtocol){
        case HX_MD981:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 12;
            tmUnit.commandLen = 11;
            tmUnit.commandId[0] = 0xAA;
            tmUnit.commandId[1] = 0x11;
            tmUnit.commandId[2] = 0x00;
            tmUnit.commandId[3] = 0x00;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x00;
            tmUnit.commandId[6] = 0x00;
            tmUnit.commandId[7] = 0x00;
            tmUnit.commandId[8] = 0x00;
            tmUnit.commandId[9] = 0x11;
            tmUnit.commandId[10] = 0xBB;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        default:
            break;
        }
    }
    default:
       break;
    }
}

int Antenna_message::parse_HX_981(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    iaddcode = d_devInfo.iAddressCode;
    DataInfo dainfo;
    dainfo.bType = true;
    if(data[3]==0x00 && data[4]==0x01)
        dainfo.fValue = 1;
    else
        dainfo.fValue = 0;
    data_ptr->mValues[0] = dainfo;
    return 0;
}


}
