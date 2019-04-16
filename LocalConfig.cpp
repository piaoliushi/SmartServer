#include "LocalConfig.h"
#include "utility.h"
#include "./rapidxml/rapidxml.hpp"
#include "./rapidxml/rapidxml_utils.hpp"
#include "./rapidxml/rapidxml_print.hpp"
#include "MsgDefine.h"
#include <string.h>
#include <iostream>
using namespace rapidxml;

LocalConfig::LocalConfig(void)
    :db_ip_("127.0.0.1")
    ,db_usr_("sa")
    ,db_psw_("1234")
    ,db_driver_("SQL Native Client")
   // ,sms_use_(false)
    //,sms_baud_rate_(9600)
    //,sms_center_number_("13888888888")
    ,local_port_(5000)
    ,report_svc_url_("")
    ,report_use_(false)
    ,report_span_(10)
    ,http_upload_use_(false)
    ,ntp_upload_use_(false)
    ,ntp_mod_(1)
    ,ntp_mod_value_(2)
    ,ntp_time_("1900-1-1 15:01:01")
{
    //load_local_config("config.xml");
}

LocalConfig::~LocalConfig(void)
{
}


bool LocalConfig::load_local_config(const char* sFileName)
{
    file<>   fdoc(sFileName);
    xml_document<>   xml_config;
    xml_config.parse<0>(fdoc.data());
    xml_node<>* root = xml_config.first_node("service");
    if(root!=NULL)
    {
        xml_node<>* xml_station = root->first_node("station");
        if(xml_station!=NULL)
        {
            local_station_id_ = xml_station->first_attribute("id")->value();
            local_station_name_ = xml_station->first_attribute("name")->value();
            local_dev_server_id_ = xml_station->first_attribute("dev_server_id")->value();
            src_code_ = xml_station->first_attribute("src_code")->value();
            dst_code_ = xml_station->first_attribute("dst_code")->value();
        }
        else
            return false;

        xml_node<>* xml_database = root->first_node("database");
        if(xml_database!=NULL)
        {
            db_ip_ = xml_database->first_attribute("ip")->value();
            db_usr_ = xml_database->first_attribute("user")->value();
            db_psw_ = xml_database->first_attribute("password")->value();
        }
        else
            return false;

        xml_node<>* xml_server = root->first_node("server" );
        if(xml_server!=NULL)
        {
            local_port_ = strtol(xml_server->first_attribute("port")->value(),NULL,10);
            xml_node<>* xml_hb = xml_server->first_node("heart_beat");
            if(xml_hb!=NULL)
            {
                hb_interval_ = strtol(xml_hb->first_attribute("interval")->value(),NULL,10);
                hb_timeout_ = strtol(xml_hb->first_attribute("timeout")->value(),NULL,10);
            }
            else
                return false;
        }
        else
            return false;

        //检查是否有数据上传服务配置
        xml_node<>* xml_upload_svc_root = root->first_node("upload_svc_config" );
        if(xml_upload_svc_root!=NULL)
        {
            //级联链接配置
            xml_node<>* xml_report_svc = xml_upload_svc_root->first_node("report_svc");
            if(xml_report_svc!=NULL)
            {
                long nUse =  strtol(xml_report_svc->first_attribute("use")->value(),NULL,10);
                if(nUse>0)
                    report_use_=true;
                else
                    report_use_=false;
                report_svc_url_ = xml_report_svc->first_attribute("url")->value();
                report_span_ = strtol(xml_report_svc->first_attribute("span")->value(),NULL,10);
            }
            //本地http服务配置
            xml_node<>* xml_http_svc = xml_upload_svc_root->first_node("http_svc");
            if(xml_http_svc!=0)
            {
                long nUse =  strtol(xml_http_svc->first_attribute("use")->value(),NULL,10);
                if(nUse>0)
                    http_upload_use_=true;
                else
                    http_upload_use_=false;
                http_svc_ip_ = xml_http_svc->first_attribute("ip")->value();
                http_svc_port_ = xml_http_svc->first_attribute("port")->value();
            }
            //ntp服务配置
            xml_node<>* xml_ntp_svc = xml_upload_svc_root->first_node("ntp_svc");
            if(xml_ntp_svc!=0){
                long nUse =  strtol(xml_ntp_svc->first_attribute("use")->value(),NULL,10);
                if(nUse>0)
                    ntp_upload_use_=true;
                else
                    ntp_upload_use_=false;
                ntp_svc_ip_ = xml_ntp_svc->first_attribute("ip")->value();
                ntp_mod_ = strtol(xml_ntp_svc->first_attribute("mod")->value(),NULL,10);
                ntp_mod_value_ = strtol(xml_ntp_svc->first_attribute("mod_value")->value(),NULL,10);
                ntp_time_ = xml_ntp_svc->first_attribute("time")->value();
            }
        }
        //串口服务器配置
        xml_node<>* xml_moxa = root->first_node("moxa_modle");
        while(xml_moxa!=0)
        {
            string moxaId = xml_moxa->first_attribute("user")->value();
            if(!moxaId.empty())
            {
                pMoxaPropertyExPtr tmpPropertyEx= pMoxaPropertyExPtr(new MoxaPropertyEx);
                //查询超时次数
                xml_node<>* xml_property = xml_moxa->first_node("query_timeout_count");
                if(xml_property)
                    tmpPropertyEx->query_timeout_count = strtol(xml_property->value(),NULL,10);
                //查询指令间隔
                xml_property = xml_moxa->first_node("query_interval");
                if(xml_property)
                    tmpPropertyEx->query_interval = strtol(xml_property->value(),NULL,10);
                //连接定时器间隔（自动重连机制）
                xml_property = xml_moxa->first_node("connect_timer_interval");
                if(xml_property)
                    tmpPropertyEx->connect_timer_interval = strtol(xml_property->value(),NULL,10);
                //网络连接超时时长
                xml_property = xml_moxa->first_node("connect_timeout_duration");
                if(xml_property)
                    tmpPropertyEx->connect_timeout_duration = strtol(xml_property->value(),NULL,10);
                moxas_property_Ex_[moxaId]=tmpPropertyEx;
            }
            xml_moxa=xml_moxa->next_sibling("moxa_modle");
        }
        //设备属性配置
        xml_node<>* xml_device = root->first_node("device");
        while(xml_device!=0)
        {
            string devId = xml_device->first_attribute("id")->value();
            if(!devId.empty())
            {
                pDevicePropertyExPtr tmpPropertyEx= pDevicePropertyExPtr(new DevicePropertyEx);
                //查询超时次数
                xml_node<>* xml_property = xml_device->first_node("query_timeout_count");
                if(xml_property)
                    tmpPropertyEx->query_timeout_count = strtol(xml_property->value(),NULL,10);
                //查询指令间隔
                xml_property = xml_device->first_node("query_interval");
                if(xml_property)
                    tmpPropertyEx->query_interval = strtol(xml_property->value(),NULL,10);
                //连接定时器间隔（自动重连机制）
                xml_property = xml_device->first_node("connect_timer_interval");
                if(xml_property)
                    tmpPropertyEx->connect_timer_interval = strtol(xml_property->value(),NULL,10);
                //网络连接超时时长
                xml_property = xml_device->first_node("connect_timeout_duration");
                if(xml_property)
                    tmpPropertyEx->connect_timeout_duration = strtol(xml_property->value(),NULL,10);
                //开机最大检测次数
                xml_property = xml_device->first_node("run_detect_max_count");
                if(xml_property)
                    tmpPropertyEx->run_detect_max_count = strtol(xml_property->value(),NULL,10);
                //关机最大检测次数
                xml_property = xml_device->first_node("shutdown_detect_max_count");
                if(xml_property)
                    tmpPropertyEx->shutdown_detect_max_count = strtol(xml_property->value(),NULL,10);
                //数据保存间隔
                xml_property = xml_device->first_node("data_save_interval");
                if(xml_property)
                    tmpPropertyEx->data_save_interval = strtol(xml_property->value(),NULL,10);
                //报警检测最大次数
                xml_property = xml_device->first_node("alarm_detect_max_count");
                if(xml_property)
                    tmpPropertyEx->alarm_detect_max_count = strtol(xml_property->value(),NULL,10);
                //命令超时间隔
                xml_property = xml_device->first_node("cmd_timeout_interval");
                if(xml_property)
                    tmpPropertyEx->cmd_timeout_interval = strtol(xml_property->value(),NULL,10);
                //命令执行超时时长
                xml_property = xml_device->first_node("cmd_excut_timeout_duration");
                if(xml_property)
                    tmpPropertyEx->cmd_excut_timeout_duration = strtol(xml_property->value(),NULL,10);
                //多查询指令发送间隔
                xml_property = xml_device->first_node("multi_query_send_interval");
                if(xml_property)
                    tmpPropertyEx->multi_query_send_interval = strtol(xml_property->value(),NULL,10);
                //功率零值设定
                xml_property = xml_device->first_node("zero_power_value");
                if(xml_property)
                    tmpPropertyEx->zero_power_value = strtod(xml_property->value(),NULL);
                //关机判定监控量序号 add by ws 2019-04-08
                xml_property = xml_device->first_node("check_run_index");
                if(xml_property)
                    tmpPropertyEx->iCheckRunIndex = atoi(xml_property->value());
                //9033A电压矫正系数
                xml_property = xml_device->first_node("ubb_ratio_value");
                if(xml_property)
                    tmpPropertyEx->ubb_ratio_value = strtod(xml_property->value(),NULL);
                //9033A电流矫正系数
                xml_property = xml_device->first_node("ibb_ratio_value");
                if(xml_property)
                    tmpPropertyEx->ibb_ratio_value = strtod(xml_property->value(),NULL);
                //9033A电流量程范围
                xml_property = xml_device->first_node("i0_range_value");
                if(xml_property)
                    tmpPropertyEx->i0_range_value = strtol(xml_property->value(),NULL,10);
                //9033A电压量程范围
                xml_property = xml_device->first_node("u0_range_value");
                if(xml_property)
                    tmpPropertyEx->u0_range_value = strtol(xml_property->value(),NULL,10);

                xml_property = xml_device->first_node("query_command_define");
                CommandAttribute cmd;
                if(xml_property)
                {
                    xml_node<>* xml_ack_len_property = xml_property->first_node("query_command");
                    vector<CommandUnit> vCmd;
                    while(xml_ack_len_property)
                    {
                        int nid = strtol(xml_ack_len_property->first_attribute("id")->value(),NULL,10);
                        int nackLen = strtol(xml_ack_len_property->first_attribute("ack_len")->value(),NULL,10);
                        CommandUnit tmUnit;
                        if(nid>=0 && nackLen>0)
                            tmUnit.ackLen = nackLen;
                        std::string qcmd = xml_ack_len_property->first_attribute("command")->value();
                        tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);

                        vCmd.push_back(tmUnit);
                        xml_ack_len_property=xml_ack_len_property->next_sibling("query_command");
                    }
                   // if(vCmd.size()>0)
                        cmd.mapCommand[MSG_DEVICE_QUERY] = vCmd;
                }
                xml_property = xml_device->first_node("turnon_command_define");
                if(xml_property)
                {
                    xml_node<>* xml_ack_len_property = xml_property->first_node("turnon_command");
                     vector<CommandUnit> vCmd;
                    while(xml_ack_len_property)
                    {
                        int nid = strtol(xml_ack_len_property->first_attribute("id")->value(),NULL,10);
                        int nackLen = strtol(xml_ack_len_property->first_attribute("ack_len")->value(),NULL,10);
                        CommandUnit tmUnit;
                        if(nid>=0 && nackLen>0)
                            tmUnit.ackLen = nackLen;
                        std::string qcmd = xml_ack_len_property->first_attribute("command")->value();
                        tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
                        vCmd.push_back(tmUnit);
                        xml_ack_len_property=xml_ack_len_property->next_sibling("turnon_command");
                    }

                   // if(vCmd.size()>0)
                        cmd.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vCmd;
                }
                xml_property = xml_device->first_node("turnoff_command_define");
                if(xml_property)
                {
                    xml_node<>* xml_ack_len_property = xml_property->first_node("turnoff_command");
                     vector<CommandUnit> vCmd;
                    while(xml_ack_len_property)
                    {
                        int nid = strtol(xml_ack_len_property->first_attribute("id")->value(),NULL,10);
                        int nackLen = strtol(xml_ack_len_property->first_attribute("ack_len")->value(),NULL,10);
                        CommandUnit tmUnit;
                        if(nid>=0 && nackLen>0)
                            tmUnit.ackLen = nackLen;
                        std::string qcmd = xml_ack_len_property->first_attribute("command")->value();
                        tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
                        vCmd.push_back(tmUnit);
                        xml_ack_len_property=xml_ack_len_property->next_sibling("turnoff_command");
                    }

                    //if(vCmd.size()>0)
                        cmd.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vCmd;
                }
                xml_property = xml_device->first_node("up_command_define");
                if(xml_property)
                {
                    xml_node<>* xml_ack_len_property = xml_property->first_node("up_command");
                      vector<CommandUnit> vCmd;
                    while(xml_ack_len_property)
                    {
                        int nid = strtol(xml_ack_len_property->first_attribute("id")->value(),NULL,10);
                        int nackLen = strtol(xml_ack_len_property->first_attribute("ack_len")->value(),NULL,10);
                        CommandUnit tmUnit;
                        if(nid>=0 && nackLen>0)
                            tmUnit.ackLen = nackLen;
                        std::string qcmd = xml_ack_len_property->first_attribute("command")->value();
                        tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
                         vCmd.push_back(tmUnit);
                        xml_ack_len_property=xml_ack_len_property->next_sibling("up_command");
                    }

                   // if(vCmd.size()>0)
                        cmd.mapCommand[MSG_TRANSMITTER_RISE_POWER_OPR] = vCmd;
                }
                xml_property = xml_device->first_node("dw_command_define");
                if(xml_property)
                {
                    xml_node<>* xml_ack_len_property = xml_property->first_node("dw_command");
                      vector<CommandUnit> vCmd;
                    while(xml_ack_len_property)
                    {
                        int nid = strtol(xml_ack_len_property->first_attribute("id")->value(),NULL,10);
                        int nackLen = strtol(xml_ack_len_property->first_attribute("ack_len")->value(),NULL,10);
                        CommandUnit tmUnit;
                        if(nid>=0 && nackLen>0)
                            tmUnit.ackLen = nackLen;
                        std::string qcmd = xml_ack_len_property->first_attribute("command")->value();
                        tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
                         vCmd.push_back(tmUnit);
                        xml_ack_len_property=xml_ack_len_property->next_sibling("dw_command");
                    }
                   // if(vCmd.size()>0)
                        cmd.mapCommand[MSG_TRANSMITTER_REDUCE_POWER_OPR] = vCmd;
                }
                xml_property = xml_device->first_node("reset_command_define");
                if(xml_property)
                {
                    xml_node<>* xml_ack_len_property = xml_property->first_node("reset_command");
                     vector<CommandUnit> vCmd;
                    while(xml_ack_len_property)
                    {
                        int nid = strtol(xml_ack_len_property->first_attribute("id")->value(),NULL,10);
                        int nackLen = strtol(xml_ack_len_property->first_attribute("ack_len")->value(),NULL,10);
                        CommandUnit tmUnit;
                        if(nid>=0 && nackLen>0)
                            tmUnit.ackLen = nackLen;
                        std::string qcmd = xml_ack_len_property->first_attribute("command")->value();
                        tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
                        vCmd.push_back(tmUnit);
                        xml_ack_len_property=xml_ack_len_property->next_sibling("reset_command");
                    }

                    //if(vCmd.size()>0)
                        cmd.mapCommand[MSG_DEV_RESET_OPR] = vCmd;
                }
                devices_cmd_[devId] = cmd;
                xml_property = xml_device->first_node("stepopenclose");
                if(xml_property)
                {
                    long nUse =  strtol(xml_property->value(),NULL,10);
                    if(nUse>0)
                        tmpPropertyEx->is_step_open_close = true;
                    else
                        tmpPropertyEx->is_step_open_close = false;
                }
                if(tmpPropertyEx->is_step_open_close)
                {
                    StepCommandAttribute stepcmd;
                    xml_property = xml_device->first_node("stepopen");
                    if(xml_property)
                    {
                         xml_node<>* xml_stcmd = xml_property->first_node("step_opcommand");
                         while(xml_stcmd!=0)
                         {
                             int nid = strtol(xml_stcmd->first_attribute("id")->value(),NULL,10);
                             StepCommandUnit stcmdUnit;
                             std::string qcmd = xml_stcmd->first_attribute("command")->value();
                             stcmdUnit.commandLen = StrToHex(qcmd,&stcmdUnit.commandId[0]);
                             stcmdUnit.checkindex = strtol(xml_stcmd->first_attribute("checkindex")->value(),NULL,10);
                             stcmdUnit.fvalue     = atof(xml_stcmd->first_attribute("checkvalue")->value());
                             stcmdUnit.stcmdtimeout = strtol(xml_stcmd->first_attribute("timeout")->value(),NULL,10);
                             stepcmd.mapstepopencmd[nid] = stcmdUnit;
                             xml_stcmd = xml_stcmd->next_sibling("step_opcommand");
                         }
                    }
                    xml_property = xml_device->first_node("stepclose");
                    if(xml_property)
                    {
                         xml_node<>* xml_stcmd = xml_property->first_node("step_clcommand");
                         while(xml_stcmd!=0)
                         {
                             int nid = strtol(xml_stcmd->first_attribute("id")->value(),NULL,10);
                             StepCommandUnit stcmdUnit;
                             std::string qcmd = xml_stcmd->first_attribute("command")->value();
                             stcmdUnit.commandLen = StrToHex(qcmd,&stcmdUnit.commandId[0]);
                             stcmdUnit.checkindex = strtol(xml_stcmd->first_attribute("checkindex")->value(),NULL,10);
                             stcmdUnit.fvalue     = atof(xml_stcmd->first_attribute("checkvalue")->value());
                             stcmdUnit.stcmdtimeout = strtol(xml_stcmd->first_attribute("timeout")->value(),NULL,10);
                             stepcmd.mapstepclosecmd[nid] = stcmdUnit;
                             xml_stcmd = xml_stcmd->next_sibling("step_clcommand");
                         }
                    }
                    devices_step_cmd[devId] = stepcmd;
                }
                device_property_Ex_[devId]=tmpPropertyEx;
            }

            xml_device=xml_device->next_sibling("device");
        }
        return true;
    }

    return false;
}

void LocalConfig::ntp_config(bool &use,int &nMod,int &nValue,string &sAdjustTime)
{
    use = ntp_upload_use_;
    nMod = ntp_mod_;
    nValue = ntp_mod_value_;
    sAdjustTime = ntp_time_;
}

map<string,pDevicePropertyExPtr>& LocalConfig::device_property_ex()
{
    return device_property_Ex_;
}

pDevicePropertyExPtr  LocalConfig::device_property_ex(string sTransmitterId)
{
    map<string,pDevicePropertyExPtr>::iterator iter = device_property_Ex_.find(sTransmitterId);
    if(iter==device_property_Ex_.end())
        return pDevicePropertyExPtr(new DevicePropertyEx);
    else
        return (*iter).second;
}

pMoxaPropertyExPtr  LocalConfig::moxa_property_ex(string sMoxaId)
{
    map<string,pMoxaPropertyExPtr>::iterator iter = moxas_property_Ex_.find(sMoxaId);
    if(iter==moxas_property_Ex_.end())
        return pMoxaPropertyExPtr(new MoxaPropertyEx);
    else
        return (*iter).second;
}

//保存本地参数(不包括短信参数配置)
bool LocalConfig::writeLocalParToXml(const char* sFileName,string stationId,string stationName,string svcId,
                                     unsigned short svcPort,string dbIp,string dbUser)
{

    file<>   fdoc(sFileName);
    xml_document<>   xml_config;
    xml_config.parse<0>(fdoc.data());
    xml_node<>* root = xml_config.first_node("service");
    if(root!=NULL)
    {
        xml_node<> *xml_station = root->first_node("station");
        if(xml_station!=NULL)
        {
            local_station_id_ = xml_station->first_attribute("id")->value();
            local_station_name_ = xml_station->first_attribute("name")->value();
            local_dev_server_id_ = xml_station->first_attribute("dev_server_id")->value();
            xml_station->remove_attribute(xml_station->first_attribute("id"));
            xml_station->insert_attribute(xml_station->first_attribute("name"),xml_config.allocate_attribute("id","ET000001"));

        }
        else
            return false;
    }
    std::ofstream out(sFileName);
    out << xml_config;

    return true;
}

bool LocalConfig::writeSmsParToXml(const char* sFileName,bool bUse,string comId,int baudRate,string smsCenterNumber)
{
    /*TiXmlDocument xml_config(sFileName);

    if (!xml_config.LoadFile())
        return false;
    TiXmlElement* root = xml_config.FirstChildElement("service");
    if(root!=NULL)
    {
        TiXmlElement* xml_sms = root->FirstChildElement("sms");
        if(xml_sms!=NULL)
        {
            xml_sms->SetAttribute("use",bUse);
            sms_use_ = bUse;
            xml_sms->SetAttribute("com",comId.c_str());
            sms_com_ = comId;
            xml_sms->SetAttribute("baud_rate",baudRate);
            sms_baud_rate_ = baudRate;
            xml_sms->SetAttribute("sms_center_number",smsCenterNumber.c_str());
            sms_center_number_ = smsCenterNumber;
        }
        else
        {
            return false;
        }
        if(xml_config.SaveFile());
        {
            return true;
        }
    }*/
    return false;
}

void LocalConfig::device_cmd( string sDevId,CommandAttribute& cmd )
{
    map<string,CommandAttribute>::iterator iter = devices_cmd_.find(sDevId);
    if(iter!=devices_cmd_.end())
    {
        map<int,vector<CommandUnit> >::iterator type_iter =  iter->second.mapCommand.begin();
        for(;type_iter!= iter->second.mapCommand.end();++type_iter){
            if(type_iter->second.size()>0){//该类型有配置则覆盖原有配置
                cmd.mapCommand[type_iter->first] = type_iter->second;
            }
        }
    }
}

void LocalConfig::device_step_cmd(string sDevId, StepCommandAttribute &stcmd)
{
    map<string,StepCommandAttribute>::iterator iter = devices_step_cmd.find(sDevId);
    if(iter != devices_step_cmd.end())
    {
        stcmd = (*iter).second;
    }
}
