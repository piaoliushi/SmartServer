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
string  Bohui_Protocol::SrcCode = "";//GetInst(LocalConfig).src_code();
string  Bohui_Protocol::DstCode = "";//GetInst(LocalConfig).dst_code();
map<int,pair<string,string> > Bohui_Protocol::mapTypeToStr= map<int,pair<string,string> >();

Bohui_Protocol::Bohui_Protocol()
{
     Bohui_Protocol::SrcCode = GetInst(LocalConfig).src_code();
     Bohui_Protocol::DstCode = GetInst(LocalConfig).dst_code();
}

//分析http请求数据(包括发射机,动环,链路告警门限,运行图,告警开关设置)
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
                else if(cmdType == CONST_STR_BOHUI_TYPE[BH_POTO_ManualPowerControl])
                    _controlDeviceCommand(BH_POTO_ManualPowerControl,requestNode,nRsltValue);//发射机控制

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
            if(sVer=="1" && msgId>=0 && priority>=0)
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

//创建控制结果上报消息
bool Bohui_Protocol::creatExcutResultReportMsg(int nReplyId,int nCmdType,string sDevId,const string &sTime,
                                               int devState,const string &sDesc,string &reportBody)
{
     xml_document<> xml_reportMsg;
     xml_node<> *msgRootNode = _createResponseXmlHeader(xml_reportMsg,2,-1);
     if(msgRootNode!=NULL){
         xml_node<> *xml_resps = xml_reportMsg.allocate_node(node_element,CONST_STR_BOHUI_TYPE[nCmdType]);
         msgRootNode->append_node(xml_resps);
         xml_resps->append_attribute(xml_reportMsg.allocate_attribute("TransmitterID",xml_reportMsg.allocate_string(sDevId.c_str())));
         xml_resps->append_attribute(xml_reportMsg.allocate_attribute("CmdStatus",boost::lexical_cast<std::string>(devState).c_str()));
         xml_resps->append_attribute(xml_reportMsg.allocate_attribute("Time",xml_reportMsg.allocate_string(sTime.c_str())));
          xml_resps->append_attribute(xml_reportMsg.allocate_attribute("CmdStatusDesc",xml_reportMsg.allocate_string(sDesc.c_str())));
     }

    rapidxml::print(std::back_inserter(reportBody), xml_reportMsg, 0);
    return true;
}

//创建上报数据消息
bool Bohui_Protocol::createReportDataMsg(int nReplyId,string sDevId,int nDevType,DevMonitorDataPtr &curData,
                         map<int,DeviceMonitorItem> &mapMonitorItem,string &reportBody)
{
    xml_document<> xml_reportMsg;
    //本地MsgId暂时固定为2
    time_t curTime = time(0);
    tm *local_time = localtime(&curTime);
    static  char str_time[64];
    strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", local_time);
    //类型分类判断
    int nCmdType = -1;
    if(nDevType>DEVICE_TRANSMITTER && nDevType<DEVICE_GS_RECIVE)
        nCmdType = BH_POTO_EnvQualityRealtimeReport;//动环设备
    else if(nDevType>=DEVICE_GS_RECIVE)
        nCmdType = BH_POTO_LinkDevQualityReport;//链路设备
    else if(nDevType==DEVICE_TRANSMITTER)
        nCmdType = BH_POTO_QualityRealtimeReport;//发射机设备

    if(nCmdType<0)
        return false;

    xml_node<> *msgRootNode = _createResponseXmlHeader(xml_reportMsg,2,-1);
    if(msgRootNode!=NULL)
    {
        xml_node<> *xml_resps = xml_reportMsg.allocate_node(node_element,CONST_STR_BOHUI_TYPE[nCmdType]);
        msgRootNode->append_node(xml_resps);
        xml_node<> *xml_Quality = NULL;
        //发射机实时数据
        if(BH_POTO_QualityRealtimeReport == nCmdType){
            xml_Quality = xml_reportMsg.allocate_node(node_element,"Quality");
            xml_Quality->append_attribute(xml_reportMsg.allocate_attribute("TransmitterID",xml_reportMsg.allocate_string(sDevId.c_str())));
            xml_Quality->append_attribute(xml_reportMsg.allocate_attribute("CheckTime",xml_reportMsg.allocate_string(str_time)));
            xml_resps->append_node(xml_Quality);

            map<int,DeviceMonitorItem>::iterator cell_iter = mapMonitorItem.begin();
            for(;cell_iter!=mapMonitorItem.end();++cell_iter){
               // if(cell_iter->second.bUpload == false)
               //    continue;
                xml_node<> *xml_Quality_Index = xml_reportMsg.allocate_node(node_element,"QualityIndex ");
                //string sType = boost::lexical_cast<std::string>(cell_iter->second.iTargetId);
                xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Type",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iTargetId).c_str())));
                xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("ModuleType",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iModTypeId).c_str())));
               xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("ModuleID",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iModDevId).c_str())));
                string  sValue = str(boost::format("%.2f")%curData->mValues[cell_iter->first].fValue);
                xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Value",xml_reportMsg.allocate_string(sValue.c_str())));
                xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Desc",boost::lexical_cast<std::string>(mapTypeToStr[cell_iter->second.iTargetId].first).c_str()));
                xml_Quality->append_node(xml_Quality_Index);

            }
        }else if(BH_POTO_EnvQualityRealtimeReport == nCmdType){//动环数据上报
            xml_node<> *xml_dev_node = NULL;
            switch (nDevType) {
            case DEVICE_ELEC:{//电力仪
                xml_Quality = xml_reportMsg.allocate_node(node_element,"ThreePhasePowerDev");
                xml_resps->append_node(xml_Quality);
                xml_dev_node = xml_reportMsg.allocate_node(node_element,"ThreePhasePower");
            }
                break;
            case DEVICE_TEMP:{//温湿度
                xml_Quality = xml_reportMsg.allocate_node(node_element,"TempHumidityDev");
                xml_resps->append_node(xml_Quality);
                xml_dev_node = xml_reportMsg.allocate_node(node_element,"TempHumidity");
            }
                break;
            case DEVICE_SMOKE:{//烟雾
                xml_Quality = xml_reportMsg.allocate_node(node_element,"FireAlarmDev");
                xml_resps->append_node(xml_Quality);
                xml_dev_node = xml_reportMsg.allocate_node(node_element,"FireAlarm");
            }
                break;
            case DEVICE_WATER:{//水禁
                xml_Quality = xml_reportMsg.allocate_node(node_element,"WaterAlarmDev");
                xml_resps->append_node(xml_Quality);
                xml_dev_node = xml_reportMsg.allocate_node(node_element,"WaterAlarm");
            }
                break;
            case DEVICE_AIR://空调
            case DEVICE_GPS://GPS传感器
            case DEVICE_SWITCH://切换设备
            case DEVICE_GPS_TIME://GPS授时器
            case DEVICE_GS_RECIVE://卫星接收机
            case DEVICE_MW://微波接收机
            case DEVICE_TR://光收发器
            case DEVICE_MUX://复用器
            case DEVICE_MO://调制器
            case DEVICE_ANTENNA://同轴开关
                break;
            }
            xml_Quality->append_node(xml_dev_node);
             map<int,DeviceMonitorItem>::iterator cell_iter = mapMonitorItem.begin();
             for(;cell_iter!=mapMonitorItem.end();++cell_iter){
                 if(cell_iter->second.bUpload == false)
                     continue;
                 if(mapTypeToStr.find(cell_iter->second.iTargetId) == mapTypeToStr.end())
                     continue;
                 if(mapTypeToStr[cell_iter->second.iTargetId].second.empty())
                     continue;
                 xml_node<> *xml_Quality_Index = xml_reportMsg.allocate_node(node_element,mapTypeToStr[cell_iter->second.iTargetId].second.c_str());
                 xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Type",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iTargetId).c_str())));
                 float curValue = curData->mValues[cell_iter->first].fValue;
                 xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Value",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(curValue).c_str())));
                 xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Desc",boost::lexical_cast<std::string>(mapTypeToStr[cell_iter->second.iTargetId].first).c_str()));
                 xml_Quality->append_node(xml_Quality_Index);
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

        if(BH_POTO_QualityAlarmReport == nCmdType){//发射机告警
            xml_Alarm = xml_reportMsg.allocate_node(node_element,"QualityAlarm");
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("TransmitterID",xml_reportMsg.allocate_string(sDevId.c_str())));
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("ModuleType",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(alarmInfo.nModuleType).c_str())));
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("ModuleID",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(alarmInfo.nModuleId).c_str())));
        }else if(BH_POTO_EnvAlarmReport == nCmdType){//动环告警
            xml_Alarm =  xml_reportMsg.allocate_node(node_element,"Alarm");
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("DevID",xml_reportMsg.allocate_string(sDevId.c_str())));
        }else if(BH_POTO_LinkDevAlarmReport == nCmdType){//链路设备告警

            //暂时未实现..........
        }else if(BH_POTO_CommunicationReport == nCmdType){//设备通讯异常
            xml_Alarm = xml_reportMsg.allocate_node(node_element,"Communication");
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("TransmitterID",xml_reportMsg.allocate_string(sDevId.c_str())));
        }
        xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("AlarmID",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(alarmInfo.nAlarmId).c_str())));
        xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Mode",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(nMod).c_str())));
        xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Type",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(alarmInfo.nType).c_str())));
       if(BH_POTO_CommunicationReport == nCmdType)
           xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Desc",xml_reportMsg.allocate_string("发射机断开")));
       else
           xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Desc",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(mapTypeToStr[alarmInfo.nType].first).c_str())));
        if(sReason.empty()==false)
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Reason",xml_reportMsg.allocate_string(sReason.c_str())));
        tm * local_time = localtime(&(alarmInfo.startTime));
        static  char str_time[64];
        strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", local_time);
         if(BH_POTO_CommunicationReport == nCmdType)
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Time",xml_reportMsg.allocate_string(str_time)));
         else
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

        nodeHeader->append_attribute(xmlMsg.allocate_attribute("SrcCode",Bohui_Protocol::SrcCode.c_str()));
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("DstCode",Bohui_Protocol::DstCode.c_str()));
        //nodeHeader->append_attribute(xmlMsg.allocate_attribute("SrcURL",xmlMsg.allocate_string(sDstUrl.c_str())));
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
void Bohui_Protocol::_query_devinfo_from_config(xml_document<> &xml_doc,int nCmdType,xml_node<> *rootNode,int  &nValue)
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
            nValue=0;
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

            if(nCmdType!=temType)
                continue;
            if(nCmdType==BH_POTO_LinkDevQuery)//链路设备
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
                    xml_linkdev_list = xml_doc.allocate_node(node_element,DEVICE_TYPE_XML_DESC[nCmdType]);
                    xml_resps_info->append_node(xml_linkdev_list);
                    xml_linkdev_list->append_attribute(xml_doc.allocate_attribute("Type",xml_doc.allocate_string(boost::lexical_cast<std::string>( (*iter).second.iDevType).c_str())));
                    xml_linkdev_list->append_attribute(xml_doc.allocate_attribute("Desc"," "));//链路设备名称暂略
                }
                xml_device = xml_doc.allocate_node(node_element,"Dev");
                xml_linkdev_list->append_node(xml_device);
                xml_device->append_attribute(xml_doc.allocate_attribute("DevID",(*iter).second.sDevNum.c_str()));
                xml_device->append_attribute(xml_doc.allocate_attribute("DevName",(*iter).second.sDevName.c_str()));
            }else{

                xml_device = xml_doc.allocate_node(node_element,DEVICE_TYPE_XML_DESC[nCmdType]);
                xml_resps_info->append_node(xml_device);
                 if(nCmdType==BH_POTO_TransmitterQuery)
                     xml_device->append_attribute(xml_doc.allocate_attribute("TransmitterID",(*iter).second.sDevNum.c_str()));
                 else
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
            //设备备注
            iter_propty =  (*iter).second.map_DevProperty.find("Remark");
            if(iter_propty!= (*iter).second.map_DevProperty.end())
                xml_device->append_attribute(xml_doc.allocate_attribute("Remark",(*iter_propty).second.property_value.c_str()));
        }
    }

}

//设置告警运行图
void Bohui_Protocol::_setAlarmTime(xml_node<> *rootNode,int &nValue)
{
    vector<string> vecDeviceNumber;//保存影响的设备ID集合
    if (GetInst(DataBaseOperation).SetAlarmTime(rootNode,nValue,vecDeviceNumber)==true)//nDevType,
    {
        // 通知设备服务......
    }
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
//手动控制(发射机)
void Bohui_Protocol::_controlDeviceCommand(int nDevType,xml_node<> *rootNode,int &nValue)
{
    rapidxml::xml_node<>* tranNode = rootNode->first_node("TranInfo");
    if(tranNode!=NULL){
        string sDevId;
        int  nSwitch;
        rapidxml::xml_attribute<char> * attr_devId = tranNode->first_attribute("TransmitterID");
        if(attr_devId==NULL){
            nValue = 11;
            return;
        }
        rapidxml::xml_attribute<char> * attr_switch = tranNode->first_attribute("Switch");
        if(attr_switch==NULL){
            nValue = 11;
            return;
        }
        sDevId = attr_devId->value();
        nSwitch = strtol(attr_switch->value(),NULL,10);
        //--------------发送控制指令------------------------------------//
        //......................
    }
}
