#include "Media_message.h"
#include"../../../utility.h"
#include "../../../database/DataBaseOperation.h"
#include "../../../LocalConfig.h"
using namespace db;
namespace hx_net
{

    Media_message::Media_message(session_ptr pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo)
		:m_pSession(pSession)
        ,d_devInfo(devInfo)
        ,task_timeout_timer_(io_service)
	{
	}

    Media_message::~Media_message(void)
	{
	}

    int Media_message::check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number)
	{
		return RE_UNKNOWDEV;
	}

    int Media_message::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		return RE_UNKNOWDEV;
	}

    int  Media_message::parse_AC103CTR_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		if(nDataLen<6)
			return -1;
		switch(data[1])
		{
		case 0x01://设备整机状态回复
			{
				return 0;
			}
			break;
		case 0x8b:
			return 0;
		}
		return -1;
	}


    bool Media_message::IsStandardCommand()
	{
        switch (d_devInfo.nSubProtocol)
		{
		case WS2032_A:
			break;
		case AC_103_CTR:
		case FRT_X06A:
			return true;
		}
		return false;
	}
	
    void Media_message::GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit)
	{
	}

    void Media_message::GetAllCmd( CommandAttribute &cmdAll )
	{
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
                   //没找到对应告警
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
         bool bRslt = GetInst(DataBaseOperation).AddProgramSignalAlarmRecord(d_devInfo.sDevNum,frqName,curAlarm.startTime,curAlarm.nLimitId,curAlarm.nType,
                                                                             curAlarm.nAlarmId);
         if(bRslt==true){
             //发送监控量报警到客户端(用频点名称填充设备名称,cellId填充告警类型)
           m_pSession->send_alarm_state_message(GetInst(LocalConfig).local_station_id(),d_devInfo.sDevNum,frqName,curAlarm.nType
                                      ,d_devInfo.iDevType,curAlarm.nLimitId,str_time,mapProgramAlarm[frqName].size(),curAlarm.sReason);
         }l

     }

    //添加数据
    bool  Media_message::add_new_data(string sIp,DevMonitorDataPtr &mapData)
    {
        return true;
    }
}
