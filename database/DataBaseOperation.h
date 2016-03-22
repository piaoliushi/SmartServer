#ifndef DATABASEOPERATION_H
#define DATABASEOPERATION_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include "DataTypeDefine.h"
#include "../net/share_ptr_object_define.h"
#include <iostream>
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
        int timeout=100,
		std::string link_driver="QODBC",
		std::string driverName="SQL Native Client");
	bool CloseDb(); 
	bool IsOpen();
	bool ReOpen();

public:
    //获得所有设备配置
    bool GetAllDevInfo( vector<ModleInfo>& v_Linkinfo );
    //获得告警配置
    bool GetAlarmConfig( string strDevnum,map<int,Alarm_config>& map_Alarmconfig );
	//打开关闭监控量报警
	bool SetEnableMonitor(string strDevnum,int iItemIndex,bool bEnabled=true);
	//更新监控量配置
	bool UpdateMonitorItem(string strDevnum,DeviceMonitorItem ditem);
	//批量更新监控量配置
	bool UpdateMonitorItems(string strDevnum,vector<DeviceMonitorItem> v_ditem);
	//更新监控量报警设置
	bool UpdateItemAlarmConfig(string strDevnum,Alarm_config alarm_config);
	bool UpdateItemAlarmConfigs(string strDevnum,vector<Alarm_config> v_alarm_config);

    //保存监控数据
   //bool SaveDeviceMonitoringData(devDataNfyMsgPtr curData);
private:
    bool GetDevInfo( string strDevnum,DeviceInfo& device );
    bool GetDevMonitorSch(string strDevnum,map<int,vector<Monitoring_Scheduler> >& map_MonitorSch);
	bool GetCmdParam(string strCmdnum,CmdParam& param);
    bool GetCmd(string strDevnum,map<int,vector<Command_Scheduler> >& map_cmd_sch);
	bool GetDevMonItem(string strDevnum,map<int,DeviceMonitorItem>& map_item);
	bool GetDevProperty(string strDevnum,map<string,DevProperty>& map_property);
	bool GetNetProperty(string strConTypeNumber,NetCommunicationMode& nmode);
	bool GetComProperty(string strConTypeNumber,ComCommunicationMode& cmode);
	bool GetLinkActionParam(string strParamnum,map<int,ActionParam>& map_Params);
	bool GetLinkAction(string strLinkRolenum,vector<LinkAction>& vLinkAction);
private:
	QSqlDatabase   q_db;
	QString dsn;
};

#endif // DATABASEOPERATION_H
