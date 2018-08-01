#include "rvrtransmmit.h"
namespace hx_net{

RvrTransmmit::RvrTransmmit(Tsmt_message_ptr ptsmessage,int subprotocol,int addresscode)
    :Transmmiter()
    ,m_subprotocol(subprotocol)
    ,m_addresscode(addresscode)
    ,m_pmessage(ptsmessage)
{
     initOid();
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

void RvrTransmmit::rvr_Callback(int reason, Snmp *session, Pdu &pdu, SnmpTarget &target)
{
    switch(m_subprotocol)
    {
    case ETL3100:
        TEL3100_decode(reason,session,pdu,target);
        break;
    default:
        break;
    }
}

void RvrTransmmit::initOid()
{
    switch(m_subprotocol)
    {
    case ETL3100:
    {
        map_Oid["1.3.6.1.4.1.27874.3.2.13.1.2.9.1.465.1"] = 0;
        Vb vbl;
        vbl.set_oid(Oid("1.3.6.1.4.1.27874.3.2.13.1.2.9.1.465.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.27874.3.2.13.1.2.9.1.469.1"] = 1;
        vbl.set_oid(Oid("1.3.6.1.4.1.27874.3.2.13.1.2.9.1.469.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.27874.3.2.13.1.2.9.1.85.1"] = 3;
        vbl.set_oid(Oid("1.3.6.1.4.1.27874.3.2.13.1.2.9.1.85.1"));

        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.27874.3.2.13.1.2.9.1.89.1"] = 4;
        vbl.set_oid(Oid("1.3.6.1.4.1.27874.3.2.13.1.2.9.1.89.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.27874.3.2.13.1.2.9.1.93.1"] = 5;
        vbl.set_oid(Oid("1.3.6.1.4.1.27874.3.2.13.1.2.9.1.93.1"));
        query_pdu += vbl;
       // map_Oid["1.3.6.1.4.1.27874.3.2.13.1.2.9.1.349.1"] = 6;
       // vbl.set_oid(Oid("1.3.6.1.4.1.27874.3.2.13.1.2.9.1.349.1"));
        //query_pdu += vbl;
       // map_Oid["1.3.6.1.4.1.27874.3.2.13.1.2.9.1.353.1"] = 7;
       // vbl.set_oid(Oid("1.3.6.1.4.1.27874.3.2.13.1.2.9.1.353.1"));
        //query_pdu += vbl;
    }
        break;
    default:
        break;
    }
}

void rvr_aysnc_callback(int reason, Snmp *session,
                         Pdu &pdu, SnmpTarget &target, void *cd)
{
    if (cd)
        ((RvrTransmmit*)cd)->rvr_Callback(reason, session, pdu, target);
}


int RvrTransmmit::get_snmp(Snmp *snmp, DevMonitorDataPtr data_ptr, CTarget *target)
{
    {
        boost::recursive_mutex::scoped_lock lock(data_mutex);
        if(data_ptr)
            curdata_ptr = data_ptr;
    }
    int status = snmp->get(query_pdu,*target,rvr_aysnc_callback,this);
    if(status)
        return -1;

    return 0;
}

void RvrTransmmit::TEL3100_decode(int reason, Snmp *session, Pdu &pdu, SnmpTarget &target)
{
    int pdu_error = pdu.get_error_status();
    if (pdu_error)
    {
        std::cout<<"ERROR"<<endl;
        return;
    }
    if (pdu.get_vb_count() == 0)
    {

        std::cout<<"COUNT==0"<<endl;
        return;
    }
    int vbcount = pdu.get_vb_count();
    boost::recursive_mutex::scoped_lock lock(data_mutex);
    for(int i=0;i<vbcount;++i)
    {
        DataInfo dainfo;
        Vb nextVb;
        pdu.get_vb(nextVb, i);
        string cur_oid = nextVb.get_printable_oid();
        string cur_value =nextVb.get_printable_value();
        dainfo.bType = false;
        dainfo.fValue = atof(cur_value.c_str());
        map<string,int>::iterator iter = map_Oid.find(cur_oid);
        std::cout<<cur_oid<<endl;
        if(iter!=map_Oid.end())
        {
            curdata_ptr->mValues[(*iter).second] = dainfo;
        }
    }
    DataInfo vsrinfo;
    vsrinfo.bType = false;
    vsrinfo.fValue = 0;
    if(curdata_ptr->mValues.find(0)!=curdata_ptr->mValues.end() && curdata_ptr->mValues.find(1)!=curdata_ptr->mValues.end())
    {
        if(curdata_ptr->mValues[0].fValue>curdata_ptr->mValues[1].fValue)
            vsrinfo.fValue = sqrt((curdata_ptr->mValues[0].fValue+curdata_ptr->mValues[1].fValue)/(curdata_ptr->mValues[0].fValue-curdata_ptr->mValues[1].fValue));
       curdata_ptr->mValues[0].fValue = curdata_ptr->mValues[0].fValue*0.001;
    }
    curdata_ptr->mValues[2] = vsrinfo;
    m_pmessage->aysnc_data(curdata_ptr);
}


int RvrTransmmit::decode_msg_body(Snmp *snmp, DevMonitorDataPtr data_ptr, CTarget *target, int &runstate)
{
    switch(m_subprotocol)
    {
    case ETL3100:
        return get_snmp(snmp,data_ptr,target);
    }
    return RE_NOPROTOCOL;
}


}
