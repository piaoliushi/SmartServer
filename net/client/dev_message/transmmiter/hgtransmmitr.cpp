#include "hgtransmmitr.h"
namespace hx_net{
HgTransmmitr::HgTransmmitr(int subprotocol,int addresscode)
            :Transmmiter()
            ,m_subprotocol(subprotocol)
            ,m_addresscode(addresscode)
{
}

int HgTransmmitr::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
{
    switch(m_subprotocol)
    {
    case HAERBIN_HG_50kw:
    case HAERBIN_HG_100kw:
    case HAERBIN_HG_10kw:
    case HAERBIN_HGEX_10kw:
    {
        if(data[0] == 0x06 && data[1] == 0x02)
            return 0;
        else
        {
            unsigned char cDes[2]={0};
            cDes[0] = 0x06;
            cDes[1] = 0x02;
            return kmp(data,number,cDes,2);
        }
    }
    default:
        return RE_NOPROTOCOL;
    }
    return RE_NOPROTOCOL;
}

int HgTransmmitr::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
   switch(m_subprotocol)
   {
   case HAERBIN_HG_50kw:
       return Hg50KwData(data,data_ptr,nDataLen,runstate);
   case HAERBIN_HG_100kw:
       return Hg100KwData(data,data_ptr,nDataLen,runstate);
   case HAERBIN_HG_10kw:
       return Hg10KwData(data,data_ptr,nDataLen,runstate);
   case HAERBIN_HGEX_10kw:
       return Hg10KwexData(data,data_ptr,nDataLen,runstate);
   }
   return RE_NOPROTOCOL;
}

bool HgTransmmitr::IsStandardCommand()
{
    return false;
}

void HgTransmmitr::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
{
    switch(m_subprotocol)
    {
    case HAERBIN_HG_50kw:
        break;
    case HAERBIN_HG_100kw:
        break;
    }
}

void HgTransmmitr::GetAllCmd(CommandAttribute &cmdAll)
{
    int hhInt = m_addresscode/100+0x30;
    int hInt = ((m_addresscode-(hhInt-0x30)*100)/10)+0x30;
    int lInt = m_addresscode%10+0x30;
    switch(m_subprotocol)
    {
    case HAERBIN_HG_10kw:
    case HAERBIN_HGEX_10kw:
    case HAERBIN_HG_50kw:
    case HAERBIN_HG_100kw:{
        CommandUnit tmUnit;
        tmUnit.commandLen = 6;
        tmUnit.ackLen = 173;
        tmUnit.commandId[0]=0x05;
        tmUnit.commandId[1]=0x46;
        tmUnit.commandId[2]=hhInt;
        tmUnit.commandId[3]=hInt;
        tmUnit.commandId[4]=lInt;
        tmUnit.commandId[5]=0x49;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.commandId[5]=0x4F;
        cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
        tmUnit.commandId[5]=0x43;
        cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
        tmUnit.commandId[5]=0x55;
        cmdAll.mapCommand[MSG_TRANSMITTER_RISE_POWER_OPR].push_back(tmUnit);
        tmUnit.commandId[5]=0x44;
        cmdAll.mapCommand[MSG_TRANSMITTER_REDUCE_POWER_OPR].push_back(tmUnit);
    }
        break;
    }
}

int HgTransmmitr::Hg100KwData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    char ResultData[4]={0};
    int HiByte=-1;
    DataInfo dtinfo;
    dtinfo.bType = false;
    if(data[6]!=0x53)
    {
        for(int i=0;i<12;i++)
        {
            if(data[13*i +1] != 0x02)
                continue;

            if(data [13*i +6] == 0x50)//序号以P开头
            {
                HiByte = data[13 *i +8] -0x30;
                if(HiByte == 1) //输入功率
                {

                    ResultData[0] = data[13*i +9];
                    ResultData[1] = data[13*i +10];
                    ResultData[2] = data[13*i +11];
                    ResultData[3] = data[13*i +12];
                    dtinfo.fValue = atof(ResultData);
                    data_ptr->mValues[0] = dtinfo;
                }else if(HiByte == 2)//反射功率
                {
                    ResultData[0] = data[13*i +9];
                    ResultData[1] = data[13*i +10];
                    ResultData[2] = data[13*i +11];
                    ResultData[3] = data[13*i +12];
                    dtinfo.fValue = atof(ResultData);
                    data_ptr->mValues[1] = dtinfo;
                }
            }
            else if(data[13*i +6] == 0x56) //各种电压值
            {
                HiByte = data[13*i+8] - 0x30;//参数编码序号,V01,V02,V03...


                ResultData[0] = data[13*i +9];
                ResultData[1] = data[13*i +10];
                ResultData[2] = data[13*i +11];
                ResultData[3] = data[13*i +12];
                dtinfo.fValue = atof(ResultData);
                data_ptr->mValues[2+HiByte] = dtinfo;

            }
        }
        if((data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue)<=0)
        {
            data_ptr->mValues[2].fValue = 0;
        }
        else
        {
            data_ptr->mValues[2].fValue = (data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue);
        }
    }
    else
        return RE_CMDACK;
    return RE_SUCCESS;
}

int HgTransmmitr::Hg50KwData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    char ResultData[4]={0};
    int HiByte=-1;
    DataInfo dtinfo;
    dtinfo.bType = false;
    if(data[6]!=0x53)
    {
        for(int i=0;i<12;i++)
        {
            if(data[13*i +1] != 0x02)
                continue;

            if(data [13*i +6] == 0x50)//序号以P开头
            {
                HiByte = data[13 *i +8] -0x30;
                if(HiByte == 1) //输入功率
                {

                    ResultData[0] = data[13*i +9];
                    ResultData[1] = data[13*i +10];
                    ResultData[2] = data[13*i +11];
                    ResultData[3] = data[13*i +12];
                    dtinfo.fValue = atof(ResultData);
                    data_ptr->mValues[0] = dtinfo;
                }else if(HiByte == 2)//反射功率
                {
                    ResultData[0] = data[13*i +9];
                    ResultData[1] = data[13*i +10];
                    ResultData[2] = data[13*i +11];
                    ResultData[3] = data[13*i +12];
                    dtinfo.fValue = atof(ResultData);
                    data_ptr->mValues[1] = dtinfo;
                }
            }
            else if(data[13*i +6] == 0x56) //各种电压值
            {
                HiByte = data[13*i+8] - 0x30;//参数编码序号,V01,V02,V03...


                ResultData[0] = data[13*i +9];
                ResultData[1] = data[13*i +10];
                ResultData[2] = data[13*i +11];
                ResultData[3] = data[13*i +12];
                dtinfo.fValue = atof(ResultData);
                data_ptr->mValues[2+HiByte] = dtinfo;

            }
        }
        if((data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue)<=0)
        {
            data_ptr->mValues[2].fValue = 0;
        }
        else
        {
            data_ptr->mValues[2].fValue = (data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue);
        }
    }
    else
        return RE_CMDACK;
    return RE_SUCCESS;
}

int HgTransmmitr::Hg10KwData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    char ResultData[10]={0};
    int HiByte=-1,Bmodel=-1;
    DataInfo dtinfo;
    dtinfo.bType = false;
    int nlastlen = nDataLen;
    while(nlastlen>13)
    {
        unsigned char cDes[2]={0};
        cDes[0] = 0x02;
        cDes[1] = 0x46;
        int npos = kmp(data,nlastlen,cDes,2);
        data = data+npos;
        unsigned char cendDes[1]={0x03};
        int fendpos =  kmp(data,nlastlen,cendDes,1);
        if(fendpos<0)
            break;
        Bmodel = data[5];// -0x30;
        for(int i=0;i<(fendpos-8);++i)
        {
            ResultData[i] = data[8+i];
        }
        if(Bmodel == 0x50)//序号以P开头
        {
            HiByte = data[7] -0x30;
            if(HiByte == 1) //输入功率
            {


                dtinfo.fValue = atof(ResultData);
                data_ptr->mValues[0] = dtinfo;
            }else if(HiByte == 2)//反射功率
            {
                dtinfo.fValue = atof(ResultData);
                data_ptr->mValues[1] = dtinfo;
            }
        }
        else if(Bmodel == 0x56) //各种电压值
        {
            HiByte = data[7] - 0x30;//参数编码序号,V01,V02,V03...
            dtinfo.fValue = atof(ResultData);
            data_ptr->mValues[2+HiByte] = dtinfo;

        }
        data = data+fendpos+1;
        nlastlen = nlastlen-(fendpos+1);
    }
    if((data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue)<=0)
    {
        data_ptr->mValues[2].fValue = 0;
    }
    else
    {
        data_ptr->mValues[2].fValue = (data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue);
    }
    return RE_SUCCESS;
    if((data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue)<=0)
    {
        data_ptr->mValues[2].fValue = 0;
    }
    else
    {
        data_ptr->mValues[2].fValue = (data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue);
    }
    return RE_SUCCESS;
}

int HgTransmmitr::Hg10KwexData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    char ResultData[10]={0};
       int HiByte=-1,Bmodel=-1;
       DataInfo dtinfo;
       dtinfo.bType = false;
       int nlastlen = nDataLen;
       while(nlastlen>13)
       {
           unsigned char cDes[2]={0};
           cDes[0] = 0x02;
           cDes[1] = 0x46;
           int npos = kmp(data,nlastlen,cDes,2);
           data = data+npos;
           unsigned char cendDes[1]={0x03};
           int fendpos =  kmp(data,nlastlen,cendDes,1);
           if(fendpos<0)
               break;
           Bmodel = data[5];// -0x30;
           for(int i=0;i<(fendpos-8);++i)
           {
               ResultData[i] = data[8+i];
           }
           if(Bmodel == 0x50)//序号以P开头
           {
               HiByte = data[7] -0x30;
               if(HiByte == 1) //输入功率
               {


                   dtinfo.fValue = atof(ResultData);
                   data_ptr->mValues[0] = dtinfo;
               }else if(HiByte == 2)//反射功率
               {
                   dtinfo.fValue = atof(ResultData);
                   data_ptr->mValues[1] = dtinfo;
               }
           }
           else if(Bmodel == 0x56) //各种电压值
           {
               HiByte = data[7] - 0x30;//参数编码序号,V01,V02,V03...
               dtinfo.fValue = atof(ResultData);
               data_ptr->mValues[2+HiByte] = dtinfo;

           }else if(Bmodel == 0x41)
           {
               dtinfo.fValue = atof(ResultData);
               data_ptr->mValues[12] = dtinfo;
           }
           data = data+fendpos+1;
           nlastlen = nlastlen-(fendpos+1);
       }
       if((data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue)<=0)
       {
           data_ptr->mValues[2].fValue = 0;
       }
       else
       {
           data_ptr->mValues[2].fValue = (data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue);
       }
       return RE_SUCCESS;
}
}
