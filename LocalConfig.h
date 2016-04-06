#ifndef LOCAL_CONFIG
#define LOCAL_CONFIG

#pragma once
#include <string>
#include "DataType.h"
using namespace std;
class LocalConfig
{
public:
	LocalConfig(void);
	~LocalConfig(void);
public:
    bool load_local_config(const char* sFileName);
	
	string local_station_id()const{return local_station_id_;}
	string local_dev_server_number()const{return local_dev_server_id_;}
	string local_station_name()const{return local_station_name_;}
	string database_ip()const{return db_ip_;}
	string database_user()const{return db_usr_;}
	string database_password()const{return db_psw_;}
	string database_drivername()const{return db_driver_;}
	unsigned short local_port()const{return local_port_;}
	int            heartbeat_interval()const{return hb_interval_;}
	int            heartbeat_timeout()const{return hb_timeout_;}

    map<string,pDevicePropertyExPtr>& device_property_ex();
    pDevicePropertyExPtr  device_property_ex(string sTransmitterId);
	pMoxaPropertyExPtr  moxa_property_ex(string sMoxaId);
	bool writeLocalParToXml(const char* sFileName,string stationId,string stationName,string svcId,
		                    unsigned short svcPort,string dbIp,string dbUser);

	bool sms_use(){return sms_use_;}
	string sms_com(){return sms_com_;}
	int    sms_baud_rate(){return sms_baud_rate_;}
	string sms_center_number(){return sms_center_number_;}

	bool   upload_use(){return upload_use_;}
	string relay_svc_ip(){return relay_svc_ip_;}
	unsigned short relay_svc_port(){return relay_svc_port_;}

	bool   accept_svc_use(){return accept_upload_use_;}
	unsigned short accept_svc_port(){return accept_svc_port_;}
	int    relay_data_save_interval(){return relay_data_save_invertal_;}
	
	//设备主动上传服务配置
	bool   accept_dev_upload_svc_use(){return accept_dev_upload_use_;}
	unsigned short accept_dev_upload_port(){return accept_dev_upload_svc_port_;}
	double    dev_upload_data_save_interval(){return dev_upload_data_save_invertal_;}

	//告警上传配置
	bool   alarm_upload_use(){return alarm_upload_use_;}
	string alarm_svc_ip(){return alarm_center_svc_ip_;}
	unsigned short alarm_svc_port(){return alarm_center_svc_port_;}

	bool writeSmsParToXml(const char* sFileName,bool bUse,string comId,int baudRate,string smsCenterNumber);
	
    void device_cmd(string sDevId,CommandAttribute& cmd);//读取配置的命令

private:
    string src_code_;//本平台唯一id
	string local_station_id_;   //本平台id
	string local_dev_server_id_;//本监测服务id
	string local_station_name_; //平台名称
	unsigned short local_port_; //监测服务绑定端口
	string db_ip_;      //数据库ip
	string db_usr_;     //数据库用户名
	string db_psw_;     //数据库密码
	string db_driver_;//数据库驱动名称
	int    hb_interval_;//心跳间隔
	int    hb_timeout_; //心跳超时时间

	bool   sms_use_;//是否开启短信报警
	string sms_com_;//串口名称
	int    sms_baud_rate_;//波特率
	string sms_center_number_;//短信中心号码

	bool   upload_use_;//是否上传数据
	string relay_svc_ip_;//级联服务ip
	unsigned short relay_svc_port_;//级联服务port

	bool   accept_upload_use_;//是否接收上传
	unsigned short accept_svc_port_;//接收上传服务端口
	int    relay_data_save_invertal_;//上传数据保存间隔

	bool   accept_dev_upload_use_;//是否接收设备数据上传
	unsigned short accept_dev_upload_svc_port_;//接收设备上传服务端口
	double    dev_upload_data_save_invertal_;//设备数据上传保存间隔

	bool   auto_switch_anttena_use_;//是否自动切换天线

	bool   alarm_upload_use_;//是否上传到告警服务中心
	string alarm_center_svc_ip_;//告警中心服务器ip
	unsigned short alarm_center_svc_port_;//告警中心服务器端口

    map<string,pDevicePropertyExPtr> device_property_Ex_;
	map<string,pMoxaPropertyExPtr> moxas_property_Ex_;

    map<string,CommandAttribute> devices_cmd_;

};
#endif
