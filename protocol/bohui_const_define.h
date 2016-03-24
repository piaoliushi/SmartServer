#ifndef BOHUI_CONST_DEFINE_H
#define BOHUI_CONST_DEFINE_H

typedef enum
{

    BH_POTO_TransmitterQuery =0,           //发射机属性查询
    BH_POTO_EnvMonDevQuery =1,          //动环属性查询
    BH_POTO_LinkDevQuery=2,                //链路属性查询

     BH_POTO_ManualPowerControl=3,     //手动发射机控制

    BH_POTO_AlarmTimeSet=4,               //告警运行图设置
    BH_POTO_AlarmSwitchSet=5,             //告警开关设置
    BH_POTO_AlarmParamSet=6,             //告警门限参数设置

    BH_POTO_AlarmSwitchSetE=7,           //动环告警开关设置
    BH_POTO_AlarmParamSetE=8,           //动环告警门限参数设置
    BH_POTO_AlarmSwitchSetD=9,           //链路告警开关设置
    BH_POTO_AlarmParamSetD=10,         //链路告警门限参数设置


    BH_POTO_CmdStatusReport=11,         //控制指令执行结果上报
    BH_POTO_QualityRealtimeReport=12,   //指标数据上报
    BH_POTO_QualityAlarmReport=13,       //告警主动上报
    BH_POTO_CommunicationReport=14,   //通讯异常上报
    BH_POTO_EnvQualityRealtimeReport=15,//动环指标上报
    BH_POTO_EnvAlarmReport =16,             //动环告警上报
    BH_POTO_LinkDevQualityReport=17,      //链路指标上报
    BH_POTO_LinkDevAlarmReport =18,       //链路告警上报


    BH_POTO_XmlParseError=19,//xml解析出错
    BH_POTO_XmlContentError=20,//xml内容错误

} BH_MSG_ID_DEF;

const char CONST_STR_BOHUI_TYPE[][32] = { "TransmitterQuery","EnvMonDevQuery","LinkDevQuery",
                                                                    "ManualPowerControl","AlarmTimeSet","AlarmSwitchSet",
                                          "AlarmParamSet","AlarmSwitchSetE","AlarmParamSetE","AlarmSwitchSetD",
                                          "AlarmParamSetD","CmdStatusReport","QualityRealtimeReport","QualityAlarmReport",
                                          "CommunicationReport","EnvQualityRealtimeReport","EnvAlarmReport","LinkDevQualityReport",
                                          "LinkDevAlarmReport","XmlParseError","XmlContentError"};

const char CONST_STR_RESPONSE_VALUE_DESC[][64] = { "成功","删除的任务不存在","没有权限",
                                                                    "内部错误","省监管平台未找到","用户名密码错",
                                          "资源不足","任务冲突"," "," ","XML解析错误","XML内容错误",
                                          "任务执行异常","没有检索到数据"};

const char DEVICE_TYPE_XML_DESC[][32]={"Transmitter","EnvMonDev","DevList"};



#endif // BOHUI_CONST_DEFINE_H
