#include "bohui_protocol.h"
#include "./net/config.h"
#include "LocalConfig.h"
#include "StationConfig.h"
#include "../net/SvcMgr.h"
#include "bohui_const_define.h"
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include<time.h>
#include "../net/server/http/RequestHandlerFactory.h"
using namespace std;
using namespace db;

string  Bohui_Protocol::SrcCode = "";
string  Bohui_Protocol::DstCode = "";
map<int,pair<string,string> > Bohui_Protocol::mapTypeToStr= map<int,pair<string,string> >();

Bohui_Protocol::Bohui_Protocol()
{
     Bohui_Protocol::SrcCode = GetInst(LocalConfig).src_code();
     Bohui_Protocol::DstCode = GetInst(LocalConfig).dst_code();

}

//分析http请求数据(包括发射机,动环,链路告警门限,运行图,告警开关设置)
bool Bohui_Protocol::parseDataFromStr(const string &strMsg,string &responseBody,string &srcUrl,string sIp)
{

    int nMsgId=-100;
    int nPriority=0;
    try
    {
        xml_document<>   xml_doc;
        xml_doc.parse<0>(const_cast<char*>(strMsg.c_str()));
        xml_node<> *rootNode = xml_doc.first_node("Msg");//bohui消息root
        if(rootNode == NULL){
            rootNode = xml_doc.first_node("service");
            if(rootNode!=NULL){
                //保存服务本地配置xml
                std::ofstream xml_out("ServerLocalConfig.xml");
                string xml_body;
                rapidxml::print(std::back_inserter(xml_body), xml_doc, 0);
                xml_out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
                xml_out << xml_body;
                return true;
            }

        }

        //检查xml头
        if(_checkXmlHeader(xml_doc,nMsgId,nPriority,srcUrl,rootNode)==true)
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
                else if(cmdType == CONST_STR_BOHUI_TYPE[BH_POTO_DeviceControl])
                    _controlSwitchCommand(BH_POTO_DeviceControl,requestNode,nRsltValue);//切换器控制
                else if(cmdType == CONST_STR_BOHUI_TYPE[BH_POTO_ManualTrunControl])
                    _controlDeviceCommand(BH_POTO_ManualTrunControl,requestNode,nRsltValue);//发射机倒机控制


                createResponseMsg(nMsgId,nRsltValue,cmdType.c_str(),responseBody);
            }else {
                //dtmb 专用 ，暂时为使用...
                xml_node<> *returnNode = rootNode->first_node("ReturnInfo");
                if(returnNode!=NULL){
                    return true;//_parseSignalReportMsg(sIp,returnNode);
                    //信号上报不用回复xml
                }else{
                     createResponseMsg(nMsgId,11,CONST_STR_BOHUI_TYPE[BH_POTO_XmlContentError],responseBody);
                }
            }
        }
        else {
            //xml内容解析错误（SrcCode，MsgId等）
            createResponseMsg(nMsgId,11,CONST_STR_BOHUI_TYPE[BH_POTO_XmlContentError],responseBody);
        }
    }
    catch(...)
    {
        //cout<<strMsg<<endl;
        //xml载入异常，replyID=-100(暂定，待修改),vlaue=10(xml解析错误),
        createResponseMsg(nMsgId,10,CONST_STR_BOHUI_TYPE[BH_POTO_XmlParseError],responseBody);
    }

    return true;

}

bool Bohui_Protocol::_checkXmlHeader(xml_document<>  &xmlMsg,int &msgId,int &priority,string &desUrl,xml_node<> *rootNode)
{
    try
    {
        if(rootNode!=NULL)
        {
            string sVer;
            xml_attribute<char> *ver = rootNode->first_attribute("Version");
            //if(ver!=NULL)
            //    sVer = ver->value();
            msgId = strtol(rootNode->first_attribute("MsgID")->value(),NULL,10);
            // xml_attribute<char> *prity =  rootNode->first_attribute("Priority");
            // if(prity!=NULL)
            //    priority = strtol( rootNode->first_attribute("Priority")->value(),NULL,10);
            xml_attribute<char> *srcUrl =  rootNode->first_attribute("SrcURL");
            if(srcUrl!=NULL)
                 desUrl =  rootNode->first_attribute("SrcURL")->value();
            //if(!desUrl.empty())// && priority>=0
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

            if(strcmp(nCmdType,CONST_STR_BOHUI_TYPE[0])==0)
                _execTsmtQueryCmd(xml_responseMsg,msgRootNode,nValue);//查询发射机信息
            else if(strcmp(nCmdType,CONST_STR_BOHUI_TYPE[1])==0)
                _execEnvMonQueryCmd(xml_responseMsg,msgRootNode,nValue);//查询动环设备信息
            else if(strcmp(nCmdType,CONST_STR_BOHUI_TYPE[2])==0)
                _execLinkDevQueryCmd(xml_responseMsg,msgRootNode,nValue);//查询链路设备信息
            else if(strcmp(nCmdType,CONST_STR_BOHUI_TYPE[HX_POTO_LocalConfigQuery])==0){//服务器本地配置信息
                //_execGetLocalConfigQueryCmd(xml_responseMsg,msgRootNode,nValue);
                file<>   fdoc("ServerLocalConfig.xml");
                xml_document<> xml_doc;
                xml_doc.parse<0>(fdoc.data());
                rapidxml::print(std::back_inserter(responseBody), xml_doc, 0);
                return true;
            }

            xml_resps->append_attribute(xml_responseMsg.allocate_attribute("Value",xml_responseMsg.allocate_string(boost::lexical_cast<std::string>(nValue).c_str())));

        }
        else
            xml_resps->append_attribute(xml_responseMsg.allocate_attribute("Value",xml_responseMsg.allocate_string(boost::lexical_cast<std::string>(nValue).c_str())));


        xml_resps->append_attribute(xml_responseMsg.allocate_attribute("Desc",xml_responseMsg.allocate_string(CONST_STR_RESPONSE_VALUE_DESC(nValue).c_str())));
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
         xml_resps->append_attribute(xml_reportMsg.allocate_attribute("CmdStatus",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(devState).c_str())));
         xml_resps->append_attribute(xml_reportMsg.allocate_attribute("Time",xml_reportMsg.allocate_string(sTime.c_str())));
         xml_resps->append_attribute(xml_reportMsg.allocate_attribute("CmdStatusDesc",xml_reportMsg.allocate_string(sDesc.c_str())));
     }

    rapidxml::print(std::back_inserter(reportBody), xml_reportMsg, 0);
    return true;
}

//创建设备控制结果上报消息
bool Bohui_Protocol::creatDeviceExcutResultReportMsg(int nReplyId,int nCmdType,string sDevId,int devType,const string &sTime,
                                int devState,const string &sDesc,string &reportBody)
{
    xml_document<> xml_reportMsg;
    xml_node<> *msgRootNode = _createResponseXmlHeader(xml_reportMsg,2,-1);
    if(msgRootNode!=NULL){
        xml_node<> *xml_resps = xml_reportMsg.allocate_node(node_element,"DevControlLog");
        msgRootNode->append_node(xml_resps);

        xml_node<> *xml_dev_resps = xml_reportMsg.allocate_node(node_element,"Dev");
        xml_resps->append_node(xml_dev_resps);

        xml_dev_resps->append_attribute(xml_reportMsg.allocate_attribute("ID",xml_reportMsg.allocate_string(sDevId.c_str())));
        if(devType == 7)//切换器
            xml_dev_resps->append_attribute(xml_reportMsg.allocate_attribute("DevType",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(116).c_str())));
        else
            xml_dev_resps->append_attribute(xml_reportMsg.allocate_attribute("DevType",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(devType).c_str())));
        xml_dev_resps->append_attribute(xml_reportMsg.allocate_attribute("CmdStatus",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(devState).c_str())));
        xml_dev_resps->append_attribute(xml_reportMsg.allocate_attribute("Time",xml_reportMsg.allocate_string(sTime.c_str())));
        xml_dev_resps->append_attribute(xml_reportMsg.allocate_attribute("Desc",xml_reportMsg.allocate_string(sDesc.c_str())));
    }

   rapidxml::print(std::back_inserter(reportBody), xml_reportMsg, 0);
   return true;
}

//创建上报消息头（合并多个同属性设备）
bool Bohui_Protocol::createReportHeadMsg(xml_document<> &xmlMsg,xml_node<>* &pHeadMsg,int ReportType)
{
    //本地MsgId暂时固定为2
    xml_node<> *msgRootNode = _createResponseXmlHeader(xmlMsg,2,-1);
    if(msgRootNode!=NULL)
    {
        xml_node<> *xml_resps = xmlMsg.allocate_node(node_element,CONST_STR_BOHUI_TYPE[ReportType]);
        msgRootNode->append_node(xml_resps);
        pHeadMsg = xml_resps;

        return true;
    }
    return false;
}

//创建动环ups消息(合并多个ups设备)
bool Bohui_Protocol::appendUpsReportBodyMsg(xml_document<> &xmlMsg,map<string,xml_node<>*> &mXml_Quality,string sDevId,int nDevType
                                            ,DevMonitorDataPtr &curData,map<int,DeviceMonitorItem> &mapMonitorItem)
{
    //return true;
    xml_node<> *xml_Quality = NULL;
    string str_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
    xml_Quality = xmlMsg.allocate_node(node_element,"Dev");
    xml_Quality->append_attribute(xmlMsg.allocate_attribute("ID",xmlMsg.allocate_string(sDevId.c_str())));
    xml_Quality->append_attribute(xmlMsg.allocate_attribute("Type",xmlMsg.allocate_string(boost::lexical_cast<std::string>(nDevType).c_str())));
    string strDevtype = GetInst(StationConfig).get_dictionary_value("DeviceType",nDevType);
    xml_Quality->append_attribute(xmlMsg.allocate_attribute("Desc",xmlMsg.allocate_string(strDevtype.c_str())));
    xml_Quality->append_attribute(xmlMsg.allocate_attribute("CheckDateTime",xmlMsg.allocate_string(str_time.c_str())));

    mXml_Quality[sDevId] = xml_Quality;

    map<int,DeviceMonitorItem>::iterator cell_iter = mapMonitorItem.begin();
    for(;cell_iter!=mapMonitorItem.end();++cell_iter){
        if(cell_iter->second.bUpload == false)
           continue;
        if(mapTypeToStr.find(cell_iter->second.iTargetId) == mapTypeToStr.end())
            continue;
        if(mapTypeToStr[cell_iter->second.iTargetId].second.empty())
            continue;
        xml_node<> *xml_Quality_Index = NULL;
        xml_Quality_Index = xmlMsg.allocate_node(node_element,"Quality");
        xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Type",xmlMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iTargetId).c_str())));
        string  sValue = str(boost::format("%.2f")%curData->mValues[cell_iter->first].fValue);
        if(curData->mValues[cell_iter->first].bType==true)
            sValue = str(boost::format("%d")%curData->mValues[cell_iter->first].fValue);
        xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Value",xmlMsg.allocate_string(sValue.c_str())));//boost::lexical_cast<std::string>()
        xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Desc",mapTypeToStr[cell_iter->second.iTargetId].first.c_str()));
        xml_Quality->append_node(xml_Quality_Index);
    }
    return true;
}

//创建动环消息体（合并多个动环设备）
bool Bohui_Protocol::appendPowerEnvReportBodyMsg(xml_document<> &xmlMsg,map<int,xml_node<>*> &mXml_Quality,string sDevId,int nDevType
                                                 ,DevMonitorDataPtr &curData,map<int,DeviceMonitorItem> &mapMonitorItem)
{
    xml_node<> *xml_Quality = NULL;
    xml_node<> *xml_dev_node = NULL;

    map<int,xml_node<>*>::iterator iter = mXml_Quality.find(nDevType);

    switch (nDevType) {
    case DEVICE_ELEC:{//电力仪

        if(iter!=mXml_Quality.end())
            xml_Quality = iter->second;
        else{
            xml_Quality = xmlMsg.allocate_node(node_element,"ThreePhasePowerDev");
            mXml_Quality[nDevType] = xml_Quality;
        }

        xml_dev_node = xmlMsg.allocate_node(node_element,"ThreePhasePower");
        xml_dev_node->append_attribute(xmlMsg.allocate_attribute("ID",xmlMsg.allocate_string(sDevId.c_str())));
    }
        break;
    case DEVICE_TEMP:{//温湿度
        if(iter!=mXml_Quality.end())
            xml_Quality = iter->second;
        else{

            xml_Quality = xmlMsg.allocate_node(node_element,"TempHumidityDev");
            mXml_Quality[nDevType] = xml_Quality;
        }

        xml_dev_node = xmlMsg.allocate_node(node_element,"TempHumidity");
        xml_dev_node->append_attribute(xmlMsg.allocate_attribute("ID",xmlMsg.allocate_string(sDevId.c_str())));
    }
        break;
    case DEVICE_SMOKE://烟雾
    case DEVICE_WATER:{//水禁
        map<int,DeviceMonitorItem>::iterator cell_iter = mapMonitorItem.begin();
        for(;cell_iter!=mapMonitorItem.end();++cell_iter){

            if(cell_iter->second.bUpload == false)
               continue;
            if(mapTypeToStr.find(cell_iter->second.iTargetId) == mapTypeToStr.end())
                continue;
            if(mapTypeToStr[cell_iter->second.iTargetId].second.empty())
                continue;

            string sItemName = cell_iter->second.sItemName;
            //499对应“水”关键字
            string waterkey = mapTypeToStr[499].second;
            size_t nOffset = sItemName.find(waterkey.c_str(),0);
            if(nOffset != string::npos){
                map<int,xml_node<>*>::iterator iter = mXml_Quality.find(DEVICE_WATER);
                if(iter!=mXml_Quality.end())
                    xml_Quality = iter->second;
                else{
                    xml_Quality = xmlMsg.allocate_node(node_element,"WaterAlarmDev");//FireAlarmDev
                    mXml_Quality[DEVICE_WATER] = xml_Quality;
                }

                xml_dev_node = xmlMsg.allocate_node(node_element,"WaterAlarm");
                //string sIndexId = str(boost::format("%d")%cell_iter->first);
                string sIndexId = str(boost::format("%s-%d")%sDevId%cell_iter->first);
                xml_dev_node->append_attribute(xmlMsg.allocate_attribute("ID",xmlMsg.allocate_string(sIndexId.c_str())));
            }else {
                //500对应“火”关键字
                string firekey = mapTypeToStr[500].second;
                size_t nOffset = sItemName.find(firekey.c_str(),0);
                if(nOffset != string::npos){

                    map<int,xml_node<>*>::iterator iter = mXml_Quality.find(DEVICE_SMOKE);
                    if(iter!=mXml_Quality.end())
                        xml_Quality = iter->second;
                    else{
                        xml_Quality = xmlMsg.allocate_node(node_element,"FireAlarmDev");
                        mXml_Quality[DEVICE_SMOKE] = xml_Quality;
                    }

                    xml_dev_node = xmlMsg.allocate_node(node_element,"FireAlarm");

                    string sIndexId = str(boost::format("%s-%d")%sDevId%cell_iter->first);
                    xml_dev_node->append_attribute(xmlMsg.allocate_attribute("ID",xmlMsg.allocate_string(sIndexId.c_str())));

                }else
                    continue;

            }

            xml_Quality->append_node(xml_dev_node);

            xml_node<> *xml_Quality_Index = NULL;
            xml_Quality_Index = xmlMsg.allocate_node(node_element,mapTypeToStr[cell_iter->second.iTargetId].second.c_str());
            string  sValue ="0";
            if(curData->mValues[cell_iter->first].bType==true)
                sValue = str(boost::format("%d")%curData->mValues[cell_iter->first].fValue);
            xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Value",xmlMsg.allocate_string(sValue.c_str())));
            xml_dev_node ->append_node(xml_Quality_Index);

        }
    }
        return true;
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

         xml_node<> *xml_Quality_Index = NULL;
         xml_Quality_Index = xmlMsg.allocate_node(node_element,mapTypeToStr[cell_iter->second.iTargetId].second.c_str());

         string  sValue = str(boost::format("%.2f")%curData->mValues[cell_iter->first].fValue);
         if(curData->mValues[cell_iter->first].bType==true)
             sValue = str(boost::format("%d")%curData->mValues[cell_iter->first].fValue);

         xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Value",xmlMsg.allocate_string(sValue.c_str())));

         if(nDevType==DEVICE_ELEC || nDevType==DEVICE_TEMP)
             xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Unit",xmlMsg.allocate_string(cell_iter->second.sUnit.c_str())));//boost::lexical_cast<std::string>()

         if(nDevType!=DEVICE_SMOKE && nDevType!=DEVICE_WATER)
             xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Desc",mapTypeToStr[cell_iter->second.iTargetId].first.c_str()));

        xml_dev_node ->append_node(xml_Quality_Index);
     }

    return true;
}

//创建发射机上报消息体（合并多个发射机设备）
bool Bohui_Protocol::appendTransmitterReportBodyMsg(xml_document<> &xmlMsg,map<string,xml_node<>*> &mXml_Quality,
                                                    string sDevId,DevMonitorDataPtr &curData,map<int,DeviceMonitorItem> &mapMonitorItem)
{
     xml_node<> *xml_Quality = NULL;
     string str_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
     xml_Quality = xmlMsg.allocate_node(node_element,"Quality");
     xml_Quality->append_attribute(xmlMsg.allocate_attribute("TransmitterID",xmlMsg.allocate_string(sDevId.c_str())));
     xml_Quality->append_attribute(xmlMsg.allocate_attribute("CheckDateTime",xmlMsg.allocate_string(str_time.c_str())));

     mXml_Quality[sDevId] = xml_Quality;

     map<int,DeviceMonitorItem>::iterator cell_iter = mapMonitorItem.begin();
     for(;cell_iter!=mapMonitorItem.end();++cell_iter){

         if(cell_iter->second.bUpload == false)
            continue;
         xml_node<> *xml_Quality_Index = xmlMsg.allocate_node(node_element,"QualityIndex ");
         xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Type",xmlMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iTargetId).c_str())));
         xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("ModuleType",xmlMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iModTypeId).c_str())));
         xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("ModuleID",xmlMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iModDevId).c_str())));
         //适应博汇发射功率为W的需求
         float fCurValue = curData->mValues[cell_iter->first].fValue;
         string sCurValue = curData->mValues[cell_iter->first].sValue;
         if( cell_iter->first==0){
             fCurValue *=1000;
         }
         string  sValue = str(boost::format("%.2f")%fCurValue);
         if(curData->mValues[cell_iter->first].bType==true)
             sValue = str(boost::format("%d")%fCurValue);
         if(sCurValue.length()>0 && cell_iter->first == 501)
             xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Value",xmlMsg.allocate_string(sCurValue.c_str())));
         else
             xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Value",xmlMsg.allocate_string(sValue.c_str())));//boost::lexical_cast<std::string>()
         xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Desc",mapTypeToStr[cell_iter->second.iTargetId].first.c_str()));
         xml_Quality->append_node(xml_Quality_Index);

     }

     return true;
}


//创建切换器设备上报消息体(合并多个切换器)
bool Bohui_Protocol::appendSwtichReportBodyMsg(xml_document<> &xmlMsg,map<string,xml_node<>*> &mXml_Quality,string sDevId,
                                             int nDevType,DevMonitorDataPtr &curData,
                                             map<int,DeviceMonitorItem> &mapMonitorItem)
{
    xml_node<> *xml_SwitchDev = NULL;
    xml_SwitchDev = xmlMsg.allocate_node(node_element,"SwitchDevStatus");
    xml_SwitchDev->append_attribute(xmlMsg.allocate_attribute("DevID",xmlMsg.allocate_string(sDevId.c_str())));
    xml_SwitchDev->append_attribute(xmlMsg.allocate_attribute("DevName",xmlMsg.allocate_string(sDevId.c_str())));
    map<int,DeviceMonitorItem>::iterator cell_iter = mapMonitorItem.begin();
    for(;cell_iter!=mapMonitorItem.end();++cell_iter){

        if(cell_iter->second.bUpload == false)
           continue;
        if(mapTypeToStr.find(cell_iter->second.iTargetId) == mapTypeToStr.end())
            continue;
        if(mapTypeToStr[cell_iter->second.iTargetId].second.empty())
            continue;
        string curValueName = mapTypeToStr[cell_iter->second.iTargetId].second;
        float curValue = curData->mValues[cell_iter->first].fValue;
        if(curValueName == "Main" || curValueName == "Stby"|| curValueName == "Aux"){
            curValue = (curValue>0)?0:1;
        }
        string  sValue = str(boost::format("%d")%curValue);
        xml_SwitchDev->append_attribute(xmlMsg.allocate_attribute(xmlMsg.allocate_string(mapTypeToStr[cell_iter->second.iTargetId].second.c_str()),
                                         xmlMsg.allocate_string(sValue.c_str())));

    }
    mXml_Quality[sDevId] = xml_SwitchDev;
    return true;
}

//创建链路设备上报消息体(合并多个链路设备)
bool Bohui_Protocol::appendLinkReportBodyMsg(xml_document<> &xmlMsg,map<string,xml_node<>*> &mXml_Quality,string sDevId,
                                             int nDevType,DevMonitorDataPtr &curData,
                                             map<int,DeviceMonitorItem> &mapMonitorItem)
{
    xml_node<> *xml_dev_node = NULL;
    xml_dev_node = xmlMsg.allocate_node(node_element,"Dev");
    xml_dev_node->append_attribute(xmlMsg.allocate_attribute("Type",xmlMsg.allocate_string(boost::lexical_cast<std::string>(nDevType).c_str())));
    xml_dev_node->append_attribute(xmlMsg.allocate_attribute("ID",xmlMsg.allocate_string(sDevId.c_str())));
    string strDevtype = GetInst(StationConfig).get_dictionary_value("DeviceType",nDevType);
    xml_dev_node->append_attribute(xmlMsg.allocate_attribute("Desc",xmlMsg.allocate_string(strDevtype.c_str())));

    mXml_Quality[sDevId] = xml_dev_node;

    map<int,DeviceMonitorItem>::iterator cell_iter = mapMonitorItem.begin();
    for(;cell_iter!=mapMonitorItem.end();++cell_iter){

        if(cell_iter->second.bUpload == false)
           continue;
        if(mapTypeToStr.find(cell_iter->second.iTargetId) == mapTypeToStr.end())
            continue;
        if(mapTypeToStr[cell_iter->second.iTargetId].second.empty())
            continue;
        xml_node<> *xml_Quality_Index = NULL;
        xml_Quality_Index = xmlMsg.allocate_node(node_element,"Quality");
        xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Type",xmlMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iTargetId).c_str())));


        xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("QualitySrc",xmlMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iModTypeId).c_str())));
        xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("SrcIndex",xmlMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iModDevId).c_str())));
        string  sValue = str(boost::format("%.2f")%curData->mValues[cell_iter->first].fValue);
        float curValue= curData->mValues[cell_iter->first].fValue;
        if(curData->mValues[cell_iter->first].bType==true)
            sValue = str(boost::format("%d")%curValue);

        if(nDevType == 116){//博汇切换器类型专用
            if(cell_iter->second.iTargetId == 241)//捷成切换器专用
                sValue = str(boost::format("%d")%((curValue>0)?0:1));
            else if(cell_iter->second.iTargetId == 242)
                sValue = str(boost::format("%d")%curValue);
        }
        xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Value",xmlMsg.allocate_string(sValue.c_str())));
        if(nDevType == 116 && cell_iter->second.iTargetId == 242)
        {
            int curChannel = (int)curValue;
            if(curChannel == 0)
                xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Desc",xmlMsg.allocate_string(boost::lexical_cast<std::string>(mapTypeToStr[302].second).c_str())));
            else if(curChannel == 1)
                xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Desc",xmlMsg.allocate_string(boost::lexical_cast<std::string>(mapTypeToStr[303].second).c_str())));
            else if(curChannel == 2)
                xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Desc",xmlMsg.allocate_string(boost::lexical_cast<std::string>(mapTypeToStr[304].second).c_str())));
        }else
            xml_Quality_Index->append_attribute(xmlMsg.allocate_attribute("Desc",xmlMsg.allocate_string(boost::lexical_cast<std::string>(mapTypeToStr[cell_iter->second.iTargetId].first).c_str())));
        xml_dev_node->append_node(xml_Quality_Index);
    }

    return true;
}



//创建上报数据消息
bool Bohui_Protocol::createReportDataMsg(int nReplyId,string sDevId,int nDevType,DevMonitorDataPtr &curData,
                         map<int,DeviceMonitorItem> &mapMonitorItem,string &reportBody)
{
    xml_document<> xml_reportMsg;
    //本地MsgId暂时固定为2
    string str_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
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
            xml_Quality->append_attribute(xml_reportMsg.allocate_attribute("CheckDateTime",xml_reportMsg.allocate_string(str_time.c_str())));
            xml_resps->append_node(xml_Quality);

            map<int,DeviceMonitorItem>::iterator cell_iter = mapMonitorItem.begin();
            for(;cell_iter!=mapMonitorItem.end();++cell_iter){

                if(cell_iter->second.bUpload == false)
                   continue;
                xml_node<> *xml_Quality_Index = xml_reportMsg.allocate_node(node_element,"QualityIndex ");
                //string sType = boost::lexical_cast<std::string>(cell_iter->second.iTargetId);
                xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Type",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iTargetId).c_str())));
                xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("ModuleType",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iModTypeId).c_str())));
                xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("ModuleID",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iModDevId).c_str())));
                //适应博汇发射功率为W的需求
                float fCurValue = curData->mValues[cell_iter->first].fValue;
                string sCurValue = curData->mValues[cell_iter->first].sValue;
                if(cell_iter->second.sUnit == "W" && cell_iter->first==0){
                    fCurValue *=1000;
                }
                string  sValue = str(boost::format("%.2f")%fCurValue);
                if(curData->mValues[cell_iter->first].bType==true)
                    sValue = str(boost::format("%d")%fCurValue);
                if(sCurValue.length()>0 && cell_iter->first == 501)
                    xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Value",xml_reportMsg.allocate_string(sCurValue.c_str())));
                else
                    xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Value",xml_reportMsg.allocate_string(sValue.c_str())));//boost::lexical_cast<std::string>()
                xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Desc",mapTypeToStr[cell_iter->second.iTargetId].first.c_str()));
                xml_Quality->append_node(xml_Quality_Index);

            }


        }else {//动环链路数据上报//if(BH_POTO_EnvQualityRealtimeReport == nCmdType)
            xml_node<> *xml_dev_node = NULL;
            switch (nDevType) {
            case DEVICE_ELEC:{//电力仪
                xml_Quality = xml_reportMsg.allocate_node(node_element,"ThreePhasePowerDev");
                xml_resps->append_node(xml_Quality);
                xml_dev_node = xml_reportMsg.allocate_node(node_element,"ThreePhasePower");
                xml_dev_node->append_attribute(xml_reportMsg.allocate_attribute("ID",xml_reportMsg.allocate_string(sDevId.c_str())));
            }
                break;
            case DEVICE_TEMP:{//温湿度
                xml_Quality = xml_reportMsg.allocate_node(node_element,"TempHumidityDev");
                xml_resps->append_node(xml_Quality);
                xml_dev_node = xml_reportMsg.allocate_node(node_element,"TempHumidity");
                xml_dev_node->append_attribute(xml_reportMsg.allocate_attribute("ID",xml_reportMsg.allocate_string(sDevId.c_str())));
            }
                break;
            case DEVICE_SMOKE:{//烟雾
                xml_Quality = xml_reportMsg.allocate_node(node_element,"FireAlarmDev");
                xml_resps->append_node(xml_Quality);
                xml_dev_node = xml_reportMsg.allocate_node(node_element,"FireAlarm");
                xml_dev_node->append_attribute(xml_reportMsg.allocate_attribute("ID",xml_reportMsg.allocate_string(sDevId.c_str())));
            }
                break;
            case DEVICE_WATER:{//水禁
                xml_Quality = xml_reportMsg.allocate_node(node_element,"WaterAlarmDev");
                xml_resps->append_node(xml_Quality);
                xml_dev_node = xml_reportMsg.allocate_node(node_element,"WaterAlarm");
                xml_dev_node->append_attribute(xml_reportMsg.allocate_attribute("ID",xml_reportMsg.allocate_string(sDevId.c_str())));
            }
                break;
            case DEVICE_AIR://空调
            case DEVICE_GPS://GPS传感器
            case DEVICE_SWITCH://切换设备
            //case DEVICE_GPS_TIME://GPS授时器
                break;
            case DEVICE_GS_RECIVE://卫星接收机
            case DEVICE_MW://微波接收机
            case DEVICE_TR://光收发器
            case DEVICE_ENCODER://编码器
            case DEVICE_MUX://复用器
            case DEVICE_MO://调制器
            case DEVICE_ADAPTER:{//适配器
                //xml_Quality = xml_reportMsg.allocate_node(node_element,"SatelliteRecever");
                //xml_resps->append_node(xml_Quality);
                xml_dev_node = xml_reportMsg.allocate_node(node_element,"Dev");
                xml_dev_node->append_attribute(xml_reportMsg.allocate_attribute("Type",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(nDevType).c_str())));
                xml_dev_node->append_attribute(xml_reportMsg.allocate_attribute("ID",xml_reportMsg.allocate_string(sDevId.c_str())));
                string strDevtype = GetInst(StationConfig).get_dictionary_value("DeviceType",nDevType);
                xml_dev_node->append_attribute(xml_reportMsg.allocate_attribute("Desc",xml_reportMsg.allocate_string(strDevtype.c_str())));
                xml_Quality = xml_resps;//接口统一,将上级LinkDevQualityReport节点的指针保存到Quality中
            }break;
            case DEVICE_ANTENNA://同轴开关
                return false;
            }
            xml_Quality->append_node(xml_dev_node);

             map<int,DeviceMonitorItem>::iterator cell_iter = mapMonitorItem.begin();
             for(;cell_iter!=mapMonitorItem.end();++cell_iter){
                 if(mapTypeToStr.find(cell_iter->second.iTargetId) == mapTypeToStr.end())
                     continue;
                 if(mapTypeToStr[cell_iter->second.iTargetId].second.empty())
                     continue;


                 xml_node<> *xml_Quality_Index = NULL;
                 if(nDevType>DEVICE_UPS)
                     xml_Quality_Index = xml_reportMsg.allocate_node(node_element,"Quality");
                 else
                     xml_Quality_Index = xml_reportMsg.allocate_node(node_element,mapTypeToStr[cell_iter->second.iTargetId].second.c_str());
                 //烟感，水浸，配电设备，温湿度，没有Type属性
                 if(nDevType < DEVICE_ELEC || nDevType >= DEVICE_GPS)
                    xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Type",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iTargetId).c_str())));
                  if(nDevType>DEVICE_UPS){
                      //链路设备通道划分与发射机的模块及块内id共用配置字段
                      xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("QualitySrc",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iModTypeId).c_str())));
                      xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("SrcIndex",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(cell_iter->second.iModDevId).c_str())));
                  }

                 string  sValue = str(boost::format("%.2f")%curData->mValues[cell_iter->first].fValue);
                 if(curData->mValues[cell_iter->first].bType==true)
                     sValue = str(boost::format("%d")%curData->mValues[cell_iter->first].fValue);
                 if(nDevType==DEVICE_GS_RECIVE || nDevType==DEVICE_SMOKE || nDevType==DEVICE_WATER)
                     xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Value",xml_reportMsg.allocate_string(curData->mValues[cell_iter->first].sValue.c_str())));
                 else
                     xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Value",xml_reportMsg.allocate_string(sValue.c_str())));
                if(nDevType==DEVICE_ELEC || nDevType==DEVICE_TEMP)
                    xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Unit",xml_reportMsg.allocate_string(cell_iter->second.sUnit.c_str())));

                if(nDevType!=DEVICE_SMOKE && nDevType!=DEVICE_WATER)
                    xml_Quality_Index->append_attribute(xml_reportMsg.allocate_attribute("Desc",boost::lexical_cast<std::string>(mapTypeToStr[cell_iter->second.iTargetId].first).c_str()));


                xml_dev_node ->append_node(xml_Quality_Index);
             }
        }
    }

    rapidxml::print(std::back_inserter(reportBody), xml_reportMsg, 0);
    return true;
}

//创建上报告警消息
bool Bohui_Protocol::createReportAlarmDataMsg(int nReplyId,int nCmdType,string sDevId,CurItemAlarmInfo &alarmInfo,int nMod,
                                string &sReason,string &reportBody)//,int alarmLevel
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
            xml_Alarm =  xml_reportMsg.allocate_node(node_element,"Alarm");
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("DevID",xml_reportMsg.allocate_string(sDevId.c_str())));
            //AlarmSrc与ModuleType共用配置字段
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("AlarmSrc",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(alarmInfo.nModuleType).c_str())));
            //SrcIndex与ModuleID共用配置字段
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("SrcIndex",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(alarmInfo.nModuleId).c_str())));
            //暂时未实现..........
        }else if(BH_POTO_CommunicationReport == nCmdType){//设备通讯异常
            xml_Alarm = xml_reportMsg.allocate_node(node_element,"Communication");
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("TransmitterID",xml_reportMsg.allocate_string(sDevId.c_str())));
        }
        xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("AlarmID",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(alarmInfo.nAlarmId).c_str())));
        xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Mode",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(nMod).c_str())));
        xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Type",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(alarmInfo.nType).c_str())));
        xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("AlarmLevel",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(alarmInfo.alarmLevel).c_str())));
       if(BH_POTO_CommunicationReport == nCmdType)
           xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Desc",xml_reportMsg.allocate_string("发射机断开")));
       else
           xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Desc",xml_reportMsg.allocate_string(boost::lexical_cast<std::string>(mapTypeToStr[alarmInfo.nType].first).c_str())));
        if(sReason.empty()==false)
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Reason",xml_reportMsg.allocate_string(sReason.c_str())));

        string str_time = QDateTime::fromTime_t(alarmInfo.startTime).toString("yyyy-MM-dd hh:mm:ss").toStdString();
         if(BH_POTO_CommunicationReport == nCmdType)
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("Time",xml_reportMsg.allocate_string(str_time.c_str())));
         else
            xml_Alarm->append_attribute(xml_reportMsg.allocate_attribute("CheckTime",xml_reportMsg.allocate_string(str_time.c_str())));

        xml_resps->append_node(xml_Alarm);
    }

     rapidxml::print(std::back_inserter(reportBody), xml_reportMsg, 0);
     return true;
}

//生成xml头
//说明：xmlMsg--
xml_node<>*  Bohui_Protocol::_createResponseXmlHeader(xml_document<>  &xmlMsg,int nMsgId,int nReplyId,string sDstUrl)
{
    try
    {
        xml_node<>* rot = xmlMsg.allocate_node(rapidxml::node_pi,xmlMsg.allocate_string("xml version=\"1.0\" encoding=\"utf-8\""));//standalone='yes'
        xmlMsg.append_node(rot);
        xml_node<>* nodeHeader=   xmlMsg.allocate_node(node_element,"Msg");
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("Version","1.0"));
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("MsgID",xmlMsg.allocate_string(boost::lexical_cast<std::string>(nReplyId).c_str())));
        //string typ=nType>0?"Down":"Up";
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("Type","Up"));
        std::string createTM = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();

        nodeHeader->append_attribute(xmlMsg.allocate_attribute("DateTime",xmlMsg.allocate_string(createTM.c_str())));

        nodeHeader->append_attribute(xmlMsg.allocate_attribute("SrcCode",Bohui_Protocol::SrcCode.c_str()));
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("DstCode",Bohui_Protocol::DstCode.c_str()));
        //string sDstUrl = "http://10.10.0.200:8089";
        if(sDstUrl.empty()==false)
            nodeHeader->append_attribute(xmlMsg.allocate_attribute("SrcURL",xmlMsg.allocate_string(sDstUrl.c_str())));
        nodeHeader->append_attribute(xmlMsg.allocate_attribute("Priority","1"));
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

//查询服务器本地配置信息
void Bohui_Protocol::_execGetLocalConfigQueryCmd(xml_document<> &xml_doc,xml_node<> *rootNode,int  &nValue)
{
    nValue = 13;

    file<>   fdoc("ServerLocalConfig.xml");
    xml_doc.parse<0>(fdoc.data());


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
            if(temType>DEVICE_TRANSMITTER && temType<DEVICE_GS_RECIVE && temType!=DEVICE_SWITCH)
                temType = BH_POTO_EnvMonDevQuery;//动环设备
            else if(temType>=DEVICE_GS_RECIVE && temType<DEVICE_ANTENNA || temType==DEVICE_MEDIA ||temType==DEVICE_SWITCH)
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

                    map<string,DevProperty>::iterator attribut_iter = (*iter).second.map_DevProperty.find("Type");
                    if(attribut_iter!=(*iter).second.map_DevProperty.end())
                        xml_linkdev_list->append_attribute(xml_doc.allocate_attribute("Type",xml_doc.allocate_string(attribut_iter->second.property_value.c_str())));
                    else
                        xml_linkdev_list->append_attribute(xml_doc.allocate_attribute("Type",xml_doc.allocate_string(boost::lexical_cast<std::string>( (*iter).second.iDevType).c_str())));

                    xml_linkdev_list->append_attribute(xml_doc.allocate_attribute("Desc"," "));//链路设备名称暂略

                }
                xml_device = xml_doc.allocate_node(node_element,"Dev");
                xml_linkdev_list->append_node(xml_device);
                xml_device->append_attribute(xml_doc.allocate_attribute("DevID",(*iter).second.sDevNum.c_str()));
                xml_device->append_attribute(xml_doc.allocate_attribute("DevName",(*iter).second.sDevName.c_str()));


                //判断是否是多通道设备，添加通道节点map_DevChannelPropertyEx
                if((*iter).second.iChanSize>1 && (*iter).second.bMulChannel){

                    xml_node<> *xml_device_channel = xml_doc.allocate_node(node_element,"Channel");
                    xml_device->append_node(xml_device_channel);

                    xml_device_channel->append_attribute(xml_doc.allocate_attribute("id","0"));
                    map<string,DevProperty>::iterator iter_pro = (*iter).second.map_DevProperty.begin();
                    for(;iter_pro!=(*iter).second.map_DevProperty.end();++iter_pro){
                        xml_device_channel->append_attribute(xml_doc.allocate_attribute((*iter_pro).second.property_name.c_str(),(*iter_pro).second.property_value.c_str()));
                        if((*iter_pro).second.property_value_unit>=0)
                            xml_device_channel->append_attribute(xml_doc.allocate_attribute("unit",xml_doc.allocate_string(boost::lexical_cast<std::string>((*iter_pro).second.property_value_unit).c_str())));

                    }
                    map<int,map<string,DevProperty> >::iterator iter_pro_ex = (*iter).second.map_DevChannelPropertyEx.begin();
                    for(;iter_pro_ex!=(*iter).second.map_DevChannelPropertyEx.end();++iter_pro_ex){

                        xml_device_channel = xml_doc.allocate_node(node_element,"Channel");
                        xml_device->append_node(xml_device_channel);
                        xml_device_channel->append_attribute(xml_doc.allocate_attribute("id",xml_doc.allocate_string(boost::lexical_cast<std::string>(iter_pro_ex->first).c_str())));
                        map<string,DevProperty>::iterator iter_child_pro = (*iter_pro_ex).second.begin();
                        for(;iter_child_pro!=(*iter_pro_ex).second.end();++iter_child_pro){
                            xml_device_channel->append_attribute(xml_doc.allocate_attribute((*iter_child_pro).second.property_name.c_str(),(*iter_child_pro).second.property_value.c_str()));
                            if((*iter_child_pro).second.property_value_unit>=0)
                                xml_device_channel->append_attribute(xml_doc.allocate_attribute("unit",xml_doc.allocate_string(boost::lexical_cast<std::string>((*iter_child_pro).second.property_value_unit).c_str())));

                        }
                    }
                }
            }else{

                 if(nCmdType==BH_POTO_TransmitterQuery){

                     xml_device = xml_doc.allocate_node(node_element,DEVICE_TYPE_XML_DESC[nCmdType]);
                     xml_resps_info->append_node(xml_device);

                     xml_device->append_attribute(xml_doc.allocate_attribute("TransmitterID",(*iter).second.sDevNum.c_str()));
                     map<string,DevProperty>::iterator attribut_iter = (*iter).second.map_DevProperty.find("Type");
                     if(attribut_iter!=(*iter).second.map_DevProperty.end())
                         xml_device->append_attribute(xml_doc.allocate_attribute("Type",xml_doc.allocate_string(attribut_iter->second.property_value.c_str())));
                     else
                         xml_device->append_attribute(xml_doc.allocate_attribute("Type","3"));
                 }
                 else{
                     int nDevType = iter->second.iDevType;
                     //针对烟感水浸进行设备id拆分
                     if(nDevType == DEVICE_SMOKE || nDevType==DEVICE_WATER){

                         map<int,DeviceMonitorItem>::iterator cell_iter = iter->second.map_MonitorItem.begin();
                         for(;cell_iter!=iter->second.map_MonitorItem.end();++cell_iter){

                             if(mapTypeToStr.find(cell_iter->second.iTargetId) == mapTypeToStr.end())
                                 continue;
                             if(mapTypeToStr[cell_iter->second.iTargetId].second.empty())
                                 continue;

                             string sItemName = cell_iter->second.sItemName;
                             //499对应“水”关键字
                             string waterkey = mapTypeToStr[499].second.c_str();
                             size_t nOffset = sItemName.find(waterkey.c_str(),0);
                             if(nOffset != string::npos){

                                 xml_device = xml_doc.allocate_node(node_element,DEVICE_TYPE_XML_DESC[nCmdType]);
                                 xml_resps_info->append_node(xml_device);

                                 string sIndexId = str(boost::format("%s-%d")%(*iter).second.sDevNum%cell_iter->first);
                                 xml_device->append_attribute(xml_doc.allocate_attribute("ID",xml_doc.allocate_string(sIndexId.c_str())));
                                 xml_device->append_attribute(xml_doc.allocate_attribute("Type",xml_doc.allocate_string(boost::lexical_cast<std::string>(DEVICE_WATER).c_str())));
                                 xml_device->append_attribute(xml_doc.allocate_attribute("Name",xml_doc.allocate_string(sItemName.c_str())));
                                 //型号
                                 map<string,DevProperty>::iterator iter_propty =  (*iter).second.map_DevProperty.find("Model");
                                 if(iter_propty!= (*iter).second.map_DevProperty.end())
                                     xml_device->append_attribute(xml_doc.allocate_attribute("Model",(*iter_propty).second.property_value.c_str()));
                                 //制造商
                                 iter_propty =  (*iter).second.map_DevProperty.find("Manufacturer");
                                 if(iter_propty!= (*iter).second.map_DevProperty.end())
                                     xml_device->append_attribute(xml_doc.allocate_attribute("Manufacturer",(*iter_propty).second.property_value.c_str()));
                                 //设备描述
                                 iter_propty =  (*iter).second.map_DevProperty.find("Desc");
                                 if(iter_propty!= (*iter).second.map_DevProperty.end())
                                     xml_device->append_attribute(xml_doc.allocate_attribute("Desc",(*iter_propty).second.property_value.c_str()));

                             }else{
                                 string firekey = mapTypeToStr[500].second;
                                 size_t nOffset = sItemName.find(firekey.c_str(),0);
                                 if(nOffset != string::npos){

                                     //分配节点
                                     xml_device = xml_doc.allocate_node(node_element,DEVICE_TYPE_XML_DESC[nCmdType]);
                                     xml_resps_info->append_node(xml_device);


                                     string sIndexId = str(boost::format("%s-%d")%(*iter).second.sDevNum%cell_iter->first);
                                     xml_device->append_attribute(xml_doc.allocate_attribute("ID",xml_doc.allocate_string(sIndexId.c_str())));
                                     xml_device->append_attribute(xml_doc.allocate_attribute("Type",xml_doc.allocate_string(boost::lexical_cast<std::string>(DEVICE_SMOKE).c_str())));
                                     xml_device->append_attribute(xml_doc.allocate_attribute("Name",xml_doc.allocate_string(sItemName.c_str())));
                                     //型号
                                     map<string,DevProperty>::iterator iter_propty =  (*iter).second.map_DevProperty.find("Model");
                                     if(iter_propty!= (*iter).second.map_DevProperty.end())
                                         xml_device->append_attribute(xml_doc.allocate_attribute("Model",(*iter_propty).second.property_value.c_str()));
                                     //制造商
                                     iter_propty =  (*iter).second.map_DevProperty.find("Manufacturer");
                                     if(iter_propty!= (*iter).second.map_DevProperty.end())
                                         xml_device->append_attribute(xml_doc.allocate_attribute("Manufacturer",(*iter_propty).second.property_value.c_str()));
                                     //设备描述
                                     iter_propty =  (*iter).second.map_DevProperty.find("Desc");
                                     if(iter_propty!= (*iter).second.map_DevProperty.end())
                                         xml_device->append_attribute(xml_doc.allocate_attribute("Desc",(*iter_propty).second.property_value.c_str()));


                                 }else
                                     continue;
                             }
                         }

                        continue;

                     }else{

                         xml_device = xml_doc.allocate_node(node_element,DEVICE_TYPE_XML_DESC[nCmdType]);
                         xml_resps_info->append_node(xml_device);
                         xml_device->append_attribute(xml_doc.allocate_attribute("ID",(*iter).second.sDevNum.c_str()));
                         xml_device->append_attribute(xml_doc.allocate_attribute("Type",xml_doc.allocate_string(boost::lexical_cast<std::string>( (*iter).second.iDevType).c_str())));
                     }

                 }

                //链路设备名称暂略
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
            //生产厂家
            iter_propty =  (*iter).second.map_DevProperty.find("Factory");
            if(iter_propty!= (*iter).second.map_DevProperty.end())
                xml_device->append_attribute(xml_doc.allocate_attribute("Factory",(*iter_propty).second.property_value.c_str()));
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
    nValue = 11;
    map<string,vector<Monitoring_Scheduler> > mapSrcMontSch;
    int nRslt = _parse_alarm_run_time(rootNode,nValue,mapSrcMontSch);
    if(nRslt==BH_ERROR)
        return;
    if (GetInst(DataBaseOperation).SetAlarmTime(mapSrcMontSch,nValue)==true)
    {
        nValue=0;
        // 通知设备服务......
        map<string,vector<Monitoring_Scheduler> >::iterator iter = mapSrcMontSch.begin();
        for(;iter!=mapSrcMontSch.end();++iter){

            map<int,vector<Monitoring_Scheduler> > monitorScheduler;
            vector<Command_Scheduler> cmmdScheduler;

            if(GetInst(DataBaseOperation).GetUpdateDevTimeScheduleInfo(iter->first,monitorScheduler,cmmdScheduler)){
                GetInst(hx_net::SvcMgr).update_monitor_time(iter->first,monitorScheduler,cmmdScheduler);
            }
        }
    }
}

//分析运行图消息
int Bohui_Protocol::_parse_alarm_run_time(xml_node<> *root_node,int &nValue,map<string,vector<Monitoring_Scheduler> > &mapMonSch)
{
    nValue = 11;
    rapidxml::xml_node<>* tranNode = root_node->first_node("TranInfo");
    if(tranNode==NULL)
        return BH_NO_TSMT_NODE;

    for(;tranNode!=NULL;tranNode=tranNode->next_sibling()){
        vector<Monitoring_Scheduler> vecSch;
        rapidxml::xml_attribute<char> * attr = tranNode->first_attribute("TransmitterID");
        if(attr==NULL)
            continue;
        string qsTransNum = attr->value();
        rapidxml::xml_node<>* setnode = tranNode->first_node();
        if(setnode==NULL)
            return BH_ERROR;
        while(setnode){
            Monitoring_Scheduler tmSch;
            string name=setnode->name();
            if(name.empty()){
                setnode = setnode->next_sibling();
                continue;
            }
            if(name!="MonthTime" && name!="WeeklyTime" && name!="DayTime" )
                return BH_ERROR;
            int shutype=0;//按星期
            if(name=="MonthTime")
                shutype = 1;//按月
            else if(name=="DayTime")
                shutype = 2;//按天
            tmSch.iMonitorType = shutype;
            rapidxml::xml_attribute<char> * attrType = setnode->first_attribute("Type");
            if(attrType==NULL)
                return BH_ERROR;
            tmSch.bMonitorFlag = true;
            tmSch.bRunModeFlag = atoi(attrType->value());//监测标志0:停播
            //add by liukun for 2019-5-5 添加控制类型
            rapidxml::xml_attribute<char> * attrCtlType = setnode->first_attribute("CtlType");
            if(attrCtlType!=NULL){
                tmSch.iCtlType = atoi(attrCtlType->value());
            }

            //开始时间
            QDateTime qdt;
            rapidxml::xml_attribute<char> * attrStarttime = setnode->first_attribute("StartTime");
            if(attrStarttime==NULL){
                attrStarttime = setnode->first_attribute("StartDateTime");
                if(attrStarttime==NULL)
                    return BH_ERROR;
                else
                    qdt=QDateTime::fromString(attrStarttime->value(),"yyyy-MM-dd hh:mm:ss");
            }else{
                qdt=QDateTime::fromString(attrStarttime->value(),"hh:mm:ss");
                qdt.setDate(QDate(1980,1,1));
            }
            if(qdt.isValid())
                tmSch.tStartTime = qdt.toTime_t();
            else
                return BH_ERROR;
            //结束时间
            rapidxml::xml_attribute<char> * attrEndtime = setnode->first_attribute("EndTime");
            if(attrEndtime==NULL){
                attrEndtime = setnode->first_attribute("EndDateTime");
                if(attrEndtime==NULL)
                    return BH_ERROR;
                else
                    qdt=QDateTime::fromString(attrEndtime->value(),"yyyy-MM-dd hh:mm:ss");
            }else{
                qdt=QDateTime::fromString(attrEndtime->value(),"hh:mm:ss");
                qdt.setDate(QDate(1980,1,1));
            }
            if(qdt.isValid())
                tmSch.tEndTime = qdt.toTime_t();
            else
                return BH_ERROR;
             rapidxml::xml_attribute<char> * attrMotn = setnode->first_attribute("Month");
             if(attrMotn!=NULL)
                    tmSch.iMonitorMonth = atoi(attrMotn->value());
             rapidxml::xml_attribute<char> * attrDay = setnode->first_attribute("Day");
             if(attrDay!=NULL)
                    tmSch.iMonitorDay = atoi(attrDay->value());
             rapidxml::xml_attribute<char> * attrWeek = setnode->first_attribute("DayofWeek");
             if(attrWeek!=NULL)
                    tmSch.iMonitorWeek = atoi(attrWeek->value());
             rapidxml::xml_attribute<char> * attrend = setnode->first_attribute("AlarmEndTime");
             if(attrend!=NULL){
                    qdt=QDateTime::fromString(attrend->value(),"yyyy-MM-dd hh:mm:ss");
                    if(qdt.isValid())
                        tmSch.tAlarmEndTime = qdt.toTime_t();
                    else if(shutype!=2)
                        return BH_ERROR;
               }
             vecSch.push_back(tmSch);
             setnode = setnode->next_sibling();

        }
        mapMonSch[qsTransNum] = vecSch;
    }
    nValue = 0;
    return BH_NO_ERROR;
}

//设置告警门限
void Bohui_Protocol::_setAlarmParam(int nDevType,xml_node<> *rootNode,int &nValue)
{
    nValue = 11;
    map<string,vector<Alarm_config> > mapAlarmSet;
    if(_parse_alarm_param_set(rootNode,nValue,mapAlarmSet)==false){
        cout<<"_parse_alarm_param_set----error!"<<endl;
        return;
    }
   if (GetInst(DataBaseOperation).SetAlarmLimit(mapAlarmSet,nValue)==true)
    {
        nValue=0;
        // 通知设备服务......
        map<string,vector<Alarm_config> >::iterator iter = mapAlarmSet.begin();
        for(;iter!=mapAlarmSet.end();++iter){
            DeviceInfo devInfo;
            if(GetInst(DataBaseOperation).GetUpdateDevAlarmInfo(iter->first,devInfo)){
                GetInst(hx_net::SvcMgr).update_dev_alarm_config(iter->first,devInfo);
            }
        }
    }else
   {
       cout<<"SetAlarmLimit----error!---nValue---"<<nValue<<endl;
   }
}

//分析告警门限消息
bool Bohui_Protocol::_parse_alarm_param_set(xml_node<> *root_node,int &nValue,map<string,vector<Alarm_config> > &mapAlarmSet)
{
    nValue = 11;
    rapidxml::xml_node<>* tranNode = root_node->first_node("TranInfo");
    if(tranNode==NULL){
        tranNode = root_node->first_node("Dev");
        if(tranNode==NULL)
            return false;
    }

    for(;tranNode!=NULL;tranNode=tranNode->next_sibling())
    {

        rapidxml::xml_attribute<char> * attr = tranNode->first_attribute("TransmitterID");
        if(attr==NULL){
            attr = tranNode->first_attribute("ID");
            if(attr==NULL){
                continue;
            }
        }
        string qsTransNum = attr->value();

        //将烟感水浸设备分解出来
        size_t nOffset = qsTransNum.find("-",0);
        if(nOffset != string::npos){
            if(qsTransNum.length()>8)
                qsTransNum = qsTransNum.substr(0,8);
        }

        if(mapAlarmSet.find(qsTransNum) == mapAlarmSet.end()){
             vector<Alarm_config> vAlarmConf;
             mapAlarmSet[qsTransNum] = vAlarmConf;
        }


        rapidxml::xml_node<>* alswNode = tranNode->first_node("AlarmParam");
        if(alswNode == NULL)
            return false;
        for(;alswNode!=NULL;alswNode=alswNode->next_sibling()) {
            Alarm_config  curConf;
            rapidxml::xml_attribute<>* atType = alswNode->first_attribute("Type");
            if(atType!=NULL)   {
                int itype = atoi(atType->value());
                curConf.iAlarmid = itype;
                rapidxml::xml_attribute<>* atDuration=NULL;
                atDuration = alswNode->first_attribute("Duration");
                if(atDuration==NULL){
                    curConf.iDelaytime = 1;
                }else
                    curConf.iDelaytime = atoi(atDuration->value());
                atDuration = alswNode->first_attribute("ResumeDuration");
                if(atDuration==NULL)
                    curConf.iResumetime = 1;
                else
                    curConf.iResumetime = atoi(atDuration->value());
                int nFind = 0;
                rapidxml::xml_attribute<>* atTP = alswNode->first_attribute("DownThreshold");
                if(atTP!=NULL){

                    string slmt=atTP->value();
                    if(slmt.empty()==false){
                        curConf.iLimittype = 1;
                        curConf.fLimitvalue = atof(atTP->value());
                        mapAlarmSet[qsTransNum].push_back(curConf);
                    }
                }else{
                    nFind++;
                }
                atTP = alswNode->first_attribute("UpThreshold");
                if(atTP!=NULL){

                    string slmt=atTP->value();
                    if(slmt.empty()==false){
                        curConf.iLimittype = 0;
                        curConf.fLimitvalue = atof(atTP->value());
                        mapAlarmSet[qsTransNum].push_back(curConf);
                    }
                }else{
                    nFind++;
                }

                if(nFind==2)
                {
                    if(itype==(511) || itype==(512)|| itype==(513)||itype==(612)||itype==(633)||itype ==(634)||itype ==(703))
                    {
                        curConf.fLimitvalue =1;
                        curConf.iLimittype = 4;
                        //check is repeated
                        _checkAndAppendAlarmLimit(mapAlarmSet[qsTransNum],curConf);
                    }
                }
            }
        }
    }
    nValue = 0;
    return true;
}

//告警开关设置
bool Bohui_Protocol::_parse_alarm_switch_set(xml_node<> *root_node,int &nValue,map<string,vector<Alarm_Switch_Set> > &mapAlarmSwitchSet)
{
    nValue = 11;
    rapidxml::xml_node<>* tranNode = root_node->first_node("TranInfo");
    if(tranNode==NULL){
        tranNode = root_node->first_node("Dev");
        if(tranNode==NULL)
            return false;
    }
    while(tranNode!=NULL){
        //string ss = tranNode->value();
        //vector<Alarm_Switch_Set>  vAlarmSwich;
        rapidxml::xml_attribute<char> * attr = tranNode->first_attribute("TransmitterID");
        if(attr==NULL){
            attr = tranNode->first_attribute("ID");
            if(attr==NULL){
                 tranNode=tranNode->next_sibling();
                 continue;
            }
        }

        string qsTransNum = attr->value();

        //将烟感水浸设备分解出来
        size_t nOffset = qsTransNum.find("-",0);
        if(nOffset != string::npos){
            if(qsTransNum.length()>8)
                qsTransNum = qsTransNum.substr(0,8);
        }
        //cout<<"qsTransNum--substr="<<qsTransNum<<endl;
        if(mapAlarmSwitchSet.find(qsTransNum) == mapAlarmSwitchSet.end()){
             vector<Alarm_Switch_Set> vAlarmSwich;
             mapAlarmSwitchSet[qsTransNum] = vAlarmSwich;
        }
        rapidxml::xml_node<>* alswNode = tranNode->first_node("AlarmSwitch");
        for(;alswNode!=NULL;alswNode=alswNode->next_sibling())
        {
            Alarm_Switch_Set tmpConf;
            rapidxml::xml_attribute<>* atType = alswNode->first_attribute("Type");
            if(atType!=NULL) {
                tmpConf.iAlarmid = atoi(atType->value());
                rapidxml::xml_attribute<>* atSwitch = alswNode->first_attribute("Switch");
                if(atSwitch==NULL)
                    return false;
                tmpConf.iSwtich = atoi(atSwitch->value());
                rapidxml::xml_attribute<>* atDesc = alswNode->first_attribute("Desc");
                if(atDesc!=NULL)
                    tmpConf.sDes = atDesc->value();
                //vAlarmSwich.push_back(tmpConf);
                //mapAlarmSwitchSet[qsTransNum].push_back(tmpConf);

                _checkAndAppendAlarmSwitch(mapAlarmSwitchSet[qsTransNum],tmpConf);
            }
        }
        //mapAlarmSwitchSet[qsTransNum] = vAlarmSwich;

        cout<<"mapAlarmSwitchSet------id:"<<qsTransNum<<"size:"<<mapAlarmSwitchSet[qsTransNum].size();
        tranNode=tranNode->next_sibling();
    }

    nValue = 0;
    return true;
}

//检查是否需要加入到告警
void Bohui_Protocol::_checkAndAppendAlarmSwitch(vector<Alarm_Switch_Set> &vConfig,Alarm_Switch_Set &curConfig)
{
    vector<Alarm_Switch_Set>::iterator iter = vConfig.begin();
    //bool bfind = false;
    for(;iter!=vConfig.end();++iter){
        if((*iter).iAlarmid == curConfig.iAlarmid)
            return;
    }
    vConfig.push_back(curConfig);
}

//检查后添加告警门限(只过滤烟感水禁)
void Bohui_Protocol::_checkAndAppendAlarmLimit(vector<Alarm_config> &vConfig, Alarm_config &curConfig)
{
    vector<Alarm_config>::iterator iter = vConfig.begin();
    //bool bfind = false;
    for(;iter!=vConfig.end();++iter){
        if((*iter).iAlarmid == curConfig.iAlarmid)
            return;
    }
    vConfig.push_back(curConfig);
}



//分析告警开关消息
void Bohui_Protocol::_setAlarmSwitchSetParam(int nDevType,xml_node<> *rootNode,int &nValue)
{
    nValue = 11;
    map<string,vector<Alarm_Switch_Set> > mapAlarmSwtichSet;
    if(_parse_alarm_switch_set(rootNode,nValue,mapAlarmSwtichSet)==false){
        cout<<"_parse_alarm_switch_set----error!"<<endl;
        return;
    }

    cout<<"_parse_alarm_switch_set-----ok!"<<endl;
    if (GetInst(DataBaseOperation).SetEnableAlarm(mapAlarmSwtichSet,nValue)==true)//nDevType,
    {
        nValue =0;
        // 通知设备服务......
        map<string,vector<Alarm_Switch_Set> >::iterator iter = mapAlarmSwtichSet.begin();
       for(;iter!=mapAlarmSwtichSet.end();++iter){
            DeviceInfo devInfo;
            if(GetInst(DataBaseOperation).GetUpdateDevAlarmInfo(iter->first,devInfo)){
                GetInst(hx_net::SvcMgr).update_dev_alarm_config(iter->first,devInfo);
            }
        }
    }else
         cout<<"写入数据库－－－－error"<<endl;
}

//手动控制(发射机)
void Bohui_Protocol::_controlDeviceCommand(int nDevType,xml_node<> *rootNode,int &nValue)
{
    rapidxml::xml_node<>* tranNode = rootNode->first_node("TranInfo");
    if(tranNode!=NULL){
        string sDevId;
        int  nSwitch;
        if(nDevType == BH_POTO_ManualPowerControl){
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
            map<int,string> mapParam;//博汇指令暂不携带控制参数
            if(nSwitch == 0)//关机
                GetInst(hx_net::SvcMgr).start_exec_task(sDevId,Bohui_Protocol::DstCode,MSG_TRANSMITTER_TURNOFF_OPR,mapParam);
            else if(nSwitch == 1)//开机
                GetInst(hx_net::SvcMgr).start_exec_task(sDevId,Bohui_Protocol::DstCode,MSG_TRANSMITTER_TURNON_OPR,mapParam);

        }else if(nDevType == BH_POTO_ManualTrunControl){
            rapidxml::xml_attribute<char> * attr_devId = tranNode->first_attribute("OnTransmitterID");
            if(attr_devId==NULL){
                nValue = 11;
                return;
            }
            map<int,string> mapParam;//博汇指令暂不携带控制参数
            sDevId = attr_devId->value();
            GetInst(hx_net::SvcMgr).start_exec_task(sDevId,Bohui_Protocol::DstCode,MSG_TRANSMITTER_TURNON_OPR,mapParam);
        }

    }
}
//切换器控制
void Bohui_Protocol::_controlSwitchCommand(int nDevType,xml_node<> *rootNode,int &nValue)
{
#if 0
   rapidxml::xml_node<>* switchDevNode = rootNode->first_node("SwitchDev");
   for(;switchDevNode!=NULL;switchDevNode=switchDevNode->next_sibling())
   {
       rapidxml::xml_attribute<char> * attr_devId = switchDevNode->first_attribute("DevID");
       if(attr_devId==NULL){
           nValue = 11;
           continue;
       }

       rapidxml::xml_attribute<char> * attr_switch = switchDevNode->first_attribute("SwitchTo");
       if(attr_switch==NULL){
           nValue = 11;
           return;
       }
       string sDevId = attr_devId->value();
       devCommdMsgPtr commandmsg_(new DeviceCommandMsg);
       commandmsg_->set_sdevid(sDevId);
       CommandParam* parma = commandmsg_->add_cparams();
       parma->set_sparamvalue(attr_switch->value());
       GetInst(SvcMgr).excute_command(sDevId,MSG_0401_SWITCH_OPR,Bohui_Protocol::DstCode,commandmsg_);
   }
#endif

    rapidxml::xml_node<>* devNode = rootNode->first_node("Dev");
    while(devNode!=NULL){

        rapidxml::xml_attribute<char> * attr_DevType= devNode->first_attribute("Type");
        rapidxml::xml_attribute<char> * attr_DevID= devNode->first_attribute("ID");

        rapidxml::xml_node<> *controlNode = devNode->first_node("Control");
        if(attr_DevType!=NULL && attr_DevID!=NULL){
            string devType = attr_DevType->value();
            if(devType == "116")//切换器类型
            {
                while(controlNode!=NULL){

                    rapidxml::xml_attribute<char> * attr_ControlType = controlNode->first_attribute("Type");
                    rapidxml::xml_attribute<char> * attr_SwitchToValue = controlNode->first_attribute("Value");
                    if(attr_ControlType!=NULL && attr_SwitchToValue!=NULL){
                        string ctrlType = attr_ControlType->value();
                        if(ctrlType == "1111")//信号切换类型
                        {
                            string sDevId = attr_DevID->value();
                            devCommdMsgPtr commandmsg_(new DeviceCommandMsg);
                            commandmsg_->set_sdevid(sDevId);
                            CommandParam* parma = commandmsg_->add_cparams();
                            int curValue = atoi(attr_SwitchToValue->value());
                            string  sValue = str(boost::format("%d")%curValue);
                            parma->set_sparamvalue(sValue.c_str());
                            GetInst(SvcMgr).excute_command(sDevId,MSG_0401_SWITCH_OPR,Bohui_Protocol::DstCode,commandmsg_);
                        }
                    }else{
                        nValue = 11;
                        return;
                    }

                    controlNode = controlNode->next_sibling("Control");
                }
            }


        }

        devNode = devNode->next_sibling("Dev");
    }
}

#if 0
//分析信号上报告警,数据上报（DTMB相关）
bool Bohui_Protocol::_parseSignalReportMsg(string sIp,xml_node<> * InfoNode)
{

    rapidxml::xml_node<>* dataNode = InfoNode->first_node("AlarmSearchFSet");
    if(dataNode!=NULL){
        rapidxml::xml_attribute<char> * attr_FreqId = dataNode->first_attribute("Freq");
        if(attr_FreqId==NULL)
            return false;
        string sPrgFreq = attr_FreqId->value();
        if(sPrgFreq.empty())
            return false;
        rapidxml::xml_node<> *itemNode = dataNode->first_node("AlarmSearchF");
        while(itemNode!=NULL){
            rapidxml::xml_attribute<char> * attr_prgType = itemNode->first_attribute("Type");
            rapidxml::xml_attribute<char> * attr_prgValue = itemNode->first_attribute("Value");
            rapidxml::xml_attribute<char> * attr_prgTime = itemNode->first_attribute("Time");
            if(attr_prgType==NULL || attr_prgValue==NULL || attr_prgTime==NULL)
                continue;
            int nTypeId = strtol(attr_prgType->value(),NULL,10);
            int nState = strtol(attr_prgValue->value(),NULL,10);
            time_t  curTm;
            QDateTime qdt=QDateTime::fromString(attr_prgTime->value(),"yyyy-MM-dd hh:mm:ss");
            if(qdt.isValid())
                curTm = qdt.toTime_t();

            request_handler_factory::get_mutable_instance().add_new_alarm(sIp,sPrgFreq,nTypeId,nState,curTm);
            itemNode = itemNode->next_sibling("AlarmSearchF");
        }
    }else {
        dataNode = InfoNode->first_node("GetIndexSet");
        if(dataNode!=NULL){
            rapidxml::xml_attribute<char> * attr_FreqId = dataNode->first_attribute("Freq");
            rapidxml::xml_attribute<char> * attr_ChannelId = dataNode->first_attribute("Index");
            if(attr_FreqId==NULL || attr_ChannelId==NULL)
                return false;
            string sPrgFreq = attr_FreqId->value();
            int nChannelId = atoi(attr_ChannelId->value());
            if(sPrgFreq.empty() || nChannelId<0 || nChannelId>100)
                return false;

            rapidxml::xml_node<> *itemNode = dataNode->first_node("GetIndex");
            DevMonitorDataPtr  curValues(new Data);
            DataInfo curFrqInfo;
            curFrqInfo.bType=0;
            curFrqInfo.sValue = sPrgFreq;
            curFrqInfo.fValue = 0.0f;
            curValues->mValues[0+5*nChannelId] = curFrqInfo;
             while(itemNode!=NULL){
                 DataInfo curItemInfo;
                 curItemInfo.bType=0;
                 curItemInfo.fValue = 0.0f;
                 rapidxml::xml_attribute<char> * attr_prgType = itemNode->first_attribute("Type");
                 if(attr_prgType!=NULL){
                     int nTypeId = atoi(attr_prgType->value());
                     if(nTypeId>=0 && nTypeId<5){
                         rapidxml::xml_attribute<char> * attr_prgType = itemNode->first_attribute("Value");
                         if(attr_prgType!=NULL){
                              curItemInfo.sValue = attr_prgType->value();
                              curValues->mValues[nTypeId+5*nChannelId] = curItemInfo;
                         }
                     }
                 }

                 itemNode = itemNode->next_sibling("GetIndex");
             }

             if(curValues->mValues.size()>0)
                  request_handler_factory::get_mutable_instance().add_new_data(sIp,nChannelId,curValues);
        }
    }
   // }
    return true;
}
//创建dtmb节目信息（DTMB相关）
bool Bohui_Protocol::creatQueryDtmbPrgInfoMsg(string &reportBody)
{
     std::string  srcUrl = str(boost::format("http://%1%:%2%")%GetInst(LocalConfig).http_svc_ip()%GetInst(LocalConfig).http_svc_port());
     xml_document<> xml_reportMsg;
     xml_node<> *msgRootNode = _createResponseXmlHeader(xml_reportMsg,2,-1,srcUrl);
     if(msgRootNode!=NULL){
         xml_node<> *xml_resps = xml_reportMsg.allocate_node(node_element,"ChannelScanQuery");
         msgRootNode->append_node(xml_resps);
     }

    rapidxml::print(std::back_inserter(reportBody), xml_reportMsg, 0);
    return true;
}
#endif
