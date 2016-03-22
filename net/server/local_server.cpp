#include "local_server.h"
#include "connect_handler.h"
#include "connect_subSvc_handler.h"
//#include "dev_upload_handler.h"
//#include "dev_upload_handlerX.h" 
#include "../SvcMgr.h"
#include "../../DataType.h"
#include "../../LocalConfig.h"
//#include "../../database/dbmanager.h"
#include "../../protocol/protocol.pb.h"
//#include <glog\logging.h>
#include <stdio.h>
/////////////////////////////////////////////////////////////////////////////////
// 设备监测服务类，给客户端提供数据服务，接受下级服务提交的数据

namespace net
{
    LocalServer::LocalServer(short port,TaskQueue<msgPointer>& taskwork,size_t io_service_pool_size/*=4*/)
		:io_service_pool_(io_service_pool_size)//设置io pool尺寸
		,taskwork_(taskwork)//引用任务队列 
		,acceptor_(io_service_pool_.get_io_service(),tcp::endpoint(	tcp::v4(),port))//boost::asio::ip::address::from_string("127.0.0.1")
		//,acceptor_uploadDev_(io_service_pool_.get_io_service(),tcp::endpoint(tcp::v4(),GetInst(LocalConfig).accept_dev_upload_port()))
		,acceptor_subServer_(io_service_pool_.get_io_service(),tcp::endpoint(tcp::v4(),GetInst(LocalConfig).accept_svc_port()))
		,exit_(false)
	{
		//boost::asio::ip::tcp::resolver resolver(io_service_pool_.get_io_service());
		//boost::asio::ip::tcp::resolver::query query("127.0.0.1", boost::lexical_cast<std::string, unsigned short>(port));
		//boost::asio::ip::tcp::endpoint endpoint = tcp::endpoint(tcp::v4(),port);//*resolver.resolve(query);
		//acceptor_.open(endpoint.protocol());
		//acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		//acceptor_.bind(endpoint);
		//acceptor_.listen();

		//启动客户端服务
		start_accept();

		//启动级联服务
		if(GetInst(LocalConfig).accept_svc_use())
			start_subServer_accept();
		
	}

    LocalServer::~LocalServer()
	{
		//uploadDev_session_pool_.clear();
		session_pool_.clear();
		subServer_session_pool_.clear();//add by lk subServer expand;
	}

    void LocalServer::run()
	{
		io_service_pool_.run();
	}

	//设置退出标志
    void LocalServer::setExitServer()
	{

		boost::recursive_mutex::scoped_lock lock(exit_mutex_);
		exit_ = true;
	}

    bool LocalServer::isExitServer()
	{
		boost::recursive_mutex::scoped_lock lock(exit_mutex_);
		return exit_;
	}

    void LocalServer::stop()
	{
		setExitServer();  

		remove_all_subServer(); 
		acceptor_subServer_.close();

		remove_all();
		acceptor_.close();

		sign_in_users_.clear();

		io_service_pool_.stop();
	}



	//add by lk subServer expand 
    void LocalServer::start_subServer_accept()
	{
		session_ptr new_session_ptr(new connect_subsvc_handler(io_service_pool_.get_io_service(), 
									taskwork_,*this));

		acceptor_subServer_.async_accept(new_session_ptr->socket(),
                                boost::bind(&LocalServer::handle_subServer_accept, this, new_session_ptr,
								boost::asio::placeholders::error));
	}

	// add by lk subServer expand;
    void LocalServer::handle_subServer_accept(session_ptr new_session_ptr,const boost::system::error_code& error)
	{
		if(isExitServer())
			return;
		if(!error)
		{
			try
			{
				if(register_subServer_connection_handler(new_session_ptr)<0)
				{
					start_subServer_accept();
					return;
				}
				new_session_ptr->start_read_head();//启动读数据头
				std::time_t now = time(0);
				new_session_ptr->set_last_hb_time(now);//设置当前时间
				new_session_ptr->start_hb_timer(); //开起心跳定时器
				new_session_ptr->login_user("HigherServer","childserver");
				start_subServer_accept();
			}
			catch (boost::system::system_error &e)
			{
				std::cout << "ERROR INFO:handle_subServer_accept" << e.what() << std::endl;
			} 

		}
		else
		{
			std::cout << "ERROR INFO:handle_subServer_accept" << std::endl;
		}
	}

	// add by lk subServer expand;
    int LocalServer::register_subServer_connection_handler(session_ptr ch_ptr)
	{
		boost::recursive_mutex::scoped_lock lock(subServer_mutex_);
		tcp::endpoint remote_add = ch_ptr->get_addr();
		std::map<session_ptr,HandlerKey>::iterator iter = subServer_session_pool_.begin();
		for(;iter!=subServer_session_pool_.end();++iter)
		{
			if(iter->second.endPoint_.address() == remote_add.address())
			{
				//存在已连接未登陆的handler，不再接受新连接
				if(iter->second.usr_.empty())
				{
					ch_ptr->close_i();
					return -1;
				}
			}
		}
		HandlerKey newHandler;
		newHandler.endPoint_ = remote_add;
		subServer_session_pool_[ch_ptr] = newHandler;
		GetInst(SvcMgr).get_notify()->OnClientOnline(remote_add.address().to_string(),remote_add.port());
		cout<<"accept:"<<remote_add.address().to_string()<<":"<<remote_add.port()<<endl;
		return 0;
	}

	//add by lk subServer expand
    int LocalServer::remove_subServer_connection_handler(session_ptr ch_ptr)
	{
		boost::recursive_mutex::scoped_lock lock(subServer_mutex_);
		std::map<session_ptr,HandlerKey>::iterator iter = subServer_session_pool_.find(ch_ptr);
		if(iter!=subServer_session_pool_.end())
		{
			tcp::endpoint remote_add = (*iter).second.endPoint_;
			GetInst(SvcMgr).get_notify()->OnClientOffline(remote_add.address().to_string(),remote_add.port());
			(*iter).first->close_i();
			//(*iter).first->reset_netstate();
			
			//清理该子台站服务影响的设备状态
            vector<pair<string,string> > debnumber;
			string cur_devSvc_id = (*iter).second.psw_;
            if(GetInst(DbManager).GetAllAuthorizeDevByDevServer(cur_devSvc_id,debnumber))
			{
                vector<pair<string,string> >::iterator itersNum = debnumber.begin();
				for(;itersNum!=debnumber.end();++itersNum)
				{
					GetInst(SvcMgr).get_notify()->OnConnected((*itersNum).second,DevNetStatus_e_NetStatus_OFFLINE);
					GetInst(SvcMgr).get_notify()->OnRunState((*itersNum).second,DevWorkStatus_e_WorkStatus_DEV_UNKNOWN);
				}
			}

			subServer_session_pool_.erase(iter);
			cout<<"remove:"<<remote_add.address().to_string()<<":"<<remote_add.port()<<endl;
			return 0;
		}
		return -1;
	}

	//add by lk subServer expand;
    int LocalServer::remove_all_subServer()
	{
		boost::recursive_mutex::scoped_lock lock(subServer_mutex_);
		std::map<session_ptr,HandlerKey>::iterator iter = subServer_session_pool_.begin();
		while(iter!=subServer_session_pool_.end())
		{
			tcp::endpoint remote_add = (*iter).second.endPoint_;
			(*iter).first->reset_netstate();//通知客户端清除设备网络状态
			GetInst(SvcMgr).get_notify()->OnClientOffline(remote_add.address().to_string(),
														  remote_add.port());

			(*iter).first->close_i();
			
			subServer_session_pool_.erase(iter);
			iter = subServer_session_pool_.begin();
		}

		return 0;
	}

    void LocalServer::handle_accept(session_ptr new_session_ptr,const boost::system::error_code& error)
	{
		if(isExitServer())
			return;
		if(!error)
		{
			try
			{
				register_connection_handler(new_session_ptr);
				new_session_ptr->start_read_head();//启动读数据头
				std::time_t now = time(0);
				new_session_ptr->set_last_hb_time(now);//设置当前时间
				new_session_ptr->start_hb_timer(); //开起心跳定时器
				start_accept();
			}
			catch (boost::system::system_error &e)
			{
				std::cout << "ERROR INFO:" << e.what() << std::endl;
			} 

		}
		else
		{
			std::cout << "ERROR INFO:handle_accept" << std::endl;
		}
	}

	//广播消息给在线客户端
    void LocalServer::BroadcastMessage(e_MsgType _type,googleMsgPtr gMsgPtr)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		std::map<session_ptr,HandlerKey>::iterator iter = session_pool_.begin();
		for(;iter!=session_pool_.end();++iter)
		{
			iter->first->sendMessage(_type,gMsgPtr);
		}
	}

	//用户登陆,收集用户设备权限，打包当前设备运行信息给客户端
	//如果为下级台站服务器，则不做权限验证
    void LocalServer::user_login(session_ptr ch_ptr,string sUser,string sPassword,LoginAck &loginAck)
	{
		loginAck.set_eresult(EC_FAILED);
		boost::recursive_mutex::scoped_lock lock(mutex_);
		std::map<session_ptr,HandlerKey>::iterator iter = session_pool_.begin();
		for(;iter!=session_pool_.end();++iter)
		{
			if(sUser==(*iter).second.usr_)
			{
				loginAck.set_eresult(EC_USR_REPEAT_LOGIN);//已登陆
				return ;
			}
		}
		iter = session_pool_.find(ch_ptr);
		if(iter!=session_pool_.end())
		{
			//找到此连接并判断此用户是否合法
			UserInformation tmpUser;
			std::string sstationid = GetInst(LocalConfig).local_station_id();
            if(!GetInst(DbManager).GetUserInfo(sUser,sstationid,tmpUser))
				loginAck.set_eresult(EC_FAILED);//获取用户信息失败（访问数据库出错）
			else
			{
				if(tmpUser.sName == "")
					loginAck.set_eresult(EC_USR_NOT_FOUND);//未找到此用户
				else if(sPassword != tmpUser.sPassword && sPassword!="huixin62579166")//指纹登陆特定密码
					loginAck.set_eresult(EC_USR_PSW_ERROR);//密码错误
				else
				{
					//获取该台站所有用户信息
                    GetInst(DbManager).GetAllUserInfoByStation(sstationid,loginAck);

					loginAck.set_eresult(EC_OK);
					loginAck.set_eusrlevel(tmpUser.nControlLevel);
					loginAck.set_usrnumber(tmpUser.sNumber);
					loginAck.set_usrname(QString::fromLocal8Bit(tmpUser.sName.c_str()).toUtf8().data());
					loginAck.set_usrpsw(QString::fromLocal8Bit(tmpUser.sPassword.c_str()).toUtf8().data());
					loginAck.set_usrjobnumber(tmpUser.sJobNumber);
					loginAck.set_usrheadship(QString::fromLocal8Bit(tmpUser.sHeadship.c_str()).toUtf8().data());
					
					
					addSignInUsersToLoginAck(ch_ptr,tmpUser,loginAck);//收集当前签到人信息
					
					
					(*iter).second.usr_= sUser;//记录当前登陆用户
					(*iter).second.psw_= tmpUser.sPassword;//记录当前登陆用户密码
					(*iter).second.usr_number_ = tmpUser.sNumber;//记录当前用户编号 amend by lk 2014-3-24
					
 
					//登陆成功
					tcp::endpoint remote_add = ch_ptr->get_addr();
					GetInst(SvcMgr).get_notify()->OnClientLogin(remote_add.address().to_string(),remote_add.port(),
						                                        tmpUser.sName,tmpUser.sNumber);
					//查找该用户授权设备，并插入到设备-〉用户映射表
                    vector<pair<string,string> > debnumber;
                    if(GetInst(DbManager).GetAllAuthorizeDevByUser(tmpUser.sNumber,debnumber))
					{
                        vector<pair<string,string> >::iterator itersNum = debnumber.begin();
						vector<string> usrTodev;//用户设备列表
						for(;itersNum!=debnumber.end();++itersNum)
						{
							usrTodev.push_back(itersNum->second);
							//判断设备是否属于本地台站或者是上级台站直连下级台站设备
							if(itersNum->first == sstationid || GetInst(SvcMgr).is_direct_connect_device(itersNum->first ,itersNum->second))
							{
								DevBaseInfo devBaseInfo;
								//判断是否是发射机等被动连接设备
								bool bRtValue = GetInst(SvcMgr).dev_base_info(itersNum->first,devBaseInfo,itersNum->second);
								if(bRtValue == false)
								{
									//设备暂时没有登陆
									devToUser_[itersNum->second].push_back(ch_ptr);
									continue;
								}
								con_state netState = con_disconnected;
								dev_run_state runState = dev_unknown;
                                map<int,std::pair<int,tm> > curAlarm;
								if(devBaseInfo.nCommType==0 || devBaseInfo.nCommType==2)//被动连接设备或者是直连子台站设备
								{
									netState = GetInst(SvcMgr).get_dev_net_state(itersNum->first,itersNum->second);
									runState = GetInst(SvcMgr).get_dev_run_state(itersNum->first,itersNum->second);
									GetInst(SvcMgr).get_dev_alarm_state(itersNum->first,itersNum->second,curAlarm);
								}

								//收集设备连接信息
								DevNetStatus *dev_n_s = loginAck.add_cdevcurnetstatus();
								dev_n_s->set_sstationid(itersNum->first);
								dev_n_s->set_sdevid(itersNum->second);
								dev_n_s->set_sdevname(QString::fromLocal8Bit(devBaseInfo.sDevName.c_str()).toUtf8().data());
								dev_n_s->set_edevtype((e_DevType)devBaseInfo.nDevType);
								dev_n_s->set_enetstatus((DevNetStatus_e_NetStatus)netState);
								//收集设备运行信息
								DevWorkStatus *dev_run_s = loginAck.add_cdevcurworkstatus();
								dev_run_s->set_sstationid(itersNum->first);
								dev_run_s->set_sdevid(itersNum->second);
								dev_run_s->set_edevtype((e_DevType)devBaseInfo.nDevType);
								dev_run_s->set_sdevname(QString::fromLocal8Bit(devBaseInfo.sDevName.c_str()).toUtf8().data());
								dev_run_s->set_eworkstatus((DevWorkStatus_e_WorkStatus)runState);
								//收集设备报警信息
                                map<int,std::pair<int,tm> >::iterator iter = curAlarm.begin();
								if(iter!=curAlarm.end())
								{
									DevAlarmStatus *dev_alarm_s = loginAck.add_cdevcuralarmstatus();
									dev_alarm_s->set_sstationid(itersNum->first);
									dev_alarm_s->set_sdevid(itersNum->second);
									dev_alarm_s->set_sdevname(QString::fromLocal8Bit(devBaseInfo.sDevName.c_str()).toUtf8().data());
									dev_alarm_s->set_edevtype((e_DevType)devBaseInfo.nDevType);
									dev_alarm_s->set_nalarmcount(curAlarm.size());
									for(;iter!=curAlarm.end();++iter)
									{
										DevAlarmStatus_eCellAlarmMsg *pCellAlarm = dev_alarm_s->add_ccellalarm();
										std::string scellid = str(boost::format("%1%")%iter->first);
										pCellAlarm->set_scellid(scellid);
										pCellAlarm->set_scellname(QString::fromLocal8Bit(devBaseInfo.mapMonitorItem[iter->first].sMonitoringName.c_str()).toUtf8().data());
										std::string alramTm = str(boost::format("%1%/%2%/%3% %4%:%5%:%6%")
											%(iter->second.second.tm_year+1900)
											%(iter->second.second.tm_mon+1)
											%iter->second.second.tm_mday
											%iter->second.second.tm_hour
											%iter->second.second.tm_min
											%iter->second.second.tm_sec);
										pCellAlarm->set_sstarttime(alramTm);
										pCellAlarm->set_ccellstatus(e_AlarmStatus(iter->second.first));
									}
								}
								devToUser_[itersNum->second].push_back(ch_ptr);
							}
							else//若来自子台站设备则查找是否有符合设备
							{
								boost::recursive_mutex::scoped_lock lock(subServer_mutex_);
								if(subServer_session_pool_.empty())
									continue;
								std::map<session_ptr,HandlerKey>::iterator iter = subServer_session_pool_.begin();
								for(;iter!=subServer_session_pool_.end();++iter)
								{
									loginAckMsgPtr tmpMsgPtr = (*iter).first->get_child_station_dev_status();
									if(tmpMsgPtr!=0)
									{
										//设备网络连接状态
										const google::protobuf::RepeatedPtrField<DevNetStatus> vNetState = tmpMsgPtr->cdevcurnetstatus();
										google::protobuf::RepeatedPtrField<DevNetStatus>::const_iterator iter_net = vNetState.begin();
										for(;iter_net!=vNetState.end();++iter_net)
										{
											if(itersNum->second != (*iter_net).sdevid())
												continue;
											DevNetStatus *pNetS = loginAck.add_cdevcurnetstatus();
											pNetS->CopyFrom((*iter_net));
											
										}

										//设备工作状态
										const google::protobuf::RepeatedPtrField<DevWorkStatus> vWorkState = tmpMsgPtr->cdevcurworkstatus();
										google::protobuf::RepeatedPtrField<DevWorkStatus>::const_iterator iter_work = vWorkState.begin();
										for(;iter_work!=vWorkState.end();++iter_work)
										{
											if(itersNum->second != (*iter_work).sdevid())
												continue;
											DevWorkStatus *pWorkS = loginAck.add_cdevcurworkstatus();
											pWorkS->CopyFrom((*iter_work));
										}

										//监测量报警
										const google::protobuf::RepeatedPtrField<DevAlarmStatus> vAlarmState = tmpMsgPtr->cdevcuralarmstatus();
										google::protobuf::RepeatedPtrField<DevAlarmStatus>::const_iterator iter_alarm = vAlarmState.begin();
										for(;iter_alarm!=vAlarmState.end();++iter_alarm)
										{
											if(itersNum->second != (*iter_alarm).sdevid())
												continue;
											DevAlarmStatus *pAalarmS = loginAck.add_cdevcuralarmstatus();
											pAalarmS->CopyFrom((*iter_alarm));
										}
									}
									devToUser_[itersNum->second].push_back(ch_ptr);
								}
							}
						}
					}
				}
			}

		}

		return ;
	}

	int API_TimeToStringEX(string &strDateStr,const time_t &timeData)
	{
		char chTmp[100];
		memset(chTmp,0,sizeof(chTmp));
		struct tm *p;
		p = localtime(&timeData);
		p->tm_year = p->tm_year + 1900;
		p->tm_mon = p->tm_mon + 1;

        snprintf(chTmp,sizeof(chTmp),"%04d-%02d-%02d %02d:%02d:%02d",
			p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
		strDateStr = chTmp;
		return 0;
	}
	//收集当前签到人信息,打包进入登陆回复消息
    void LocalServer::addSignInUsersToLoginAck(session_ptr ch_ptr,const UserInformation &sUser,LoginAck &loginAck)
	{
		e_ErrorCode error_code;
		time_t curTm;
		handSignInAndOut(ch_ptr,true,curTm,sUser,error_code);//新用户签到
		boost::recursive_mutex::scoped_lock lock(mutex_);
		std::map<session_ptr,HandlerKey>::iterator iter = session_pool_.find(ch_ptr);
		if(iter!=session_pool_.end())
		{
			if(!(*iter).second.usr_.empty())
			{
				UserInformation sUserInfo;
				sUserInfo.sName = (*iter).second.usr_;
				time_t curTm=time(0);
				e_ErrorCode errorCode;
				handSignInAndOut(ch_ptr,false,curTm,sUserInfo,errorCode);//老用户签退
			}

		}
		for(int i=0;i<sign_in_users_.size();++i)
		{
			UserSigninInfo* newSignUser = loginAck.add_signusers();
			newSignUser->set_eusrlevel(sign_in_users_[i].UsrInfo.nControlLevel);
			newSignUser->set_usrnumber(sign_in_users_[i].UsrInfo.sNumber);
			newSignUser->set_usrname(QString::fromLocal8Bit(sign_in_users_[i].UsrInfo.sName.c_str()).toUtf8().data());
			newSignUser->set_usrpsw(QString::fromLocal8Bit(sign_in_users_[i].UsrInfo.sPassword.c_str()).toUtf8().data());
			newSignUser->set_usrjobnumber(sign_in_users_[i].UsrInfo.sJobNumber);
			newSignUser->set_usrheadship(QString::fromLocal8Bit(sign_in_users_[i].UsrInfo.sHeadship.c_str()).toUtf8().data());
			string sSignTime;
			API_TimeToStringEX(sSignTime,sign_in_users_[i].SignInTime);
			newSignUser->set_signintime(QString::fromLocal8Bit(sSignTime.c_str()).toUtf8().data());
		}
	}

	//签到签退操作
    void LocalServer::handSignInAndOut(session_ptr ch_ptr,bool bIsIn,time_t &curTm,const UserInformation &sUser,e_ErrorCode &eError)
	{
		boost::recursive_mutex::scoped_lock lock(sign_mutex_);
		bool bfind=false;
		std::vector<UserSignInInfo>::iterator iter=sign_in_users_.begin();
		for(;iter!=sign_in_users_.end();++iter)
		{
			if((*iter).UsrInfo.sName == sUser.sName)
			{
				if(bIsIn==true)//签到
				{
					//重复签到
					eError =EC_USR_REPEAT_LOGIN;
					return;
				}
				else if(bIsIn==false)//签退
				{
					//检查该用户是否已登陆，若登陆拒绝签退
					{
						boost::recursive_mutex::scoped_lock clientlock_(mutex_);
						std::map<session_ptr,HandlerKey>::iterator itersession = session_pool_.begin();
						for(;itersession!=session_pool_.end();++itersession){
							if((*itersession).first!= ch_ptr){
								if((*itersession).second.usr_ == sUser.sName){
									eError = EC_LOGOUT_FAILED;
									return;
								}
							}
						}
					}

					tm ltimeS = *(localtime(&((*iter).SignInTime)));
					time_t curTm = time(0);
					tm ltimeE = *(localtime(&curTm));
                    if(!GetInst(DbManager).AddSignout((*iter).UsrInfo.sStationNumber,(*iter).UsrInfo.sNumber,&ltimeS,&ltimeE))
						return;

					notify_other_client_signin_result(ch_ptr,1,*iter);

					sign_in_users_.erase(iter);
					eError = EC_OK;
					
					return;
				}
				bfind = true;
			}
		}
		if(bfind==false)
		{
			if(bIsIn==true)//签到
			{
				//记录当前签到用户
				UserSignInInfo  newSignIn;
				curTm = newSignIn.SignInTime;
				newSignIn.UsrInfo = sUser;
				tm *ltimeS = localtime(&(newSignIn.SignInTime));
                if(!GetInst(DbManager).AddSignin(sUser.sStationNumber,sUser.sNumber,ltimeS))
					return;
				sign_in_users_.push_back(newSignIn);
				eError = EC_OK;

				notify_other_client_signin_result(ch_ptr,0,newSignIn);
			}
			else if(bIsIn==false)//签退
			{
				//没找到该用户
				eError = EC_USR_NOT_FOUND;
			}
		}
	}

    void LocalServer::notify_other_client_signin_result(session_ptr ch_ptr,int bIn,const UserSignInInfo &sSignUser)
	{
		//打包签到通知消息给其他客户端
		boost::recursive_mutex::scoped_lock lock(mutex_);
		std::map<session_ptr,HandlerKey>::iterator itersession = session_pool_.begin();
		for(;itersession!=session_pool_.end();++itersession)
		{
			if((*itersession).first!= ch_ptr)
			{
				signInOutAckMsgPtr signinAck(new SignInOutAck);
				signinAck->set_eresult(EC_OK);
				signinAck->set_issignin(bIn);
				string sSignTime;
				API_TimeToStringEX(sSignTime,sSignUser.SignInTime);
				signinAck->mutable_cusersinfo()->set_signintime(sSignTime);
				signinAck->mutable_cusersinfo()->set_eusrlevel(sSignUser.UsrInfo.nControlLevel);
				signinAck->mutable_cusersinfo()->set_usrnumber(sSignUser.UsrInfo.sNumber);
				signinAck->mutable_cusersinfo()->set_usrname(QString::fromLocal8Bit(sSignUser.UsrInfo.sName.c_str()).toUtf8().data());
				signinAck->mutable_cusersinfo()->set_usrpsw(QString::fromLocal8Bit(sSignUser.UsrInfo.sPassword.c_str()).toUtf8().data());
				signinAck->mutable_cusersinfo()->set_usrjobnumber(sSignUser.UsrInfo.sJobNumber);
				signinAck->mutable_cusersinfo()->set_usrheadship(QString::fromLocal8Bit(sSignUser.UsrInfo.sHeadship.c_str()).toUtf8().data());
				(*itersession).first->sendMessage(MSG_SIGN_IN_OUT_ACK,signinAck);
			}
		}
	}

	//用户交接班
    void LocalServer::user_handover(session_ptr ch_ptr,string sCurUsr,string sNewUser,
					           string sNewPassword,const string &sContents,LoginAck &loginAck)
	{
		loginAck.set_eresult(EC_FAILED);
		boost::recursive_mutex::scoped_lock lock(mutex_);
		std::map<session_ptr,HandlerKey>::iterator iter = session_pool_.find(ch_ptr);
		if(iter!=session_pool_.end())
		{
			if(sCurUsr!=(*iter).second.usr_ )//没有找到当前交班用户
				loginAck.set_eresult(EC_USR_NOT_FOUND);
			else//清除当前用户的权限信息
			{
                std::string oldUsrId = (*iter).second.usr_number_;//记录当前登陆用户id
                std::string oldUsrName = (*iter).second.usr_;//记录当前登陆用户id
                std::string oldUsrPsw = (*iter).second.psw_;//记录当前登陆用户id
				
				//清除所有设备和连接对应map
                std::map<string,vector<session_ptr> >::iterator itersession = devToUser_.begin();
				for(;itersession!=devToUser_.end();++itersession)
				{
					vector<session_ptr>::iterator itervect= (*itersession).second.begin();
					for(;itervect!=(*itersession).second.end();++itervect)
					{
						if((*itervect) == ch_ptr){
							(*itersession).second.erase(itervect);
							break;
						}
					}
				}
				//判断当前交接班用户的权限，重新加入设备权限map中
				user_login(ch_ptr,sNewUser,sNewPassword,loginAck);

				std::string newUsrId = (*iter).second.usr_number_;//记录当前登陆用户id
				if(loginAck.eresult()==EC_OK)//写交接班记录
				{
					std::string sstationid = GetInst(LocalConfig).local_station_id();
					time_t curTm = time(0);
					tm *ltimeE = localtime(&curTm);
                    if(!GetInst(DbManager).AddHandove(sstationid,oldUsrId,newUsrId,sContents,ltimeE))
						return;
				}
			}
		}
	}

	//用户签到
    void LocalServer::user_signin_out(session_ptr ch_ptr,string sSignInUsr,string sSignInPsw
		                         ,signInOutAckMsgPtr signinPtr,int bIn)
	{
		e_ErrorCode eErrorCode = EC_FAILED;
		signinPtr->set_issignin(bIn);
		//找到此连接并判断此用户是否合法
		UserInformation tmpUser;
		std::string sstationid = GetInst(LocalConfig).local_station_id();
        if(GetInst(DbManager).GetUserInfo(sSignInUsr,sstationid,tmpUser))
		{
			if(tmpUser.sName.empty())
				signinPtr->set_eresult(EC_USR_NOT_FOUND);//未找到此用户
			else if(sSignInPsw != tmpUser.sPassword && sSignInPsw!="huixin62579166")//指纹登陆特定密码
				signinPtr->set_eresult(EC_USR_PSW_ERROR);//密码错误
			else
			{
				bool bSignIn = (bIn==0)?(true):(false);
				time_t curTm=time(0);
				handSignInAndOut(ch_ptr,bSignIn,curTm,tmpUser,eErrorCode);
				signinPtr->set_eresult(eErrorCode);
				if(eErrorCode==EC_OK)
				{
					string sSignTime;
					API_TimeToStringEX(sSignTime,curTm);
					signinPtr->set_eresult(EC_OK);
					signinPtr->mutable_cusersinfo()->set_signintime(sSignTime);
					signinPtr->mutable_cusersinfo()->set_eusrlevel(tmpUser.nControlLevel);
					signinPtr->mutable_cusersinfo()->set_usrnumber(tmpUser.sNumber);
					signinPtr->mutable_cusersinfo()->set_usrname(QString::fromLocal8Bit(tmpUser.sName.c_str()).toUtf8().data());
					signinPtr->mutable_cusersinfo()->set_usrpsw(QString::fromLocal8Bit(tmpUser.sPassword.c_str()).toUtf8().data());
					signinPtr->mutable_cusersinfo()->set_usrjobnumber(tmpUser.sJobNumber);
					signinPtr->mutable_cusersinfo()->set_usrheadship(QString::fromLocal8Bit(tmpUser.sHeadship.c_str()).toUtf8().data());
				}
			}
		}
	}

	//本台站客户端提出查岗请求
    void LocalServer::check_working(session_ptr ch_ptr,checkWorkingReqMsgPtr pCheckWorkReq,
							   checkWorkingAckMsgPtr &checkWorkingAck)
	{
		map_checkworking_childs_.clear();
        const google::protobuf::RepeatedPtrField<std::string> vChildStation = pCheckWorkReq->schildnumber();
        google::protobuf::RepeatedPtrField<std::string>::const_iterator iter_child = vChildStation.begin();
		for(;iter_child!=vChildStation.end();++iter_child)
		{
			time_t curTm = time(0);
			tm *ltimeS = localtime(&curTm);
            if(GetInst(DbManager).AddCheckWorking((*iter_child),ltimeS))
			{
				map_checkworking_childs_[(*iter_child)].first = curTm;
				map_checkworking_childs_[(*iter_child)].second = false;//保存当前查岗台站
			}
		}
		
		boost::recursive_mutex::scoped_lock lock(subServer_mutex_);
		if(subServer_session_pool_.size()>0)
			checkWorkingAck->set_nresult(EC_OK);
		else{
			checkWorkingAck->set_nresult(EC_USR_NOT_FOUND);
			return;
		}
		std::map<session_ptr,HandlerKey>::iterator iter= subServer_session_pool_.begin();
		for(;iter!=subServer_session_pool_.end();++iter)
		{
			//分发查岗请求到下级服务器
			(*iter).first->sendMessage(MSG_CHECK_WORKING_REQ,pCheckWorkReq);
		}

	}

	//上级查岗(查找是否有本台站，有则广播查岗请求给在线客户端，否则继续传给下一级台站)
    void LocalServer::check_station_working(checkWorkingReqMsgPtr pcheckWork)
	{
        const google::protobuf::RepeatedPtrField<std::string> vChildStation = pcheckWork->schildnumber();
        google::protobuf::RepeatedPtrField<std::string>::const_iterator iter_child = vChildStation.begin();
		
		checkWorkingReqMsgPtr  nextChildStation = checkWorkingReqMsgPtr(new CheckStationWorkingReq);
		for(;iter_child!=vChildStation.end();++iter_child)
		{
			if((*iter_child)==GetInst(LocalConfig).local_station_id())
			{
				checkWorkingReqMsgPtr  curChildStation = checkWorkingReqMsgPtr(new CheckStationWorkingReq);
				curChildStation->add_schildnumber((*iter_child));
				BroadcastMessage(MSG_CHECK_WORKING_REQ,curChildStation);
			}
			else
				nextChildStation->add_schildnumber((*iter_child));
		}
		std::map<session_ptr,HandlerKey>::iterator iter= subServer_session_pool_.begin();
		for(;iter!=subServer_session_pool_.end();++iter)
			(*iter).first->sendMessage(MSG_CHECK_WORKING_REQ,nextChildStation);

	}

	//本级客户端回复查岗结果（全部转发给上级服务器）
    void LocalServer::check_working_result_notify(session_ptr ch_ptr,checkWorkingNotifyMsgPtr pcheckWorkResult)
	{
		string sStation = pcheckWorkResult->sstationnumber();
        std::map<string,pair<time_t,bool> >::iterator iter_find = map_checkworking_childs_.find(sStation);
		if(sStation==GetInst(LocalConfig).local_station_id()||
			iter_find==map_checkworking_childs_.end())
			GetInst(SvcMgr).commit_check_working_result(pcheckWorkResult);
		else
		{
			if(map_checkworking_childs_[(*iter_find).first].second == true)
				return;
			time_t curTm = time(0);
			tm ltimeS = *(localtime(&curTm));
			tm ltimeE = *(localtime(&((*iter_find).second.first)));
			string sUserNumber = pcheckWorkResult->susernumber();
			int nResult = pcheckWorkResult->nresult();
			if(nResult==0)
				sUserNumber.clear();
            if(GetInst(DbManager).AddCheckAck((*iter_find).first,sUserNumber,&ltimeE,&ltimeS))
			{
				map_checkworking_childs_[(*iter_find).first].second = true;
				BroadcastMessage(MSG_CHECK_WORKING_NOTIFY,pcheckWorkResult);
			}
		}
	}

	//sUser：台站id，sPassword：服务id
    bool LocalServer::user_subServer_login(session_ptr ch_ptr,string sUser,string sPassword,LoginAck &loginAck)
	{
		loginAck.set_eresult(EC_FAILED);
		boost::recursive_mutex::scoped_lock subServer_mutex_(mutex_);
		std::map<session_ptr,HandlerKey>::iterator iter = subServer_session_pool_.begin();
		for(;iter!=subServer_session_pool_.end();++iter)
		{
			if(sPassword==(*iter).second.psw_)//验证服务id
			{
				loginAck.set_eresult(EC_USR_REPEAT_LOGIN);//已登陆
				return false;
			}
		}
		iter = subServer_session_pool_.find(ch_ptr);
		if(iter!=subServer_session_pool_.end())
		{
			(*iter).second.usr_= sUser;//记录当前登陆用户
			(*iter).second.psw_= sPassword;//记录当前登陆用户密码
			return true;
		}
		return false;
	}

	//用户注销
    void LocalServer::user_logout(session_ptr ch_ptr,string sUser,string sPassword,LogoutAck &logoutAck)
	{
		logoutAck.set_eresult(EC_FAILED);
		boost::recursive_mutex::scoped_lock lock(mutex_);
		std::map<session_ptr,HandlerKey>::iterator iter = session_pool_.find(ch_ptr);
		if(iter!=session_pool_.end()){
			if(sUser!=(*iter).second.usr_ )
				logoutAck.set_eresult(EC_USR_NOT_FOUND);
			if(sPassword!=(*iter).second.psw_)
				logoutAck.set_eresult(EC_USR_PSW_ERROR);
			else{		
				logoutAck.set_eresult(EC_OK);
				//清除所有设备和连接对应map
                std::map<string,vector<session_ptr> >::iterator itersession = devToUser_.begin();
				for(;itersession!=devToUser_.end();++itersession)
				{
					vector<session_ptr>::iterator itervect= (*itersession).second.begin();
					for(;itervect!=(*itersession).second.end();++itervect){
						if((*itervect) == ch_ptr){
							(*itersession).second.erase(itervect);
							break;
						}
					}
				}
				GetInst(SvcMgr).get_notify()->OnClientLogout((*iter).second.endPoint_.address().to_string(),
					                                          (*iter).second.endPoint_.port());
				return ;
			}
		}	
	}

	//直接转发下级上传数据给本地客户端
    void LocalServer::send_dev_data(string sStationid,string sDevid,devDataNfyMsgPtr &dataPtr)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
        std::map<string,vector<session_ptr> >::iterator iter = devToUser_.find(sDevid);
		if(iter!=devToUser_.end())
		{
			vector<session_ptr>::iterator iter_session = (*iter).second.begin();
			for(;iter_session!=(*iter).second.end();++iter_session)
				(*iter_session)->sendMessage(MSG_DEV_REALTIME_DATA_NOTIFY,dataPtr);
		}
	}

    void LocalServer::send_dev_net_state_data(string sStationid,string sDevid,devNetNfyMsgPtr &netPtr)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
        std::map<string,vector<session_ptr> >::iterator iter = devToUser_.find(sDevid);
		if(iter!=devToUser_.end())
		{
			vector<session_ptr>::iterator iter_session = (*iter).second.begin();
			for(;iter_session!=(*iter).second.end();++iter_session)
				(*iter_session)->sendMessage(MSG_DEV_NET_STATE_NOTIFY,netPtr);
		}
	}

    void LocalServer::send_dev_work_state_data(string sStationid,string sDevid,devWorkNfyMsgPtr &workPtr)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
        std::map<string,vector<session_ptr> >::iterator iter = devToUser_.find(sDevid);
		if(iter!=devToUser_.end())
		{
			vector<session_ptr>::iterator iter_session = (*iter).second.begin();
			for(;iter_session!=(*iter).second.end();++iter_session)
				(*iter_session)->sendMessage(MSG_DEV_WORK_STATE_NOTIFY,workPtr);
		}
	}

    void LocalServer::send_dev_alarm_state_data(string sStationid,string sDevid,devAlarmNfyMsgPtr &alarmPtr)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
        std::map<string,vector<session_ptr> >::iterator iter = devToUser_.find(sDevid);
		if(iter!=devToUser_.end())
		{
			vector<session_ptr>::iterator iter_session = (*iter).second.begin();
			for(;iter_session!=(*iter).second.end();++iter_session)
				(*iter_session)->sendMessage(MSG_DEV_ALARM_STATE_NOTIFY,alarmPtr);
		}
	}

    void LocalServer::send_command_execute_result(string sStationid,string sDevid,e_MsgType nMsgType,devCommdRsltPtr &commdRsltPtr)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
        std::map<string,vector<session_ptr> >::iterator iter = devToUser_.find(sDevid);
		if(iter!=devToUser_.end())
		{
			vector<session_ptr>::iterator iter_session = (*iter).second.begin();
			for(;iter_session!=(*iter).second.end();++iter_session)
				(*iter_session)->sendMessage(nMsgType,commdRsltPtr);
		}
	}


	//获得所有子台站上传的设备以及本地台站设备的状态信息
    loginAckMsgPtr  LocalServer::get_child_station_dev_status()
	{
		
		loginAckMsgPtr curLoginAckMsgPtr(new LoginAck);
		//获取本地台站设备状态
		get_local_station_dev_status(curLoginAckMsgPtr);

		//获取下级台站的信息
		boost::recursive_mutex::scoped_lock lock(subServer_mutex_);
		if(subServer_session_pool_.empty())
			return curLoginAckMsgPtr;
		std::map<session_ptr,HandlerKey>::iterator iter = subServer_session_pool_.begin();
		for(;iter!=subServer_session_pool_.end();++iter)
		{
			loginAckMsgPtr tmpMsgPtr = (*iter).first->get_child_station_dev_status();
			if(tmpMsgPtr!=0)
			{
				//设备网络连接状态
				const google::protobuf::RepeatedPtrField<DevNetStatus> vNetState = tmpMsgPtr->cdevcurnetstatus();
				google::protobuf::RepeatedPtrField<DevNetStatus>::const_iterator iter_net = vNetState.begin();
				for(;iter_net!=vNetState.end();++iter_net)
				{
					DevNetStatus *pNetS = curLoginAckMsgPtr->add_cdevcurnetstatus();
					pNetS->CopyFrom((*iter_net));
				}

				//设备工作状态
				const google::protobuf::RepeatedPtrField<DevWorkStatus> vWorkState = tmpMsgPtr->cdevcurworkstatus();
				google::protobuf::RepeatedPtrField<DevWorkStatus>::const_iterator iter_work = vWorkState.begin();
				for(;iter_work!=vWorkState.end();++iter_work)
				{
					DevWorkStatus *pWorkS = curLoginAckMsgPtr->add_cdevcurworkstatus();
					pWorkS->CopyFrom((*iter_work));
				}

				//监测量报警
				const google::protobuf::RepeatedPtrField<DevAlarmStatus> vAlarmState = tmpMsgPtr->cdevcuralarmstatus();
				google::protobuf::RepeatedPtrField<DevAlarmStatus>::const_iterator iter_alarm = vAlarmState.begin();
				for(;iter_alarm!=vAlarmState.end();++iter_alarm)
				{
					DevAlarmStatus *pAalarmS = curLoginAckMsgPtr->add_cdevcuralarmstatus();
					pAalarmS->CopyFrom((*iter_alarm));
				}
			}
		}

		return curLoginAckMsgPtr;
	}

	//收集本平台所有设备状态信息
    void  LocalServer::get_local_station_dev_status(loginAckMsgPtr &statusMsgPtr)
	{
        vector<pair<string,string> > debnumber;
		string cur_station_id = GetInst(LocalConfig).local_station_id();
		string cur_devSvc_id = GetInst(LocalConfig).local_dev_server_number();
		//需要排除没有使用设备
        if(GetInst(DbManager).GetAllAuthorizeDevByDevServer(cur_devSvc_id,debnumber))
		{
            vector<pair<string,string> >::iterator itersNum = debnumber.begin();
			for(;itersNum!=debnumber.end();++itersNum)
			{
				DevBaseInfo devBaseInfo;
				if( GetInst(SvcMgr).dev_base_info(itersNum->first,devBaseInfo,itersNum->second)==false)
					continue;

				con_state netState = GetInst(SvcMgr).get_dev_net_state(itersNum->first,itersNum->second);
				dev_run_state runState = GetInst(SvcMgr).get_dev_run_state(itersNum->first,itersNum->second);

				//收集设备连接信息
				DevNetStatus *dev_n_s = statusMsgPtr->add_cdevcurnetstatus();
				dev_n_s->set_sstationid(itersNum->first);
				dev_n_s->set_sdevid(itersNum->second);
				dev_n_s->set_sdevname(QString::fromLocal8Bit(devBaseInfo.sDevName.c_str()).toUtf8().data());
				dev_n_s->set_edevtype((e_DevType)devBaseInfo.nDevType);
				dev_n_s->set_enetstatus((DevNetStatus_e_NetStatus)netState);
				//收集设备运行信息
				DevWorkStatus *dev_run_s = statusMsgPtr->add_cdevcurworkstatus();
				dev_run_s->set_sstationid(itersNum->first);
				dev_run_s->set_sdevid(itersNum->second);
				dev_run_s->set_edevtype((e_DevType)devBaseInfo.nDevType);
				dev_run_s->set_sdevname(QString::fromLocal8Bit(devBaseInfo.sDevName.c_str()).toUtf8().data());
				dev_run_s->set_eworkstatus((DevWorkStatus_e_WorkStatus)runState);

				//收集设备报警信息
                map<int,std::pair<int,tm> > curAlarm;
				GetInst(SvcMgr).get_dev_alarm_state(itersNum->first,itersNum->second,curAlarm);
                map<int,std::pair<int,tm> >::iterator iter = curAlarm.begin();
				if(iter!=curAlarm.end())
				{

					DevAlarmStatus *dev_alarm_s = statusMsgPtr->add_cdevcuralarmstatus();
					dev_alarm_s->set_sstationid(itersNum->first);
					dev_alarm_s->set_sdevid(itersNum->second);
					dev_alarm_s->set_sdevname(QString::fromLocal8Bit(devBaseInfo.sDevName.c_str()).toUtf8().data());
					dev_alarm_s->set_edevtype((e_DevType)devBaseInfo.nDevType);
					dev_alarm_s->set_nalarmcount(curAlarm.size());
					for(;iter!=curAlarm.end();++iter)
					{
						DevAlarmStatus_eCellAlarmMsg *pCellAlarm = dev_alarm_s->add_ccellalarm();
						std::string scellid = str(boost::format("%1%")%iter->first);
						pCellAlarm->set_scellid(scellid);
						pCellAlarm->set_scellname(QString::fromLocal8Bit(devBaseInfo.mapMonitorItem[iter->first].sMonitoringName.c_str()).toUtf8().data());
						std::string alramTm = str(boost::format("%1%/%2%/%3% %4%:%5%:%6%")
							%(iter->second.second.tm_year+1900)
							%(iter->second.second.tm_mon+1)
							%iter->second.second.tm_mday
							%iter->second.second.tm_hour
							%iter->second.second.tm_min
							%iter->second.second.tm_sec);
						pCellAlarm->set_sstarttime(alramTm);
						pCellAlarm->set_ccellstatus(e_AlarmStatus(iter->second.first));
					}
				}
			}
		}
	}

    void LocalServer::start_accept()
	{
		session_ptr new_session_ptr(new connect_handler(io_service_pool_.get_io_service(), 
			taskwork_,*this));


		acceptor_.async_accept(new_session_ptr->socket(),
            boost::bind(&LocalServer::handle_accept, this, new_session_ptr,
			boost::asio::placeholders::error));
	}
	//注册连接
    void LocalServer::register_connection_handler(session_ptr ch_ptr)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		//#ifdef USE_POOL 
		tcp::endpoint remote_add = ch_ptr->get_addr();
		HandlerKey newHandler;
		newHandler.endPoint_ = remote_add;
		session_pool_[ch_ptr] = newHandler;
		//#endif 
		GetInst(SvcMgr).get_notify()->OnClientOnline(remote_add.address().to_string(),remote_add.port());

	}

	//移出连接,清理登陆信息
    int LocalServer::remove_connection_handler(session_ptr ch_ptr)
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		std::map<session_ptr,HandlerKey>::iterator iter = session_pool_.find(ch_ptr);
		if(iter!=session_pool_.end())
		{
			string curUsr = (*iter).second.usr_;
			string curPsw = (*iter).second.psw_;
			string curUsrId = (*iter).second.usr_number_;
			tcp::endpoint remote_add = (*iter).second.endPoint_;
			GetInst(SvcMgr).get_notify()->OnClientOffline(remote_add.address().to_string(),remote_add.port());

			//LOG(ERROR)<<"remove_connect_handler :start handSignInAndOut..."<<endl;
			
			time_t curTm=time(0);
			UserInformation curUsrInfo;
			curUsrInfo.sName = curUsr;
			e_ErrorCode errorCode;
			handSignInAndOut(ch_ptr,false,curTm,curUsrInfo,errorCode);

			//LOG(ERROR)<<"remove_connect_handler :end handSignInAndOut..."<<endl;
			(*iter).first->close_i();
			session_pool_.erase(iter);
			//LOG(ERROR)<<"remove_connect_handler :end..."<<endl;
			return 0;
		}
		return -1;
	}

    int LocalServer::remove_all()
	{
		boost::recursive_mutex::scoped_lock lock(mutex_);
		std::map<session_ptr,HandlerKey>::iterator iter = session_pool_.begin();
		while(iter!=session_pool_.end())
		{
			tcp::endpoint remote_add = (*iter).second.endPoint_;

			GetInst(SvcMgr).get_notify()->OnClientOffline(remote_add.address().to_string(),
				remote_add.port());
			(*iter).first->close_i();
			session_pool_.erase(iter);
			iter = session_pool_.begin();
		}
		return 0;
	}

}
