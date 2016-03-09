#include "connect_subSvc_handler.h"
#include "../SvcMgr.h"
#include "../../LocalConfig.h"
//接受下级服务器主动连接上来的handler
//与下级服务器建立心跳保持机制
//注意：下级服务器主动连接上级服务器，等待上级服务器发送注册请求，上级服务器缓存下级服务器提交的所有
//       设备状态信息并发送给上级服务器与在线客户端。
namespace net
{
	connect_subsvc_handler::connect_subsvc_handler(boost::asio::io_service& io_service, 
        TaskQueue<msgPointer>& taskwork,LocalServer &srv)
		:session(io_service)
		,h_b_timer_(io_service) 
#ifdef USE_CLIENT_STRAND
		, strand_(io_service)
#endif 
		,taskwork_(taskwork)
		,receive_msg_ptr_(new message)
		,srv_(srv)
		,task_count_(0)
		,usr_state_(usr_offline)
		,child_station_dev_status_ptr(new LoginAck)
		,child_station_dev_status_weakptr(child_station_dev_status_ptr)
	{

	}

	connect_subsvc_handler::~connect_subsvc_handler(void)
	{
	}

	//关闭连接，并清理自己
	void connect_subsvc_handler::close_handler()
	{
		h_b_timer_.cancel();
		close_i();
		reset_netstate();//复位网络状态
		
		wait_task_end();//等待任务结束

		srv_.remove_subServer_connection_handler(shared_from_this());
	}

	//启动心跳定时器
	void connect_subsvc_handler::start_hb_timer()
	{
		h_b_timer_.expires_from_now(boost::posix_time::seconds(60));
		h_b_timer_.async_wait(strand_.wrap(
			boost::bind(&connect_subsvc_handler::handle_timeout,this,
			boost::asio::placeholders::error))); 
	}

	//心跳超时
	void connect_subsvc_handler::handle_timeout(const boost::system::error_code& e)
	{
		if (!e)
		{
			std::time_t now = time(0);
			if (now - get_last_hb_time() >180)//超时
			{
 				close_handler();
				cout<<"heartbeat timeout!"<<endl;
				return;
			}			
			else
			{
				start_hb_timer();
			}
				
		}
		
	}

	//获得最后心跳时间
	std::time_t connect_subsvc_handler::get_last_hb_time()
	{
		boost::mutex::scoped_lock lock(hb_mutex_);
		return last_hb_time_;
	}

	//设置最后心跳时间
	void connect_subsvc_handler::set_last_hb_time(std::time_t t)
	{
		boost::mutex::scoped_lock lock(hb_mutex_);
		last_hb_time_ = t;
	}
// 
// 	bool connect_subsvc_handler::is_online()
// 	{
// 		if(get_state()==usr_online)
// 			return true;
// 		return false;
// 	}
	//回复心跳请求
	int connect_subsvc_handler::heart_beat_rsp()
	{
		std::time_t now = time(0);
		set_last_hb_time(now);
		googleMsgPtr nullMsg;
		if(!sendMessage(MSG_HEARTBEAT_ACK,nullMsg))
		{
			std::cout << "send heartbeat Ack error"<< std::endl;
			return -1;
		}

		cout<<"send heart beat ack successful!"<<endl;
		return 0;
	}

	//向下级服务器发送注册请求
	void connect_subsvc_handler::login_user(string sUser,string sPassword)
	{
		loginReqMsgPtr sloginReq(new LoginReq);
		//LoginReq sloginReq;
		sloginReq->set_susrname(sUser);
		sloginReq->set_susrpsw(sPassword);
		sendMessage(MSG_LOGIN_REQ,sloginReq);
	}

	void connect_subsvc_handler::logout_user(string sUser,string sPassword)
	{
		// 		LogoutReq slogoutReq;
		// 		slogoutReq.set_susrname(QString::fromLocal8Bit(sUser.c_str()).toUtf8().data());
		// 		slogoutReq.set_susrpsw(QString::fromLocal8Bit(sPassword.c_str()).toUtf8().data());
		// 		sendMessage(MSG_LOGOUT_REQ,&slogoutReq);
	}
	//重新初始化下级台站的设备状态信息
	void connect_subsvc_handler::login_user_ack(string sUser,string sPassword,msgPointer &pMsg)
	{

		boost::recursive_mutex::scoped_lock lock(child_station_s_mutex);
		child_station_dev_status_ptr->Clear();
		//int nbodySize = pMsg->bodySize();
		child_station_dev_status_ptr->ParseFromArray(pMsg->body(),pMsg->bodySize());
		
		string sUsr = child_station_dev_status_ptr->usrname().c_str();//对应台站号
		string sUsrNumber = child_station_dev_status_ptr->usrnumber().c_str();//对应服务编号
		
		if(!srv_.user_subServer_login(shared_from_this(),sUsr,sUsrNumber,*child_station_dev_status_ptr))
			close_handler();
		else
		{
			tcp::endpoint remote_add = get_addr();
			GetInst(SvcMgr).get_notify()->OnClientLogin(remote_add.address().to_string(),remote_add.port(),sUsr,sUsrNumber);
			//分析状态数据
			//设备网络连接状态
			const google::protobuf::RepeatedPtrField<DevNetStatus> vNetState = child_station_dev_status_ptr->cdevcurnetstatus();
			google::protobuf::RepeatedPtrField<DevNetStatus>::const_iterator iter_net = vNetState.begin();
			for(;iter_net!=vNetState.end();++iter_net)
			{
				string sStationId = (*iter_net).sstationid();
				string sDevId = (*iter_net).sdevid();
				cout<<sStationId<<"->"<<sDevId<<endl;
				DevNetStatus_e_NetStatus sState = (*iter_net).enetstatus();
				GetInst(SvcMgr).get_notify()->OnConnected(sDevId,sState);
				devNetNfyMsgPtr dev_netstate_ptr(new DevNetStatusNotify);
				dev_netstate_ptr->add_cdevcurnetstatus()->CopyFrom((*iter_net));
				GetInst(SvcMgr).send_dev_net_state_to_client(sStationId,sDevId,dev_netstate_ptr);
				
				//初始化所有设备的定时保存时间
				init_sava_data_time(sDevId);
			}
			//设备工作状态
			const google::protobuf::RepeatedPtrField<DevWorkStatus> vWorkState = child_station_dev_status_ptr->cdevcurworkstatus();
			google::protobuf::RepeatedPtrField<DevWorkStatus>::const_iterator iter_work = vWorkState.begin();
			for(;iter_work!=vWorkState.end();++iter_work)
			{
				string sStationId = (*iter_work).sstationid();
				string sDevId = (*iter_work).sdevid();
				cout<<sStationId<<"->"<<sDevId<<endl;
				DevWorkStatus_e_WorkStatus sState = (*iter_work).eworkstatus();
				GetInst(SvcMgr).get_notify()->OnRunState(sDevId,sState);

				devWorkNfyMsgPtr dev_workstate_ptr(new DevWorkStatusNotify);
				dev_workstate_ptr->add_cdevcurworkstatus()->CopyFrom((*iter_work));
				GetInst(SvcMgr).send_dev_work_state_to_client(sStationId,sDevId,dev_workstate_ptr);
			}
			
			//设备报警状态
			const google::protobuf::RepeatedPtrField<DevAlarmStatus> vAlarmState =child_station_dev_status_ptr->cdevcuralarmstatus();
			google::protobuf::RepeatedPtrField<DevAlarmStatus>::const_iterator iter_alarm = vAlarmState.begin();
			for(;iter_alarm!=vAlarmState.end();++iter_alarm)
			{
				string sStationId = (*iter_alarm).sstationid();
				string sDevId = (*iter_alarm).sdevid();

				const google::protobuf::RepeatedPtrField<DevAlarmStatus_eCellAlarmMsg> vcellState = (*iter_alarm).ccellalarm();
				google::protobuf::RepeatedPtrField<DevAlarmStatus_eCellAlarmMsg>::const_iterator iter_cell_alarm = vcellState.begin();
				for(;iter_cell_alarm!=vcellState.end();++iter_cell_alarm)
				{
					check_and_update_alarm_state(sDevId,(*iter_alarm),(*iter_cell_alarm));
				}

				devAlarmNfyMsgPtr dev_alarmstate_ptr(new DevAlarmStatusNotify);
				dev_alarmstate_ptr->add_cdevcuralarmstatus()->CopyFrom((*iter_alarm));
				GetInst(SvcMgr).send_dev_alarm_state_to_client(sStationId,sDevId,dev_alarmstate_ptr);
			}
		}

	}

	loginAckMsgPtr  connect_subsvc_handler::get_child_station_dev_status()
	{
		boost::recursive_mutex::scoped_lock lock(child_station_s_mutex);
		return child_station_dev_status_ptr;
	}

	void connect_subsvc_handler::logout_user_ack(string sUser,string sPassword,msgPointer &pMsg)
	{
		//LogoutAck sloginOutAck;
		//srv_.user_logout(shared_from_this(),sUser,sPassword,sloginOutAck);
		//sendMessage(MSG_LOGOUT_ACK,&sloginOutAck);
	}

	//下级台站上传的实时数据
	void connect_subsvc_handler::dev_data_notify(msgPointer &pMsg)
	{
		devDataNfyMsgPtr dev_cur_data(new DevDataNotify);
		dev_cur_data->ParseFromArray(pMsg->body(),pMsg->bodySize());

		//下级台站数据既广播给客户端也转给更上级服务器
		GetInst(SvcMgr).send_monitor_data_to_client(dev_cur_data->sstationid(),dev_cur_data->sdevid()
																						    ,dev_cur_data,dev_cur_data);
		//采用线程池处理数据(保存到数据库)
		if(boost::detail::thread::singleton<boost::threadpool::pool>::instance()
			.schedule(boost::bind(&connect_subsvc_handler::handler_data,this,dev_cur_data)))
		{
			task_count_increase();
		}
	}
	
	void connect_subsvc_handler::handler_data(devDataNfyMsgPtr curData)
	{
		//判断是否保存数据
		if(curData->edevtype()!=AUDIO)	{
			if(is_need_save_data(curData->sdevid())){
                GetInst(DbManager).SaveTransmitterMonitoringData(curData);
			}
		}
		task_count_decrease();
	}

	//提交任务
	void connect_subsvc_handler::task_count_increase()
	{
		boost::mutex::scoped_lock lock(task_mutex_);
		++task_count_;
	}
	//任务递减
	void connect_subsvc_handler::task_count_decrease()
	{
		boost::mutex::scoped_lock lock(task_mutex_);
		--task_count_;
		task_end_conditon_.notify_all();
	}

	int connect_subsvc_handler::task_count()
	{
		boost::mutex::scoped_lock lock(task_mutex_);
		return task_count_;
	}

	//等待任务结束
	void connect_subsvc_handler::wait_task_end()
	{
		boost::mutex::scoped_lock lock(task_mutex_);
		while(task_count_>0)
		{
			task_end_conditon_.wait(task_mutex_);
		}
	}

	void connect_subsvc_handler::init_sava_data_time(const string &sDevId)
	{
		boost::recursive_mutex::scoped_lock lock(dev_data_save_mutex);
		if(tmLastSaveTime.find(sDevId)==tmLastSaveTime.end())
				tmLastSaveTime.insert(std::make_pair(sDevId,time(0)));
	}
	//判断当前时间是否需要保存监控数据
	bool connect_subsvc_handler::is_need_save_data(string sDevId)
	{
		boost::recursive_mutex::scoped_lock lock(dev_data_save_mutex);
		if(tmLastSaveTime.find(sDevId)==tmLastSaveTime.end())
			return false;

		time_t tmCurTime;
		time(&tmCurTime);
		double ninterval = difftime(tmCurTime,tmLastSaveTime[sDevId]);
		if(ninterval<GetInst(LocalConfig).relay_data_save_interval()*60)//默认15分钟间隔保存时间 need amend;
			return false;
		tmLastSaveTime[sDevId] = tmCurTime;
		return true;
	}

	//复位所有设备网络状态为离线
	void connect_subsvc_handler::reset_netstate()
	{
		boost::recursive_mutex::scoped_lock lock(child_station_s_mutex);
		google::protobuf::RepeatedPtrField<DevNetStatus> *vNetState = child_station_dev_status_ptr->mutable_cdevcurnetstatus();
		google::protobuf::RepeatedPtrField<DevNetStatus>::iterator iter_net = vNetState->begin();
		for(;iter_net!=vNetState->end();++iter_net)
		{
			string sStationId = (*iter_net).sstationid();
			string sDevId = (*iter_net).sdevid();
			devNetNfyMsgPtr dev_netstate_ptr(new DevNetStatusNotify);
			DevNetStatus *dev_n_s = dev_netstate_ptr->add_cdevcurnetstatus();
			dev_n_s->CopyFrom((*iter_net));
			dev_n_s->set_enetstatus(DevNetStatus_e_NetStatus_OFFLINE);
			GetInst(SvcMgr).send_dev_net_state_to_client(sStationId,sDevId,dev_netstate_ptr);
		}
	}
	//下级台站更新设备网络连接状态,刷新缓存状态
	void  connect_subsvc_handler::dev_netstate_update(msgPointer &pMsg)
	{
		 devNetNfyMsgPtr dev_netstate_ptr(new DevNetStatusNotify);
		 dev_netstate_ptr->ParseFromArray(pMsg->body(),pMsg->bodySize());
		 const google::protobuf::RepeatedPtrField<DevNetStatus> vNetState = dev_netstate_ptr->cdevcurnetstatus();
		 google::protobuf::RepeatedPtrField<DevNetStatus>::const_iterator iter_net = vNetState.begin();
		 for(;iter_net!=vNetState.end();++iter_net)
		 {
			 string sStationId = (*iter_net).sstationid();
			 string sDevId = (*iter_net).sdevid();
			 DevNetStatus_e_NetStatus sState = (*iter_net).enetstatus();
			 GetInst(SvcMgr).get_notify()->OnConnected(sDevId,sState);
			 GetInst(SvcMgr).send_dev_net_state_to_client(sStationId,sDevId,dev_netstate_ptr);
			 if(check_and_update_net_state((*iter_net))==true)
				 return;
		 }
			 
	}
	//检查并更新当前设备状态
	bool connect_subsvc_handler::check_and_update_net_state(const DevNetStatus &netState)
	{
		boost::recursive_mutex::scoped_lock lock(child_station_s_mutex);
		google::protobuf::RepeatedPtrField<DevNetStatus> *vNetState = child_station_dev_status_ptr->mutable_cdevcurnetstatus();
		google::protobuf::RepeatedPtrField<DevNetStatus>::iterator iter_net = vNetState->begin();
		for(;iter_net!=vNetState->end();++iter_net)
		{
			string sDevId = (*iter_net).sdevid();
			if(sDevId == netState.sdevid())
			{
				 DevNetStatus_e_NetStatus sState = netState.enetstatus();
				(*iter_net).set_enetstatus(sState);
				return true;
			}
		}
		return false;
	}

	//下级台站更新设备工作运行状态
	void  connect_subsvc_handler::dev_workstate_update(msgPointer &pMsg)
	{
		devWorkNfyMsgPtr dev_workstate_ptr(new DevWorkStatusNotify);
		dev_workstate_ptr->ParseFromArray(pMsg->body(),pMsg->bodySize());
		const google::protobuf::RepeatedPtrField<DevWorkStatus> vWorkState = dev_workstate_ptr->cdevcurworkstatus();
		google::protobuf::RepeatedPtrField<DevWorkStatus>::const_iterator iter_work = vWorkState.begin();
		for(;iter_work!=vWorkState.end();++iter_work)
		{
			string sStationId = (*iter_work).sstationid();
			string sDevId = (*iter_work).sdevid();
			DevWorkStatus_e_WorkStatus sState = (*iter_work).eworkstatus();
			GetInst(SvcMgr).get_notify()->OnRunState(sDevId,sState);
			GetInst(SvcMgr).send_dev_work_state_to_client(sStationId,sDevId,dev_workstate_ptr);
			if(check_and_update_work_state((*iter_work))==true)
				return;
		}
	}
	//检查并更新当前工作状态
	bool  connect_subsvc_handler::check_and_update_work_state(const DevWorkStatus &workState)
	{
		boost::recursive_mutex::scoped_lock lock(child_station_s_mutex);
		google::protobuf::RepeatedPtrField<DevWorkStatus> *vWorkState = child_station_dev_status_ptr->mutable_cdevcurworkstatus();
		google::protobuf::RepeatedPtrField<DevWorkStatus>::iterator iter_work = vWorkState->begin();
		for(;iter_work!=vWorkState->end();++iter_work)
		{
			string sDevId = (*iter_work).sdevid();
			if(sDevId == workState.sdevid())
			{
				DevWorkStatus_e_WorkStatus sState = workState.eworkstatus();
				(*iter_work).set_eworkstatus(sState);
				return true;
			}
		}
		return false;
	}

	//下级台站更新设备报警状态
	void  connect_subsvc_handler::dev_alarmstate_update(msgPointer &pMsg)
	{
		devAlarmNfyMsgPtr dev_alarmstate_ptr(new DevAlarmStatusNotify);
		dev_alarmstate_ptr->ParseFromArray(pMsg->body(),pMsg->bodySize());
		const google::protobuf::RepeatedPtrField<DevAlarmStatus> vAlarmState = dev_alarmstate_ptr->cdevcuralarmstatus();
		google::protobuf::RepeatedPtrField<DevAlarmStatus>::const_iterator iter_alarm = vAlarmState.begin();
		for(;iter_alarm!=vAlarmState.end();++iter_alarm)
		{
			string sStationId = (*iter_alarm).sstationid();
			string sDevId = (*iter_alarm).sdevid();
			const google::protobuf::RepeatedPtrField<DevAlarmStatus_eCellAlarmMsg> vcellState = (*iter_alarm).ccellalarm();
			google::protobuf::RepeatedPtrField<DevAlarmStatus_eCellAlarmMsg>::const_iterator iter_cell_alarm = vcellState.begin();
			for(;iter_cell_alarm!=vcellState.end();++iter_cell_alarm)
			{
				check_and_update_alarm_state(sDevId,(*iter_alarm),(*iter_cell_alarm));
			}
			GetInst(SvcMgr).send_dev_alarm_state_to_client(sStationId,sDevId,dev_alarmstate_ptr);
		}
	}

	//检查并更新当前报警状态
	bool  connect_subsvc_handler::check_and_update_alarm_state(string sDevNumber,const DevAlarmStatus &devAlarmState
		                                                      ,const DevAlarmStatus_eCellAlarmMsg &cellAlarmState)
	{
		boost::recursive_mutex::scoped_lock lock(child_station_s_mutex);
		google::protobuf::RepeatedPtrField<DevAlarmStatus> *vAlarmState = child_station_dev_status_ptr->mutable_cdevcuralarmstatus();
		google::protobuf::RepeatedPtrField<DevAlarmStatus>::iterator iter_alarm = vAlarmState->begin();
		for(;iter_alarm!=vAlarmState->end();++iter_alarm)
		{
			string sDevId = (*iter_alarm).sdevid();
			if(sDevId == sDevNumber)//查找设备
			{
				google::protobuf::RepeatedPtrField<DevAlarmStatus_eCellAlarmMsg> *vcellState = (*iter_alarm).mutable_ccellalarm();
				google::protobuf::RepeatedPtrField<DevAlarmStatus_eCellAlarmMsg>::iterator iter_cell_alarm = vcellState->begin();
				int nIndex=0;
				for(;iter_cell_alarm!=vcellState->end();++iter_cell_alarm,++nIndex)
				{
					//查找监控项报警是否已经存在
					if((*iter_cell_alarm).scellid()==cellAlarmState.scellid())
					{
						//如果该报警项已经恢复
						if(cellAlarmState.ccellstatus()==RESUME)
						{
							//采用线程池处理数据(数据库更新报警项恢复时间)，同时更新数据库，且将当前报警项从缓存报警中清除
							update_alarm_end_time(&(*iter_alarm),cellAlarmState.sstarttime());
							vcellState->SwapElements(nIndex,vcellState->size()-1);
							vcellState->RemoveLast();
							return true;
						}
						else if(cellAlarmState.ccellstatus()!=(*iter_cell_alarm).ccellstatus())	{//该项报警产生变化（下限变成上限等等）
 							//(*iter_cell_alarm).set_sstarttime(cellAlarmState.sstarttime());
							update_alarm_end_time(&(*iter_alarm),cellAlarmState.sstarttime());//保存之前的状态告警
							(*iter_cell_alarm).set_ccellstatus(cellAlarmState.ccellstatus());//修改当前的告警状态
							//采用线程池处理数据(保存报警到数据库)
 							if(boost::detail::thread::singleton<boost::threadpool::pool>::instance()
 								.schedule(boost::bind(&connect_subsvc_handler::save_alarm_data,this,&(*iter_alarm)))){
 								task_count_increase();
 							}
							return true;
						}
					}					
				}
				//没找到该报警项，插入一条报警项记录
				DevAlarmStatus_eCellAlarmMsg *pCellAlarm = vcellState->Add();
				pCellAlarm->CopyFrom(cellAlarmState);
				//采用线程池处理数据(保存报警到数据库)
				if(boost::detail::thread::singleton<boost::threadpool::pool>::instance()
					.schedule(boost::bind(&connect_subsvc_handler::save_alarm_data,this,&(*iter_alarm))))
				{
					task_count_increase();
				}
				return true;
			}
		}
		//没有找到该设备
		DevAlarmStatus *pDevAlarm = vAlarmState->Add();
		pDevAlarm->CopyFrom(devAlarmState);
		//采用线程池处理数据(保存报警到数据库)
		if(boost::detail::thread::singleton<boost::threadpool::pool>::instance()
			.schedule(boost::bind(&connect_subsvc_handler::save_alarm_data,this,pDevAlarm)))
		{
			task_count_increase();
		}
		return true;

	}

	//保存报警数据(线程池异步执行)
	void connect_subsvc_handler::save_alarm_data(DevAlarmStatus *pDevAlarm)
	{
		const google::protobuf::RepeatedPtrField<DevAlarmStatus_eCellAlarmMsg> &vcellState = pDevAlarm->ccellalarm();
		time_t curTime = time(0);
		tm *ltime = localtime(&curTime);
		google::protobuf::RepeatedPtrField<DevAlarmStatus_eCellAlarmMsg>::const_iterator iter_cell_alarm = vcellState.begin();
		for(;iter_cell_alarm!=vcellState.end();++iter_cell_alarm)
		{
			//查找报警项
			int nCellid = atoi((*iter_cell_alarm).scellid().c_str());
			string  sCellAlarmStartTm = (*iter_cell_alarm).sstarttime();
			tm  curTm;
			sscanf(sCellAlarmStartTm.c_str(), "%d/%d/%d %d:%d:%d" , 	&(curTm.tm_year), &(curTm.tm_mon),&(curTm.tm_mday), 
				&(curTm.tm_hour),	&(curTm.tm_min),&(curTm.tm_sec)); 
			curTm.tm_year-=1900;
			curTm.tm_mon-=1;
			//记录数据库
			if(pDevAlarm->edevtype()==TRANSMITTER){
                GetInst(DbManager).AddTransmitterAlarm(&curTm,pDevAlarm->sstationid(),pDevAlarm->sdevid(),
															 nCellid,(*iter_cell_alarm).ccellstatus());//ltime
			}
			else	{
                GetInst(DbManager).AddOthDevAlarm(&curTm,pDevAlarm->sstationid(),pDevAlarm->sdevid(),
														nCellid,(*iter_cell_alarm).ccellstatus());//ltime
			}
			mapStrItemAlarmStartTime[pDevAlarm->sdevid()][nCellid] = std::pair<int,string>((*iter_cell_alarm).ccellstatus(),sCellAlarmStartTm);//*ltime
		}
		task_count_decrease();
	}

	//报警恢复
	void connect_subsvc_handler::update_alarm_end_time(DevAlarmStatus *pDevAlarm,const  string & sAlarmEndTm)
	{
		const google::protobuf::RepeatedPtrField<DevAlarmStatus_eCellAlarmMsg> &vcellState = pDevAlarm->ccellalarm();
		//time_t curTime = time(0);
		//tm *ltime = localtime(&curTime);
		google::protobuf::RepeatedPtrField<DevAlarmStatus_eCellAlarmMsg>::const_iterator iter_cell_alarm = vcellState.begin();
		for(;iter_cell_alarm!=vcellState.end();++iter_cell_alarm)
		{
			//查找报警项
			int nCellid = atoi((*iter_cell_alarm).scellid().c_str());	
			tm startTm;
			sscanf(mapStrItemAlarmStartTime[pDevAlarm->sdevid()][nCellid].second.c_str(), "%d/%d/%d %d:%d:%d" , 	&(startTm.tm_year), &(startTm.tm_mon),&(startTm.tm_mday), 
				&(startTm.tm_hour),	&(startTm.tm_min),&(startTm.tm_sec)); 
			startTm.tm_year-=1900;
			startTm.tm_mon-=1;
			
			tm endTm;
			sscanf(sAlarmEndTm.c_str(), "%d/%d/%d %d:%d:%d" , 	&(endTm.tm_year), &(endTm.tm_mon),&(endTm.tm_mday), 
				&(endTm.tm_hour),	&(endTm.tm_min),&(endTm.tm_sec)); 
			endTm.tm_year-=1900;
			endTm.tm_mon-=1;

			//记录数据库
			if(pDevAlarm->edevtype()==TRANSMITTER)
			{
                GetInst(DbManager).UpdateTransmitterAlarmEndTime(&endTm,startTm,//mapItemAlarmStartTime[pDevAlarm->sdevid()][nCellid].second,
																	   pDevAlarm->sstationid(),pDevAlarm->sdevid(),nCellid);
				mapStrItemAlarmStartTime[pDevAlarm->sdevid()].erase(nCellid);
			}
			else
			{
                GetInst(DbManager).UpdateOthDevAlarm(&endTm,startTm,//mapItemAlarmStartTime[pDevAlarm->sdevid()][nCellid].second,
														pDevAlarm->sstationid(),pDevAlarm->sdevid(),nCellid);
				mapStrItemAlarmStartTime[pDevAlarm->sdevid()].erase(nCellid);
			}

		}
	}

	//开始接收消息头
	void connect_subsvc_handler::start_read_head()
	{
		receive_msg_ptr_->setsession(shared_from_this());
		boost::asio::async_read(socket(), boost::asio::buffer(
			receive_msg_ptr_->data(),HeadSize),
#ifdef USE_STRAND
			strand_.wrap(
#endif
			boost::bind(&session::handle_read_head, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
#ifdef USE_STRAND
			)
#endif
			);
	}

	//消息头接收回调
	void connect_subsvc_handler::handle_read_head(const boost::system::error_code& error,
													size_t bytes_transferred)
	{
		if (!error && receive_msg_ptr_->decode_header())
		{
			//如果已经是完整消息（无参消息)
			if(receive_msg_ptr_->is_full_message())
			{
				// 将数据提交到工作队列.
				taskwork_.SubmitTask(receive_msg_ptr_);
				// 继续读取下一个数据包头.
				receive_msg_ptr_ = msgPointer(new message);
				//receive_msg_ptr_->reset_head_size();//重设消息长度
				//重新请求数据头
				start_read_head();

			}
			else//继续请求消息体
			{
				// 请求数据包.
				boost::asio::async_read(socket(), boost::asio::buffer(
					receive_msg_ptr_->body(), receive_msg_ptr_->body_length()),
#ifdef USE_STRAND
					strand_.wrap
					(
#endif // USE_STRAND
					boost::bind(&session::handle_read_body, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred)
#ifdef USE_STRAND
					)
#endif // USE_STRAND
					);
			}

		}
		else
		{
			std::cout << boost::system::system_error(error).what() << std::endl;
			close_handler();// 出错，清理handler
		}
	}


	void connect_subsvc_handler::handle_read_body(const boost::system::error_code& error, 
		size_t bytes_transferred)
	{
		if (!error && receive_msg_ptr_->check_body(bytes_transferred))
		{
			// 将数据提交到工作队列.
			taskwork_.SubmitTask(receive_msg_ptr_);
			// 继续读取下一个数据包头.
			receive_msg_ptr_ = msgPointer(new message);
			start_read_head();
		}
		else
		{
			std::cout << boost::system::system_error(error).what() << std::endl;
			close_handler();// 出错，清理handler
		}
	}

	
	int connect_subsvc_handler::putq(msgPointer msgPtr)
	{
// 		boost::mutex::scoped_lock lock(msg_q_mutex_);
// 		send_msg_q_.push_back(msgPtr);
// 		if (!send_msg_q_.empty())
// 			start_write();
		return 0;
	}

	void connect_subsvc_handler::start_write(msgPointer msgPtr)
	{
		boost::asio::async_write
			(socket(),
			boost::asio::buffer(&(msgPtr->data()[0]),msgPtr->length()),
			boost::bind(&session::handle_write,shared_from_this(),
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred)
			);
	}

	//查岗确认通知
	void  connect_subsvc_handler::check_station_result_notify(checkWorkingNotifyMsgPtr pcheckWorkResult)
	{
		srv_.check_working_result_notify(shared_from_this(),pcheckWorkResult);
	}

	//发送google消息
	bool connect_subsvc_handler::sendMessage(e_MsgType _type,googleMsgPtr gMsgPtr)//google::protobuf::Message *
	{
		msgPointer msgPtr(new message);
		if(msgPtr->encode_google_message(_type,gMsgPtr))
		{
			start_write(msgPtr);
			return true;
		}
		return false;
	}

	void connect_subsvc_handler::handle_write(const boost::system::error_code& error,size_t bytes_transferred)
	{
		if (!error)
		{
// 			boost::mutex::scoped_lock lock(msg_q_mutex_);
// 			send_msg_q_.pop_front();

			//若在线则继续发送队列消息
			//if (get_state() == usr_online)
// 			{
// 
// 				if (!send_msg_q_.empty())
// 					start_write();
// 				return;
// 			}
		}
		else
		{
			close_handler();// 出错，清理handler
		}
	}
}

