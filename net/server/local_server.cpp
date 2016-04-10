#include "local_server.h"
#include "connect_handler.h"
#include "../SvcMgr.h"
#include "../../DataType.h"
#include "../../LocalConfig.h"
#include "../../database/DataBaseOperation.h"
#include "../../protocol/protocol.pb.h"
#include <stdio.h>
/////////////////////////////////////////////////////////////////////////////////
// 设备监测服务类，给客户端提供数据服务，接受下级服务提交的数据
using namespace db;
namespace hx_net
{
    LocalServer::LocalServer(short port,TaskQueue<msgPointer>& taskwork,size_t io_service_pool_size/*=4*/)
		:io_service_pool_(io_service_pool_size)//设置io pool尺寸
		,taskwork_(taskwork)//引用任务队列 
        ,acceptor_(io_service_pool_.get_io_service(),tcp::endpoint(tcp::v4(),port))
		,exit_(false)
	{
		start_accept();
	}

    LocalServer::~LocalServer()
	{
		session_pool_.clear();
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
        remove_all();
		acceptor_.close();
		io_service_pool_.stop();
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
        for(;iter!=session_pool_.end();++iter){
            if(sUser==(*iter).second.usr_)	{
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
            if(!GetInst(DataBaseOperation).GetUserInfo(sUser,tmpUser))
				loginAck.set_eresult(EC_FAILED);//获取用户信息失败（访问数据库出错）
			else
			{
				if(tmpUser.sName == "")
					loginAck.set_eresult(EC_USR_NOT_FOUND);//未找到此用户
                else if(sPassword != tmpUser.sPassword)
					loginAck.set_eresult(EC_USR_PSW_ERROR);//密码错误
				else
				{
					loginAck.set_eresult(EC_OK);
					loginAck.set_eusrlevel(tmpUser.nControlLevel);
					loginAck.set_usrnumber(tmpUser.sNumber);
                    loginAck.set_usrname(tmpUser.sName.c_str());
                    loginAck.set_usrpsw(tmpUser.sPassword.c_str());
					loginAck.set_usrjobnumber(tmpUser.sJobNumber);
                    loginAck.set_usrheadship(tmpUser.sHeadship.c_str());
					
					(*iter).second.usr_= sUser;//记录当前登陆用户
					(*iter).second.psw_= tmpUser.sPassword;//记录当前登陆用户密码
					(*iter).second.usr_number_ = tmpUser.sNumber;//记录当前用户编号 amend by lk 2014-3-24
					
 
                    //登陆成功
                    tcp::endpoint remote_add = ch_ptr->get_addr();
                    GetInst(SvcMgr).get_notify()->OnClientLogin(remote_add.address().to_string(),remote_add.port(),
                                                                tmpUser.sName,tmpUser.sNumber);
                    //查找该用户授权设备，并插入到设备-〉用户映射表
                    vector<string> debnumber;
                    if(GetInst(DataBaseOperation).GetAllAuthorizeDevByUser(tmpUser.sNumber,debnumber))
                    {
                        vector<string>::iterator itersNum = debnumber.begin();
                        vector<string> usrTodev;//用户设备列表
                        for(;itersNum!=debnumber.end();++itersNum)
                        {
                            usrTodev.push_back(*itersNum);
                            //判断设备是否属于本地台站或者是上级台站直连下级台站设备
                            DevBaseInfo devBaseInfo;
                            GetInst(SvcMgr).dev_base_info(sstationid,devBaseInfo,*itersNum);

                            map<int,map<int,CurItemAlarmInfo> > curAlarm;
                            con_state  netState = GetInst(SvcMgr).get_dev_net_state(sstationid,*itersNum);
                            dev_run_state   runState = GetInst(SvcMgr).get_dev_run_state(sstationid,*itersNum);

                            GetInst(SvcMgr).get_dev_alarm_state(sstationid,*itersNum,curAlarm);
                            //收集设备连接信息
                            DevNetStatus *dev_n_s = loginAck.add_cdevcurnetstatus();
                            dev_n_s->set_sstationid(sstationid);
                            dev_n_s->set_sdevid(*itersNum);
                            dev_n_s->set_sdevname(devBaseInfo.sDevName.c_str());
                            dev_n_s->set_edevtype(devBaseInfo.nDevType);
                            dev_n_s->set_enetstatus((DevNetStatus_e_NetStatus)netState);
                            //收集设备运行信息
                            DevWorkStatus *dev_run_s = loginAck.add_cdevcurworkstatus();
                            dev_run_s->set_sstationid(sstationid);
                            dev_run_s->set_sdevid(*itersNum);
                            dev_run_s->set_edevtype(devBaseInfo.nDevType);
                            dev_run_s->set_sdevname(devBaseInfo.sDevName.c_str());
                            dev_run_s->set_eworkstatus((DevWorkStatus_e_WorkStatus)runState);
                            //收集设备报警信息
                            map<int,map<int,CurItemAlarmInfo> >::iterator iter = curAlarm.begin();
                            if(iter!=curAlarm.end())
                            {
                                DevAlarmStatus *dev_alarm_s = loginAck.add_cdevcuralarmstatus();
                                dev_alarm_s->set_sstationid(sstationid);
                                dev_alarm_s->set_sdevid(*itersNum);
                                dev_alarm_s->set_sdevname(devBaseInfo.sDevName.c_str());
                                dev_alarm_s->set_edevtype(devBaseInfo.nDevType);
                                dev_alarm_s->set_nalarmcount(curAlarm.size());
                                for(;iter!=curAlarm.end();++iter)
                                {
                                    map<int,CurItemAlarmInfo>::iterator iter_a = iter->second.begin();
                                    for(;iter_a!=iter->second.begin();++iter_a){
                                        DevAlarmStatus_eCellAlarmMsg *pCellAlarm = dev_alarm_s->add_ccellalarm();
                                        std::string scellid = str(boost::format("%1%")%iter->first);
                                        pCellAlarm->set_scellid(scellid);
                                        pCellAlarm->set_sdesp(iter_a->second.sReason);
                                        char str_time[64];
                                        tm *local_time = localtime(&iter_a->second.startTime);
                                        strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", local_time);
                                        pCellAlarm->set_sstarttime(str_time);
                                        pCellAlarm->set_ccellstatus(e_AlarmStatus(iter_a->first));
                                    }
                                }
                            }
                            devToUser_[*itersNum].push_back(ch_ptr);
                        }

                    }
                }
            }

        }

		return ;
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

			(*iter).first->close_i();
			session_pool_.erase(iter);
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
