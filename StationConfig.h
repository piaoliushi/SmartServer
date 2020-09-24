#ifndef STATION_CONFIG
#define STATION_CONFIG
#pragma once

#include "./database/DataBaseOperation.h"
#include "DataType.h"
#include "DataTypeDefine.h"
#include <map>
using namespace std;

#define GET_CONST_STR(X,Y)  GetInst(StationConfig).get_dictionary_value(X,Y)
#define CONST_STR_RESPONSE_VALUE_DESC(Y)  GetInst(StationConfig).get_dictionary_value("s_rsps_result_desc",Y)
#define DEV_CMD_OPR_DESC(Y)    GetInst(StationConfig).get_dictionary_value("s_cmd_opr_desc",Y)
#define DEV_CMD_OPR_MODE(Y)    GetInst(StationConfig).get_dictionary_value("s_cmd_excute_mode",Y)
#define DEV_CMD_RESULT_DESC(Y)      GetInst(StationConfig).get_dictionary_value("s_cmd_result_desc",Y)
#define CONST_STR_ALARM_CONTENT(Y)  GetInst(StationConfig).get_dictionary_value("s_alarm_event",Y)
#define GLOBAL_STR(Y)  GetInst(StationConfig).get_dictionary_value("s_base",Y)
#define GET_POWER_STR(Y)   GetInst(StationConfig).get_dictionary_value("s_power_base",Y)

#define DEV_CMD_OPR_TYPE(Y)    GetInst(StationConfig).get_dictionary_value("CommandType",Y)

#define REMIND_ORIGINATOR_TYPE(Y)    GetInst(StationConfig).get_dictionary_value("remind_notify_originator",Y)
#define REMIND_TARGET_TYPE(Y)    GetInst(StationConfig).get_dictionary_value("remind_notify_target",Y)
#define REMIND_TYPE(Y)    GetInst(StationConfig).get_dictionary_value("remind_notify_type",Y)
#define REMIND_STATE(Y)    GetInst(StationConfig).get_dictionary_value("remind_notify_state",Y)

class StationConfig
{
public:
	StationConfig(void);
	~StationConfig(void);
public:
	 //加载某台站配置信息
	 bool load_station_config();
     //获得关联设备信息
     DeviceInfo * get_devinfo_by_id(string sDevId);
	 //所有协议转换器信息
     vector<ModleInfo>& get_Modle();
	 //获得该等级的用户电话号码
	 vector<SendMSInfo>& get_sms_user_info();

	 //查找other设备对应的串口服务器id
	 string get_modle_id_by_devid(string sStationId,string sDevId);
     //获得数据字典值
     string get_dictionary_value(string sType,int nCode);
     //是否有gsm
     bool IsHaveGsm(){return bHave_GSM_model;}
     //获取gms网络配置信息
     ComCommunicationMode getGsmInfo(){return Gsm_model_info;}
     //获取设备属性
     bool get_dev_propery(string sDevId,string sName,string &sValue);
     //获取提醒配置信息
     map<string,Remind_Scheduler>&  get_all_remind_info();
     //获取单个提醒配置信息
     Remind_Scheduler*  get_device_remind_info(string sNumber);



private:

    vector<ModleInfo> mapModleInfo;
	vector<SendMSInfo> vSmsInfo;

	string cur_station_id;//当前台站id
	string cur_devSvc_id; //当前服务id

    map<string,map<int,string> > mapDeviceDicry;

    bool                 bHave_GSM_model;
    ComCommunicationMode Gsm_model_info;

    map<string,Remind_Scheduler>  mapRemindInfo;//本地服务负责的提醒任务
    map<string,Remind_Scheduler>  mapDeviceCmdRemindInfo;//设备控制提醒任务
};
#endif
