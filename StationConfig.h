#ifndef STATION_CONFIG
#define STATION_CONFIG
#pragma once

#include "./database/DataBaseOperation.h"
#include "DataType.h"
#include "DataTypeDefine.h"
#include <map>
using namespace std;

#define GET_CONST_STR(X,Y)  GetInst(StationConfig).get_dictionary_value(X,Y)
#define CONST_STR_RESPONSE_VALUE_DESC(Y)  GetInst(StationConfig).get_dictionary_value("s_rsps_result_desc",Y)
#define DEV_CMD_OPR_DESC(Y)    GetInst(StationConfig).get_dictionary_value("s_cmd_opr_desc",Y)
#define DEV_CMD_OPR_MODE(Y)    GetInst(StationConfig).get_dictionary_value("s_cmd_excute_mode",Y)
#define DEV_CMD_RESULT_DESC(Y)      GetInst(StationConfig).get_dictionary_value("s_cmd_result_desc",Y)
#define CONST_STR_ALARM_CONTENT(Y)  GetInst(StationConfig).get_dictionary_value("s_alarm_event",Y)
#define GLOBAL_STR(Y)  GetInst(StationConfig).get_dictionary_value("s_base",Y)
#define GET_POWER_STR(Y)   GetInst(StationConfig).get_dictionary_value("s_power_base",Y)

#define DEV_CMD_OPR_TYPE(Y)    GetInst(StationConfig).get_dictionary_value("CommandType",Y)


class StationConfig
{
public:
	StationConfig(void);
	~StationConfig(void);
public:
	 //加载某台站配置信息
	 bool load_station_config();
     //获得关联设备信息
     DeviceInfo * get_devinfo_by_id(string sDevId);
     //获取某台站某发射机对应的天线信息
	 AntennaInformation* get_antenna_by_host_transmitter(string sTransmitterNumber);
	 //获取某台站的发射机信息
	 vector<TransmitterInformation>& get_transmitter();
	 //获得所有天线信息
	 vector<AntennaInformation>& get_antenna();
	 //获得指定number的天线信息
	 //AntennaInformation& get_antenna_by_id(string sAntennaNumber);
	 //获得指定number的天线信息指针
	 AntennaInformation* get_antenna_ptr_by_id(string sAntennaNumber);
	 //所有协议转换器信息
     vector<ModleInfo>& get_Modle();
	 //获得所有发射机代理信息
     //vector<ModleInfo>& get_transmitter_agent();
	 //获得天线关联信息
	 AssociateInfo* get_antenna_associate_info(string sAntennaNumber);
	 //获得天线关联信息
     AssociateInfo* get_antenna_associate_info_by_backup(string sBackupTransmitterNumber);
	 //获得天线关联信息
     AssociateInfo* get_antenna_associate_info_by_host(string sHostTransmitterNumber);
	 //获得该等级的用户电话号码
	 vector<SendMSInfo>& get_sms_user_info();
	 //获得所有上传设备
	 vector<MediaDeviceParamInfo>& get_upload_dev_info();
	 //获得上传设备信息
	 bool get_upload_dev_info(int ndevAddr,MediaDeviceParamInfo& uploadDevInfo);
	 //获得设备基本信息
	 bool get_dev_base_info(string sStationId,string sDevId,DevBaseInfo &devBaseInfo);
	 //查找other设备对应的串口服务器id
	 string get_modle_id_by_devid(string sStationId,string sDevId);
	 //查找发射机代理设备管理器对应的串口服务器id
	 string get_transmitter_id_by_agent(string sStationId,string sDevId);
	 //发射机是否使用代理
	 bool tranmitter_is_use(string sTransmitterNumber);
	 //通过发射机id找所属moxa
	 bool get_transmitter_agent_endpoint_by_id(string sTransmitterId,string &sIp,int &sPort);

	 //获得所有主动连接媒体设备
	 vector<MediaDeviceParamInfo>& get_media_dev_info();
	 //获得制定地址码的信息
	 bool get_media_dev_info(int ndevAddr,MediaDeviceParamInfo& mediaDevInfo);
     //获得数据字典值
     string get_dictionary_value(string sType,int nCode);

     bool IsHaveGsm(){return bHave_GSM_model;}
     ComCommunicationMode getGsmInfo(){return Gsm_model_info;}

     bool get_dev_propery(string sDevId,string sName,string &sValue);

private:
	vector<TransmitterInformation>mapTransmitterInfo;//台站id为索引
    vector<AntennaInformation> mapAntennaInfo;//台站id为索引
    vector<AssociateInfo> mapAssociateInfo;//台站id为索引
    vector<ModleInfo> mapModleInfo;
    //vector<ModleInfo> mapTransmitAgentInfo;//发射机代理
	vector<MediaDeviceParamInfo> vecUploadDevInfo;//上传设备
	vector<MediaDeviceParamInfo> vecMediaDevInfo;//主动连接媒体设备
	vector<SendMSInfo> vSmsInfo;

	vector<LinkageInfo> vLinkageInfo;//告警联动配置信息

	string cur_station_id;//当前台站id
	string cur_devSvc_id; //当前服务id

    map<string,map<int,string> > mapDeviceDicry;

    bool                 bHave_GSM_model;
    ComCommunicationMode Gsm_model_info;
};
#endif
