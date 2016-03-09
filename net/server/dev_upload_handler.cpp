#include "dev_upload_handler.h"
#include "../SvcMgr.h"
#include "../LocalConfig.h"
#include "StationConfig.h"
//接受汇鑫设备主动数据上传handler
//与下级服务器建立心跳保持机制
//注意：接受下级设备主动上传数据
namespace net
{
	dev_upload_handler::dev_upload_handler(boost::asio::io_service& io_service, 
		TaskQueue<msgPointer>& taskwork,server &srv)
		:session(io_service)
		,h_b_timer_(io_service)
#ifdef USE_CLIENT_STRAND
		, strand_(io_service)
#endif 
		,taskwork_(taskwork)
		,srv_(srv)
		,usr_state_(usr_offline)
		,dev_con_state_(con_disconnected)//con_connecting
		,dev_run_state_(dev_unknown)
		,task_count_(0)
	{
		pars_agent_ = HMsgHandlePtr(new MsgHandleAgent(this,io_service));
		receive_msg_ptr_ = uploadMsgPtr(new dev_upload_message(1048));
		//cur_upload_dev_info_.nConnectType=-1;//主动上传类型
	}

	dev_upload_handler::~dev_upload_handler(void)
	{

	}

	//关闭连接，并清理自己
	void dev_upload_handler::close_handler()
	{
		h_b_timer_.cancel();
		close_i();
		srv_.remove_uploadDev_connection_handler(shared_from_this());
	}

	//关闭
	void dev_upload_handler::close_i()
	{

		set_con_state(con_disconnected);
		session::close_i();
		wait_task_end();//等待任务结束
		clear_all_alarm();
	}

	//启动心跳定时器
	void dev_upload_handler::start_hb_timer()
	{
		h_b_timer_.expires_from_now(boost::posix_time::seconds(10));
		h_b_timer_.async_wait(strand_.wrap(
			boost::bind(&dev_upload_handler::handle_timeout,this,
			boost::asio::placeholders::error))); 
	}

	//心跳超时
	void dev_upload_handler::handle_timeout(const boost::system::error_code& e)
	{
		if (!e)
		{
			std::time_t now = time(0);
			if (now - get_last_hb_time() >60)//超时
			{
				close_handler();
				return;
			}			
			else
			{
				start_hb_timer();
			}

		}

	}

	//获得最后心跳时间
	std::time_t dev_upload_handler::get_last_hb_time()
	{
		boost::mutex::scoped_lock lock(hb_mutex_);
		return last_hb_time_;
	}

	//设置最后心跳时间
	void dev_upload_handler::set_last_hb_time(std::time_t t)
	{
		boost::mutex::scoped_lock lock(hb_mutex_);
		last_hb_time_ = t;
	}

	//回复心跳请求
	int dev_upload_handler::heart_beat_rsp()
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

	//提交任务
	void dev_upload_handler::task_count_increase()
	{
		boost::mutex::scoped_lock lock(task_mutex_);
		++task_count_;
	}
	//任务递减
	void dev_upload_handler::task_count_decrease()
	{
		boost::mutex::scoped_lock lock(task_mutex_);
		--task_count_;
		task_end_conditon_.notify_all();
	}

	int dev_upload_handler::task_count()
	{
		boost::mutex::scoped_lock lock(task_mutex_);
		return task_count_;
	}

	//等待任务结束
	void dev_upload_handler::wait_task_end()
	{
		boost::mutex::scoped_lock lock(task_mutex_);
		while(task_count_>0)
		{
			task_end_conditon_.wait(task_mutex_);
		}
	}

	//判断当前时间是否需要保存监控数据
	bool dev_upload_handler::is_need_save_data()
	{
		time_t tmCurTime;
		time(&tmCurTime);
		double ninterval = difftime(tmCurTime,tmLastSaveTime);
		if(ninterval<GetInst(LocalConfig).dev_upload_data_save_interval()*60)
			return false;
		tmLastSaveTime = tmCurTime;
		
		return true;
	}

	//开始接收消息头
	void dev_upload_handler::start_read_head()
	{
		boost::asio::async_read(socket(), boost::asio::buffer(
			receive_msg_ptr_->data(),UPLOAD_MSG_HEAD_SIZE),
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
	void dev_upload_handler::handle_read_head(const boost::system::error_code& error,
		size_t bytes_transferred)
	{
		if(srv_.isExitServer())
			return;
		if (!error && receive_msg_ptr_->decode_header())
		{
			// 请求数据包.
			boost::asio::async_read(socket(), boost::asio::buffer(
				receive_msg_ptr_->body(), receive_msg_ptr_->bodySize()),
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
		else
		{
			std::cout << boost::system::system_error(error).what() << std::endl;
			close_handler();// 出错，清理handler
		}
	}


	void dev_upload_handler::handle_read_body(const boost::system::error_code& error, 
		size_t bytes_transferred)
	{
		if(srv_.isExitServer())
			return;
		if (!error )
		{
			std::time_t now = time(0);
			set_last_hb_time(now);

			if(receive_msg_ptr_->cur_commandType()==0x22)//监测数据
			{
				if(get_con_state()!=con_connected)
					return;
				DevMonitorDataPtr curData_ptr(new Data);
				if(receive_msg_ptr_->decode_msg(pars_agent_,curData_ptr,bytes_transferred)>=0)
				{
					if(boost::detail::thread::singleton<boost::threadpool::pool>::instance()
						.schedule(boost::bind(&dev_upload_handler::handler_data,this,curData_ptr)))
					{
						task_count_increase();
					}
				}
				else
				{
					std::cout << boost::system::system_error(error).what() << std::endl;
					close_handler();// 出错，清理handler
					return;
				}

				receive_msg_ptr_->reset();
				start_read_head();
			}
			else if(receive_msg_ptr_->cur_commandType()==0x11)//音频数据
			{
				if(get_con_state()!=con_connected)
					return;
				unchar_ptr curAudioData(new vector<unsigned char>);
				curAudioData->assign(receive_msg_ptr_->body()+1,receive_msg_ptr_->tailed());
				boost::uint8_t uChannel = *(receive_msg_ptr_->body());
				if(boost::detail::thread::singleton<boost::threadpool::pool>::instance()
					.schedule(boost::bind(&dev_upload_handler::handler_mp3_data,this,uChannel,curAudioData)))
				{
					task_count_increase();
				}

				receive_msg_ptr_->reset();
				start_read_head();
			}
			else //其他命令回复
			{
				if(handler_command_data(receive_msg_ptr_->cur_commandType(),receive_msg_ptr_))
				{
					receive_msg_ptr_->reset();
					start_read_head();
				}
				else
					close_handler();
			}

		}
		else
		{
			//std::cout << boost::system::system_error(error).what() << std::endl;
			close_handler();// 出错，清理handler
		}
	}

	void dev_upload_handler::handler_mp3_data(boost::uint8_t uChannel,unchar_ptr curDataPtr)
	{
		map<int,DevParamerMonitorItem>::iterator iter = cur_upload_dev_info_.mapMonitorItem.find(uChannel+59);
		if(iter==cur_upload_dev_info_.mapMonitorItem.end())
		{
			task_count_decrease();
			return;
		}
		send_monitor_data_message(cur_upload_dev_info_.sStationNumber,cur_upload_dev_info_.sDevNum,
			AUDIO,curDataPtr,(*iter).second);
		//录制
		bool bIsMonitorTime = is_record_time((int)uChannel);
		if(bIsMonitorTime)
		{
			pRecordAgentPtr pRdAgent = GetInst(SvcMgr).record_agent_ptr();
			if(pRdAgent!=NULL)
			{
				PrgInfo prgInfo = cur_upload_dev_info_.devForEncoderInfo.map_Info[(int)uChannel];
				if(pRdAgent->isRecording(prgInfo.sPrgNum)==false)
					pRdAgent->startChannelRecord(cur_upload_dev_info_.sDevName,prgInfo.sPrgNum,prgInfo.sPrgName,0);
				pRdAgent->inputBodyDataToFile(prgInfo.sPrgNum,&(curDataPtr->front()),curDataPtr->size());
			}
		}
		else
		{
			pRecordAgentPtr pRdAgent = GetInst(SvcMgr).record_agent_ptr();
			if(pRdAgent!=NULL)
			{
				PrgInfo prgInfo = cur_upload_dev_info_.devForEncoderInfo.map_Info[(int)uChannel];
				pRdAgent->stopChannelRecord(prgInfo.sPrgNum);
			}
		}

		task_count_decrease();
	}

	//监测报警,保存数据
	void dev_upload_handler::handler_data(DevMonitorDataPtr curDataPtr)
	{


		send_monitor_data_message(cur_upload_dev_info_.sStationNumber,cur_upload_dev_info_.sDevNum,
			e_DevType(cur_upload_dev_info_.nDevType),curDataPtr,cur_upload_dev_info_.mapMonitorItem);

		/*
		bool bIsMonitorTime = is_monitor_time(0);
		if(bIsMonitorTime)
		detect_alarm_state(curDataPtr,bIsMonitorTime);
		else
		clear_all_alarm();

		//如果在监测时间段则保存当前记录
		if(bIsMonitorTime)
		save_monitor_record(curDataPtr);
		*/

		bool bIsSave = is_need_save_data();
		for(int i=1;i<=pars_agent_->getChannelCount();++i)
		{
			bool bIsMonitorTime = is_monitor_time(i);
			if(bIsMonitorTime)
			{
				detect_alarm_state(i,curDataPtr,bIsMonitorTime);
				//如果在监测时间段则保存当前记录
				if(bIsSave)
					save_monitor_record(i,curDataPtr);
			}
			else
				clear_channel_alarm(i);					
		}


		task_count_decrease();
	}

	//检测报警状态
	void dev_upload_handler::detect_alarm_state(int nChannelId,DevMonitorDataPtr curDataPtr,bool bMonitor)
	{
		//判断当前时间是否在监测时间段内，若不在，清理该机器历史报警记录
		//......
		boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);
		map<int,DevParamerMonitorItem>::iterator iter = cur_upload_dev_info_.mapMonitorItem.begin();
		for(;iter!=cur_upload_dev_info_.mapMonitorItem.end();++iter)
		{
			if((*iter).second.bAlarmEnable!=true)
				continue;
			//如果当前监控量不属于该通道的，跳过
			if(!pars_agent_->isBelongChannel(nChannelId,(*iter).first))
				continue;
			dev_alarm_state curState = invalid_alarm;
			if(ItemValueIsAlarm(curDataPtr->datainfoBuf[(*iter).first].fValue,(*iter).second,
				curState))
			{
				//记录数据库
				time_t curTime = time(0);
				tm *ltime = localtime(&curTime);
				GetInst(DataBaseManager).AddOthDevAlarm(ltime,cur_upload_dev_info_.sStationNumber,
					cur_upload_dev_info_.sDevNum,(*iter).first,curState);

				mapItemAlarmStartTime[(*iter).first]=std::pair<int,tm>(curState,*ltime);
				if(bMonitor==true)//如果在监测时间段则通知客户端
				{
					//通知客户端
					//广播设备监控量报警到客户端
					std::string alramTm = str(boost::format("%1%:%2%:%3%")
						%ltime->tm_hour
						%ltime->tm_min
						%ltime->tm_sec);
					//string sPrgName = cur_upload_dev_info_.devForEncoderInfo.map_Info[nChannelId].sPrgName;
					send_alarm_state_message(cur_upload_dev_info_.sStationNumber,cur_upload_dev_info_.sDevNum
						,cur_upload_dev_info_.sDevName,(*iter).second.nMonitoringIndex//
						,(*iter).second.sMonitoringName
						,(e_DevType)cur_upload_dev_info_.nDevType,
						curState,alramTm,mapItemAlarmStartTime.size());

					//根据报警等级确定是否进行短信通知用户
					string sAlarmContent = str(boost::format("%1%%2%%3%[时间:%4%]")
						%cur_upload_dev_info_.sDevName
						%(*iter).second.sMonitoringName
						%CONST_STR_ALARM_CONTENT[(int)curState]
					%alramTm
						);
					sendSmsAndCallPhone((*iter).second.nAlarmLevel,sAlarmContent);
				}


			}
			else
			{
				//该item报警恢复
				if(curState==resume_normal)
				{
					time_t curTime = time(0);
					tm *ltime = localtime(&curTime);//恢复时间

					GetInst(DataBaseManager).UpdateOthDevAlarm(ltime,
						mapItemAlarmStartTime[(*iter).first].second,cur_upload_dev_info_.sStationNumber,
						cur_upload_dev_info_.sDevNum,(*iter).first);

					//移除该报警项
					mapItemAlarmStartTime.erase((*iter).first);

					//通知客户端(报警恢复通知客户端)
					std::string alramTm = str(boost::format("%1%:%2%:%3%")
						%ltime->tm_hour
						%ltime->tm_min
						%ltime->tm_sec);
					send_alarm_state_message(cur_upload_dev_info_.sStationNumber,cur_upload_dev_info_.sDevNum
						,cur_upload_dev_info_.sDevName,(*iter).second.nMonitoringIndex
						,(*iter).second.sMonitoringName
						,(e_DevType)cur_upload_dev_info_.nDevType,curState,alramTm,mapItemAlarmStartTime.size());
				}
			}


		}
	}
	//当前项是否在报警
	bool dev_upload_handler::ItemValueIsAlarm(float fValue,DevParamerMonitorItem &ItemInfo,
		dev_alarm_state &alarm_state)
	{
		if(ItemInfo.nMonitoringType == 0)//模拟量
		{
			if(fValue > ItemInfo.dUpperLimit)
			{
				if(mapItemAlarmRecord.find(ItemInfo.nMonitoringIndex)!=mapItemAlarmRecord.end())
				{
					if(mapItemAlarmRecord[ItemInfo.nMonitoringIndex].first == 0)
					{
						if(mapItemAlarmRecord[ItemInfo.nMonitoringIndex].second == 1)
						{
							alarm_state = upper_alarm;
							mapItemAlarmRecord[ItemInfo.nMonitoringIndex].second++;
							return true;
						}
					}
					else
					{
						//第一次数据超上限
						mapItemAlarmRecord[ItemInfo.nMonitoringIndex].first = 0;
						mapItemAlarmRecord[ItemInfo.nMonitoringIndex].second = 1;
					}
				}
				else
				{
					//无报警则添加该报警项
					mapItemAlarmRecord[ItemInfo.nMonitoringIndex].first=0;
					mapItemAlarmRecord[ItemInfo.nMonitoringIndex].second=1;
				}
			}
			else if(fValue < ItemInfo.dLowerLimit)
			{
				if(mapItemAlarmRecord.find(ItemInfo.nMonitoringIndex)!=mapItemAlarmRecord.end())
				{
					if(mapItemAlarmRecord[ItemInfo.nMonitoringIndex].first == 1)
					{
						if(mapItemAlarmRecord[ItemInfo.nMonitoringIndex].second == 1)
						{
							alarm_state = lower_alarm;
							mapItemAlarmRecord[ItemInfo.nMonitoringIndex].second++;
							return true;
						}
					}
					else
					{
						//第一次数据低于下限
						mapItemAlarmRecord[ItemInfo.nMonitoringIndex].first = 1;
						mapItemAlarmRecord[ItemInfo.nMonitoringIndex].second = 1;
					}
				}
				else
				{
					//无报警则添加该报警项
					mapItemAlarmRecord[ItemInfo.nMonitoringIndex].first=1;
					mapItemAlarmRecord[ItemInfo.nMonitoringIndex].second=1;
				}
			}else
			{
				//查找此报警是否已经存在
				if(mapItemAlarmRecord.find(ItemInfo.nMonitoringIndex)!=mapItemAlarmRecord.end())
				{
					mapItemAlarmRecord.erase(ItemInfo.nMonitoringIndex);
					alarm_state = resume_normal;
				}
			}

		}
		else //状态量
		{
			//报警
			if(fValue==ItemInfo.dLowerLimit)
			{
				//判断当前item是否在报警
				if(mapItemAlarmRecord.find(ItemInfo.nMonitoringIndex)
					!=mapItemAlarmRecord.end())
				{
					//是否达到检测次数(2次)
					if(mapItemAlarmRecord[ItemInfo.nMonitoringIndex].second
						==1)
					{
						alarm_state = state_alarm;
						++mapItemAlarmRecord[ItemInfo.nMonitoringIndex].second;
						return true;
					}
				}
				else
				{//若当前是越下限或低功率报警,则修改后越上限报警
					mapItemAlarmRecord[ItemInfo.nMonitoringIndex].first=0;
					mapItemAlarmRecord[ItemInfo.nMonitoringIndex].second=1;
				}
			}
			else
			{
				//查找此报警是否已经存在
				if(mapItemAlarmRecord.find(ItemInfo.nMonitoringIndex)
					!=mapItemAlarmRecord.end())
				{
					mapItemAlarmRecord.erase(ItemInfo.nMonitoringIndex);
					alarm_state = resume_normal;
				}
			}
		}
		return false;
	}

	//是否在录制时间段
	bool dev_upload_handler::is_record_time(int nChannel)
	{
		time_t curTime = time(0);
		tm *pCurTime = localtime(&curTime);
		map<int,PrgInfo>::iterator map_iter = cur_upload_dev_info_.devForEncoderInfo.map_Info.find(nChannel);
		if(map_iter!=cur_upload_dev_info_.devForEncoderInfo.map_Info.end())
		{
			vector<MonitorScheduler>::iterator witer = map_iter->second.vecRecoringScheduler.begin();
			for(;witer!=map_iter->second.vecRecoringScheduler.end();++witer)
			{
				int nDataWeek = ((*witer).nMonitorDay+1)%7;
				if(nDataWeek == pCurTime->tm_wday && (*witer).bEnable == true)
				{
					unsigned long tmStart = (*witer).tmStartTime.tm_hour*3600+
						(*witer).tmStartTime.tm_min*60+
						(*witer).tmStartTime.tm_sec;
					unsigned long tmEnd   = (*witer).tmCloseTime.tm_hour*3600+
						(*witer).tmCloseTime.tm_min*60+
						(*witer).tmCloseTime.tm_sec;
					unsigned long tmCur   =  pCurTime->tm_hour*3600+pCurTime->tm_min*60+pCurTime->tm_sec;

					if(tmCur>=tmStart && tmCur<=tmEnd)
						return true;
				}
			}
		}

		return false;
	}

	bool dev_upload_handler::is_monitor_time(int nChannel)
	{
		// 		time_t curTime = time(0);
		// 		tm *pCurTime = localtime(&curTime);
		// 		vector<MonitorScheduler>::iterator witer = cur_upload_dev_info_.vecMonitorScheduler.begin();
		// 		for(;witer!=cur_upload_dev_info_.vecMonitorScheduler.end();++witer)
		// 		{
		// 			int nDataWeek = ((*witer).nMonitorDay+1)%7;
		// 			if(nDataWeek == pCurTime->tm_wday && (*witer).bEnable == true)
		// 			{
		// 				unsigned long tmStart = (*witer).tmStartTime.tm_hour*3600+
		// 					(*witer).tmStartTime.tm_min*60+
		// 					(*witer).tmStartTime.tm_sec;
		// 				unsigned long tmEnd   = (*witer).tmCloseTime.tm_hour*3600+
		// 					(*witer).tmCloseTime.tm_min*60+
		// 					(*witer).tmCloseTime.tm_sec;
		// 				unsigned long tmCur   =  pCurTime->tm_hour*3600+pCurTime->tm_min*60+pCurTime->tm_sec;
		// 
		// 				if(tmCur>=tmStart && tmCur<=tmEnd)
		// 					return true;
		// 			}
		// 		}
		//		return true;


		time_t curTime = time(0);
		tm *pCurTime = localtime(&curTime);
		map<int,PrgInfo>::iterator map_iter = cur_upload_dev_info_.devForEncoderInfo.map_Info.find(nChannel);
		if(map_iter!=cur_upload_dev_info_.devForEncoderInfo.map_Info.end())
		{
			vector<MonitorScheduler>::iterator witer = map_iter->second.vecMonitorScheduler.begin();
			for(;witer!=map_iter->second.vecMonitorScheduler.end();++witer)
			{
				int nDataWeek = ((*witer).nMonitorDay+1)%7;
				if(nDataWeek == pCurTime->tm_wday && (*witer).bEnable == true)
				{
					unsigned long tmStart = (*witer).tmStartTime.tm_hour*3600+
						(*witer).tmStartTime.tm_min*60+
						(*witer).tmStartTime.tm_sec;
					unsigned long tmEnd   = (*witer).tmCloseTime.tm_hour*3600+
						(*witer).tmCloseTime.tm_min*60+
						(*witer).tmCloseTime.tm_sec;
					unsigned long tmCur   =  pCurTime->tm_hour*3600+pCurTime->tm_min*60+pCurTime->tm_sec;

					if(tmCur>=tmStart && tmCur<=tmEnd)
						return true;
				}
			}
		}
		return false;
	}

	//判断是否保存当前记录
	void dev_upload_handler::save_monitor_record(int nChannelId,DevMonitorDataPtr curDataPtr)
	{
		map<int,DevParamerMonitorItem> channelMonitorItem;
		map<int,DevParamerMonitorItem>::iterator iter = cur_upload_dev_info_.mapMonitorItem.begin();
		for(;iter!=cur_upload_dev_info_.mapMonitorItem.end();++iter)
		{
			if(!pars_agent_->isBelongChannel(nChannelId,(*iter).first))//如果不是对应通道的id
				continue;
			channelMonitorItem[(*iter).first] = iter->second;
		}

		GetInst(DataBaseManager).SaveOthDevMonitoringData(cur_upload_dev_info_.sStationNumber,
			cur_upload_dev_info_.sDevNum,channelMonitorItem,curDataPtr);

	}

	void dev_upload_handler::clear_channel_alarm(int nChannelId)
	{
		boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);
		if(mapItemAlarmStartTime.size()>0)
		{
			map<int,DevParamerMonitorItem>::iterator iter = cur_upload_dev_info_.mapMonitorItem.begin();
			for(;iter!=cur_upload_dev_info_.mapMonitorItem.end();++iter)
			{
				if((*iter).second.bAlarmEnable!=true)
					continue;
				if(mapItemAlarmRecord.find((*iter).first)==mapItemAlarmRecord.end())
					continue;
				if(!pars_agent_->isBelongChannel(nChannelId,(*iter).first))//如果不是对应通道的id
					continue;
				time_t curTime = time(0);
				tm *ltime = localtime(&curTime);//恢复时间
				GetInst(DataBaseManager).UpdateOthDevAlarm(ltime,
					mapItemAlarmStartTime[(*iter).first].second,cur_upload_dev_info_.sStationNumber,
					cur_upload_dev_info_.sDevNum,(*iter).first);

				//移除该报警项
				mapItemAlarmStartTime.erase((*iter).first);

				//通知客户端(报警恢复通知客户端)
				//广播设备监控量报警到客户端
				std::string alramTm = str(boost::format("%1%:%2%:%3%")
					%ltime->tm_hour
					%ltime->tm_min
					%ltime->tm_sec);

				send_alarm_state_message(cur_upload_dev_info_.sStationNumber,cur_upload_dev_info_.sDevNum
					,cur_upload_dev_info_.sDevName,(*iter).second.nMonitoringIndex
					,(*iter).second.sMonitoringName
					,(e_DevType)cur_upload_dev_info_.nDevType
					,resume_normal,alramTm,mapItemAlarmStartTime.size());
			}
		}
	}

	//清除所有报警标志
	void dev_upload_handler::clear_all_alarm()
	{
		boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);
		if(mapItemAlarmStartTime.size()>0)
		{
			map<int,DevParamerMonitorItem>::iterator iter = cur_upload_dev_info_.mapMonitorItem.begin();
			for(;iter!=cur_upload_dev_info_.mapMonitorItem.end();++iter)
			{
				if((*iter).second.bAlarmEnable!=true)
					continue;
				if(mapItemAlarmRecord.find((*iter).first)==mapItemAlarmRecord.end())
					continue;
				time_t curTime = time(0);
				tm *ltime = localtime(&curTime);//恢复时间
				GetInst(DataBaseManager).UpdateOthDevAlarm(ltime,
					mapItemAlarmStartTime[(*iter).first].second,cur_upload_dev_info_.sStationNumber,
					cur_upload_dev_info_.sDevNum,(*iter).first);

				//移除该报警项
				mapItemAlarmStartTime.erase((*iter).first);

				//通知客户端(报警恢复通知客户端)
				//广播设备监控量报警到客户端
				std::string alramTm = str(boost::format("%1%:%2%:%3%")
					%ltime->tm_hour
					%ltime->tm_min
					%ltime->tm_sec);

				send_alarm_state_message(cur_upload_dev_info_.sStationNumber,cur_upload_dev_info_.sDevNum
					,cur_upload_dev_info_.sDevName,(*iter).second.nMonitoringIndex
					,(*iter).second.sMonitoringName
					,(e_DevType)cur_upload_dev_info_.nDevType
					,resume_normal,alramTm,mapItemAlarmStartTime.size());
			}

			mapItemAlarmStartTime.clear();//报警项开始时间清除
		}

		if(mapItemAlarmRecord.size()>0)
		{
			mapItemAlarmRecord.clear();   //报警记录清除
		}
	}

	//解析设备回复命令
	bool dev_upload_handler::handler_command_data(boost::uint8_t cmdType,uploadMsgPtr pMsg)
	{
		if(pMsg->bodySize()<1 || *(pMsg->tailed())!=0x55)
			return false;

		switch(cmdType)
		{
		case 0x33://设置频点
			break;
		case 0x44:
			break;
		case 0x66:
			break;
		case 0xEE://登陆
			{
				if(get_con_state()==con_connected)
					return true;
				DevUploadHeadPtr msg_ = (DevUploadHeadPtr)(pMsg->data());
				msgPointer msgPtr(new message);
				msgPtr->reset_size(UPLOAD_MSG_HEAD_SIZE+2);
				DevUploadHeadPtr ackMsgH= (DevUploadHeadPtr)(&(msgPtr->data()[0]));
				ackMsgH->commandCode = msg_->commandCode;
				ackMsgH->devAddress = msg_->devAddress;
				ackMsgH->msgBodyLen = msg_->msgBodyLen+1;
				ackMsgH->productCode = msg_->productCode;
				ackMsgH->startCode = msg_->startCode;
				msgPtr->data()[UPLOAD_MSG_HEAD_SIZE+1] = 0x55;
				if(GetInst(StationConfig).get_upload_dev_info(msg_->devAddress,cur_upload_dev_info_))
				{
					string sdevAdd=boost::lexical_cast<string>(msg_->devAddress);
					if(!srv_.uploadDev_login(shared_from_this(),sdevAdd,sdevAdd))
						return false;

					map<int,double> itemRadio;
					map<int,DevParamerMonitorItem>::iterator iterItem = cur_upload_dev_info_.mapMonitorItem.begin();
					for(;iterItem!=cur_upload_dev_info_.mapMonitorItem.end();++iterItem)
						itemRadio.insert(pair<int,double>(iterItem->first,iterItem->second.dRatio));

					pars_agent_->Init(cur_upload_dev_info_.nDevProtocol,cur_upload_dev_info_.nSubProtocol,
									  cur_upload_dev_info_.nDevAddr,itemRadio);

					set_con_state(con_connected);
					msgPtr->data()[UPLOAD_MSG_HEAD_SIZE] = 0x05;
					start_write(msgPtr);

					//初始化最后保存时间
					std::time(&tmLastSaveTime);
					return true;
				}
				else
				{
					msgPtr->data()[UPLOAD_MSG_HEAD_SIZE] = 0x0A;
					start_write(msgPtr);
					return false;
				}
			}
			break;
		}
		return true;
	}

	void dev_upload_handler::set_con_state(con_state curState)
	{
		boost::recursive_mutex::scoped_lock llock(con_state_mutex_);
		if(dev_con_state_ != curState)
		{
			dev_con_state_=curState;
			GetInst(SvcMgr).get_notify()->OnConnected(cur_upload_dev_info_.sDevNum,dev_con_state_);
			//广播设备状态到在线客户端
			send_net_state_message(cur_upload_dev_info_.sStationNumber,cur_upload_dev_info_.sDevNum
				,cur_upload_dev_info_.sDevName,ENVIR,dev_con_state_);
		}
	}

	void dev_upload_handler::set_run_state(dev_run_state curState)
	{
		boost::recursive_mutex::scoped_lock llock(run_state_mutex_);
		if(dev_run_state_ != curState)
		{
			dev_run_state_=curState;
			GetInst(SvcMgr).get_notify()->OnRunState(cur_upload_dev_info_.sDevNum,dev_run_state_);
			//广播设备状态到在线客户端
			send_work_state_message(cur_upload_dev_info_.sStationNumber,cur_upload_dev_info_.sDevNum
				,cur_upload_dev_info_.sDevName,ENVIR,dev_run_state_);
		}
	}

	con_state  dev_upload_handler::get_con_state()
	{
		boost::recursive_mutex::scoped_lock llock(con_state_mutex_);
		return dev_con_state_;
	}

	dev_run_state dev_upload_handler::get_run_state()
	{
		boost::recursive_mutex::scoped_lock llock(run_state_mutex_);
		return dev_run_state_;
	}

	void dev_upload_handler::get_alarm_state(map<int,std::pair<int,tm>>& cellAlarm)
	{
		boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);
		cellAlarm = mapItemAlarmStartTime;
	}

	void dev_upload_handler::dev_base_info(DevBaseInfo& devInfo,string iId)
	{
		devInfo.mapMonitorItem = cur_upload_dev_info_.mapMonitorItem;
		devInfo.nDevType = DEV_MEDIA;
		devInfo.sDevName = cur_upload_dev_info_.sDevName;//节目名称
		devInfo.sDevNum = cur_upload_dev_info_.sDevNum;
		devInfo.sStationNumber = cur_upload_dev_info_.sStationNumber;
		devInfo.nCommType = cur_upload_dev_info_.nCommType;//主动还是被动（0:被动，1:主动）
		devInfo.nConnectType = cur_upload_dev_info_.nConnectType;//连接方式0：tcp
	}

	void dev_upload_handler::start_write(msgPointer msgPtr)
	{
		boost::asio::async_write
			(socket(),
			boost::asio::buffer(&(msgPtr->data()[0]),msgPtr->length()),
			boost::bind(&session::handle_write,shared_from_this(),
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred)
			);
	}

	//切换音频通道
	bool dev_upload_handler::switch_audio_channel(string sUser,int sNewChannel,e_ErrorCode &opResult)
	{
		if(get_con_state()!=con_connected)
			return false;

		msgPointer msgPtr(new message);
		msgPtr->reset_size(UPLOAD_MSG_HEAD_SIZE+2);
		DevUploadHeadPtr ackMsgH= (DevUploadHeadPtr)(&(msgPtr->data()[0]));
		ackMsgH->commandCode = 0x66;
		ackMsgH->devAddress = cur_upload_dev_info_.nDevAddr;
		ackMsgH->msgBodyLen = 0x02;
		ackMsgH->productCode = 0x20;
		ackMsgH->startCode = 0x7E;
		if(sNewChannel==0)
			msgPtr->data()[UPLOAD_MSG_HEAD_SIZE] = 0x00;
		else 
			msgPtr->data()[UPLOAD_MSG_HEAD_SIZE] = 0x01<<(sNewChannel-1);
		msgPtr->data()[UPLOAD_MSG_HEAD_SIZE+1] = 0x55;

		start_write(msgPtr);
		return true;

	}

	//执行通用指令
	bool dev_upload_handler::excute_command(int cmdType,devCommdMsgPtr lpParam,e_ErrorCode &opResult)
	{
		if(get_con_state()!=con_connected)
			return false;

		switch(cmdType)
		{
		case MSG_0401_SWITCH_OPR:
		case MSG_SWITCH_AUDIO_CHANNEL_OPR:
			{
				if(lpParam->cparams_size()<1)
					return false;
				msgPointer msgPtr(new message);
				msgPtr->reset_size(UPLOAD_MSG_HEAD_SIZE+2);
				DevUploadHeadPtr ackMsgH= (DevUploadHeadPtr)(&(msgPtr->data()[0]));
				ackMsgH->commandCode = 0x66;
				ackMsgH->devAddress = cur_upload_dev_info_.nDevAddr;
				ackMsgH->msgBodyLen = 0x02;
				ackMsgH->productCode = 0x20;
				ackMsgH->startCode = 0x7E;

				int sNewChannel = atoi(lpParam->cparams(0).sparamvalue().c_str());
				if(sNewChannel==0)
					msgPtr->data()[UPLOAD_MSG_HEAD_SIZE] = 0x00;
				else 
					msgPtr->data()[UPLOAD_MSG_HEAD_SIZE] = 0x01<<(sNewChannel-1);
				msgPtr->data()[UPLOAD_MSG_HEAD_SIZE+1] = 0x55;

				start_write(msgPtr);

			}
			break;
		case MSG_SET_FREQUENCY_OPR://频率设置
			{
				if(lpParam->cparams_size()<3)
					return false;
				msgPointer msgPtr(new message);
				msgPtr->reset_size(UPLOAD_MSG_HEAD_SIZE+5);
				DevUploadHeadPtr ackMsgH= (DevUploadHeadPtr)(&(msgPtr->data()[0]));
				ackMsgH->commandCode = 0x33;
				ackMsgH->devAddress = cur_upload_dev_info_.nDevAddr;
				ackMsgH->msgBodyLen = 0x05;
				ackMsgH->productCode = 0x20;
				ackMsgH->startCode = 0x7E;
				int nChannel = atoi(lpParam->cparams(0).sparamvalue().c_str());
				msgPtr->data()[UPLOAD_MSG_HEAD_SIZE]=nChannel;
				int nMod = atoi(lpParam->cparams(1).sparamvalue().c_str());
				msgPtr->data()[UPLOAD_MSG_HEAD_SIZE+1]=nMod;

				double nFreq= atof(lpParam->cparams(2).sparamvalue().c_str());
				if(nMod==1)//调频
				{
					msgPtr->data()[UPLOAD_MSG_HEAD_SIZE+2]=((int)(nFreq*100))%256;
					msgPtr->data()[UPLOAD_MSG_HEAD_SIZE+3]=((int)(nFreq*100))/256;
				}
				else if(nMod==0)//调幅
				{
					msgPtr->data()[UPLOAD_MSG_HEAD_SIZE+2]=((int)nFreq)%256;
					msgPtr->data()[UPLOAD_MSG_HEAD_SIZE+3]=nFreq/256;
				}
				msgPtr->data()[UPLOAD_MSG_HEAD_SIZE+4]=0x55;

				start_write(msgPtr);

			}
			break;
		case MSG_SEARCH_FREQUENCY_OPR://搜台
			{
				if(lpParam->cparams_size()<2)
					return false;
				msgPointer msgPtr(new message);
				msgPtr->reset_size(UPLOAD_MSG_HEAD_SIZE+4);
				DevUploadHeadPtr ackMsgH= (DevUploadHeadPtr)(&(msgPtr->data()[0]));
				ackMsgH->commandCode = 0x44;
				ackMsgH->devAddress = cur_upload_dev_info_.nDevAddr;
				ackMsgH->msgBodyLen = 0x04;
				ackMsgH->productCode = 0x20;
				ackMsgH->startCode = 0x7E;
				int nChannel = atoi(lpParam->cparams(0).sparamvalue().c_str());
				msgPtr->data()[UPLOAD_MSG_HEAD_SIZE]=nChannel;
				int nMod = atoi(lpParam->cparams(1).sparamvalue().c_str());
				msgPtr->data()[UPLOAD_MSG_HEAD_SIZE+1]=nMod;
				int nDirect = atoi(lpParam->cparams(2).sparamvalue().c_str());
				msgPtr->data()[UPLOAD_MSG_HEAD_SIZE+2] = nDirect;
				msgPtr->data()[UPLOAD_MSG_HEAD_SIZE+3]=0x55;

				start_write(msgPtr);
			}
			break;
		}


		return true;
	}


	//发送google消息
	bool dev_upload_handler::sendMessage(e_MsgType _type,googleMsgPtr gMsgPtr)
	{
		msgPointer msgPtr(new message);
		if(msgPtr->encode_google_message(_type,gMsgPtr))
		{
			start_write(msgPtr);
			return true;
		}
		return false;
	}

	void dev_upload_handler::handle_write(const boost::system::error_code& error,size_t bytes_transferred)
	{
		if (!error)
		{
		}
		else
		{
			close_handler();// 出错，清理handler
		}
	}
}

