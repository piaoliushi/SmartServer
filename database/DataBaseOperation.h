#ifndef DATABASEOPERATION_H
#define DATABASEOPERATION_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include "DataTypeDefine.h"
#include <iostream>
#include "../rapidxml/rapidxml.hpp"
#include "../qnotifyhandler.h"
#include <boost/thread.hpp>
using namespace rapidxml;
namespace db {
class DataBaseOperation
{
public:
    DataBaseOperation();
    ~DataBaseOperation();

public:
    void set_notify(QNotifyHandler* pNotify){d_cur_Notify = pNotify;}
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
    bool check_database();

public:
    //获得所有设备信息
    bool GetAllDevInfo(vector<ModleInfo>& v_Linkinfo);
    //获得数据字典映射表(不包含设备类型字段)
    bool GetDataDictionary(map<int,pair<string,string> >& mapDicry);
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
    bool AddItemAlarmRecord( string strDevnum,time_t startTime,int nMonitoringIndex,int nlimitType,int nalarmTypeId,double dValue,
                                                const string &sreason,unsigned long long& irecordid );
    bool AddItemEndAlarmRecord(time_t endTime,unsigned long long irecordid);

    //添加历史记录
    bool AddItemMonitorRecord(string strDevnum,time_t savetime,DevMonitorDataPtr pdata);
    bool SetAlarmLimit(rapidxml::xml_node<char>* root_node,int& resValue,vector<string>& vecDevid);//0上，1下
    bool SetAlarmTime(rapidxml::xml_node<char>* root_node,int& resValue,vector<string>& vecDevid);

    bool GetUserInfo( const string sName,UserInformation &user );
    bool GetAllAuthorizeDevByUser( const string sUserId,vector<string> &vDevice );
private:
    void StartReOpen();//启动重连线程
    bool GetDevMonitorSch(string strDevnum,map<int,vector<Monitoring_Scheduler> >& mapMonitorSch);
    bool GetCmdParam(string strCmdnum,CmdParam& param);
    bool GetCmd(string strDevnum,vector<Command_Scheduler>& vcmdsch);
    bool GetDevMonItem(string strDevnum,QString qsPrtocolNum,map<int,DeviceMonitorItem>& map_item);
    bool GetDevProperty(string strDevnum,map<string,DevProperty>& map_property);
    bool GetNetProperty(string strConTypeNumber,NetCommunicationMode& nmode);
    bool GetComProperty(string strConTypeNumber,ComCommunicationMode& cmode);
    bool GetLinkActionParam(string strParamnum,map<int,ActionParam>& map_Params);
    bool GetLinkAction(string strLinkRolenum,vector<LinkAction>& vLinkAction);
    bool GetAlarmConfig(string strDevnum,map<int,Alarm_config>& map_Alarmconfig);
    bool GetItemAlarmConfig(string strDevnum,int iIndex,vector<Alarm_config>& vAlarmconfig);
    bool GetDevInfo(string strDevnum,DeviceInfo& device);
    bool GetAssDevChan( QString strDevNum,map<int,vector<AssDevChan> >& mapAssDev );
private:
    QSqlDatabase   q_db;
    QString d_serveraddress;
    QString d_database;
    QString d_uid;
    QString d_pwd;
    boost::recursive_mutex   db_connect_mutex_;
    boost::shared_ptr<boost::thread> reconnect_thread_;//网络监听线程
    QDateTime d_db_check_time;
    QNotifyHandler  *d_cur_Notify;
};
}


#endif // DATABASEOPERATION_H
