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
    //创建xml头信息
    bool  createXmlHeader(xml_document<>  &xmlMsg,int nType,int nMsgId,int nReplyId);
    //检查xml头信息
    bool  checkXmlHeader(xml_document<>  &xmlMsg,int &msgId,int &priority);
    //根据类型创建xml
    bool  createXmlResponseMsg(string &sXmlBody,int nType);
    //分析xml数据是否符合协议
    bool  parseDataFromStr(string &strMsg,string &responseBody);
protected:
    bool _parse_ManualPowerControl_xml(xml_document<> &xmlMsg,int &nMsgId,string &responseBody);
};

#endif // BOHUI_PROTOCOL_H
