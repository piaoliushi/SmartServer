#include "DevClient.h"
#include "client_session.h"
//#include "Transmitter_session.h"
//#include "Antenna_session.h"
#include "StationConfig.h"
#include "device_session.h"
#include "./dev_message/104/iec104_types.h"
#include "LocalConfig.h"
//#include <glog/logging.h>
//--------------------remark--------------------------------------//
//@author:liukun
//@data:2013-11-16
//@
//@增加汇鑫发射机数据管理器(该管理器可以连接1-2部发射机或993采集器与1部汇鑫天线控制器),本管理对象需区分该设备连接所
//关联的设备信息,自动区分无需连接的设备,自动装载需要连接的设备.
namespace hx_net
{
	DevClient::DevClient(TaskQueue<msgPointer>& taskwork,size_t io_service_pool_size/* =4 */)
		:io_service_pool_(io_service_pool_size)
		,taskwork_(taskwork)
	{

	}

	DevClient::~DevClient()
	{

		device_pool_.clear();
	}
	
	//连接上级服务器
	void DevClient::connect_relay_server()
	{
		//创建与上一级设备服务器的连接
		boost::recursive_mutex::scoped_lock lock(relay_svc_session_mutex_);
		if(GetInst(LocalConfig).upload_use()==true)
		{
			relay_server_seesion_ = session_ptr(new client_session(io_service_pool_.get_io_service(),taskwork_));
			relay_server_seesion_->connect(GetInst(LocalConfig).relay_svc_ip(),GetInst(LocalConfig).relay_svc_port(),true);
		}
	}

	//连接告警决策服务器
	void DevClient::connect_alarm_server()
	{
		//创建与上一级设备服务器的连接
		boost::recursive_mutex::scoped_lock lock(alarm_svc_session_mutex_);
		if(GetInst(LocalConfig).alarm_upload_use()==true)
		{
			alarm_server_seesion_ = session_ptr(new client_session(io_service_pool_.get_io_service(),taskwork_));
			alarm_server_seesion_->connect(GetInst(LocalConfig).alarm_svc_ip(),GetInst(LocalConfig).alarm_svc_port(),true);
		}
	}

	//连接所有设备
	void DevClient::connect_all()
	{
        return;
        /*string sLocalStationId = GetInst(LocalConfig).local_station_id();
		boost::recursive_mutex::scoped_lock lock(device_pool_mutex_);

		vector<ModleInfo> &vModle = GetInst(StationConfig).get_Modle();
		vector<ModleInfo>::iterator modle_iter = vModle.begin();
		for(;modle_iter != vModle.end();modle_iter++)
		{
			if((*modle_iter).sStationNumber!=sLocalStationId && (*modle_iter).nCommType!=2)//2:直连，0：被动，1：主动
				continue;
			session_ptr new_session(new device_session(io_service_pool_.get_io_service(),taskwork_,(*modle_iter)));
			if(device_pool_.find(DevKey((*modle_iter).sStationNumber,(*modle_iter).sModleNumber))==device_pool_.end())
			{
				device_pool_[DevKey((*modle_iter).sStationNumber,(*modle_iter).sModleNumber)]=new_session;

				if((*modle_iter).nConnectType==0)
					new_session->connect((*modle_iter).sModIP,(*modle_iter).nModPort);
				else if((*modle_iter).nConnectType==3)
					new_session->udp_connect((*modle_iter).sModIP,(*modle_iter).nModPort);
			}
			return;
        }*/

		//连接级联服务器
        //connect_relay_server();

		//连接告警决策服务器
        //connect_alarm_server();
	}

	void DevClient::disconnect_all()
	{		
		{
			boost::recursive_mutex::scoped_lock lock(device_pool_mutex_);
			std::map<DevKey,session_ptr>::iterator iter = device_pool_.begin();
			for(;iter!=device_pool_.end();iter++)
				(*iter).second->disconnect();
		}

 		if(GetInst(LocalConfig).upload_use()==true){
 			boost::recursive_mutex::scoped_lock lock(relay_svc_session_mutex_);
 			if(relay_server_seesion_)
 				relay_server_seesion_->disconnect();
 		}

		if(GetInst(LocalConfig).alarm_upload_use()==true){
			boost::recursive_mutex::scoped_lock lock(alarm_svc_session_mutex_);
			if(alarm_server_seesion_)
				alarm_server_seesion_->disconnect();
		}
	}

	void DevClient::run()
	{
		io_service_pool_.run();
	}
	void DevClient::stop()
	{
		io_service_pool_.stop();
	}

	//获得设备连接
	con_state DevClient::get_dev_net_state(string sStationId,string sDevid)
	{
		boost::recursive_mutex::scoped_lock lock(device_pool_mutex_);
		std::map<DevKey,session_ptr>::iterator iter = device_pool_.begin();
		for(;iter!=device_pool_.end();++iter)
		{
			if(iter->first.stationId == sStationId)
			{
				if(iter->second->dev_type()==DEV_OTHER )
				{
					if(iter->second->is_contain_dev(sDevid))
						return iter->second->get_con_state();
				}
				else if(iter->second->dev_type()==DEV_TRANSMITTER_AGENT)//发射机代理设备
				{
					if(iter->second->is_contain_dev(sDevid))
						return iter->second->get_child_con_state(sDevid);
				}
				else if(iter->first.devId == sDevid)
				{
					return iter->second->get_con_state();
				}
			}

		}

		return con_disconnected;
	}
	//获得设备运行状态
	dev_run_state DevClient::get_dev_run_state(string sStationId,string sDevid)
	{
		boost::recursive_mutex::scoped_lock lock(device_pool_mutex_);
		std::map<DevKey,session_ptr>::iterator iter = device_pool_.begin();
		for(;iter!=device_pool_.end();++iter)
		{
			if(iter->first.stationId == sStationId)
			{
				if(iter->second->dev_type()==DEV_OTHER )
				{
					if(iter->second->is_contain_dev(sDevid))
						return iter->second->get_run_state();
				}
				else if(iter->second->dev_type()==DEV_TRANSMITTER_AGENT)//发射机代理设备
				{
					if(iter->second->is_contain_dev(sDevid))
						return iter->second->get_child_run_state(sDevid);
				}
				else if(iter->first.devId == sDevid)
				{
					return iter->second->get_run_state();
				}
			}

		}

		return dev_unknown;
	}
	//获得设备运行状态
    void DevClient::get_dev_alarm_state(string sStationId,string sDevid,map<int,std::pair<int,tm> >& cellAlarm)
	{
		boost::recursive_mutex::scoped_lock lock(device_pool_mutex_);
		std::map<DevKey,session_ptr>::iterator iter = device_pool_.begin();
		for(;iter!=device_pool_.end();++iter)
		{
			if(iter->first.stationId == sStationId)
			{
				if(iter->second->dev_type()==DEV_OTHER )
				{
					if(iter->second->is_contain_dev(sDevid))
						return iter->second->get_alarm_state(sDevid,cellAlarm);
				}
				else if(iter->second->dev_type()==DEV_TRANSMITTER_AGENT)//发射机代理设备
				{
					if(iter->second->is_contain_dev(sDevid))
						return iter->second->get_alarm_state(sDevid,cellAlarm);
				}
				else if(iter->first.devId == sDevid)
				{
					return iter->second->get_alarm_state(cellAlarm);
				}
			}

		}
		return ;
	}

	bool DevClient::dev_base_info(string sStationId,DevBaseInfo& devInfo,string sDevid)
	{
		bool bRtValue = false;
		boost::recursive_mutex::scoped_lock lock(device_pool_mutex_);
		std::map<DevKey,session_ptr>::iterator iter = device_pool_.begin();
		for(;iter!=device_pool_.end();++iter)
		{
			if(iter->first.stationId == sStationId){
				if(iter->second->dev_type()==DEV_OTHER ){
					if(iter->second->is_contain_dev(sDevid)){
						iter->second->dev_base_info(devInfo,sDevid);
						bRtValue = true;
					}
				}else if(iter->second->dev_type()==DEV_TRANSMITTER_AGENT){
					if(iter->second->is_contain_dev(sDevid)){
						iter->second->dev_base_info(devInfo,sDevid);
						bRtValue = true;
					}
				}else if(iter->first.devId == sDevid){
					iter->second->dev_base_info(devInfo);
					bRtValue = true;
				}
			}
		}

		return bRtValue;
	}

	//通用命令执行
	e_ErrorCode DevClient::excute_command(int cmdType,devCommdMsgPtr lpParam)
	{
		boost::recursive_mutex::scoped_lock lock(device_pool_mutex_);
		e_ErrorCode opr_rlt = EC_DEVICE_NOT_FOUND;
		std::map<DevKey,session_ptr>::iterator iter = device_pool_.find(DevKey(lpParam->sstationid(),lpParam->sdevid()));
		if(iter!=device_pool_.end())//直连设备
			(*iter).second->excute_command(cmdType,lpParam,opr_rlt);
		else
		{
			string sMoxaId = GetInst(StationConfig).get_modle_id_by_devid(lpParam->sstationid(),lpParam->sdevid());
			if(!sMoxaId.empty())
			{
				std::map<DevKey,session_ptr>::iterator iter = device_pool_.find(DevKey(lpParam->sstationid(),sMoxaId));
				if(iter!=device_pool_.end())//一带多设备
					(*iter).second->excute_command(cmdType,lpParam,opr_rlt);
			}
		}
		return opr_rlt;
	}

	int DevClient::get_modle_online_count()
	{
		boost::recursive_mutex::scoped_lock lock(device_pool_mutex_);
		int ncount=0;
		std::map<DevKey,session_ptr>::iterator iter = device_pool_.begin();
		for(;iter!=device_pool_.end();iter++)
		{
			if((*iter).second->dev_type()!=DEV_OTHER && 
				(*iter).second->dev_type()!=DEV_TRANSMITTER_AGENT)
				continue;
			if((*iter).second->is_connected())
				++ncount;
		}
		return (ncount>=0)?ncount:-1;
	}

	//----------------------relay server-------------------------------------------------//
	//发送设备数据给上级平台
	void DevClient::send_dev_data(string sStationid,string sDevid,devDataNfyMsgPtr &dataPtr)
	{
		boost::recursive_mutex::scoped_lock lock(relay_svc_session_mutex_);
		if(relay_server_seesion_==0)
			return;
		relay_server_seesion_->sendMessage(MSG_DEV_REALTIME_DATA_NOTIFY,dataPtr);
	}

	//发送设备网络状态数据
	void DevClient::send_dev_net_state_data(string sStationid,string sDevid,devNetNfyMsgPtr &netPtr)
	{
		boost::recursive_mutex::scoped_lock lock(relay_svc_session_mutex_);
		if(relay_server_seesion_==0)
			return;
		relay_server_seesion_->sendMessage(MSG_DEV_NET_STATE_NOTIFY,netPtr);
	}

	void DevClient::send_dev_work_state_data(string sStationid,string sDevid,devWorkNfyMsgPtr &workPtr)
	{
		boost::recursive_mutex::scoped_lock lock(relay_svc_session_mutex_);
		if(relay_server_seesion_==0)
			return;
		relay_server_seesion_->sendMessage(MSG_DEV_WORK_STATE_NOTIFY,workPtr);
	}

	//发送设备报警状态数据
	void DevClient::send_dev_alarm_state_data(string sStationid,string sDevid,devAlarmNfyMsgPtr &alarmPtr)
	{
		boost::recursive_mutex::scoped_lock lock(relay_svc_session_mutex_);
		if(relay_server_seesion_==0)
			return;
		relay_server_seesion_->sendMessage(MSG_DEV_ALARM_STATE_NOTIFY,alarmPtr);
	}

	//发送控制执行结果通知
	void DevClient::send_command_execute_result(string sStationid,string sDevid,e_MsgType nMsgType,devCommdRsltPtr &commdRsltPtr)
	{
		boost::recursive_mutex::scoped_lock lock(relay_svc_session_mutex_);
		if(relay_server_seesion_==0)
			return;
		relay_server_seesion_->sendMessage(nMsgType,commdRsltPtr);
	}

	//向上提交查岗结果
	void DevClient::commit_check_working_result(checkWorkingNotifyMsgPtr pcheckWorkResult)
	{
		boost::recursive_mutex::scoped_lock lock(relay_svc_session_mutex_);
		if(relay_server_seesion_==0)
			return;
		relay_server_seesion_->sendMessage(MSG_CHECK_WORKING_NOTIFY,pcheckWorkResult);
	}
	//---------------------------end-----------------------------------------------------//

	//是否是通过上级平台直连下级平台设备
	bool DevClient::is_direct_connect_device(string sStationId,string sDevNumber)
	{
		boost::recursive_mutex::scoped_lock lock(device_pool_mutex_);
		std::map<DevKey,session_ptr>::iterator iter = device_pool_.find(DevKey(sStationId,sDevNumber));
		if(iter!=device_pool_.end())
			return true;
		return false;
	}
}
