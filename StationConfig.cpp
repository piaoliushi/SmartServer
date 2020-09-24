#include "StationConfig.h"
#include "LocalConfig.h"
#include "./net/config.h"
//#include "./protocol/bohui_protocol.h"
using namespace db;
StationConfig::StationConfig(void)
{

}

StationConfig::~StationConfig(void)
{

}

//加载某台站配置信息
bool StationConfig::load_station_config()
{
    mapModleInfo.clear();
    string sLocalStationId = GetInst(LocalConfig).local_station_id();
    string sLocalServerId  = GetInst(LocalConfig).local_dev_server_number();

    if(!GetInst(DataBaseOperation).GetAllDevInfo(mapModleInfo,sLocalStationId,sLocalServerId))
        return false;

    if(!GetInst(DataBaseOperation).GetDeviceDataDictionary(mapDeviceDicry))
        return false;

    if(!GetInst(DataBaseOperation).GetGSMInfo(bHave_GSM_model,Gsm_model_info))
        return false;

    if(!GetInst(DataBaseOperation).GetRemindInfoByServer(sLocalServerId,mapRemindInfo))
        return false;

    if(!GetInst(DataBaseOperation).GetRemindInfoByServer("00000000",mapDeviceCmdRemindInfo))
        return false;
    return true;
}

//获得数据字典值
string StationConfig::get_dictionary_value(string sType,int nCode)
{
    map<string,map<int,string> >::iterator iter = mapDeviceDicry.find(sType);
    if(iter!= mapDeviceDicry.end()){
        map<int,string>::iterator iter_code = iter->second.find(nCode);
        if(iter_code!= iter->second.end())
            return iter_code->second;
    }
    return "";
}


//获得关联设备信息
DeviceInfo *StationConfig::get_devinfo_by_id(string sDevId)
{
      vector<ModleInfo>::iterator iter = mapModleInfo.begin();
      for(;iter!=mapModleInfo.end();++iter){
          map<string,DeviceInfo>::iterator dev_iter = (*iter).mapDevInfo.find(sDevId);
          if(dev_iter!=(*iter).mapDevInfo.end())
              return &(dev_iter->second);

      }
    return NULL;
}


//获得协议转换器
vector<ModleInfo>& StationConfig::get_Modle()
{
	return mapModleInfo;
}


//获得该等级的用户电话号码
vector<SendMSInfo>& StationConfig::get_sms_user_info()
{
	return vSmsInfo;
}



//查找other设备对应的串口服务器id
string StationConfig::get_modle_id_by_devid(string sStationId,string sDevId)
{
    vector<ModleInfo>::iterator iter = mapModleInfo.begin();
    for(;iter!=mapModleInfo.end();++iter){
        map<string,DeviceInfo>::iterator dev_iter = (*iter).mapDevInfo.find(sDevId);
        if(dev_iter!=(*iter).mapDevInfo.end())
            return (*iter).sModleNumber;

    }
    return "";
}


//根据设备id与属性名,查找属性值
bool StationConfig::get_dev_propery(string sDevId,string sName,string &sValue){
    vector<ModleInfo>::iterator iter = mapModleInfo.begin();
    for(;iter!=mapModleInfo.end();++iter){
        map<string,DeviceInfo>::iterator dev_iter = (*iter).mapDevInfo.find(sDevId);
        if(dev_iter!=(*iter).mapDevInfo.end()){

            map<string,DevProperty>::iterator prp_iter = (dev_iter->second).map_DevProperty.find(sName);
            if(prp_iter!= (dev_iter->second).map_DevProperty.end()){

                sValue = (prp_iter->second).property_value;
                return true;
            }
        }
    }

    return false;
}

//获取提醒配置信息
map<string,Remind_Scheduler>&  StationConfig::get_all_remind_info()
{
    return mapRemindInfo;
}

Remind_Scheduler*  StationConfig::get_device_remind_info(string sNumber)
{
    map<string,Remind_Scheduler>::iterator iter_remind = mapDeviceCmdRemindInfo.find(sNumber);
    if(iter_remind!=mapDeviceCmdRemindInfo.end())
        return &(iter_remind->second);
    return 0;
}
