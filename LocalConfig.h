#ifndef LOCAL_CONFIG
#define LOCAL_CONFIG

#pragma once
#include <string>
#include "DataType.h"
using namespace std;
class   LocalConfig
{
public:
	LocalConfig(void);
	~LocalConfig(void);
public:
    bool load_local_config(const char* sFileName);
	
	string local_station_id()const{return local_station_id_;}
	string local_dev_server_number()const{return local_dev_server_id_;}
	string local_station_name()const{return local_station_name_;}
    string src_code(){return src_code_;}
    string dst_code(){return dst_code_;}
	string database_ip()const{return db_ip_;}
	string database_user()const{return db_usr_;}
	string database_password()const{return db_psw_;}
	string database_drivername()const{return db_driver_;}
	unsigned short local_port()const{return local_port_;}
	int            heartbeat_interval()const{return hb_interval_;}
	int            heartbeat_timeout()const{return hb_timeout_;}
    void ntp_config(bool &use,int &nMod,int &nValue,string &sAdjustTime);

    map<string,pDevicePropertyExPtr>& device_property_ex();
    pDevicePropertyExPtr  device_property_ex(string sTransmitterId);
	pMoxaPropertyExPtr  moxa_property_ex(string sMoxaId);
	bool writeLocalParToXml(const char* sFileName,string stationId,string stationName,string svcId,
		                    unsigned short svcPort,string dbIp,string dbUser);

    //bool sms_use(){return sms_use_;}
    //string sms_com(){return sms_com_;}
    //int    sms_baud_rate(){return sms_baud_rate_;}
    //string sms_center_number(){return sms_center_number_;}

    bool   report_use(){return report_use_;}
    string report_svc_url(){return report_svc_url_;}

    bool   http_svc_use(){return http_upload_use_;}
    string http_svc_ip(){return http_svc_ip_;}
    string http_svc_port(){return http_svc_port_;}
    string ntp_svc_ip(){return ntp_svc_ip_;}
    bool  ntp_svc_use(){return ntp_upload_use_;}
	
	bool writeSmsParToXml(const char* sFileName,bool bUse,string comId,int baudRate,string smsCenterNumber);
	
    void device_cmd(string sDevId,CommandAttribute& cmd);//读取配置的命令

private:
    string src_code_;//本平台唯一id
    string dst_code_;//目标平台唯一id

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

    //bool   sms_use_;//是否开启短信报警
    //string sms_com_;//串口名称
    //int    sms_baud_rate_;//波特率
    //string sms_center_number_;//短信中心号码

    bool   report_use_;//是否上传数据
    string report_svc_url_;//上报目标url

    string ntp_svc_ip_;//ntp服务器
    bool ntp_upload_use_;
    int ntp_mod_;//0:天1:星期2:月
    int ntp_mod_value_;//星期1-7,月1-12,天1-31
    string  ntp_time_;//校时时间

    bool   http_upload_use_;//是否接收http请求
    string http_svc_port_;//接收上传服务端口
    string http_svc_ip_;//上报目标url

    map<string,pDevicePropertyExPtr> device_property_Ex_;
	map<string,pMoxaPropertyExPtr> moxas_property_Ex_;

    map<string,CommandAttribute> devices_cmd_;

};
#endif
