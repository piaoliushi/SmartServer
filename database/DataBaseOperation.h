#ifndef DATABASEOPERATION_H
#define DATABASEOPERATION_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include "DataTypeDefine.h"
#include <iostream>
#include "../rapidxml/rapidxml.hpp"

using namespace rapidxml;
namespace db {
class DataBaseOperation
{
public:
    DataBaseOperation();
    ~DataBaseOperation();

public:
    bool OpenDb(const std::string& serveraddress,
        const std::string& database,
        const std::string& uid,
        const std::string& pwd,
        int timeout=0,
        std::string link_driver="QODBC",
        std::string driverName="SQL Native Client");
    bool CloseDb();
    bool IsOpen();
    bool ReOpen();

public:
    bool GetAllDevInfo(vector<ModleInfo>& v_Linkinfo);
    //打开关闭监控量报警
    bool SetEnableMonitor(string strDevnum,int iItemIndex,bool bEnabled=true);
    //bool SetEnableAlarm(xml_node<char>* root_node);
    //更新监控量配置
    bool UpdateMonitorItem(string strDevnum,DeviceMonitorItem ditem);
    //批量更新监控量配置
    bool UpdateMonitorItems(string strDevnum,vector<DeviceMonitorItem> v_ditem);
    //更新监控量报警设置
    bool UpdateItemAlarmConfig(string strDevnum,Alarm_config alarm_config);
    bool UpdateItemAlarmConfigs(string strDevnum,vector<Alarm_config> v_alarm_config);


    //添加报警记录
    bool AddItemAlarmRecord(string strDevnum,DevAlarmRecord alrecord);
    bool AddItemEndAlarmRecord(string strDevnum,DevAlarmRecord alrecord);

    //添加历史记录
    bool AddItemMonitorRecord(string strDevnum,map<int,MonitorItemRecord> mapRecord);
    //报警开关设置
    bool SetEnableAlarm(int nDevType, rapidxml::xml_node<char>* root_node,int& resValue,vector<string> &vecDevId);
    //报警参数设置
    bool SetAlarmLimit(int nDevType, rapidxml::xml_node<char>* root_node,int& resValue,vector<string>& vecDevid );
private:

    bool GetDevMonitorSch(string strDevnum,vector<Monitoring_Scheduler>& vMonitorSch);
    bool GetCmdParam(string strCmdnum,CmdParam& param);
    bool GetCmd(string strDevnum,vector<Command_Scheduler>& vcmdsch);
    bool GetDevMonItem(string strDevnum,map<int,DeviceMonitorItem>& map_item);
    bool GetDevProperty(string strDevnum,map<string,DevProperty>& map_property);
    bool GetNetProperty(string strConTypeNumber,NetCommunicationMode& nmode);
    bool GetComProperty(string strConTypeNumber,ComCommunicationMode& cmode);
    bool GetLinkActionParam(string strParamnum,map<int,ActionParam>& map_Params);
    bool GetLinkAction(string strLinkRolenum,vector<LinkAction>& vLinkAction);
    bool GetAlarmConfig(string strDevnum,map<int,Alarm_config>& map_Alarmconfig);
    bool GetDevInfo(string strDevnum,DeviceInfo& device);
private:
    QSqlDatabase   q_db;
    QString dsn;
};
}


#endif // DATABASEOPERATION_H
