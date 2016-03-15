#include "bohui_protocol.h"
#include "bohui_const_define.h"
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include<time.h>
Bohui_Protocol::Bohui_Protocol()
{
}
//生成xml头
//说明：xmlMsg--
bool  Bohui_Protocol::createXmlHeader(xml_document<>  &xmlMsg,int nType,int nMsgId,int nReplyId)
{
    try
    {
        xml_node<>* rot = xmlMsg.allocate_node(rapidxml::node_pi,xmlMsg.allocate_string("xml version='1.0' encoding='utf-8' standalone='yes'"));
        xmlMsg.append_node(rot);
        xml_node<>* nodeHeader=   xmlMsg.allocate_node(node_element,"Msg","message header");
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("Version","1.0"));
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("MsgID",boost::lexical_cast<std::string>(nMsgId).c_str()));

        //string typ=nType>0?"Down":"Up";
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("Type","Up"));//typ.c_str()

        time_t curTime = time(0);
        tm *ltime = localtime(&curTime);
        std::string createTM = str(boost::format("%1%-%2%-%3% %4%:%5%:%6%")%(ltime->tm_year+1900)%(ltime->tm_mon+1)
                                                        %ltime->tm_mday%ltime->tm_hour%ltime->tm_min%ltime->tm_sec);
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("DateTime",createTM.c_str()));

        nodeHeader->append_attribute(xmlMsg.allocate_attribute("SrcCode","110000N01"));
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("DstCode","110000X01"));
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("SrcURL","http://192.168.1.140/Server"));

        nodeHeader->append_attribute(xmlMsg.allocate_attribute("ReplyID",boost::lexical_cast<std::string>(nReplyId).c_str()));
    }
    catch(...)
    {
        return false;
    }

    return true;
}
bool Bohui_Protocol::parseDataFromStr(string &strMsg,string &responseBody)
{
    try
    {
        //strMsg.replace("[\\n\\r]*","");
        //file<>   fdoc(strMsg.data());
        xml_document<>   xml_doc;
       char * xxxx=const_cast<char*>(strMsg.c_str());
        xml_doc.parse<0>(xxxx);//strMsg)
        int nMsgId=-1;
        int nPriority=0;
         xml_document<> xml_responseBody;
        if(checkXmlHeader(xml_doc,nMsgId,nPriority)==true)
        {
            switch(nMsgId)
            {
            case BH_POTO_ManualPowerControl:
                return  _parse_ManualPowerControl_xml(xml_responseBody,nMsgId,responseBody);
            case BH_POTO_TransmitterQuery:
                break;
            case BH_POTO_AlarmSwitchSet:
                break;
            }
        }
        else{
                if(createXmlHeader(xml_responseBody,1,nMsgId,-1)==true)
                {
                    xml_node<> *xml_resps = xml_responseBody.allocate_node(node_element,"Response");
                    xml_resps->append_attribute(xml_responseBody.allocate_attribute("Type",boost::lexical_cast<std::string>(nMsgId).c_str()));
                    xml_resps->append_attribute(xml_responseBody.allocate_attribute("Value","-1"));
                    xml_resps->append_attribute(xml_responseBody.allocate_attribute("Desc","失败"));
                    xml_resps->append_attribute(xml_responseBody.allocate_attribute("Comment","该MsgId未定义"));
                    return true;
                }
                else
                    return false;
        }
    }
    catch(...)
    {
        return false;
    }

return true;

}

bool Bohui_Protocol::checkXmlHeader(xml_document<>  &xmlMsg,int &msgId,int &priority)
{
    try
    {
        xml_node<> *rootNode = xmlMsg.first_node("Msg");
        if(rootNode!=NULL)
        {
            string sVer = rootNode->first_attribute("Version")->value();
            msgId = strtol(rootNode->first_attribute("MsgID")->value(),NULL,10);
            priority = strtol( rootNode->first_attribute("Priority")->value(),NULL,10);
            if(sVer=="1.0" && msgId>0 && priority>=0)
                return true;
         }
    }
    catch(...)
    {
        return false;
    }
    return false;
}

bool Bohui_Protocol::createXmlResponseMsg(string &sXmlBody,int nType)
{
    return false;
}

bool Bohui_Protocol::_parse_ManualPowerControl_xml(xml_document<> &xmlMsg,int &nMsgId,string &responseBody)
{
    if(createXmlHeader(xmlMsg,1,nMsgId,-1)==true)
    {
        xml_node<> *xml_resps = xmlMsg.allocate_node(node_element,"Response");
        xml_resps->append_attribute(xmlMsg.allocate_attribute("Type",boost::lexical_cast<std::string>(nMsgId).c_str()));
        xml_resps->append_attribute(xmlMsg.allocate_attribute("Value","-1"));
        xml_resps->append_attribute(xmlMsg.allocate_attribute("Desc","失败"));
        xml_resps->append_attribute(xmlMsg.allocate_attribute("Comment","该MsgId未定义"));
        return true;
    }
    else
        return false;
    return false;
}
