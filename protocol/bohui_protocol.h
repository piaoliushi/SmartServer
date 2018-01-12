#ifndef BOHUI_PROTOCOL_H
#define BOHUI_PROTOCOL_H
#include <string>
#include <map>
#include "./rapidxml/rapidxml.hpp"
#include "./rapidxml/rapidxml_utils.hpp"
#include "./rapidxml/rapidxml_print.hpp"
#include "../DataTypeDefine.h"
using namespace rapidxml;
using namespace std;

class Bohui_Protocol
{
public:
    Bohui_Protocol();
    //分析xml数据是否符合协议
    bool  parseDataFromStr(const string &strMsg,string &responseBody,string &srcUrl,string sIp="");
    //创建回复消息
    bool createResponseMsg(int nReplyId,int nValue,const char* nCmdType,string &responseBody);
    //创建上报数据消息
    bool createReportDataMsg(int nReplyId,string sDevId,int nDevType,DevMonitorDataPtr &curData,
                             map<int,DeviceMonitorItem> &mapMonitorItem,string &reportBody);

    //创建上报指标告警消息
    bool createReportAlarmDataMsg(int nReplyId,int nCmdType,string sDevId,CurItemAlarmInfo &alarmInfo,int nMod,
                                  string &sReason,string &reportBody);
    //创建控制结果上报消息
    bool creatExcutResultReportMsg(int nReplyId,int nCmdType,string sDevId,const string &sTime,
                                    int devState,const string &sDesc,string &reportBody);
    //创建dtmb节目信息查询消息
    bool creatQueryDtmbPrgInfoMsg(string &reportBody);

    //创建动环消息头
    bool createReportHeadMsg(xml_document<> &xmlMsg,xml_node<>* &pHeadMsg,int ReportType);

    //创建动环ups消息
    bool appendUpsReportBodyMsg(xml_document<> &xmlMsg,map<string,xml_node<>*> &mXml_Quality,string sDevId,int nDevType
                                                ,DevMonitorDataPtr &curData,map<int,DeviceMonitorItem> &mapMonitorItem);

    //创建动环消息体（合并多个动环设备）
    bool appendPowerEnvReportBodyMsg(xml_document<> &xmlMsg,map<int,xml_node<>*> &mXml_Quality,string sDevId,int nDevType
                                                     ,DevMonitorDataPtr &curData,map<int,DeviceMonitorItem> &mapMonitorItem);

    //创建发射机上报消息体（合并多个发射机设备）
    bool appendTransmitterReportBodyMsg(xml_document<> &xmlMsg,map<string,xml_node<>*> &mXml_Quality,string sDevId,DevMonitorDataPtr &curData,
                                     map<int,DeviceMonitorItem> &mapMonitorItem);

    //创建链路设备上报消息体(合并多个链路设备)
    bool appendLinkReportBodyMsg(xml_document<> &xmlMsg,map<string,xml_node<>*> &mXml_Quality,string sDevId,
                                 int nDevType,DevMonitorDataPtr &curData,
                                 map<int,DeviceMonitorItem> &mapMonitorItem);

protected:
    //创建xml头信息
    xml_node<>*  _createResponseXmlHeader(xml_document<>  &xmlMsg,int nCmdId,int nReplyId,string sDstUrl="");
    //检查xml头信息
    bool  _checkXmlHeader(xml_document<>  &xmlMsg,int &msgId,int &priority,string &desUrl,xml_node<> *rootNode);

    //查询发射机信息
    void _execTsmtQueryCmd(xml_document<> &xml_doc,xml_node<> *rootNode,int  &nValue);

    //查询动环信息
    void _execEnvMonQueryCmd(xml_document<> &xml_doc,xml_node<> *rootNode,int  &nValue);

    //查询链路设备信息
    void _execLinkDevQueryCmd(xml_document<> &xml_doc,xml_node<> *rootNode,int  &nValue);

    //查询服务器本地配置信息
    void _execGetLocalConfigQueryCmd(xml_document<> &xml_doc,xml_node<> *rootNode,int  &nValue);

    //设置告警运行图
    int _parse_alarm_run_time(xml_node<> *root_node,int &nValue,map<string,vector<Monitoring_Scheduler> > &mapMonSch);
    void _setAlarmTime(xml_node<> *rootNode,int &nValue);

    //设置告警门限
    bool _parse_alarm_param_set(xml_node<> *root_node,int &nValue,map<string,vector<Alarm_config> > &mapAlarmSet);
    void _setAlarmParam(int nDevType,xml_node<> *rootNode,int &nValue);

    //告警开关设置
    bool _parse_alarm_switch_set(xml_node<> *root_node,int &nValue,map<string,vector<Alarm_Switch_Set> > &mapAlarmSwitchSet);
    void _setAlarmSwitchSetParam(int nDevType,xml_node<> *rootNode,int &nValue);

    //设备手动控制(发射机)
    void _controlDeviceCommand(int nDevType,xml_node<> *rootNode,int &nValue);

    //从配置数据中获得查询信息
    void _query_devinfo_from_config(xml_document<> &xml_doc,int nCmdType,xml_node<> *rootNode,int  &nValue);

    //分析信号告警,数据上报
    bool _parseSignalReportMsg(string sIp,xml_node<> * InfoNode);

    //检查后添加告警使能
    void _checkAndAppendAlarmSwitch(vector<Alarm_Switch_Set> &vConfig, Alarm_Switch_Set &curConfig);

    //检查后添加告警门限(只过滤烟感水禁)
    void _checkAndAppendAlarmLimit(vector<Alarm_config> &vConfig, Alarm_config &curConfig);


public:
    static  map<int,pair<string,string> >   mapTypeToStr;
    static  string  SrcCode;
    static  string  DstCode;



};

#endif // BOHUI_PROTOCOL_H
