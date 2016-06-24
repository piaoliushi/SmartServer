#include "ShTransmmit.h"
#include "../../../../utility.h"
#include "../../../../StationConfig.h"
#include <QString>
#include<QtCore/qmath.h>
namespace hx_net{
const char LDPCQAM[][16]={"0.8&4QAMNR","0.4&4QAM","0.6&4QAM",
                          "0.8&4QAM","0.4&16QAM","0.6&16QAM",
                          "0.8&16QAM","0.8&32QAM","0.4&64QAM",
                          "0.6&64QAM","0.8&64QAM"};
//const char chPN[][32]={"",""};//{"420固定相位","420旋转相位","595",
//                       //"945固定相位","945旋转相位"};
//const char CPILOT[][32]={"",""};//{"单载波关导频","单载波开导频","多载波无导频"};
//const char ADPC[][32]={"",""};//{"关闭","开启","更新"};
const char ADPCCheckInfo[][32]={"OK","FEEDBACK LINK TOO LARGE","FEEDBACK LINK TOO SMALL",
                                "","POWER UNSTABLE","ADPC ERROR","IN_B TOO SMALL",
                                "TIMEOUT(15分钟)"};
const char AGCstate[][10]={"OFF","INT","A_IN","IN_A"};
//const char GPSState[][32]={"",""};//{"初始化","未知状态","正常","短路","开路"};

#define DTMB_CHPN_STR(Y)  GetInst(StationConfig).get_dictionary_value("s_sh_tsmt_chpn",Y)
#define DTMB_CPILOT_STR(Y)  GetInst(StationConfig).get_dictionary_value("s_sh_tsmt_cpilot",Y)
#define DTMB_BASE_STR(Y)  GetInst(StationConfig).get_dictionary_value("s_dtmb_base",Y)
#define DTMB_GPS_STR(Y)  GetInst(StationConfig).get_dictionary_value("s_sh_tsmt_gps_s",Y)
#define DTMB_RF_MOD_STR(Y)  GetInst(StationConfig).get_dictionary_value("s_sh_tsmt_rf_mod",Y)
#define DTMB_DTMB_MOD_STR(Y)  GetInst(StationConfig).get_dictionary_value("s_dtmb_mod",Y)


    ShTransmmit::ShTransmmit(dev_session_ptr pSession,int subprotocol,int addresscode)
        :Transmmiter()
        ,m_subprotocol(subprotocol)
        ,m_addresscode(addresscode)
        ,m_curMsgType(NO_CMDTYPE)
        ,m_curMsgId(0)
        ,m_pSession(pSession)
        //,m_sDevId(sDevId)
    {
    }

    ShTransmmit::~ShTransmmit()
    {

    }

    int ShTransmmit::check_msg_header( unsigned char *data,int nDataLen ,CmdType cmdType,int number)
    {
        m_curMsgType = cmdType;
        m_curMsgId = number;
        switch(m_subprotocol)
        {
        case All_Band_Pa:
            {
                if(data[0]==0xA5 && data[1]==0x5A)
                    return (((data[3]<<8)|data[2])-4);
                else
                    return -1;
            }
        case All_Band_Exc:
        {
            if(data[0]==0x00 && data[1]==0x76 && data[3]==0x80)
                return (data[2]-4);
            else
                return -1;
        }
        default:
            return -1;
        }
        return -1;
    }

    bool ShTransmmit::isLastQueryCmd()
    {
          if(m_curMsgId == m_maxMsgId-1)
              return true;
          return false;
    }

    bool ShTransmmit::IsStandardCommand()
    {
        switch(m_subprotocol)
        {
        case All_Band_Pa:
        case All_Band_Exc:
            return true;
        }
        return false;
    }

    void ShTransmmit::GetSignalCommand( devCommdMsgPtr lpParam,CommandUnit &cmdUnit )
    {

    }

    int ShTransmmit::decode_msg_body( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate )
    {

        int nResult=-1;
        switch(m_subprotocol)
        {
        case All_Band_Pa:{
            nResult = OnAllBandData(data,data_ptr,nDataLen,runstate);

            break;
        }
        case All_Band_Exc:{
            nResult = OnAllBandExcData(data,data_ptr,nDataLen,runstate);
            break;
        }
        }
        return nResult;
    }

     bool ShTransmmit::isMultiQueryCmd()
     {
         switch(m_subprotocol)
         {
         case All_Band_Pa:
             return true;
         }
         return false;
     }

    void ShTransmmit::GetAllCmd( CommandAttribute &cmdAll )
    {
        switch(m_subprotocol)
        {
        case All_Band_Pa:
            {
                CommandUnit tmUnit;
                tmUnit.ackLen = 4;
                tmUnit.commandLen=16;
                tmUnit.commandId[0] = 0xA5;
                tmUnit.commandId[1] = 0x5A;
                tmUnit.commandId[2] = 0x10;
                tmUnit.commandId[3] = 0x00;
                tmUnit.commandId[4] = 0x00;
                tmUnit.commandId[5] = 0x00;
                tmUnit.commandId[6] = 0x00;
                tmUnit.commandId[7] = 0x00;
                tmUnit.commandId[8] = 0x01;
                tmUnit.commandId[9] = 0x00;
                tmUnit.commandId[10] = 0x01;
                tmUnit.commandId[11] = 0x00;
                /*int isum=0;
                for(int i=0;i<12;++i)
                {
                    isum+=tmUnit.commandId[i];
                }
                isum = isum&0xFFFF;
                tmUnit.commandId[12] = ((isum&0xFF00)>>8);
                tmUnit.commandId[13] = (isum&0xFF);*/
                tmUnit.commandId[12] = 0x11;
                tmUnit.commandId[13] = 0x01;

                tmUnit.commandId[14] = 0xEF;
                tmUnit.commandId[15] = 0xFE;
                vector<CommandUnit> vtUnit;
                vtUnit.push_back(tmUnit);
                /*tmUnit.commandId[10] = 0x02;
                tmUnit.commandId[12] = 0x12;
                vtUnit.push_back(tmUnit);*/
                tmUnit.commandId[10] = 0x20;
                tmUnit.commandId[12] = 0x30;
                vtUnit.push_back(tmUnit);
                tmUnit.commandId[10] = 0x03;
                tmUnit.commandId[12] = 0x13;
                vtUnit.push_back(tmUnit);
                /*tmUnit.commandId[10] = 0x04;
                tmUnit.commandId[12] = 0x14;
                vtUnit.push_back(tmUnit);
                tmUnit.commandId[10] = 0x05;
                tmUnit.commandId[12] = 0x15;
                vtUnit.push_back(tmUnit);
                tmUnit.commandId[10] = 0x10;
                tmUnit.commandId[12] = 0x20;
                vtUnit.push_back(tmUnit);*/
                cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
                m_maxMsgId = 3;
            }
            break;
        case All_Band_Exc:
            {
               CommandUnit tmUnit;
               tmUnit.ackLen = 4;
               tmUnit.commandLen=6;
               tmUnit.commandId[0] = 0x80;
               tmUnit.commandId[1] = 0x46;
               tmUnit.commandId[2] = 0x06;
               tmUnit.commandId[3] = 0x31;
               tmUnit.commandId[4] = 0xFD;
               tmUnit.commandId[5] = 0x96;
               vector<CommandUnit> vtUnit;
               vtUnit.push_back(tmUnit);
               tmUnit.commandId[3] = 0x32;
               vtUnit.push_back(tmUnit);
               tmUnit.commandId[3] = 0x33;
               vtUnit.push_back(tmUnit);
               cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
               m_maxMsgId = 3;
            }
            break;
        }
    }

    int ShTransmmit::OnAllBandData( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate )
    {
        int nDataType = data[10];
        switch(nDataType)
        {
        case 0x01://功放整体运行状态
            {
                int index = 0;
                int basebit=11;
                DataInfo dainfo;
                dainfo.bType = false;
                for(int i=0;i<9;++i)
                {
                    int nValue=0;

                    for(int j=4;j>0;--j)
                    {
                        int num = basebit+j+4*i;
                        nValue = ((nValue<<8)|data[num]);
                    }
                    dainfo.fValue = nValue*0.01;
                    if(i==0)
                        dainfo.fValue/=1000;
                    dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
                    data_ptr->mValues[index++] = dainfo;
                }

            }
            break;
        case 0x02://功放整体告警状态
            {
                int index = 9;
                int basebit=12;
                DataInfo dainfo;
                dainfo.bType = true;
                for(int i=0;i<9;++i)
                {
                    dainfo.fValue = data[basebit+i];
                    //dainfo.sValue = (data[basebit+i]==0 ? "正常":"告警");
                    dainfo.sValue = (data[basebit+i]==0 ? GLOBAL_STR(3):GLOBAL_STR(4));
                    data_ptr->mValues[index++] = dainfo;
                }
            }
            break;
        case 0x03://预放运行状态
            {
                int index = 28;
                int basebit=11;
                DataInfo dainfo;
                dainfo.bType = false;
                for(int i=0;i<9;++i)
                {
                    int nValue=0;
                    for(int j=4;j>0;--j)
                    {
                        nValue = ((nValue<<8)|data[basebit+j+4*i]);
                    }
                    dainfo.fValue = nValue*0.01;
                    dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
                    data_ptr->mValues[index++] = dainfo;
                }
            }
            break;
        case 0x04://预放告警状态
            {
                int index = 39;
                int basebit=12;
                DataInfo dainfo;
                dainfo.bType = true;
                for(int i=0;i<6;++i)
                {
                    dainfo.fValue = data[basebit+i];
                    //dainfo.sValue = (data[basebit+i]==0 ? "正常":"告警");
                    dainfo.sValue = (data[basebit+i]==0 ? GLOBAL_STR(3):GLOBAL_STR(4));
                    data_ptr->mValues[index++] = dainfo;
                }
            }
            break;
        case 0x20://功放整体运行参数
            {
                int index = 18;
                int basebit=12;
                DataInfo dainfo;
                dainfo.bType = true;
                dainfo.fValue = data[basebit];
                //dainfo.sValue = (data[basebit]==0 ? "正常":"告警");
                dainfo.sValue = (data[basebit]==0 ? GLOBAL_STR(3):GLOBAL_STR(4));
                data_ptr->mValues[index++] = dainfo;
                dainfo.bType = false;
                dainfo.fValue = data[basebit+1];
                dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
                data_ptr->mValues[index++] = dainfo;
                dainfo.fValue = ((data[basebit+3]<<8)|data[basebit+2]);
                dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
                data_ptr->mValues[index++] = dainfo;
                dainfo.fValue = data[basebit+4];
                dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
                data_ptr->mValues[index++] = dainfo;
                dainfo.fValue = data[basebit+5];
                dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
                data_ptr->mValues[index++] = dainfo;
                dainfo.fValue = data[basebit+6];
                dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
                data_ptr->mValues[index++] = dainfo;
                dainfo.fValue = data[basebit+7];
                dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
                data_ptr->mValues[index++] = dainfo;
                dainfo.fValue = ((data[basebit+9]<<8)|data[basebit+8]);
                dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
                data_ptr->mValues[index++] = dainfo;
                dainfo.fValue = data[basebit+10];
                dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
                data_ptr->mValues[index++] = dainfo;
            }
            break;
        case 0x05://后级功放模块1运行状态
            LastBandRunData(data,data_ptr,nDataLen,0);
            break;
        case 0x06://后级功放模块2运行状态
            LastBandRunData(data,data_ptr,nDataLen,1);
            break;
        case 0x07://后级功放模块3运行状态
            LastBandRunData(data,data_ptr,nDataLen,2);
            break;
        case 0x08://后级功放模块4运行状态
            LastBandRunData(data,data_ptr,nDataLen,3);
            break;
        case 0x09://后级功放模块5运行状态
            LastBandRunData(data,data_ptr,nDataLen,4);
            break;
        case 0x0A://后级功放模块6运行状态
            LastBandRunData(data,data_ptr,nDataLen,5);
            break;
        case 0x0B://后级功放模块7运行状态
            LastBandRunData(data,data_ptr,nDataLen,6);
            break;
        case 0x0C://后级功放模块8运行状态
            LastBandRunData(data,data_ptr,nDataLen,7);
            break;
        case 0x0D://后级功放模块9运行状态
            LastBandRunData(data,data_ptr,nDataLen,8);
            break;
        case 0x0E://后级功放模块10运行状态
            LastBandRunData(data,data_ptr,nDataLen,9);
            break;
        case 0x10://后级功放模块1告警状态
            LastBandSateData(data,data_ptr,nDataLen,0);
            break;
        case 0x11://后级功放模块2告警状态
            LastBandSateData(data,data_ptr,nDataLen,1);
            break;
        case 0x12://后级功放模块3告警状态
            LastBandSateData(data,data_ptr,nDataLen,2);
            break;
        case 0x13://后级功放模块4告警状态
            LastBandSateData(data,data_ptr,nDataLen,3);
            break;
        case 0x14://后级功放模块5告警状态
            LastBandSateData(data,data_ptr,nDataLen,4);
            break;
        case 0x15://后级功放模块6告警状态
            LastBandSateData(data,data_ptr,nDataLen,5);
            break;
        case 0x16://后级功放模块7告警状态
            LastBandSateData(data,data_ptr,nDataLen,6);
            break;
        case 0x17://后级功放模块8告警状态
            LastBandSateData(data,data_ptr,nDataLen,7);
            break;
        case 0x18://后级功放模块9告警状态
            LastBandSateData(data,data_ptr,nDataLen,8);
            break;
        case 0x19://后级功放模块10告警状态
            LastBandSateData(data,data_ptr,nDataLen,9);
            break;
        }
        return 0;
    }

    void ShTransmmit::LastBandRunData( unsigned char *data,DevMonitorDataPtr pBandData,int nDataLen,int nmodenum )
    {
        int index = 45+24*nmodenum;
        int basebit=12;
        DataInfo dainfo;
        dainfo.bType = false;
        for(int i=0;i<9;++i)
        {
            int nValue=0;
            for(int j=4;j>=0;--j)
            {
                nValue = ((nValue<<8)|data[basebit+j+4*i]);
            }
            dainfo.fValue = nValue*0.01;
            dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
            pBandData->mValues[index++] = dainfo;
        }
        basebit = 48;
        for(int j=0;j<3;++j)
        {
            dainfo.fValue = ((data[basebit+j+1]<<8)|data[basebit+j])*0.01;
            dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
            pBandData->mValues[index++] = dainfo;
        }
    }

    void ShTransmmit::LastBandSateData( unsigned char *data,DevMonitorDataPtr pBandData,int nDataLen,int nmodenum )
    {
        int index = 57+24*nmodenum;
        int basebit=12;
        DataInfo dainfo;
        dainfo.bType = true;
        for(int i=0;i<12;++i)
        {
            dainfo.fValue = data[basebit+i];
            //dainfo.sValue = (data[basebit+i]==0 ? "正常":"告警");
            dainfo.sValue = (data[basebit+i]==0 ? GLOBAL_STR(3):GLOBAL_STR(4));
            pBandData->mValues[index++] = dainfo;
        }
    }

    int ShTransmmit::OnAllBandExcData(unsigned char *data, DevMonitorDataPtr pBandData, int nDataLen, int &runstate)
    {
        int index = 0;
        DataInfo dainfo;
        QString saveStr;
        if(m_curMsgId==0)
        {
            dainfo.bType = false;
            dainfo.fValue =data[4];
            int iFreq=0;
            if(data[4]>2 && data[4]<10)
            {
                iFreq = 171+(data[4]-3)*8;
            }
            else if(data[4]>=10)
            {
                iFreq = 450+(data[4]-10)*8;
            }
            saveStr = QString("%1MHz").arg(iFreq);
            dainfo.sValue = saveStr.toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[5];
            //dainfo.sValue = (data[5]==0 ? "SFN单频网":"MFN多频网");
            dainfo.sValue = (data[5]==0 ? DTMB_DTMB_MOD_STR(0):DTMB_DTMB_MOD_STR(1));
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[6];//LDPCQAM
            if(data[6]<=0x0A)
               dainfo.sValue = LDPCQAM[data[6]];
            pBandData->mValues[index++] = dainfo;
            //chPN
            dainfo.fValue =data[7];
            if(data[7]<=0x04)
               dainfo.sValue = DTMB_CHPN_STR(data[7]);//chPN[data[7]];
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[8];
            if(data[8]<=0x02)
               dainfo.sValue = DTMB_CPILOT_STR(data[8]);//[data[8]];
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[9];
            //dainfo.sValue = (data[9]==0x00 ? "240浅交织":"720深交织");
            dainfo.sValue = (data[9]==0x00 ? DTMB_BASE_STR(15):DTMB_BASE_STR(16));
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[10];
            if(data[10]== 0x00)
                dainfo.sValue = GLOBAL_STR(0);//ADPC[data[10]];
            else if(data[10]== 0x01)
                dainfo.sValue = GLOBAL_STR(1);//ADPC[data[10]];
            else if(data[10]== 0x02)
                dainfo.sValue = GLOBAL_STR(2);//ADPC[data[10]];
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[11];
            if(data[11]==0x00)
                dainfo.sValue = "TS1_USED";
            else if(data[11]==0x01)
                dainfo.sValue = "TS2_USED";
            else
                dainfo.sValue = "NONE";
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[12];
            dainfo.bType = true;
            dainfo.sValue = (data[12]==0x00 ? "LOCK":"ERROR");
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[13];
            dainfo.bType = false;
            if(data[13]==0x00)
                dainfo.sValue = "NoGPS";
            else if(data[13]==0x01)
                dainfo.sValue = "UnLOCK";
            else
                dainfo.sValue = "LOCK";
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[14];
            dainfo.bType = true;
            dainfo.sValue = (data[14]==0x00 ? "LOCK":"ERROR");
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[15];
            dainfo.sValue = (data[15]==0x00 ? "LOCK":"ERROR");
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[16];
            //dainfo.sValue = (data[16]==0x00 ? "正常":"大于70度");
            dainfo.sValue = (data[16]==0x00 ? GLOBAL_STR(3):DTMB_BASE_STR(0));
            pBandData->mValues[index++] = dainfo;
            dainfo.bType = false;
            int iMark = ((data[18]&0x03)>>1);
            int iTemp = (((data[18]&0x01)<<8)|data[17]);
            if(iMark==1)
            {
                dainfo.fValue = iTemp*(-0.25);
            }
            else
            {
                dainfo.fValue = iTemp*(0.25);
            }
            saveStr = QString("%1°C").arg(dainfo.fValue);
            dainfo.sValue = saveStr.toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[19];
            if(data[19]==0x00)
                dainfo.sValue = "9600 bps";
            else if(data[19]==0x01)
                dainfo.sValue = "19200 bps";
            else if(data[19]==0x02)
                dainfo.sValue = "38400 bps";
            pBandData->mValues[index++] = dainfo;
            dainfo.bType = true;
            dainfo.fValue =data[20];
            //dainfo.sValue = (data[20]==0x00 ? "射频关闭":"射频打开");
            dainfo.sValue = (data[20]==0x00 ? DTMB_RF_MOD_STR(1):DTMB_RF_MOD_STR(0));
            pBandData->mValues[index++] = dainfo;
            dainfo.bType = false;
            dainfo.fValue =data[21];
            if(data[21]==0x00)
                dainfo.sValue = DTMB_BASE_STR(1);//"外部GPS";
            else if(data[21]==0x01)
                dainfo.sValue = DTMB_BASE_STR(2);//"内部GPS";
            else if(data[21]==0x02)
                dainfo.sValue = DTMB_BASE_STR(3);//"自动选择";
            else if(data[21]==0x03)
                dainfo.sValue = DTMB_BASE_STR(4);//"外部高级GPS";
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =((data[23]<<8)|data[22])*0.05-10.00;
            dainfo.sValue = (QString("%1dBm").arg(dainfo.fValue)).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =((((((data[27]<<8)|data[26])<<8)|data[25])<<8)|data[24]);
            dainfo.sValue = (QString("%1bps").arg(dainfo.fValue)).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.bType = true;
            dainfo.fValue =data[28];
            //dainfo.sValue = (data[28]==0x00 ? "正常":"失锁");
             dainfo.sValue = (data[28]==0x00 ? GLOBAL_STR(3):DTMB_BASE_STR(6));
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[29];
            dainfo.sValue = (data[29]==0x00 ? "DISA":"ENA");
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[31];
            dainfo.sValue = (data[31]==0x00 ? "OVERFLOW":"OK");
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[32];
            //dainfo.sValue = (data[32]==0x00 ? "失锁":"正常");
            dainfo.sValue = (data[28]==0x00 ? GLOBAL_STR(6):DTMB_BASE_STR(3));
            pBandData->mValues[index++] = dainfo;
            //ADPCCheckInfo
            dainfo.bType = false;
            dainfo.fValue =data[33];
            if(data[33]>=0 && data[33]<=0x07)
                dainfo.sValue = ADPCCheckInfo[data[33]];
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[34];
            dainfo.sValue = (data[34]==0x00 ? "CTTB":"CMMB");
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[35];
            if(data[35]==0x00)
                dainfo.sValue = GLOBAL_STR(5);//"未知";
            else if(data[35]==0x01)
                dainfo.sValue = DTMB_BASE_STR(7);//"188字节";
            else if(data[35]==0x02)
                dainfo.sValue = DTMB_BASE_STR(8);//"204字节";
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =((data[37]<<8)|data[36]);
            dainfo.sValue = (QString("%1ms").arg(dainfo.fValue)).toStdString();
            pBandData->mValues[index++] = dainfo;
        }
        else if(m_curMsgId==1)
        {
            index = 27;
            dainfo.bType = false;
            dainfo.fValue = ((data[6]<<8)|data[3]);
            dainfo.sValue = (QString("%1Hz").arg(0.25*dainfo.fValue-200)).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.bType = true;
            dainfo.fValue = data[7];
            if(data[7]==0x00)
                dainfo.sValue = DTMB_BASE_STR(9);//"单音关闭";
            else
                dainfo.sValue = DTMB_BASE_STR(10);//"单音打开";
            pBandData->mValues[index++] = dainfo;
            dainfo.bType = false;
            int idealy = (((((data[10]<<8)|data[9])<<8)|data[8]));
            dainfo.fValue = idealy;
            dainfo.sValue = (QString("%1μs").arg(0.1*dainfo.fValue)).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = data[11];
            if(data[11]==0x00)
                dainfo.sValue = "KEEP";
            else
                dainfo.sValue = "DELETE";
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = data[12];
            if(data[12]==0x00)
                dainfo.sValue = DTMB_BASE_STR(11);//"正常模式";
            else
                dainfo.sValue= DTMB_BASE_STR(12);//"ZP1S开启";//"ZP1S开启";
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = data[14];
            if(data[14]>=0 && data[14]<4)
                dainfo.sValue = AGCstate[data[14]];
            else
                dainfo.sValue = GLOBAL_STR(5);//"未知";
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = ((data[16]<<8)|data[15]);
            dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.bType = true;
            dainfo.fValue = data[17];
            if(data[17]==0x00)
                dainfo.sValue =DTMB_BASE_STR(13);// "CF关闭";
            else
                dainfo.sValue =DTMB_BASE_STR(14);// "CF开启";
            pBandData->mValues[index++] = dainfo;
            dainfo.bType = false;
            dainfo.fValue = ((((((data[18]<<8)|data[19])<<8)|data[20])<<8)|data[21]);
            dainfo.sValue = (QString("%1.%2.%3.%4").arg(data[18]).arg(data[19]).arg(data[20]).arg(data[21])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = ((((((data[22]<<8)|data[23])<<8)|data[24])<<8)|data[25]);
            dainfo.sValue = (QString("%1.%2.%3.%4").arg(data[22]).arg(data[23]).arg(data[24]).arg(data[25])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = ((((((data[26]<<8)|data[27])<<8)|data[28])<<8)|data[29]);
            dainfo.sValue = (QString("%1.%2.%3.%4").arg(data[26]).arg(data[27]).arg(data[28]).arg(data[29])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = data[30];
            dainfo.sValue = (QString("%1").arg(data[30])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = (data[31]&0x0F);
            dainfo.sValue = (QString("%1Minute").arg((data[31]&0x0F))).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = (data[32]);
            dainfo.sValue = (QString("%1").arg((data[32]))).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = (data[33])*0.5;
            dainfo.sValue = (QString("%1").arg((data[33])*0.5)).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = (data[34])*0.5;
            dainfo.sValue = (QString("%1").arg((data[34])*0.5)).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = (data[35])*0.5;
            dainfo.sValue = (QString("%1").arg((data[35])*0.5)).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = (data[36]);
            dainfo.sValue = (QString("%1").arg(data[36])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = (data[37]);
            dainfo.sValue = (QString("%1").arg(data[37])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = (data[38]);
            dainfo.sValue = (QString("%1").arg(data[38])).toStdString();
            pBandData->mValues[index++] = dainfo;
        }
        else if(m_curMsgId==2)
        {
            index = 47;
            dainfo.bType = false;
            dainfo.fValue = (data[4]);
            if(data[4]==0x00)
                dainfo.sValue = "8MHz";
            else
                dainfo.sValue = "6MHz";
            pBandData->mValues[index++] = dainfo;
            dainfo.bType = true;
            dainfo.fValue = (data[5]);
            if(data[5]==0x00)
                dainfo.sValue = GLOBAL_STR(0);// "关闭";
            else
                dainfo.sValue =GLOBAL_STR(1);// "开启";
            pBandData->mValues[index++] = dainfo;
            dainfo.bType = false;
            dainfo.fValue = ((((((data[8]<<8)|data[9])<<8)|data[10])<<8)|data[11]);
            dainfo.sValue = (QString("%1.%2.%3.%4").arg(data[8]).arg(data[9]).arg(data[10]).arg(data[11])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = ((((((data[12]<<8)|data[13])<<8)|data[14])<<8)|data[15]);
            dainfo.sValue = (QString("%1.%2.%3.%4").arg(data[12]).arg(data[13]).arg(data[14]).arg(data[15])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = ((((((data[16]<<8)|data[17])<<8)|data[18])<<8)|data[19]);
            dainfo.sValue = (QString("%1.%2.%3.%4").arg(data[16]).arg(data[17]).arg(data[18]).arg(data[19])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = ((((((data[20]<<8)|data[21])<<8)|data[22])<<8)|data[23]);
            dainfo.sValue = (QString("%1.%2.%3.%4").arg(data[20]).arg(data[21]).arg(data[22]).arg(data[23])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = ((data[25]<<8)|data[24]);
            dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue = (data[26]);
            if(data[26]==0x00)
                dainfo.sValue = "UDP";
            else
                dainfo.sValue = "TCP";
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[27];
            if(data[27]==0x00)
                dainfo.sValue = DTMB_BASE_STR(7);//"188字节";
            else
                dainfo.sValue = DTMB_BASE_STR(8);//"204字节";
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[28];
            if(data[28]==0x00)
                dainfo.sValue = GLOBAL_STR(7);//"组播";
            else
                dainfo.sValue = GLOBAL_STR(8);//"单播";
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[29];
            dainfo.sValue = (QString("%1").arg(data[29])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[30];
            dainfo.sValue = (QString("%1").arg(data[30])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[31];
            dainfo.sValue = (QString("%1").arg(data[31])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[32];
            dainfo.sValue = (QString("%1").arg(data[32])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[33];
            dainfo.sValue = (QString("%1").arg(data[33])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[34];
            dainfo.sValue = (QString("%1").arg(data[34])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[35];
            dainfo.sValue = (QString("%1").arg(data[35])).toStdString();
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[36];
            dainfo.sValue = (QString("%1").arg(data[36])).toStdString();
            pBandData->mValues[index++] = dainfo;

            dainfo.fValue = qPow(10,((data[37]&0x0F))*-1)*((data[37]&0xF0)>>4); //data[37];
            dainfo.sValue = (QString("%1").arg(dainfo.fValue)).toStdString();
            pBandData->mValues[index++] = dainfo;

            dainfo.fValue =data[38];
            dainfo.sValue = DTMB_GPS_STR(data[38]);//GPSState[data[38]];
            pBandData->mValues[index++] = dainfo;
            dainfo.fValue =data[39];
            if(data[39]==0x00)
                dainfo.sValue = "+";
            else
                dainfo.sValue = "-";
            pBandData->mValues[index++] = dainfo;
        }
     return 0;
    }
}
