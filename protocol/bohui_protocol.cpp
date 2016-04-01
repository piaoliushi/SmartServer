#include "bohui_protocol.h"
#include "./net/config.h"
#include "LocalConfig.h"
#include "StationConfig.h"
#include "bohui_const_define.h"
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include<time.h>
using namespace std;
using namespace db;
string  Bohui_Protocol::SrcCode = GetInst(LocalConfig).local_station_id();
map<int,string> Bohui_Protocol::mapTypeToStr= map<int,string>();

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
                     _setAlarmTime(requestNode,nRsltValue);//发射机告警运行图设置
                else if(cmdType == CONST_STR_BOHUI_TYPE[BH_POTO_AlarmParamSet])
                    _setAlarmParam(BH_POTO_AlarmSwitchSet,requestNode,nRsltValue); //发射机告警门限参数设置
                else if(cmdType == CONST_STR_BOHUI_TYPE[BH_POTO_AlarmSwitchSet])
                    _setAlarmSwitchSetParam(BH_POTO_AlarmSwitchSet,requestNode,nRsltValue);//发射机告警开关设置
                else if(cmdType == CONST_STR_BOHUI_TYPE[BH_POTO_AlarmParamSetE])
                    _setAlarmParam(BH_POTO_AlarmParamSetE,requestNode,nRsltValue); //动环告警门限参数设置
                else if(cmdType == CONST_STR_BOHUI_TYPE[BH_POTO_AlarmSwitchSetE])
                    _setAlarmSwitchSetParam(BH_POTO_AlarmSwitchSetE,requestNode,nRsltValue);//动环告警开关设置
                else if(cmdType == CONST_STR_BOHUI_TYPE[BH_POTO_AlarmParamSetD])
                    _setAlarmParam(BH_POTO_AlarmParamSetD,requestNode,nRsltValue); //链路告警门限参数设置
                else if(cmdType == CONST_STR_BOHUI_TYPE[BH_POTO_AlarmSwitchSetD])
                    _setAlarmSwitchSetParam(BH_POTO_AlarmSwitchSetD,requestNode,nRsltValue);//链路告警开关设置

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
           // int rsltValue = 0;
            if(strcmp(nCmdType,CONST_STR_BOHUI_TYPE[0])==0)
                _execTsmtQueryCmd(xml_responseMsg,msgRootNode,nValue);//查询发射机信息
            else if(strcmp(nCmdType,CONST_STR_BOHUI_TYPE[1])==0)
                _execEnvMonQueryCmd(xml_responseMsg,msgRootNode,nValue);//查询动环设备信息
            else if(strcmp(nCmdType,CONST_STR_BOHUI_TYPE[2])==0)
                _execLinkDevQueryCmd(xml_responseMsg,msgRootNode,nValue);//查询链路设备信息

            xml_resps->append_attribute(xml_responseMsg.allocate_attribute("Value",xml_responseMsg.allocate_string(boost::lexical_cast<std::string>(nValue).c_str())));

        }
        else
            xml_resps->append_attribute(xml_responseMsg.allocate_attribute("Value",xml_responseMsg.allocate_string(boost::lexical_cast<std::string>(nValue).c_str())));


        xml_resps->append_attribute(xml_responseMsg.allocate_attribute("Desc",CONST_STR_RESPONSE_VALUE_DESC[nValue]));
        xml_resps->append_attribute(xml_responseMsg.allocate_attribute("Comment",""));
    }

   rapidxml::print(std::back_inserter(responseBody), xml_responseMsg, 0);

   return true;
}

//创建上报数据消息
bool Bohui_Protocol::createReportDataMsg(int nReplyId,int nCmdType,string sDevId,int nDevType,DevMonitorDataPtr curData,
                         map<int,DeviceMonitorItem> &mapMonitorItem,string &reportBody)
{
    xml_document<> xml_reportMsg;
    //本地MsgId暂时固定为2
    time_t curTime = time(0);
    tm *local_time = localtime(&curTime);
    static  char str_time[64];
    strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", local_time);


    xml_node<> *msgRootNode = _createResponseXmlHeader(xml_reportMsg,2,-1);
    if(msgRootNode!=NULL)
    {
        xml_node<> *xml_resps = xml_reportMsg.allocate_node(node_element,CONST_STR_BOHUI_TYPE[nCmdType]);
        msgRootNode->append_node(xml_resps);
        xml_node<> *xml_Quality = NULL;
        if(BH_POTO_QualityRealtimeReport == nCmdType){
            xml_Quality = xml_reportMsg.allocate_node(node_element,"Quality");
            xml_Quality->append_attribute(xml_reportMsg.allocate_attribute("TransmitterID",xml_reportMsg.allocate_string(sDevId.c_str())));
            xml_Quality->append_attribute(xml_reportMsg.allocate_attribute("CheckTime",xml_reportMsg.allocate_string(str_time)));
            xml_resps->append_node(xml_Quality);

            map<int,DataInfo>::iterator iter  = curData->mValues.begin();
            for(;iter!=curData->mValues.end();++iter){
                    xml_node<> *xml_Quality_Index = xml_reportMsg.allocate_node(node_element,"QualityIndex ");
                    xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Type",boost::lexical_cast<std::string>(mapMonitorItem[iter->first].iTargetId).c_str()));
                    xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("ModuleType",boost::lexical_cast<std::string>(mapMonitorItem[iter->first].iModTypeId).c_str()));
                    xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("ModuleID",boost::lexical_cast<std::string>(mapMonitorItem[iter->first].iModDevId).c_str()));
                    float curValue = iter->second.fValue;
                    xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Value",boost::lexical_cast<std::string>(curValue).c_str()));
                    xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Desc",boost::lexical_cast<std::string>(mapTypeToStr[mapMonitorItem[iter->first].iTargetId]).c_str()));
                    xml_Quality->append_node(xml_Quality_Index);
            }

        }else if(BH_POTO_EnvQualityRealtimeReport == nCmdType){
            switch (nDevType) {
            case 1://电力仪
                break;
            case 2://温湿度
                break;
            case 3://烟雾
                break;
            case 4://水禁
                break;
            case 5://空调
                break;
            case 6://GPS传感器
                break;
            }
        }
    }

    rapidxml::print(std::back_inserter(reportBody), xml_reportMsg, 0);
    return true;
}

//创建上报告警消息
bool Bohui_Protocol::createReportAlarmDataMsg(int nReplyId,int nCmdType,string sDevId,CurItemAlarmInfo &alarmInfo,int nMod,
                                string &sReason,string &reportBody)
{
    xml_document<> xml_reportMsg;
    //本地MsgId暂时固定为2
    xml_node<> *msgRootNode = _createResponseXmlHeader(xml_reportMsg,2,-1);
    if(msgRootNode!=NULL)
    {
        xml_node<> *xml_resps = xml_reportMsg.allocate_node(node_element,CONST_STR_BOHUI_TYPE[nCmdType]);
        msgRootNode->append_node(xml_resps);
        xml_node<> *xml_Alarm = NULL;
        if(BH_POTO_QualityAlarmReport == nCmdType){
            xml_Alarm = xml_reportMsg.allocate_node(node_element,"QualityAlarm");
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("TransmitterID",xml_reportMsg.allocate_string(sDevId.c_str())));
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("ModuleType",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(alarmInfo.nModuleType).c_str())));
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("ModuleID",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(alarmInfo.nModuleId).c_str())));
        }else if(BH_POTO_EnvAlarmReport == nCmdType){//
           xml_Alarm =  xml_reportMsg.allocate_node(node_element,"Alarm");
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("DevID",xml_reportMsg.allocate_string(sDevId.c_str())));
        }
        xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("AlarmID",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(alarmInfo.nAlarmId).c_str())));
        xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Mode",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(nMod).c_str())));
        xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Type",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(alarmInfo.nType).c_str())));
        xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Desc",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(mapTypeToStr[alarmInfo.nType]).c_str())));
        xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Reason",xml_reportMsg.allocate_string(sReason.c_str())));
        tm * local_time = localtime(&(alarmInfo.startTime));
        static  char str_time[64];
        strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", local_time);
        xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("CheckTime",xml_reportMsg.allocate_string(str_time)));

        xml_resps->append_node(xml_Alarm);
    }

     rapidxml::print(std::back_inserter(reportBody), xml_reportMsg, 0);
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
void Bohui_Protocol::_execTsmtQueryCmd(xml_document<> &xml_doc,xml_node<> *rootNode,int  &nValue)
{
     nValue = 13;
    _query_devinfo_from_config(xml_doc,BH_POTO_TransmitterQuery,rootNode,nValue);

}

//查询动环信息
void Bohui_Protocol::_execEnvMonQueryCmd(xml_document<> &xml_doc,xml_node<> *rootNode,int  &nValue)
{
    nValue = 13;
   _query_devinfo_from_config(xml_doc,BH_POTO_EnvMonDevQuery,rootNode,nValue);
}

//查询链路设备信息
void Bohui_Protocol::_execLinkDevQueryCmd(xml_document<> &xml_doc,xml_node<> *rootNode,int  &nValue)
{
    nValue = 13;
   _query_devinfo_from_config(xml_doc,BH_POTO_LinkDevQuery,rootNode,nValue);
}

//从配置数据中获得查询信息
void Bohui_Protocol::_query_devinfo_from_config(xml_document<> &xml_doc,int nDevType,xml_node<> *rootNode,int  &nValue)
{

    xml_node<> *xml_resps_info = xml_doc.allocate_node(node_element,"ResponseInfo");
    rootNode->append_node(xml_resps_info);
     xml_node<> *xml_linkdev_list=NULL;
     xml_node<> *xml_device=NULL;
    vector<ModleInfo> &mapModleInfo = GetInst(StationConfig).get_Modle();
    int nDevSize = mapModleInfo.size();
    for(int nIndex=0;nIndex<nDevSize;++nIndex)
    {

        map<string,DeviceInfo>::iterator iter = mapModleInfo[nIndex].mapDevInfo.begin();
        if(iter!=mapModleInfo[nIndex].mapDevInfo.end())
            nValue=12;
        for(;iter!=mapModleInfo[nIndex].mapDevInfo.end();++iter)
        {
            xml_linkdev_list=NULL;
            xml_device=NULL;
            //类型分类判断
            int temType=(*iter).second.iDevType;
            if(temType>DEVICE_TRANSMITTER && temType<DEVICE_GS_RECIVE)
                temType = BH_POTO_EnvMonDevQuery;//动环设备
            else if(temType>=DEVICE_GS_RECIVE)
                temType = BH_POTO_LinkDevQuery;     //链路设备
            else if(temType==DEVICE_TRANSMITTER)
                temType = BH_POTO_TransmitterQuery;//发射机设备

            if(nDevType!=temType)
                continue;
            if(nDevType==BH_POTO_LinkDevQuery)
            {
                xml_linkdev_list = xml_resps_info->first_node("DevList");
                bool bFind=false;
                while(xml_linkdev_list!=NULL){
                   int nValue =  strtol(xml_linkdev_list->first_attribute("Type")->value(),NULL,10);
                   if(nValue == (*iter).second.iDevType){
                       bFind=true;
                       break;
                   }
                    xml_linkdev_list = xml_linkdev_list->next_sibling("DevList");
                }
                if(bFind==false){
                    xml_linkdev_list = xml_doc.allocate_node(node_element,DEVICE_TYPE_XML_DESC[nDevType]);
                    xml_resps_info->append_node(xml_linkdev_list);
                    xml_linkdev_list->append_attribute(xml_doc.allocate_attribute("Type",xml_doc.allocate_string(boost::lexical_cast<std::string>( (*iter).second.iDevType).c_str())));
                    xml_linkdev_list->append_attribute(xml_doc.allocate_attribute("Desc"," "));//链路设备名称暂略
                }
                xml_device = xml_doc.allocate_node(node_element,"Dev");
                xml_linkdev_list->append_node(xml_device);
                xml_device->append_attribute(xml_doc.allocate_attribute("DevID",(*iter).second.sDevNum.c_str()));
                xml_device->append_attribute(xml_doc.allocate_attribute("DevName",(*iter).second.sDevName.c_str()));
            }else{

                xml_device = xml_doc.allocate_node(node_element,DEVICE_TYPE_XML_DESC[nDevType]);
                xml_resps_info->append_node(xml_device);
                xml_device->append_attribute(xml_doc.allocate_attribute("ID",(*iter).second.sDevNum.c_str()));
                xml_device->append_attribute(xml_doc.allocate_attribute("Type",xml_doc.allocate_string(boost::lexical_cast<std::string>( (*iter).second.iDevType).c_str())));//链路设备名称暂略
                xml_device->append_attribute(xml_doc.allocate_attribute("Name",(*iter).second.sDevName.c_str()));
            }

            //频率属性
            map<string,DevProperty>::iterator iter_propty =  (*iter).second.map_DevProperty.find("Freq");
            if(iter_propty!= (*iter).second.map_DevProperty.end())
                xml_device->append_attribute(xml_doc.allocate_attribute("Freq",(*iter_propty).second.property_value.c_str()));
            //功率属性(W)
            iter_propty =  (*iter).second.map_DevProperty.find("Power");
            if(iter_propty!= (*iter).second.map_DevProperty.end())
                xml_device->append_attribute(xml_doc.allocate_attribute("Power",(*iter_propty).second.property_value.c_str()));
            //主备机标识
            iter_propty =  (*iter).second.map_DevProperty.find("Standby");
            if(iter_propty!= (*iter).second.map_DevProperty.end())
                xml_device->append_attribute(xml_doc.allocate_attribute("Standby",(*iter_propty).second.property_value.c_str()));
            //型号
            iter_propty =  (*iter).second.map_DevProperty.find("Model");
            if(iter_propty!= (*iter).second.map_DevProperty.end())
                xml_device->append_attribute(xml_doc.allocate_attribute("Model",(*iter_propty).second.property_value.c_str()));
            //分组ID
            iter_propty =  (*iter).second.map_DevProperty.find("GroupID");
            if(iter_propty!= (*iter).second.map_DevProperty.end())
                xml_device->append_attribute(xml_doc.allocate_attribute("GroupID",(*iter_propty).second.property_value.c_str()));
            //分组名称
            iter_propty =  (*iter).second.map_DevProperty.find("GroupName");
            if(iter_propty!= (*iter).second.map_DevProperty.end())
                xml_device->append_attribute(xml_doc.allocate_attribute("GroupName",(*iter_propty).second.property_value.c_str()));
            //节点名称
            iter_propty =  (*iter).second.map_DevProperty.find("NodeName");
            if(iter_propty!= (*iter).second.map_DevProperty.end())
                xml_device->append_attribute(xml_doc.allocate_attribute("NodeName",(*iter_propty).second.property_value.c_str()));
            //制造商
            iter_propty =  (*iter).second.map_DevProperty.find("Manufacturer");
            if(iter_propty!= (*iter).second.map_DevProperty.end())
                xml_device->append_attribute(xml_doc.allocate_attribute("Manufacturer",(*iter_propty).second.property_value.c_str()));
            //设备描述
            iter_propty =  (*iter).second.map_DevProperty.find("Desc");
            if(iter_propty!= (*iter).second.map_DevProperty.end())
                xml_device->append_attribute(xml_doc.allocate_attribute("Desc",(*iter_propty).second.property_value.c_str()));
        }
    }

}

//设置告警运行图
void Bohui_Protocol::_setAlarmTime(xml_node<> *rootNode,int &nValue)
{

}

//设置告警门限
void Bohui_Protocol::_setAlarmParam(int nDevType,xml_node<> *rootNode,int &nValue)
{
    vector<string> vecDeviceNumber;//保存影响的设备ID集合
    if (GetInst(DataBaseOperation).SetAlarmLimit(rootNode,nValue,vecDeviceNumber)==true)//nDevType,
    {
        // 通知设备服务......
    }
}

//告警开关设置
void Bohui_Protocol::_setAlarmSwitchSetParam(int nDevType,xml_node<> *rootNode,int &nValue)
{
    vector<string> vecDeviceNumber;//保存影响的设备ID集合
    if (GetInst(DataBaseOperation).SetEnableAlarm(rootNode,nValue,vecDeviceNumber)==true)//nDevType,
    {
        // 通知设备服务......
    }
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
