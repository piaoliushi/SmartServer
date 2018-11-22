#include "antenna_message.h"
#include "./net/SvcMgr.h"
#include "../../../StationConfig.h"
#include "../../../LocalConfig.h"
#include "../../../database/DataBaseOperation.h"
using namespace db;
namespace hx_net {

unsigned char  Cmd_SwitchToMaster[] ={0xaa,0x22,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x22,0xbb};
unsigned char  Cmd_SwitchToSlave[]  ={0xaa,0x22,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x22,0xbb};

Antenna_message::Antenna_message(session_ptr pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo)
    :base_message()
    ,d_devInfo(devInfo)
    ,task_timeout_timer_(io_service)
    ,d_cur_task_(-1)
    ,dev_run_state_(dev_unknown)
    ,d_relate_host_tsmt_ptr_(NULL)
    ,d_relate_backup_tsmt_ptr_(NULL)
    ,can_switch_(true)
{

    m_pSession = boost::dynamic_pointer_cast<device_session>(pSession);
    if(IsStandardCommand())
        d_curData_ptr.reset(new Data);

    map<int,vector<AssDevChan> >::iterator find_iter = d_devInfo.map_AssDevChan.find(0);
    if(find_iter!=d_devInfo.map_AssDevChan.end()){
        vector<AssDevChan>::iterator dev_iter = find_iter->second.begin();
        for(;dev_iter!=find_iter->second.end();++dev_iter){
            DeviceInfo *curDev =  GetInst(StationConfig).get_devinfo_by_id((*dev_iter).sAstNum);
            if(curDev==NULL)
                continue;
            if(curDev->iDevType == DEVICE_TRANSMITTER && (*dev_iter).iAssType == ATTENA_TO_H_TSMT)
                d_relate_host_tsmt_ptr_ = curDev;
            if(curDev->iDevType == DEVICE_TRANSMITTER && (*dev_iter).iAssType == ATTENA_TO_B_TSMT)
                d_relate_backup_tsmt_ptr_ = curDev;
        }
    }
}

Antenna_message::~Antenna_message()
{
     task_timeout_timer_.cancel();
}

int Antenna_message::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
{
    switch(d_devInfo.nDevProtocol)
    {
    case ANTENNA_CONTROL:{
        switch (d_devInfo.nSubProtocol) {
        case HX_MD981:
            if(data[0]==0xAA && data[1]==0x11 && data[11]==0xCC)
                return RE_SUCCESS;
            else
                return RE_HEADERROR;
        default:
            return RE_NOPROTOCOL;
        }
    }
    default:
        return RE_NOPROTOCOL;
    }
}

int Antenna_message::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    if(data_ptr!=NULL)
        d_curData_ptr = data_ptr;
    switch(d_devInfo.nDevProtocol)
    {
    case ANTENNA_CONTROL:{
        switch (d_devInfo.nSubProtocol) {
        case HX_MD981:{
            return parse_HX_981(data,d_curData_ptr,nDataLen,iaddcode);
        }
        default:
            return RE_NOPROTOCOL;
        }
    }
    default:
        return RE_NOPROTOCOL;
    }
}

bool Antenna_message::IsStandardCommand()
{
    switch (d_devInfo.nDevProtocol) {
    case ANTENNA_CONTROL:{
        switch(d_devInfo.nSubProtocol){
        default:
            return false;
        }
    }
    default:
        return false;
    }
}

void Antenna_message::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
{
    switch (d_devInfo.nDevProtocol) {
    case ANTENNA_CONTROL:
    {
        switch(d_devInfo.nSubProtocol){
        case HX_MD981:
        {
            if(lpParam->cparams_size()<1)
                break;
            //默认命令备机->主机
            cmdUnit.ackLen=0;
            cmdUnit.commandLen=11;
            cmdUnit.commandId[0]=0xAA;
            cmdUnit.commandId[1]=0x22;
            cmdUnit.commandId[2]=0x01;
            cmdUnit.commandId[3]=0x01;
            cmdUnit.commandId[4]=0x00;
            cmdUnit.commandId[5]=0x00;
            cmdUnit.commandId[6]=0x00;
            cmdUnit.commandId[7]=0x00;
            cmdUnit.commandId[8]=0x00;
            cmdUnit.commandId[9]=0x22;
            cmdUnit.commandId[10]=0xBB;
            if(atoi(lpParam->cparams(0).sparamvalue().c_str())==1)
            {
                cmdUnit.commandId[3]=0x00;
                cmdUnit.commandId[4]=0x01;
            }
        }
            break;
        default:
            break;
        }
    }
    default:
       break;
    }
}

void Antenna_message::GetAllCmd(CommandAttribute &cmdAll)
{
    switch (d_devInfo.nDevProtocol) {
    case ANTENNA_CONTROL:
    {
        switch(d_devInfo.nSubProtocol){
        case HX_MD981:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 12;
            tmUnit.commandLen = 11;
            tmUnit.commandId[0] = 0xAA;
            tmUnit.commandId[1] = 0x11;
            tmUnit.commandId[2] = 0x00;
            tmUnit.commandId[3] = 0x00;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x00;
            tmUnit.commandId[6] = 0x00;
            tmUnit.commandId[7] = 0x00;
            tmUnit.commandId[8] = 0x00;
            tmUnit.commandId[9] = 0x11;
            tmUnit.commandId[10] = 0xBB;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[1]=0x22;
            tmUnit.commandId[2]=0x01;
            tmUnit.commandId[3]=0x01;
            tmUnit.commandId[4]=0x00;
            tmUnit.commandId[5]=0x00;
            tmUnit.commandId[6]=0x00;
            tmUnit.commandId[7]=0x00;
            tmUnit.commandId[8]=0x00;
            tmUnit.commandId[9]=0x22;
            cmdAll.mapCommand[MSG_ANTENNA_BTOH_OPR].push_back(tmUnit);
            tmUnit.commandId[3]=0x00;
            tmUnit.commandId[4]=0x01;
            cmdAll.mapCommand[MSG_ANTENNA_HTOB_OPR].push_back(tmUnit);

        }
            break;
        default:
            break;
        }
    }
    default:
       break;
    }
}

int Antenna_message::get_run_state()
{
    boost::recursive_mutex::scoped_lock llock(run_state_mutex_);
    return dev_run_state_;
}

void Antenna_message::restart_task_timeout_timer()
{

    task_timeout_timer_.cancel();
    task_timeout_timer_.expires_from_now(boost::posix_time::seconds(180));
    task_timeout_timer_.async_wait(boost::bind(&Antenna_message::schedules_task_time_out,
        this,boost::asio::placeholders::error));
}

void Antenna_message::schedules_task_time_out(const boost::system::error_code& error)
{
     if(error!= boost::asio::error::operation_aborted){
        boost::recursive_mutex::scoped_lock lock(can_switch_mutex_);
        can_switch_ = true;
    }
}

bool Antenna_message::can_switch_antenna()
{
    boost::recursive_mutex::scoped_lock lock(can_switch_mutex_);
    return can_switch_;
}

void Antenna_message::set_run_state(int curState)
{
    boost::recursive_mutex::scoped_lock llock(run_state_mutex_);
    if(dev_run_state_ != curState)
    {
         {
             boost::recursive_mutex::scoped_lock lock(can_switch_mutex_);
             can_switch_ = false;
         }

        restart_task_timeout_timer();

        dev_run_state_=curState;
        GetInst(SvcMgr).get_notify()->OnDevStatus(d_devInfo.sDevNum,dev_run_state_+1);
        if(m_pSession!=NULL)//GetInst(LocalConfig).local_station_id()
            m_pSession->send_work_state_message(d_devInfo.sStationNum,d_devInfo.sDevNum
                                            ,d_devInfo.sDevName,DEVICE_ANTENNA,(dev_run_state)dev_run_state_);
    }
}

bool Antenna_message::is_running()
{
    return (get_run_state()==antenna_host)?true:false;
}


bool Antenna_message::is_shut_down()
{
    return (get_run_state()==antenna_backup)?true:false;
}


bool Antenna_message::is_detecting()
{
    return (get_run_state()==dev_detecting)?true:false;
}

 void Antenna_message::reset_run_state()
 {
     boost::recursive_mutex::scoped_lock llock(run_state_mutex_);
     dev_run_state_=dev_unknown;
 }

 bool Antenna_message::dev_can_excute_cmd()
 {
     return can_switch_antenna();
 }

int Antenna_message::parse_HX_981(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    iaddcode = d_devInfo.iAddressCode;
    DataInfo dainfo;
    dainfo.bType = true;
    if(data[3]==0x00 && data[4]==0x01){
        dainfo.fValue = 0;
        set_run_state(antenna_backup);
    }
    else if(data[3]==0x01 && data[4]==0x00){
        dainfo.fValue = 1;
        set_run_state(antenna_host);
    }else if(data[3]==0x00 && data[4]==0x00){
         dainfo.fValue = -1;
         set_run_state(dev_detecting);
    }else{
        dainfo.fValue = -1;
        set_run_state(dev_unknown);
    }

    data_ptr->mValues[0] = dainfo;
    return 0;
}


bool  Antenna_message::cmd_excute_is_ok()
{
     bool  bCmdExcComplet = false;
     switch (d_cur_task_) {
     case MSG_ANTENNA_HTOB_OPR:{
         if(is_shut_down())
             bCmdExcComplet = true;
     } break;
     case MSG_ANTENNA_BTOH_OPR:{
         if(is_running())
             bCmdExcComplet = true;
         } break;
     }

     return bCmdExcComplet;
}

void Antenna_message::switch_antenna_pos(e_ErrorCode &eErrCode,int &nExcutResult)
{

    if(is_detecting()){
        nExcutResult = 5;
        return;
    }

    if(!can_switch_antenna()){
        eErrCode = EC_UNKNOWN;
        return;
    }

    //天线在位执行开机，否则返回
    dev_run_state nHostRunS = GetInst(SvcMgr).get_dev_run_state(d_relate_host_tsmt_ptr_->sStationNum,
                               d_relate_host_tsmt_ptr_->sDevNum);
    dev_run_state nBackupRunS = GetInst(SvcMgr).get_dev_run_state(d_relate_backup_tsmt_ptr_->sStationNum,
                               d_relate_backup_tsmt_ptr_->sDevNum);

    if((nHostRunS == dev_running && get_run_state()==antenna_host)
            || (nBackupRunS == dev_running && get_run_state()==antenna_backup))
    {
        eErrCode = EC_FAILED;
        nExcutResult = 9;
        return;
    }


    if(m_pSession!=NULL)
        m_pSession->send_cmd_to_dev(d_devInfo.sDevNum,d_cur_task_,0,eErrCode);

    return;
}

void Antenna_message::excute_task_cmd(e_ErrorCode &eErrCode,int &nExcutResult)
{
    switch (d_cur_task_) {
    case MSG_ANTENNA_HTOB_OPR:
    case MSG_ANTENNA_BTOH_OPR:{
        eErrCode = EC_OPR_ON_GOING;
        switch_antenna_pos(eErrCode,nExcutResult);
    }
        break;

    }

    string  sResult = DEV_CMD_RESULT_DESC(nExcutResult);
    GetInst(DataBaseOperation).AddExcuteCommandLog(d_devInfo.sDevNum,d_cur_task_,sResult,d_cur_user_);

}


void Antenna_message::exec_task_now(int icmdType,string sUser,e_ErrorCode &eErrCode,map<int,string> &mapParam,
                           bool bSnmp,Snmp *snmp,CTarget *target)
{
    eErrCode = EC_UNKNOWN;
     if(m_pSession ==NULL)
         return;
    d_cur_task_ = icmdType;
    d_cur_user_ = sUser;
    if(cmd_excute_is_ok()){
        eErrCode = EC_OK;
        m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
        return;
    }

    int nResult = 2;
    excute_task_cmd(eErrCode,nResult);

    time_t   d_OprStartTime;
    std::time(&d_OprStartTime);
    tm *pCurTime = localtime(&d_OprStartTime);
    m_pSession-> notify_client_execute_result(d_devInfo.sStationNum,d_devInfo.sDevNum,d_devInfo.sDevName,d_devInfo.iDevType,sUser,
                              d_cur_task_,pCurTime,false,eErrCode);

    m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
}


}
