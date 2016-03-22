#ifndef DATA_TYPE_DEFINE
#define DATA_TYPE_DEFINE
#include <string>
#include <vector>
#include <map>
using namespace std;
typedef struct
{
    int  gid;
    int  iWeek;
	tm  tStartTime;
	tm  tEndTime;
    int  iDatetype;
}Monitoring_Scheduler;
typedef struct 
{
	string sParam1;
	bool bUseP2;
	string sParam2;
}CmdParam;
typedef struct
{
	int gid;
	int iCommandType;
	int iWeek;
	tm tExecuteTime;
	int iHasParam;
	CmdParam cParam;
}Command_Scheduler;
typedef struct
{
	int iItemIndex;
	string sItemName;
	double dRatio;
	int iItemType;
	int iItemvalueType;
	bool bAlarmEnable;
	bool bUpload;
	string sUnit;
}DeviceMonitorItem;
typedef struct
{
	string property_num;
	string property_name;
	int    property_type;
	string property_value;
}DevProperty;
typedef struct
{
	int    inet_type;
	string strIp;
	int    ilocal_port;
	int    iremote_port;
	int    ilink_type;
}NetCommunicationMode;
typedef struct
{
	int    icomport;
	int    irate;
	int    idata_bit;
	int    istop_bit;
	int    iparity_bit;
}ComCommunicationMode;
typedef struct
{
	string strParamValue;
	int    iParamType;
}ActionParam;
typedef struct
{
	string strActionNum;
	string strActionNam;
	int    iActionType;
	int    iIshaveParam;
	map<int,ActionParam> map_Params;
}LinkAction;
typedef struct
{
    int iItemid;
    double fLimitvalue;
    int iAlarmlevel;
    int iLimittype;
    int iLinkageEnable;
    int iDelaytime;
    int iAlarmtype;
    string strLinkageRoleNumber;
    vector<LinkAction> vLinkAction;
}Alarm_config;
typedef struct
{
	int iWeek;
	int iUse;
	tm  tStartTime;
	tm  tEndTime;
}Record_Scheduler;
typedef struct
{
	string sPrgNum;
	string sPrgName;
	int    iPrgType;
	string sPrgFrequency;
    map<int,vector<Record_Scheduler> > map_PrgRecord_Sch;
    map<int,vector<Monitoring_Scheduler> > map_MonitorSch;
}ProgamConfig;

typedef struct
{
	string sDevNum;
	string sAstNum;//关联编号
	string sDevName;
	int    iDevType;//设备类型
	bool   bAst;//是否关联
	bool   bUsed;//是否启用
	int    iMcNum;//主类型编号
	int    iScNum;//子类型编号
	bool   bMulChannel;//是否是多通道设备
	int    iChanSize;//通道数
	int    iAddressCode;//地址码
	int    iCommunicationMode;//连接类型
	NetCommunicationMode netmode;
	ComCommunicationMode commode;
	map<int,DeviceMonitorItem> map_MonitorItem;
    map<int,vector<Monitoring_Scheduler> > map_MonitorSch;
    map<int,vector<Command_Scheduler> > map_CommSch;
	map<string,DevProperty>            map_DevProperty;
	map<int,Alarm_config>              map_AlarmConfig;
}DeviceInfo,*pDeviceInfo;

typedef struct
{
    string    sModleNumber;
    int    iCommunicationMode;//连接类型
    NetCommunicationMode netMode;
    ComCommunicationMode comMode;
    map<string,DeviceInfo>   mapDevInfo;
}ModleInfo;

#endif
