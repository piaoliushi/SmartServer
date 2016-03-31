#ifndef DATA_TYPE_DEFINE
#define DATA_TYPE_DEFINE
#include <string>
#include <vector>
#include <map>
using namespace std;


enum DEVType
{
    DEVICE_TRANSMITTER = 0,//发射机
    DEVICE_ELEC              = 1,  //电力仪
    DEVICE_TEMP             = 2,   //温度计
    DEVICE_SMOKE          = 3,   //烟雾
    DEVICE_WATER           = 4, //水禁
    DEVICE_AIR                = 5,//空调
    DEVICE_GPS               = 6,//gps

    DEVICE_SWITCH         = 7,//切换设备
    DEVICE_GPS_TIME      = 8,  //GPS授时器

    DEVICE_GS_RECIVE    =100,//卫星接收机
    DEVICE_MW               =101,//微波接收机
    DEVICE_TR                 =102,//光收发器

    DEVICE_MUX              =111,//复用器
    DEVICE_MO                =112,//调制器
    DEVICE_ANTENNA       =114,//同轴开关

};

#define    ITEM_ANALOG   0  //模拟量
#define    ITEM_DIGITAL    1  //数字量

#define    CON_MOD_NET   1 //网口
#define    CON_MOD_COM  0 //串口

#define    NET_MOD_TCP   0
#define    NET_MOD_UDP   1

//运行图结构
typedef struct
{
    int      iMonitorType;    //0=星期，1=月，2=天
    bool    bMonitorFlag;    //flase-不监测，true-监测
    int      iMonitorWeek;   //星期(1-7）
    int      iMonitorMonth;  //月（0-12，0=all）
    int      iMonitorDay;      //日(1-31)
    tm      tStartTime;        //开始时间
    tm      tEndTime;          //结束时间
    tm      tAlarmEndTime;     //月运行计划终止日期
}Monitoring_Scheduler;

typedef struct
{
    string sParam1;
    bool   bUseP2;
    string sParam2;
}CmdParam;

typedef struct
{
    int gid;
    int iCommandType;
    int iWeek;
    int      iMonitorMonth;  //月（0-12，0=all）
    int      iMonitorDay;      //日(1-31)
    tm       tCmdEndTime;     //月运行计划终止日期
    tm tExecuteTime;
    int iHasParam;
    CmdParam cParam;
}Command_Scheduler;

typedef struct
{
    string property_num;
    string property_name;
    int     property_type;
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

#define    ALARM_UPPER            0
#define    ALARM_LOWER           1
#define    ALARM_UP_UPPER      2
#define    ALARM_LOW_LOWER   3
#define    ALARM_SWITCH          4


typedef struct
{
    int iItemid;            //告警id
    double fLimitvalue;//门限值
    int iAlarmlevel;      //告警等级
    int iLimittype;        //0:上限,1:下线,2:上上限,3:下下限,4:状态量
    int iLinkageEnable;//联动标志
    int iDelaytime;       //告警延迟(秒)
    int iResumetime;   //恢复延迟(秒)
    int iAlarmtype;       //0:监控量,1:设备
    string strLinkageRoleNumber;//联动角色
    bool  bIsAlarm;
    vector<LinkAction> vLinkAction;//联动动作
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
    int iItemIndex;
    string sItemName;
    double dRatio;
    int iItemType;//0:模拟量,1:状态量
    int iItemvalueType;//值类型
    bool bAlarmEnable;
    bool bUpload;
    string sUnit;//单位
    vector<Alarm_config>  vItemAlarm;
}DeviceMonitorItem;

typedef struct
{
    string sDevNum;
    string sAstNum;//关联编号
    string sDevName;
    int     iDevType;//设备类型（0：发射机,1:天线，2：环境，3：电力，4：链路设备）
    bool   bAst;//是否关联
    bool   bUsed;//是否启用
    int     nDevProtocol; //设备协议号
    int     nSubProtocol; //设备子协议号
    bool   bMulChannel;//是否是多通道设备
    int      iChanSize;//通道数
    int      iAddressCode;//地址码
    map<int,DeviceMonitorItem>        map_MonitorItem;//设备监控量
    vector<Monitoring_Scheduler>      vMonitorSch;  //监控计划
    vector<Command_Scheduler>       vCommSch;   //控制计划
    map<string,DevProperty>                  map_DevProperty;//设备属性列表
    map<int,Alarm_config >                    map_AlarmConfig;//设备告警配置
}DeviceInfo,*pDeviceInfo;

typedef struct
{
    string    sModleNumber;
    int    iCommunicationMode;//连接类型0:网口,1:串口
    NetCommunicationMode   netMode;
    ComCommunicationMode comMode;
    map<string,DeviceInfo>    mapDevInfo;
}ModleInfo;

typedef struct
{
    int    iItemindex;
    tm     tStarttime;
    tm     tEndtime;
    int    iAlarmtype;
    double dValue;
}DevAlarmRecord;
typedef struct
{
    tm tMonitoringTime;
    double dMonitoringValue;
}MonitorItemRecord;

typedef struct
{
    unsigned long long  nAlarmId;   //告警id
    int           nType;      //告警类型
    time_t     startTime; //开始时间
    int           nDuring;    //持续时间
}CurItemAlarmInfo;

typedef struct DATA_INFO_TAG
{
    DATA_INFO_TAG()
    {
        bUpdate=true;
    }
    bool       bType;//数据类型: false表示模拟量，true表示状态量
    float       fValue;//当bType为true表示状态量时，dValue == 1.0，界面亮绿灯，反之红灯。
    bool       bUpdate;//是否已经更新（适应多指令分数据段解析）
}DataInfo,*pDataInfo;

typedef struct
{
     map<int,DataInfo> mValues;
}Data,pData;


#endif
