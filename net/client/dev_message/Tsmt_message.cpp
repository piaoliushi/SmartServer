#include "Tsmt_message.h"
#include "../../../LocalConfig.h"
#include "../../../StationConfig.h"
#include "./net/SvcMgr.h"
#include "./transmmiter/Transmmiter.h"
#include "./transmmiter/CDtransmmiter.h"
#include "./transmmiter/GmeTransmmit.h"
#include "./transmmiter/ShTransmmit.h"
#include "./transmmiter/AhhxTransmmit.h"
#include "./transmmiter/GlsqTransmmit.h"
#include "./transmmiter/dexintransmmit.h"
#include "./transmmiter/gsbrtransmmit.h"
#include "./transmmiter/zctransmmit.h"
#include "./transmmiter/ckangtetransmmit.h"
#include "./transmmiter/harristransmmit.h"
#include "./transmmiter/sgtransmmit.h"
#include "./transmmiter/hgtransmmitr.h"
#include "./transmmiter/bgtransmmiter.h"
#include "./transmmiter/hctransmmiter.h"
#include "./transmmiter/rstransmmit.h"
#include "./transmmiter/rvrtransmmit.h"
#include "./transmmiter/astransmmit.h"

#include "../../../database/DataBaseOperation.h"
using namespace db;
namespace hx_net
{

    Tsmt_message::Tsmt_message(session_ptr pSession,boost::asio::io_service& io_service,
                               DeviceInfo &devInfo,pDevicePropertyExPtr devProperty)
        :base_message()
        ,d_devInfo(devInfo)
        ,task_timeout_timer_(io_service)
        ,d_devProperty(devProperty)
        ,d_relate_tsmt_ptr_(NULL)
        ,d_relate_antenna_ptr_(NULL)
        ,d_Host_(-1)
        ,d_relate_Agent_(false)
        ,d_antenna_Agent_(false)
        ,d_onekeyopen_996(false)
        ,d_onekeyopen_soft(false)
        ,d_cur_task_(-1)
        ,d_ptransmmit(NULL)
        ,dev_run_state_(dev_unknown)
        ,d_shutdown_count_(0)
        ,d_run_count_(0)
        ,d_bUse_snmp(false)
        ,d_cur_snmp(NULL)
        ,d_cur_target(NULL)
        ,d_curStep(-1)
    {
        m_pSession = boost::dynamic_pointer_cast<device_session>(pSession);
        CreateObject();
         if(d_ptransmmit!=NULL && d_ptransmmit->IsStandardCommand())
            d_curData_ptr = DevMonitorDataPtr(new Data);
        map<int,vector<AssDevChan> >::iterator find_iter = d_devInfo.map_AssDevChan.find(0);
        if(find_iter!=d_devInfo.map_AssDevChan.end()){
            vector<AssDevChan>::iterator dev_iter = find_iter->second.begin();
            for(;dev_iter!=find_iter->second.end();++dev_iter){
                DeviceInfo *curDev =  GetInst(StationConfig).get_devinfo_by_id((*dev_iter).sAstNum);
                if(curDev==NULL)
                    continue;
                if(curDev->iDevType == DEVICE_TRANSMITTER && (*dev_iter).iAssType == DEV_TO_DEV)
                    d_relate_tsmt_ptr_ = curDev;
                if(curDev->iDevType == DEVICE_ANTENNA && (*dev_iter).iAssType == TSMT_TO_ATTENA)
                    d_relate_antenna_ptr_ = curDev;
            }
        }
        map<string,DevProperty>::iterator iter = d_devInfo.map_DevProperty.find("Standby");
        if(iter!=d_devInfo.map_DevProperty.end())
            d_Host_= (iter->second.property_value == "0")?TRANSMITTER_HOST:TRANSMITTER_BACKUP;//0->主机，1->备机

        if(d_relate_tsmt_ptr_!=NULL){
            iter = d_relate_tsmt_ptr_->map_DevProperty.find("Agent");
            if(iter!=d_relate_tsmt_ptr_->map_DevProperty.end())
                 d_relate_Agent_= true;
        }

        if(d_relate_antenna_ptr_!=NULL){
            iter = d_relate_antenna_ptr_->map_DevProperty.find("Agent");
            if(iter!=d_relate_antenna_ptr_->map_DevProperty.end())
                 d_antenna_Agent_= true;
            iter = d_relate_antenna_ptr_->map_DevProperty.find("WPBootFor996");
            if(iter!=d_relate_antenna_ptr_->map_DevProperty.end())
                 d_onekeyopen_996= true;
            iter = d_relate_antenna_ptr_->map_DevProperty.find("WPBootForServer");
            if(iter!=d_relate_antenna_ptr_->map_DevProperty.end())
                 d_onekeyopen_soft= true;
        }
        if(d_devProperty->is_step_open_close)
        {
            GetInst(LocalConfig).device_step_cmd(d_devInfo.sDevNum,d_step_oc_cmd);
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

    int Tsmt_message::check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number)
	{
        if(!d_ptransmmit)
            return RE_UNKNOWDEV;
        return d_ptransmmit->check_msg_header(data,nDataLen,cmdType,number);
	}

    int Tsmt_message::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode)
    {
        int irunstate=dev_unknown;
        iaddcode = d_devInfo.iAddressCode;
        if(data_ptr!=NULL)
            d_curData_ptr = data_ptr;
        int idecresult = d_ptransmmit->decode_msg_body(data,d_curData_ptr,nDataLen,irunstate);
        if(idecresult == 0 ) {
            GetResultData(d_curData_ptr);
            if(irunstate==dev_unknown)
                detect_run_state(d_curData_ptr);
            else {
                //设置运行状态
                set_run_state(irunstate);
            }
            //
            d_checkData_ptr = d_curData_ptr;
            if(d_ptransmmit->IsStandardCommand()){
                m_pSession->start_handler_data(d_devInfo.sDevNum,d_curData_ptr);
            }
        }
        return idecresult;
    }

    //snmp设备
    int Tsmt_message::decode_msg_body(Snmp *snmp, DevMonitorDataPtr data_ptr, CTarget *target)
    {
        int irunstate=dev_unknown;
        if(data_ptr!=NULL)
            d_curData_ptr = data_ptr;
        if(!d_ptransmmit)
            return RE_UNKNOWDEV;
        int idecresult = d_ptransmmit->decode_msg_body(snmp,d_curData_ptr,target,irunstate);

        return idecresult;
    }
    
    //http消息解析
    int  Tsmt_message::decode_http_msg(const string &data,DevMonitorDataPtr data_ptr,CmdType cmdType,int number)
    {
        int irunstate=dev_unknown;

        if(data_ptr!=NULL)
            d_curData_ptr = data_ptr;
        int idecresult = d_ptransmmit->decode_msg_body(data,d_curData_ptr,cmdType,number,irunstate);
        if(idecresult == 0 ) {
            GetResultData(d_curData_ptr);
            if(irunstate==dev_unknown)
                detect_run_state(d_curData_ptr);
            else {
                //设置运行状态
                set_run_state(irunstate);
            }

            if(d_ptransmmit->IsStandardCommand()){
                m_pSession->start_handler_data(d_devInfo.sDevNum,d_curData_ptr);
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
            GetInst(SvcMgr).get_notify()->OnDevStatus(d_devInfo.sDevNum,dev_run_state_+1);
            //广播设备状态到在线客户端GetInst(LocalConfig).local_station_id()
            m_pSession->send_work_state_message(d_devInfo.sStationNum,d_devInfo.sDevNum
                                                ,d_devInfo.sDevName,DEVICE_TRANSMITTER,(dev_run_state)dev_run_state_);
            //如果天线是代理，且当前机器在运行，则设置当前天线状态为当前设备
            if(d_relate_antenna_ptr_!=NULL){
                if(dev_run_state_ == dev_running && d_antenna_Agent_){

                    int nAtennaS = (d_Host_ == 1)?antenna_backup:antenna_host;//GetInst(LocalConfig).local_station_id()
                    GetInst(SvcMgr).set_dev_run_state(d_devInfo.sStationNum,
                                                      d_relate_antenna_ptr_->sDevNum,nAtennaS);
                }
            }

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

    void Tsmt_message::reset_run_state()
    {
        set_run_state(dev_unknown);
    }

    void Tsmt_message::aysnc_data(DevMonitorDataPtr curDataPtr)
    {
        GetResultData(d_curData_ptr);
        detect_run_state(d_curData_ptr);
        m_pSession->start_handler_data(d_devInfo.sDevNum,d_curData_ptr);
    }

    void  Tsmt_message::check_device_alarm(int nAlarmType)
    {

    }

    int Tsmt_message::getsteptimeout()
    {
        if(d_curStep<0)
            return -1;
        if(d_cur_task_!=MSG_TRANSMITTER_TURNOFF_OPR)
        {
           return  d_step_oc_cmd.mapstepopencmd[d_curStep].stcmdtimeout;
        }
        else{
            return  d_step_oc_cmd.mapstepclosecmd[d_curStep].stcmdtimeout;
        }
    }

    bool Tsmt_message::is_step_task_exeok()
    {
        if(d_curStep<0)
            return false;
        if(d_cur_task_!=MSG_TRANSMITTER_TURNOFF_OPR)
        {
           // map<int,DataInfo> mValues;
            if(d_checkData_ptr->mValues.find(d_step_oc_cmd.mapstepopencmd[d_curStep].checkindex)!=d_checkData_ptr->mValues.end())
            {

                if(d_checkData_ptr->mValues[d_step_oc_cmd.mapstepopencmd[d_curStep].checkindex].fValue==d_step_oc_cmd.mapstepopencmd[d_curStep].fvalue)
                    return true;
                else
                    return false;
            }
            else
                return false;
        }
        else
        {
            if(d_checkData_ptr->mValues.find(d_step_oc_cmd.mapstepclosecmd[d_curStep].checkindex)!=d_checkData_ptr->mValues.end())
            {

                if(d_checkData_ptr->mValues[d_step_oc_cmd.mapstepclosecmd[d_curStep].checkindex].fValue==d_step_oc_cmd.mapstepclosecmd[d_curStep].fvalue)
                    return true;
                else
                    return false;
            }
            else
                return false;
        }
    }

    bool Tsmt_message::is_step_all_exeok()
    {
        if(d_curStep<0)
            return true;
        if(d_cur_task_!=MSG_TRANSMITTER_TURNOFF_OPR)
        {
            if(d_step_oc_cmd.mapstepopencmd.size()<=d_curStep)
                return true;
            else
                return false;
        }
        else
        {
            if(d_step_oc_cmd.mapstepclosecmd.size()<=d_curStep)
                return true;
            else
                return false;
        }
    }

    //检测发射机运行状态
    void Tsmt_message::detect_run_state(DevMonitorDataPtr curDataPtr)
    {
        //float  zeroValue = d_devProperty->zero_power_value;
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
        if(!d_ptransmmit)
            return false;
        return d_ptransmmit->IsStandardCommand();
	}
	
    void Tsmt_message::GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit)
	{
        if(d_ptransmmit!=NULL)
        {
            d_ptransmmit->GetSignalCommand(lpParam,cmdUnit);
        }
	}

    void Tsmt_message::GetSignalCommand(int cmmType,int nIndex,CommandUnit &cmdUnit)
    {
        if(d_ptransmmit!=NULL)
        {
            d_ptransmmit->GetSignalCommand(cmmType,nIndex,cmdUnit);
        }
    }

    int   Tsmt_message::cur_dev_state()
    {
            return -1;
    }

    void Tsmt_message::GetAllCmd(CommandAttribute &cmdAll)
    {
        if(d_ptransmmit!=NULL)
        {
            d_ptransmmit->GetAllCmd(cmdAll);
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

                if((*iter).second.iItemType == 0){
                    data_ptr->mValues[(*iter).first].fValue *= (*iter).second.dRatio;
                    if(data_ptr->mValues[(*iter).first].sValue.empty())
                        data_ptr->mValues[(*iter).first].sValue = QString::number(data_ptr->mValues[(*iter).first].fValue,'g',2).toStdString();
                }
                else {
                    if((*iter).second.dRatio==0)
                        data_ptr->mValues[(*iter).first].fValue = data_ptr->mValues[(*iter).first].fValue==1.0f ? 0:1;
                }
            }
        }
        if(d_devInfo.nDevProtocol==HUIXIN)
        {
            if(d_devInfo.iDevType==0 || d_devInfo.iDevType==22)
            {
                float fdata1 = sqrt(data_ptr->mValues[0].fValue*1000)+sqrt(data_ptr->mValues[1].fValue);
                float fdata2 = sqrt(data_ptr->mValues[0].fValue*1000)-sqrt(data_ptr->mValues[1].fValue);
                DataInfo dtinfo;
                dtinfo.bType = false;
                if(fdata2>0.0001)
                    dtinfo.fValue = fdata1/fdata2;
                else
                    dtinfo.fValue = 0;
                data_ptr->mValues[2] = dtinfo;
            }
        }
    }

    void Tsmt_message::CreateObject()
    {
        switch(d_devInfo.nDevProtocol)
        {
        case BEIJ_GME:
            d_ptransmmit = new GmeTransmmit(d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case BEIJING_BEIGUANG:
            d_ptransmmit = new BgTransmmiter(d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case CHENGDU_KT_CG:
            d_ptransmmit = new CDtransmmiter(d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case SHANGUANG:
            d_ptransmmit = new SgTransmmit(d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case SHANGHAI_ALL_BAND:
            d_ptransmmit = new ShTransmmit(m_pSession,d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case HUIXIN:
            d_ptransmmit = new AhhxTransmmit(d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case HAGUANG:
            d_ptransmmit = new HgTransmmitr(d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case ANSHAN:
            d_ptransmmit = new AsTransmmit(boost::shared_ptr<hx_net::Tsmt_message>(this),d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case HANGCHUN:
            d_ptransmmit = new HcTransmmiter(d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case ITALY_TECHNO:
            break;
        case EKA:
            break;
        case GE_RS:
            d_ptransmmit = new RsTransmmit(boost::shared_ptr<hx_net::Tsmt_message>(this),d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case DLDZ:
            break;
        case CHENGDU_KANGTE_N:
            d_ptransmmit = new CKangteTransmmit(d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case LIAONING_HS:
            break;
        case SHANGHAI_MZ:
            break;
        case HARRIS:
            d_ptransmmit = new CHarrisTransmmit(boost::shared_ptr<hx_net::Tsmt_message>(this),d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case DE_XIN:
            d_ptransmmit = new DeXinTransmmit(d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case GLSQ:
            d_ptransmmit = new GlsqTransmmit(d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case GSBR:
            d_ptransmmit = new GsbrTransmmit(boost::shared_ptr<hx_net::Tsmt_message>(this),d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case ZHC:
            d_ptransmmit = new ZcTransmmit(d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;
        case RVR:
            d_ptransmmit = new RvrTransmmit(boost::shared_ptr<hx_net::Tsmt_message>(this),d_devInfo.nSubProtocol,d_devInfo.iAddressCode);
            break;

        }
    }


    //执行联动命令
    void Tsmt_message::exec_action_task_now(map<int,vector<ActionParam> > &param,int actionType,
                                                string sUser,e_ErrorCode &eErrCode)
    {
        //此处要进行联动参数到控制参数的转换，从而实现控制带参功能
        map<int,string> curParam;//控制参数
        switch(actionType)
        {
        case ACTP_OPEN_DEVICE:
            exec_task_now(MSG_TRANSMITTER_TURNON_OPR,sUser,eErrCode,curParam);
            break;
        case ACTP_CLOSE_DEVICE:
            exec_task_now(MSG_TRANSMITTER_TURNOFF_OPR,sUser,eErrCode,curParam);
            break;
        }


    }

    void Tsmt_message::exec_task_now(int icmdType,string sUser,e_ErrorCode &eErrCode,map<int,string> &mapParam,
                                     bool bSnmp,Snmp *snmp,CTarget *target)
    {
        //记录当前下发任务信息
        d_cur_task_ = icmdType;
        d_cur_task_param_ = mapParam;
        d_cur_user_ = sUser;//记录当前用户，定时，自动，人工
        if(cmd_excute_is_ok()){
            eErrCode = EC_OK;
            m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);

            return;
        }
        //snmp参数设置专用
        d_bUse_snmp = bSnmp;
        d_cur_snmp = snmp;
        d_cur_target = target;

        //初始化错误码
        eErrCode = EC_UNKNOWN;


        //执行任务2：正在执行（首次发送指令）
        int nResult = 2;//2：正在执行（循环发送指令）

        excute_task_cmd(eErrCode,nResult);
        //天线防抖，不在位
        if(nResult == 7 || nResult==8){
            string  sResult = DEV_CMD_RESULT_DESC(nResult);
            GetInst(DataBaseOperation).AddExcuteCommandLog(d_devInfo.sDevNum,d_cur_task_,sResult,d_cur_user_);
            m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
            return;
        }

        std::time(&d_OprStartTime);//循环执行计时开始
        tm *pCurTime = localtime(&d_OprStartTime);
        m_pSession-> notify_client_execute_result(d_devInfo.sStationNum,d_devInfo.sDevNum,
                                                  d_devInfo.sDevName,d_devInfo.iDevType,sUser,
                                                  d_cur_task_,pCurTime,false,eErrCode);
        //目前只有发射机的开关机进行循环发送（）
        if(icmdType >=MSG_TRANSMITTER_TURNON_OPR &&  icmdType < MSG_TRANSMITTER_RISE_POWER_OPR){
            if(!d_devProperty->is_step_open_close)
            {
                start_task_timeout_timer();
            }
            else
            {
                start_step_task_timeout_timer();
            }
        }else{
            //恢复执行标志
            m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
        }
    }

    //启动任务定时器
    void Tsmt_message::start_task_timeout_timer()
    {
        task_timeout_timer_.expires_from_now(boost::posix_time::seconds(d_devProperty->cmd_timeout_interval));
        task_timeout_timer_.async_wait(boost::bind(&Tsmt_message::schedules_task_time_out,
                                                   this,boost::asio::placeholders::error));
    }

    void Tsmt_message::start_step_task_timeout_timer()
    {
        task_timeout_timer_.expires_from_now(boost::posix_time::seconds(d_devProperty->cmd_timeout_interval));
        task_timeout_timer_.async_wait(boost::bind(&Tsmt_message::schedules_step_task_time_out,
                                                   this,boost::asio::placeholders::error));
    }

    void Tsmt_message::excute_task_cmd(e_ErrorCode &eErrCode,int &nExcutResult)
    {
        if(!d_devProperty->is_step_open_close)
        {
            switch (d_cur_task_) {
            case MSG_TRANSMITTER_TURNON_OPR:{
                eErrCode = EC_OPR_ON_GOING;
                exec_trunon_task_(HIGH_POWER_ON,eErrCode,nExcutResult);
            }
                break;
            case MSG_TRANSMITTER_MIDDLE_POWER_TURNON_OPR:{
                eErrCode = EC_OPR_ON_GOING;
                exec_trunon_task_(MIDDLE_POWER_ON,eErrCode,nExcutResult);
            }
                break;
            case MSG_TRANSMITTER_LOW_POWER_TURNON_OPR:{
                eErrCode = EC_OPR_ON_GOING;
                exec_trunon_task_(LOW_POWER_ON,eErrCode,nExcutResult);
            }
                break;
            case MSG_TRANSMITTER_TURNOFF_OPR:{
                eErrCode = EC_OPR_ON_GOING;
                exec_trunoff_task_(eErrCode,nExcutResult);
            }
                break;
            case MSG_GENERAL_COMMAND_OPR:
            {
                CommandUnit cmdUnit;
                d_ptransmmit->GetSignalCommand(d_cur_task_param_,cmdUnit);
                if(m_pSession && cmdUnit.commandLen>0)
                    m_pSession->send_cmd_to_dev(cmdUnit,eErrCode);
            }
            default:{//执行其他任务

                exec_other_task_(eErrCode);
            }
                break;
            }
        }
        else
        {
            //if(d_curStep
            switch (d_cur_task_) {
            case MSG_TRANSMITTER_TURNON_OPR:
            case MSG_TRANSMITTER_MIDDLE_POWER_TURNON_OPR:
            case MSG_TRANSMITTER_LOW_POWER_TURNON_OPR:{
                if(d_curStep==-1)
                    d_curStep = 0;
                eErrCode = EC_OPR_ON_GOING;
                exec_step_trunon_task_(eErrCode,nExcutResult);
            }
                break;
            case MSG_TRANSMITTER_TURNOFF_OPR:{
                if(d_curStep==-1)
                    d_curStep = 0;
                eErrCode = EC_OPR_ON_GOING;
                exec_step_trunoff_task_(eErrCode,nExcutResult);
            }
                break;
            case MSG_GENERAL_COMMAND_OPR:
            {
                CommandUnit cmdUnit;
                d_ptransmmit->GetSignalCommand(d_cur_task_param_,cmdUnit);
                if(m_pSession && cmdUnit.commandLen>0)
                    m_pSession->send_cmd_to_dev(cmdUnit,eErrCode);
            }
                break;
            default:{//执行其他任务

                exec_other_task_(eErrCode);
            }
                break;
            }
        }


        string  sResult = DEV_CMD_RESULT_DESC(nExcutResult);//3：正在执行（循环发送指令）
        GetInst(DataBaseOperation).AddExcuteCommandLog(d_devInfo.sDevNum,d_cur_task_,sResult,d_cur_user_);

    }

    //适用于需要循环执行的控制指令（暂时只用于发射机开关状态）
    bool  Tsmt_message::cmd_excute_is_ok()
    {
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

         return bCmdExcComplet;
    }

    //任务超时回调
    void Tsmt_message::schedules_task_time_out(const boost::system::error_code& error)
    {
        if(error!= boost::asio::error::operation_aborted)
        {
            time_t tmCurTime;
            time(&tmCurTime);
            double ninterval = difftime(tmCurTime,d_OprStartTime);
             tm *pCurTime = localtime(&tmCurTime);


             if(cmd_excute_is_ok()){

                 //写日志
                 string  sResult = DEV_CMD_RESULT_DESC(0);//0：成功
                 GetInst(DataBaseOperation).AddExcuteCommandLog(d_devInfo.sDevNum,d_cur_task_,sResult,d_cur_user_);
                //通知到客户端...
                m_pSession->notify_client_execute_result(d_devInfo.sStationNum,d_devInfo.sDevNum,d_devInfo.sDevName,d_devInfo.iDevType,d_cur_user_,
                                          d_cur_task_,pCurTime,true,EC_OK);

                m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
                d_cur_user_.clear();
                d_cur_task_ = -1;
                return;

            }else{
                //超时
                if(ninterval>=d_devProperty->cmd_excut_timeout_duration) {
                    //写日志
                    string  sResult = DEV_CMD_RESULT_DESC(4);//4：失败（超时）
                    GetInst(DataBaseOperation).AddExcuteCommandLog(d_devInfo.sDevNum,d_cur_task_,sResult,d_cur_user_);
                    //通知到客户端...
                    m_pSession->notify_client_execute_result(d_devInfo.sStationNum,d_devInfo.sDevNum,
                                                             d_devInfo.sDevName,d_devInfo.iDevType,d_cur_user_,
                                                             d_cur_task_,pCurTime,true,EC_FAILED);
                    m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
                    d_cur_user_.clear();
                    d_cur_task_ = -1;
                    return;
                }else{
                    e_ErrorCode eErrCode = EC_OPR_ON_GOING;

                    int nResult = 3;//3：正在执行（循环发送指令）
                    excute_task_cmd(eErrCode,nResult);
                    if(eErrCode == EC_OK){
                        m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
                        d_cur_user_.clear();
                        d_cur_task_ = -1;
                        return;
                    }
                    start_task_timeout_timer();
                }
            }
        }
    }

    void Tsmt_message::schedules_step_task_time_out(const boost::system::error_code &error)
    {
        if(error!= boost::asio::error::operation_aborted)
        {
            time_t tmCurTime;
            time(&tmCurTime);
            double ninterval = difftime(tmCurTime,d_OprStartTime);
             tm *pCurTime = localtime(&tmCurTime);


             if(is_step_task_exeok()){
                 ++d_curStep;
                 if(is_step_all_exeok())
                 {
                     string  sResult = DEV_CMD_RESULT_DESC(0);//0：成功
                     GetInst(DataBaseOperation).AddExcuteCommandLog(d_devInfo.sDevNum,d_cur_task_,sResult,d_cur_user_);
                    //通知到客户端...
                    m_pSession->notify_client_execute_result(d_devInfo.sStationNum,d_devInfo.sDevNum,d_devInfo.sDevName,d_devInfo.iDevType,d_cur_user_,
                                              d_cur_task_,pCurTime,true,EC_OK);

                    m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
                    d_cur_user_.clear();
                    d_cur_task_ = -1;
                    d_curStep=-1;
                    return;
                 }
                 else
                 {
                     d_OprStartTime = tmCurTime;
                     e_ErrorCode eErrCode = EC_OPR_ON_GOING;
                     int nResult = 3;
                     excute_task_cmd(eErrCode,nResult);
                     start_step_task_timeout_timer();
                 }
             }
             else{
                //超时
                 //d_cur_task_
                 int ntime = getsteptimeout();
                 if(ntime<=0 || ninterval>=ntime) {
                    //写日志
                    string  sResult = DEV_CMD_RESULT_DESC(4);//4：失败（超时）
                    GetInst(DataBaseOperation).AddExcuteCommandLog(d_devInfo.sDevNum,d_cur_task_,sResult,d_cur_user_);
                    //通知到客户端...
                    m_pSession->notify_client_execute_result(d_devInfo.sStationNum,d_devInfo.sDevNum,
                                                             d_devInfo.sDevName,d_devInfo.iDevType,d_cur_user_,
                                                             d_cur_task_,pCurTime,true,EC_FAILED);
                    m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
                    d_cur_user_.clear();
                    d_cur_task_ = -1;
                    d_curStep=-1;
                    return;
                 }
                 else{
                    e_ErrorCode eErrCode = EC_OPR_ON_GOING;

                    int nResult = 3;//3：正在执行（循环发送指令）
                    excute_task_cmd(eErrCode,nResult);
                    if(eErrCode == EC_OK){
                        m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
                        d_cur_user_.clear();
                        d_cur_task_ = -1;
                        d_curStep=-1;
                        return;
                    }
                    start_step_task_timeout_timer();
                }
            }
        }
    }

    void Tsmt_message::exec_trunon_task_(int nType,e_ErrorCode &eErrCode,int &nExcutResult)
    {
        if(!d_ptransmmit)
            return;
        //如果发射机正在监测运行状态，不执行开机操作
        if(is_detecting()){
            nExcutResult = 5;
            return;
        }
        if(d_relate_tsmt_ptr_!=NULL && d_relate_antenna_ptr_!=NULL){
            //////-----2018-2-3 15:53-------------/////
            bool bIgnoreAntenna = false;
            if(d_cur_task_param_.size()>0){
                if(d_cur_task_param_[0]=="ignore_antenna")
                    bIgnoreAntenna=true;
            }
             //////-----2018-2-3 15:53---end----------/////

            //既不是自动开关机也不是定时开关机
            if( d_cur_user_!="auto"){// && d_cur_user_!="timer"

                if(d_onekeyopen_996==false && d_onekeyopen_soft==false){
                    //天线在位执行开机，否则返回
                    if(d_antenna_Agent_ == false){
                        dev_run_state nAntennaS = GetInst(SvcMgr).get_dev_run_state(d_relate_antenna_ptr_->sStationNum,
                                                                                    d_relate_antenna_ptr_->sDevNum);


                        if((nAntennaS == antenna_host && d_Host_==TRANSMITTER_HOST) ||
                                (nAntennaS == antenna_backup && d_Host_==TRANSMITTER_BACKUP))
                        {

                        }else{
                            eErrCode = EC_OK;
                            nExcutResult = 7;//天线不在位
                            return;
                        }
                    }
                }
                else if(d_onekeyopen_soft)
                {
                    if(d_antenna_Agent_ == false){

                        bool can_excute =  GetInst(SvcMgr).dev_can_excute_cmd(d_relate_antenna_ptr_->sStationNum,d_relate_antenna_ptr_->sDevNum);
                        if(can_excute == false){
                            eErrCode = EC_OK;
                            nExcutResult = 8;//天线防抖
                            return ;
                        }
                    }

                    //关联机器在使用则进行关主机动作
                    if(d_relate_tsmt_ptr_->bUsed==true) {
                        //待处理，由开机指令引起的关机参数，可以依据开机带入的参数来转化为关机参数，暂时不用
                        map<int,string>  tmParam;
                        if(EC_OK != GetInst(SvcMgr).start_exec_task(d_relate_tsmt_ptr_->sDevNum,
                                                                    d_cur_user_,MSG_TRANSMITTER_TURNOFF_OPR,tmParam))
                            return ;

                      //  devCommdMsgPtr commandmsg_(new DeviceCommandMsg);
                      //  commandmsg_->set_sdevid(d_relate_tsmt_ptr_->sDevNum);
                     //   GetInst(SvcMgr).excute_command(d_relate_tsmt_ptr_->sDevNum,MSG_TRANSMITTER_TURNOFF_OPR,d_cur_user_,commandmsg_);
                    }

                    //计算目标天线位置
                    e_MsgType nAntennaCmd = (d_Host_== 0)?MSG_ANTENNA_BTOH_OPR:MSG_ANTENNA_HTOB_OPR;

                    //验证并切换天线
                    if(d_antenna_Agent_ == false){//如果天线是代理，不进行倒天线动作
                        //待处理，由开机指令引起的倒天线参数，可以依据开机带入的参数来转化为倒天线参数，暂时不用
                        map<int,string>  tmParam;
                        int nResult = GetInst(SvcMgr).start_exec_task(d_relate_antenna_ptr_->sDevNum,
                                                                      d_cur_user_,nAntennaCmd,tmParam);
                        if(EC_OK != nResult){

                            return ;
                        }
                    }
                }

            }else{

                if(d_antenna_Agent_ == false){

                    bool can_excute =  GetInst(SvcMgr).dev_can_excute_cmd(d_relate_antenna_ptr_->sStationNum,d_relate_antenna_ptr_->sDevNum);
                    if(can_excute == false){
                        eErrCode = EC_OK;
                        nExcutResult = 8;//天线防抖
                        return ;
                    }
                }

                //关联机器在使用则进行关主机动作
                if(d_relate_tsmt_ptr_->bUsed==true) {
                    //待处理，由开机指令引起的关机参数，可以依据开机带入的参数来转化为关机参数，暂时不用
                    //map<int,string>  tmParam;
                    //if(EC_OK != GetInst(SvcMgr).start_exec_task(d_relate_tsmt_ptr_->sDevNum,
                    //                                            d_cur_user_,MSG_TRANSMITTER_TURNOFF_OPR,tmParam))
                    //    return ;

                    devCommdMsgPtr commandmsg_(new DeviceCommandMsg);
                    commandmsg_->set_sdevid(d_relate_tsmt_ptr_->sDevNum);
                    GetInst(SvcMgr).excute_command(d_relate_tsmt_ptr_->sDevNum,MSG_TRANSMITTER_TURNOFF_OPR,d_cur_user_,commandmsg_);
                }

                //计算目标天线位置
                e_MsgType nAntennaCmd = (d_Host_== 0)?MSG_ANTENNA_BTOH_OPR:MSG_ANTENNA_HTOB_OPR;

                //验证并切换天线
                if(d_antenna_Agent_ == false){//如果天线是代理，不进行倒天线动作
                    //待处理，由开机指令引起的倒天线参数，可以依据开机带入的参数来转化为倒天线参数，暂时不用
                    map<int,string>  tmParam;
                    int nResult = GetInst(SvcMgr).start_exec_task(d_relate_antenna_ptr_->sDevNum,
                                                                  d_cur_user_,nAntennaCmd,tmParam);
                    if(EC_OK != nResult){

                        return ;
                    }
                }

            }

        }

        if(!d_bUse_snmp){
            if(m_pSession!=NULL)
                m_pSession->send_cmd_to_dev(d_devInfo.sDevNum,d_cur_task_,nType,eErrCode);
        }
        else
            d_ptransmmit->exec_cmd(d_cur_snmp,MSG_TRANSMITTER_TURNON_OPR,d_cur_target);

    }

    //关机
    void Tsmt_message::exec_trunoff_task_(e_ErrorCode &eErrCode,int &nExcutResult)
    {
        if(!d_ptransmmit)
            return;
        if(is_detecting()){
            nExcutResult = 5;
            return;
        }

        if(!d_bUse_snmp){
            if(m_pSession!=NULL)
                m_pSession->send_cmd_to_dev(d_devInfo.sDevNum,d_cur_task_,0,eErrCode);
        }
        else
            d_ptransmmit->exec_cmd(d_cur_snmp,MSG_TRANSMITTER_TURNOFF_OPR,d_cur_target);

        return;
    }

    //其他控制指令
    void Tsmt_message::exec_other_task_(e_ErrorCode &eErrCode)
    {
        if(!d_ptransmmit)
            return;

        eErrCode = EC_CMD_SEND_SUCCEED;
        if(!d_bUse_snmp){
            if(m_pSession!=NULL)
                m_pSession->send_cmd_to_dev(d_devInfo.sDevNum,d_cur_task_,0,eErrCode);
        }
        else
            d_ptransmmit->exec_cmd(d_cur_snmp,d_cur_task_,d_cur_target);

        return;
    }

    void Tsmt_message::exec_step_trunon_task_(e_ErrorCode &eErrCode, int &nExcutResult)
    {
        if(!d_ptransmmit)
            return;
        //如果发射机正在监测运行状态，不执行开机操作
     /*   if(is_detecting()){
            nExcutResult = 5;
            return;
        }*/
        if(d_relate_tsmt_ptr_!=NULL && d_relate_antenna_ptr_!=NULL){
            //////-----2018-2-3 15:53-------------/////
            bool bIgnoreAntenna = false;
            if(d_cur_task_param_.size()>0){
                if(d_cur_task_param_[0]=="ignore_antenna")
                    bIgnoreAntenna=true;
            }
             //////-----2018-2-3 15:53---end----------/////

            //既不是自动开关机也不是定时开关机
            if( d_cur_user_!="auto"){// && d_cur_user_!="timer"

                if(d_onekeyopen_996==false && d_onekeyopen_soft==false){
                    //天线在位执行开机，否则返回
                    if(d_antenna_Agent_ == false){
                        dev_run_state nAntennaS = GetInst(SvcMgr).get_dev_run_state(d_relate_antenna_ptr_->sStationNum,
                                                                                    d_relate_antenna_ptr_->sDevNum);


                        if((nAntennaS == antenna_host && d_Host_==TRANSMITTER_HOST) ||
                                (nAntennaS == antenna_backup && d_Host_==TRANSMITTER_BACKUP))
                        {

                        }else{
                            eErrCode = EC_OK;
                            nExcutResult = 7;//天线不在位
                            return;
                        }
                    }
                }
                else if(d_onekeyopen_soft)
                {
                    if(d_antenna_Agent_ == false){

                        bool can_excute =  GetInst(SvcMgr).dev_can_excute_cmd(d_relate_antenna_ptr_->sStationNum,d_relate_antenna_ptr_->sDevNum);
                        if(can_excute == false){
                            eErrCode = EC_OK;
                            nExcutResult = 8;//天线防抖
                            return ;
                        }
                    }

                    //关联机器在使用则进行关主机动作
                    if(d_relate_tsmt_ptr_->bUsed==true) {
                        //待处理，由开机指令引起的关机参数，可以依据开机带入的参数来转化为关机参数，暂时不用
                        map<int,string>  tmParam;
                        if(EC_OK != GetInst(SvcMgr).start_exec_task(d_relate_tsmt_ptr_->sDevNum,
                                                                    d_cur_user_,MSG_TRANSMITTER_TURNOFF_OPR,tmParam))
                            return ;
                    }

                    //计算目标天线位置
                    e_MsgType nAntennaCmd = (d_Host_== 0)?MSG_ANTENNA_BTOH_OPR:MSG_ANTENNA_HTOB_OPR;

                    //验证并切换天线
                    if(d_antenna_Agent_ == false){//如果天线是代理，不进行倒天线动作
                        //待处理，由开机指令引起的倒天线参数，可以依据开机带入的参数来转化为倒天线参数，暂时不用
                        map<int,string>  tmParam;
                        int nResult = GetInst(SvcMgr).start_exec_task(d_relate_antenna_ptr_->sDevNum,
                                                                      d_cur_user_,nAntennaCmd,tmParam);
                        if(EC_OK != nResult){

                            return ;
                        }
                    }
                }

            }else{

                if(d_antenna_Agent_ == false){

                    bool can_excute =  GetInst(SvcMgr).dev_can_excute_cmd(d_relate_antenna_ptr_->sStationNum,d_relate_antenna_ptr_->sDevNum);
                    if(can_excute == false){
                        eErrCode = EC_OK;
                        nExcutResult = 8;//天线防抖
                        return ;
                    }
                }

                //关联机器在使用则进行关主机动作
                if(d_relate_tsmt_ptr_->bUsed==true) {
                    devCommdMsgPtr commandmsg_(new DeviceCommandMsg);
                    commandmsg_->set_sdevid(d_relate_tsmt_ptr_->sDevNum);
                    GetInst(SvcMgr).excute_command(d_relate_tsmt_ptr_->sDevNum,MSG_TRANSMITTER_TURNOFF_OPR,d_cur_user_,commandmsg_);
                }

                //计算目标天线位置
                e_MsgType nAntennaCmd = (d_Host_== 0)?MSG_ANTENNA_BTOH_OPR:MSG_ANTENNA_HTOB_OPR;

                //验证并切换天线
                if(d_antenna_Agent_ == false){//如果天线是代理，不进行倒天线动作
                    //待处理，由开机指令引起的倒天线参数，可以依据开机带入的参数来转化为倒天线参数，暂时不用
                    map<int,string>  tmParam;
                    int nResult = GetInst(SvcMgr).start_exec_task(d_relate_antenna_ptr_->sDevNum,
                                                                  d_cur_user_,nAntennaCmd,tmParam);
                    if(EC_OK != nResult){

                        return ;
                    }
                }

            }

        }
        eErrCode = EC_CMD_SEND_SUCCEED;
        if(m_pSession!=NULL)
            m_pSession->start_write(d_step_oc_cmd.mapstepopencmd[d_curStep].commandId,d_step_oc_cmd.mapstepopencmd[d_curStep].commandLen);
    }

    void Tsmt_message::exec_step_trunoff_task_(e_ErrorCode &eErrCode, int &nExcutResult)
    {
        if(!d_ptransmmit)
            return;
        eErrCode = EC_CMD_SEND_SUCCEED;
        if(m_pSession!=NULL)
            m_pSession->start_write(d_step_oc_cmd.mapstepclosecmd[d_curStep].commandId,d_step_oc_cmd.mapstepclosecmd[d_curStep].commandLen);

    }


    //是否需要清除告警
    bool  Tsmt_message::is_need_clear_alarm()
    {
        if(d_relate_tsmt_ptr_ == NULL || d_relate_antenna_ptr_== NULL)
            return false;
        //如果天线不在位，则清除告警GetInst(LocalConfig).local_station_id()
        int nAtennaS = GetInst(SvcMgr).get_dev_run_state(d_relate_antenna_ptr_->sStationNum,
                                          d_relate_antenna_ptr_->sDevNum);

        if(nAtennaS == antenna_backup && d_Host_==0)
            return true;
        if(nAtennaS == antenna_host && d_Host_==1)
            return true;
        return false;
    }

     void Tsmt_message::exec_general_task(int icmdType, string sUser, devCommdMsgPtr lpParam, e_ErrorCode &eErrCode)
     {
         if(!d_ptransmmit)
             return;
         switch (icmdType) {
         case MSG_TRANSMITTER_TURNOFF_OPR:
         case MSG_DEV_TURNOFF_OPR:{

             if(!d_bUse_snmp){
                 if(m_pSession!=NULL)
                     m_pSession->send_cmd_to_dev(d_devInfo.sDevNum,icmdType,0,eErrCode);
             }
             else
                 d_ptransmmit->exec_cmd(d_cur_snmp,MSG_TRANSMITTER_TURNOFF_OPR,d_cur_target);
            }

             m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
             break;
         case MSG_GENERAL_COMMAND_OPR:
         {
             CommandUnit cmdUnit;
             d_ptransmmit->GetSignalCommand(lpParam,cmdUnit);
             if(m_pSession && cmdUnit.commandLen>0)
                 m_pSession->send_cmd_to_dev(cmdUnit,eErrCode);
         }
             break;
         }
     }
}
