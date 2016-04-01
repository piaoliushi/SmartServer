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
    bool  parseDataFromStr(string &strMsg,string &responseBody);
    //创建回复消息
    bool createResponseMsg(int nReplyId,int nValue,const char* nCmdType,string &responseBody);
    //创建上报数据消息
    bool createReportDataMsg(int nReplyId,int nCmdType,string sDevId,int nDevType,DevMonitorDataPtr curData,
                             map<int,DeviceMonitorItem> &mapMonitorItem,string &reportBody);
    //创建上报告警消息
    bool createReportAlarmDataMsg(int nReplyId,int nCmdType,string sDevId,CurItemAlarmInfo &alarmInfo,int nMod,
                                  string &sReason,string &reportBody);
protected:
    bool _parse_ManualPowerControl_xml(xml_document<> &xmlMsg,int &nMsgId,string &responseBody);
    //创建xml头信息
    xml_node<>*  _createResponseXmlHeader(xml_document<>  &xmlMsg,int nCmdId,int nReplyId);
    //检查xml头信息
    bool  _checkXmlHeader(xml_document<>  &xmlMsg,int &msgId,int &priority,xml_node<> *rootNode);
    //根据类型创建xml
    bool  _createResponseXmlBody(string &sXmlBody,int nType);

    //查询发射机信息
    void _execTsmtQueryCmd(xml_document<> &xml_doc,xml_node<> *rootNode,int  &nValue);

    //查询动环信息
    void _execEnvMonQueryCmd(xml_document<> &xml_doc,xml_node<> *rootNode,int  &nValue);

    //查询链路设备信息
    void _execLinkDevQueryCmd(xml_document<> &xml_doc,xml_node<> *rootNode,int  &nValue);

    //设置告警运行图
    void _setAlarmTime(xml_node<> *rootNode,int &nValue);

    //设置告警门限
    void _setAlarmParam(int nDevType,xml_node<> *rootNode,int &nValue);

    //告警开关设置
    void _setAlarmSwitchSetParam(int nDevType,xml_node<> *rootNode,int &nValue);

    //控制结果主动上报
    //从配置数据中获得查询信息
    //void _report_CmdStatusReport(xml_document<> &xml_doc,xml_node<> *rootNode,
     //                            DevMonitorDataPtr curData,map<int,DeviceMonitorItem> &mapMonitorItem);

    //从配置数据中获得查询信息
    void _query_devinfo_from_config(xml_document<> &xml_doc,int nDevType,xml_node<> *rootNode,int  &nValue);
public:
    static  map<int,string>   mapTypeToStr;
    static  string  SrcCode;

};

#endif // BOHUI_PROTOCOL_H
