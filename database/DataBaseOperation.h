#ifndef DATABASEOPERATION_H
#define DATABASEOPERATION_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include "DataTypeDefine.h"
#include <iostream>
#include "../rapidxml/rapidxml.hpp"
#include <boost/thread.hpp>
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
    bool SetEnableAlarm(rapidxml::xml_node<char>* root_node,int& resValue,vector<string>& vecDevid);
    //更新监控量配置
    bool UpdateMonitorItem(string strDevnum,DeviceMonitorItem ditem);
    //批量更新监控量配置
    bool UpdateMonitorItems(string strDevnum,vector<DeviceMonitorItem> v_ditem);
    //更新监控量报警设置
    bool UpdateItemAlarmConfig(string strDevnum,int iIndex,Alarm_config alarm_config);
    bool UpdateItemAlarmConfigs(string strDevnum,map<int,Alarm_config> mapAlarmConfig);


    //添加报警记录
    bool AddItemAlarmRecord(string strDevnum,time_t startTime,int nMonitoringIndex,int alarmType,double dValue,unsigned long long& irecordid);
    bool AddItemEndAlarmRecord(time_t endTime,unsigned long long irecordid);

    //添加历史记录
    bool AddItemMonitorRecord(string strDevnum,time_t savetime,Data* pdata);
    bool SetAlarmLimit(rapidxml::xml_node<char>* root_node,int& resValue,vector<string>& vecDevid);//0上，1下
    bool SetAlarmTime(rapidxml::xml_node<char>* root_node,int& resValue,vector<string>& vecDevid);
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
    bool GetItemAlarmConfig(string strDevnum,int iIndex,vector<Alarm_config>& vAlarmconfig);
    bool GetDevInfo(string strDevnum,DeviceInfo& device);
private:
    QSqlDatabase   q_db;
    QString dsn;
    boost::mutex   db_connect_mutex_;
};
}


#endif // DATABASEOPERATION_H
