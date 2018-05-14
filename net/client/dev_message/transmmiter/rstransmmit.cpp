#include "rstransmmit.h"
namespace hx_net{
RsTransmmit::RsTransmmit(Tsmt_message_ptr ptsmessage,int subprotocol,int addresscode)
    :Transmmiter()
    ,m_subprotocol(subprotocol)
    ,m_addresscode(addresscode)
    ,m_pmessage(ptsmessage)
{
    switch (subprotocol) {
    case GER_DIG_ONE:
    case GER_DIG_TWO:
        initOid();
        break;
    default:
        break;
    }
}

int RsTransmmit::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
{
    switch(m_subprotocol)
    {
    case GERMANY_RS:
    {
        if(data[0]==0x02 && data[1]==0x66 && data[2]==0x65)
                return RE_SUCCESS;
        else if(data[0]==0x02 && data[1]==0x66 && data[2]==0x66)
            return RE_CMDACK;
        else
            return RE_HEADERROR;
    }
        break;
    }
    return RE_NOPROTOCOL;
}

int RsTransmmit::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    switch(m_subprotocol)
    {
    case GERMANY_RS:
        return GermanyRSData(data,data_ptr,nDataLen,runstate);
    }
    return RE_NOPROTOCOL;
}

int RsTransmmit::decode_msg_body(Snmp *snmp, DevMonitorDataPtr data_ptr, CTarget *target, int &runstate)
{
    switch(m_subprotocol)
    {
    case GER_DIG_ONE:
    case GER_DIG_TWO:
        return get_snmp(snmp,data_ptr,target);
    }
    return RE_NOPROTOCOL;
}

bool RsTransmmit::IsStandardCommand()
{
    return false;
}

void RsTransmmit::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
{

}

void RsTransmmit::GetAllCmd(CommandAttribute &cmdAll)
{
    switch(m_subprotocol)
    {
    case GERMANY_RS:
    {//0x02,0x36,0x32,0x30,0x31,0x31,0x38,0x37,0x42,0x03
        CommandUnit tmUnit;
        tmUnit.commandLen = 10;
        tmUnit.ackLen = 18;
        tmUnit.commandId[0] = 0x02;
        tmUnit.commandId[1] = 0x36;
        tmUnit.commandId[2] = 0x32;
        tmUnit.commandId[3] = 0x30;
        tmUnit.commandId[4] = 0x31;
        tmUnit.commandId[5] = 0x31;
        tmUnit.commandId[6] = 0x38;
        tmUnit.commandId[7] = 0x37;
        tmUnit.commandId[8] = 0x42;
        tmUnit.commandId[9] = 0x03;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.commandId[6] = 0x39;
        tmUnit.commandId[8] = 0x43;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.commandId[5] = 0x32;
        tmUnit.commandId[7] = 0x38;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.commandId[6] = 0x36;
        tmUnit.commandId[8] = 0x39;
        tmUnit.ackLen = 10;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.ackLen = 18;
        tmUnit.commandId[5] = 0x31;
        tmUnit.commandId[7] = 0x37;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.commandId[6] = 0x37;
        tmUnit.commandId[8] = 0x41;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.commandId[5] = 0x32;
        tmUnit.commandId[7] = 0x38;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.commandId[6] = 0x38;
        tmUnit.commandId[8] = 0x42;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.commandId[5] = 0x31;
        tmUnit.commandId[6] = 0x30;
        tmUnit.commandId[7] = 0x37;
        tmUnit.commandId[8] = 0x33;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.commandId[6] = 0x32;
        tmUnit.commandId[8] = 0x35;
        cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        tmUnit.commandId[1] = 0x30;
        tmUnit.commandId[2] = 0x30;
        tmUnit.commandId[5] = 0x34;
        tmUnit.commandId[6] = 0x30;
        tmUnit.commandId[7] = 0x34;
        tmUnit.commandId[8] = 0x31;
        cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
        tmUnit.commandId[5] = 0x31;
        tmUnit.commandId[7] = 0x31;
        cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
    }
        break;
    default:
        break;
    }
}

void RsTransmmit::rs_Callback(int reason, Snmp *session, Pdu &pdu, SnmpTarget &target)
{
    switch(m_subprotocol)
    {
    case GER_DIG_ONE:
    case GER_DIG_TWO:
        rs_decode(reason,session,pdu,target);
        break;
    default:
        break;
    }
}

void RsTransmmit::initOid()
{
    switch(m_subprotocol)
    {
    case GER_DIG_ONE:
    {
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.1.1"] = 0;
        Vb vbl;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.2.1"] = 1;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.2.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.6.1"] = 2;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.6.1"));

        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.1.1.1"] = 3;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.1.1.2"] = 4;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.1.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.1.1.1"] = 5;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.1.1.2"] = 6;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.1.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.1.1.1"] = 7;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.1.1.2"] = 8;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.1.1.2"));
        query_pdu += vbl;

        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.1.1.1"] = 9;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.1.1.2"] = 10;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.1.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.1.1.1"] = 11;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.1.1.2"] = 12;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.1.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.1.1.1"] = 13;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.1.1.2"] = 14;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.1.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.1.1.1"] = 15;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.1.1.2"] = 16;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.1.1.2"));
        query_pdu += vbl;
    }
        break;
    case GER_DIG_TWO:
    {
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.1.1"] = 0;
        Vb vbl;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.2.1"] = 1;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.2.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.6.1"] = 2;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.6.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.1.2"] = 3;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.2.2"] = 4;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.2.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.6.2"] = 5;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.1.3.1.6.2"));
        query_pdu += vbl;

        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.1.1.1"] = 6;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.1.1.2"] = 7;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.1.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.1.1.3"] = 8;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.1.1.3"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.2.1.1"] = 9;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.2.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.2.1.2"] = 10;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.2.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.2.1.3"] = 11;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.1.2.1.3"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.1.1.1"] = 12;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.1.1.2"] = 13;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.1.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.1.1.3"] = 14;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.1.1.3"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.2.1.1"] = 15;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.2.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.2.1.2"] = 16;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.2.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.2.1.3"] = 17;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.2.2.1.3"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.1.1.1"] = 18;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.1.1.2"] = 19;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.1.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.1.1.3"] = 20;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.1.1.3"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.2.1.1"] = 21;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.2.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.2.1.2"] = 22;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.2.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.2.1.3"] = 23;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.3.2.1.3"));
        query_pdu += vbl;


        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.1.1.1"] = 24;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.1.1.2"] = 25;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.1.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.1.1.3"] = 26;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.1.1.3"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.2.1.1"] = 27;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.2.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.2.1.2"] = 28;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.2.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.2.1.3"] = 29;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.4.2.1.3"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.1.1.1"] = 30;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.1.1.2"] = 31;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.1.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.1.1.3"] = 32;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.1.1.3"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.2.1.1"] = 33;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.2.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.2.1.2"] = 34;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.2.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.2.1.3"] = 35;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.5.2.1.3"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.1.1.1"] = 36;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.1.1.2"] = 37;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.1.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.1.1.3"] = 38;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.1.1.3"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.2.1.1"] = 39;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.2.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.2.1.2"] = 40;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.2.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.2.1.3"] = 41;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.6.2.1.3"));
        query_pdu += vbl;

        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.1.1.1"] = 42;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.1.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.1.1.2"] = 43;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.1.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.1.1.3"] = 44;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.1.1.3"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.2.1.1"] = 45;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.2.1.1"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.2.1.2"] = 46;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.2.1.2"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.2.1.3"] = 47;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.3.1.10.2.1.22.2.1.3"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.2566.127.1.2.216.101.1.1.2.1.2.1"] = 48;
        vbl.set_oid(Oid("1.3.6.1.4.1.2566.127.1.2.216.101.1.1.2.1.2.1"));
        query_pdu += vbl;
    }
        break;
    }
}
void rs_aysnc_callback(int reason, Snmp *session,
                         Pdu &pdu, SnmpTarget &target, void *cd)
{
    if (cd)
        ((RsTransmmit*)cd)->rs_Callback(reason, session, pdu, target);
}

int RsTransmmit::get_snmp(Snmp *snmp, DevMonitorDataPtr data_ptr, CTarget *target)
{
    {
        boost::recursive_mutex::scoped_lock lock(data_mutex);
        if(data_ptr)
            curdata_ptr = data_ptr;
    }
    int status = snmp->get(query_pdu,*target,rs_aysnc_callback,this);
    if(status)
        return -1;

    return 0;
}

int RsTransmmit::GermanyRSData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
{
    DataInfo dtinfo;
    dtinfo.bType = false;
    unsigned char *pctmp = data;
    unsigned char cdata1,cdata2,cdata3,cdata4;
    for(int i=0;i<10;++i)
    {
        if(i<3)
            pctmp = data+i*18;
        else
            pctmp = data+i*18-8;
        cdata1 = AsciiToInt(pctmp[13])*16+AsciiToInt(pctmp[14]);
        cdata2 = AsciiToInt(pctmp[11])*16+AsciiToInt(pctmp[12]);
        cdata3 = AsciiToInt(pctmp[9])*16+AsciiToInt(pctmp[10]);
        cdata4 = AsciiToInt(pctmp[7])*16+AsciiToInt(pctmp[8]);
        if(cdata1!=0 || cdata2!=0 || cdata3!=0 || cdata4!=0)
        {
            int iexponent = 150-(((cdata1 << 1) & 0xfe)+((cdata2>>7)&0x01));
            unsigned int uidata = ((cdata2&0x7F)<<16)+(1<<23)+(cdata3<<8)+cdata4;
            if((cdata1>>7)&0x01)
            {
                dtinfo.fValue = (-1)*float(uidata)/pow((float)2,(float)iexponent);
            }
            else
                dtinfo.fValue = float(uidata)/pow((float)2,(float)iexponent);
        }
        else
            dtinfo.fValue = 0;
        int index = AsciiToInt(pctmp[5])*10+AsciiToInt(pctmp[6]);
        switch(index)
        {
        case 18:
        {
            //下面的步骤是修改输出功率的单位的,精确度位0.1
            dtinfo.fValue = dtinfo.fValue*0.001;
            data_ptr->mValues[0] = dtinfo; //输出功率
        }
            break;
        case 26:
        {
            data_ptr->mValues[1] = dtinfo;
            if(data_ptr->mValues[1].fValue == 0)
            {
                dtinfo.fValue = 1.0;
                data_ptr->mValues[2] = dtinfo;
            }
            else
            {
                dtinfo.fValue = (sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue)));
                data_ptr->mValues[2] = dtinfo;
            }

        }
            break;
        case 19:
        {
            dtinfo.fValue = dtinfo.fValue*0.001;
            data_ptr->mValues[3] = dtinfo;
        }
            break;
        case 29:
            data_ptr->mValues[4] = dtinfo;
            break;
        case 16:
            data_ptr->mValues[5] = dtinfo;
            break;
        case 17:
            data_ptr->mValues[6] = dtinfo;
            break;
        case 27:
            data_ptr->mValues[7] = dtinfo;
            break;
        case 28:
            data_ptr->mValues[8] = dtinfo;
            break;
        case 10:
            data_ptr->mValues[9] = dtinfo;
            break;
        case 12:
            data_ptr->mValues[10] = dtinfo;
            break;
        default:
            break;
        }
    }
    return RE_SUCCESS;
}

void RsTransmmit::rs_decode(int reason, Snmp *session, Pdu &pdu, SnmpTarget &target)
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
    m_pmessage->aysnc_data(curdata_ptr);
}

}
