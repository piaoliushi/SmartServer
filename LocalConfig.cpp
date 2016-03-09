#include "LocalConfig.h"
//#include "./tinyxml/tinyxml.h"
#include <string.h>
#include <iostream>

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
}

LocalConfig::~LocalConfig(void)
{
}

bool LocalConfig::load_local_config(const char* sFileName)
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
			local_station_id_ = xml_station->Attribute("id");
			local_station_name_ = xml_station->Attribute("name");
			local_dev_server_id_ = xml_station->Attribute("dev_server_id");
		}
		else
			return false;

		TiXmlElement* xml_sms = root->FirstChildElement("sms" );
		if(xml_sms!=NULL)
		{
			int nUse=0;
			xml_sms->Attribute("use",&nUse);
			sms_use_ = nUse;
			sms_com_ = xml_sms->Attribute("com");
			xml_sms->Attribute("baud_rate",(int*)(&sms_baud_rate_));
			sms_center_number_ = xml_sms->Attribute("sms_center_number");
		}
		else
			return false;


		TiXmlElement* xml_database = root->FirstChildElement("database" );
		if(xml_database!=NULL)
		{
			db_ip_ = xml_database->Attribute("ip");
			db_usr_ = xml_database->Attribute("user");
			db_psw_ = xml_database->Attribute("password");
			const char *tmpName  = xml_database->Attribute("drivername");
			if(tmpName!=NULL)
				db_driver_ = tmpName;
		}
		else
			return false;

		TiXmlElement* xml_server = root->FirstChildElement("server" );
		if(xml_server!=NULL)
		{
			int queryRlt = xml_server->QueryIntAttribute("port",(int *)(&local_port_));
			if(queryRlt!=TIXML_SUCCESS)
				return false;

			TiXmlElement* xml_hb = xml_server->FirstChildElement("heart_beat");
			if(xml_hb!=NULL)
			{
				int queryRlt = xml_hb->QueryIntAttribute("interval",&hb_interval_);
				if(queryRlt!=TIXML_SUCCESS)
					return false;
				queryRlt = xml_hb->QueryIntAttribute("timeout",&hb_timeout_);
				if(queryRlt!=TIXML_SUCCESS)
					return false;
			}
			else
				return false;
		}
		else
			return false;
		//检查是否有数据上传服务配置
		TiXmlElement* xml_relay_svc_root = root->FirstChildElement("upload_svc_config" );
		if(xml_server!=NULL)
		{
			//级联链接配置
			TiXmlElement* xml_relay_svc = xml_relay_svc_root->FirstChildElement("relay_svc");
			if(xml_relay_svc!=NULL)
			{
				
				int nUse=0;
				xml_relay_svc->Attribute("use",&nUse);
				upload_use_ = nUse;
				relay_svc_ip_ = xml_relay_svc->Attribute("ip");
				xml_relay_svc->QueryIntAttribute("port",(int *)(&relay_svc_port_));
			}
			//级联上传服务配置
			TiXmlElement* xml_accept_svc = xml_relay_svc_root->FirstChildElement("upload_svc");
			if(xml_accept_svc!=0)
			{

				int nUse=0;
				xml_accept_svc->Attribute("use",&nUse);
				accept_upload_use_ = nUse;
				xml_accept_svc->QueryIntAttribute("port",(int *)(&accept_svc_port_));
				xml_accept_svc->QueryIntAttribute("data_save_interval",&relay_data_save_invertal_);
			}
			//设备上传服务配置
			TiXmlElement* xml_dev_upload_svc = xml_relay_svc_root->FirstChildElement("dev_upload_svc");
			if(xml_dev_upload_svc!=0)
			{
				int nUse=0;
				xml_dev_upload_svc->Attribute("use",&nUse);
				accept_dev_upload_use_ = nUse;
				xml_dev_upload_svc->QueryIntAttribute("port",(int *)(&accept_dev_upload_svc_port_));
				xml_dev_upload_svc->QueryDoubleAttribute("data_save_interval",&dev_upload_data_save_invertal_);
			}

			//告警上传服务器配置
			TiXmlElement* xml_alarm_svc = xml_relay_svc_root->FirstChildElement("alarm_svc");
			if(xml_alarm_svc!=NULL)
			{

				int nUse=0;
				xml_alarm_svc->Attribute("use",&nUse);
				alarm_upload_use_ = nUse;
				alarm_center_svc_ip_ = xml_alarm_svc->Attribute("ip");
				xml_alarm_svc->QueryIntAttribute("port",(int *)(&alarm_center_svc_port_));
			}
			
		}


		TiXmlElement* xml_moxa = root->FirstChildElement("moxa_modle");
		while(xml_moxa!=0)
		{

			string moxaId = xml_moxa->Attribute("id");
			if(!moxaId.empty())
			{
				pMoxaPropertyExPtr tmpPropertyEx= pMoxaPropertyExPtr(new MoxaPropertyEx);
				//查询超时次数
				TiXmlElement* xml_property = xml_moxa->FirstChildElement("query_timeout_count");
				if(xml_property)
					tmpPropertyEx->query_timeout_count = atoi(xml_property->GetText());
				//查询指令间隔
				xml_property = xml_moxa->FirstChildElement("query_interval");
				if(xml_property)
					tmpPropertyEx->query_interval = atoi(xml_property->GetText());
				//连接定时器间隔（自动重连机制）
				xml_property = xml_moxa->FirstChildElement("connect_timer_interval");
				if(xml_property)
					tmpPropertyEx->connect_timer_interval = atoi(xml_property->GetText());
				//网络连接超时时长 
				xml_property = xml_moxa->FirstChildElement("connect_timeout_duration");
				if(xml_property)
					tmpPropertyEx->connect_timeout_duration = atoi(xml_property->GetText());
				moxas_property_Ex_[moxaId]=tmpPropertyEx;
			}
			xml_moxa=xml_moxa->NextSiblingElement("moxa_modle");
		}



		TiXmlElement* xml_transmitter = root->FirstChildElement("transmitter");
		while(xml_transmitter!=0)
		{
			
			string devId = xml_transmitter->Attribute("id");
			if(!devId.empty())
			{
				pTransmitterPropertyExPtr tmpPropertyEx= pTransmitterPropertyExPtr(new TransmitterPropertyEx);
				//查询超时次数
				TiXmlElement* xml_property = xml_transmitter->FirstChildElement("query_timeout_count");
				if(xml_property)
					tmpPropertyEx->query_timeout_count = atoi(xml_property->GetText());
				//查询指令间隔
				xml_property = xml_transmitter->FirstChildElement("query_interval");
				if(xml_property)
					tmpPropertyEx->query_interval = atoi(xml_property->GetText()); 
				//连接定时器间隔（自动重连机制）
				xml_property = xml_transmitter->FirstChildElement("connect_timer_interval");
				if(xml_property)
					tmpPropertyEx->connect_timer_interval = atoi(xml_property->GetText());
				//网络连接超时时长 
				xml_property = xml_transmitter->FirstChildElement("connect_timeout_duration");
				if(xml_property)
					tmpPropertyEx->connect_timeout_duration = atoi(xml_property->GetText());
				//开机最大检测次数
				xml_property = xml_transmitter->FirstChildElement("run_detect_max_count");
				if(xml_property)
					tmpPropertyEx->run_detect_max_count = atoi(xml_property->GetText());
				//关机最大检测次数
				xml_property = xml_transmitter->FirstChildElement("shutdown_detect_max_count");
				if(xml_property)
					tmpPropertyEx->shutdown_detect_max_count = atoi(xml_property->GetText());
				//数据保存间隔
				xml_property = xml_transmitter->FirstChildElement("data_save_interval");
				if(xml_property)
					tmpPropertyEx->data_save_interval = atoi(xml_property->GetText());
				//报警检测最大次数
				xml_property = xml_transmitter->FirstChildElement("alarm_detect_max_count");
				if(xml_property)
					tmpPropertyEx->alarm_detect_max_count = atoi(xml_property->GetText());
				//命令超时间隔
				xml_property = xml_transmitter->FirstChildElement("cmd_timeout_interval");
				if(xml_property)
					tmpPropertyEx->cmd_timeout_interval = atoi(xml_property->GetText());
				//命令执行超时时长
				xml_property = xml_transmitter->FirstChildElement("cmd_excut_timeout_duration");
				if(xml_property)
					tmpPropertyEx->cmd_excut_timeout_duration = atoi(xml_property->GetText());
				//多查询指令发送间隔
				xml_property = xml_transmitter->FirstChildElement("multi_query_send_interval");
				if(xml_property)
					tmpPropertyEx->multi_query_send_interval = atoi(xml_property->GetText());
				//功率零值设定
				xml_property = xml_transmitter->FirstChildElement("zero_power_value");
				if(xml_property)
					tmpPropertyEx->zero_power_value = atof(xml_property->GetText());
				//9033A电压矫正系数
				xml_property = xml_transmitter->FirstChildElement("ubb_ratio_value");
				if(xml_property)
					tmpPropertyEx->ubb_ratio_value = atof(xml_property->GetText());
				//9033A电流矫正系数
				xml_property = xml_transmitter->FirstChildElement("ibb_ratio_value");
				if(xml_property)
					tmpPropertyEx->ibb_ratio_value = atof(xml_property->GetText());
				//9033A电流量程范围
				xml_property = xml_transmitter->FirstChildElement("i0_range_value");
				if(xml_property)
					tmpPropertyEx->i0_range_value = atoi(xml_property->GetText());
				//9033A电压量程范围
				xml_property = xml_transmitter->FirstChildElement("u0_range_value");
				if(xml_property)
					tmpPropertyEx->u0_range_value = atoi(xml_property->GetText());

				xml_property = xml_transmitter->FirstChildElement("associate_tranmitter");
				if(xml_property)
				{
					int bUse=0;
					xml_property->Attribute("use",&bUse);
					tmpPropertyEx->associate_use = bUse;
					tmpPropertyEx->associate_transmit_number = xml_property->Attribute("id");
				}

				//倒机功率设定
				xml_property = xml_transmitter->FirstChildElement("auto_switch");//switch_power_lower_limit_value
				if(xml_property)
				{
					int bUse=0;
					xml_property->Attribute("use",&bUse);
					tmpPropertyEx->is_auto_switch_transmitter = bUse;
					double limit_value=0.05;
					xml_property->Attribute("limit_value",&limit_value);
					tmpPropertyEx->switch_power_lower_limit_value = limit_value;//atof(xml_property->GetText())
					int detect_times=2;
					xml_property->Attribute("detect_times",&detect_times);
					tmpPropertyEx->auto_switch_detect_max_times = detect_times;
					int timeout_times=5;
					xml_property->Attribute("timeout_times",&timeout_times);
					tmpPropertyEx->auto_switch_timeout_max_times = timeout_times;
				}

				xml_property = xml_transmitter->FirstChildElement("query_command_length");
				CommandAttribute cmd;
				if(xml_property) 
				{
					TiXmlElement* xml_ack_len_property = xml_property->FirstChildElement("query_command");
					while(xml_ack_len_property)
					{
						int nid=0,nackLen=0;
						int idRlt = xml_ack_len_property->QueryIntAttribute("id",&nid);
						int ackLenRlt = xml_ack_len_property->QueryIntAttribute("ack_len",&nackLen);
						//if(idRlt==TIXML_SUCCESS && ackLenRlt==TIXML_SUCCESS)
						//	tmpPropertyEx->cmd_ack_length_by_id[nid]=nackLen;

						CommandUnit tmUnit;
						if(idRlt==TIXML_SUCCESS && ackLenRlt==TIXML_SUCCESS)
						{
							tmpPropertyEx->cmd_ack_length_by_id[nid]=nackLen;
							tmUnit.ackLen = nackLen;
						}
						std::string qcmd = xml_ack_len_property->Attribute("command");
						tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
						//if(tmUnit.commandLen>0)
						cmd.queryComm.push_back(tmUnit);


						xml_ack_len_property=xml_ack_len_property->NextSiblingElement("query_command");

					} 
				}

				xml_property = xml_transmitter->FirstChildElement("turnon_command_length");
				if(xml_property) 
				{
					TiXmlElement* xml_ack_len_property = xml_property->FirstChildElement("turnon_command");
					while(xml_ack_len_property)
					{
						int nid=0,nackLen=0;
						int idRlt = xml_ack_len_property->QueryIntAttribute("id",&nid);
						int ackLenRlt = xml_ack_len_property->QueryIntAttribute("ack_len",&nackLen);
						CommandUnit tmUnit;
						if(idRlt==TIXML_SUCCESS && ackLenRlt==TIXML_SUCCESS)
						{
							//	tmpPropertyEx->cmd_ack_length_by_id[nid]=nackLen;
							tmUnit.ackLen = nackLen;
						}
						std::string qcmd = xml_ack_len_property->Attribute("command");
						tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
						//if(tmUnit.commandLen>0)
						cmd.turnonComm.push_back(tmUnit);
						xml_ack_len_property=xml_ack_len_property->NextSiblingElement("turnon_command");
					} 
				}

				xml_property = xml_transmitter->FirstChildElement("turnoff_command_length");
				if(xml_property) 
				{
					TiXmlElement* xml_ack_len_property = xml_property->FirstChildElement("turnoff_command");
					while(xml_ack_len_property)
					{
						int nid=0,nackLen=0;
						int idRlt = xml_ack_len_property->QueryIntAttribute("id",&nid);
						int ackLenRlt = xml_ack_len_property->QueryIntAttribute("ack_len",&nackLen);
						CommandUnit tmUnit;
						if(idRlt==TIXML_SUCCESS && ackLenRlt==TIXML_SUCCESS)
						{
							//		tmpPropertyEx->cmd_ack_length_by_id[nid]=nackLen;
							tmUnit.ackLen = nackLen;
						}
						std::string qcmd = xml_ack_len_property->Attribute("command");
						tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
						//if(tmUnit.commandLen>0)
							cmd.turnoffComm.push_back(tmUnit);
						xml_ack_len_property=xml_ack_len_property->NextSiblingElement("turnoff_command");
					} 
				}

				xml_property = xml_transmitter->FirstChildElement("up_command_length");
				if(xml_property) 
				{
					TiXmlElement* xml_ack_len_property = xml_property->FirstChildElement("up_command");
					while(xml_ack_len_property)
					{
						int nid=0,nackLen=0;
						int idRlt = xml_ack_len_property->QueryIntAttribute("id",&nid);
						int ackLenRlt = xml_ack_len_property->QueryIntAttribute("ack_len",&nackLen);
						CommandUnit tmUnit;
						if(idRlt==TIXML_SUCCESS && ackLenRlt==TIXML_SUCCESS)
						{
							//		tmpPropertyEx->cmd_ack_length_by_id[nid]=nackLen;
							tmUnit.ackLen = nackLen;
						}
						std::string qcmd = xml_ack_len_property->Attribute("command");
						tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
						//if(tmUnit.commandLen>0)
							cmd.uppowerComm.push_back(tmUnit);
						xml_ack_len_property=xml_ack_len_property->NextSiblingElement("up_command");
					} 
				}

				xml_property = xml_transmitter->FirstChildElement("dw_command_length");
				if(xml_property) 
				{
					TiXmlElement* xml_ack_len_property = xml_property->FirstChildElement("dw_command");
					while(xml_ack_len_property)
					{
						int nid=0,nackLen=0;
						int idRlt = xml_ack_len_property->QueryIntAttribute("id",&nid);
						int ackLenRlt = xml_ack_len_property->QueryIntAttribute("ack_len",&nackLen);
						CommandUnit tmUnit;
						if(idRlt==TIXML_SUCCESS && ackLenRlt==TIXML_SUCCESS)
						{
							//	tmpPropertyEx->cmd_ack_length_by_id[nid]=nackLen;
							tmUnit.ackLen = nackLen;
						}
						std::string qcmd = xml_ack_len_property->Attribute("command");
						tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
					//	if(tmUnit.commandLen>0)
							cmd.lowpowerComm.push_back(tmUnit);
						xml_ack_len_property=xml_ack_len_property->NextSiblingElement("dw_command");
					} 
				}
				xml_property = xml_transmitter->FirstChildElement("reset_command_length");
				if(xml_property) 
				{
					TiXmlElement* xml_ack_len_property = xml_property->FirstChildElement("reset_command");
					while(xml_ack_len_property)
					{
						int nid=0,nackLen=0;
						int idRlt = xml_ack_len_property->QueryIntAttribute("id",&nid);
						int ackLenRlt = xml_ack_len_property->QueryIntAttribute("ack_len",&nackLen);
						CommandUnit tmUnit;
						if(idRlt==TIXML_SUCCESS && ackLenRlt==TIXML_SUCCESS)
						{
							//	tmpPropertyEx->cmd_ack_length_by_id[nid]=nackLen;
							tmUnit.ackLen = nackLen;
						}
						std::string qcmd = xml_ack_len_property->Attribute("command");
						tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
					//	if(tmUnit.commandLen>0)
							cmd.resetComm.push_back(tmUnit);
						xml_ack_len_property=xml_ack_len_property->NextSiblingElement("reset_command");
					} 
				}
				xml_property = xml_transmitter->FirstChildElement("switch_command_length");
				if(xml_property) 
				{
					TiXmlElement* xml_ack_len_property = xml_property->FirstChildElement("switch_command");
					while(xml_ack_len_property)
					{
						int nid=0,nackLen=0;
						int idRlt = xml_ack_len_property->QueryIntAttribute("id",&nid);
						int ackLenRlt = xml_ack_len_property->QueryIntAttribute("ack_len",&nackLen);
						CommandUnit tmUnit;
						if(idRlt==TIXML_SUCCESS && ackLenRlt==TIXML_SUCCESS)
						{
							//	tmpPropertyEx->cmd_ack_length_by_id[nid]=nackLen;
							tmUnit.ackLen = nackLen;
						}
						std::string qcmd = xml_ack_len_property->Attribute("command");
						tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
					//	if(tmUnit.commandLen>0)
							cmd.switchComm.push_back(tmUnit);
						xml_ack_len_property=xml_ack_len_property->NextSiblingElement("switch_command");
					} 
				}
				xml_property = xml_transmitter->FirstChildElement("switch2_command_length");
				if(xml_property) 
				{
					TiXmlElement* xml_ack_len_property = xml_property->FirstChildElement("switch2_command");
					while(xml_ack_len_property)
					{
						int nid=0,nackLen=0;
						int idRlt = xml_ack_len_property->QueryIntAttribute("id",&nid);
						int ackLenRlt = xml_ack_len_property->QueryIntAttribute("ack_len",&nackLen);
						CommandUnit tmUnit;
						if(idRlt==TIXML_SUCCESS && ackLenRlt==TIXML_SUCCESS)
						{
							//tmpPropertyEx->cmd_ack_length_by_id[nid]=nackLen;
							tmUnit.ackLen = nackLen;
						}
						std::string qcmd = xml_ack_len_property->Attribute("command");
						tmUnit.commandLen = StrToHex(qcmd,&tmUnit.commandId[0]);
					//	if(tmUnit.commandLen>0)
							cmd.switch2Comm.push_back(tmUnit);
						xml_ack_len_property=xml_ack_len_property->NextSiblingElement("switch2_command");
					} 
				}

				transmintters_cmd[devId] = cmd;
				transmitters_property_Ex_[devId]=tmpPropertyEx;
			}
			xml_transmitter=xml_transmitter->NextSiblingElement("transmitter");
		}

		return true;
	}
*/
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

bool LocalConfig::writeSwitchPar(const char* sFileName,const string sTransmitterId,bool bUse,
								 string switchPower,string detectCount,string maxDetect)
{
    /*TiXmlDocument xml_config(sFileName);

	if (!xml_config.LoadFile())
		return false;
	TiXmlElement* root = xml_config.FirstChildElement("service"); 
	if(root!=NULL)
	{
		TiXmlElement* xml_transmitter = root->FirstChildElement("transmitter");
		while(xml_transmitter!=0)
		{
			string devId = xml_transmitter->Attribute("id");
			if(!devId.empty()&& sTransmitterId==devId)
			{
				pTransmitterPropertyExPtr pCurTransmitter =  transmitter_property_ex(devId);
				//倒机功率设定
				TiXmlElement* xml_property = xml_transmitter->FirstChildElement("auto_switch");//switch_power_lower_limit_value
				if(xml_property)
				{
					xml_property->SetAttribute("use",bUse);
					xml_property->SetAttribute("limit_value",switchPower.c_str());
				}
				if(pCurTransmitter)
				{
					pCurTransmitter->is_auto_switch_transmitter = bUse;
					pCurTransmitter->switch_power_lower_limit_value = atof(switchPower.c_str());
				}
			}
			xml_transmitter=xml_transmitter->NextSiblingElement("transmitter");
		}
		return xml_config.SaveFile();
    }*/
	return false;
}

int LocalConfig::HexChar( char src )
{
	if((src>='0')&&(src<='9'))
		return src-0x30;
	else if((src>='A')&&(src<='F'))
		return src-'A'+10;
	else if((src>='a')&&(src<='f'))
		return src-'a'+10;
	else 
		return 0x10;
}

int LocalConfig::StrToHex( string str,unsigned char* Data )
{
	int t,t1;
	int rlen=0,len=str.length();
	for(int i=0;i<len;)
	{
		char l,h=str[i];
		if(h==' ')
		{
			i++;
			continue;
		}
		i++;
		if(i>=len)
			break;
		l=str[i];
		t=HexChar(h);
		t1=HexChar(l);
		if((t==16)||(t1==16))
			break;
		else 
			t=t*16+t1;
		i++;
		Data[rlen]=(unsigned char)t;
		rlen++;
	}
	return rlen;
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
