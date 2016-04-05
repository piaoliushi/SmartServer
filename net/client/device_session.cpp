#include "device_session.h"
#include <boost/thread/detail/singleton.hpp>
#include "../../ErrorCode.h"
#include "../SvcMgr.h"
#include "../../LocalConfig.h"
#include "../../StationConfig.h"
#include "../../database/DataBaseOperation.h"
#include "../../protocol/bohui_const_define.h"
//#include "../sms/SmsTraffic.h"
using namespace db;
namespace hx_net
{
device_session::device_session(boost::asio::io_service& io_service,
                               TaskQueue<msgPointer>& taskwork,ModleInfo & modinfo,http_request_session_ptr &httpPtr)
    :net_session(io_service)
    ,taskwork_(taskwork)
    #ifdef USE_CLIENT_STRAND
    , strand_(io_service)
    #endif
    ,resolver_(io_service)
    ,uresolver_(io_service)
    ,query_timer_(io_service)
    ,connect_timer_(io_service)
    ,timeout_timer_(io_service)
    ,schedules_task_timer_(io_service)
    ,receive_msg_ptr_(new othdev_message(2048))
    ,othdev_con_state_(con_disconnected)
    ,modleInfos(modinfo)
    ,cur_msg_q_id_(0)
    ,query_timeout_count_(0)
    ,stop_flag_(false)
    ,task_count_(0)
    ,http_ptr_(httpPtr)
{
    //获得网络协议转换器相关配置
    /*moxa_config_ptr = GetInst(LocalConfig).moxa_property_ex(modleInfos.sModleNumber);
        map<string,DeviceInfo>::iterator iter = modleInfos.mapDevInfo.begin();
        for(;iter!=modleInfos.mapDevInfo.end();++iter)
        {
            map<int,double> itemRadio;
            map<int,DeviceMonitorItem>::iterator iterItem = iter->second.map_MonitorItem.begin();
            for(;iterItem!=iter->second.map_MonitorItem.end();++iterItem)
            {
                itemRadio.insert(pair<int,double>(iterItem->first,iterItem->second.dRatio));
            }

            //------------------ new code -----------------------------------------//
            //HMsgHandlePtr pars_agent = HMsgHandlePtr(new MsgHandleAgent(this,io_service));
            //boost::shared_ptr<CommandAttribute> tmpCommand(new CommandAttribute);
            //pars_agent->Init((*iter).second.nDevProtocol,(*iter).second.nSubProtocol,(*iter).second.iDevAddress,itemRadio);


            pTransmitterPropertyExPtr dev_config_ptr = GetInst(LocalConfig).transmitter_property_ex((*iter).first);

            //保存moxa下的设备自定义配置信息
            run_config_ptr[(*iter).first]=dev_config_ptr;

            //dev_agent_and_com[iter->first]=pair<CommandAttrPtr,HMsgHandlePtr>(tmpCommand,pars_agent);

            //报警项初始化
            map<int,std::pair<int,unsigned int> > devAlarmItem;
            mapItemAlarmRecord.insert(std::make_pair(iter->first,devAlarmItem));
            //定时数据保存时间初始化
            tmLastSaveTime.insert(std::make_pair(iter->first,time(0)));
        }
      cur_dev_id_ = dev_agent_and_com.begin()->first;
*/

}

device_session::~device_session()
{

}


void device_session::dev_base_info(DevBaseInfo& devInfo,string iId)
{

    if(modleInfos.mapDevInfo.find(iId)!=modleInfos.mapDevInfo.end())
    {
        // devInfo.mapMonitorItem = modleInfos.mapDevInfo[iId].map_MonitorItem;
        // devInfo.nDevType = modleInfos.mapDevInfo[iId].iDevType;
        //devInfo.sDevName = modleInfos.mapDevInfo[iId].sDevName;
        //devInfo.sDevNum = modleInfos.mapDevInfo[iId].sDevNum;
        //devInfo.sDevNum = modleInfos.mapDevInfo[iId].sStationNumber;
        //devInfo.nCommType = modleInfos.mapDevInfo[iId].nCommType;//主被动连接
        //devInfo.nConnectType =modleInfos.mapDevInfo[iId].nConnectType;//连接方式0：tcp
    }
}

//是否包含该id
bool device_session::is_contain_dev(string sDevId)
{
    if(modleInfos.mapDevInfo.find(sDevId) == modleInfos.mapDevInfo.end())
        return false;
    return true;
}

string device_session::next_dev_id()
{
    map<string,pair<CommandAttrPtr,HMsgHandlePtr> >::iterator iter = dev_agent_and_com.find(cur_dev_id_);
    ++iter;
    if(iter==dev_agent_and_com.end())
        iter = dev_agent_and_com.begin();
    return iter->first;
}

//获得连接状态
con_state device_session::get_con_state()
{
    boost::recursive_mutex::scoped_lock llock(con_state_mutex_);
    return othdev_con_state_;
}
//获得运行状态
dev_run_state device_session::get_run_state()
{

    if(get_con_state()==con_connected)
        return dev_running;
    else
        return dev_unknown;
}

void device_session::set_con_state(con_state curState)
{
    /*boost::recursive_mutex::scoped_lock llock(con_state_mutex_);
        if(othdev_con_state_!=curState)
        {
            othdev_con_state_ = curState;
            //GetInst(SvcMgr).get_notify()->OnConnected(this->modleInfos.sModleNumber,othdev_con_state_);
            map<string,DeviceInfo>::iterator iter = modleInfos.mapDevInfo.begin();
            for(;iter!=modleInfos.mapDevInfo.end();++iter)
            {
                //广播设备状态到在线客户端
                send_net_state_message("1000",(*iter).first
                                        ,(*iter).second.sDevName,(e_DevType)((*iter).second.iDevType)
                                        ,othdev_con_state_);
                GetInst(SvcMgr).get_notify()->OnConnected((*iter).first,othdev_con_state_);
            }

        }*/
}

//获得发射机报警状态
void device_session::get_alarm_state(string sDevId,map<int,std::pair<int,tm> >& cellAlarm)
{
    boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);
    //if(mapItemAlarmStartTime.find(sDevId)!=mapItemAlarmStartTime.end())
    //	cellAlarm = mapItemAlarmStartTime[sDevId];
}


void device_session::connect(std::string hostname,unsigned short port,bool bReconnect)
{
    if(is_connected())
        return;

    set_stop(false);

    boost::system::error_code ec;
    tcp::resolver::query query(hostname, boost::lexical_cast<std::string, unsigned short>(port));
    tcp::resolver::iterator iter = resolver_.resolve(query, ec);
    if(iter != tcp::resolver::iterator())
    {
        //正在连接
        set_con_state(con_connecting);
        endpoint_ = (*iter).endpoint();
        socket().async_connect(endpoint_,boost::bind(&device_session::handle_connected,
                                                     this,boost::asio::placeholders::error));
        start_timeout_timer();//启动超时重连定时器
    }
    else
    {
        //出错。。。
    }
}

//udp连接
void device_session::udp_connect(std::string hostname,unsigned short port)
{
    set_tcp(false);
    boost::system::error_code ec;
    udp::resolver::query query(hostname,boost::lexical_cast<std::string, unsigned short>(port));
    udp::resolver::iterator iter = uresolver_.resolve(query,ec);
    if(iter!=udp::resolver::iterator())
    {
        uendpoint_ = (*iter).endpoint();
        usocket().open(udp::v4());
        const udp::endpoint local_endpoint = udp::endpoint(udp::v4(),port);
        //local_endpoint.
        usocket().bind(local_endpoint);
        //set_con_state(con_connected);
        boost::system::error_code err= boost::system::error_code();
        handle_connected(err);
    }
}

//启动超时定时器
void device_session::start_timeout_timer(unsigned long nSeconds)
{
    timeout_timer_.expires_from_now(boost::posix_time::seconds(nSeconds));
    timeout_timer_.async_wait(boost::bind(&device_session::connect_timeout,
                                          this,boost::asio::placeholders::error));
}

//连接超时回调
void device_session::connect_timeout(const boost::system::error_code& error)
{
    if(is_stop())
        return;
    //只有当前状态是正在连接才执行超时。。。
    if(!is_connecting())
        return;
    //超时了
    if(error!= boost::asio::error::operation_aborted)
        start_connect_timer();//启动重连尝试
}

void device_session::set_stop(bool bStop)
{
    boost::mutex::scoped_lock lock(stop_mutex_);
    stop_flag_=bStop;
}

bool device_session::is_stop()
{
    boost::mutex::scoped_lock lock(stop_mutex_);
    return stop_flag_;
}
//是否已经建立连接
bool device_session::is_connected(string sDevId)
{
    return (get_con_state()==con_connected)?true:false;
}
//是否正在连接
bool  device_session::is_connecting()
{
    return (get_con_state()==con_connecting)?true:false;
}
//是否已经断开连接
bool device_session::is_disconnected(string sDevId)
{
    return (get_con_state()==con_disconnected)?true:false;
}


void device_session::start_connect_timer(unsigned long nSeconds)
{
    set_con_state(con_connecting);//设置正在重连标志
    connect_timer_.expires_from_now(boost::posix_time::seconds(nSeconds));
    connect_timer_.async_wait(boost::bind(&device_session::connect_time_event,
                                          this,boost::asio::placeholders::error));
}

//连接超时
void device_session::connect_time_event(const boost::system::error_code& error)
{
    //只有当前状态是正在连接才执行超时。。。
    if(!is_connecting())
        return;
    if(error!= boost::asio::error::operation_aborted)
    {
        if(is_tcp())
        {
            socket().async_connect(endpoint_,boost::bind(&device_session::handle_connected,
                                                         this,boost::asio::placeholders::error));
        }else{
            usocket().open(udp::v4());
            //const udp::endpoint local_endpoint = udp::endpoint(udp::v4(),modleInfos.netMode.ilocal_port);
            //usocket().bind(local_endpoint);
            boost::system::error_code err= boost::system::error_code();
            handle_connected(err);
        }


        //std::cout<<"reconnect request..."<<std::endl;
        start_timeout_timer();//启动超时定时器
    }
}


void device_session::disconnect()
{
    set_stop();
    close_all();
    clear_all_alarm();//当停止服务时，清除所有报警
}

void device_session::start_read(int msgLen)
{
    if(msgLen > receive_msg_ptr_->space())
    {
        cout<<"data overflow !!!!"<<endl;
        return;
    }
    boost::asio::async_read(socket(),boost::asio::buffer(receive_msg_ptr_->w_ptr(),msgLen),
                        #ifdef USE_STRAND
                            strand_.wrap(
                            #endif
                                boost::bind(&device_session::handle_read,this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred)
                            #ifdef USE_STRAND
                                )
                        #endif
                            );
}

void device_session::start_query_timer(unsigned long nSeconds/* =3 */)
{
    query_timer_.expires_from_now(boost::posix_time::millisec(nSeconds));
    query_timer_.async_wait(
            #ifdef USE_STRAND
                strand_.wrap(
                #endif
                    boost::bind(&device_session::query_send_time_event,this,boost::asio::placeholders::error)
                #ifdef USE_STRAND
                    )
            #endif
                );
}

void  device_session::query_send_time_event(const boost::system::error_code& error)
{
    if(!is_connected())
        return;
    if(error!= boost::asio::error::operation_aborted)
    {
        if(query_timeout_count_<moxa_config_ptr->query_timeout_count)
        {
            ++query_timeout_count_;
            if(dev_agent_and_com[cur_dev_id_].first->queryComm[cur_msg_q_id_].commandLen>0){
                start_write(dev_agent_and_com[cur_dev_id_].first->queryComm[cur_msg_q_id_].commandId,
                            dev_agent_and_com[cur_dev_id_].first->queryComm[cur_msg_q_id_].commandLen
                            );
            }
        }
        else{
            query_timeout_count_ = 0;
            cur_msg_q_id_ = 0;
            //发送清零数据给客户端,该设备可能连接异常
            //DevMonitorDataPtr curData_ptr(new Data);
            //send_monitor_data_message(modleInfos.sStationNumber,cur_dev_id_,(e_DevType)modleInfos.mapDevInfo[cur_dev_id_].nDevType
            //	,curData_ptr,modleInfos.mapDevInfo[cur_dev_id_].mapMonitorItem);

            cur_dev_id_ = next_dev_id();
            if(dev_agent_and_com[cur_dev_id_].first->queryComm[cur_msg_q_id_].commandLen>0){
                start_write(dev_agent_and_com[cur_dev_id_].first->queryComm[cur_msg_q_id_].commandId,
                            dev_agent_and_com[cur_dev_id_].first->queryComm[cur_msg_q_id_].commandLen
                            );
            }
        }
        start_query_timer(moxa_config_ptr->query_interval);
    }
}

//发送消息
bool device_session::sendRawMessage(unsigned char * data_,int nDatalen)
{
    char dataPrint[100];
    memset(dataPrint,0,100);
    memcpy(dataPrint,data_,nDatalen);
    start_write(data_,nDatalen);
    return true;
}

void device_session::start_write(unsigned char* commStr,int commLen)
{
    if(is_tcp())
    {
        boost::asio::async_write(
                    socket(),
                    boost::asio::buffer(commStr,commLen),
            #ifdef USE_STRAND
                    strand_.wrap(
                #endif
                        boost::bind(&net_session::handle_write,shared_from_this(),
                                    boost::asio::placeholders::error,
                                    boost::asio::placeholders::bytes_transferred)
                #ifdef USE_STRAND
                        )
            #endif
                    );
    }


    else
    {
        usocket().async_send_to(boost::asio::buffer(commStr,commLen),uendpoint_,
                        #ifdef USE_STRAND
                                strand_.wrap(
                            #endif
                                    boost::bind(&net_session::handle_write,shared_from_this(),
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred)
                            #ifdef USE_STRAND
                                    )
                        #endif
                                );
    }
}

//等待任务结束
void device_session::wait_task_end()
{
    boost::mutex::scoped_lock lock(task_mutex_);
    while(task_count_>0)
    {
        task_end_conditon_.wait(task_mutex_);
    }
}

//提交任务
void device_session::task_count_increase()
{
    boost::mutex::scoped_lock lock(task_mutex_);
    ++task_count_;
}
//任务递减
void device_session::task_count_decrease()
{
    boost::mutex::scoped_lock lock(task_mutex_);
    --task_count_;
    task_end_conditon_.notify_all();
}

int device_session::task_count()
{
    boost::mutex::scoped_lock lock(task_mutex_);
    return task_count_;
}
void device_session::close_all()
{
    set_con_state(con_disconnected);
    connect_timer_.cancel();
    timeout_timer_.cancel();
    close_i();   //关闭socket
    //等待任务结束，任务里面，必须放在运行状态检测之前
    //状态检测会
    wait_task_end();
    cur_msg_q_id_         =0;//当前命令id
    query_timeout_count_=0;//命令发送超时次数清零

    clear_all_alarm();

}

//启动任务定时器
void device_session::start_task_schedules_timer()
{
    schedules_task_timer_.expires_from_now(boost::posix_time::seconds(1));
    schedules_task_timer_.async_wait(boost::bind(&device_session::schedules_task_time_out,
                                                 this,boost::asio::placeholders::error));
}

//定时任务回调
void device_session::schedules_task_time_out(const boost::system::error_code& error)
{
    if(!is_connected())
        return;
    if(error!= boost::asio::error::operation_aborted)
    {
        time_t curTime = time(0);
        tm *pCurTime = localtime(&curTime);

        unsigned long cur_tm = pCurTime->tm_hour*3600+pCurTime->tm_min*60+pCurTime->tm_sec;
        map<string,DeviceInfo>::iterator witer = modleInfos.mapDevInfo.begin();
        for(;witer!=modleInfos.mapDevInfo.end();++witer)
        {
            vector<Command_Scheduler>::iterator cmd_iter = witer->second.vCommSch.begin();
            for(;cmd_iter!=witer->second.vCommSch.end();++cmd_iter)
            {
                //按天控制
                if((*cmd_iter).iDateType == RUN_TIME_DAY){
                    //在５秒内
                    if(curTime>=(*cmd_iter).tExecuteTime && curTime<(*cmd_iter).tExecuteTime+5){
                        e_ErrorCode eResult = EC_OBJECT_NULL;
                        bool bRslt =  start_exec_task(witer->first,"timer",eResult,(*cmd_iter).iCommandType);
                        //通知客户端正在执行
                        if(bRslt==true)
                            notify_client(witer->first,witer->second.sDevName,"timer",(*cmd_iter).iCommandType,pCurTime,eResult);
                    }
                }
                //按星期控制
                if((*cmd_iter).iDateType == RUN_TIME_WEEK){
                    if(curTime> (*cmd_iter).tCmdEndTime &&  (*cmd_iter).tCmdEndTime>0) continue;//超过运行图终止时间且终止时间不为0,则跳过
                    if((pCurTime->tm_wday+1)== (*cmd_iter).iWeek){
                        tm *pSetTimeS = localtime(&((*cmd_iter).tExecuteTime));
                        unsigned long set_tm_s = pSetTimeS->tm_hour*3600+pSetTimeS->tm_min*60+pSetTimeS->tm_sec;
                        if(cur_tm>=set_tm_s && cur_tm<(set_tm_s+5)){
                            e_ErrorCode eResult = EC_OBJECT_NULL;
                            bool bRslt = start_exec_task(witer->first,"timer",eResult,(*cmd_iter).iCommandType);
                            //通知客户端正在执行
                            if(bRslt==true)
                                notify_client(witer->first,witer->second.sDevName,"timer",(*cmd_iter).iCommandType,pCurTime,eResult);
                        }
                    }
                }

                //按月控制
                if((*cmd_iter).iDateType == RUN_TIME_WEEK){
                    if(curTime> (*cmd_iter).tCmdEndTime &&  (*cmd_iter).tCmdEndTime>0) continue;//超过运行图终止时间且终止时间不为0,则跳过
                    if((pCurTime->tm_mon+1)== (*cmd_iter).iMonitorMonth ||  (*cmd_iter).iMonitorMonth==0){
                        tm *pSetTimeS = localtime(&((*cmd_iter).tExecuteTime));
                        unsigned long set_tm_s = pSetTimeS->tm_hour*3600+pSetTimeS->tm_min*60+pSetTimeS->tm_sec;
                        if(cur_tm>=set_tm_s && cur_tm<(set_tm_s+5)){
                            e_ErrorCode eResult = EC_OBJECT_NULL;
                            bool bRslt = start_exec_task(witer->first,"timer",eResult,(*cmd_iter).iCommandType);
                            //通知客户端正在执行
                            if(bRslt==true)
                                notify_client(witer->first,witer->second.sDevName,"timer",(*cmd_iter).iCommandType,pCurTime,eResult);
                        }
                    }
                }

            }
        }
        start_task_schedules_timer();
    }
}

void device_session::notify_client(string sDevId,string devName,string user,int cmdType, tm *pCurTime,int eResult)//witer->second.sDevNum
{
    static  char str_time[64];
    strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", pCurTime);

    int CommandType= (cmdType==DEV_TASK_TURNON)?MSG_TRANSMITTER_TURNON_ACK:MSG_TRANSMITTER_TURNOFF_ACK;
    send_command_execute_result_message(GetInst(LocalConfig).local_station_id(),sDevId,
                                        TRANSMITTER,devName,"timer",(e_MsgType)CommandType,(e_ErrorCode)eResult);
    //通知http服务器
    CommandType= (cmdType==DEV_TASK_TURNON)?CMD_AUTO_TURNON_SEND:CMD_AUTO_TURNOFF_SEND;
    string sDesc = devName+DEV_CMD_RESULT_DESC[CommandType];
    http_ptr_->send_http_excute_result_messge_to_platform(sDevId,str_time,CommandType,sDesc);
}

void device_session::set_opr_state(string sdevId,dev_opr_state curState)
{
    boost::mutex::scoped_lock lock(opr_state_mutex_);
    dev_opr_state_[sdevId] = curState;
}

dev_opr_state  device_session::get_opr_state(string sdevId){
    boost::mutex::scoped_lock lock(opr_state_mutex_);
    return dev_opr_state_[sdevId];
}

//开始执行任务
bool device_session::start_exec_task(string sDevId,string sUser,e_ErrorCode &opResult,int cmdType)
{
    if(!is_connected()){
        opResult = EC_NET_ERROR;
        return false;
    }
    if(get_opr_state(sDevId)==dev_no_opr)
        set_opr_state(sDevId,dev_opr_turn_on);//设置正在执行任务标志
    else{
        opResult = EC_OPR_ON_GOING;//正在执行控制命令
        return false;//已经有任务正在执行
    }

    cur_opr_user_[sDevId] = sUser;//记录当前操作用户
    cur_task_type_[sDevId] = cmdType;//记录当前任务类型
    //现在执行任务
    dev_agent_and_com[sDevId].second->exec_task_now(cmdType,opResult);
    opResult = EC_OPR_ON_GOING;//正在执行控制命令

    return true;
}

//判断是否保存当前记录
void device_session::save_monitor_record(string sDevId,DevMonitorDataPtr curDataPtr)
{
    time_t tmCurTime;
    time(&tmCurTime);
    double ninterval = difftime(tmCurTime,tmLastSaveTime[sDevId]);
    if(ninterval<run_config_ptr[sDevId]->data_save_interval)//间隔保存时间 need amend;
        return ;
    if(GetInst(DataBaseOperation).AddItemMonitorRecord(sDevId,tmCurTime,curDataPtr))
        tmLastSaveTime[sDevId] = tmCurTime;

}

//判断当前时间是否需要保存监控数据
bool device_session::is_need_save_data(string sDevId)
{
    time_t tmCurTime;
    time(&tmCurTime);
    double ninterval = difftime(tmCurTime,tmLastSaveTime[sDevId]);
    if(ninterval<run_config_ptr[sDevId]->data_save_interval)//间隔保存时间 need amend;
        return false;
    tmLastSaveTime[sDevId] = tmCurTime;
    return true;
}


//是否在监测时间段--------2016-4-1 ----完成
bool device_session::is_monitor_time(string sDevId)
{
    time_t curTime = time(0);
    tm *pCurTime = localtime(&curTime);
    unsigned long cur_tm = pCurTime->tm_hour*3600+pCurTime->tm_min*60+pCurTime->tm_sec;
    //运行图----天
    map<int,vector<Monitoring_Scheduler> >::iterator day_iter = modleInfos.mapDevInfo[sDevId].vMonitorSch.find(RUN_TIME_DAY);
    if(day_iter!=modleInfos.mapDevInfo[sDevId].vMonitorSch.end()){
        vector<Monitoring_Scheduler>::iterator iter = day_iter->second.begin();
        for(;iter!=day_iter->second.end();++iter){
            if(curTime>=(*iter).tStartTime && curTime<(*iter).tEndTime){
                return (*iter).bMonitorFlag;
            }
        }
    }
    //运行图----星期
    map<int,vector<Monitoring_Scheduler> >::iterator week_iter = modleInfos.mapDevInfo[sDevId].vMonitorSch.find(RUN_TIME_WEEK);
    if(week_iter!=modleInfos.mapDevInfo[sDevId].vMonitorSch.end()){
        vector<Monitoring_Scheduler>::iterator iter = week_iter->second.begin();
        for(;iter!=week_iter->second.end();++iter){
            if(curTime> (*iter).tAlarmEndTime &&  (*iter).tAlarmEndTime>0) continue;//超过运行图终止时间且终止时间不为0,则跳过
            if((pCurTime->tm_wday+1)== (*iter).iMonitorWeek){
                tm *pSetTimeS = localtime(&((*iter).tStartTime));
                tm *pSetTimeE = localtime(&((*iter).tEndTime));
                unsigned long set_tm_s = pSetTimeS->tm_hour*3600+pSetTimeS->tm_min*60+pSetTimeS->tm_sec;
                unsigned long set_tm_e = pSetTimeE->tm_hour*3600+pSetTimeE->tm_min*60+pSetTimeE->tm_sec;
                if(cur_tm>=set_tm_s && cur_tm<set_tm_e){
                    return (*iter).bMonitorFlag;
                }
            }
        }
    }
    //运行图----月
    map<int,vector<Monitoring_Scheduler> >::iterator month_iter = modleInfos.mapDevInfo[sDevId].vMonitorSch.find(RUN_TIME_MONTH);
    if(month_iter!=modleInfos.mapDevInfo[sDevId].vMonitorSch.end()){
        vector<Monitoring_Scheduler>::iterator iter = month_iter->second.begin();
        for(;iter!=month_iter->second.end();++iter){
            if(curTime> (*iter).tAlarmEndTime &&  (*iter).tAlarmEndTime>0) continue;//超过运行图终止时间且终止时间不为0,则跳过
            if((pCurTime->tm_mon+1)== (*iter).iMonitorMonth ||  (*iter).iMonitorMonth==0){ //如果是当前月或者是all则比较
                if((pCurTime->tm_mday)== (*iter).iMonitorDay){
                    tm *pSetTimeS = localtime(&((*iter).tStartTime));
                    tm *pSetTimeE = localtime(&((*iter).tEndTime));
                    unsigned long set_tm_s = pSetTimeS->tm_hour*3600+pSetTimeS->tm_min*60+pSetTimeS->tm_sec;
                    unsigned long set_tm_e = pSetTimeE->tm_hour*3600+pSetTimeE->tm_min*60+pSetTimeE->tm_sec;
                    if(cur_tm>=set_tm_s && cur_tm<set_tm_e){
                        return (*iter).bMonitorFlag;
                    }
                }
            }
        }
    }

    return true;
}

//2016-3-31------处理设备数据----完成
void device_session::handler_data(string sDevId,DevMonitorDataPtr curDataPtr)
{
    boost::recursive_mutex::scoped_lock lock(data_deal_mutex);
    //是否在运行图时间
    bool bIsMonitorTime = is_monitor_time(sDevId);
    //打包发送客户端
    send_monitor_data_message(GetInst(LocalConfig).local_station_id(),sDevId,modleInfos.mapDevInfo[sDevId].iDevType
                              ,curDataPtr,modleInfos.mapDevInfo[sDevId].map_MonitorItem);
    //打包发送http消息到上级平台
    http_ptr_->send_http_data_messge_to_platform(sDevId,modleInfos.mapDevInfo[sDevId].iDevType,
                                                 curDataPtr,modleInfos.mapDevInfo[sDevId].map_MonitorItem);
    //检测当前报警状态
    check_alarm_state(sDevId,curDataPtr,bIsMonitorTime);
    //如果在监测时间段则保存当前记录
    if(bIsMonitorTime)
        save_monitor_record(sDevId,curDataPtr);
    //任务数递减
    task_count_decrease();
    return;
}

void device_session::clear_dev_alarm(string sDevId)
{
    boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);

    map<int,map<int,CurItemAlarmInfo> >::iterator iter = mapItemAlarm[sDevId].begin();
    for(;iter!=mapItemAlarm[sDevId].end();++iter) {
        map<int,CurItemAlarmInfo>::iterator typeIter = iter->second.begin();
        for(;iter!=mapItemAlarm[sDevId].end();++iter) {
            //写入恢复记录,通知客户端
            //.......

        }
        iter->second.clear();
    }
    mapItemAlarm[sDevId].clear();
}

void device_session::handle_connected(const boost::system::error_code& error)
{
    if(is_stop())
        return;
    timeout_timer_.cancel();//关闭重连超时定时器

    if(!error)
    {
        set_con_state(con_connected);
        //开始启动接收第一条查询指令的回复数据头
        receive_msg_ptr_->reset();
        if(dev_agent_and_com[cur_dev_id_].second->IsStandardCommand()==true)
            start_read_head(dev_agent_and_com[cur_dev_id_].first->queryComm[cur_msg_q_id_].ackLen);
        else
            start_read(dev_agent_and_com[cur_dev_id_].first->queryComm[cur_msg_q_id_].ackLen);

        if(is_tcp())	{
            if(dev_agent_and_com[cur_dev_id_].second->is_auto_run()==false)
                start_query_timer(moxa_config_ptr->query_interval);
            else
                dev_agent_and_com[cur_dev_id_].second->start();
        }
        //初始化最后保存时间
        time_t curTm = time(0);
        map<string,time_t>::iterator iter = tmLastSaveTime.begin();
        for(;iter!=tmLastSaveTime.end();++iter)
            (*iter).second = curTm;
        //启动任务定时器
        start_task_schedules_timer();
        return;
    }
    else	{
        close_i();
        start_connect_timer();
    }
}

void device_session::handle_read_head(const boost::system::error_code& error, size_t bytes_transferred)
{
    if(!is_connected())
        return;
    if (!error)// || error.value() == ERROR_MORE_DATA
    {
        int nResult = receive_msg_ptr_->check_normal_msg_header(dev_agent_and_com[cur_dev_id_].second,bytes_transferred,CMD_QUERY,cur_msg_q_id_);
        if(nResult>0)//检查消息头
            start_read_body(nResult);
        else{
            close_all();
            start_connect_timer(moxa_config_ptr->connect_timer_interval);
        }
    }
    else{
        close_all();
        start_connect_timer(moxa_config_ptr->connect_timer_interval);
    }
}

void device_session::start_read_head(int msgLen)
{
    if(msgLen>receive_msg_ptr_->space())
    {
        cout<<"data overflow !!!!"<<endl;
        return;
    }

    if(is_tcp())
    {
        boost::asio::async_read(socket(), boost::asio::buffer(receive_msg_ptr_->w_ptr(),
                                                              msgLen),
                        #ifdef USE_STRAND
                                strand_.wrap(
                            #endif
                                    boost::bind(&device_session::handle_read_head,this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred)
                            #ifdef USE_STRAND
                                    )
                        #endif
                                );
    }
    else
    {
        udp::endpoint sender_endpoint;
        usocket().async_receive_from(boost::asio::buffer(receive_msg_ptr_->w_ptr(),receive_msg_ptr_->space()),sender_endpoint,//msgLen
                             #ifdef USE_STRAND
                                     strand_.wrap(
                                 #endif
                                         boost::bind(&device_session::handle_udp_read,this,
                                                     boost::asio::placeholders::error,
                                                     boost::asio::placeholders::bytes_transferred)
                                 #ifdef USE_STRAND
                                         )
                             #endif
                                     );
    }
}

void device_session::handle_udp_read(const boost::system::error_code& error,size_t bytes_transferred)
{
    if(!is_connected())
        return;
    if (!error || error == boost::asio::error::message_size)
    {
        int nResult = receive_msg_ptr_->check_normal_msg_header(dev_agent_and_com[cur_dev_id_].second,bytes_transferred,CMD_QUERY,cur_msg_q_id_);

        if(nResult == 0)
        {
            DevMonitorDataPtr curData_ptr(new Data);
            int nResult = receive_msg_ptr_->decode_msg_body(dev_agent_and_com[cur_dev_id_].second,curData_ptr,bytes_transferred);
            if(nResult==0)//查询数据解析正确
            {
                query_timeout_count_ = 0;
                if(boost::detail::thread::singleton<boost::threadpool::pool>::instance()
                        .schedule(boost::bind(&device_session::handler_data,this,cur_dev_id_,curData_ptr)))
                {
                    task_count_increase();
                }
            }
        }
        start_read_head(bytes_transferred);


        cout<< "接收长度---"<<bytes_transferred << std::endl;

    }
    else{
        cout<< error.message() << std::endl;
        close_all();
        start_connect_timer(moxa_config_ptr->connect_timer_interval);
    }
}



void device_session::start_read_body(int msgLen)
{

    if(is_tcp())
    {
        boost::asio::async_read(socket(), boost::asio::buffer(receive_msg_ptr_->w_ptr(),
                                                              msgLen),
                        #ifdef USE_STRAND
                                strand_.wrap(
                            #endif
                                    boost::bind(&device_session::handle_read_body,this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred)
                            #ifdef USE_STRAND
                                    )
                        #endif
                                );
    }
    else
    {
        udp::endpoint sender_endpoint;
        usocket().async_receive_from(boost::asio::buffer(receive_msg_ptr_->w_ptr(),
                                                         msgLen),
                                     sender_endpoint,
                             #ifdef USE_STRAND
                                     strand_.wrap(
                                 #endif
                                         boost::bind(&device_session::handle_read_body,this,
                                                     boost::asio::placeholders::error,
                                                     boost::asio::placeholders::bytes_transferred)
                                 #ifdef USE_STRAND
                                         )
                             #endif
                                     );
    }

}


void device_session::handle_read_body(const boost::system::error_code& error, size_t bytes_transferred)
{
    if(!is_connected())
        return;
    if (!error)
    {

        //pTsmtAgentMsgPtr curData_ptr(new TsmtAgentMsg);
        //int nResult = receive_msg_ptr_->decode_msg_body(dev_agent_,curData_ptr,bytes_transferred);
        DevMonitorDataPtr curData_ptr(new Data);
        int nResult = receive_msg_ptr_->decode_msg_body(dev_agent_and_com[cur_dev_id_].second,curData_ptr,bytes_transferred);
        if(nResult==0)//查询数据解析正确
        {
            query_timeout_count_ = 0;
            if(boost::detail::thread::singleton<boost::threadpool::pool>::instance()
                    .schedule(boost::bind(&device_session::handler_data,this,cur_dev_id_,curData_ptr)))
            {
                task_count_increase();
            }

            if(cur_msg_q_id_<dev_agent_and_com[cur_dev_id_].first->queryComm.size()-1)
                ++cur_msg_q_id_;
            else
                cur_msg_q_id_=0;
            start_read_head(dev_agent_and_com[cur_dev_id_].first->queryComm[cur_msg_q_id_].ackLen);
        }
        else if(nResult>0)//跳过数据(针对数据管理器等设备的非查询指令回复)
            start_read_head(dev_agent_and_com[cur_dev_id_].first->queryComm[cur_msg_q_id_].ackLen);
        else{
            close_all();
            start_connect_timer(moxa_config_ptr->connect_timer_interval);
        }
    }
    else{
        close_all();
        start_connect_timer(moxa_config_ptr->connect_timer_interval);
    }
}


void device_session::handle_read(const boost::system::error_code& error, size_t bytes_transferred)
{
    /*if(!is_connected())
            return;
        if(!error)
        {
            //amend by lk 2013-11-26
            int nResult = receive_msg_ptr_->check_msg_header(dev_agent_and_com[cur_dev_id_].second->DevAgent(),
                                                             bytes_transferred,CMD_QUERY,cur_msg_q_id_);
            if(nResult == 0)
            {
                //cout<<"nResult:"<<nResult<<"transferred:"<<bytes_transferred<<endl;
                query_timeout_count_ = 0;
                if(cur_msg_q_id_<dev_agent_and_com[cur_dev_id_].first->queryComm.size()-1)
                {
                    ++cur_msg_q_id_;
                    boost::this_thread::sleep(boost::posix_time::milliseconds(200));
                    start_write(dev_agent_and_com[cur_dev_id_].first->queryComm[cur_msg_q_id_].commandId,
                                dev_agent_and_com[cur_dev_id_].first->queryComm[cur_msg_q_id_].commandLen);
                }
                else
                {
                    cur_msg_q_id_=0;
                    DevMonitorDataPtr curData_ptr(new Data);
                    if(receive_msg_ptr_->decode_msg(dev_agent_and_com[cur_dev_id_].second->DevAgent(),curData_ptr))
                    {

                        if(boost::detail::thread::singleton<boost::threadpool::pool>::instance()
                            .schedule(boost::bind(&device_session::handler_data,this,cur_dev_id_,curData_ptr)))
                        {
                            task_count_increase();
                        }
                        cur_dev_id_ = next_dev_id();//切换到下一个设备
                    }
                    else{
                        close_all();
                        start_connect_timer();
                        return;
                    }
                }
                start_read(dev_agent_and_com[cur_dev_id_].first->queryComm[cur_msg_q_id_].ackLen);
            }
            else if(nResult>0){
                cout<<"nResult:"<<nResult<<"transferred:"<<bytes_transferred<<endl;
                start_read(nResult);
            }
            else if(nResult == -1){
                close_all();
                start_connect_timer();
                return;
            }
        }
        else{
            close_all();
            start_connect_timer();
            return;
        }*/
}
void device_session::handle_write(const boost::system::error_code& error,size_t bytes_transferred)
{
    if(!is_connected())
        return;
    if(error){
        close_all();
        start_connect_timer();
    }
}
//记录发送告警---2016-4-3--完成
void  device_session::record_alarm_and_notify(string &devId,float fValue,const float &fLimitValue,bool bMod,
                                              DeviceMonitorItem &ItemInfo,CurItemAlarmInfo &curAlarm)
{
    static  char str_time[64];
    tm *local_time = localtime(&curAlarm.startTime);
    strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", local_time);

    string sAlarm_reason = str(boost::format("%1%%2,当前值:%3%%4%,门限值:%5%%6%")
                               %ItemInfo.sItemName%CONST_STR_ALARM_CONTENT[curAlarm.nLimitId]%fValue%ItemInfo.sUnit%fLimitValue%ItemInfo.sUnit);
    //存储告警到数据库
    if(!bMod){//0:告警产生
        bool bRslt = GetInst(DataBaseOperation).AddItemAlarmRecord(devId,curAlarm.startTime,ItemInfo.iItemIndex,curAlarm.nLimitId,curAlarm.nType,
                                                                   fValue,sAlarm_reason,curAlarm.nAlarmId);
        if(bRslt==true){
            //提交监控量告警到上级平台
            http_ptr_->send_http_alarm_messge_to_platform(devId,modleInfos.mapDevInfo[devId].iDevType,bMod,curAlarm,sAlarm_reason);
            //发送监控量报警到客户端
            send_alarm_state_message(GetInst(LocalConfig).local_station_id(),devId,modleInfos.mapDevInfo[devId].sDevName,ItemInfo.iItemIndex
                                     ,sAlarm_reason//暂用reason替代item名称
                                     ,modleInfos.mapDevInfo[devId].iDevType,curAlarm.nType,str_time,mapItemAlarm[devId][ItemInfo.iItemIndex].size());
            // 联动.....
        }
    }else{//1:告警恢复
        time_t curTime = time(0);
        bool bRslt = GetInst(DataBaseOperation).AddItemEndAlarmRecord(curTime,curAlarm.nAlarmId);
        if(bRslt==true){
            //提交监控量告警到上级平台
            http_ptr_->send_http_alarm_messge_to_platform(devId,modleInfos.mapDevInfo[devId].iDevType,bMod,curAlarm,sAlarm_reason);
            //发送监控量报警到客户端
            send_alarm_state_message(GetInst(LocalConfig).local_station_id(),devId,modleInfos.mapDevInfo[devId].sDevName,ItemInfo.iItemIndex
                                     ,sAlarm_reason//暂用reason替代item名称
                                     ,modleInfos.mapDevInfo[devId].iDevType,RESUME,str_time,mapItemAlarm[devId][ItemInfo.iItemIndex].size());
            // 联动.....
        }
    }
}
//分析告警---2016-4-1--完成
void  device_session::parse_item_alarm(string devId,float fValue,DeviceMonitorItem &ItemInfo)
{
    boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);
    //遍历监控量告警配置
    for(int nIndex=0;nIndex<ItemInfo.vItemAlarm.size();++nIndex)
    {
        if(ItemInfo.vItemAlarm[nIndex].bIsAlarm==false)
            continue;
        CurItemAlarmInfo  tmp_alarm_info;
        int iLimittype = ItemInfo.vItemAlarm[nIndex].iLimittype;
        bool  bIsAlarm=false;
        if(ItemInfo.iItemType == ITEM_ANALOG){//模拟量
            //上限,上上限告警
            if(iLimittype == ALARM_UPPER || iLimittype == ALARM_UP_UPPER) {
                if(fValue > ItemInfo.vItemAlarm[nIndex].fLimitvalue)
                    bIsAlarm=true;
            }else{//下限,下下限告警
                if(fValue > ItemInfo.vItemAlarm[nIndex].fLimitvalue)
                    bIsAlarm=true;
            }
        }else{//状态量
            if(fValue == ItemInfo.vItemAlarm[nIndex].fLimitvalue)
                bIsAlarm=true;
        }

        if(bIsAlarm == true){
            //判断该监控项是否在告警
            map<int,map<int,CurItemAlarmInfo> >::iterator findIter = mapItemAlarm[devId].find(ItemInfo.iItemIndex);
            if(findIter != mapItemAlarm[devId].end()){
                //判断该告警类型是否在告警
                map<int,CurItemAlarmInfo>::iterator alarm_type_findIter = mapItemAlarm[devId][ItemInfo.iItemIndex].find(iLimittype);
                if(alarm_type_findIter != mapItemAlarm[devId][ItemInfo.iItemIndex].end()){
                    //计算持续时间,判断进行告警
                    time_t curTime = time(0);
                    double  dtime_during = difftime( curTime, alarm_type_findIter->second.startTime );
                    if(dtime_during>=ItemInfo.vItemAlarm[nIndex].iDelaytime){
                        //存储告警,通知告警,联动告警
                        record_alarm_and_notify(devId,fValue, ItemInfo.vItemAlarm[nIndex].fLimitvalue,0,ItemInfo,alarm_type_findIter->second);
                    }
                }else{ //没有找到,增加该告警类型
                    //tmp_alarm_info.nAlarmId = 1000;//需修改
                    tmp_alarm_info.startTime = time(0);//记录时间
                    tmp_alarm_info.nType = ItemInfo.vItemAlarm[nIndex].iAlarmid;//告警类型
                    tmp_alarm_info.nLimitId = iLimittype;
                    mapItemAlarm[devId][ItemInfo.iItemIndex][iLimittype] = tmp_alarm_info;
                }
            }else {
                //没有找到增加该告警监控项
                //tmp_alarm_info.nAlarmId = 1000;//需修改
                tmp_alarm_info.startTime = time(0);//记录时间
                tmp_alarm_info.nType = ItemInfo.vItemAlarm[nIndex].iAlarmid;//告警类型
                tmp_alarm_info.nLimitId = iLimittype;
                map<int,CurItemAlarmInfo>  tmTypeAlarm;
                tmTypeAlarm[iLimittype] = tmp_alarm_info;
                mapItemAlarm[devId][ItemInfo.iItemIndex] = tmTypeAlarm;
            }
        }else {
            //当前没有告警,判断是否存在此告警,确定是否需要恢复,移除该告警
            map<int,map<int,CurItemAlarmInfo> >::iterator findIter = mapItemAlarm[devId].find(ItemInfo.iItemIndex);
            //查找是否有该监控项
            if(findIter != mapItemAlarm[devId].end()){
                map<int,CurItemAlarmInfo>::iterator findTypeIter =  findIter->second.find(iLimittype);
                if(findTypeIter != findIter->second.end()){
                    time_t curTime = time(0);
                    double  dtime_during = difftime( curTime, findTypeIter->second.startTime );
                    if(dtime_during>=ItemInfo.vItemAlarm[nIndex].iResumetime){

                        //存储告警恢复,通知客户端,移除该告警
                        record_alarm_and_notify(devId,fValue, ItemInfo.vItemAlarm[nIndex].fLimitvalue,1,ItemInfo,findTypeIter->second);

                        findIter->second.erase(findTypeIter);//移除该告警类型告警
                        if( findIter->second.size()<=0)
                            mapItemAlarm[devId].erase(findIter);//如果监控项告警为空,则移除该监控量告警
                    }
                }
            }
        }
    }
}

//清除所有报警状态
void device_session::clear_all_alarm()
{
    boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);
    //mapItemAlarmStartTime.clear();//报警项开始时间清除
    //mapItemAlarmRecord.clear();   //报警记录清除
}

void device_session::check_alarm_state(string sDevId,DevMonitorDataPtr curDataPtr,bool bMonitor)
{
    //非检测时间段不进行报警检测,同时清除当前告警
    if(bMonitor!=true){
        clear_dev_alarm(sDevId);
        return;
    }
    map<string,DeviceInfo>::iterator iter = modleInfos.mapDevInfo.find(sDevId);
    if(iter== modleInfos.mapDevInfo.end())
        return;
    map<int,DeviceMonitorItem>::iterator iterItem = iter->second.map_MonitorItem.begin();
    for(;iterItem!=iter->second.map_MonitorItem.end();++iterItem){
        double dbValue =curDataPtr->mValues[iterItem->first].fValue;
        parse_item_alarm(sDevId,dbValue,iterItem->second);
    }
}

void device_session::sendSmsToUsers(int nLevel,string &sContent)
{
    vector<SendMSInfo>& smsInfo=GetInst(StationConfig).get_sms_user_info();
    for(int i=0;i<smsInfo.size();++i)
    {
        if(smsInfo[i].iAlarmLevel==nLevel)
        {
            string sCenterId = GetInst(LocalConfig).sms_center_number();
            //扩平台需要，暂时删去
            //GetInst(CSmsTraffic).SendSMSContent(sCenterId,smsInfo[i].sPhoneNumber,sContent);
        }
    }

}
//用来执行通用指令,根据具体参数,组织命令字符串,同时发送命令消息
bool device_session::excute_general_command(int cmdType,devCommdMsgPtr lpParam,e_ErrorCode &opResult)
{
    CommandUnit adjustTmCmd;
    dev_agent_and_com[lpParam->sdevid()].second->GetSignalCommand(lpParam,adjustTmCmd);
    if(adjustTmCmd.commandLen>0)
    {
        start_write(adjustTmCmd.commandId ,adjustTmCmd.commandLen);
        opResult=EC_CMD_SEND_SUCCEED;
        return true;
    }
    return false;
}

//执行通用指令
bool device_session::excute_command(int cmdType,devCommdMsgPtr lpParam,e_ErrorCode &opResult)
{
    if(get_con_state()!=con_connected)
        return false;
    switch(cmdType)
    {
    case MSG_0401_SWITCH_OPR:
    {
        if(lpParam->cparams_size()!=1)
            return false;
        int nChannel = atoi(lpParam->cparams(0).sparamvalue().c_str());
        if(dev_agent_and_com.find(lpParam->sdevid())!=dev_agent_and_com.end()){
            if(dev_agent_and_com[lpParam->sdevid()].first->switchComm.size()>nChannel){
                start_write(dev_agent_and_com[lpParam->sdevid()].first->switchComm[nChannel].commandId ,
                        dev_agent_and_com[lpParam->sdevid()].first->switchComm[nChannel].commandLen );
            }
        }
    }
        break;
    case MSG_CONTROL_MOD_SWITCH_OPR:
    {
        if(lpParam->cparams_size()!=1)
            return false;
        int nMode = 0;//atoi(lpParam->cparams(0).sparamvalue().c_str());
        if(dev_agent_and_com.find(lpParam->sdevid())!=dev_agent_and_com.end()){
            if(dev_agent_and_com[lpParam->sdevid()].first->switch2Comm.size()>nMode){
                start_write(dev_agent_and_com[lpParam->sdevid()].first->switch2Comm[nMode].commandId ,
                        dev_agent_and_com[lpParam->sdevid()].first->switch2Comm[nMode].commandLen );
            }
        }
    }
        break;
    case MSG_ADJUST_TIME_SET_OPR:
    {
        if(lpParam->cparams_size()!=0)
            return false;
        if(dev_agent_and_com.find(lpParam->sdevid())!=dev_agent_and_com.end()){

            CommandUnit adjustTmCmd;
            vector<string> params;
            ////amend by lk 2013-11-26
            //dev_agent_and_com[lpParam->sdevid()].second->DevAgent()->HxGetSignalCmd(CMD_ADJUST_TIME,params,adjustTmCmd);
            //if(adjustTmCmd.commandLen>0)
            //	start_write(adjustTmCmd.commandId ,adjustTmCmd.commandLen);
        }
    }
        break;
    case MSG_GENERAL_COMMAND_OPR:
    {
        excute_general_command(cmdType,lpParam,opResult);
    }
    }

    return true;
}
}
