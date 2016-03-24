#ifndef BOHUI_PROTOCOL_H
#define BOHUI_PROTOCOL_H
#include <string>
#include "./rapidxml/rapidxml.hpp"
#include "./rapidxml/rapidxml_utils.hpp"
#include "./rapidxml/rapidxml_print.hpp"
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

    //从配置数据中获得查询信息
    void _query_devinfo_from_config(xml_document<> &xml_doc,int nDevType,xml_node<> *rootNode,int  &nValue);
private:
    static  string  SrcCode;
};

#endif // BOHUI_PROTOCOL_H
