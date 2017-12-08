#ifndef BOHUI_CONST_DEFINE_H
#define BOHUI_CONST_DEFINE_H

typedef enum
{

    BH_POTO_TransmitterQuery =0,        //发射机属性查询
    BH_POTO_EnvMonDevQuery =1,          //动环属性查询
    BH_POTO_LinkDevQuery=2,             //链路属性查询

    BH_POTO_ManualPowerControl=3,       //手动发射机控制

    BH_POTO_AlarmTimeSet=4,             //告警运行图设置
    BH_POTO_AlarmSwitchSet=5,           //告警开关设置
    BH_POTO_AlarmParamSet=6,            //告警门限参数设置

    BH_POTO_AlarmSwitchSetE=7,          //动环告警开关设置
    BH_POTO_AlarmParamSetE=8,           //动环告警门限参数设置
    BH_POTO_AlarmSwitchSetD=9,          //链路告警开关设置
    BH_POTO_AlarmParamSetD=10,          //链路告警门限参数设置


    BH_POTO_CmdStatusReport=11,         //控制指令执行结果上报
    BH_POTO_QualityRealtimeReport=12,   //指标数据上报
    BH_POTO_QualityAlarmReport=13,      //告警主动上报
    BH_POTO_CommunicationReport=14,     //通讯异常上报
    BH_POTO_EnvQualityRealtimeReport=15,//动环指标上报
    BH_POTO_EnvAlarmReport =16,         //动环告警上报
    BH_POTO_LinkDevQualityReport=17,    //链路指标上报
    BH_POTO_LinkDevAlarmReport =18,     //链路告警上报


    BH_POTO_XmlParseError=19,           //xml解析出错
    BH_POTO_XmlContentError=20,         //xml内容错误

} BH_MSG_ID_DEF;

typedef enum {
    BH_NO_ERROR     =  0,
    BH_ERROR        =  1,
    BH_NO_TSMT_NODE =  2,//运行图无发射机节点

}BH_ERROR_CODE;

const char CONST_STR_BOHUI_TYPE[][32] = { "TransmitterQuery","EnvMonDevQuery","LinkDevQuery",
                                                                    "ManualPowerControl","AlarmTimeSet","AlarmSwitchSet",
                                          "AlarmParamSet","AlarmSwitchSetE","AlarmParamSetE","AlarmSwitchSetD",
                                          "AlarmParamSetD","CmdStatusReport","QualityRealtimeReport","QualityAlarmReport",
                                          "CommunicationReport","EnvQualityRealtimeReport","EnvAlarmReport","LinkDevQualityReport",
                                          "LinkDevAlarmReport","XmlParseError","XmlContentError"};



const char DEVICE_TYPE_XML_DESC[][32]={"Transmitter","EnvMonDev","DevList"};


enum {
    CMD_EXC_M_ON  =0, //手动开
    CMD_EXC_M_OFF =1, //手动关
    CMD_EXC_A_ON  =2, //自动开
    CMD_EXC_A_OFF =3, //自动关
    CMD_H_TO_B    =4, //主到备
    CMD_B_TO_H    =5  //备到主
};

enum {
    CMD_EXC_SUCCESS =0,
    CMD_EXC_FAILER  =1,
    CMD_EXC_GOING   =2
};


#endif // BOHUI_CONST_DEFINE_H
