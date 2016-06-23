#include "StationConfig.h"
#include "LocalConfig.h"
#include "./net/config.h"
#include "./protocol/bohui_protocol.h"
using namespace db;
StationConfig::StationConfig(void)
{

}

StationConfig::~StationConfig(void)
{
	if(mapTransmitterInfo.size()>0)
		mapTransmitterInfo.clear();
}

//加载某台站配置信息
bool StationConfig::load_station_config()
{
    mapModleInfo.clear();
    if(!GetInst(DataBaseOperation).GetAllDevInfo(mapModleInfo))
        return false;
    if(!GetInst(DataBaseOperation).GetDataDictionary(Bohui_Protocol::mapTypeToStr))
        return false;
    if(!GetInst(DataBaseOperation).GetDeviceDataDictionary(mapDeviceDicry))
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

//获取某台站的发射机信息
vector<TransmitterInformation>& StationConfig::get_transmitter()
{
	return mapTransmitterInfo;
}

//获得所有天线信息
vector<AntennaInformation>& StationConfig::get_antenna()
{
	return mapAntennaInfo;
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

AntennaInformation* StationConfig::get_antenna_ptr_by_id(string sAntennaNumber)
{
	vector<AntennaInformation>::iterator iter=mapAntennaInfo.begin();
	for(;iter!=mapAntennaInfo.end();++iter)
	{
		if((*iter).sNumber == sAntennaNumber)
			return &(*iter);
	}
	return 0;
}

//获得协议转换器
vector<ModleInfo>& StationConfig::get_Modle()
{
	return mapModleInfo;
}

//获得发射机代理设备
/*vector<ModleInfo>& StationConfig::get_transmitter_agent()
{
	return mapTransmitAgentInfo;
}*/

//获得发射机代理ip
bool StationConfig::get_transmitter_agent_endpoint_by_id(string sTransmitterId,string &sIp,int &nPort)
{
/*	for(int i=0;i<mapTransmitAgentInfo.size();++i)
	{
		if(mapTransmitAgentInfo[i].mapDevInfo.find(sTransmitterId)!=
			mapTransmitAgentInfo[i].mapDevInfo.end())
		{
            //sIp = mapTransmitAgentInfo[i].sModIP;
            //nPort = mapTransmitAgentInfo[i].nModPort;
			return true;
		}
    }*/

	return false;
}


//获得该等级的用户电话号码
vector<SendMSInfo>& StationConfig::get_sms_user_info()
{
	return vSmsInfo;
}

//获得所有上传设备
vector<MediaDeviceParamInfo>& StationConfig::get_upload_dev_info()
{
	return vecUploadDevInfo;
}
//获得指定上传设备
bool StationConfig::get_upload_dev_info(int ndevAddr,MediaDeviceParamInfo& uploadDevInfo)
{
	vector<MediaDeviceParamInfo>::iterator iter=vecUploadDevInfo.begin();
	for(;iter!=vecUploadDevInfo.end();++iter)
	{
		if((*iter).nDevAddr == ndevAddr)
		{
			uploadDevInfo = (*iter);
			return true;
		}
	}
	return false;
}

//获得所有主动连接媒体设备
vector<MediaDeviceParamInfo>& StationConfig::get_media_dev_info()
{
	return vecMediaDevInfo;
}

//获得指定上传设备
bool StationConfig::get_media_dev_info(int ndevAddr,MediaDeviceParamInfo& mediaDevInfo)
{
	vector<MediaDeviceParamInfo>::iterator iter=vecMediaDevInfo.begin();
	for(;iter!=vecMediaDevInfo.end();++iter)
	{
		if((*iter).nDevAddr == ndevAddr)
		{
			mediaDevInfo = (*iter);
			return true;
		}
	}
	return false;
}

//发射机是否使用代理
bool StationConfig::tranmitter_is_use(string sTransmitterNumber)
{
	vector<TransmitterInformation>::iterator iter = mapTransmitterInfo.begin();
	for(;iter!=mapTransmitterInfo.end();++iter)
	{
		if((*iter).sNumber == sTransmitterNumber)
		{
			return (*iter).IsUsed;
		}
	}
	return false;
}

//由主机获得发射机对应的天线信息
AntennaInformation* StationConfig::get_antenna_by_host_transmitter(string sTransmitterNumber)
{
	vector<AssociateInfo>::iterator iterAss = mapAssociateInfo.begin();
	for(;iterAss!=mapAssociateInfo.end();++iterAss)
	{
		if((*iterAss).sHostNumber == sTransmitterNumber)
		{
			vector<AntennaInformation>::iterator iter = mapAntennaInfo.begin();
			for(;iter!=mapAntennaInfo.end();++iter)
			{
				if((*iter).sNumber == (*iterAss).sHostNumber)
					return &(*iter);
			}
		}
	}

	return 0;

}

//由天线获得天线关联信息
AssociateInfo* StationConfig::get_antenna_associate_info(string sAntennaNumber)
{
	vector<AssociateInfo>::iterator iter = mapAssociateInfo.begin();
	for(;iter!=mapAssociateInfo.end();++iter)
	{
		if((*iter).sAntennaNumber == sAntennaNumber)
			return &(*iter);
	}
	return 0;
}

//由主机获得天线关联信息
AssociateInfo* StationConfig::get_antenna_associate_info_by_host(string sHostTransmitterNumber)
{
	vector<AssociateInfo>::iterator iter = mapAssociateInfo.begin();
	for(;iter!=mapAssociateInfo.end();++iter)
	{
		if((*iter).sHostNumber == sHostTransmitterNumber)
			return &(*iter);
	}

	return 0;
}

//由备机获得天线关联信息
AssociateInfo* StationConfig::get_antenna_associate_info_by_backup(string sBackupTransmitterNumber)
{
	vector<AssociateInfo>::iterator iter = mapAssociateInfo.begin();
	for(;iter!=mapAssociateInfo.end();++iter)
	{
		if((*iter).sBackupNumber == sBackupTransmitterNumber)
			return &(*iter);
	}

	return 0;
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

//查找发射机代理设备管理器对应的串口服务器id
string StationConfig::get_transmitter_id_by_agent(string sStationId,string sDevId)
{
/*	for(int i=0;i<mapTransmitAgentInfo.size();++i)
	{
		if(mapTransmitAgentInfo[i].mapDevInfo.find(sDevId)!=mapTransmitAgentInfo[i].mapDevInfo.end())
			return mapTransmitAgentInfo[i].sModleNumber;
    }*/
	return "";
}


//获得设备基本信息
bool StationConfig::get_dev_base_info(string sStationId,string sDevId,DevBaseInfo &devBaseInfo)
{
	return false;
}

