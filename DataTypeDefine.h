#ifndef DATA_TYPE_DEFINE
#define DATA_TYPE_DEFINE
#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
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
    DEVICE_ENCODER      =110,//编码器
    DEVICE_MUX              =111,//复用器
    DEVICE_MO                =112,//调制器
    DEVICE_ANTENNA       =114,//同轴开关
    DEVICE_EXCITATION    =115,//激励器单设备
    DEVICE_POWER          =116,//功放单设备
    DEVICE_MEDIA           =117,//媒体设备

};

#define    ITEM_ANALOG   0  //模拟量
#define    ITEM_DIGITAL    1  //数字量

#define    CON_MOD_NET   1 //网口
#define    CON_MOD_COM  0 //串口

#define    NET_MOD_TCP     0 //TCP
#define    NET_MOD_UDP    1 //UDP
#define    NET_MOD_SNMP  2//SNMP
#define    NET_MOD_HTTP   3// HTTP
#define    NET_MOD_COM   4//COM

#define    RUN_TIME_DAY         2 //运行图-天
#define    RUN_TIME_WEEK      0 //运行图-星期
#define    RUN_TIME_MONTH    1 //运行图-月


enum _tag_CommandType{
    DEV_TASK_TURNON  = 0,
    DEV_TASK_TURNOFF = 1,
};
//运行图结构
typedef struct _tag_M_Sch
{
    _tag_M_Sch(){
        iMonitorType=-1;
        iMonitorWeek=-1;
        iMonitorMonth=-1;
        iMonitorDay=-1;

    }

    int      iMonitorType;    //0=星期，1=月，2=天
    bool    bMonitorFlag;    //flase-不监测，true-监测
    int      iMonitorWeek;   //星期(1-7）
    int      iMonitorMonth;  //月（0-12，0=all）
    int      iMonitorDay;      //日(1-31)
    time_t      tStartTime;        //开始时间
    time_t      tEndTime;          //结束时间
    time_t      tAlarmEndTime;     //月运行计划终止日期
}Monitoring_Scheduler;

typedef struct
{
    string sParam1;
    bool   bUseP2;
    string sParam2;
}CmdParam;

typedef struct
{
    int      gid;
    int      iCommandType; //命令类型
    int      iDateType;         //时间类型
    int      iWeek;              //星期(1-7)
    int      iMonitorMonth;  //月（0-12，0=all）
    int      iMonitorDay;      //日(1-31)
    time_t  tCmdEndTime;  //月运行计划终止日期
    time_t  tExecuteTime;
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
    int    icomport; //com口ID
    int    irate;       //波特率
    int    idata_bit; //数据位
    int    istop_bit; //停止位
    int    iparity_bit;//校验位
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
#define    ALARM_DEVICE           5
#define    ALARM_RESUME         6

const char CONST_STR_ALARM_CONTENT[][32] = {"跃上限","跃下限","跃上上限","跃下下限","报警","恢复正常"};
typedef struct
{
    int iAlarmid;            //告警类型id
    double fLimitvalue;//门限值
    int iAlarmlevel;      //告警等级
    int iLimittype;        //0:上限,1:下线,2:上上限,3:下下限,4:状态量
    int iLinkageEnable;//联动标志
    int iDelaytime;       //告警延迟(秒)
    int iResumetime;   //恢复延迟(秒)
    int iAlarmtype;       //0:监控量,1:设备
    string strLinkageRoleNumber;//联动角色
    //bool  bIsAlarm;
    vector<LinkAction> vLinkAction;//联动动作
}Alarm_config;

typedef struct
{
    int iAlarmid;//告警类型id
    int iSwtich;
    string sDes;
}Alarm_Switch_Set;


typedef struct
{
    int iWeek;
    int iUse;
    time_t  tStartTime;
    time_t  tEndTime;
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
    int  iTargetId;
    int  iModTypeId;
    int iModDevId;
    vector<Alarm_config>  vItemAlarm;
}DeviceMonitorItem;

typedef struct
{
    string sAstNum;//关联设备编号
    int    iChannel;//关联设备通道
}AssDevChan;

typedef struct
{
    string sDevNum;
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
    map<int,vector<Monitoring_Scheduler> >      vMonitorSch;  //监控计划
    vector<Command_Scheduler>           vCommSch;   //控制计划
    map<string,DevProperty>                  map_DevProperty;//设备属性列表
    map<int,Alarm_config >                    map_AlarmConfig;//设备告警配置
    map<int,vector<AssDevChan> >       map_AssDevChan;//设备关联设置
}DeviceInfo,*pDeviceInfo;


typedef struct
{
    string sStationNumber;//台站编号
    string sDevNum;       //设备编号
    string sDevName;      //设备名称
    int    nDevType;        //设备类型
    int    nConnectType;  //连接类型
    int    nCommType;     //通信方式
    map<int,DeviceMonitorItem> mapMonitorItem;//对应的监控项信息
}DevBaseInfo;


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
    time_t tMonitoringTime;
    double dMonitoringValue;
}MonitorItemRecord;

typedef struct
{
    unsigned long long  nAlarmId;   //告警id
    int           nType;      //告警类型
    time_t     startTime; //开始时间
    int           nTargetId; //指标id
    int           nModuleId;//模块id
    int           nModuleType;//模块type
    int           nLimitId;//跃限类型,跃上限...
    string      sReason;//告警描述
    bool        bNotifyed;//已通知标志
}CurItemAlarmInfo;

typedef struct DATA_INFO_TAG
{
    bool       bType;//数据类型: false表示模拟量，true表示状态量
    float       fValue;//当bType为true表示状态量时，dValue == 1.0，界面亮绿灯，反之红灯。
    string     sValue;
    //bool       bUpdate;//是否已经更新（适应多指令分数据段解析）
}DataInfo,*pDataInfo;

typedef struct
{
     map<int,DataInfo> mValues;
}Data,pData;
typedef boost::shared_ptr<Data> DevMonitorDataPtr;


//用户信息
typedef struct
{
    string sStationNumber;
    string sNumber;
    string sName;
    string sPassword;
    string sGender;
    string sHeadship;
    string sJobNumber;
    string sTelephone;
    string sEducation;
    string sEmail;
    short  nControlLevel;
    string sNote;
} UserInformation;

enum _tagAssType
{
    TSMT_TO_ATTENA = 0,//发射机关联天线
    DEV_TO_CHANNEL = 1,//设备关联通道
    DEV_TO_MOUDLE  = 2,//设备关联模块
    ATTENA_TO_TSMT = 3,//天线关联发射机
    CHNNEL_TO_DEV  = 4,//通道关联设备
    MOUDLE_TO_DEV = 5,//模块关联设备
};

#endif
