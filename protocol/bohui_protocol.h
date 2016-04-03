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
    bool createReportDataMsg(int nReplyId,string sDevId,int nDevType,DevMonitorDataPtr &curData,
                             map<int,DeviceMonitorItem> &mapMonitorItem,string &reportBody);
    //创建上报指标告警消息
    bool createReportAlarmDataMsg(int nReplyId,int nCmdType,string sDevId,CurItemAlarmInfo &alarmInfo,int nMod,
                                  string &sReason,string &reportBody);
    //创建控制结果上报消息
    bool creatExcutResultReportMsg(int nReplyId,int nCmdType,string sDevId,const string &sTime,
                                    int devState,const string &sDesc,string &reportBody);
protected:
    //创建xml头信息
    xml_node<>*  _createResponseXmlHeader(xml_document<>  &xmlMsg,int nCmdId,int nReplyId);
    //检查xml头信息
    bool  _checkXmlHeader(xml_document<>  &xmlMsg,int &msgId,int &priority,xml_node<> *rootNode);

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

    //设备手动控制(发射机)
    void _controlDeviceCommand(int nDevType,xml_node<> *rootNode,int &nValue);

    //从配置数据中获得查询信息
    void _query_devinfo_from_config(xml_document<> &xml_doc,int nCmdType,xml_node<> *rootNode,int  &nValue);
public:
    static  map<int,pair<string,string> >   mapTypeToStr;
    static  string  SrcCode;

};

#endif // BOHUI_PROTOCOL_H
