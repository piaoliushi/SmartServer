#include "Media_message.h"
#include"../../../utility.h"
#include "../../../database/DataBaseOperation.h"
#include "../../../LocalConfig.h"
using namespace db;
namespace hx_net
{

    Media_message::Media_message(session_ptr pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo,
                                 pDevicePropertyExPtr devProperty)
		:m_pSession(pSession)
        ,d_devInfo(devInfo)
        ,d_devProperty(devProperty)
        ,task_timeout_timer_(io_service)
    {
        time_t curTm = time(0);
        if(devInfo.bMulChannel==true){
            for(int i=0;i<devInfo.iChanSize;++i)
                tmLastSaveTime[i]=curTm;
        }else
            tmLastSaveTime[0]=curTm;
        if(IsStandardCommand())
            d_curData_ptr = DevMonitorDataPtr(new Data);
	}

    Media_message::~Media_message(void)
	{
	}

    int Media_message::check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number)
	{
        switch (d_devInfo.nSubProtocol)
        {
        case MD_740P:{
            if(data[0]!=0x7E || data[1]!=0x74)
                return RE_HEADERROR;
            else
            {
                return int((data[5]<<8)|data[6]);
            }

        }
        case MD_740BD_II:{
            if(data[0]!=0x7E || data[4]!=0x74)
                return RE_HEADERROR;
            else
            {
                return int((data[3]<<8)|data[2])-3;
            }
        }
        case MD_760BD_IV:{
            if(data[0]!=0x7E || data[4]!=0x22)
                return RE_HEADERROR;
            else
            {
                return int((data[3]<<8)|data[2])-3;
            }
        }
        case DTMB_BD:{
            if(data[0]!=0x7E)
                return RE_HEADERROR;
            else
            {
                return data[2];
            }
        }
        }
		return RE_UNKNOWDEV;
	}

    int Media_message::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode)
	{
        if(data_ptr!=NULL)
            d_curData_ptr = data_ptr;
        switch (d_devInfo.nSubProtocol)
        {
        case MD_740P:
            return Md740PData(data,data_ptr,nDataLen,iaddcode);
        case MD_740BD_II:
            return Md740BDData(data,data_ptr,nDataLen,iaddcode);
        case MD_760BD_IV:
            return Md760BDData(data,data_ptr,nDataLen,iaddcode);
        case DTMB_BD:
            return DtmbBDData(data,data_ptr,nDataLen,iaddcode);
        }
		return RE_UNKNOWDEV;
	}

    bool Media_message::IsStandardCommand()
	{
        switch (d_devInfo.nSubProtocol)
		{
        case MEDIA_DTMB:
        case MD_740P:
        case MD_740BD_II:
        case MD_760BD_IV:
        case DTMB_BD:
			return true;
		}
		return false;
	}
	
    void Media_message::GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit)
	{
	}

    void Media_message::GetAllCmd( CommandAttribute &cmdAll )
	{
        switch (d_devInfo.nSubProtocol)
        {
        case MD_740P:{
            CommandUnit tmUnit;
            tmUnit.commandLen = 13;
            tmUnit.ackLen = 7;
            tmUnit.commandId[0] = 0x7E;
            tmUnit.commandId[1] = 0x74;
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
        case MD_740BD_II:{
            CommandUnit tmUnit;
            tmUnit.commandLen = 14;
            tmUnit.ackLen = 7;
            tmUnit.commandId[0] = 0x7E;
            tmUnit.commandId[1] = 0x11;
            tmUnit.commandId[2] = 0x0A;
            tmUnit.commandId[3] = 0x00;
            tmUnit.commandId[4] = 0x74;
            tmUnit.commandId[5] = (d_devInfo.iAddressCode&0x00FF);
            tmUnit.commandId[6] = ((d_devInfo.iAddressCode&0xFF00)>>8);
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
        case MD_760BD_IV:{
            CommandUnit tmUnit;
            tmUnit.commandLen = 0;
            tmUnit.ackLen = 7;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case DTMB_BD:{
            CommandUnit tmUnit;
            tmUnit.commandLen = 5;
            tmUnit.ackLen = 3;
            tmUnit.commandId[0] = 0x7E;
            tmUnit.commandId[1] = 0x56;
            tmUnit.commandId[2] = 0x02;
            tmUnit.commandId[3] = 0x05;
            tmUnit.commandId[4] = 0x55;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        }
    }

    //添加新告警
    bool  Media_message::add_new_alarm(string sPrgName,int alarmId,int nState,time_t  startTime)
    {
        //告警产生
        boost::recursive_mutex::scoped_lock lock(alarm_mutex_);
        if(nState == 0){
               CurItemAlarmInfo  tmp_alarm_info;
               map<string ,map<int,CurItemAlarmInfo > >::iterator iter= mapProgramAlarm.find(sPrgName);
               if(iter!=mapProgramAlarm.end()){
                   map<int,CurItemAlarmInfo >::iterator iter_type = iter->second.find(alarmId);
                   //没找到对应告警l
                   if(iter_type==iter->second.end()){
                       tmp_alarm_info.startTime = startTime;//记录时间
                       tmp_alarm_info.nType = alarmId;//告警类型
                       tmp_alarm_info.nLimitId = ALARM_SWITCH;//限值类型
                       tmp_alarm_info.bNotifyed = false;
                       iter->second[alarmId] = tmp_alarm_info;
                        record_alarm_and_notify(sPrgName,0,mapProgramAlarm[sPrgName][alarmId]);
                   }
               }else{
                   //没有找到增加该告警监控项
                   tmp_alarm_info.startTime = startTime;//记录时间
                   tmp_alarm_info.nType = alarmId;//告警类型
                   tmp_alarm_info.nLimitId = ALARM_SWITCH;//限值类型
                   tmp_alarm_info.bNotifyed = false;
                   map<int,CurItemAlarmInfo>  tmTypeAlarm;
                   tmTypeAlarm[alarmId] = tmp_alarm_info;
                   mapProgramAlarm[sPrgName] = tmTypeAlarm;
                   record_alarm_and_notify(sPrgName,0,mapProgramAlarm[sPrgName][alarmId]);
               }

        }else if(nState==1){
            map<string ,map<int,CurItemAlarmInfo > >::iterator iter= mapProgramAlarm.find(sPrgName);
            if(iter!=mapProgramAlarm.end()){
                map<int,CurItemAlarmInfo >::iterator iter_type = iter->second.find(alarmId);
                if(iter_type!=iter->second.end()){
                    //记录数据库
                    record_alarm_and_notify(sPrgName,1,iter_type->second);
                    iter->second.erase(iter_type);
                }
            }
        }
        return true;
    }

    void  Media_message::record_alarm_and_notify(string &frqName,int nMod,CurItemAlarmInfo &curAlarm)
     {
         static  char str_time[64];
         tm *local_time = localtime(&curAlarm.startTime);
         strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", local_time);
         bool bRslt = GetInst(DataBaseOperation).AddProgramSignalAlarmRecord(d_devInfo.sDevNum,frqName,curAlarm.startTime,
                                                                             curAlarm.nLimitId,curAlarm.nType,curAlarm.nAlarmId);
         if(bRslt==true){
             //发送监控量报警到客户端(用频点名称填充设备名称,cellId填充告警类型)
           m_pSession->send_alarm_state_message(GetInst(LocalConfig).local_station_id(),d_devInfo.sDevNum,frqName,curAlarm.nType
                                      ,d_devInfo.iDevType,curAlarm.nLimitId,str_time,mapProgramAlarm[frqName].size(),curAlarm.sReason);
         }

    }

    int Media_message::Md740PData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
    {
        if(data[1]!=0x11)
            return RE_CMDACK;
        int index = 0;
        iaddcode = data[3]*256+data[4];
        DataInfo dtinfo;
        for(int i=0;i<6;i++)
        {
            dtinfo.bType = false;
            dtinfo.fValue = data[6*i+7];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = true;
            dtinfo.fValue = data[6*i+8];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = false;
            dtinfo.fValue = data[6*i+9];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.fValue = data[6*i+10];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = true;
            dtinfo.fValue = data[6*i+11];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = false;
            dtinfo.fValue = data[6*i+12];
            data_ptr->mValues[index++] = dtinfo;
        }
        return RE_SUCCESS;
    }

    int Media_message::Md740BDData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
    {
        if(data[1]!=0x11)
            return RE_CMDACK;
        int index = 0;
        iaddcode = data[6]*256+data[5];
        DataInfo dtinfo;
        for(int i=0;i<2;i++)
        {
            dtinfo.bType = false;
            dtinfo.fValue = data[8*i+8];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = true;
            dtinfo.fValue = data[8*i+9];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = false;
            dtinfo.fValue = data[8*i+10];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.fValue = data[8*i+11];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = true;
            dtinfo.fValue = data[8*i+12];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = false;
            dtinfo.fValue = data[8*i+13];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = true;
            dtinfo.fValue = data[8*i+14];
            data_ptr->mValues[index++] = dtinfo;
        }
        return RE_SUCCESS;
    }

    int Media_message::Md760BDData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
    {
        if(data[1]!=0x11)
            return RE_CMDACK;
        int index = 0;
        iaddcode = data[6]*256+data[5];
        DataInfo dtinfo;
        for(int i=0;i<4;++i)
        {
            dtinfo.bType = true;
            dtinfo.fValue = data[12*i+7];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = false;
            if(data[12*i+7]==0x00)
            {
                dtinfo.fValue = (data[12*i+8]+data[12*i+9]*256)*0.01;
            }
            else
            {
                dtinfo.fValue = (data[12*i+8]+data[12*i+9]*256);
            }
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.fValue = data[12*i+10];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.fValue = data[12*i+11];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = true;
            dtinfo.fValue = data[12*i+12];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.fValue = data[12*i+13];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = false;
            dtinfo.fValue = data[12*i+14];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.fValue = data[12*i+15];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.fValue = data[12*i+16];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = true;
            for(int j=0;j<7;++j)
            {
                dtinfo.fValue = Getbit(data[12*i+17],j);
                data_ptr->mValues[index++] = dtinfo;
            }
            for(int k=0;k<4;++k)
            {
                dtinfo.fValue = Getbit(data[12*i+17],k);
                data_ptr->mValues[index++] = dtinfo;
            }
        }
    }

    int Media_message::DtmbBDData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
    {
        if(data[1]!=0x56)
            return RE_CMDACK;
        iaddcode = d_devInfo.iAddressCode;
        DataInfo dtinfo;
        int index = 0;
        for(int i=0;i<4;i++)
        {
            dtinfo.bType = false;
            dtinfo.fValue = (data[4+i*5]+data[5+i*5]*256+data[6+i*5]*256*256+data[7+i*5]*256*256*256)*0.01;
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = true;
            dtinfo.fValue = data[8+i*5];
            data_ptr->mValues[index++] = dtinfo;
        }
        for(int i=0;i<4;++i)
        {
            dtinfo.bType = false;
            dtinfo.fValue = data[24+i*12];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.fValue = data[25+i*12];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = true;
            dtinfo.fValue = data[26+i*12];
            data_ptr->mValues[index++] = dtinfo;
            dtinfo.bType = false;
            dtinfo.fValue = data[27+i*12];
            data_ptr->mValues[index++] = dtinfo;
            double ber;
            *(((char*)(&ber) + 0)) = data[28+12*i];
            *(((char*)(&ber) + 1)) = data[29+12*i];
            *(((char*)(&ber) + 2)) = data[30+12*i];
            *(((char*)(&ber) + 3)) = data[31+12*i];
            *(((char*)(&ber) + 4)) = data[32+12*i];
            *(((char*)(&ber) + 5)) = data[33+12*i];
            *(((char*)(&ber) + 6)) = data[34+12*i];
            *(((char*)(&ber) + 7)) = data[35+12*i];
            dtinfo.fValue = ber;
            data_ptr->mValues[index++] = dtinfo;
        }
    }

    //添加数据
    bool  Media_message::add_new_data(string sIp,int nChannel,DevMonitorDataPtr &mapData)
    {

        m_pSession->send_monitor_data_message(GetInst(LocalConfig).local_station_id(),d_devInfo.sDevNum,
                                              d_devInfo.iDevType,mapData,d_devInfo.map_MonitorItem);
        time_t tmCurTime;
        time(&tmCurTime);
        double ninterval = difftime(tmCurTime,tmLastSaveTime[nChannel]);
        if(ninterval<d_devProperty->data_save_interval)//间隔保存时间 need amend;
            return  false;
        if(GetInst(DataBaseOperation).AddItemMonitorRecord(d_devInfo.sDevNum,tmCurTime,mapData,d_devInfo.map_MonitorItem))
            tmLastSaveTime[nChannel] = tmCurTime;
        return true;
    }
}
