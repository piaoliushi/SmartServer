#include "Tsmt_message.h"
#include "../../../LocalConfig.h"
#include "../../../StationConfig.h"
#include "./net/SvcMgr.h"
#include "./transmmiter/Transmmiter.h"
#include "./transmmiter/CDtransmmiter.h"
namespace hx_net
{

    Tsmt_message::Tsmt_message(session_ptr pSession,boost::asio::io_service& io_service,
                               DeviceInfo &devInfo,pDevicePropertyExPtr devProperty)
        :d_devInfo(devInfo)
        ,task_timeout_timer_(io_service)
        ,d_devProperty(devProperty)
        ,d_relate_tsmt_ptr_(NULL)
        ,d_relate_antenna_ptr_(NULL)
        ,d_Host_(-1)
        ,d_relate_Agent_(false)
        ,d_antenna_Agent_(false)
        ,d_cur_task_(-1)
        ,m_ptransmmit(NULL)
    {
        CreateObject();

        m_pSession = boost::dynamic_pointer_cast<device_session>(pSession);
        map<int,vector<AssDevChan> >::iterator find_iter = d_devInfo.map_AssDevChan.find(0);
        if(find_iter!=d_devInfo.map_AssDevChan.end()){
            vector<AssDevChan>::iterator dev_iter = find_iter->second.begin();
            for(;dev_iter!=find_iter->second.end();++dev_iter){
                DeviceInfo *curDev =  GetInst(StationConfig).get_devinfo_by_id((*dev_iter).sAstNum);
                if(curDev->iDevType == DEVICE_TRANSMITTER)
                    d_relate_tsmt_ptr_ = curDev;
                if(curDev->iDevType == DEVICE_ANTENNA)
                    d_relate_antenna_ptr_ = curDev;
            }
        }
        map<string,DevProperty>::iterator iter = d_devInfo.map_DevProperty.find("Standby");
        if(iter!=d_devInfo.map_DevProperty.end())
            d_Host_= (iter->second.property_value == "1")?1:0;//1->主机，０->备机

        if(d_relate_tsmt_ptr_!=NULL){
        iter = d_relate_tsmt_ptr_->map_DevProperty.find("Agent");
        if(iter!=d_relate_tsmt_ptr_->map_DevProperty.end())
             d_relate_Agent_= true;
        }

        if(d_relate_antenna_ptr_!=NULL){
        iter = d_relate_antenna_ptr_->map_DevProperty.find("Agent");
        if(iter!=d_relate_antenna_ptr_->map_DevProperty.end())
             d_antenna_Agent_= true;
        }

	}

    Tsmt_message::~Tsmt_message(void)
	{
	}

    void Tsmt_message::SetProtocol(int mainprotocol,int subprotocol)
    {
        m_Subprotocol =   subprotocol;
        m_mainprotocol =  mainprotocol;
    }

    int Tsmt_message::check_msg_header(unsigned char *data,int nDataLen)
	{
        return m_ptransmmit->check_msg_header(data,nDataLen);
	}
    int Tsmt_message::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
        int irunstate=dev_unknown;
        int idecresult = m_ptransmmit->decode_msg_body(data,data_ptr,nDataLen,irunstate);
        if(idecresult == 0) {
            GetResultData(data_ptr);
            if(irunstate==dev_unknown)
                detect_run_state(data_ptr);
            else {
                //设置运行状态
            }
        }
        return idecresult;
	}
    //获得运行状态
    int Tsmt_message::get_run_state()
    {
        boost::recursive_mutex::scoped_lock llock(run_state_mutex_);
        return dev_run_state_;
    }

    //设置运行状态
    void Tsmt_message::set_run_state(int curState)
    {
        boost::recursive_mutex::scoped_lock llock(run_state_mutex_);
        if(dev_run_state_ != curState)
        {
            dev_run_state_=curState;
            //GetInst(SvcMgr).get_notify()->OnRunState(this->TransmitterInfo.sNumber,dev_run_state_);
            //广播设备状态到在线客户端
            m_pSession->send_work_state_message(GetInst(LocalConfig).local_station_id(),d_devInfo.sDevNum
                                    ,d_devInfo.sDevName,TRANSMITTER,(dev_run_state)dev_run_state_);
        }
    }

    //设备运行状态
    bool Tsmt_message::is_running()
    {
        return (get_run_state()==dev_running)?true:false;
    }

     bool Tsmt_message::is_shut_down()
     {
         return (get_run_state()==dev_shutted_down)?true:false;
     }

     bool Tsmt_message::is_detecting()
     {
         return (get_run_state()==dev_detecting)?true:false;
     }

    //检测发射机运行状态
    void Tsmt_message::detect_run_state(DevMonitorDataPtr curDataPtr)
    {
        if(curDataPtr->mValues[0].fValue >d_devProperty->zero_power_value)
        {
            if(is_running())
                return;
            set_run_state(dev_detecting);
            d_shutdown_count_=0;
            ++d_run_count_;
            if(d_run_count_>d_devProperty->run_detect_max_count) {//已开机
               set_run_state(dev_running);
            }
        } else {
            if(is_shut_down())
                return;
            set_run_state(dev_detecting);
            d_run_count_=0;
            ++d_shutdown_count_;
            if(d_shutdown_count_>d_devProperty->shutdown_detect_max_count)  {//已关机
               set_run_state(dev_shutted_down);
            }
        }
    }

    bool Tsmt_message::IsStandardCommand()
	{

        return m_ptransmmit->IsStandardCommand();
	}
	
    void Tsmt_message::GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit)
	{
		
	}

    int   Tsmt_message::cur_dev_state()
    {
            return -1;
    }

    void Tsmt_message::GetAllCmd(CommandAttribute &cmdAll)
    {
        if(m_ptransmmit!=NULL)
        {
            m_ptransmmit->GetAllCmd(cmdAll);
        }
    }

    void Tsmt_message::GetResultData( DevMonitorDataPtr data_ptr )
    {
        map<int,DeviceMonitorItem>::iterator iter = d_devInfo.map_MonitorItem.begin();
        for(;iter!=d_devInfo.map_MonitorItem.end();++iter)
        {
            map<int,DataInfo>::iterator diter = data_ptr->mValues.find((*iter).first);
            if(diter!=data_ptr->mValues.end())
            {
                if((*iter).second.iItemType == 0)
                    data_ptr->mValues[(*iter).first].fValue *= (*iter).second.dRatio;
                else
                {
                    if((*iter).second.dRatio==0)
                    {
                        data_ptr->mValues[(*iter).first].fValue = data_ptr->mValues[(*iter).first].fValue==1.0f ? 0:1;
                    }
                }
            }
        }
    }

    void Tsmt_message::CreateObject()
    {
        switch(d_devInfo.nDevProtocol)
        {
        case BEIJ_GME:
            break;
        case BEIJING_BEIGUANG:
            break;
        case CHENGDU_KT_CG:
            {
                m_ptransmmit = new CDtransmmiter(d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            }
            break;
        case SHANGUANG:
            break;
        case HAGUANG:
            break;
        case ANSHAN:
            break;
        case HANGCHUN:
            break;
        case ITALY_TECHNO:
            break;
        case EKA:
            break;
        case GE_RS:
            break;
        case DLDZ:
            break;
        case CHENGDU_KANGTE_N:
            break;
        case LIAONING_HS:
            break;
        case SHANGHAI_MZ:
            break;
        case HARRIS:
            break;
        case DE_XIN:
            break;
        }
    }

    void Tsmt_message::exec_task_now(int icmdType,string sUser)
    {
        //连接失效,返回
        if(!m_pSession->is_connected()){
            //nResult = EC_NET_ERROR;
            return;
        }

        excute_task_cmd();

        d_cur_user_ = sUser;//记录当前用户，定时，自动，人工
        std::time(&d_OprStartTime);//循环执行计时开始
        d_cur_task_ = icmdType;
        start_task_timeout_timer();

    }

    //启动任务定时器
    void Tsmt_message::start_task_timeout_timer()
    {
        task_timeout_timer_.expires_from_now(boost::posix_time::seconds(d_devProperty->cmd_timeout_interval));
        task_timeout_timer_.async_wait(boost::bind(&Tsmt_message::schedules_task_time_out,
            this,boost::asio::placeholders::error));
    }

    void Tsmt_message::excute_task_cmd()
    {
        switch (d_cur_task_) {
        case MSG_TRANSMITTER_TURNON_OPR:
            exec_trunon_task_(HIGH_POWER_ON);
            break;
        case MSG_TRANSMITTER_MIDDLE_POWER_TURNON_OPR:
            exec_trunon_task_(MIDDLE_POWER_ON);
            break;
        case MSG_TRANSMITTER_LOW_POWER_TURNON_OPR:
            exec_trunon_task_(LOW_POWER_ON);
            break;
        case MSG_TRANSMITTER_TURNOFF_OPR:
            exec_trunoff_task_();
            break;
        }
    }

    //任务超时回调
    void Tsmt_message::schedules_task_time_out(const boost::system::error_code& error)
    {
        if(!m_pSession->is_connected())
            return;

        if(error!= boost::asio::error::operation_aborted)
        {
            time_t tmCurTime;
            time(&tmCurTime);
            double ninterval = difftime(tmCurTime,d_OprStartTime);
             tm *pCurTime = localtime(&tmCurTime);
            bool  bCmdExcComplet = false;
            switch (d_cur_task_) {
            case MSG_TRANSMITTER_TURNON_OPR:
            case MSG_TRANSMITTER_MIDDLE_POWER_TURNON_OPR:
            case MSG_TRANSMITTER_LOW_POWER_TURNON_OPR:{
                if(is_running())
                    bCmdExcComplet = true;
            } break;
            case MSG_TRANSMITTER_TURNOFF_OPR:{
                if(is_shut_down())
                    bCmdExcComplet = true;
            } break;
            }

            if(bCmdExcComplet == true){
                //通知到客户端...
                m_pSession->notify_client(d_devInfo.sDevNum,d_devInfo.sDevName,d_cur_user_,
                                          d_cur_task_,pCurTime,EC_OK);
                m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
                d_cur_user_.clear();
                d_cur_task_ = -1;
                return;
            }else{
                //超时
                if(ninterval>=d_devProperty->cmd_excut_timeout_duration) {
                    //通知到客户端...
                    m_pSession->notify_client(d_devInfo.sDevNum,d_devInfo.sDevName,d_cur_user_,
                                              d_cur_task_,pCurTime,EC_FAILED);
                    m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
                    d_cur_user_.clear();
                    d_cur_task_ = -1;
                    return;
                }else{
                    excute_task_cmd();
                    start_task_timeout_timer();
                }
            }
        }
    }

    void Tsmt_message::exec_trunon_task_(int nType)
    {
        //如果发射机正在监测运行状态，不执行开机操作
        if(is_detecting())
            return;
        if(d_relate_tsmt_ptr_!=NULL && d_relate_antenna_ptr_!=NULL){
            //非定时开关机，且关联主机在使用
            if(d_cur_user_!="timer" && d_relate_tsmt_ptr_->bUsed==true){
                //当前主机在运行且天线不是代理则返回，不进行单步开机动作
                if(GetInst(SvcMgr).get_dev_run_state(GetInst(LocalConfig).local_station_id(),
                       d_relate_tsmt_ptr_->sDevNum)==dev_running  && d_antenna_Agent_==false) {
                    return;
                }
            }
            //主机使用则进行关主机动作
            if(d_relate_tsmt_ptr_->bUsed==true) {
                if(EC_OK != GetInst(SvcMgr).start_exec_task(d_relate_tsmt_ptr_->sDevNum,d_cur_user_,MSG_TRANSMITTER_TURNOFF_OPR
                                                            ))
                    return ;
            }
            //计算目标天线位置
            e_MsgType nAntennaPos = (d_Host_==true)?MSG_ANTENNA_BTOH_OPR:MSG_ANTENNA_HTOB_OPR;
            //验证并切换天线
            if(d_antenna_Agent_ == false){//如果天线是代理，不进行倒天线动作
                if(EC_OK != GetInst(SvcMgr).start_exec_task(d_relate_antenna_ptr_->sDevNum,d_cur_user_,nAntennaPos
                                                            ))
                    return ;
            }
        }

        if(m_pSession!=NULL)
            m_pSession->send_cmd_to_dev(d_devInfo.sDevNum,d_cur_task_,nType);
    }
    //关机
    void Tsmt_message::exec_trunoff_task_()
    {
        if(is_detecting())
            return;
        if(m_pSession!=NULL)
            m_pSession->send_cmd_to_dev(d_devInfo.sDevNum,d_cur_task_,0);
        return;
    }
}
