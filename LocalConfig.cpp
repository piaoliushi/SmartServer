#include "LocalConfig.h"
#include "utility.h"
#include "./rapidxml/rapidxml.hpp"
#include "./rapidxml/rapidxml_utils.hpp"
#include "./rapidxml/rapidxml_print.hpp"
#include <string.h>
#include <iostream>
using namespace rapidxml;

LocalConfig::LocalConfig(void)
    :db_ip_("127.0.0.1")
    ,db_usr_("sa")
    ,db_psw_("1234")
    ,db_driver_("SQL Native Client")
    ,local_dev_server_id_("")
    ,sms_use_(false)
    ,sms_com_("")
    ,sms_baud_rate_(9600)
    ,sms_center_number_("13888888888")
    ,local_port_(5000)
    ,relay_svc_ip_("")
    ,relay_svc_port_(5001)
    ,upload_use_(false)
    ,accept_svc_port_(5001)
    ,accept_upload_use_(false)
    ,relay_data_save_invertal_(30)
    ,accept_dev_upload_use_(false)
    ,accept_dev_upload_svc_port_(5002)
    ,dev_upload_data_save_invertal_(15)
    ,alarm_upload_use_(false)
    ,alarm_center_svc_ip_("")
    ,alarm_center_svc_port_(5003)
{
    //load_local_config("config.xml");
}

LocalConfig::~LocalConfig(void)
{
}

#include <QString>
#include <QDebug>
#include <QCoreApplication>
bool LocalConfig::load_local_config(const char* sFileName)
{
    file<>   fdoc(sFileName);
    xml_document<>   xml_config;
    const char *p;
    xml_config.parse<0>(fdoc.data());
    xml_node<>* root = xml_config.first_node("service");
    if(root!=NULL)
    {
        xml_node<>* xml_station = root->first_node("station");
        if(xml_station!=NULL)
        {
            local_station_id_ = xml_station->first_attribute("id")->value();
            local_station_name_ = xml_station->first_attribute("name")->value();
            //QString ssname  = QString::fromUtf8(local_station_name_.c_str());
            local_dev_server_id_ = xml_station->first_attribute("dev_server_id")->value();
        }
        else
            return false;

        xml_node<>* xml_sms = root->first_node("sms" );
        if(xml_sms!=NULL)
        {
            long nUse =  strtol(xml_sms->first_attribute("use")->value(),NULL,10);
            if(nUse>0)
                sms_use_=true;
            else
                sms_use_=false;
            sms_com_ = xml_sms->first_attribute("com")->value();
            sms_baud_rate_ =  strtol( xml_sms->first_attribute("baud_rate")->value(),NULL,10);
            sms_center_number_ = xml_sms->first_attribute("sms_center_number")->value();
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
        xml_node<>* xml_relay_svc_root = root->first_node("upload_svc_config" );
        if(xml_server!=NULL)
        {
            //级联链接配置
            xml_node<>* xml_relay_svc = xml_relay_svc_root->first_node("relay_svc");
            if(xml_relay_svc!=NULL)
            {
                long nUse =  strtol(xml_relay_svc->first_attribute("use")->value(),NULL,10);
                if(nUse>0)
                    upload_use_=true;
                else
                    upload_use_=false;
                relay_svc_ip_ = xml_relay_svc->first_attribute("ip")->value();
                relay_svc_port_ = strtol(xml_relay_svc->first_attribute("port")->value(),NULL,10);
            }
            //级联上传服务配置
            xml_node<>* xml_accept_svc = xml_relay_svc_root->first_node("upload_svc");
            if(xml_accept_svc!=0)
            {
                long nUse =  strtol(xml_accept_svc->first_attribute("use")->value(),NULL,10);
                if(nUse>0)
                    accept_upload_use_=true;
                else
                    accept_upload_use_=false;
                accept_svc_port_ = strtol(xml_accept_svc->first_attribute("port")->value(),NULL,10);
                relay_data_save_invertal_ =  strtol(xml_accept_svc->first_attribute("data_save_interval")->value(),NULL,10);
            }
            //设备上传服务配置
            xml_node<>* xml_dev_upload_svc = xml_relay_svc_root->first_node("dev_upload_svc");
            if(xml_dev_upload_svc!=0)
            {
                long nUse =  strtol(xml_dev_upload_svc->first_attribute("use")->value(),NULL,10);
                if(nUse>0)
                    accept_dev_upload_use_=true;
                else
                    accept_dev_upload_use_=false;
                accept_dev_upload_svc_port_ = strtol(xml_dev_upload_svc->first_attribute("port")->value(),NULL,10);
                dev_upload_data_save_invertal_ =  strtod(xml_dev_upload_svc->first_attribute("data_save_interval")->value(),NULL);
            }

            //告警上传服务器配置
            xml_node<>* xml_alarm_svc = xml_relay_svc_root->first_node("alarm_svc");
            if(xml_alarm_svc!=NULL)
            {
                long nUse =  strtol(xml_alarm_svc->first_attribute("use")->value(),NULL,10);
                if(nUse>0)
                    upload_use_=true;
                else
                    upload_use_=false;
                alarm_center_svc_ip_ = xml_alarm_svc->first_attribute("ip")->value();
                alarm_center_svc_port_ = strtol(xml_alarm_svc->first_attribute("port")->value(),NULL,10);
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
                pTransmitterPropertyExPtr tmpPropertyEx= pTransmitterPropertyExPtr(new TransmitterPropertyEx);
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
                    while(xml_ack_len_property)
                    {
                        int nid = strtol(xml_ack_len_property->first_attribute("id")->value(),NULL,10);
                        int nackLen = strtol(xml_ack_len_property->first_attribute("ack_len")->value(),NULL,10);
                        CommandUnit tmUnit;
                        if(nid>=0 && nackLen>0)
                            tmUnit.ackLen = nackLen;
                        std::string qcmd = xml_ack_len_property->first_attribute("command")->value();
                        tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);

                        cmd.queryComm.push_back(tmUnit);
                        xml_ack_len_property=xml_ack_len_property->next_sibling("query_command");
                    }
                }
                xml_property = xml_device->first_node("turnon_command_define");
                if(xml_property)
                {
                    xml_node<>* xml_ack_len_property = xml_property->first_node("turnon_command");
                    while(xml_ack_len_property)
                    {
                        int nid = strtol(xml_ack_len_property->first_attribute("id")->value(),NULL,10);
                        int nackLen = strtol(xml_ack_len_property->first_attribute("ack_len")->value(),NULL,10);
                        CommandUnit tmUnit;
                        if(nid>=0 && nackLen>0)
                            tmUnit.ackLen = nackLen;
                        std::string qcmd = xml_ack_len_property->first_attribute("command")->value();
                        tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
                        cmd.turnonComm.push_back(tmUnit);
                        xml_ack_len_property=xml_ack_len_property->next_sibling("turnon_command");
                    }
                }
                xml_property = xml_device->first_node("turnoff_command_define");
                if(xml_property)
                {
                    xml_node<>* xml_ack_len_property = xml_property->first_node("turnoff_command");
                    while(xml_ack_len_property)
                    {
                        int nid = strtol(xml_ack_len_property->first_attribute("id")->value(),NULL,10);
                        int nackLen = strtol(xml_ack_len_property->first_attribute("ack_len")->value(),NULL,10);
                        CommandUnit tmUnit;
                        if(nid>=0 && nackLen>0)
                            tmUnit.ackLen = nackLen;
                        std::string qcmd = xml_ack_len_property->first_attribute("command")->value();
                        tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
                        cmd.turnoffComm.push_back(tmUnit);
                        xml_ack_len_property=xml_ack_len_property->next_sibling("turnoff_command");
                    }
                }
                xml_property = xml_device->first_node("up_command_define");
                if(xml_property)
                {
                    xml_node<>* xml_ack_len_property = xml_property->first_node("up_command");
                    while(xml_ack_len_property)
                    {
                        int nid = strtol(xml_ack_len_property->first_attribute("id")->value(),NULL,10);
                        int nackLen = strtol(xml_ack_len_property->first_attribute("ack_len")->value(),NULL,10);
                        CommandUnit tmUnit;
                        if(nid>=0 && nackLen>0)
                            tmUnit.ackLen = nackLen;
                        std::string qcmd = xml_ack_len_property->first_attribute("command")->value();
                        tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
                        cmd.uppowerComm.push_back(tmUnit);
                        xml_ack_len_property=xml_ack_len_property->next_sibling("up_command");
                    }
                }
                xml_property = xml_device->first_node("dw_command_define");
                if(xml_property)
                {
                    xml_node<>* xml_ack_len_property = xml_property->first_node("dw_command");
                    while(xml_ack_len_property)
                    {
                        int nid = strtol(xml_ack_len_property->first_attribute("id")->value(),NULL,10);
                        int nackLen = strtol(xml_ack_len_property->first_attribute("ack_len")->value(),NULL,10);
                        CommandUnit tmUnit;
                        if(nid>=0 && nackLen>0)
                            tmUnit.ackLen = nackLen;
                        std::string qcmd = xml_ack_len_property->first_attribute("command")->value();
                        tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
                        cmd.lowpowerComm.push_back(tmUnit);
                        xml_ack_len_property=xml_ack_len_property->next_sibling("dw_command");
                    }
                }
                xml_property = xml_device->first_node("reset_command_define");
                if(xml_property)
                {
                    xml_node<>* xml_ack_len_property = xml_property->first_node("reset_command");
                    while(xml_ack_len_property)
                    {
                        int nid = strtol(xml_ack_len_property->first_attribute("id")->value(),NULL,10);
                        int nackLen = strtol(xml_ack_len_property->first_attribute("ack_len")->value(),NULL,10);
                        CommandUnit tmUnit;
                        if(nid>=0 && nackLen>0)
                            tmUnit.ackLen = nackLen;
                        std::string qcmd = xml_ack_len_property->first_attribute("command")->value();
                        tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
                        cmd.lowpowerComm.push_back(tmUnit);
                        xml_ack_len_property=xml_ack_len_property->next_sibling("reset_command");
                    }
                }

                transmintters_cmd[devId] = cmd;
                transmitters_property_Ex_[devId]=tmpPropertyEx;
            }

            xml_device=xml_device->next_sibling("device");
        }
        return true;
    }

    return false;
}
map<string,pTransmitterPropertyExPtr>& LocalConfig::transmitter_property_ex()
{
    return transmitters_property_Ex_;
}
pTransmitterPropertyExPtr  LocalConfig::transmitter_property_ex(string sTransmitterId)
{
    map<string,pTransmitterPropertyExPtr>::iterator iter = transmitters_property_Ex_.find(sTransmitterId);
    if(iter==transmitters_property_Ex_.end())
        return pTransmitterPropertyExPtr(new TransmitterPropertyEx);
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
    /*TiXmlDocument xml_config(sFileName);

    if (!xml_config.LoadFile())
        return false;
    TiXmlElement* root = xml_config.FirstChildElement("service");
    if(root!=NULL)
    {
        TiXmlElement* xml_station = root->FirstChildElement("station" );
        if(xml_station!=NULL)
        {
            local_station_id_ =stationId;
            xml_station->SetAttribute("id",stationId.c_str());
            local_station_name_ = stationName;
            xml_station->SetAttribute("name",stationName.c_str());
            local_dev_server_id_ = svcId;
            xml_station->SetAttribute("dev_server_id",svcId.c_str());
        }
        else
            return false;

        TiXmlElement* xml_database = root->FirstChildElement("database" );
        if(xml_database!=NULL)
        {
            db_ip_ = dbIp;
            xml_database->SetAttribute("ip",dbIp.c_str());
            db_usr_ = dbUser;
            xml_database->SetAttribute("user",dbUser.c_str());
        }
        else
            return false;

        TiXmlElement* xml_server = root->FirstChildElement("server" );
        if(xml_server!=NULL)
        {
            local_port_ = svcPort;
            xml_server->SetAttribute("port",svcPort);
        }
        else
            return false;

        xml_config.SaveFile();
    }*/
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

void LocalConfig::transmitter_cmd( string sTransmitterId,CommandAttribute& cmd )
{
    map<string,CommandAttribute>::iterator iter = transmintters_cmd.find(sTransmitterId);
    if(iter!=transmintters_cmd.end())
    {

        if((*iter).second.queryComm.size()>0)
        {
            //cmd.queryComm.clear();
            for(int i=0;i<(*iter).second.queryComm.size();i++)
            {
                //cmd.queryComm.push_back((*iter).second.queryComm[i]);
                if(i>=cmd.queryComm.size())
                {
                    cmd.queryComm.push_back((*iter).second.queryComm[i]);
                    continue;
                }
                cmd.queryComm[i].ackLen = (*iter).second.queryComm[i].ackLen;
                if((*iter).second.queryComm[i].commandLen>0)
                {
                    memcpy(cmd.queryComm[i].commandId,(*iter).second.queryComm[i].commandId,(*iter).second.queryComm[i].commandLen);
                    cmd.queryComm[i].commandLen=(*iter).second.queryComm[i].commandLen;
                }
            }
        }
        if((*iter).second.turnonComm.size()>0)
        {
            for(int i=0;i<(*iter).second.turnonComm.size();i++)
            {
                if(i>=cmd.turnonComm.size())
                {
                    cmd.turnonComm.push_back((*iter).second.turnonComm[i]);
                    continue;
                }
                cmd.turnonComm[i].ackLen = (*iter).second.turnonComm[i].ackLen;
                if((*iter).second.turnonComm[i].commandLen>0)
                {
                    memcpy(cmd.turnonComm[i].commandId,(*iter).second.turnonComm[i].commandId,(*iter).second.turnonComm[i].commandLen);
                    cmd.turnonComm[i].commandLen=(*iter).second.turnonComm[i].commandLen;
                }
            }
        }
        if((*iter).second.turnoffComm.size()>0)
        {
            for(int i=0;i<(*iter).second.turnoffComm.size();i++)
            {
                if(i>=cmd.turnoffComm.size())
                {
                    cmd.turnoffComm.push_back((*iter).second.turnoffComm[i]);
                    continue;
                }
                cmd.turnoffComm[i].ackLen = (*iter).second.turnoffComm[i].ackLen;
                if((*iter).second.turnoffComm[i].commandLen>0)
                {
                    memcpy(cmd.turnoffComm[i].commandId,(*iter).second.turnoffComm[i].commandId,(*iter).second.turnoffComm[i].commandLen);
                    cmd.turnoffComm[i].commandLen=(*iter).second.turnoffComm[i].commandLen;
                }
            }
        }
        /**/
        if((*iter).second.uppowerComm.size()>0)
        {
            for(int i=0;i<(*iter).second.uppowerComm.size();i++)
            {
                if(i>=cmd.uppowerComm.size())
                {
                    cmd.uppowerComm.push_back((*iter).second.uppowerComm[i]);
                    continue;
                }
                cmd.uppowerComm[i].ackLen = (*iter).second.uppowerComm[i].ackLen;
                if((*iter).second.uppowerComm[i].commandLen>0)
                {
                    memcpy(cmd.uppowerComm[i].commandId,(*iter).second.uppowerComm[i].commandId,(*iter).second.uppowerComm[i].commandLen);
                    cmd.uppowerComm[i].commandLen=(*iter).second.uppowerComm[i].commandLen;
                }
            }
        }
        if((*iter).second.lowpowerComm.size()>0)
        {

            for(int i=0;i<(*iter).second.lowpowerComm.size();i++)
            {
                if(i>=cmd.lowpowerComm.size())
                {
                    cmd.lowpowerComm.push_back((*iter).second.lowpowerComm[i]);
                    continue;
                }
                cmd.lowpowerComm[i].ackLen = (*iter).second.lowpowerComm[i].ackLen;
                if((*iter).second.lowpowerComm[i].commandLen>0)
                {
                    memcpy(cmd.lowpowerComm[i].commandId,(*iter).second.lowpowerComm[i].commandId,(*iter).second.lowpowerComm[i].commandLen);
                    cmd.lowpowerComm[i].commandLen=(*iter).second.lowpowerComm[i].commandLen;
                }
            }
        }
        if((*iter).second.resetComm.size()>0)
        {
            for(int i=0;i<(*iter).second.resetComm.size();i++)
            {
                if(i>=cmd.resetComm.size())
                {
                    cmd.resetComm.push_back((*iter).second.resetComm[i]);
                    continue;
                }
                cmd.resetComm[i].ackLen = (*iter).second.resetComm[i].ackLen;
                if((*iter).second.resetComm[i].commandLen>0)
                {
                    memcpy(cmd.resetComm[i].commandId,(*iter).second.resetComm[i].commandId,(*iter).second.resetComm[i].commandLen);
                    cmd.resetComm[i].commandLen=(*iter).second.resetComm[i].commandLen;
                }
            }
        }

        if((*iter).second.switchComm.size()>0)
        {
            for(int i=0;i<(*iter).second.switchComm.size();i++)
            {
                if(i>=cmd.switchComm.size())
                {
                    cmd.switchComm.push_back((*iter).second.switchComm[i]);
                    continue;
                }
                cmd.switchComm[i].ackLen = (*iter).second.switchComm[i].ackLen;
                if((*iter).second.switchComm[i].commandLen>0)
                {
                    memcpy(cmd.switchComm[i].commandId,(*iter).second.switchComm[i].commandId,(*iter).second.switchComm[i].commandLen);
                    cmd.switchComm[i].commandLen=(*iter).second.switchComm[i].commandLen;
                }
            }
        }
        if((*iter).second.switch2Comm.size()>0)
        {
            for(int i=0;i<(*iter).second.switch2Comm.size();i++)
            {
                if(i>=cmd.switch2Comm.size())
                {
                    cmd.switch2Comm.push_back((*iter).second.switch2Comm[i]);
                    continue;
                }
                cmd.switch2Comm[i].ackLen = (*iter).second.switch2Comm[i].ackLen;
                if((*iter).second.switch2Comm[i].commandLen>0)
                {
                    memcpy(cmd.switch2Comm[i].commandId,(*iter).second.switch2Comm[i].commandId,(*iter).second.switch2Comm[i].commandLen);
                    cmd.switch2Comm[i].commandLen=(*iter).second.switch2Comm[i].commandLen;
                }
            }
        }

    }
}
