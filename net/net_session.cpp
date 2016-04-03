#include "net_session.h"
#include "../StationConfig.h"
#include "SvcMgr.h"
#include "LocalConfig.h"
//#include "../sms/SmsTraffic.h"
//#include <glog/logging.h>
/*************************************************************************
* 说明：这是一个基类，客户端与设备连接handler需从此类派生，重载相关虚接口
        完成消息的插队，验证，通知派发操作。
*
*************************************************************************/
namespace hx_net
{

    net_session::net_session(boost::asio::io_service& io_service)
		:socket_(io_service)
		,udp_socket_(io_service)
		,bTcp_(true)
	{

	}

    net_session::~net_session(void)
	{

	}

    tcp::socket& net_session::socket()
	{
		boost::mutex::scoped_lock lock(socket_mutex_);
		//socket_.set_option(tcp::no_delay(true));
		return socket_;
	}

    udp::socket& net_session::usocket()
	{
		boost::mutex::scoped_lock(socket_mutex_);
		return udp_socket_;
	}
	
    bool  net_session::is_tcp()
	{
		return bTcp_;
	}

    void net_session::set_tcp(bool bTcp)
	{
		bTcp_ = bTcp;
	}
    tcp::endpoint net_session::get_addr()
	{
		return socket().remote_endpoint();
	}

    udp::endpoint net_session::get_udp_addr()
	{
		return usocket().remote_endpoint();
	}

    void net_session::close_i()
	{
		boost::system::error_code ignored_ec;
		boost::mutex::scoped_lock lock(socket_mutex_);
		if(bTcp_)
		{
			socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
			socket_.close(ignored_ec);
		}else
		{
			udp_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
			udp_socket_.close(ignored_ec);
		}

	}

    void net_session::handle_read_body(const boost::system::error_code& error,
		size_t bytes_transferred)
	{
	}

    void net_session::handle_read_head(const boost::system::error_code& error,
		size_t bytes_transferred)
	{
		
	}

    bool net_session::sendMessage(e_MsgType _type,googleMsgPtr gMsgPtr)//google::protobuf::Message *
	{
		return false;
	}


    void net_session::handle_write(const boost::system::error_code& error,size_t bytes_transferred)
	{
	}
	
	//打包发送实时多媒体数据（汇鑫760音频）
    void net_session::send_monitor_data_message(string sStationid,string sDevid,int devType,
                                            unchar_ptr curData,DeviceMonitorItem &mapMonitorItem)
	{
        /*devDataNfyMsgPtr dev_cur_data_ptr(new DevDataNotify);
		dev_cur_data_ptr->set_edevtype(devType);
		dev_cur_data_ptr->set_sstationid(sStationid);
		dev_cur_data_ptr->set_sdevid(sDevid);

		devDataNfyMsgPtr dev_cur_data_tosvr_ptr;
		if(GetInst(LocalConfig).upload_use()==true)
		{
			dev_cur_data_tosvr_ptr = devDataNfyMsgPtr(new DevDataNotify);
			dev_cur_data_tosvr_ptr->set_edevtype(devType);
			dev_cur_data_tosvr_ptr->set_sstationid(sStationid);
			dev_cur_data_tosvr_ptr->set_sdevid(sDevid);
		}

		DevDataNotify_eCellMsg *cell = dev_cur_data_ptr->add_ccelldata();
		cell->set_ecelltype((e_CellType)mapMonitorItem.nMonitoringType);
		cell->set_scellid(boost::lexical_cast<string>(mapMonitorItem.nMonitoringIndex-59));
		cell->set_scellname(QString::fromLocal8Bit(mapMonitorItem.sMonitoringName.c_str()).toUtf8().data());

		cell->set_baudiovalue(&curData->at(0),curData->size());//
		if(dev_cur_data_tosvr_ptr!=0)
		{
			if(mapMonitorItem.bIsUpload==true)
			{
				DevDataNotify_eCellMsg *upcell = dev_cur_data_tosvr_ptr->add_ccelldata();
				upcell->set_ecelltype((e_CellType)mapMonitorItem.nMonitoringType);
				upcell->set_scellid(boost::lexical_cast<string>(mapMonitorItem.nMonitoringIndex-59));
				upcell->set_scellname(QString::fromLocal8Bit(mapMonitorItem.sMonitoringName.c_str()).toUtf8().data());
				//cell->set_scellvalue(sValue);
				upcell->set_baudiovalue(&curData->at(0),curData->size());
			}
		}
        GetInst(SvcMgr).send_monitor_data_to_client(sStationid,sDevid,dev_cur_data_ptr,dev_cur_data_tosvr_ptr);*/
	}
	//打包发送761数据（Mp3）
    void net_session::send_monitor_data_message_ex(string sStationid,string sDevid,int devType,
                                unsigned char *curData,int nDataLen,DeviceMonitorItem &mapMonitorItem)
	{
        /*devDataNfyMsgPtr dev_cur_data_ptr(new DevDataNotify);
		dev_cur_data_ptr->set_edevtype(devType);
		dev_cur_data_ptr->set_sstationid(sStationid);
		dev_cur_data_ptr->set_sdevid(sDevid);

		devDataNfyMsgPtr dev_cur_data_tosvr_ptr;
		if(GetInst(LocalConfig).upload_use()==true)
		{
			dev_cur_data_tosvr_ptr = devDataNfyMsgPtr(new DevDataNotify);
			dev_cur_data_tosvr_ptr->set_edevtype(devType);
			dev_cur_data_tosvr_ptr->set_sstationid(sStationid);
			dev_cur_data_tosvr_ptr->set_sdevid(sDevid);
		}

		DevDataNotify_eCellMsg *cell = dev_cur_data_ptr->add_ccelldata();
		cell->set_ecelltype((e_CellType)mapMonitorItem.nMonitoringType);
		cell->set_scellid(boost::lexical_cast<string>(mapMonitorItem.nMonitoringIndex-59));
		cell->set_scellname(QString::fromLocal8Bit(mapMonitorItem.sMonitoringName.c_str()).toUtf8().data());
		cell->set_baudiovalue(curData,nDataLen);
		if(dev_cur_data_tosvr_ptr!=0)
		{
			if(mapMonitorItem.bIsUpload==true)
			{
				DevDataNotify_eCellMsg *upcell = dev_cur_data_tosvr_ptr->add_ccelldata();
				upcell->set_ecelltype((e_CellType)mapMonitorItem.nMonitoringType);
				upcell->set_scellid(boost::lexical_cast<string>(mapMonitorItem.nMonitoringIndex-59));
				upcell->set_scellname(QString::fromLocal8Bit(mapMonitorItem.sMonitoringName.c_str()).toUtf8().data());
				upcell->set_baudiovalue(curData,nDataLen);
			}
		}
        GetInst(SvcMgr).send_monitor_data_to_client(sStationid,sDevid,dev_cur_data_ptr,dev_cur_data_tosvr_ptr);*/
	}

	//打包发送实时数据消息
    void net_session::send_monitor_data_message(string sStationid,string sDevid,int devType,
                                            DevMonitorDataPtr curData,map<int,DeviceMonitorItem> &mapMonitorItem)
	{
		devDataNfyMsgPtr dev_cur_data_ptr(new DevDataNotify);
		dev_cur_data_ptr->set_edevtype(devType);
		dev_cur_data_ptr->set_sstationid(sStationid);
		dev_cur_data_ptr->set_sdevid(sDevid);

        devDataNfyMsgPtr dev_cur_data_tosvr_ptr;
        if(GetInst(LocalConfig).upload_use()==true){
			dev_cur_data_tosvr_ptr = devDataNfyMsgPtr(new DevDataNotify);
			dev_cur_data_tosvr_ptr->set_edevtype(devType);
			dev_cur_data_tosvr_ptr->set_sstationid(sStationid);
			dev_cur_data_tosvr_ptr->set_sdevid(sDevid);
		}

        map<int,DeviceMonitorItem>::iterator cell_iter = mapMonitorItem.begin();
        for(;cell_iter!=mapMonitorItem.end();++cell_iter){
			int cellId = (*cell_iter).first;			
			//未更新的监测量
            if(curData->mValues[cellId].bUpdate==false)
				continue;
			DevDataNotify_eCellMsg *cell = dev_cur_data_ptr->add_ccelldata();
            cell->set_ecelltype((e_CellType)(*cell_iter).second.iItemType);
            cell->set_scellid(boost::lexical_cast<string>((*cell_iter).second.iItemIndex));
            cell->set_scellname((*cell_iter).second.sItemName.c_str());
            string  sValue = str(boost::format("%.2f")%curData->mValues[cellId].fValue);
			cell->set_scellvalue(sValue);
            if(dev_cur_data_tosvr_ptr!=0){
                if((*cell_iter).second.bUpload==true){
					DevDataNotify_eCellMsg *upcell = dev_cur_data_tosvr_ptr->add_ccelldata();
                    upcell->CopyFrom(*cell);
				}
			}
		}

		GetInst(SvcMgr).send_monitor_data_to_client(sStationid,sDevid,dev_cur_data_ptr,dev_cur_data_tosvr_ptr);


	}

	//打包发送设备连接状态消息
    void net_session::send_net_state_message(string sStationid,string sDevid,string sDevName,int devType,
		                                 con_state netState)
	{
		devNetNfyMsgPtr  dev_net_nfy_ptr(new DevNetStatusNotify);
		DevNetStatus *dev_n_s = dev_net_nfy_ptr->add_cdevcurnetstatus();
		dev_n_s->set_edevtype(devType);
		dev_n_s->set_sstationid(sStationid);
		dev_n_s->set_sdevid(sDevid);
		dev_n_s->set_sdevname(QString::fromLocal8Bit(sDevName.c_str()).toUtf8().data());
		dev_n_s->set_enetstatus((DevNetStatus_e_NetStatus)netState);

		GetInst(SvcMgr).send_dev_net_state_to_client(sStationid,sDevid,dev_net_nfy_ptr);
	}

    void net_session::send_work_state_message( string sStationid,string sDevid,string sDevName,int devType, dev_run_state runState )
	{
		devWorkNfyMsgPtr dev_run_nfy_ptr(new DevWorkStatusNotify);// dev_run_nfy;
		DevWorkStatus *dev_n_s = dev_run_nfy_ptr->add_cdevcurworkstatus();
		dev_n_s->set_edevtype(devType);
		dev_n_s->set_sstationid(sStationid);
		dev_n_s->set_sdevid(sDevid);
		dev_n_s->set_sdevname(QString::fromLocal8Bit(sDevName.c_str()).toUtf8().data());
		dev_n_s->set_eworkstatus((DevWorkStatus_e_WorkStatus)runState);

		GetInst(SvcMgr).send_dev_work_state_to_client(sStationid,sDevid,dev_run_nfy_ptr);
	}
	
    void net_session::send_alarm_state_message(string sStationid,string sDevid,string sDevName,
                                           int nCellId,string sCellName,int devType,int  alarmState,
                                           string sStartTime,int alarmCount)
	{
		devAlarmNfyMsgPtr dev_alarm_nfy_ptr(new DevAlarmStatusNotify);
		DevAlarmStatus *dev_n_s = dev_alarm_nfy_ptr->add_cdevcuralarmstatus();
		dev_n_s->set_edevtype(devType);
		dev_n_s->set_sdevid(sDevid);
        dev_n_s->set_sdevname(sDevName.c_str());
		dev_n_s->set_nalarmcount(alarmCount);
		DevAlarmStatus_eCellAlarmMsg *dev_cell_alarm = dev_n_s->add_ccellalarm();
		std::string scellid = str(boost::format("%1%")%nCellId);
		dev_cell_alarm->set_scellid(scellid);
        dev_cell_alarm->set_scellname(sCellName.c_str());
		dev_cell_alarm->set_sstarttime(sStartTime);
		dev_cell_alarm->set_ccellstatus((e_AlarmStatus)alarmState);

		GetInst(SvcMgr).send_dev_alarm_state_to_client(sStationid,sDevid,dev_alarm_nfy_ptr);
	}

    void net_session::send_command_execute_result_message(string sStationid,string sDevid,int devType,string sDevName,
													  string sUsrName,e_MsgType nMsgType,e_ErrorCode eResult)
	{
		devCommdRsltPtr ackMsgPtr(new DeviceCommandResultNotify);
		ackMsgPtr->set_sstationid(sStationid);
		ackMsgPtr->set_sdevid(sDevid);
        ackMsgPtr->set_sdevname(sDevName.c_str());
        ackMsgPtr->set_edevtype((e_DevType)devType);
		ackMsgPtr->set_eerrorid(eResult);
        ackMsgPtr->set_soperuser(sUsrName.c_str());

		GetInst(SvcMgr).send_command_execute_result(sStationid,sDevid,nMsgType,ackMsgPtr);
	}
 
	//判断是否发短信与打电话
    void net_session::sendSmsAndCallPhone(int nAlarmLevel,string sContent)
	{

		vector<SendMSInfo>& smsInfo=GetInst(StationConfig).get_sms_user_info();
		
		for(int i=0;i<smsInfo.size();++i)
		{
			if(smsInfo[i].iAlarmLevel==nAlarmLevel)
			{
				//如果告警级别需要发送短信，且短信使能打开
				if((nAlarmLevel==SMS || nAlarmLevel==SMSANDTEL)&&
					GetInst(LocalConfig).sms_use())
				{
					string sCenterId = GetInst(LocalConfig).sms_center_number();
				}
                //发送短信2016-3-22
                //GetInst(DbManager).WriteCallTask(smsInfo[i].sPhoneNumber,sContent,nAlarmLevel);
			}
		}
	}
}
