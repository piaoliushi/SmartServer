//#include "CommonPrecomp.h"
#include "RequestHandlerFactory.h"
#include "RequestHandler.h"
#include "../../../database/DataBaseOperation.h"
#include "../../config.h"
#include "../../share_ptr_object_define.h"
#include "../../SvcMgr.h"
using namespace db;
using namespace hx_net;
request_handler_factory::~request_handler_factory()
{
}

request_handler_ptr request_handler_factory::create()
{
    boost::recursive_mutex::scoped_lock llock(mutex_);
    request_handler_ptr request_handler_(new request_handler);
    request_handler_lst.push_back(request_handler_);
	return request_handler_;
}

void request_handler_factory::destroy(request_handler_ptr handler)
{
    boost::recursive_mutex::scoped_lock llock(mutex_);
    for (std::list<request_handler_ptr>::iterator iter = request_handler_lst.begin();
        iter != request_handler_lst.end(); iter++){
        if (*iter== handler)
        {
            request_handler_lst.erase(iter);
            return;
        }
    }
}

bool request_handler_factory::check_ip(string sIp,string &sDevId)
{
    return true;
}

 bool  request_handler_factory::add_new_alarm(string sIp,string sPrgName,int alarmId,int nState,time_t  startTime)
 {
     //检查sIp是否合法
     string sDevId="00000000";
     if(check_ip(sIp,sDevId) == false)
         return false;
     //告警产生
     boost::recursive_mutex::scoped_lock lock(alarm_mutex_);
     if(nState == 0){
            CurItemAlarmInfo  tmp_alarm_info;
            map<string ,map<int,CurItemAlarmInfo > >::iterator iter= mapProgramAlarm[sDevId].find(sPrgName);
            if(iter!=mapProgramAlarm[sDevId].end()){
                map<int,CurItemAlarmInfo >::iterator iter_type = iter->second.find(alarmId);
                //没找到对应告警
                if(iter_type==iter->second.end()){
                    tmp_alarm_info.startTime = startTime;//记录时间
                    tmp_alarm_info.nType = alarmId;//告警类型
                    tmp_alarm_info.nLimitId = ALARM_SWITCH;//限值类型
                    tmp_alarm_info.bNotifyed = false;
                    iter->second[alarmId] = tmp_alarm_info;
                     record_alarm_and_notify(sPrgName,0,mapProgramAlarm[sDevId][sPrgName][alarmId]);
                }
            }else{
                //没有找到增加该告警监控项
                tmp_alarm_info.startTime = startTime;//记录时间
                tmp_alarm_info.nType = alarmId;//告警类型
                tmp_alarm_info.nLimitId = ALARM_SWITCH;//限值类型
                tmp_alarm_info.bNotifyed = false;
                map<int,CurItemAlarmInfo>  tmTypeAlarm;
                tmTypeAlarm[alarmId] = tmp_alarm_info;
                mapProgramAlarm[sDevId][sPrgName] = tmTypeAlarm;
                record_alarm_and_notify(sPrgName,0,mapProgramAlarm[sDevId][sPrgName][alarmId]);
            }

     }else if(nState==1){
         map<string ,map<int,CurItemAlarmInfo > >::iterator iter= mapProgramAlarm[sDevId].find(sPrgName);
         if(iter!=mapProgramAlarm[sDevId].end()){
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

 void  request_handler_factory::record_alarm_and_notify(string &prgName,int nMod,CurItemAlarmInfo &curAlarm)
 {
     static  char str_time[64];
     tm *local_time = localtime(&curAlarm.startTime);
     strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", local_time);
     bool bRslt;// = GetInst(DataBaseOperation).AddProgramSignalAlarmRecord(prgName,curAlarm.startTime,curAlarm.nLimitId,curAlarm.nType,
                        //                                                 curAlarm.nAlarmId);
     if(bRslt==true){
         //发送监控量报警到客户端
       //  send_alarm_state_message(GetInst(LocalConfig).local_station_id(),prgName," ",0,
        //                          ,modleInfos_.mapDevInfo[devId].iDevType,curAlarm.nLimitId,str_time,mapItemAlarm[devId][ItemInfo.iItemIndex].size(),curAlarm.sReason);
     }

 }

 void request_handler_factory::send_alarm_state_message(string sStationid,string sDevid,string sFrqName,
                                        int nType,int devType,int  alarmState,
                                        string sStartTime,int alarmCount,string sReason)
 {
     devAlarmNfyMsgPtr dev_alarm_nfy_ptr(new DevAlarmStatusNotify);
     DevAlarmStatus *dev_n_s = dev_alarm_nfy_ptr->add_cdevcuralarmstatus();
     dev_n_s->set_sstationid(sStationid);
     dev_n_s->set_edevtype(devType);
     dev_n_s->set_sdevid(sDevid);
     dev_n_s->set_sdevname(sFrqName);
     dev_n_s->set_nalarmcount(alarmCount);
     dev_n_s->set_nalarmmod(MOD_CELL);//该标志指示时量值告警还是其他告警（整机）
     DevAlarmStatus_eCellAlarmMsg *dev_cell_alarm = dev_n_s->add_ccellalarm();
     //std::string scellid = str(boost::format("%1%")%nCellId);
     dev_cell_alarm->set_scellid(nType);
     dev_cell_alarm->set_sstarttime(sStartTime);
     dev_cell_alarm->set_ccellstatus((e_AlarmStatus)alarmState);
     dev_cell_alarm->set_sdesp(sReason);

     GetInst(SvcMgr).send_dev_alarm_state_to_client(sStationid,sDevid,dev_alarm_nfy_ptr);
 }
