#include "bohui_protocol.h"
#include "./net/config.h"
#include "LocalConfig.h"
#include "bohui_const_define.h"
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include<time.h>
using namespace std;

string  Bohui_Protocol::SrcCode = GetInst(LocalConfig).local_station_id();


Bohui_Protocol::Bohui_Protocol()
{
}


bool Bohui_Protocol::parseDataFromStr(string &strMsg,string &responseBody)
{
    int nMsgId=-100;
    int nPriority=0;
    try
    {
        xml_document<>   xml_doc;
        xml_doc.parse<0>(const_cast<char*>(strMsg.c_str()));
        xml_node<> *rootNode = xml_doc.first_node("Msg");
        //检查xml头
        if(_checkXmlHeader(xml_doc,nMsgId,nPriority,rootNode)==true)
        {
            xml_node<> *requestNode = rootNode->first_node("Request");
            if(requestNode!=NULL)
            {
                int nRsltValue=0;
                string cmdType = requestNode->first_attribute("Type")->value();
                if(cmdType == CONST_STR_BOHUI_TYPE[BH_POTO_AlarmTimeSet])
                     _setAlarmTime(requestNode,nRsltValue);
                else if(cmdType == CONST_STR_BOHUI_TYPE[BH_POTO_AlarmParamSet])
                    _setAlarmParam(requestNode,nRsltValue);

                createResponseMsg(nMsgId,nRsltValue,cmdType.c_str(),responseBody);
            }else
                createResponseMsg(nMsgId,11,CONST_STR_BOHUI_TYPE[BH_POTO_XmlContentError],responseBody);
        }
        else {
            //xml内容解析错误（SrcCode，MsgId等）
            createResponseMsg(nMsgId,11,CONST_STR_BOHUI_TYPE[BH_POTO_XmlContentError],responseBody);
        }
    }
    catch(...)
    {
        //xml载入异常，replyID=-100(暂定，待修改),vlaue=10(xml解析错误),
         createResponseMsg(nMsgId,10,CONST_STR_BOHUI_TYPE[BH_POTO_XmlParseError],responseBody);
    }

    return true;

}

bool Bohui_Protocol::_checkXmlHeader(xml_document<>  &xmlMsg,int &msgId,int &priority,xml_node<> *rootNode)
{
    try
    {
        if(rootNode!=NULL)
        {
            string sVer = rootNode->first_attribute("Version")->value();
            msgId = strtol(rootNode->first_attribute("MsgID")->value(),NULL,10);
            priority = strtol( rootNode->first_attribute("Priority")->value(),NULL,10);
            if(sVer=="1.0" && msgId>=0 && priority>=0)
                return true;
        }
    }
    catch(...)
    {
        return false;
    }
    return false;
}

//创建回复消息
bool Bohui_Protocol::createResponseMsg(int nReplyId,int nValue,const char* nCmdType,string &responseBody)
{
    xml_document<> xml_responseMsg;
    //本地MsgId暂时固定为0
    xml_node<> *msgRootNode = _createResponseXmlHeader(xml_responseMsg,0,nReplyId);
    if(msgRootNode!=NULL)
    {
        xml_node<> *xml_resps = xml_responseMsg.allocate_node(node_element,"Response");
        msgRootNode->append_node(xml_resps);
        xml_resps->append_attribute(xml_responseMsg.allocate_attribute("Type",nCmdType));//CONST_STR_BOHUI_TYPE[nCmdType]
        if(nValue==0)
        {
            int rsltValue = 0;
            if(strcmp(nCmdType,CONST_STR_BOHUI_TYPE[0])==0)
                _execTsmtQueryCmd(msgRootNode,rsltValue);
            else if(strcmp(nCmdType,CONST_STR_BOHUI_TYPE[1])==0)
                _execEnvMonQueryCmd(msgRootNode,rsltValue);
            else if(strcmp(nCmdType,CONST_STR_BOHUI_TYPE[2])==0)
                _execLinkDevQueryCmd(msgRootNode,rsltValue);

            xml_resps->append_attribute(xml_responseMsg.allocate_attribute("Value",xml_responseMsg.allocate_string(boost::lexical_cast<std::string>(rsltValue).c_str())));

        }
        else
            xml_resps->append_attribute(xml_responseMsg.allocate_attribute("Value",xml_responseMsg.allocate_string(boost::lexical_cast<std::string>(nValue).c_str())));


        xml_resps->append_attribute(xml_responseMsg.allocate_attribute("Desc",CONST_STR_RESPONSE_VALUE_DESC[nValue]));
        xml_resps->append_attribute(xml_responseMsg.allocate_attribute("Comment",""));
    }

   rapidxml::print(std::back_inserter(responseBody), xml_responseMsg, 0);

   return true;
}

//生成xml头
//说明：xmlMsg--
xml_node<>*  Bohui_Protocol::_createResponseXmlHeader(xml_document<>  &xmlMsg,int nMsgId,int nReplyId)
{
    try
    {
        xml_node<>* rot = xmlMsg.allocate_node(rapidxml::node_pi,xmlMsg.allocate_string("xml version='1.0' encoding='utf-8' standalone='yes'"));
        xmlMsg.append_node(rot);
        xml_node<>* nodeHeader=   xmlMsg.allocate_node(node_element,"Msg");
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("Version","1.0"));
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("MsgID",xmlMsg.allocate_string(boost::lexical_cast<std::string>(nMsgId).c_str())));
        //string typ=nType>0?"Down":"Up";
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("Type","Up"));//typ.c_str()
        time_t curTime = time(0);
        tm *ltime = localtime(&curTime);
        std::string createTM = str(boost::format("%1%-%2%-%3% %4%:%5%:%6%")%(ltime->tm_year+1900)%(ltime->tm_mon+1)
                                   %ltime->tm_mday%ltime->tm_hour%ltime->tm_min%ltime->tm_sec);
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("DateTime",xmlMsg.allocate_string(createTM.c_str())));

        nodeHeader->append_attribute(xmlMsg.allocate_attribute("SrcCode","110000N01"));
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("DstCode","110000X01"));
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("SrcURL","http://192.168.1.140/Server"));
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("ReplyID",xmlMsg.allocate_string(boost::lexical_cast<std::string>(nReplyId).c_str())));
        xmlMsg.append_node(nodeHeader);
        return nodeHeader;

    }
    catch(...)
    {
        return NULL;
    }

    return NULL;
}

bool Bohui_Protocol::_createResponseXmlBody(string &sXmlBody,int nType)
{
    return false;
}

//查询发射机信息
void Bohui_Protocol::_execTsmtQueryCmd(xml_node<> *rootNode,int  &nValue)
{
    nValue = 13;
}

//查询动环信息
void Bohui_Protocol::_execEnvMonQueryCmd(xml_node<> *rootNode,int  &nValue)
{
nValue = 2;
}

//查询链路设备信息
void Bohui_Protocol::_execLinkDevQueryCmd(xml_node<> *rootNode,int  &nValue)
{
nValue = 3;
}

//设置告警运行图
void Bohui_Protocol::_setAlarmTime(xml_node<> *rootNode,int &nValue)
{

}

//设置告警门限
void Bohui_Protocol::_setAlarmParam(xml_node<> *rootNode,int &nValue)
{

}

bool Bohui_Protocol::_parse_ManualPowerControl_xml(xml_document<> &xmlMsg,int &nMsgId,string &responseBody)
{
    /* if(_createXmlHeader(xmlMsg,1,nMsgId,-1)==true)
    {
        xml_node<> *msg_node=xmlMsg.first_node("Msg");
        xml_node<> *xml_resps = xmlMsg.allocate_node(node_element,"Response");

        xml_resps->append_attribute(xmlMsg.allocate_attribute("Type",CONST_STR_BOHUI_TYPE[0]));
        xml_resps->append_attribute(xmlMsg.allocate_attribute("Value","-1"));
        xml_resps->append_attribute(xmlMsg.allocate_attribute("Desc","失败"));
        xml_resps->append_attribute(xmlMsg.allocate_attribute("Comment","类型出错"));
        msg_node->append_node(xml_resps);
        rapidxml::print(std::back_inserter(responseBody), xmlMsg, 0);
        return true;
    }
    else
        return false;*/
    return false;
}
