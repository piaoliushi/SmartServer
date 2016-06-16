#ifndef DATABASEOPERATION_H
#define DATABASEOPERATION_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include "DataTypeDefine.h"
#include <iostream>
//#include "../rapidxml/rapidxml.hpp"
#include "../qnotifyhandler.h"
#include <boost/thread.hpp>
//using namespace rapidxml;
namespace db {
class DataBaseOperation
{
public:
    DataBaseOperation();
    ~DataBaseOperation();

public:
    //设置数据库连接状态通知回调
    void set_notify(QNotifyHandler* pNotify){d_cur_Notify = pNotify;}

    //打开数据库
    bool OpenDb(const std::string& serveraddress,const std::string& database,const std::string& uid,
        const std::string& pwd,int timeout=0,std::string link_driver="QODBC",std::string driverName="SQL Native Client");

    //关闭数据库
    bool CloseDb();

    //数据库是否打开
    bool IsOpen();

    //重连数据库
    bool ReOpen();

    //检查数据库活动状态(读取数据库时间)
    bool check_database();

    //获得所有设备信息
    bool GetAllDevInfo(vector<ModleInfo>& v_Linkinfo);

    //获得数据字典映射表(不包含设备类型字段)
    bool GetDataDictionary(map<int,pair<string,string> >& mapDicry);

    //打开关闭监控量报警
    bool SetEnableMonitor(string strDevnum,int iItemIndex,bool bEnabled=true);

    //设置告警使能
    bool SetEnableAlarm(map<string,vector<Alarm_Switch_Set> > &mapAlarmSwitchSet,int& resValue);

    //更新监控量配置
    bool UpdateMonitorItem(string strDevnum,DeviceMonitorItem ditem);

    //批量更新监控量配置
    bool UpdateMonitorItems(string strDevnum,vector<DeviceMonitorItem> v_ditem);

    //更新监控量报警设置
    bool UpdateItemAlarmConfig(string strDevnum,int iIndex,Alarm_config alarm_config);

    //更新批量监控量告警设置
    bool UpdateItemAlarmConfigs(string strDevnum,map<int,Alarm_config> mapAlarmConfig);

    //添加报警记录
    bool AddItemAlarmRecord( string strDevnum,time_t startTime,int nMonitoringIndex,int nlimitType,int nalarmTypeId,double dValue,
                                                const string &sreason,unsigned long long& irecordid );
    //更新告警结束记录
    bool AddItemEndAlarmRecord(time_t endTime,unsigned long long irecordid);

    //添加历史记录
    bool AddItemMonitorRecord(string strDevnum,time_t savetime,DevMonitorDataPtr pdata,const map<int,DeviceMonitorItem> &mapMonitorItem);

    //设置告警门限
    bool SetAlarmLimit(map<string,vector<Alarm_config> > &mapAlarmSet,int& resValue);

    //设置运行图
    bool SetAlarmTime(map<string,vector<Monitoring_Scheduler> > &mapSch,int& resValue);

    //获得用户信息
    bool GetUserInfo( const string sName,UserInformation &user );

    //获得用户授权设备
    bool GetAllAuthorizeDevByUser( const string sUserId,vector<string> &vDevice );

    //获得更新设备的运行图信息(用于动态配置回读)
    bool GetUpdateDevTimeScheduleInfo( string strDevnum,map<int,vector<Monitoring_Scheduler> >& monitorScheduler,
                                                  vector<Command_Scheduler> &cmmdScheduler  );
    //获得更新设备告警信息(用于动态配置回读)
    bool GetUpdateDevAlarmInfo( string strDevnum,DeviceInfo& device );

    //节目信号告警/恢复
    bool AddProgramSignalAlarmRecord(string strDevNum, string strFrqName,time_t startTime,int nlimitType,
                                      int nalarmTypeId,unsigned long long& irecordid );

protected:
    void StartReOpen();
    bool GetDevMonitorSch(QSqlDatabase &db,string strDevnum,map<int,vector<Monitoring_Scheduler> >& mapMonitorSch);
    bool GetCmdParam(QSqlDatabase &db,string strCmdnum,CmdParam& param);
    bool GetCmd(QSqlDatabase &db,string strDevnum,vector<Command_Scheduler>& vcmdsch);
    bool GetDevMonItem(QSqlDatabase &db,string strDevnum,QString qsPrtocolNum,map<int,DeviceMonitorItem>& map_item);
    bool GetDevProperty(QSqlDatabase &db,string strDevnum,map<string,DevProperty>& map_property);
    bool GetNetProperty(QSqlDatabase &db,string strConTypeNumber,NetCommunicationMode& nmode);
    bool GetComProperty(QSqlDatabase &db,string strConTypeNumber,ComCommunicationMode& cmode);
    bool GetLinkActionParam(QSqlDatabase &db,string strParamnum,map<int,ActionParam>& map_Params);
    bool GetLinkAction(QSqlDatabase &db,string strLinkRolenum,vector<LinkAction>& vLinkAction);
    bool GetAlarmConfig(QSqlDatabase &db,string strDevnum,map<int,Alarm_config>& map_Alarmconfig);
    bool GetItemAlarmConfig(QSqlDatabase &db,string strDevnum,int iIndex,vector<Alarm_config>& vAlarmconfig);
    bool GetDevInfo(QSqlDatabase &db,string strDevnum,DeviceInfo& device);
    bool GetAssDevChan(QSqlDatabase &db, QString strDevNum,map<int,vector<AssDevChan> >& mapAssDev );
private:
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
