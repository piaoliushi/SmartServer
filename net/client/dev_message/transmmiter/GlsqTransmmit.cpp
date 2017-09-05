#include "GlsqTransmmit.h"
#include<QtCore/qmath.h>
namespace hx_net{
    GlsqTransmmit::GlsqTransmmit(int subprotocol,int addresscode)
        :Transmmiter()
        ,m_subprotocol(subprotocol)
        ,m_addresscode(addresscode)
    {

    }

    GlsqTransmmit::~GlsqTransmmit()
    {

    }

    int GlsqTransmmit::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
    {
        switch (m_subprotocol) {
        case GLSQ_T:{
            if(data[0]==0x24)
                return data[1]-4;
            else
                return RE_HEADERROR;
        }
            break;
        default:
            break;
        }

        return RE_NOPROTOCOL;
    }

    int GlsqTransmmit::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        switch (m_subprotocol) {
        case GLSQ_T:{
            return Glsq_T_Data(data,data_ptr,nDataLen,runstate);
        }
            break;
        default:
            break;
        }
        return RE_NOPROTOCOL;
    }

    bool GlsqTransmmit::IsStandardCommand()
    {
        switch (m_subprotocol) {
        case GLSQ_T:
            return true;
        default:
            break;
        }
        return false;
    }

    void GlsqTransmmit::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
    {

    }

    void GlsqTransmmit::GetAllCmd(CommandAttribute &cmdAll)
    {
        switch (m_subprotocol) {
        case GLSQ_T:{
            CommandUnit tmUnit;
            tmUnit.commandLen = 8;
            tmUnit.ackLen = 7;
            tmUnit.commandId[0]=0x24;
            tmUnit.commandId[1]=0x24;
            tmUnit.commandId[2]=0x00;
            tmUnit.commandId[3]=((m_addresscode&0xFF00)>>8);
            tmUnit.commandId[4]=(m_addresscode&0x00FF);
            tmUnit.commandId[5]=0x00;
            tmUnit.commandId[6]=0x01;
            tmUnit.commandId[7]=0xF2;
            tmUnit.commandId[1]=((0x00-(tmUnit.commandId[3]+
                                  tmUnit.commandId[4]+
                                 tmUnit.commandId[6]+
                    tmUnit.commandId[7]))&0xFF);
            vector<CommandUnit> vtUnit;
            vtUnit.push_back(tmUnit);
            tmUnit.commandId[6]=0x02;
            tmUnit.commandId[7]=0x80;
            tmUnit.commandId[8]=0x31;
            tmUnit.commandLen = 9;
            tmUnit.commandId[1]=((0x00-(tmUnit.commandId[3]+
                                  tmUnit.commandId[4]+
                                 tmUnit.commandId[6]+
                    tmUnit.commandId[7]+tmUnit.commandId[8]))&0xFF);
            vtUnit.push_back(tmUnit);
            tmUnit.commandId[7]=0x81;
            tmUnit.commandId[1]=((0x00-(tmUnit.commandId[3]+
                                  tmUnit.commandId[4]+
                                 tmUnit.commandId[6]+
                    tmUnit.commandId[7]+tmUnit.commandId[8]))&0xFF);
            vtUnit.push_back(tmUnit);
            tmUnit.commandLen = 8;
            tmUnit.commandId[6]=0x01;
            tmUnit.commandId[7]=0x02;
            tmUnit.commandId[1]=((0x00-(tmUnit.commandId[3]+
                                  tmUnit.commandId[4]+
                                 tmUnit.commandId[6]+
                    tmUnit.commandId[7]))&0xFF);
            vtUnit.push_back(tmUnit);
            tmUnit.commandId[7]=0x03;
            tmUnit.commandId[1]=((0x00-(tmUnit.commandId[3]+
                                  tmUnit.commandId[4]+
                                 tmUnit.commandId[6]+
                    tmUnit.commandId[7]))&0xFF);
            vtUnit.push_back(tmUnit);
            tmUnit.commandId[7]=0x04;
            tmUnit.commandId[1]=((0x00-(tmUnit.commandId[3]+
                                  tmUnit.commandId[4]+
                                 tmUnit.commandId[6]+
                    tmUnit.commandId[7]))&0xFF);
            vtUnit.push_back(tmUnit);
            tmUnit.commandId[7]=0x05;
            tmUnit.commandId[1]=((0x00-(tmUnit.commandId[3]+
                                  tmUnit.commandId[4]+
                                 tmUnit.commandId[6]+
                    tmUnit.commandId[7]))&0xFF);
            vtUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
            tmUnit.commandLen=10;
            tmUnit.ackLen=7;
            tmUnit.commandId[5]=0x01;
            tmUnit.commandId[6]=0x03;
            tmUnit.commandId[7]=0x13;
            tmUnit.commandId[8]=0x01;
            tmUnit.commandId[9]=0x01;
            tmUnit.commandId[1]=((0x00-(tmUnit.commandId[3]+
                                  tmUnit.commandId[4]+
                                 tmUnit.commandId[6]+tmUnit.commandId[7]+
                    tmUnit.commandId[8]+tmUnit.commandId[9]))&0xFF);
            vector<CommandUnit> vtHTurnOnUnit;
            vtHTurnOnUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtHTurnOnUnit;
            tmUnit.commandId[9]=0x00;
            tmUnit.commandId[1]=((0x00-(tmUnit.commandId[3]+
                                  tmUnit.commandId[4]+
                                 tmUnit.commandId[6]+tmUnit.commandId[7]+
                    tmUnit.commandId[8]+tmUnit.commandId[9]))&0xFF);
            vector<CommandUnit> vtTurnOffUnit;
            vtTurnOffUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnOffUnit;
            tmUnit.commandLen=12;
            tmUnit.commandId[6]=0x05;
            tmUnit.commandId[7]=0xFC;
            tmUnit.commandId[8]=0x03;
            tmUnit.commandId[9]=0x55;
            tmUnit.commandId[10]=0xAA;
            tmUnit.commandId[11]=0x0D;
            tmUnit.commandId[1]=((0x00-(tmUnit.commandId[3]+tmUnit.commandId[11]+
                                  tmUnit.commandId[4]+tmUnit.commandId[10]+
                                 tmUnit.commandId[6]+tmUnit.commandId[7]+
                    tmUnit.commandId[8]+tmUnit.commandId[9]))&0xFF);
            //MSG_DEV_RESET_OPR
            vector<CommandUnit> vtResetUnit;
            vtResetUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_DEV_RESET_OPR] = vtResetUnit;
        }
            break;
        default:
            break;
        }
    }

    int GlsqTransmmit::Glsq_T_Data(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
    {
        if(data[2]!=0)//不是查询数据，不解析
            return 0;
        int datype=0;
        if(data[6]==0x80 || data[6]==0x81)
            datype = data[6];
        else
            datype = data[5];
        switch(datype){
        case 0xF2:{
            int lastlen=nDataLen;
            data+=6;
            lastlen-=6;
            int paramindex = 0;
            int datalen = 0;
            float npower=0,rfpower=0;
            DataInfo dtinfo;
            while(lastlen>2)
            {
                paramindex=data[0];
                datalen = data[1];
                if(paramindex==0x32)
                {
                   // data_ptr->mValues
                    dtinfo.bType = false;
                    dtinfo.fValue = ((data[2]<<8)|data[3])*0.001;
                    npower=((data[3]<<8)|data[2]);
                    dtinfo.sValue=(QString("%1").arg(dtinfo.fValue)).toStdString();
                    data_ptr->mValues[0]=dtinfo;
                }
                else if(paramindex==0x31){
                    dtinfo.bType = false;
                    dtinfo.fValue = ((data[2]<<8)|data[3]);
                    dtinfo.sValue=(QString("%1").arg(dtinfo.fValue)).toStdString();
                    rfpower=((data[3]<<8)|data[2]);
                    data_ptr->mValues[1]=dtinfo;
                }
                data+=(datalen+2);
                lastlen-=(datalen+2);
            }
            dtinfo.bType = false;
            if(npower>rfpower)
            {
                dtinfo.fValue=(sqrt(npower)+sqrt(rfpower))/(sqrt(npower)-sqrt(rfpower));
            }
            else
                dtinfo.fValue=0;
            dtinfo.sValue=(QString("%1").arg(dtinfo.fValue)).toStdString();
            data_ptr->mValues[2]=dtinfo;
            return 0;
        }
        case 0x80:{
            return GetExcitData(data,data_ptr,nDataLen,0);
        }
        case 0x81:{
            return GetExcitData(data,data_ptr,nDataLen,1);
        }
        case 0x02:{
            return GetPowerampData(data,data_ptr,nDataLen,0);
        }
        case 0x03:{
            return GetPowerampData(data,data_ptr,nDataLen,1);
        }
        case 0x04:{
            return GetPowerampData(data,data_ptr,nDataLen,2);
        }
        case 0x05:{
            return GetPowerampData(data,data_ptr,nDataLen,3);
        }
        }
        return 0;
    }

    int GlsqTransmmit::GetExcitData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int excitnum)
    {
        int baseindex=3,step=26;
        DataInfo dtinfo;
        QString saveStr;
        dtinfo.bType=false;
        dtinfo.fValue =data[8];
        int iFreq=0;
        if(data[8]>2 && data[8]<10)
        {
            iFreq = 171+(data[8]-3)*8;
        }
        else if(data[8]>=10)
        {
            iFreq = 450+(data[8]-10)*8;
        }
        saveStr = QString("%1MHz").arg(iFreq);
        dtinfo.sValue = saveStr.toStdString();
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue=data[9];
        dtinfo.sValue = (data[9]==0 ? DTMB_DTMB_MOD_STR(0):DTMB_DTMB_MOD_STR(1));
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue=data[10];
        if(data[10]<=0x0A)
           dtinfo.sValue = LDPCQAM[data[10]];
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue=data[11];
        if(data[11]<=0x04)
           dtinfo.sValue = DTMB_CHPN_STR(data[11]);
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue=data[12];
        if(data[12]<=0x02)
           dtinfo.sValue = DTMB_CPILOT_STR(data[12]);
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue=data[13];
        dtinfo.sValue = (data[13]==0x00 ? DTMB_BASE_STR(15):DTMB_BASE_STR(16));
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue=data[14];
        if(data[10]== 0x00)
            dtinfo.sValue = GLOBAL_STR(0);
        else if(data[10]== 0x01)
            dtinfo.sValue = GLOBAL_STR(1);
        else if(data[10]== 0x02)
            dtinfo.sValue = GLOBAL_STR(2);
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue=data[15];
        if(data[15]==0x00)
            dtinfo.sValue = "TS1_USED";
        else if(data[15]==0x01)
            dtinfo.sValue = "TS2_USED";
        else if(data[15]==0x04)
            dtinfo.sValue = "IP2TS_USED";
        else if(data[15]==0x07)
            dtinfo.sValue = "IP2TS_ERROR";
        else
            dtinfo.sValue = "NONE";
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.bType=TRUE;
        dtinfo.fValue = data[16];
        dtinfo.sValue = (data[16]==0x00 ? "LOCK":"ERROR");
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;

        dtinfo.bType=false;
        dtinfo.fValue = data[17];
        if(data[17]==0x00)
            dtinfo.sValue = "NoGPS";
        else if(data[17]==0x01)
            dtinfo.sValue = "UnLOCK";
        else
            dtinfo.sValue = "LOCK";
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.bType=TRUE;
        dtinfo.fValue = data[18];
        dtinfo.sValue = (data[18]==0x00 ? "LOCK":"ERROR");
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue = data[19];
        dtinfo.sValue = (data[19]==0x00 ? "LOCK":"ERROR");
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue = data[20];
        dtinfo.sValue = (data[20]==0x00 ? GLOBAL_STR(3):DTMB_BASE_STR(0));
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.bType=false;
        int iMark = ((data[22]&0x03)>>1);
        int iTemp = (((data[22]&0x01)<<8)|data[21]);
        if(iMark==1)
        {
            dtinfo.fValue = iTemp*(-0.25);
        }
        else
        {
            dtinfo.fValue = iTemp*(0.25);
        }
        saveStr = QString("%1°C").arg(dtinfo.fValue);
        dtinfo.sValue = saveStr.toStdString();
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue =data[23];
        if(data[23]==0x00)
            dtinfo.sValue = "9600 bps";
        else if(data[23]==0x01)
            dtinfo.sValue = "19200 bps";
        else if(data[23]==0x02)
            dtinfo.sValue = "38400 bps";
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.bType=true;
        dtinfo.fValue =data[24];
        dtinfo.sValue = (data[24]==0x00 ? DTMB_RF_MOD_STR(1):DTMB_RF_MOD_STR(0));
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.bType=false;
        dtinfo.fValue =data[25];
        if(data[25]==0x00)
            dtinfo.sValue = DTMB_BASE_STR(1);
        else if(data[25]==0x01)
            dtinfo.sValue = DTMB_BASE_STR(2);
        else if(data[25]==0x02)
            dtinfo.sValue = DTMB_BASE_STR(3);
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue =((data[27]<<8)|data[26])*0.05-10.00;
        dtinfo.sValue = (QString("%1dBm").arg(dtinfo.fValue)).toStdString();
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue =((((((data[31]<<8)|data[30])<<8)|data[29])<<8)|data[28]);
        dtinfo.sValue = (QString("%1bps").arg(dtinfo.fValue)).toStdString();
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;

        dtinfo.bType = true;
        dtinfo.fValue =data[32];
        dtinfo.sValue = (data[32]==0x00 ? GLOBAL_STR(3):DTMB_BASE_STR(6));
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue =data[33];
        dtinfo.sValue = (data[33]==0x00 ? "DISA":"ENA");
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue =data[34];
        dtinfo.sValue = (data[34]==0x00 ? "DISA":"ENA");
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue =data[35];
        dtinfo.sValue = (data[35]==0x00 ? "OVERFLOW":"OK");
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue =data[36];
        dtinfo.sValue = (data[36]==0x00 ? GLOBAL_STR(6):DTMB_BASE_STR(3));
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;

        dtinfo.bType = false;
        dtinfo.fValue =data[37];
        if(data[37]>=0 && data[37]<=0x07)
            dtinfo.sValue = ADPCCheckInfo[data[37]];
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        dtinfo.fValue =data[38];
        dtinfo.sValue = (data[38]==0x00 ? "CTTB":"CMMB");
        data_ptr->mValues[baseindex+step*excitnum] = dtinfo;
        baseindex++;
        return 0;
    }

    int GlsqTransmmit::GetPowerampData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int pampnum)
    {
        int baseindex=54,step=12;
        DataInfo dtinfo;
        dtinfo.bType = false;
        QString saveStr;
        int lastlen=nDataLen;
        data+=6;
        lastlen-=6;
        int paramindex = 0;
        int datalen = 0;
        while(lastlen>2){
            paramindex=data[0];
            datalen = data[1];
            switch(paramindex){
            case 0x01:{
                dtinfo.fValue=data[2];
                dtinfo.sValue = QString("%1V").arg(data[2]).toStdString();
                data_ptr->mValues[55+12*pampnum]=dtinfo;
            }
                break;
            case 0x02:{
                dtinfo.fValue=data[2]*0.1;
                dtinfo.sValue = QString("%1A").arg(dtinfo.fValue).toStdString();
                data_ptr->mValues[56+12*pampnum]=dtinfo;
            }
                break;
            case 0x03:{
                dtinfo.fValue=data[2]*0.1;
                dtinfo.sValue = QString("%1A").arg(dtinfo.fValue).toStdString();
                data_ptr->mValues[57+12*pampnum]=dtinfo;
            }
                break;
            case 0x04:{
                dtinfo.fValue=data[2]*0.1;
                dtinfo.sValue = QString("%1A").arg(dtinfo.fValue).toStdString();
                data_ptr->mValues[58+12*pampnum]=dtinfo;
            }
                break;
            case 0x05:{
                dtinfo.fValue=data[2]*0.1;
                dtinfo.sValue = QString("%1A").arg(dtinfo.fValue).toStdString();
                data_ptr->mValues[59+12*pampnum]=dtinfo;
            }
                break;
            case 0x06:{
                dtinfo.fValue=data[2];
                dtinfo.sValue = (QString("%1").arg(dtinfo.fValue)).toStdString();
                data_ptr->mValues[60+12*pampnum]=dtinfo;
            }
                break;
            case 0x07:{
                dtinfo.fValue=data[2];
                dtinfo.sValue = QString("%1").arg(dtinfo.fValue).toStdString();
                data_ptr->mValues[61+12*pampnum]=dtinfo;
            }
                break;
            case 0x08:{
                dtinfo.fValue=data[2];
                dtinfo.sValue = QString("%1").arg(dtinfo.fValue).toStdString();
                data_ptr->mValues[62+12*pampnum]=dtinfo;
            }
                break;
            case 0x09:{
                dtinfo.fValue=data[2];
                dtinfo.sValue = QString("%1").arg(dtinfo.fValue).toStdString();
                data_ptr->mValues[63+12*pampnum]=dtinfo;
            }
                break;
            case 0x0A:{
                dtinfo.fValue=data[2]*0.1;
                dtinfo.sValue = QString("%1").arg(dtinfo.fValue).toStdString();
                data_ptr->mValues[64+12*pampnum]=dtinfo;
            }
                break;
            case 0x0B:{
                dtinfo.fValue=data[2];
                //dtinfo.sValue = QString("%1").arg(dtinfo.fValue).toStdString();
                if(data[2]==0x01)
                    dtinfo.sValue = GLOBAL_STR(1);
                else if(data[2]==0x00)
                    dtinfo.sValue = GLOBAL_STR(0);
                else
                    dtinfo.sValue = GLOBAL_STR(9);
                data_ptr->mValues[65+12*pampnum]=dtinfo;
            }
                break;
            case 0x0C:{
                dtinfo.fValue=data[2];
                dtinfo.sValue = GET_POWER_STR(data[2]);
                data_ptr->mValues[66+12*pampnum]=dtinfo;
            }
                break;
            default:
                break;
            }
            data+=(datalen+2);
            lastlen-=(datalen+2);
        }
        return 0;
    }
}
