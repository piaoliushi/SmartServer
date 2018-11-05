#include "Link_message.h"
#include"../../../utility.h"
//#include "./snmp_pp/snmp_pp.h"

#define NUM_SYS_VBS	6
//信号电平
#define rflevel	"1.3.6.1.4.1.8201.5.7.1.1.4.0"
//锁定状态
#define signalLock	"1.3.6.1.4.1.8201.5.7.1.1.1.0"
//误码率
#define singalber	"1.3.6.1.4.1.8201.5.7.1.1.2.0"
//载噪比
#define signalcn	"1.3.6.1.4.1.8201.5.7.1.1.3.0"
//统计码率
#define totalrate    "1.3.6.1.4.1.8201.5.7.1.1.5.0"
//频率
#define frequency	"1.3.6.1.4.1.8201.5.7.1.1.7.0"
#include <QString>
namespace hx_net
{

    Link_message::Link_message(session_ptr pSession,DeviceInfo &devInfo)
        :m_pSession(pSession)
        ,d_devInfo(devInfo)
        //,d_task_queue_ptr(new TaskQueue< DevMonitorDataPtr >)
    {
        initOid();
        if(IsStandardCommand())
            d_curData_ptr = DevMonitorDataPtr(new Data);
    }

    Link_message::~Link_message(void)
    {
    }
    void Link_message::initOid()
    {
        switch (d_devInfo.nSubProtocol) {
        case LINK_SING_NET_ADAPTER:{
            init_SingApt_Oid();
        }
            break;
        case LINK_DMP_SWITCH:
            init_Dmp_Oid();
            break;
        case LINK_ASI_ADAPTER:
            init_AsiApt_Oid();
            break;
        case LINK_WEILE_AVSP_DECODER:
            init_weile_avsp_decoder_Oid();
            break;
        case LINK_WEILE_AVSP_ADAPTER:
            init_avsApt_Oid();
            break;
        case LINK_SMSX_ASI_ADAPTER:
            init_smsx_asi_Oid();
            break;
        case LINK_SMSX_ASI_ENCODER:
            init_smsx_asi_decoder_Oid();
            break;
        case LINK_NORMAL_SNMP_DEV:
            init_normal_snmp_oid();
            break;
        default:
            break;
        }
    }

    void Link_message::init_SingApt_Oid()
    {
        map_Oid["1.3.6.1.4.1.8201.5.12.1.1"] = 0;
        Vb vbl;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.1.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.2"] = 1;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.2.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.3"] = 2;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.3.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.4"] = 3;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.4.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.5"] = 4;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.5.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.6"] = 5;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.6.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.7"] = 6;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.7.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.8"] = 7;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.8.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.9"] = 8;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.9.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.10"] = 9;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.10.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.11"] = 10;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.11.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.12"] = 11;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.12.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.13"] = 12;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.13.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.14"] = 13;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.14.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.15"] = 14;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.15.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.12.1.16"] = 15;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.12.1.16.0"));
        query_pdu += vbl;
    }

    void Link_message::init_Dmp_Oid()
    {
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.1.3"] = 0;
        Vb vbl;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.1.3.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.1.4"] = 1;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.1.4.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.2.3"] = 2;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.2.3.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.2.4"] = 3;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.2.4.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.3.3"] = 4;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.3.3.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.3.4"] = 5;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.3.4.0"));
        query_pdu += vbl;
    }

    void Link_message::init_AsiApt_Oid()
    {
        Vb vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.1.3"] = 0;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.1.3.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.1.4"] = 1;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.1.4.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.2.3"] = 2;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.2.3.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.2.4"] = 3;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.2.4.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.3.3"] = 4;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.3.3.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.3.4"] = 5;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.3.4.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.4.3"] = 6;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.4.3.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.4.4"] = 7;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.4.4.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.5.3"] = 8;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.5.3.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.5.4"] = 9;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.5.4.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.6.3"] = 10;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.6.3.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.6.4"] = 11;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.6.4.0"));
        query_pdu += vbl;
    }

    void Link_message::init_avsApt_Oid()
    {
        Vb vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.1.3"] = 0;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.1.3.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.1.4"] = 1;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.1.4.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.2.3"] = 2;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.2.3.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.2.4"] = 3;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.2.4.0"));
        query_pdu += vbl;
        map_Oid["1.3.6.1.4.1.8201.5.11.1.1.4.3"] = 4;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.11.1.1.4.3.0"));
        query_pdu += vbl;
    }

    void Link_message::init_weile_avsp_decoder_Oid(){
        Vb vbl;
        //锁定状态
        map_Oid["1.3.6.1.4.1.8201.5.7.1.1"] = 0;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.7.1.1.0"));
        query_pdu += vbl;
        //误码率
        map_Oid["1.3.6.1.4.1.8201.5.7.1.2"] = 1;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.7.1.2.0"));
        query_pdu += vbl;
        //C/N载噪比
        map_Oid["1.3.6.1.4.1.8201.5.7.1.3"] = 2;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.7.1.3.0"));
        query_pdu += vbl;
        //输入1总码率
        map_Oid["1.3.6.1.4.1.8201.5.7.1.4"] = 3;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.7.1.4.0"));
        query_pdu += vbl;
        //输入1有效码率
        map_Oid["1.3.6.1.4.1.8201.5.7.1.5"] = 4;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.7.1.5.0"));
        query_pdu += vbl;
        //ASI1输出总码率
        map_Oid["1.3.6.1.4.1.8201.5.7.1.36"] = 5;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.7.1.36.0"));
        query_pdu += vbl;
        //ASI1输出有效码率
        map_Oid["1.3.6.1.4.1.8201.5.7.1.37"] = 6;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.7.1.37.0"));
        query_pdu += vbl;
        //ASI2输出总码率
        map_Oid["1.3.6.1.4.1.8201.5.7.1.38"] = 7;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.7.1.38.0"));
        query_pdu += vbl;
        //ASI2输出有效码率
        map_Oid["1.3.6.1.4.1.8201.5.7.1.39"] = 8;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.7.1.39.0"));
        query_pdu += vbl;

        //IP1输出总码率
        map_Oid["1.3.6.1.4.1.8201.5.7.1.40"] = 9;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.7.1.40.0"));
        query_pdu += vbl;
        //IP1输出有效码率
        map_Oid["1.3.6.1.4.1.8201.5.7.1.41"] = 10;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.7.1.41.0"));
        query_pdu += vbl;
        //IP2输出总码率
        map_Oid["1.3.6.1.4.1.8201.5.7.1.42"] = 11;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.7.1.42.0"));
        query_pdu += vbl;
        //IP2输出有效码率
        map_Oid["1.3.6.1.4.1.8201.5.7.1.43"] = 12;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.7.1.43.0"));
        query_pdu += vbl;

        //接收频率
        map_Oid["1.3.6.1.4.1.8201.5.7.2.1.2.1.1"] = 13;
        vbl.set_oid(Oid("1.3.6.1.4.1.8201.5.7.2.1.2.1.1.0"));
        query_pdu += vbl;
    }

    int Link_message::decode_0401AV(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
    {
        // aa 41 11 00 01 00 06 00 00 33 10 43 55
        iaddcode = data[3]*256+data[4];
        DataInfo dainfo;
        int ichanel;
        ichanel = data[7];
        dainfo.bType = true;
        for(int i=0;i<4;++i)
        {
            dainfo.fValue = 0;
            data_ptr->mValues[i] = dainfo;
        }
        if(ichanel>=0 && ichanel<4)
        {
            data_ptr->mValues[ichanel].fValue = 1.0;
        }


        if(data[8]==0x00)
        {
            dainfo.fValue = 1.0;
            data_ptr->mValues[4] = dainfo;
            dainfo.fValue = 0;
            data_ptr->mValues[5] = dainfo;
        }
        else
        {
            dainfo.fValue = 1.0;
            data_ptr->mValues[5] = dainfo;
            dainfo.fValue = 0;
            data_ptr->mValues[4] = dainfo;
        }
        int inputstate = data[9];
        for(int i=0;i<8;++i)
        {
           dainfo.fValue = Getbit(inputstate,i);
           data_ptr->mValues[6+i] = dainfo;
        }
        dainfo.bType = false;
        dainfo.fValue = data[10];
        data_ptr->mValues[14] = dainfo;
        return 0;
    }

    int Link_message::decode_0401DA(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
    {
        iaddcode = data[3]*256+data[4];
        DataInfo dainfo;
        int ichanel = data[7];
        dainfo.bType = true;
        for(int i=0;i<4;++i)
        {
            dainfo.fValue = 0;
            data_ptr->mValues[i] = dainfo;
        }
        data_ptr->mValues[15] = dainfo;
        if(ichanel>=0 && ichanel<4)
        {
            data_ptr->mValues[ichanel].fValue = 1.0;
        }
        if(ichanel==4)
            data_ptr->mValues[15].fValue = 1.0;
        if(data[8]==0x00)
        {
            dainfo.fValue = 1.0;
            data_ptr->mValues[4] = dainfo;
            dainfo.fValue = 0;
            data_ptr->mValues[5] = dainfo;
        }
        else
        {
            dainfo.fValue = 1.0;
            data_ptr->mValues[5] = dainfo;
            dainfo.fValue = 0;
            data_ptr->mValues[4] = dainfo;
        }
        int inputstate = data[9];
        for(int i=0;i<4;++i)
        {
           dainfo.fValue = Getbit(inputstate,i)==0 ? 1:0;
           data_ptr->mValues[6+i] = dainfo;
        }
        inputstate = data[10];
        dainfo.bType = false;
        int ornum=0;
        for(int i=0;i<4;++i)
        {
            ornum = int(3*pow(4.0,i));
            dainfo.fValue = ((inputstate&ornum)>>2*i);
            data_ptr->mValues[10+i] = dainfo;
        }
        dainfo.fValue = data[11];
        data_ptr->mValues[14] = dainfo;
        dainfo.fValue = data[22];
        data_ptr->mValues[16] = dainfo;
        dainfo.fValue = data[21];
        data_ptr->mValues[17] = dainfo;
        return 0;
    }

    int Link_message::decode_0401DABS(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
    {
        iaddcode = data[3]*256+data[4];
        int nCmd = data[2];
        DataInfo dainfo;
        if(nCmd!=0x77)
        {
            int ichanel = data[7];
            dainfo.bType = true;
            for(int i=0;i<4;++i)
            {
                dainfo.fValue = 0;
                data_ptr->mValues[i] = dainfo;
            }
            data_ptr->mValues[15] = dainfo;
            if(ichanel>=0 && ichanel<4)
            {
                data_ptr->mValues[ichanel].fValue = 1.0;
            }
            if(ichanel==4)
                data_ptr->mValues[15].fValue = 1.0;
            if(data[8]==0x00)
            {
                dainfo.fValue = 1.0;
                data_ptr->mValues[4] = dainfo;
                dainfo.fValue = 0;
                data_ptr->mValues[5] = dainfo;
            }
            else
            {
                dainfo.fValue = 1.0;
                data_ptr->mValues[5] = dainfo;
                dainfo.fValue = 0;
                data_ptr->mValues[4] = dainfo;
            }
            int inputstate = data[9];
            for(int i=0;i<4;++i)
            {
               dainfo.fValue = Getbit(inputstate,i)==0 ? 1:0;
               data_ptr->mValues[6+i] = dainfo;
            }
            inputstate = data[10];
            dainfo.bType = false;
            int ornum=0;
            for(int i=0;i<4;++i)
            {
                ornum = int(3*pow(4.0,i));
                dainfo.fValue = ((inputstate&ornum)>>2*i);
                data_ptr->mValues[10+i] = dainfo;
            }
            dainfo.fValue = data[11];
            data_ptr->mValues[14] = dainfo;
        }
        else
        {
            dainfo.bType = false;
            dainfo.fValue = data[8];
            data_ptr->mValues[16] = dainfo;
            dainfo.fValue = data[7];
            data_ptr->mValues[17] = dainfo;
            dainfo.fValue = data[10];
            data_ptr->mValues[18] = dainfo;
            dainfo.fValue = data[9];
            data_ptr->mValues[19] = dainfo;
            dainfo.fValue = data[12];
            data_ptr->mValues[20] = dainfo;
            dainfo.fValue = data[11];
            data_ptr->mValues[21] = dainfo;
            dainfo.fValue = data[14];
            data_ptr->mValues[22] = dainfo;
            dainfo.fValue = data[13];
            data_ptr->mValues[23] = dainfo;
            dainfo.fValue = data[16];
            data_ptr->mValues[24] = dainfo;
            dainfo.fValue = data[15];
            data_ptr->mValues[25] = dainfo;
        }
        return 0;
    }

    void Link_message::GetSwitchCmd(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
    {
        switch (d_devInfo.nSubProtocol){
        case LINK_HX_0401_AV:{
            if(lpParam->cparams().size()<1)
                return;
            int nChannel = atoi(lpParam->cparams(0).sparamvalue().c_str());
            cmdUnit.commandLen = 13;
            cmdUnit.ackLen = 0;
            cmdUnit.commandId[0] = 0xAA;
            cmdUnit.commandId[1] = 0x41;
            cmdUnit.commandId[2] = 0x22;
            cmdUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            cmdUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            cmdUnit.commandId[5] = 0x00;
            cmdUnit.commandId[6] = 0x06;
            cmdUnit.commandId[7] = nChannel;
            cmdUnit.commandId[8] = 0x00;
            cmdUnit.commandId[9] = 0x00;
            cmdUnit.commandId[10] = 0x00;
            cmdUnit.commandId[11] = nChannel;
            cmdUnit.commandId[12] = 0x55;
        }
            break;
        case LINK_HX_0401_DA:
        case LINK_HX_0401_DABS:
        {
            if(lpParam->cparams().size()<1)
                return;
            int nChannel = atoi(lpParam->cparams(0).sparamvalue().c_str());
            cmdUnit.commandLen = 14;
            cmdUnit.ackLen = 7;
            cmdUnit.commandId[0] = 0xAA;
            cmdUnit.commandId[1] = 0x45;
            cmdUnit.commandId[2] = 0x22;
            cmdUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            cmdUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            cmdUnit.commandId[5] = 0x00;
            cmdUnit.commandId[6] = 0x07;
            cmdUnit.commandId[7] = nChannel;
            cmdUnit.commandId[8] = 0x00;
            cmdUnit.commandId[9] = 0x00;
            cmdUnit.commandId[10] = 0x00;
            cmdUnit.commandId[11] = 0x00;
            cmdUnit.commandId[12] = nChannel;
            cmdUnit.commandId[13] = 0x55;
        }
            break;
        case LINK_HX_0214_DA:
        {
            if(lpParam->cparams().size()<2)
                return;
            cmdUnit.commandLen = 10;
            cmdUnit.commandId[0] = 0x7E;
            cmdUnit.commandId[1] = 0x40;
            cmdUnit.commandId[2] = 0x66;
            cmdUnit.commandId[3] = (d_devInfo.iAddressCode&0x00FF);
            cmdUnit.commandId[4] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            cmdUnit.commandId[5] = 0x03;
            cmdUnit.commandId[6] = 0x00;
            cmdUnit.commandId[7] = atoi(lpParam->cparams(0).sparamvalue().c_str());
            cmdUnit.commandId[8] = atoi(lpParam->cparams(1).sparamvalue().c_str());
            cmdUnit.commandId[9] = 0x55;
        }
            break;
        case LINK_HX_0401_SP:
       {
           if(lpParam->cparams().size()<1)
               return;
           cmdUnit.commandLen = 6;
           cmdUnit.commandId[0] = 0xAA;
           cmdUnit.commandId[1] = 0x22;
           cmdUnit.commandId[2] = atoi(lpParam->cparams(0).sparamvalue().c_str());
           cmdUnit.commandId[3] = 0x00;
           cmdUnit.commandId[4] = cmdUnit.commandId[1]^cmdUnit.commandId[2];
           cmdUnit.commandId[5] = 0x55;
       }
           break;
        case LINK_JC_5103:
        {
            if(lpParam->cparams().size()<1)
                return;
            int nChan = atoi(lpParam->cparams(0).sparamvalue().c_str());
            GetSwitchCmdByChannel(nChan,cmdUnit);
        }
            break;
        default:
            break;
        }
    }

    void Link_message::GetSwitchCmdByChannel(int channelId, CommandUnit &cmdUnit)
    {
        switch (d_devInfo.nSubProtocol){
        case LINK_HX_0401_AV:{
            cmdUnit.commandLen = 13;
            cmdUnit.ackLen = 0;
            cmdUnit.commandId[0] = 0xAA;
            cmdUnit.commandId[1] = 0x41;
            cmdUnit.commandId[2] = 0x22;
            cmdUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            cmdUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            cmdUnit.commandId[5] = 0x00;
            cmdUnit.commandId[6] = 0x06;
            cmdUnit.commandId[7] = channelId;
            cmdUnit.commandId[8] = 0x00;
            cmdUnit.commandId[9] = 0x00;
            cmdUnit.commandId[10] = 0x00;
            cmdUnit.commandId[11] = channelId;
            cmdUnit.commandId[12] = 0x55;
        }
            break;
        case LINK_HX_0401_DA:
        case LINK_HX_0401_DABS:
        {
            cmdUnit.commandLen = 14;
            cmdUnit.ackLen = 7;
            cmdUnit.commandId[0] = 0xAA;
            cmdUnit.commandId[1] = 0x45;
            cmdUnit.commandId[2] = 0x22;
            cmdUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            cmdUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            cmdUnit.commandId[5] = 0x00;
            cmdUnit.commandId[6] = 0x07;
            cmdUnit.commandId[7] = channelId;
            cmdUnit.commandId[8] = 0x00;
            cmdUnit.commandId[9] = 0x00;
            cmdUnit.commandId[10] = 0x00;
            cmdUnit.commandId[11] = 0x00;
            cmdUnit.commandId[12] = channelId;
            cmdUnit.commandId[13] = 0x55;
        }
            break;
        case LINK_HX_0401_SP:
        {
            cmdUnit.commandLen = 6;
            cmdUnit.commandId[0] = 0xAA;
            cmdUnit.commandId[1] = 0x22;
            cmdUnit.commandId[2] = channelId;
            cmdUnit.commandId[3] = 0x00;
            cmdUnit.commandId[4] = cmdUnit.commandId[1]^cmdUnit.commandId[2];
            cmdUnit.commandId[5] = 0x55;
        }
            break;
        case LINK_JC_5103:
        {
            if(channelId<3)
            {
                cmdUnit.commandLen = 14;
                cmdUnit.commandId[0] = 0x01;
                cmdUnit.commandId[1] = d_devInfo.iAddressCode;
                cmdUnit.commandId[2] = 0x30;
                cmdUnit.commandId[3] = 0x30;
                cmdUnit.commandId[4] = 0x57;
                cmdUnit.commandId[5] = 0x53;
                cmdUnit.commandId[6] = 0x2B;
                unsigned char chadd[10]={'0','0','0','0','0','0','0','0','0','\0'};

                IntToAscii(channelId,&chadd[0]);
                cmdUnit.commandId[7] = 0x30;
                cmdUnit.commandId[8] = chadd[0];
                cmdUnit.commandId[9] = 0x20;
                cmdUnit.commandId[10] = 0xFE;
                int ixor=0;
                for(int i=1;i<11;++i)
                {
                    ixor = ixor^cmdUnit.commandId[i];
                }
                IntToAscii(ixor,&chadd[0]);
                cmdUnit.commandId[11] = chadd[1];
                cmdUnit.commandId[12] = chadd[0];
                cmdUnit.commandId[13] = 0x04;
            }
        }
            break;
        default:
            break;
        }
    }

    void Link_message::DAS_5103_ACK(unsigned char *data)
    {
        CommandUnit tmUnit;
        tmUnit.commandLen = 9;
        tmUnit.commandId[0] = 0x01;
        tmUnit.commandId[1] = data[1];
        tmUnit.commandId[2] = data[2];
        tmUnit.commandId[3] = data[3];
        tmUnit.commandId[4] = 0x53;
        tmUnit.commandId[5] = 0x43;
        int checknum = (tmUnit.commandId[1]^tmUnit.commandId[4]^tmUnit.commandId[5]^data[2]^data[3])&0xFF;
        unsigned char ccheck[3]={'0'};
        IntToAscii(checknum,&ccheck[0]);
        tmUnit.commandId[6] = ccheck[1];
        tmUnit.commandId[7] = ccheck[0];
        tmUnit.commandId[8] = 0x04;
        e_ErrorCode eErrCode;
        m_pSession->send_cmd_to_dev(tmUnit,eErrCode);
    }

    void Link_message::GetControlModCmdByModId(int modId, CommandUnit &cmdUnit)
    {
        switch (d_devInfo.nSubProtocol){
        case LINK_HX_0401_AV:
        {
            cmdUnit.commandLen = 13;
            cmdUnit.ackLen = 0;
            cmdUnit.commandId[0] = 0xAA;
            cmdUnit.commandId[1] = 0x41;
            cmdUnit.commandId[2] = 0x66;
            cmdUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            cmdUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            cmdUnit.commandId[5] = 0x00;
            cmdUnit.commandId[6] = 0x06;
            cmdUnit.commandId[7] = 0x00;
            cmdUnit.commandId[8] = modId;
            cmdUnit.commandId[9] = 0x00;
            cmdUnit.commandId[10] = 0x00;
            cmdUnit.commandId[11] = modId;
            cmdUnit.commandId[12] = 0x55;
        }
            break;
        case LINK_HX_0401_DA:
        case LINK_HX_0401_DABS:
        {
            cmdUnit.commandLen = 14;
            cmdUnit.ackLen = 0;
            cmdUnit.commandId[0] = 0xAA;
            cmdUnit.commandId[1] = 0x45;
            cmdUnit.commandId[2] = 0x66;
            cmdUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            cmdUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            cmdUnit.commandId[5] = 0x00;
            cmdUnit.commandId[6] = 0x07;
            cmdUnit.commandId[7] = 0x00;
            cmdUnit.commandId[8] = modId;
            cmdUnit.commandId[9] = 0x00;
            cmdUnit.commandId[10] = 0x00;
            cmdUnit.commandId[11] = 0x00;
            cmdUnit.commandId[12] = modId;
            cmdUnit.commandId[13] = 0x55;
        }
            break;
        case LINK_HX_0401_SP:
       {
           cmdUnit.commandLen = 6;
           cmdUnit.commandId[0] = 0xAA;
           cmdUnit.commandId[1] = 0xF1;
           cmdUnit.commandId[2] = 0x00;
           cmdUnit.commandId[3] = modId;
           cmdUnit.commandId[4] = cmdUnit.commandId[1]^cmdUnit.commandId[2];
           cmdUnit.commandId[5] = 0x55;

       }
           break;
        default:
            break;
        }
    }

    void Link_message::GetControlModCmd(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
    {
        switch (d_devInfo.nSubProtocol){
        case LINK_HX_0401_AV:
        {
            if(lpParam->cparams().size()<1)
                return;
            int nMode = atoi(lpParam->cparams(0).sparamvalue().c_str());
            cmdUnit.commandLen = 13;
            cmdUnit.ackLen = 0;
            cmdUnit.commandId[0] = 0xAA;
            cmdUnit.commandId[1] = 0x41;
            cmdUnit.commandId[2] = 0x66;
            cmdUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            cmdUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            cmdUnit.commandId[5] = 0x00;
            cmdUnit.commandId[6] = 0x06;
            cmdUnit.commandId[7] = 0x00;
            cmdUnit.commandId[8] = nMode;
            cmdUnit.commandId[9] = 0x00;
            cmdUnit.commandId[10] = 0x00;
            cmdUnit.commandId[11] = nMode;
            cmdUnit.commandId[12] = 0x55;
        }
            break;
        case LINK_HX_0401_DA:
        case LINK_HX_0401_DABS:
        {
            if(lpParam->cparams().size()<1)
                return;
            int nMode = atoi(lpParam->cparams(0).sparamvalue().c_str());
            cmdUnit.commandLen = 14;
            cmdUnit.ackLen = 0;
            cmdUnit.commandId[0] = 0xAA;
            cmdUnit.commandId[1] = 0x45;
            cmdUnit.commandId[2] = 0x66;
            cmdUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            cmdUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            cmdUnit.commandId[5] = 0x00;
            cmdUnit.commandId[6] = 0x07;
            cmdUnit.commandId[7] = 0x00;
            cmdUnit.commandId[8] = nMode;
            cmdUnit.commandId[9] = 0x00;
            cmdUnit.commandId[10] = 0x00;
            cmdUnit.commandId[11] = 0x00;
            cmdUnit.commandId[12] = nMode;
            cmdUnit.commandId[13] = 0x55;
        }
            break;
        case LINK_HX_0401_SP:
       {
           if(lpParam->cparams().size()<1)
               return;
           cmdUnit.commandLen = 6;
           cmdUnit.commandId[0] = 0xAA;
           cmdUnit.commandId[1] = 0xF1;
           cmdUnit.commandId[2] = 0x00;
           cmdUnit.commandId[3] = atoi(lpParam->cparams(0).sparamvalue().c_str());
           cmdUnit.commandId[4] = cmdUnit.commandId[1]^cmdUnit.commandId[2];
           cmdUnit.commandId[5] = 0x55;
       }
           break;
        case LINK_HX_6300_AD:{
           if(lpParam->cparams().size()<=1)
               return;
           cmdUnit.ackLen = 0;
           cmdUnit.commandLen = 14;
           cmdUnit.commandId[0] = 0xAA;
           cmdUnit.commandId[1] = 0x63;
           cmdUnit.commandId[2] = 0x22;
           cmdUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
           cmdUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
           cmdUnit.commandId[5] = 0x00;
           cmdUnit.commandId[6] = 0x07;
           cmdUnit.commandId[7] = atoi(lpParam->cparams(0).sparamvalue().c_str());
           cmdUnit.commandId[8] = atoi(lpParam->cparams(1).sparamvalue().c_str());
           cmdUnit.commandId[9] = 0x00;
           cmdUnit.commandId[10] = 0x00;
           cmdUnit.commandId[11] = 0x00;
           cmdUnit.commandId[12] = cmdUnit.commandId[7]+cmdUnit.commandId[8];
           cmdUnit.commandId[13] = 0x55;
            }
        break;
        case LINK_HX_9020:
            break;
        default:
            break;
        }
    }

    int Link_message::check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number)
    {
        switch (d_devInfo.nSubProtocol){
        case LINK_HX_0401_AV:{
            if(data[0]==0xAA && data[1]==0x41)
                return (data[5]*256+data[6]);
            else
                return RE_HEADERROR;
        }
        case LINK_HX_0401_DA:
        case LINK_HX_0401_DABS:
        {
            if(data[0]==0xAA && data[1]==0x45)
                return (data[5]*256+data[6]);
            else
                return RE_HEADERROR;
        }
        case LINK_HX_0214_DA:
        {
            if(data[0]==0x7E && data[1]==0x40)
                return (data[6]*256+data[5]);
            else
                return RE_HEADERROR;
        }
        case LINK_HX_0401_SP:
        {
            if(data[0]==0xAA)
                return 0;
            else
            {
                unsigned char cDes[1]={0};
                cDes[0]=0xAA;
                return kmp(data,nDataLen,cDes,1);
            }
        }
            break;
        case LINK_JC_5103://&& data[1]==d_devInfo.iAddressCode
        {
            if(data[0]==0x01)
                return 0;
            else
            {
               /* unsigned char cDes[6]={0};
                cDes[0]=0x01;
            //    cDes[1]=d_devInfo.iAddressCode;
                cDes[2]=0x30;
                cDes[3]=0x30;
                cDes[4]=0x52;
                cDes[5]=0x53;
                return kmp(data,nDataLen,cDes,1);*/
                return RE_HEADERROR;
            }
        }
            break;
        case LINK_HX_6300:
        case LINK_HX_6300_AD:
        {
               if(data[0]==0xAA && data[1]==0x63)
                   return (data[5]*256+data[6]);
               else
                   return RE_HEADERROR;
           }
        case LINK_HX_9020:{
               if(data[0]==0xAA && data[1]==0x92)
                   return (data[5]*256+data[6]);
               else
                   return RE_HEADERROR;
           }
            break;
        default:
            break;
        }
        return RE_UNKNOWDEV;
    }

    int Link_message::decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target)
     {

         if(data_ptr!=NULL)
             d_curData_ptr = data_ptr;

         switch(d_devInfo.nDevProtocol)
         {
         case LINK_DEVICE:
             {
                 switch (d_devInfo.nSubProtocol)
                 {
                 case LINK_STATELITE_RECEVIE:
                     return parse_SatelliteReceive_data(snmp,d_curData_ptr,target);
                 case LINK_TEST_RECEVIE:
                     return parse_TestReceive_data(snmp,d_curData_ptr,target);
                 case LINK_SING_NET_ADAPTER:          
                 case LINK_DMP_SWITCH: 
                 case LINK_ASI_ADAPTER:
                 case LINK_WEILE_AVSP_DECODER:
                 case LINK_WEILE_AVSP_ADAPTER:
                 case LINK_SMSX_ASI_ADAPTER:
                 case LINK_SMSX_ASI_ENCODER:
                 case LINK_NORMAL_SNMP_DEV:
                     return parse_SingAptReceive_data(snmp,d_curData_ptr,target);
                 default:
                     return RE_NOPROTOCOL;
                 }
             }
             break;
         }
         return RE_UNKNOWDEV;
     }

    int Link_message::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode)
    {
        if(data_ptr!=NULL)
            d_curData_ptr = data_ptr;
        int idecresult = RE_UNKNOWDEV;
        switch (d_devInfo.nSubProtocol) {
        case LINK_HX_0401_AV:
            idecresult = decode_0401AV(data,d_curData_ptr,nDataLen,iaddcode);
            break;
        case LINK_HX_0401_DA:
            idecresult = decode_0401DA(data,d_curData_ptr,nDataLen,iaddcode);
            break;
        case LINK_HX_0401_DABS:
            idecresult = decode_0401DABS(data,d_curData_ptr,nDataLen,iaddcode);
            break;
        case LINK_HX_0214_DA:
            idecresult = decode_0214DA(data,d_curData_ptr,nDataLen,iaddcode);
            break;
        case LINK_HX_0401_SP:
            idecresult = decode_0401SP(data,d_curData_ptr,nDataLen,iaddcode);
            break;
        case LINK_JC_5103:
            idecresult = decode_JC5103(data,d_curData_ptr,nDataLen,iaddcode);
            break;
        case LINK_HX_6300:
            idecresult = decode_6300(data,d_curData_ptr,nDataLen,iaddcode);
            break;
        case LINK_HX_6300_AD:
            idecresult = decode_6300_AD(data,d_curData_ptr,nDataLen,iaddcode);
            break;
        case LINK_HX_9020:
            idecresult = decode_9020(data,d_curData_ptr,nDataLen,iaddcode);
            break;
        default:
            break;
        }
        if(idecresult == 0 ) {
            GetResultData(d_curData_ptr);
            if(IsStandardCommand()){
                m_pSession->start_handler_data(iaddcode,d_curData_ptr);
            }
        }
        return idecresult;
    }

    bool Link_message::IsStandardCommand()
    {
        switch (d_devInfo.nSubProtocol){
        case LINK_HX_0401_AV:
        case LINK_HX_0401_DA:
        case LINK_HX_0401_DABS:
        case LINK_HX_0214_DA:
        case LINK_HX_6300:
        case LINK_HX_6300_AD:
        case LINK_HX_9020:
            return true;
        }
        return false;
    }

    void Link_message::GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit)
    {
    }

    void Link_message::GetSignalCommand(map<int, string> mapParam, CommandUnit &cmdUnit)
    {
        switch (d_devInfo.nSubProtocol){
        case LINK_HX_6300:
        case LINK_HX_6300_AD:
        {
            if(mapParam.size()<=1)
                break;
            cmdUnit.ackLen = 0;
            cmdUnit.commandLen = 14;
            cmdUnit.commandId[0] = 0xAA;
            cmdUnit.commandId[1] = 0x63;
            cmdUnit.commandId[2] = 0x22;
            cmdUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            cmdUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            cmdUnit.commandId[5] = 0x00;
            cmdUnit.commandId[6] = 0x07;
            cmdUnit.commandId[7] = atoi(mapParam[0].c_str());
            cmdUnit.commandId[8] = atoi(mapParam[1].c_str());
            cmdUnit.commandId[9] = 0x00;
            cmdUnit.commandId[10] = 0x00;
            cmdUnit.commandId[11] = 0x00;
            cmdUnit.commandId[12] = cmdUnit.commandId[7]+cmdUnit.commandId[8];
            cmdUnit.commandId[13] = 0x55;
            break;
        }
        case LINK_HX_9020:
        {
            if(mapParam.size()<=1)
                break;
            cmdUnit.ackLen = 0;
            cmdUnit.commandLen = 14;
            cmdUnit.commandId[0] = 0xAA;
            cmdUnit.commandId[1] = 0x92;
            cmdUnit.commandId[2] = 0x66;
            cmdUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            cmdUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            cmdUnit.commandId[5] = 0x00;
            cmdUnit.commandId[6] = 0x07;
            cmdUnit.commandId[7] = atoi(mapParam[0].c_str());
            cmdUnit.commandId[8] = atoi(mapParam[1].c_str());
            cmdUnit.commandId[9] = 0x00;
            cmdUnit.commandId[10] = 0x00;
            cmdUnit.commandId[11] = 0x00;
            cmdUnit.commandId[12] = cmdUnit.commandId[7]+cmdUnit.commandId[8];
            cmdUnit.commandId[13] = 0x55;
        }
            break;
        default:
            break;
        }
    }


    void Link_message::GetAllCmd( CommandAttribute &cmdAll )
    {
        switch (d_devInfo.nSubProtocol){
        case LINK_HX_0401_AV:{
            CommandUnit tmUnit;
            tmUnit.commandLen = 13;
            tmUnit.ackLen = 7;
            tmUnit.commandId[0] = 0xAA;
            tmUnit.commandId[1] = 0x41;
            tmUnit.commandId[2] = 0x11;
            tmUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            tmUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            tmUnit.commandId[5] = 0x00;
            tmUnit.commandId[6] = 0x06;
            tmUnit.commandId[7] = 0x00;
            tmUnit.commandId[8] = 0x00;
            tmUnit.commandId[9] = 0x00;
            tmUnit.commandId[10] = 0x00;
            tmUnit.commandId[11] = 0x00;
            tmUnit.commandId[12] = 0x55;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case LINK_HX_0401_DA:{
            CommandUnit tmUnit;
            tmUnit.commandLen = 14;
            tmUnit.ackLen = 7;
            tmUnit.commandId[0] = 0xAA;
            tmUnit.commandId[1] = 0x45;
            tmUnit.commandId[2] = 0x11;
            tmUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            tmUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            tmUnit.commandId[5] = 0x00;
            tmUnit.commandId[6] = 0x07;
            tmUnit.commandId[7] = 0x00;
            tmUnit.commandId[8] = 0x00;
            tmUnit.commandId[9] = 0x00;
            tmUnit.commandId[10] = 0x00;
            tmUnit.commandId[11] = 0x00;
            tmUnit.commandId[12] = 0x00;
            tmUnit.commandId[13] = 0x55;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case LINK_HX_0401_DABS:
        {
            CommandUnit tmUnit;
            tmUnit.commandLen = 14;
            tmUnit.ackLen = 7;
            tmUnit.commandId[0] = 0xAA;
            tmUnit.commandId[1] = 0x45;
            tmUnit.commandId[2] = 0x11;
            tmUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            tmUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            tmUnit.commandId[5] = 0x00;
            tmUnit.commandId[6] = 0x07;
            tmUnit.commandId[7] = 0x00;
            tmUnit.commandId[8] = 0x00;
            tmUnit.commandId[9] = 0x00;
            tmUnit.commandId[10] = 0x00;
            tmUnit.commandId[11] = 0x00;
            tmUnit.commandId[12] = 0x00;
            tmUnit.commandId[13] = 0x55;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[2] = 0x77;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case LINK_HX_0214_DA:
        {
            CommandUnit tmUnit;
            tmUnit.commandLen = 0;
            tmUnit.ackLen = 7;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case LINK_HX_0401_SP:
        {
           CommandUnit tmUnit;
           tmUnit.commandLen = 6;
           tmUnit.ackLen = 11;
           tmUnit.commandId[0] = 0xAA;
           tmUnit.commandId[1] = 0x44;
           tmUnit.commandId[2] = 0x00;
           tmUnit.commandId[3] = 0x00;
           tmUnit.commandId[4] = 0x44;
           tmUnit.commandId[5] = 0x55;
           cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case LINK_JC_5103:
        {
            CommandUnit tmUnit;
            tmUnit.commandLen = 9;
            tmUnit.ackLen = -1;
            tmUnit.commandId[0] = 0x01;
            tmUnit.commandId[1] = d_devInfo.iAddressCode;
            unsigned char ccmdindex[3]={'0'};
            IntToAscii(d_devInfo.iAddressCode,&ccmdindex[0]);
            tmUnit.commandId[2] = ccmdindex[1];
            tmUnit.commandId[3] = ccmdindex[0];
            tmUnit.commandId[4] = 0x52;
            tmUnit.commandId[5] = 0x53;
            int checknum = (tmUnit.commandId[1]^tmUnit.commandId[2]^tmUnit.commandId[3]^tmUnit.commandId[4]^tmUnit.commandId[5])&0xFF;
            unsigned char ccheck[3]={'0'};
            IntToAscii(checknum,&ccheck[0]);
            tmUnit.commandId[6] = ccheck[1];
            tmUnit.commandId[7] = ccheck[0];
            tmUnit.commandId[8] = 0x04;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case LINK_HX_6300:
        case LINK_HX_6300_AD:
        {
            CommandUnit tmUnit;
            tmUnit.commandLen = 14;
            tmUnit.ackLen = 7;
            tmUnit.commandId[0] = 0xAA;
            tmUnit.commandId[1] = 0x63;
            tmUnit.commandId[2] = 0x11;
            tmUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            tmUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            tmUnit.commandId[5] = 0x00;
            tmUnit.commandId[6] = 0x07;
            tmUnit.commandId[7] = 0x00;
            tmUnit.commandId[8] = 0x00;
            tmUnit.commandId[9] = 0x00;
            tmUnit.commandId[10] = 0x00;
            tmUnit.commandId[11] = 0x00;
            tmUnit.commandId[12] = 0x00;
            tmUnit.commandId[13] = 0x55;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case LINK_HX_9020:
        {
            CommandUnit tmUnit;
            tmUnit.commandLen = 14;
            tmUnit.ackLen = 7;
            tmUnit.commandId[0] = 0xAA;
            tmUnit.commandId[1] = 0x92;
            tmUnit.commandId[2] = 0x11;
            tmUnit.commandId[3] = ((d_devInfo.iAddressCode&0xFF00)>>8);
            tmUnit.commandId[4] = (d_devInfo.iAddressCode&0x00FF);
            tmUnit.commandId[5] = 0x00;
            tmUnit.commandId[6] = 0x07;
            tmUnit.commandId[7] = 0x00;
            tmUnit.commandId[8] = 0x00;
            tmUnit.commandId[9] = 0x00;
            tmUnit.commandId[10] = 0x00;
            tmUnit.commandId[11] = 0x00;
            tmUnit.commandId[12] = 0x00;
            tmUnit.commandId[13] = 0x55;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        }
    }


    void aysnc_callback(int reason, Snmp *session,
                Pdu &pdu, SnmpTarget &target, void *cd)
    {
        //判断是否接收到查询应答或者trap通知
        if(reason != SNMP_CLASS_ASYNC_RESPONSE && reason != SNMP_CLASS_NOTIFICATION){
            cout << "Failed to issue SNMP aysnc_callback: (" << reason  << ") "
                   << session->error_msg(reason) << endl;
            return;
        }

        if (cd){

            //cout << "Succeed to issue SNMP aysnc_callback!!";
            ((Link_message*)cd)->Link_Callback(reason, session, pdu, target);
        }
    }


    void Link_message::Link_Callback(int reason, Snmp *session,Pdu &pdu, SnmpTarget &target)
    {
        //int pdu_error = pdu.get_error_status();
        //if (pdu_error)
        //    return;
        //if (pdu.get_vb_count() == 0)
        //    return;

        switch (d_devInfo.nSubProtocol)
        {
        case LINK_STATELITE_RECEVIE:
            return parse_Satellite_data_(pdu,target);
        case LINK_TEST_RECEVIE:
            return parse_Testllite_data_(pdu,target);
        case LINK_SING_NET_ADAPTER:
            return parse_SingApt_data(pdu,target);
        case LINK_DMP_SWITCH:
            return parse_DmpSwitch_data(pdu,target);
        case LINK_ASI_ADAPTER:
            return parse_AsiApt_data(pdu,target);
        case LINK_WEILE_AVSP_DECODER:
            return parse_weile_avsp_decorder_data(pdu,target);
        case LINK_WEILE_AVSP_ADAPTER:
        case LINK_SMSX_ASI_ADAPTER:
        case LINK_SMSX_ASI_ENCODER:
        case LINK_NORMAL_SNMP_DEV:
            return parse_weile_avsp_apt_data(pdu,target);
        }
    }

    void Link_message::exec_task_now(int icmdType, string sUser, e_ErrorCode &eErrCode,map<int,string> &mapParam,
                                     bool bSnmp, Snmp *snmp, CTarget *target)
    {
        eErrCode = EC_UNKNOWN;
        CommandUnit cmdUnit;
        cmdUnit.commandLen = 0;



        switch (icmdType) {
        case MSG_ADJUST_TIME_SET_OPR:
            break;
        case MSG_CONTROL_MOD_SWITCH_OPR:{
            int nChannel = atoi(mapParam[0].c_str());
            GetControlModCmdByModId(nChannel,cmdUnit);
        }
            break;
        case MSG_0401_SWITCH_OPR:{//通道切换
            int nChannel = atoi(mapParam[0].c_str());
            GetSwitchCmdByChannel(nChannel,cmdUnit);
        }
            break;
        default:
            break;
        }

        if(cmdUnit.commandLen>0)
        {
            eErrCode = EC_OK;
            m_pSession->send_cmd_to_dev(cmdUnit,eErrCode);
        }
    }

    void Link_message::exec_general_task(int icmdType, string sUser, devCommdMsgPtr lpParam, e_ErrorCode &eErrCode)
    {
        eErrCode = EC_UNKNOWN;
        CommandUnit cmdUnit;
        cmdUnit.commandLen = 0;
        switch (icmdType) {
        case MSG_CONTROL_MOD_SWITCH_OPR:
        {
            GetControlModCmd(lpParam,cmdUnit);
        }
            break;
        case MSG_0401_SWITCH_OPR://通道切换
        {
            GetSwitchCmd(lpParam,cmdUnit);
        }
            break;
        case MSG_GENERAL_COMMAND_OPR://
            break;
        case MSG_SWITCH_AUDIO_CHANNEL_OPR://切换音频通道
            break;
        default:
            break;
        }
        if(cmdUnit.commandLen>0)
        {
            eErrCode = EC_OK;
            m_pSession->send_cmd_to_dev(cmdUnit,eErrCode);
        }
    }

    void Link_message::parse_Satellite_data_(Pdu &pdu, SnmpTarget &target)
    {
        if(d_curData_ptr == NULL)
            return;
        for (int i=0; i<pdu.get_vb_count(); i++)
        {
            Vb nextVb;
            pdu.get_vb(nextVb, i);
            string cur_oid = nextVb.get_printable_oid();//Oid
            string cur_value =nextVb.get_printable_value();
            DataInfo dainfo;
            if(cur_oid == rflevel){
                if(cur_value.empty())
                    cur_value ="-94";
                dainfo.sValue = cur_value;
                dainfo.bType=false;
                dainfo.fValue =  atof(cur_value.c_str());
                d_curData_ptr->mValues[0] = dainfo;
            }else if(cur_oid == signalLock){
                dainfo.bType=true;
                int nRlt = cur_value.find("CB F8 B6 A8");
                if(nRlt!=-1){//信号锁
                    dainfo.sValue = "1";
                    dainfo.fValue=1.0f;
                }else{
                    dainfo.sValue = "0";
                    dainfo.fValue=0.0f;
                }
                d_curData_ptr->mValues[1] = dainfo;
            }else if(cur_oid == singalber){
                if(cur_value.empty())
                    cur_value ="45";
                dainfo.bType=false;
                dainfo.sValue = cur_value;
                string_replace(cur_value,"dB","");
                string_replace(cur_value," ","");
                if(cur_value=="N/A")
                    dainfo.fValue=0.0f;
                else
                    dainfo.fValue =  atof(cur_value.c_str());
                d_curData_ptr->mValues[2] = dainfo;
            }else if(cur_oid == signalcn){
                if(cur_value.empty())
                    cur_value ="30";
                dainfo.bType=false;
                dainfo.sValue = cur_value;
                string_replace(cur_value,"dB","");
                string_replace(cur_value," ","");
                dainfo.fValue = atof(cur_value.c_str());
                d_curData_ptr->mValues[3] = dainfo;
            }else if(cur_oid == totalrate){
                if(cur_value.empty())
                    cur_value ="10";
                dainfo.bType=false;
                dainfo.sValue = cur_value;
                dainfo.fValue = atof(cur_value.c_str());
                d_curData_ptr->mValues[4] = dainfo;
            }else if(cur_oid == frequency){
                if(cur_value.empty())
                    cur_value ="658";
                dainfo.bType=false;
                dainfo.sValue = cur_value;
                string_replace(cur_value,"MHz","");
                string_replace(cur_value," ","");
                dainfo.fValue = atof(cur_value.c_str());
                d_curData_ptr->mValues[5] = dainfo;
            }
        }


        m_pSession->start_handler_data(d_devInfo.sDevNum,d_curData_ptr);

    }

    int Link_message::parse_TestReceive_data(Snmp *snmp, DevMonitorDataPtr data_ptr, CTarget *target)
    {

        Pdu pdu;
        Vb vbl[3];
        vbl[0].set_oid(Oid("1.3.6.1.2.1.25.2.2.0"));
        vbl[1].set_oid(Oid("1.3.6.1.2.1.25.1.6.0"));
        vbl[2].set_oid(Oid("1.3.6.1.2.1.25.1.5.0"));
        for (int i=0; i<NUM_SYS_VBS-3;i++)
            pdu += vbl[i];
        int status = snmp->get(pdu,*target, aysnc_callback,this);
        if (status != SNMP_ERROR_SUCCESS){
            return -1;
        }else{
            return 0;
        }
        return -1;
    }

    void Link_message::parse_Testllite_data_(Pdu &pdu, SnmpTarget &target)
    {

        if(d_curData_ptr == NULL)
            return;

        int pdu_error = pdu.get_error_status();
        if (pdu_error)
            return;
        if (pdu.get_vb_count() == 0)
            return;
        int vbcount = pdu.get_vb_count();
        for(int i=0;i<vbcount;++i)
        {
            DataInfo dainfo;
            Vb nextVb;
            pdu.get_vb(nextVb, i);
            string cur_oid = nextVb.get_printable_oid();
            string cur_value =nextVb.get_printable_value();
            dainfo.bType = false;
            dainfo.fValue = atof(cur_value.c_str());
            if(cur_oid=="1.3.6.1.2.1.25.2.2.0")
            {
                dainfo.fValue = dainfo.fValue/1048576.00;
                dainfo.sValue = str(boost::format("%.2f G")%dainfo.fValue);
                d_curData_ptr->mValues[0] = dainfo;
            }
            else if(cur_oid=="1.3.6.1.2.1.25.1.6.0")
            {
                dainfo.sValue = cur_value;
                d_curData_ptr->mValues[1] = dainfo;
            }
            else if(cur_oid=="1.3.6.1.2.1.25.1.5.0")
            {
                dainfo.sValue = cur_value;
                d_curData_ptr->mValues[2] = dainfo;
            }
        }

        m_pSession->start_handler_data(d_devInfo.sDevNum,d_curData_ptr);
    }

    int Link_message::parse_SingAptReceive_data(Snmp *snmp, DevMonitorDataPtr data_ptr, CTarget *target)
    {
        if(snmp == NULL)
            return -1;

        int status = snmp->get(query_pdu,*target, aysnc_callback,this);
        if (status){
            return -1;
        }else{
            return 0;
        }
        return -1;
    }

    void Link_message::parse_SingApt_data(Pdu &pdu, SnmpTarget &target)
    {

        if(d_curData_ptr == NULL)
            return;

        int pdu_error = pdu.get_error_status();
        if (pdu_error)
            return;
        if (pdu.get_vb_count() == 0)
            return;
        int vbcount = pdu.get_vb_count();
        for(int i=0;i<vbcount;++i)
        {
            DataInfo dainfo;
            Vb nextVb;
            pdu.get_vb(nextVb, i);
            string cur_oid = nextVb.get_printable_oid();
            string cur_value =nextVb.get_printable_value();
            dainfo.bType = false;
            dainfo.fValue = atof(cur_value.c_str());
            dainfo.sValue = str(boost::format("%.2f Kbps")%dainfo.fValue);
            map<string,int>::iterator iter = map_Oid.find(cur_oid);
            if(iter!=map_Oid.end())
            {
                d_curData_ptr->mValues[(*iter).second] = dainfo;
            }
        }


        m_pSession->start_handler_data(d_devInfo.sDevNum,d_curData_ptr);

    }

    void Link_message::parse_DmpSwitch_data(Pdu &pdu, SnmpTarget &target)
    {

        if(d_curData_ptr == NULL)
            return;
        int pdu_error = pdu.get_error_status();
        if (pdu_error)
            return;
        if (pdu.get_vb_count() == 0)
            return;
        int vbcount = pdu.get_vb_count();
        for(int i=0;i<vbcount;++i)
        {
            DataInfo dainfo;
            Vb nextVb;
            pdu.get_vb(nextVb, i);
            string cur_oid = nextVb.get_printable_oid();
            string cur_value =nextVb.get_printable_value();
            dainfo.bType = false;
            dainfo.fValue = atof(cur_value.c_str());
            dainfo.sValue = str(boost::format("%.2f Mbps")%dainfo.fValue);
            map<string,int>::iterator iter = map_Oid.find(cur_oid);
            if(iter!=map_Oid.end())
            {
                d_curData_ptr->mValues[(*iter).second] = dainfo;
            }
        }

        m_pSession->start_handler_data(d_devInfo.sDevNum,d_curData_ptr);

    }

    void Link_message::parse_AsiApt_data(Pdu &pdu, SnmpTarget &target)
    {
        parse_DmpSwitch_data(pdu,target);
    }

    void Link_message::parse_weile_avsp_decorder_data(Pdu &pdu, SnmpTarget &target)
    {
        if(d_curData_ptr == NULL)
            return;
        int pdu_error = pdu.get_error_status();
        if (pdu_error){
             cout << "parse_weile_avsp_decorder_data-----status---"<<pdu_error<<endl;
            return;
        }
        if (pdu.get_vb_count() == 0){
            cout << "parse_weile_avsp_decorder_data-----get_vb_count=0---"<<endl;
            return;
        }
        int vbcount = pdu.get_vb_count();
        for(int i=0;i<vbcount;++i)
        {
            DataInfo dainfo;
            Vb nextVb;
            pdu.get_vb(nextVb, i);
            string cur_oid = nextVb.get_printable_oid();
            string cur_value =nextVb.get_printable_value();
            dainfo.sValue = cur_value;
            map<string,int>::iterator iter = map_Oid.find(cur_oid);
            dainfo.bType = false;
            if(iter!=map_Oid.end())
            {
                if((*iter).second==0)
                {
                    dainfo.bType = true;
                    dainfo.fValue = atof(cur_value.c_str());
                }else if((*iter).second==1)
                {
                    dainfo.fValue = atof(cur_value.substr(0,cur_value.find('*')).c_str());
                }
                else
                {
                    dainfo.fValue = atof(cur_value.substr(0,cur_value.find(' ')).c_str());
                }
                d_curData_ptr->mValues[(*iter).second] = dainfo;
            }
        }

        m_pSession->start_handler_data(d_devInfo.sDevNum,d_curData_ptr);
    }

    void Link_message::GetResultData(DevMonitorDataPtr data_ptr)
     {
         map<int,DeviceMonitorItem>::iterator iter = d_devInfo.map_MonitorItem.begin();
         for(;iter!=d_devInfo.map_MonitorItem.end();++iter)
         {
             map<int,DataInfo>::iterator diter = data_ptr->mValues.find((*iter).first);
             if(diter!=data_ptr->mValues.end())
             {

                 if((*iter).second.iItemType == 0){
                     data_ptr->mValues[(*iter).first].fValue *= (*iter).second.dRatio;
                     if(data_ptr->mValues[(*iter).first].sValue.empty())
                         data_ptr->mValues[(*iter).first].sValue = QString::number(data_ptr->mValues[(*iter).first].fValue,'g',2).toStdString();
                 }
                 else {
                     if((*iter).second.dRatio==0)
                         data_ptr->mValues[(*iter).first].fValue = data_ptr->mValues[(*iter).first].fValue==1.0f ? 0:1;
                 }
             }
         }
     }

    int  Link_message::parse_SatelliteReceive_data(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target)
    {

        Pdu pdu;
        Vb vbl[NUM_SYS_VBS];
        vbl[0].set_oid(rflevel);
        vbl[1].set_oid(signalLock);
        vbl[2].set_oid(singalber);
        vbl[3].set_oid(signalcn);
        vbl[4].set_oid(totalrate);
        vbl[5].set_oid(frequency);
        for (int i=0; i<NUM_SYS_VBS;i++)
            pdu += vbl[i];
        //d_data_ptr = data_ptr;//保存
        int status = snmp->get(pdu,*target, aysnc_callback,this);
        if (status){
            return -1;
        }else{
           return 0;
       }
        return -1;

    }

    void Link_message::parse_weile_avsp_apt_data(Pdu &pdu, SnmpTarget &target)
    {
        if(d_curData_ptr == NULL)
            return;
        int pdu_error = pdu.get_error_status();
        if (pdu_error){
            cout << "parse_weile_avsp_apt_data-----pdu.get_error_status() == ---"<<pdu_error<<endl;
            return;
        }
        if (pdu.get_vb_count() == 0){
            cout << "parse_weile_avsp_apt_data-----get_vb_count() == 0---"<<endl;
            return;
        }
        int vbcount = pdu.get_vb_count();
        for(int i=0;i<vbcount;++i)
        {
            DataInfo dainfo;
            Vb nextVb;
            pdu.get_vb(nextVb, i);
            string cur_oid = nextVb.get_printable_oid();
            string cur_value =nextVb.get_printable_value();
            dainfo.bType = false;
            dainfo.fValue = atof(cur_value.c_str());
            dainfo.sValue = str(boost::format("%.2f Mbps")%dainfo.fValue);
            map<string,int>::iterator iter = map_Oid.find(cur_oid);
            if(iter!=map_Oid.end())
            {
                d_curData_ptr->mValues[(*iter).second] = dainfo;
            }
        }

        m_pSession->start_handler_data(d_devInfo.sDevNum,d_curData_ptr);
    }


   void Link_message::init_smsx_asi_Oid()
   {
       Vb vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4022.200.1.6.7.1.1"] = 0;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4022.200.1.6.7.1.1"));
       query_pdu += vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4022.200.1.5.7.1.1"] = 1;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4022.200.1.5.7.1.1"));
       query_pdu += vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4022.200.1.6.7.2.1"] = 2;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4022.200.1.6.7.2.1"));
       query_pdu += vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4022.200.1.5.7.2.1"] = 3;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4022.200.1.5.7.2.1"));
       query_pdu += vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4023.300.1.8.1.2"] = 4;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4023.300.1.8.1.2"));
       query_pdu += vbl;

       map_Oid["1.3.6.1.4.1.32285.2.2.1.4023.300.1.9.1.2"] = 5;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4023.300.1.9.1.2"));
       query_pdu += vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4023.300.1.8.1.3"] = 6;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4023.300.1.8.1.3"));
       query_pdu += vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4023.300.1.9.1.3"] = 7;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4023.300.1.9.1.3"));
       query_pdu += vbl;
   }

   void Link_message::init_smsx_asi_decoder_Oid()
   {
       Vb vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4023.200.1.14.1.2"] = 0;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4023.200.1.14.1.2"));
       query_pdu += vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4023.200.1.14.1.3"] = 1;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4023.200.1.14.1.3"));
       query_pdu += vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4023.200.1.14.1.4"] = 2;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4023.200.1.14.1.4"));
       query_pdu += vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4023.200.1.14.1.5"] = 3;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4023.200.1.14.1.5"));
       query_pdu += vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4023.200.1.15.1.2"] = 4;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4023.200.1.15.1.2"));
       query_pdu += vbl;

       map_Oid["1.3.6.1.4.1.32285.2.2.1.4023.200.1.15.1.3"] = 5;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4023.200.1.15.1.3"));
       query_pdu += vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4023.200.1.15.1.4"] = 6;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4023.200.1.15.1.4"));
       query_pdu += vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4023.200.1.15.1.5"] = 7;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4023.200.1.15.1.5"));
       query_pdu += vbl;

       map_Oid["1.3.6.1.4.1.32285.2.2.1.4023.300.1.8.1.1"] = 8;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4023.300.1.8.1.1"));
       query_pdu += vbl;
       map_Oid["1.3.6.1.4.1.32285.2.2.1.4023.300.1.9.1.1"] = 9;
       vbl.set_oid(Oid("1.3.6.1.4.1.32285.2.2.1.4023.300.1.9.1.1"));
       query_pdu += vbl;
   }

   void Link_message::init_normal_snmp_oid()
   {
        map<int,DeviceMonitorItem>::iterator iter = d_devInfo.map_MonitorItem.begin();
        for(;iter!=d_devInfo.map_MonitorItem.end();++iter)
        {
            Vb vbl;
            string sOid = iter->second.cmdSnmpOid;
            if(!sOid.empty()){
                map_Oid[sOid] = iter->first;
                vbl.set_oid(Oid(sOid.c_str()));
                query_pdu += vbl;
            }

        }
   }

   int Link_message::decode_0214DA(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
    {
       if(data[2]!=0x11)
            return RE_CMDACK;

        iaddcode = data[4]*256+data[3];
        DataInfo dainfo;
        dainfo.bType = true;
        if(data[7]==0x01)
        {
            dainfo.fValue = 1;
            data_ptr->mValues[0] = dainfo;
            dainfo.fValue = 0;
            data_ptr->mValues[1] = dainfo;
        }
        else if(data[7]==0x02)
        {
            dainfo.fValue = 0;
            data_ptr->mValues[0] = dainfo;
            dainfo.fValue = 1;
            data_ptr->mValues[1] = dainfo;
        }
        else {
            dainfo.fValue = 1;
            data_ptr->mValues[0] = dainfo;
            data_ptr->mValues[1] = dainfo;
        }
        for(int i=0;i<16;++i)
        {
            dainfo.fValue = data[8+i];
            data_ptr->mValues[2+i] = dainfo;
        }

        return 0;
    }

   int Link_message::decode_0401SP(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
   {
       if(data[1]!=0x44)
           return RE_CMDACK;
       iaddcode = d_devInfo.iAddressCode;
       DataInfo dainfo;
       dainfo.bType = true;
       int ichanel;
       ichanel = data[2];
       switch(ichanel)
       {
       case 0:
       {
           dainfo.fValue = 1.0;
           data_ptr->mValues[0] = dainfo;
           dainfo.fValue = 0.0;
           data_ptr->mValues[1] = dainfo;
           data_ptr->mValues[2] = dainfo;
       }
           break;
       case 1:
       {
           dainfo.fValue = 1.0;
           data_ptr->mValues[1] = dainfo;
           dainfo.fValue = 0.0;
           data_ptr->mValues[0] = dainfo;
           data_ptr->mValues[2] = dainfo;
       }
           break;
       case 2:
       {
           dainfo.fValue = 1.0;
           data_ptr->mValues[2] = dainfo;
           dainfo.fValue = 0.0;
           data_ptr->mValues[1] = dainfo;
           data_ptr->mValues[0] = dainfo;
       }
           break;
       default:
       {
           dainfo.fValue = 0.0;
           data_ptr->mValues[0] = dainfo;
           data_ptr->mValues[1] = dainfo;
           data_ptr->mValues[2] = dainfo;
       }
           break;
       }
       dainfo.fValue = data[3];
       data_ptr->mValues[3] = dainfo;
       for(int i=0;i<3;++i)
       {
           dainfo.fValue = Getbit(data[4],i);
           data_ptr->mValues[4+i] = dainfo;
       }
       dainfo.bType = false;
       dainfo.fValue = data[5];
       data_ptr->mValues[7] = dainfo;
      /* for(int j=0;j<6;++j)
       {
           dainfo.fValue = data[6+j];
           data_ptr->mValues[8+j] = dainfo;
       }
       data_ptr->mValues[14] = data_ptr->mValues[8+2*ichanel];
       data_ptr->mValues[15] = data_ptr->mValues[9+2*ichanel];*/
       return RE_SUCCESS;
   }


   int Link_message::decode_6300(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
  {
      if(data[2]!=0x11)
          return RE_CMDACK;
      iaddcode = data[3]*256+data[4];
      DataInfo dainfo;
      dainfo.bType = false;
      dainfo.fValue = (data[8]-120)*0.10;
      data_ptr->mValues[0] = dainfo;
      dainfo.fValue = (data[9]-120)*0.10;
      data_ptr->mValues[1] = dainfo;
      dainfo.fValue = (data[10]-100)*0.1;
      data_ptr->mValues[2] = dainfo;
      return RE_SUCCESS;
  }


  /* int Link_message::decode_JC5103(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
  {
       int nresult = RE_CMDACK;
       int lastlen = nDataLen;
       while(lastlen>10){
           if(data[4]==0x52 && data[5]==0x53)
           {
               iaddcode = data[1];
               int nDataType;
               char cNum[10]={0};
               data = data+6;
               int iLastNum=(nDataLen-6);
               DataInfo dainfo;
               while(1)
               {
                   nDataType = data[0];
                   if(nDataType==0xFE  || iLastNum<3)
                       break;
                   int iReadNum = 1;
                   for(int jpos=0;iLastNum>iReadNum && jpos<10;++jpos)
                   {
                       if(data[iReadNum]!=0x20)
                       {
                           cNum[jpos] = data[iReadNum];
                           ++iReadNum;
                       }
                       else
                       {
                           ++iReadNum;
                           break;
                       }
                   }
                   data = data+iReadNum;
                   iLastNum -= iReadNum;
                   switch(nDataType)
                   {
                   case 0x21:
                   {
                       dainfo.bType = true;
                       dainfo.fValue = atoi(cNum);// == 1? 0:1
                       data_ptr->mValues[0] = dainfo;
                   }
                       break;
                   case 0x22:
                   {
                       dainfo.bType = true;
                       dainfo.fValue = atoi(cNum);// == 1? 0:1
                       data_ptr->mValues[1] = dainfo;
                   }
                       break;
                   case 0x23:
                   {
                       dainfo.bType = true;
                       dainfo.fValue = atoi(cNum);// == 1? 0:1
                       data_ptr->mValues[2] = dainfo;
                   }
                       break;
                   case 0x24:
                   {
                       dainfo.bType = true;
                       dainfo.fValue = atoi(cNum);// == 1? 0:1
                       data_ptr->mValues[3] = dainfo;
                   }
                       break;
                   case 0x25:
                   {
                       dainfo.bType = false;
                       dainfo.fValue = atof(cNum);
                       data_ptr->mValues[4] = dainfo;
                   }
                       break;
                   case 0x26:
                   {
                       dainfo.bType = false;
                       dainfo.fValue = atof(cNum);
                       data_ptr->mValues[5] = dainfo;
                   }
                       break;
                   case 0x27:
                   {
                       dainfo.bType = false;
                       dainfo.fValue = atof(cNum);
                       data_ptr->mValues[6] = dainfo;
                   }
                       break;
                   case 0x28:
                   {
                       dainfo.bType = false;
                       dainfo.fValue = atof(cNum);
                       data_ptr->mValues[7] = dainfo;
                   }
                       break;
                   case 0x29:
                   {
                       dainfo.bType = false;
                       dainfo.fValue = atof(cNum);
                       data_ptr->mValues[8] = dainfo;
                   }
                       break;
                   case 0x2A:
                   {
                       dainfo.bType = false;
                       dainfo.fValue = atof(cNum);
                       data_ptr->mValues[9] = dainfo;
                   }
                       break;
                   case 0x2B:
                   {
                       dainfo.bType = false;
                       dainfo.fValue = atof(cNum);
                       data_ptr->mValues[10] = dainfo;
                   }
                       break;
                   case 0x2C:
                   {
                       dainfo.bType = true;
                       dainfo.fValue = atoi(cNum);// == 1? 0:1
                       data_ptr->mValues[11] = dainfo;
                   }
                       break;
                   case 0x2D:
                   {
                       dainfo.bType = false;
                       dainfo.fValue = atof(cNum);
                       data_ptr->mValues[12] = dainfo;
                   }
                       break;
                   case 0x2E:
                   {
                       dainfo.bType = true;
                       dainfo.fValue = atoi(cNum);
                       data_ptr->mValues[11] = dainfo;
                   }
                       break;
                   }

                   memset(&cNum,0,10);
               }
               nresult = RE_SUCCESS;
               break;
           }else if(data[4]==0x53 && data[5]==0x43)
           {
               DAS_5103_ACK(data);
               for(int i=9;i<lastlen;++i)
               {
                   if(data[i]==0x04)
                   {
                       data = data+i+1;
                       lastlen = lastlen-i-1;
                       break;
                   }
               }
           }
           else
           {
               for(int i=9;i<lastlen;++i)
               {
                   if(data[i]==0x04)
                   {
                       data = data+i+1;
                       lastlen = lastlen-i-1;
                       break;
                   }
               }
           }
       }
       return nresult;
  }*/

   int Link_message::decode_JC5103(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
  {
      int nresult = RE_CMDACK;
      int lastlen = nDataLen;
      unsigned char cEnd[1]={0x04};
      while(lastlen>10){
          if(data[1]!=d_devInfo.iAddressCode)
          {
              int npos = kmp(data,lastlen,cEnd,1);
              if(npos>=0)
              {
                  data = data+npos+1;
                  lastlen = lastlen-npos-1;
              }
              else
                  break;
          }
          else if(data[4]==0x52 && data[5]==0x53)
          {
              iaddcode = data[1];
              int nDataType;
              char cNum[10]={0};
              data = data+6;
              int iLastNum=(nDataLen-6);
              DataInfo dainfo;
              cEnd[0] = 0xFE;
              int npos = kmp(data,lastlen,cEnd,1);
              if(npos<0)
                  return RE_HEADERROR;
              while(1)
              {
                  nDataType = data[0];
                  if(nDataType==0xFE || iLastNum<11)
                      break;
                  int iReadNum = 1;
                  for(int jpos=0;iLastNum>iReadNum && jpos<10;++jpos)
                  {
                      if(data[iReadNum]!=0x20)
                      {
                          cNum[jpos] = data[iReadNum];
                          ++iReadNum;
                      }
                      else
                      {
                          ++iReadNum;
                          break;
                      }
                  }
                  data = data+iReadNum;
                  iLastNum -= iReadNum;
                  switch(nDataType)
                  {
                  case 0x21:
                  {
                      dainfo.bType = true;
                      dainfo.fValue = atoi(cNum);// == 1? 0:1;
                      data_ptr->mValues[0] = dainfo;
                  }
                      break;
                  case 0x22:
                  {
                      dainfo.bType = true;
                      dainfo.fValue = atoi(cNum);// == 1? 0:1;
                      data_ptr->mValues[1] = dainfo;
                  }
                      break;
                  case 0x23:
                  {
                      dainfo.bType = true;
                      dainfo.fValue = atoi(cNum);// == 1? 0:1;
                      data_ptr->mValues[2] = dainfo;
                  }
                      break;
                  case 0x24:
                  {
                      dainfo.bType = true;
                      dainfo.fValue = atoi(cNum);// == 1? 0:1;
                      data_ptr->mValues[3] = dainfo;
                  }
                      break;
                  case 0x25:
                  {
                      dainfo.bType = false;
                      dainfo.fValue = atof(cNum);
                      data_ptr->mValues[4] = dainfo;
                  }
                      break;
                  case 0x26:
                  {
                      dainfo.bType = false;
                      dainfo.fValue = atof(cNum);
                      data_ptr->mValues[5] = dainfo;
                  }
                      break;
                  case 0x27:
                  {
                      dainfo.bType = false;
                      dainfo.fValue = atof(cNum);
                      data_ptr->mValues[6] = dainfo;
                  }
                      break;
                  case 0x28:
                  {
                      dainfo.bType = false;
                      dainfo.fValue = atof(cNum);
                      data_ptr->mValues[7] = dainfo;
                  }
                      break;
                  case 0x29:
                  {
                      dainfo.bType = false;
                      dainfo.fValue = atof(cNum);
                      data_ptr->mValues[8] = dainfo;
                  }
                      break;
                  case 0x2A:
                  {
                      dainfo.bType = false;
                      dainfo.fValue = atof(cNum);
                      data_ptr->mValues[9] = dainfo;
                  }
                      break;
                  case 0x2B:
                  {
                      dainfo.bType = false;
                      dainfo.fValue = atof(cNum);
                      data_ptr->mValues[10] = dainfo;
                  }
                      break;
                  case 0x2C:
                  {
                      dainfo.bType = true;
                      dainfo.fValue = atoi(cNum);// == 1? 0:1;
                      data_ptr->mValues[11] = dainfo;
                  }
                      break;
                  case 0x2D:
                  {
                      dainfo.bType = false;
                      dainfo.fValue = atof(cNum);
                      data_ptr->mValues[12] = dainfo;
                  }
                      break;
                  case 0x2E:
                  {
                      dainfo.bType = true;
                      dainfo.fValue = atoi(cNum);
                      data_ptr->mValues[11] = dainfo;
                  }
                      break;
                  }

                  memset(&cNum,0,10);
              }
              nresult = RE_SUCCESS;
              break;
          }else if(data[4]==0x53 && data[5]==0x43)
          {
              DAS_5103_ACK(data);
              int npos = kmp(data,lastlen,cEnd,1);
              if(npos>=0)
              {
                  data = data+npos+1;
                  lastlen = lastlen-npos-1;
              }
              else
                  break;
          }
          else
          {
              int npos = kmp(data,lastlen,cEnd,1);
              if(npos>=0)
              {
                  data = data+npos+1;
                  lastlen = lastlen-npos-1;
              }
              else
                  break;
          }
      }
      return nresult;
  }

   int Link_message::decode_6300_AD(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
  {
      if(data[2]!=0x11)
          return RE_CMDACK;
      iaddcode = data[3]*256+data[4];
      DataInfo dainfo;
      dainfo.bType = false;
      dainfo.fValue = (data[8]-120)*0.10;
      data_ptr->mValues[0] = dainfo;
      dainfo.fValue = (data[9]-120)*0.10;
      data_ptr->mValues[1] = dainfo;
      dainfo.fValue = (data[10]-100)*0.1;
      data_ptr->mValues[2] = dainfo;
      dainfo.bType = true;
      dainfo.fValue = data[11];
      data_ptr->mValues[3] = dainfo;
      dainfo.bType = false;
      dainfo.fValue = data[12];
      data_ptr->mValues[4] = dainfo;
      dainfo.fValue = data[13];
      data_ptr->mValues[5] = dainfo;
      return RE_SUCCESS;
  }

   int Link_message::decode_9020(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
  {
      iaddcode = data[3]*256+data[4];
      DataInfo dainfo;
      for(int i=0;i<6;++i)
      {
          dainfo.bType = false;
          dainfo.fValue = data[3*i+7];
          data_ptr->mValues[3*i] = dainfo;
          dainfo.fValue = data[3*i+8];
          data_ptr->mValues[3*i+1] = dainfo;
          dainfo.bType = true;
          dainfo.fValue = data[3*i+9];
          data_ptr->mValues[3*i+2] = dainfo;
      }
      return RE_SUCCESS;
  }
}
