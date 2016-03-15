#ifndef BOHUI_CONST_DEFINE_H
#define BOHUI_CONST_DEFINE_H

typedef enum
{
    BH_POTO_ManualPowerControl=0,      //手动发射机控制
    BH_POTO_TransmitterQuery=1,           //发射机属性查询
    BH_POTO_AlarmSwitchSet=2,             //告警开关设置
    BH_POTO_AlarmParamSet=3,             //告警参数设置
    BH_POTO_AlarmTimeSet=4,               //告警运行图设置
    BH_POTO_EnvMonDevQuery=5,          //动环属性查询
    BH_POTO_CmdStatusReport=6,         //控制指令执行结果上报
    BH_POTO_QualityRealtimeReport=7,   //指标数据上报
    BH_POTO_QualityAlarmReport=8,       //告警主动上报
    BH_POTO_CommunicationReport=9,   //通讯异常上报

} BH_MSG_DEF;

const char CONST_STR_BOHUI_TYPE[][32] = {"ManualPowerControl",
                                                                   "TransmitterQuery",
                                                                   "AlarmSwitchSet",
                                                                   "AlarmParamSet",
                                                                   "AlarmTimeSet",
                                                                   "EnvMonDevQuery",
                                                                   "CmdStatusReport",
                                                                   "QualityRealtimeReport",
                                                                   "QualityAlarmReport",
                                                                   "CommunicationReport"};


#endif // BOHUI_CONST_DEFINE_H
