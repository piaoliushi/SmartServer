#include "device_session.h"
#include <boost/thread/detail/singleton.hpp>
#include <boost/asio/serial_port.hpp>
#include "../../ErrorCode.h"
#include "../SvcMgr.h"
#include "../../LocalConfig.h"
#include "../../StationConfig.h"
#include "../../database/DataBaseOperation.h"
#include "../../protocol/bohui_const_define.h"
#include "./snmp_pp/snmp_pp.h"
#include "../server/http/RequestHandlerFactory.h"
#include "../../utility.h"
#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif
using namespace db;
namespace hx_net
{
device_session::device_session(boost::asio::io_service& io_service,
                               ModleInfo & modinfo,http_request_session_ptr &httpPtr)
    :net_session(io_service)
    #ifdef USE_CLIENT_STRAND
    , strand_(io_service)
    #endif
    ,resolver_(io_service)
    ,uresolver_(io_service)
    ,connect_timer_(io_service)
    ,query_timer_(io_service)
    ,timeout_timer_(io_service)
    ,schedules_task_timer_(io_service)
    ,receive_msg_ptr_(new othdev_message(2048))
    ,othdev_con_state_(con_disconnected)
    ,modleInfos_(modinfo)
    ,cur_msg_q_id_(0)
    ,query_timeout_count_(0)
    //,task_count_(0)
    ,http_ptr_(httpPtr)
    ,io_service_(io_service)
    ,snmp_ptr_(NULL)
    ,target_ptr_(NULL)
    ,all_dev_is_use_(false)
    ,http_content_length_(0)
    ,http_content_("")
    ,short_connect_(false)
    //,http_command_id_("")
    ,http_stream_(io_service)

{

}

device_session::~device_session()
{
    if(snmp_ptr_!=NULL){
        snmp_ptr_->stop_poll_thread();
        delete snmp_ptr_,snmp_ptr_=NULL;
    }
    if(target_ptr_!=NULL)
        delete target_ptr_,target_ptr_=NULL;
}

//初始化设备配置
void device_session::init_session_config()
{
    //获得网络协议转换器相关配置
    moxa_config_ptr = GetInst(LocalConfig).moxa_property_ex(modleInfos_.sModleNumber);
    map<string,DeviceInfo>::iterator iter = modleInfos_.mapDevInfo.begin();
    bool bIsRunTaskTimer = false;
    for(;iter!=modleInfos_.mapDevInfo.end();++iter)
    {
        if(iter->second.bUsed)
            all_dev_is_use_ = true;
        dev_opr_state_[iter->first] = dev_no_opr;
        HMsgHandlePtr pars_agent = HMsgHandlePtr(new MsgHandleAgent(shared_from_this(),io_service_,iter->second));
        boost::shared_ptr<CommandAttribute> tmpCommand(new CommandAttribute);

        pDevicePropertyExPtr dev_config_ptr = GetInst(LocalConfig).device_property_ex((*iter).first);
        //初始化解析库
        pars_agent->Init(dev_config_ptr);
        //保存moxa下的设备自定义配置信息
        run_config_ptr[(*iter).first]=dev_config_ptr;
        //获取默认命令配置
        pars_agent->GetAllCmd(*tmpCommand);

        //将配置文件中的自定义命令更新到command_;
        GetInst(LocalConfig).device_cmd((*iter).first,*tmpCommand);

        dev_agent_and_com[iter->first]=pair<CommandAttrPtr,HMsgHandlePtr>(tmpCommand,pars_agent);

        //判断是否时http代理设置
        if(modleInfos_.iCommunicationMode==CON_MOD_NET && modleInfos_.netMode.inet_type == NET_MOD_HTTP){
            if(iter->second.map_DevProperty.find("Agent")!=iter->second.map_DevProperty.end())
                request_handler_factory::get_mutable_instance().register_data_callback(modleInfos_.netMode.strIp,pars_agent);
        }

        //报警项初始化
        map<int,map<int,CurItemAlarmInfo> > devAlarmItem;
        mapItemAlarm.insert(std::make_pair(iter->first,devAlarmItem));
        //定时数据保存时间初始化
        tmLastSaveTime.insert(std::make_pair(iter->first,time(0)));

        //定时数据发送时间
        tmLastSendHttpTime.insert(std::make_pair(iter->first,time(0)));
        //目前只有发射机类型设备才启用定时任务(定时开关机)
        if(iter->second.iDevType == DEVICE_TRANSMITTER || iter->second.iDevType == DEVICE_ELEC)
            bIsRunTaskTimer=true;
        //创建和分析关联设备信息
        parse_ass_dev_ptr  ass_dev_ptr(new Parse_Ass_Device(iter->second));
        map_dev_ass_parse_ptr_[iter->first] = ass_dev_ptr;
        map_addcode_devid_[iter->second.iAddressCode]=iter->first;
    }
    cur_dev_id_ = dev_agent_and_com.begin()->first;
    netAlarm.nAlarmId = -1;//默认值
    //启动发射机任务定时器
    if(bIsRunTaskTimer==true)
        start_task_schedules_timer();
}

void device_session::dev_base_info(DevBaseInfo& devInfo,string iId)
{
    map<string,DeviceInfo>::iterator find_iter = modleInfos_.mapDevInfo.find(iId);
    if(find_iter !=modleInfos_.mapDevInfo.end()){
        devInfo.sDevNum = find_iter->second.sDevNum;
        devInfo.mapMonitorItem = find_iter->second.map_MonitorItem;
        devInfo.nDevType = find_iter->second.iDevType;
        devInfo.sDevName = find_iter->second.sDevName;
    }
}

//是否包含该id
bool device_session::is_contain_dev(string sDevId)
{
    if(modleInfos_.mapDevInfo.find(sDevId) == modleInfos_.mapDevInfo.end())
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

string device_session::get_devid_by_addcode(int iaddcode)
{
    map<int,string>::iterator iter = map_addcode_devid_.find(iaddcode);
    if(iter!=map_addcode_devid_.end())
        return iter->second;
    return "-1";
}

//获得连接状态
con_state device_session::get_con_state()
{
    boost::recursive_mutex::scoped_lock llock(con_state_mutex_);
    //    if(get_run_state(dev_agent_and_com.begin()->first) == dev_running)
    //        return con_connected;
    return othdev_con_state_;
}
//获得运行状态
dev_run_state device_session::get_run_state(string sDevId)
{
    if(dev_agent_and_com.find(sDevId)!=dev_agent_and_com.end())
        return  (dev_run_state)dev_agent_and_com[sDevId].second->get_run_state();

    return dev_unknown;

}

void device_session::set_con_state(con_state curState)
{
    boost::recursive_mutex::scoped_lock llock(con_state_mutex_);
    if(othdev_con_state_!=curState)
    {
        othdev_con_state_ = curState;

        //判断该状态翻转是否需要进行设备通断状态通知(http...)
        if(short_connect_)
            return;

        map<string,DeviceInfo>::iterator iter = modleInfos_.mapDevInfo.begin();
        for(;iter!=modleInfos_.mapDevInfo.end();++iter)
        {
            //广播设备状态到在线客户端
            send_net_state_message(GetInst(LocalConfig).local_station_id(),(*iter).first
                                   ,(*iter).second.sDevName,(*iter).second.iDevType
                                   ,othdev_con_state_);
            if(othdev_con_state_== con_connected)
                GetInst(SvcMgr).get_notify()->OnDevStatus((*iter).first,0);
            else {
                dev_agent_and_com[(*iter).first].second->reset_run_state();
                GetInst(SvcMgr).get_notify()->OnDevStatus((*iter).first,-1);
            }

            //send_device_data_state_notify((*iter).first);
            //通知http服务器(设备网络异常)

            if(iter->second.iDevType == DEVICE_TRANSMITTER){
                int nMod = (curState==con_connected)?1:0;
                if(netAlarm.nAlarmId==-1 && nMod==0){
                    netAlarm.nAlarmId = time(0);
                    netAlarm.startTime = netAlarm.nAlarmId;
                    netAlarm.nType = 2;//发射机断开
                }
                if(netAlarm.nAlarmId>0){
                    //string sReason;
                    //http_ptr_->send_http_alarm_messge_to_platform((*iter).first,nMod,netAlarm,sReason);
                    if(nMod==1)
                        netAlarm.nAlarmId=-1;
                }

            }
        }
    }
}

//设备断线告警（博汇）
void device_session::send_device_data_state_notify(string sDevId,bool bHaveData){

    int nMod = (bHaveData==true)?1:0;
    if(netAlarm.nAlarmId==-1 && nMod==0){
        netAlarm.nAlarmId = time(0);
        netAlarm.startTime = netAlarm.nAlarmId;
        netAlarm.nType = 2;//发射机断开
    }
    if(netAlarm.nAlarmId>0){
        string sReason;
        http_ptr_->send_http_alarm_messge_to_platform(sDevId,nMod,netAlarm,sReason);
        if(nMod==1)
            netAlarm.nAlarmId=-1;
    }
}

//获得发射机报警状态
void device_session::get_alarm_state(string sDevId,map<int,map<int,CurItemAlarmInfo> >& cellAlarm)
{
    boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);
    if(mapItemAlarm.find(sDevId)!=mapItemAlarm.end())
        cellAlarm = mapItemAlarm[sDevId];
}

//返回连接模式
int  device_session::con_mod()
{
    if(modleInfos_.iCommunicationMode==CON_MOD_NET) {
        return modleInfos_.netMode.inet_type;
    }else if(modleInfos_.iCommunicationMode==CON_MOD_COM){
        return NET_MOD_COM;
    }
}

void device_session::connect()
{

    if(all_dev_is_use_==false)
        return;
    if(modleInfos_.iCommunicationMode==CON_MOD_NET) {
        if(modleInfos_.netMode.inet_type == NET_MOD_TCP || modleInfos_.netMode.inet_type == NET_MOD_HTTP){
            connect(modleInfos_.netMode.strIp,modleInfos_.netMode.iremote_port);
        }else if(modleInfos_.netMode.inet_type == NET_MOD_UDP){
            udp_connect(modleInfos_.netMode.strIp,modleInfos_.netMode.iremote_port);
        }else if(modleInfos_.netMode.inet_type == NET_MOD_SNMP){
            agent_connect(modleInfos_.netMode.strIp,modleInfos_.netMode.iremote_port);
        }
        /*else if(modleInfos_.netMode.inet_type == NET_MOD_HTTP){
            http_connect(modleInfos_.netMode.strIp,modleInfos_.netMode.iremote_port);
        }*/
    }else if(modleInfos_.iCommunicationMode==CON_MOD_COM){
        open_com();
    }
}

void   device_session::open_com()
{
    try
    {
        if (!pSerialPort_ptr_){
            pSerialPort_ptr_.reset(new boost::asio::serial_port(io_service_));
        }
        if(pSerialPort_ptr_->is_open()){
            boost::system::error_code err=boost::system::error_code();
            handle_connected(err);
            return;
        }
        boost::system::error_code  ec;

        if(modleInfos_.comMode.icomport>=0 && modleInfos_.comMode.icomport<64){
            string sComStr = str(boost::format("/dev/ttyO%1%")%modleInfos_.comMode.icomport);
            pSerialPort_ptr_->open(sComStr,ec);
            if(ec){
                std::cout<< sComStr<<" open error!!"<< ec.message()<<std::endl;
                return;
            }else {

                pSerialPort_ptr_->set_option(serial_port::baud_rate(modleInfos_.comMode.irate),ec);
                pSerialPort_ptr_->set_option(serial_port::flow_control(serial_port::flow_control::none),ec);
                pSerialPort_ptr_->set_option(serial_port::parity(serial_port::parity::none),ec);
                pSerialPort_ptr_->set_option(serial_port::stop_bits(serial_port::stop_bits::one),ec);
                if(modleInfos_.comMode.idata_bit>0)
                    pSerialPort_ptr_->set_option(serial_port::character_size(modleInfos_.comMode.idata_bit),ec);
                else
                    pSerialPort_ptr_->set_option(serial_port::character_size(8),ec);
            }

            boost::system::error_code err= boost::system::error_code();
            handle_connected(err);
        }
    }
    catch (std::exception& e){
        std::cerr << "com open exception"<<e.what() << std::endl;
    }

}

void device_session::connect(std::string hostname,unsigned short port,bool bReconnect)
{
    if(is_connected())
        return;

    boost::system::error_code ec;
    tcp::resolver::query query(hostname, boost::lexical_cast<std::string, unsigned short>(port));
    tcp::resolver::iterator iter = resolver_.resolve(query, ec);
    if(iter != tcp::resolver::iterator())
    {
        //正在连接
        //set_con_state(con_connecting);
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

//http连接(用来进行控制操作连接)
/*void device_session::http_connect(std::string hostname,unsigned short port)
{
    boost::system::error_code ec;
    tcp::resolver::query query(hostname, boost::lexical_cast<std::string, unsigned short>(port));
    tcp::resolver::iterator iter = resolver_.resolve(query, ec);
    if(iter != tcp::resolver::iterator()){
        endpoint_ = (*iter).endpoint();
        httpSocket().async_connect(endpoint_,boost::bind(&device_session::http_handle_connected,
                                                         this,boost::asio::placeholders::error));
        start_timeout_timer();//启动超时重连定时器
    }
    else {
        //出错。。。
    }
}*/

//关闭http控制连接
void device_session::http_cmd_close_i(){

    boost::system::error_code ignored_ec;
    httpSocket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    httpSocket().close(ignored_ec);
}

void device_session::http_handle_callback(const boost::system::error_code& error)
{
    if (!error)
    {

        boost::asio::streambuf http_urdl_response;
        boost::system::error_code error;
        while (boost::asio::read(http_stream_, http_urdl_response,
                   boost::asio::transfer_at_least(1), error));

        boost::asio::streambuf::const_buffers_type cbt = http_urdl_response.data();
        std::string request_contents(boost::asio::buffers_begin(cbt), boost::asio::buffers_end(cbt));
        cout<<request_contents<<endl;

        http_stream_.close();

    }else {

        http_stream_.close();
      std::cerr << error.message() << std::endl;
    }
}


//void device_session::http_handle_connected(const boost::system::error_code& error)
//{
//    if(http_command_id_.length() <= 0)
//        return;

//    std::ostream request_stream(&http_cmd_request_);

//    request_stream << "GET " << "http://"<<modleInfos_.netMode.strIp<<":"
//                   <<modleInfos_.netMode.iremote_port<<http_command_id_<< " HTTP/1.0\r\n";
//    request_stream << "Host: " <<modleInfos_.netMode.strIp << "\r\n";
//    request_stream << "Accept: */*\r\n";
//    request_stream << "Connection: close\r\n\r\n";

//    try{
//        // Send the request.
//        boost::asio::write(httpSocket(), http_cmd_request_);
//        boost::asio::streambuf response;
//        boost::asio::read_until(httpSocket(), response, "\r\n");

//        // Check that response is OK.
//        std::istream response_stream(&response);
//        std::string http_version;
//        response_stream >> http_version;
//        unsigned int status_code;
//        response_stream >> status_code;
//        std::string status_message;
//        std::getline(response_stream, status_message);
//        if (!response_stream || http_version.substr(0, 5) != "HTTP/"){
//            std::cout << "Invalid response\n";
//            http_cmd_close_i();
//            return ;
//        }
//        if (status_code != 200){
//            std::cout << "Response returned with status code " << status_code << "\n";
//            http_cmd_close_i();
//            return ;
//        }

//        // Read the response headers, which are terminated by a blank line.
//        boost::asio::read_until(httpSocket(), response, "\r\n\r\n");

//        // Process the response headers.
//        std::string header;
//        while (std::getline(response_stream, header) && header != "\r")
//            std::cout << header << "\n";
//        std::cout << "\n";

//        // Write whatever content we already have to output.
//        if (response.size() > 0)
//            std::cout << &response;

//        // Read until EOF, writing data to output as we go.
//        boost::system::error_code error;
//        while (boost::asio::read(httpSocket(), response,
//                   boost::asio::transfer_at_least(1), error))
//            std::cout << &response;


//        if (error != boost::asio::error::eof)
//            throw boost::system::system_error(error);

//        http_cmd_close_i();
//    }
//    catch(std::exception& e){

//        http_cmd_close_i();
//        std::cout << "Exception: " << e.what() << "\n";
//    }
//}



//udp连接
void device_session::udp_connect(std::string hostname,unsigned short port)
{
    boost::system::error_code ec;
    udp::resolver::query query(hostname,boost::lexical_cast<std::string, unsigned short>(port));
    udp::resolver::iterator iter = uresolver_.resolve(query,ec);
    if(iter!=udp::resolver::iterator())
    {
        uendpoint_ = (*iter).endpoint();
        usocket().open(udp::v4());
        const udp::endpoint local_endpoint = udp::endpoint(udp::v4(),port);
        usocket().bind(local_endpoint);
        //set_con_state(con_connected);
        boost::system::error_code err= boost::system::error_code();
        handle_connected(err);
    }
}

//agent 连接，适用于http,snmp
void device_session::agent_connect(std::string hostname,unsigned short port)
{


    IpAddress ipAddr(hostname.c_str());
    if (!ipAddr.valid())
        return;

    int status;
    if(snmp_ptr_==NULL){
        snmp_ptr_ = new Snmp(status);
        if(!snmp_ptr_->start_poll_thread(4))
            cout<<"start_poll_thread-----error!";
    }

    if(target_ptr_==NULL){
        target_ptr_ = new CTarget(ipAddr);
        if (! target_ptr_->valid())
            return;
    }

    set_con_state(con_connected);
    start_query_timer(moxa_config_ptr->query_interval);

    //初始化最后保存时间
    time_t curTm = time(0);
    map<string,time_t>::iterator iter = tmLastSaveTime.begin();
    for(;iter!=tmLastSaveTime.end();++iter)
        (*iter).second = curTm;
    return;
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
    //只有当前状态是正在连接才执行超时。。。
    // if(!is_connecting())
    //     return;
    if(is_connected())
        return;
    //超时了
    if(error!= boost::asio::error::operation_aborted)
        start_connect_timer();//启动重连尝试
}

//是否已经建立连接
bool device_session::is_connected()//string sDevId
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
    connect_timer_.expires_from_now(boost::posix_time::seconds(nSeconds));
    connect_timer_.async_wait(boost::bind(&device_session::connect_time_event,
                                          this,boost::asio::placeholders::error));
}

//连接超时
void device_session::connect_time_event(const boost::system::error_code& error)
{
    //只有当前状态是正在连接才执行超时。。。
    if(is_connected())
        return;
    if(error!= boost::asio::error::operation_aborted)
    {
        if(modleInfos_.iCommunicationMode == CON_MOD_NET){
            if(modleInfos_.netMode.inet_type == NET_MOD_TCP || modleInfos_.netMode.inet_type == NET_MOD_HTTP)
            {
                socket().async_connect(endpoint_,boost::bind(&device_session::handle_connected,
                                                             this,boost::asio::placeholders::error));
            }else if(modleInfos_.netMode.inet_type == NET_MOD_UDP){
                usocket().open(udp::v4());
                boost::system::error_code err= boost::system::error_code();
                handle_connected(err);
            }else if(modleInfos_.netMode.inet_type == NET_MOD_SNMP){
                agent_connect(modleInfos_.netMode.strIp,modleInfos_.netMode.iremote_port);
            }
        }else if(modleInfos_.iCommunicationMode == CON_MOD_COM){
            open_com();
        }
        start_timeout_timer();//启动超时定时器
    }
}


void device_session::disconnect()
{
    close_all();
    clear_all_alarm();//当停止服务时，清除所有报警
}

void device_session::start_read(int msgLen)
{
    if(msgLen > receive_msg_ptr_->space() || msgLen<=0)
        return;

    if(modleInfos_.iCommunicationMode==CON_MOD_NET){
        if(modleInfos_.netMode.inet_type == NET_MOD_TCP)
        {
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
        }else if(modleInfos_.netMode.inet_type == NET_MOD_UDP){

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
    }else if(modleInfos_.iCommunicationMode==CON_MOD_COM){

        boost::asio::async_read(*pSerialPort_ptr_,boost::asio::buffer(receive_msg_ptr_->w_ptr(),msgLen),
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
}

void device_session::start_read_head(int msgLen)
{
    if(msgLen>receive_msg_ptr_->space()  || msgLen<=0)
        return;

    if(modleInfos_.iCommunicationMode==CON_MOD_NET){
        if(modleInfos_.netMode.inet_type == NET_MOD_TCP)
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
        else if(modleInfos_.netMode.inet_type == NET_MOD_UDP){
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
    }else if(modleInfos_.iCommunicationMode==CON_MOD_COM){

        boost::asio::async_read(*pSerialPort_ptr_, boost::asio::buffer(receive_msg_ptr_->w_ptr(),
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
    //static  char tmp_query_cur_time[64];
    //time_t nowtm = time(0);
    //tm *local_time = localtime(&nowtm);
    //strftime(tmp_query_cur_time, sizeof(tmp_query_cur_time), "%Y-%m-%d %H:%M:%S", local_time);

    //cout<<"query_timer_event--->devId="<<cur_dev_id_<<"----"<<tmp_query_cur_time<<endl;
    if(error!= boost::asio::error::operation_aborted)
    {
        if(query_timeout_count_<moxa_config_ptr->query_timeout_count)
        {
            ++query_timeout_count_;
            if(modleInfos_.iCommunicationMode == CON_MOD_NET){
                if(modleInfos_.netMode.inet_type == NET_MOD_SNMP)// || modleInfos_.netMode.inet_type == NET_MOD_HTTP
                    get_sync_net_data();//获取网络数据(同步)，使用http,snmp
                else{
                    e_ErrorCode eErrCode = EC_UNKNOWN;
                    send_cmd_to_dev(cur_dev_id_,MSG_DEVICE_QUERY,cur_msg_q_id_,eErrCode);
                }
            }else if(modleInfos_.iCommunicationMode == CON_MOD_COM){
                e_ErrorCode eErrCode = EC_UNKNOWN;
                send_cmd_to_dev(cur_dev_id_,MSG_DEVICE_QUERY,cur_msg_q_id_,eErrCode);
            }
        }
        else{

            if(modleInfos_.mapDevInfo.size()<=1){
                close_all();
                start_connect_timer(moxa_config_ptr->connect_timer_interval);
                return;
            }
            query_timeout_count_ = 0;
            cur_msg_q_id_ = 0;
            //发送清零数据给客户端,该设备可能连接异常
            //DevMonitorDataPtr curData_ptr(new Data);
            //send_monitor_data_message(modleInfos_.sStationNumber,cur_dev_id_,(e_DevType)modleInfos_.mapDevInfo[cur_dev_id_].nDevType
            //	,curData_ptr,modleInfos_.mapDevInfo[cur_dev_id_].mapMonitorItem);

            cur_dev_id_ = next_dev_id();
            e_ErrorCode eErrCode = EC_UNKNOWN;
            send_cmd_to_dev(cur_dev_id_,MSG_DEVICE_QUERY,cur_msg_q_id_,eErrCode);
        }

        start_query_timer(moxa_config_ptr->query_interval);
    }
}
//获取同步网络数据，适用http,snmp
void device_session::get_sync_net_data()
{

    DevMonitorDataPtr curData_ptr(new Data);
    int nResult = dev_agent_and_com[cur_dev_id_].second->decode_msg_body(snmp_ptr_,curData_ptr,target_ptr_);
    if(nResult != 0){
        cout<<"get_sync_net_data---->dev_agent_and_com--->return:"<<nResult<<endl;
    }else{
        query_timeout_count_=0;
    }

    /* if(nResult == 0){
        if(boost::detail::thread::singleton<boost::threadpool::pool>::instance()
                .schedule(boost::bind(&device_session::handler_data,this,cur_dev_id_,curData_ptr))) {
            task_count_increase();
        }
    }*/
}
//发送消息
bool device_session::sendRawMessage(unsigned char * data_,int nDatalen)
{
    start_write(data_,nDatalen);
    return true;
}

//专门用于http协议发送控制指令（同步）
void device_session::http_send_task(const string &sCommandId){

    /*http_command_id_ = sCommandId;
    boost::system::error_code ec;
    tcp::resolver::query query(modleInfos_.netMode.strIp, boost::lexical_cast<std::string, unsigned short>(modleInfos_.netMode.iremote_port));
    tcp::resolver::iterator iter = resolver_.resolve(query, ec);
    if(iter != tcp::resolver::iterator()){
        endpoint_ = (*iter).endpoint();
        httpSocket().async_connect(endpoint_,boost::bind(&device_session::http_handle_connected,
                                                         this,boost::asio::placeholders::error));
    }*/


   try
   {
        std::ostringstream oss;
        std::ostream &os = oss;
        os << "http://" << modleInfos_.netMode.strIp<<":"
           <<modleInfos_.netMode.iremote_port<<sCommandId;

        boost::system::error_code ec;
        urdl::url url = urdl::url::from_string(oss.str(), ec);

        //urdl::option_set common_options;
        //common_options.set_option(urdl::http::max_redirects(0));
        http_stream_.set_option(urdl::http::request_method("GET"));
        http_stream_.set_option(urdl::http::request_content_type("text/plain"));
        //http_stream_.set_option(urdl::http::request_content("1"));
        //http_stream_.set_options(common_options);
        //http_stream_.set_ignore_return_content(true);

        http_stream_.async_open(url,boost::bind(&device_session::http_handle_callback,
                                                         this,boost::asio::placeholders::error));

    }
    catch(...)
    {
        http_stream_.close();
        std::cerr << "open url error ! "<< std::endl;
    }

}

void device_session::send_cmd_to_dev(CommandUnit cmdUnit,e_ErrorCode &eErrCode)
{
    start_write(cmdUnit.commandId,cmdUnit.commandLen);
}

//发送命令到设备
void device_session::send_cmd_to_dev(string sDevId,int cmdType,int childId,e_ErrorCode &eErrCode)
{
    //同步等待
    //boost::asio::deadline_timer delay_send_timer(io_service_, boost::posix_time::milliseconds(2));
    //    delay_send_timer.wait();

    if(modleInfos_.netMode.inet_type == NET_MOD_HTTP){

        map<int,vector<CommandUnit> >::iterator iter = dev_agent_and_com[sDevId].first->mapCommand.find(cmdType);
        if(iter!=dev_agent_and_com[sDevId].first->mapCommand.end()){
            if(iter->second.size()>childId){

                eErrCode = EC_CMD_SEND_SUCCEED;
                if(cmdType != MSG_DEVICE_QUERY)
                    http_send_task(iter->second[childId].sCommandId);
                else
                    start_write(iter->second[childId].commandId,iter->second[childId].commandLen);
            }
        }
        return;
    }


    map<int,vector<CommandUnit> >::iterator iter = dev_agent_and_com[sDevId].first->mapCommand.find(cmdType);
    if(iter!=dev_agent_and_com[sDevId].first->mapCommand.end()){
        if(iter->second.size()>childId){
            //string strHex;
            //CharStr2HexStr(iter->second[childId].commandId,strHex,iter->second[childId].commandLen);
            //cout<<"command string:"<<strHex<<endl;

            eErrCode = EC_CMD_SEND_SUCCEED;
            start_write(iter->second[childId].commandId,iter->second[childId].commandLen);
        }
    }
}

void device_session::start_write(unsigned char* commStr,int commLen)
{
    if(commLen<=0)
        return;
    if(modleInfos_.iCommunicationMode==CON_MOD_NET){
        if(modleInfos_.netMode.inet_type == NET_MOD_TCP)
        {
            if(get_con_state()!=con_connected)
                return;
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
        }else if(modleInfos_.netMode.inet_type == NET_MOD_HTTP){

            http_make_request_msg(MSG_DEVICE_QUERY);//组织request
            boost::asio::async_write(socket(), http_request_,
                         #ifdef USE_STRAND
                                     strand_.wrap(
                             #endif
                                         boost::bind(&device_session::http_handle_write_request, this,
                                                     boost::asio::placeholders::error)
                             #ifdef USE_STRAND
                                         )
                         #endif
                                     );
        }
        else if(modleInfos_.netMode.inet_type == NET_MOD_UDP)
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
    }else if(modleInfos_.iCommunicationMode == CON_MOD_COM)
    {
        if(get_con_state()!=con_connected)
            return;
        boost::asio::async_write(
                    *pSerialPort_ptr_,
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
}


//HTTP
void device_session::http_make_request_msg(int cmdType){

    devCommdMsgPtr commandmsg_(new DeviceCommandMsg);
    CommandUnit adjustTmCmd;
    //dev_agent_and_com[cur_dev_id_].second->GetSignalCommand(commandmsg_,adjustTmCmd);
    dev_agent_and_com[cur_dev_id_].second->GetSignalCommand(cmdType,0,adjustTmCmd);

    std::ostream request_stream(&http_request_);
    request_stream << "GET " << "http://"<<modleInfos_.netMode.strIp<<":"
                   <<modleInfos_.netMode.iremote_port<<adjustTmCmd.sCommandId<< " HTTP/1.0\r\n";
    request_stream << "Host: " <<modleInfos_.netMode.strIp << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";
}

void device_session::http_handle_write_request(const boost::system::error_code& err)
{
    if (!err)
    {
        boost::asio::async_read_until(socket(), http_response_, "\r\n",
                                      boost::bind(&device_session::http_handle_read_status_line, this,
                                                  boost::asio::placeholders::error));
    }
    else
    {
        std::cout << "Error: " << err.message() << "\n";
    }
}

void device_session::http_handle_read_status_line(const boost::system::error_code& err)
{
    if (!err)
    {
        // Check that response is OK.
        std::istream response_stream(&http_response_);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/"){
            std::cout << "Invalid response\n";
            return;
        }
        if (status_code != 200){
            std::cout << "Response returned with status code ";
            std::cout << status_code << "\n";
            return;
        }

        // Read the response headers, which are terminated by a blank line.
        boost::asio::async_read_until(socket(), http_response_, "\r\n\r\n",
                                      boost::bind(&device_session::http_handle_read_headers, this,
                                                  boost::asio::placeholders::error));
    }
    else
    {
        std::cout << "Error: " << err << "\n";
    }
}

void device_session::http_handle_read_headers(const boost::system::error_code& err)
{
    if (!err)
    {
        // Process the response headers.
        std::istream response_stream(&http_response_);
        std::string header;
        while (std::getline(response_stream, header) && header != "\r");
            std::cout << "\n";
        //std::cout << "\n";

        // Write whatever content we already have to output.
        //if (http_response_.size() > 0)
        //std::cout << &http_response_;

        // Start reading remaining data until EOF.
        boost::asio::async_read(socket(), http_response_,
                                boost::asio::transfer_at_least(1),
                                boost::bind(&device_session::http_handle_read_content, this,
                                            boost::asio::placeholders::error));
    }
    else
    {
        std::cout << "Error: " << err << "\n";
    }
}


void device_session::http_handle_read_content(const boost::system::error_code& err)
{
    if (!err)
    {
        // Write all of the data that has been read so far.
        // std::cout << &http_response_;


        // Continue reading remaining data until EOF.
        boost::asio::async_read(socket(), http_response_,
                                boost::asio::transfer_at_least(1),
                                boost::bind(&device_session::http_handle_read_content, this,
                                            boost::asio::placeholders::error));
    }
    else if (err != boost::asio::error::eof)
    {
        std::cout << "Error: " << err << "\n";
    }else if(err == boost::asio::error::eof){
        boost::asio::streambuf::const_buffers_type cbt = http_response_.data();
        http_content_ = std::string(buffers_begin(cbt), buffers_begin(cbt) + http_response_.size());
        //cout<<http_content_;

        //断开http连接
        http_close_all();

        DevMonitorDataPtr curData_ptr(new Data);
        int nResult = dev_agent_and_com[cur_dev_id_].second->decode_http_msg(http_content_,curData_ptr,CMD_QUERY,0);
        if(nResult == 0)
            handler_data(cur_dev_id_,curData_ptr);//处理数据
        start_connect_timer(moxa_config_ptr->connect_timer_interval);
    }
}


//等待任务结束
//void device_session::wait_task_end()
//{
//    boost::mutex::scoped_lock lock(task_mutex_);
//    while(task_count_>0){
//        task_end_conditon_.wait(task_mutex_);
//    }
//}

//提交任务
//void device_session::task_count_increase()
//{
//    boost::mutex::scoped_lock lock(task_mutex_);
//    ++task_count_;
//}
////任务递减
//void device_session::task_count_decrease()
//{
//    boost::mutex::scoped_lock lock(task_mutex_);
//    --task_count_;
//    task_end_conditon_.notify_all();
//}

//int device_session::task_count()
//{
//    boost::mutex::scoped_lock lock(task_mutex_);
//    return task_count_;
//}

void device_session::http_close_all(){

    short_connect_ = true;//短连接标志，不进行连接状态通知
    set_con_state(con_disconnected);
    connect_timer_.cancel();
    timeout_timer_.cancel();
    query_timer_.cancel();


    cur_msg_q_id_         =0;//当前命令id
    query_timeout_count_  =0;//命令发送超时次数清零
    close_i();   //关闭socket
    cur_msg_q_id_         =0;//当前命令id
    query_timeout_count_  =0;//命令发送超时次数清零

    http_request_.consume(http_request_.size());
    http_response_.consume(http_response_.size());
}

void device_session::close_all()
{
    set_con_state(con_disconnected);
    connect_timer_.cancel();
    timeout_timer_.cancel();
    query_timer_.cancel();
    if(modleInfos_.iCommunicationMode==CON_MOD_COM){
        boost::system::error_code    ec;
        pSerialPort_ptr_->close(ec);
        cout<<"close_all----close--com"<<ec<<endl;
    }
    else
        close_i();   //关闭socket

    //等待任务结束，任务里面，必须放在运行状态检测之前
    //状态检测会
    //wait_task_end();
    cur_msg_q_id_         =0;//当前命令id
    query_timeout_count_  =0;//命令发送超时次数清零

    if(snmp_ptr_!=NULL){
        snmp_ptr_->stop_poll_thread();
        delete snmp_ptr_,snmp_ptr_=NULL;
    }
    if(target_ptr_!=NULL){
        delete target_ptr_,target_ptr_=NULL;
    }

    http_request_.consume(http_request_.size());
    http_response_.consume(http_response_.size());

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

    time_t curTime = time(0);
    tm *pCurTime = localtime(&curTime);

    static  char str_time[64];
    strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", pCurTime);
    unsigned long cur_tm = pCurTime->tm_hour*3600+pCurTime->tm_min*60+pCurTime->tm_sec;

    map<string,DeviceInfo>::iterator witer = modleInfos_.mapDevInfo.begin();
    for(;witer!=modleInfos_.mapDevInfo.end();++witer)
    {
        boost::recursive_mutex::scoped_lock lock(update_cmd_schedule_mutex_);//增加锁防止读写竞争
        vector<Command_Scheduler>::iterator cmd_iter = witer->second.vCommSch.begin();
        for(;cmd_iter!=witer->second.vCommSch.end();++cmd_iter)
        {

            //按天控制
            if((*cmd_iter).iDateType == RUN_TIME_DAY){
                //在５秒内
                if(curTime>=(*cmd_iter).tExecuteTime && curTime<(*cmd_iter).tExecuteTime+5){

                    e_ErrorCode eResult = EC_OBJECT_NULL;
                    bool bRslt =  start_exec_task(witer->first,"timer",eResult,(*cmd_iter).iCommandType,
                                                  (*cmd_iter).iChannelId);
                    //通知客户端正在执行
                    if(bRslt==true){
                        notify_client_execute_result(witer->first,witer->second.sDevName,witer->second.iDevType,"timer",
                                                     (*cmd_iter).iCommandType,pCurTime,true,eResult);
                    }
                }
            }
            //按星期控制
            if((*cmd_iter).iDateType == RUN_TIME_WEEK){
                if(curTime> (*cmd_iter).tCmdEndTime &&  (*cmd_iter).tCmdEndTime>0)
                    continue;//超过运行图终止时间且终止时间不为0,则跳过

                pCurTime = localtime(&curTime);
                if((pCurTime->tm_wday)== (*cmd_iter).iWeek%7){

                    tm *pSetTimeS = localtime(&((*cmd_iter).tExecuteTime));
                    unsigned long set_tm_s = pSetTimeS->tm_hour*3600+pSetTimeS->tm_min*60+pSetTimeS->tm_sec;
                    if(cur_tm>=set_tm_s && cur_tm<(set_tm_s+5)){

                        e_ErrorCode eResult = EC_OBJECT_NULL;
                        bool bRslt = start_exec_task(witer->first,"timer",eResult,(*cmd_iter).iCommandType
                                                     ,(*cmd_iter).iChannelId);
                        //通知客户端正在执行
                        if(bRslt==true)
                            notify_client_execute_result(witer->first,witer->second.sDevName,witer->second.iDevType,"timer",
                                                         (*cmd_iter).iCommandType,pCurTime,true,eResult);
                    }
                }
            }

            //按月控制
            if((*cmd_iter).iDateType == RUN_TIME_MONTH){
                if(curTime> (*cmd_iter).tCmdEndTime &&  (*cmd_iter).tCmdEndTime>0)
                    continue;//超过运行图终止时间且终止时间不为0,则跳过
                pCurTime = localtime(&curTime);
                if((pCurTime->tm_mon+1)== (*cmd_iter).iMonitorMonth || (*cmd_iter).iMonitorMonth==0){
                    tm *pSetTimeS = localtime(&((*cmd_iter).tExecuteTime));
                    unsigned long set_tm_s = pSetTimeS->tm_hour*3600+pSetTimeS->tm_min*60+pSetTimeS->tm_sec;
                    if(cur_tm>=set_tm_s && cur_tm<(set_tm_s+5)){
                        e_ErrorCode eResult = EC_OBJECT_NULL;
                        bool bRslt = start_exec_task(witer->first,"timer",eResult,(*cmd_iter).iCommandType,
                                                     (*cmd_iter).iChannelId);
                        //通知客户端正在执行
                        if(bRslt==true)
                            notify_client_execute_result(witer->first,witer->second.sDevName,witer->second.iDevType,"timer",
                                                         (*cmd_iter).iCommandType,pCurTime,true,eResult);
                    }
                }
            }

        }
    }
    start_task_schedules_timer();
}

void device_session::notify_client_execute_result(string sDevId,string devName,int devType,string user,int cmdType, tm *pCurTime,
                                                  bool bNtfFlash,int eResult)
{
    static  char str_time[64];
    strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", pCurTime);

    //通知http服务器
    int cmdAckMsgType = cmdType;
    int cmdOpr=-1;
    int cmdRlt=-1;
    switch (cmdType) {
    case MSG_TRANSMITTER_TURNON_OPR:
    case MSG_TRANSMITTER_MIDDLE_POWER_TURNON_OPR:
    case MSG_TRANSMITTER_LOW_POWER_TURNON_OPR: {
        cmdOpr = (user=="timer")?CMD_EXC_A_ON:CMD_EXC_M_ON;
        cmdAckMsgType = MSG_TRANSMITTER_TURNON_ACK;
    } break;
    case MSG_TRANSMITTER_TURNOFF_OPR:{
        cmdOpr = (user=="timer")?CMD_EXC_A_OFF:CMD_EXC_M_OFF;
        cmdAckMsgType +=1;
    }break;
    case MSG_ANTENNA_HTOB_OPR:{
        cmdOpr = CMD_H_TO_B;
        cmdAckMsgType +=1;
    }break;
    case MSG_ANTENNA_BTOH_OPR:{
        cmdOpr = CMD_B_TO_H;
        cmdAckMsgType +=1;
    }
    default:
        cmdAckMsgType +=1;
        break;
    }
    //动环设备博汇要求收集发送(暂针对动环做单独收集处理...)
    if(GetInst(LocalConfig).http_svc_use() == true){
        if(eResult == EC_OPR_ON_GOING)
            cmdRlt = CMD_EXC_GOING;
        else if(eResult == EC_OK)
            cmdRlt = CMD_EXC_SUCCESS;
        else if(eResult == EC_FAILED)
            cmdRlt = CMD_EXC_FAILER;

        if(cmdRlt>-1 && cmdOpr>-1){
            string sDesc = devName+DEV_CMD_OPR_DESC(cmdOpr);
            sDesc+=DEV_CMD_RESULT_DESC(cmdRlt);
            http_ptr_->send_http_excute_result_messge_to_platform(sDevId,str_time,cmdOpr,sDesc);
        }

    }
    //发送给客户端
    if(bNtfFlash){

        send_command_execute_result_message(GetInst(LocalConfig).local_station_id(),sDevId,
                                            devType,devName,user,(e_MsgType)cmdAckMsgType,
                                            (e_ErrorCode)eResult);//DEVICE_TRANSMITTER
    }
}

void device_session::set_opr_state(string sdevId,dev_opr_state curState)
{
    boost::recursive_mutex::scoped_lock lock(opr_state_mutex_);
    dev_opr_state_[sdevId] = curState;
}

dev_opr_state  device_session::get_opr_state(string sdevId){
    boost::recursive_mutex::scoped_lock lock(opr_state_mutex_);
    return dev_opr_state_[sdevId];
}

//开始执行任务
bool device_session::start_exec_task(string sDevId,string sUser,e_ErrorCode &opResult,int cmdType,int nChannel)
{

    static  char str_time[64];
    time_t curTime = time(0);
    tm *pCurTime = localtime(&curTime);
    strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", pCurTime);

    if(!is_connected() && short_connect_==false){
        opResult = EC_NET_ERROR;
        return false;
    }


    if(get_opr_state(sDevId)==dev_no_opr)
        set_opr_state(sDevId,dev_opr_excuting);//设置正在执行任务标志
    else{
        opResult = EC_OPR_ON_GOING;//正在执行控制命令
        return false;//已经有任务正在执行
    }


    //现在执行任务
    if(modleInfos_.iCommunicationMode==CON_MOD_NET && modleInfos_.netMode.inet_type == NET_MOD_SNMP){
        dev_agent_and_com[sDevId].second->exec_task_now(cmdType,sUser,opResult,nChannel,
                                                        true,snmp_ptr_,target_ptr_);
    }
    else
        dev_agent_and_com[sDevId].second->exec_task_now(cmdType,sUser,opResult,nChannel);


    return true;

}

//判断是否保存当前记录
void device_session::save_monitor_record(string sDevId,DevMonitorDataPtr curDataPtr,
                                         const map<int,DeviceMonitorItem> &mapMonitorItem)
{
    time_t tmCurTime;
    time(&tmCurTime);
    double ninterval = difftime(tmCurTime,tmLastSaveTime[sDevId]);

    if(ninterval<run_config_ptr[sDevId]->data_save_interval)//间隔保存时间 need amend;
        return ;
    GetInst(DataBaseOperation).AddItemMonitorRecord(sDevId,tmCurTime,curDataPtr,mapMonitorItem);
    //amend by lk at 2017-7-12 无论是否记录成功都更新保存时间
    tmLastSaveTime[sDevId] = tmCurTime;
    //}

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

//判断当前时间是否需要上传
bool device_session::is_need_report_data(string sDevId)
{
    time_t tmCurTime;
    time(&tmCurTime);
    double ninterval = difftime(tmCurTime,tmLastSendHttpTime[sDevId]);

    int nReportSpan =  GetInst(LocalConfig).report_span();//10;
    map<string,DevProperty>::iterator iter_propty = modleInfos_.mapDevInfo[sDevId].map_DevProperty.find("ReportSpan");
    if(iter_propty!= modleInfos_.mapDevInfo[sDevId].map_DevProperty.end())
        nReportSpan = atoi(iter_propty->second.property_value.c_str());
    if(ninterval<nReportSpan)//间隔上报时间 need amend;
        return false;
    tmLastSendHttpTime[sDevId] = tmCurTime;
    return true;
}


//是否在监测时间段--------2016-4-1 ----完成
bool device_session::is_monitor_time(string sDevId)
{
    time_t curTime = time(0);
    tm *pnowTime = localtime(&curTime);
    unsigned long cur_tm = pnowTime->tm_hour*3600+pnowTime->tm_min*60+pnowTime->tm_sec;

    boost::recursive_mutex::scoped_lock lock(update_time_schedule_mutex_);
    //运行图----天
    map<int,vector<Monitoring_Scheduler> >::iterator day_iter = modleInfos_.mapDevInfo[sDevId].vMonitorSch.find(RUN_TIME_DAY);
    if(day_iter!=modleInfos_.mapDevInfo[sDevId].vMonitorSch.end()){
        vector<Monitoring_Scheduler>::iterator iter = day_iter->second.begin();
        for(;iter!=day_iter->second.end();++iter){
            if(!(*iter).bMonitorFlag)
                continue;

            if(curTime>=(*iter).tStartTime && curTime<(*iter).tEndTime && (*iter).bMonitorFlag){
                return (*iter).bRunModeFlag;
            }
        }
    }
    //运行图----星期
    map<int,vector<Monitoring_Scheduler> >::iterator week_iter = modleInfos_.mapDevInfo[sDevId].vMonitorSch.find(RUN_TIME_WEEK);
    if(week_iter!=modleInfos_.mapDevInfo[sDevId].vMonitorSch.end()){
        vector<Monitoring_Scheduler>::iterator iter = week_iter->second.begin();
        for(;iter!=week_iter->second.end();++iter){
            if(!(*iter).bMonitorFlag)
                continue;
            //if((*iter).iChannelId != ichannel)//add by lk 2017-8-17(区分通道控制)
            //    continue;
            tm *pCurTime = localtime(&curTime);
            if(curTime> (*iter).tAlarmEndTime &&  (*iter).tAlarmEndTime>0)
                continue;//超过运行图终止时间且终止时间不为0,则跳过

            if((pCurTime->tm_wday)== (*iter).iMonitorWeek%7){
                tm *pSetTimeS = localtime(&((*iter).tStartTime));
                unsigned long set_tm_s = pSetTimeS->tm_hour*3600+pSetTimeS->tm_min*60+pSetTimeS->tm_sec;
                tm *pSetTimeE = localtime(&((*iter).tEndTime));
                unsigned long set_tm_e = pSetTimeE->tm_hour*3600+pSetTimeE->tm_min*60+pSetTimeE->tm_sec;
                if(cur_tm>=set_tm_s && cur_tm<set_tm_e && (*iter).bMonitorFlag){
                    return (*iter).bRunModeFlag;
                }
            }
        }
    }
    //运行图----月
    map<int,vector<Monitoring_Scheduler> >::iterator month_iter = modleInfos_.mapDevInfo[sDevId].vMonitorSch.find(RUN_TIME_MONTH);
    if(month_iter!=modleInfos_.mapDevInfo[sDevId].vMonitorSch.end()){
        vector<Monitoring_Scheduler>::iterator iter = month_iter->second.begin();
        for(;iter!=month_iter->second.end();++iter){

            if(!(*iter).bMonitorFlag)
                continue;
            //if((*iter).iChannelId != ichannel)//add by lk 2017-8-17(区分通道控制)
            //    continue;
            tm *pCurTime = localtime(&curTime);
            if(curTime> (*iter).tAlarmEndTime &&  (*iter).tAlarmEndTime>0)
                continue;//超过运行图终止时间且终止时间不为0,则跳过

            if((pCurTime->tm_mon+1)== (*iter).iMonitorMonth ||  (*iter).iMonitorMonth==0){ //如果是当前月或者是all则比较
                if((pCurTime->tm_mday)== (*iter).iMonitorDay){
                    tm *pSetTimeS = localtime(&((*iter).tStartTime));
                    unsigned long set_tm_s = pSetTimeS->tm_hour*3600+pSetTimeS->tm_min*60+pSetTimeS->tm_sec;
                    tm *pSetTimeE = localtime(&((*iter).tEndTime));
                    unsigned long set_tm_e = pSetTimeE->tm_hour*3600+pSetTimeE->tm_min*60+pSetTimeE->tm_sec;
                    if(cur_tm>=set_tm_s && cur_tm<set_tm_e){
                        return (*iter).bRunModeFlag;
                    }
                }
            }
        }
    }

    return true;
}

//开始处理监测数据
void device_session::start_handler_data(string sDevId,DevMonitorDataPtr curDataPtr,bool bCheckAlarm)
{
    /*if(boost::detail::thread::singleton<boost::threadpool::pool>::instance()
            .schedule(boost::bind(&device_session::handler_data,this,sDevId,curDataPtr)))
    {
        task_count_increase();
    }*/
    handler_data(sDevId,curDataPtr);
}

void device_session::start_handler_data(int iaddcode, DevMonitorDataPtr curDataPtr, bool bCheckAlarm)
{
    string sdevid = get_devid_by_addcode(iaddcode);
    handler_data(sdevid,curDataPtr);
}

//2016-3-31------处理设备数据----完成
void device_session::handler_data(string sDevId,DevMonitorDataPtr curDataPtr)
{

    boost::recursive_mutex::scoped_lock lock(data_deal_mutex);
    if(curDataPtr->mValues.size()<=0)
        return;
    //是否在运行图时间
    bool bIsMonitorTime = is_monitor_time(sDevId);

    //打包发送http消息到上级平台
    int nDevType = modleInfos_.mapDevInfo[sDevId].iDevType;
    if(nDevType == DEVICE_TRANSMITTER){//博汇发射机需要上报开关机状态
        //插入一条编号为500的运行状态指标
        int runState = (get_run_state(sDevId)==dev_running)?1:0;
        DataInfo  tmInfo;
        tmInfo.bType=1;
        tmInfo.fValue = runState;
        curDataPtr->mValues[500] = tmInfo;
    }

    //打包发送客户端
    send_monitor_data_message(GetInst(LocalConfig).local_station_id(),sDevId,modleInfos_.mapDevInfo[sDevId].iDevType
                              ,curDataPtr,modleInfos_.mapDevInfo[sDevId].map_MonitorItem);



    //动环设备博汇要求收集发送(暂针对动环做单独收集处理...)
    if(GetInst(LocalConfig).http_svc_use() == true){

        //if(nDevType>DEVICE_TRANSMITTER && nDevType<DEVICE_GS_RECIVE){

            string sDesDevId = sDevId;
            map_dev_ass_parse_ptr_[sDevId]->get_parent_device_id(sDesDevId);
            http_ptr_->send_http_data_messge_to_platform(sDesDevId,nDevType,
                                                         curDataPtr,modleInfos_.mapDevInfo[sDevId].map_MonitorItem);
        //}else if(is_need_report_data(sDevId)){
//            string sDesDevId = sDevId;
//            //用来适应模块隶属于设备的问题
//            map_dev_ass_parse_ptr_[sDevId]->get_parent_device_id(sDesDevId);
//            http_ptr_->send_http_data_messge_to_platform(sDesDevId,nDevType,
//                                                         curDataPtr,modleInfos_.mapDevInfo[sDevId].map_MonitorItem);
//        }
    }



    //检测当前报警状态
    check_alarm_state(sDevId,curDataPtr,bIsMonitorTime);
    //如果在监测时间段则保存当前记录
    if(bIsMonitorTime)
        save_monitor_record(sDevId,curDataPtr,modleInfos_.mapDevInfo[sDevId].map_MonitorItem);

    //任务数递减
    //task_count_decrease();
    return;
}

void device_session::clear_dev_alarm(string sDevId)
{
    boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);
    map<int,map<int,CurItemAlarmInfo> >::iterator iter = mapItemAlarm[sDevId].begin();
    for(;iter!=mapItemAlarm[sDevId].end();++iter) {
        map<int,CurItemAlarmInfo>::iterator typeIter = iter->second.begin();
        for(;typeIter!=iter->second.end();++typeIter) {
            //写入恢复记录,通知客户端
            //.......
            record_alarm_and_notify(sDevId,0, 0,1,modleInfos_.mapDevInfo[sDevId].map_MonitorItem[iter->first],typeIter->second);
        }
        iter->second.clear();
    }
    mapItemAlarm[sDevId].clear();
}

void device_session::handle_connected(const boost::system::error_code& error)
{

    timeout_timer_.cancel();//关闭重连超时定时器
    if(!error)
    {
        set_con_state(con_connected);
        //开始启动接收第一条查询指令的回复数据头
        receive_msg_ptr_->reset();
        if(dev_agent_and_com[cur_dev_id_].second->IsStandardCommand()==true){

            start_read_head(dev_agent_and_com[cur_dev_id_].first->mapCommand[MSG_DEVICE_QUERY][cur_msg_q_id_].ackLen);
        }
        else
            start_read(dev_agent_and_com[cur_dev_id_].first->mapCommand[MSG_DEVICE_QUERY][cur_msg_q_id_].ackLen);

        if(modleInfos_.netMode.inet_type == NET_MOD_TCP || modleInfos_.netMode.inet_type == NET_MOD_HTTP ||
                modleInfos_.iCommunicationMode==CON_MOD_COM)
        {
            if(dev_agent_and_com[cur_dev_id_].second->is_auto_run()==false)
                start_query_timer(moxa_config_ptr->query_interval);
            else{
                dev_agent_and_com[cur_dev_id_].second->start();
            }

        }
        //初始化最后保存时间
        time_t curTm = time(0);
        map<string,time_t>::iterator iter = tmLastSaveTime.begin();
        for(;iter!=tmLastSaveTime.end();++iter)
            (*iter).second = curTm;
        return;
    }
    else	{

        cout<< error.message() << std::endl;
        close_i();
        start_connect_timer();
    }
}

void device_session::handle_read_head(const boost::system::error_code& error, size_t bytes_transferred)
{
    if(!is_connected())
        return;
    if (!error)
    {
        int nResult = receive_msg_ptr_->check_normal_msg_header(dev_agent_and_com[cur_dev_id_].second,
                                                                bytes_transferred,CMD_QUERY,cur_msg_q_id_);
        if(nResult>RE_SUCCESS)
            start_read_body(nResult);
        else{
            close_all();
            cout<<"handle_read_head---nResult<=0-----close"<<endl;
            start_connect_timer(moxa_config_ptr->connect_timer_interval);
        }
    }
    else{
        close_all();
        cout<<"handle_read_head---error!=0-----close"<<endl;
        start_connect_timer(moxa_config_ptr->connect_timer_interval);
    }
}



void device_session::handle_udp_read(const boost::system::error_code& error,size_t bytes_transferred)
{
    if(!is_connected())
        return;
    if (!error || error == boost::asio::error::message_size)
    {
        int nResult = receive_msg_ptr_->check_normal_msg_header(dev_agent_and_com[cur_dev_id_].second,bytes_transferred,CMD_QUERY,cur_msg_q_id_);

        if(nResult == RE_SUCCESS || nResult == RE_CMDACK)
        {
            DevMonitorDataPtr curData_ptr(new Data);
            int iaddcode=-1;
            int nResult = receive_msg_ptr_->decode_msg_body(dev_agent_and_com[cur_dev_id_].second,curData_ptr,bytes_transferred,iaddcode);
            if(nResult==0)//查询数据解析正确
            {
                query_timeout_count_ = 0;
                string sdevid = get_devid_by_addcode(iaddcode);
                //                if(boost::detail::thread::singleton<boost::threadpool::pool>::instance()
                //                        .schedule(boost::bind(&device_session::handler_data,this,sdevid,curData_ptr)))
                //                {
                //                    task_count_increase();
                //                }

                handler_data(sdevid,curData_ptr);
            }
        }
        start_read_head(bytes_transferred);


        //cout<< "接收长度---"<<bytes_transferred << std::endl;

    }
    else{
        cout<< error.message() << std::endl;
        close_all();
        start_connect_timer(moxa_config_ptr->connect_timer_interval);
    }
}



void device_session::start_read_body(int msgLen)
{
    if(modleInfos_.iCommunicationMode==CON_MOD_NET){
        if(modleInfos_.netMode.inet_type == NET_MOD_TCP)
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
        else if(modleInfos_.netMode.inet_type == NET_MOD_UDP)
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
    }else if(modleInfos_.iCommunicationMode==CON_MOD_COM){
        boost::asio::async_read(*pSerialPort_ptr_, boost::asio::buffer(receive_msg_ptr_->w_ptr(),
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

}


void device_session::handle_read_body(const boost::system::error_code& error, size_t bytes_transferred)
{
    if(!is_connected())
        return;
    if (!error)
    {

        DevMonitorDataPtr curData_ptr;//(new Data);
        //curData_ptr.reset();
        int iaddcode=-1;
        int nResult = receive_msg_ptr_->decode_msg_body(dev_agent_and_com[cur_dev_id_].second,curData_ptr,
                                                        bytes_transferred,iaddcode);
        if(nResult==RE_SUCCESS || nResult==RE_CMDACK)//查询数据解析正确,-2为控制指令返回值
        {
            query_timeout_count_ = 0;
            /*if(boost::detail::thread::singleton<boost::threadpool::pool>::instance()
                    .schedule(boost::bind(&device_session::handler_data,this,cur_dev_id_,curData_ptr)))
            {
                task_count_increase();
            }*/

            if(cur_msg_q_id_<dev_agent_and_com[cur_dev_id_].first->mapCommand[MSG_DEVICE_QUERY].size()-1)
                ++cur_msg_q_id_;
            else{
                cur_dev_id_ = next_dev_id();//切换到下一个设备
                cur_msg_q_id_=0;
            }
            start_read_head(dev_agent_and_com[cur_dev_id_].first->mapCommand[MSG_DEVICE_QUERY][cur_msg_q_id_].ackLen);
        }
        //else if(nResult>0)//跳过数据(针对数据管理器等设备的非查询指令回复)
        //    start_read_head(dev_agent_and_com[cur_dev_id_].first->mapCommand[MSG_DEVICE_QUERY][cur_msg_q_id_].ackLen);
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

//用于一次接收所有查询信息
void device_session::handle_read(const boost::system::error_code& error, size_t bytes_transferred)
{
    if(!is_connected())
        return;
    if(!error)
    {

        int nResult = receive_msg_ptr_->check_msg_header(dev_agent_and_com[cur_dev_id_].second,
                                                                bytes_transferred,CMD_QUERY,cur_msg_q_id_);
        if(nResult == RE_SUCCESS || nResult == RE_CMDACK)
        {
            query_timeout_count_ = 0;
            if(cur_msg_q_id_<dev_agent_and_com[cur_dev_id_].first->mapCommand[MSG_DEVICE_QUERY].size()-1)
            {
                ++cur_msg_q_id_;
                boost::this_thread::sleep(boost::posix_time::milliseconds(200));
                e_ErrorCode eErrCode = EC_UNKNOWN;
                send_cmd_to_dev(cur_dev_id_,MSG_DEVICE_QUERY,cur_msg_q_id_,eErrCode);
            }
            else{
                cur_msg_q_id_=0;
                DevMonitorDataPtr curData_ptr(new Data);
                int iaddcode=-1;
                int nResult = receive_msg_ptr_->decode_msg(dev_agent_and_com[cur_dev_id_].second,curData_ptr,bytes_transferred,iaddcode);
                if(nResult == RE_SUCCESS || nResult == RE_CMDACK){
                    string sdevid = get_devid_by_addcode(iaddcode);
                    handler_data(sdevid,curData_ptr);//处理数据
                    cur_dev_id_ = next_dev_id();//切换到下一个设备
                }
                else{
                    close_all();
                    cout<<"handle_read---nResult!=0-----close"<<endl;
                    start_connect_timer();
                    return;
                }
            }
            start_read(dev_agent_and_com[cur_dev_id_].first->mapCommand[MSG_DEVICE_QUERY][cur_msg_q_id_].ackLen);
        }
        else if(nResult > RE_SUCCESS){//还有后续消息
            start_read(nResult);
        }
        else if(nResult == RE_HEADERROR){
            close_all();
            start_connect_timer();
            return;
        }
    }
    else{
        close_all();
        cout<<"handle_read---error!=0-----close"<<endl;
        start_connect_timer();
        return;
    }
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


    //存储告警到数据库
    if(!bMod){//0:告警产生

        curAlarm.sReason = str(boost::format("%1%%2%,当前值：%3%%4%,门限值：%5%%6%")
                               %ItemInfo.sItemName%CONST_STR_ALARM_CONTENT(curAlarm.nLimitId)
                               %fValue%ItemInfo.sUnit%fLimitValue%ItemInfo.sUnit);

        bool bRslt = GetInst(DataBaseOperation).AddItemAlarmRecord(devId,curAlarm.startTime,
                                                                   ItemInfo.iItemIndex,curAlarm.nLimitId,curAlarm.nType,
                                                                   fValue,curAlarm. sReason,curAlarm.nAlarmId);
        if(bRslt==true){
            //提交监控量告警到上级平台
            string sDesDevId = devId;
            map_dev_ass_parse_ptr_[devId]->get_parent_device_id(sDesDevId);
            if(GetInst(LocalConfig).http_svc_use()){
                int nDevType = modleInfos_.mapDevInfo[devId].iDevType;
                //针对烟感水浸设备做id替换
                if(nDevType==DEVICE_SMOKE || nDevType==DEVICE_WATER){

                   sDesDevId = str(boost::format("%s-%d")%sDesDevId%ItemInfo.iItemIndex);
                }

                http_ptr_->send_http_alarm_messge_to_platform(sDesDevId,modleInfos_.mapDevInfo[devId].iDevType,
                                                              bMod,curAlarm,curAlarm.sReason);
            }
            //发送监控量报警到客户端
            send_alarm_state_message(GetInst(LocalConfig).local_station_id(),devId,modleInfos_.mapDevInfo[devId].sDevName,ItemInfo.iItemIndex
                                     ,modleInfos_.mapDevInfo[devId].iDevType,curAlarm.nLimitId,str_time,mapItemAlarm[devId][ItemInfo.iItemIndex].size(),curAlarm.sReason);


        }
    }else{//1:告警恢复
        time_t curTime = time(0);
        bool bRslt = GetInst(DataBaseOperation).AddItemEndAlarmRecord(curTime,curAlarm.nAlarmId);
        if(bRslt==true){
            //提交监控量告警到上级平台
            string sDesDevId = devId;
            map_dev_ass_parse_ptr_[devId]->get_parent_device_id(sDesDevId);
            if(GetInst(LocalConfig).http_svc_use()){
                int nDevType = modleInfos_.mapDevInfo[devId].iDevType;
                //针对烟感水浸设备做id替换
                if(nDevType==DEVICE_SMOKE || nDevType==DEVICE_WATER){

                   sDesDevId = str(boost::format("%s-%d")%sDesDevId%ItemInfo.iItemIndex);
                }

                http_ptr_->send_http_alarm_messge_to_platform(sDesDevId,nDevType,bMod,curAlarm,curAlarm.sReason);
            }

            //发送监控量报警到客户端
            send_alarm_state_message(GetInst(LocalConfig).local_station_id(),devId,modleInfos_.mapDevInfo[devId].sDevName,ItemInfo.iItemIndex
                                     ,modleInfos_.mapDevInfo[devId].iDevType,RESUME,str_time,mapItemAlarm[devId][ItemInfo.iItemIndex].size(),curAlarm.sReason);
            // 联动.....
        }
    }
}
//分析告警---2016-4-1--完成
void  device_session::parse_item_alarm(string devId,float fValue,DeviceMonitorItem &ItemInfo,bool &bAlarmNow)
{
    boost::recursive_mutex::scoped_lock lock(update_alarm_config_mutex_);
    //遍历监控量告警配置
    for(int nIndex=0;nIndex<ItemInfo.vItemAlarm.size();++nIndex)
    {
        //  if(ItemInfo.vItemAlarm[nIndex].bIsAlarm==false)
        //       continue;
        CurItemAlarmInfo  tmp_alarm_info;
        int iLimittype = ItemInfo.vItemAlarm[nIndex].iLimittype;
        bool  bIsAlarm=false;
        if(ItemInfo.iItemType == ITEM_ANALOG){//模拟量
            //上限,上上限告警
            if(iLimittype == ALARM_UPPER || iLimittype == ALARM_UP_UPPER) {
                if(fValue > ItemInfo.vItemAlarm[nIndex].fLimitvalue)
                    bIsAlarm=true;
            }else{//下限,下下限告警
                if(fValue < ItemInfo.vItemAlarm[nIndex].fLimitvalue)
                    bIsAlarm=true;
            }
        }else{//状态量
            if(fValue == ItemInfo.vItemAlarm[nIndex].fLimitvalue)
                bIsAlarm=true;
        }

        if(bIsAlarm == true){
            boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);
            //判断该监控项是否在告警
            map<int,map<int,CurItemAlarmInfo> >::iterator findIter = mapItemAlarm[devId].find(ItemInfo.iItemIndex);
            if(findIter != mapItemAlarm[devId].end()){
                //判断该告警类型是否在告警
                map<int,CurItemAlarmInfo>::iterator alarm_type_findIter = mapItemAlarm[devId][ItemInfo.iItemIndex].find(iLimittype);
                if(alarm_type_findIter != mapItemAlarm[devId][ItemInfo.iItemIndex].end()){
                    //判断该告警是否已经通知
                    if(alarm_type_findIter->second.bNotifyed == true)
                        continue;
                    //计算持续时间,判断进行告警
                    time_t curTime = time(0);
                    double  dtime_during = difftime( curTime, alarm_type_findIter->second.startTime );
                    if(dtime_during>=ItemInfo.vItemAlarm[nIndex].iDelaytime){
                        //存储告警,通知告警,联动告警
                        record_alarm_and_notify(devId,fValue, ItemInfo.vItemAlarm[nIndex].fLimitvalue,0,ItemInfo,alarm_type_findIter->second);
                        //分析告警联动
                        paser_action(ItemInfo.vItemAlarm[nIndex].vLinkAction,devId,ItemInfo,alarm_type_findIter->second);

                        alarm_type_findIter->second.bNotifyed = true;
                        //标志当前有新的告警发生
                        bAlarmNow = true;
                    }
                }else{ //没有找到,增加该告警类型

                    tmp_alarm_info.startTime = time(0);//记录时间
                    tmp_alarm_info.nType = ItemInfo.vItemAlarm[nIndex].iAlarmid;//告警类型
                    tmp_alarm_info.nLimitId = iLimittype;
                    tmp_alarm_info.nModuleId = ItemInfo.iModDevId;
                    tmp_alarm_info.nModuleType = ItemInfo.iModTypeId;
                    tmp_alarm_info.nTargetId = ItemInfo.iTargetId;
                    tmp_alarm_info.bNotifyed = false;
                    mapItemAlarm[devId][ItemInfo.iItemIndex][iLimittype] = tmp_alarm_info;
                }
            }else {
                //没有找到增加该告警监控项
                tmp_alarm_info.startTime = time(0);//记录时间
                tmp_alarm_info.nType = ItemInfo.vItemAlarm[nIndex].iAlarmid;//告警类型
                tmp_alarm_info.nLimitId = iLimittype;//限值类型
                tmp_alarm_info.nModuleId = ItemInfo.iModDevId;
                tmp_alarm_info.nModuleType = ItemInfo.iModTypeId;
                tmp_alarm_info.nTargetId = ItemInfo.iTargetId;
                tmp_alarm_info.bNotifyed = false;
                map<int,CurItemAlarmInfo>  tmTypeAlarm;
                tmTypeAlarm[iLimittype] = tmp_alarm_info;
                // boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);
                mapItemAlarm[devId][ItemInfo.iItemIndex] = tmTypeAlarm;
            }
        }else {
            //当前没有告警,判断是否存在此告警,确定是否需要恢复,移除该告警
            boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);
            map<int,map<int,CurItemAlarmInfo> >::iterator findIter = mapItemAlarm[devId].find(ItemInfo.iItemIndex);
            //查找是否有该告警项
            if(findIter != mapItemAlarm[devId].end()){
                map<int,CurItemAlarmInfo>::iterator findTypeIter =  findIter->second.find(iLimittype);
                if(findTypeIter != findIter->second.end()){
                    if(findTypeIter->second.nLimitId !=ALARM_RESUME){
                        findTypeIter->second.startTime = time(0);//记录时间
                        findTypeIter->second.nLimitId = ALARM_RESUME;
                    }else{
                        //如果该告警项没有在延迟范围内通知，且已经恢复，则清除告警项，不通知用户
                        if(findTypeIter->second.bNotifyed != true){
                            findIter->second.erase(findTypeIter);//移除该告警类型告警
                            if( findIter->second.size()<=0)
                                mapItemAlarm[devId].erase(findIter);//如果监控项告警为空,则移除该监控量告警
                            continue;
                        }
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
}

//清除所有报警状态
void device_session::clear_all_alarm()
{
    //boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);
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
    map<string,DeviceInfo>::iterator iter = modleInfos_.mapDevInfo.find(sDevId);
    if(iter== modleInfos_.mapDevInfo.end())
        return;
    bool bTmpAlarmNow = false;
    map<int,DeviceMonitorItem>::iterator iterItem = iter->second.map_MonitorItem.begin();
    for(;iterItem!=iter->second.map_MonitorItem.end();++iterItem){
        double dbValue =curDataPtr->mValues[iterItem->first].fValue;
        parse_item_alarm(sDevId,dbValue,iterItem->second,bTmpAlarmNow);
    }
    //如果有新告警产生,则立刻发送一次监控数据
    if(bTmpAlarmNow==true && GetInst(LocalConfig).report_use()){
        string sDesDevId = sDevId;
        map_dev_ass_parse_ptr_[sDesDevId]->get_parent_device_id(sDesDevId);
        http_ptr_->send_http_data_messge_to_platform(sDesDevId,modleInfos_.mapDevInfo[sDevId].iDevType,
                                                     curDataPtr,modleInfos_.mapDevInfo[sDevId].map_MonitorItem);
    }
}

void device_session::sendSmsToUsers(int nLevel,string &sContent)
{
    vector<SendMSInfo>& smsInfo=GetInst(StationConfig).get_sms_user_info();
    for(int i=0;i<smsInfo.size();++i)
    {
        if(smsInfo[i].iAlarmLevel==nLevel)
        {
            //string sCenterId = GetInst(LocalConfig).sms_center_number();
            //扩平台需要，暂时删去
            //GetInst(CSmsTraffic).SendSMSContent(sCenterId,smsInfo[i].sPhoneNumber,sContent);
        }
    }

}
//用来执行通用指令,根据具体参数,组织命令字符串,同时发送命令消息（废弃）
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
bool device_session::excute_command(int cmdType,string sUser,devCommdMsgPtr lpParam,e_ErrorCode &opResult)
{
    if(!is_connected()){
        opResult = EC_NET_ERROR;
        return false;
    }

    string sDevId = lpParam->sdevid();
    if(modleInfos_.mapDevInfo[sDevId].iDevType == DEVICE_TRANSMITTER){
        if(get_opr_state(sDevId)==dev_no_opr)
            set_opr_state(sDevId,dev_opr_excuting);//设置正在执行任务标志
        else{
            opResult = EC_OPR_ON_GOING;//正在执行控制命令
            return false;//已经有任务正在执行
        }
    }

    //现在执行任务
    if(modleInfos_.iCommunicationMode==CON_MOD_NET && modleInfos_.netMode.inet_type == NET_MOD_SNMP){
        //dev_agent_and_com[sDevId].second->exec_task_now(cmdType,sUser,opResult,true,snmp_ptr_,target_ptr_);
    }
    else{
        dev_agent_and_com[sDevId].second->exec_general_task(cmdType,sUser,lpParam,opResult);
    }
    return true;
}

//更新运行图
void device_session::update_monitor_time(string devId,map<int,vector<Monitoring_Scheduler> >& monitorScheduler,
                                         vector<Command_Scheduler> &cmmdScheduler)
{
    {
        boost::recursive_mutex::scoped_lock lock(update_time_schedule_mutex_);
        modleInfos_.mapDevInfo[devId].vMonitorSch = monitorScheduler;
    }

    {
        boost::recursive_mutex::scoped_lock lock(update_cmd_schedule_mutex_);
        modleInfos_.mapDevInfo[devId].vCommSch = cmmdScheduler;
    }
}

//更新告警配置
void  device_session::update_dev_alarm_config(string sDevId,DeviceInfo &devInfo)
{
    {
        //更新整机告警配置
        boost::recursive_mutex::scoped_lock alarm_lock(update_alarm_config_mutex_);
        modleInfos_.mapDevInfo[sDevId].map_AlarmConfig = devInfo.map_AlarmConfig;
    }
    {
        //更新监控项告警
        map<int,DeviceMonitorItem>::iterator iterItem = modleInfos_.mapDevInfo[sDevId].map_MonitorItem.begin();
        for(;iterItem!=modleInfos_.mapDevInfo[sDevId].map_MonitorItem.end();++iterItem){
            boost::recursive_mutex::scoped_lock alarm_lock(update_alarm_config_mutex_);
            iterItem->second.vItemAlarm = devInfo.map_MonitorItem[iterItem->first].vItemAlarm;
            if(iterItem->second.vItemAlarm.size() ==0)
                clear_dev_item_alarm(sDevId,iterItem->first);
            //.....清理之前已产生的告警
        }
    }
}

//清理设备未设置的告警项
void device_session::clear_dev_item_alarm(string sDevId,int nitemId)
{
    boost::recursive_mutex::scoped_lock lock(alarm_state_mutex);
    map<int,map<int,CurItemAlarmInfo> >::iterator iter = mapItemAlarm[sDevId].find(nitemId);
    if(iter==mapItemAlarm[sDevId].end())
        return ;
    map<int,CurItemAlarmInfo>::iterator typeIter = iter->second.begin();
    for(;typeIter!=iter->second.end();++typeIter) {
        //写入恢复记录,通知客户端
        //.......
        record_alarm_and_notify(sDevId,0, 0,1,modleInfos_.mapDevInfo[sDevId].map_MonitorItem[iter->first],typeIter->second);
    }
    iter->second.clear();
}

//发送联动指令
void device_session::send_action_conmmand(map<int,vector<ActionParam> > &param,string sUser,int actionType,e_ErrorCode &opResult){

    if(!is_connected()){
        opResult = EC_NET_ERROR;
        return;
    }
    //如果是发射机则启用循环发送
    string sDevId = param[0][0].strParamValue;
    if(modleInfos_.mapDevInfo[sDevId].iDevType == DEVICE_TRANSMITTER){
        if(get_opr_state(sDevId)==dev_no_opr)
            set_opr_state(sDevId,dev_opr_excuting);//设置正在执行任务标志
        else{
            opResult = EC_OPR_ON_GOING;//正在执行控制命令
            return;//已经有任务正在执行
        }
    }


    dev_agent_and_com[sDevId].second->exec_action_task_now(param,actionType,sUser,opResult);

}

void device_session::doAction(LinkAction &action, string sStationid, string sDevid, string sDevName,
                              int devType, DeviceMonitorItem &ItemInfo, string sStartTime, string sReason)
{
    switch(action.iActionType)
    {
    case ACTP_SENDMSG:{//发短信
        string smscontent;
        string strDevtype = GetInst(StationConfig).get_dictionary_value("DeviceType",devType);
        smscontent = str(boost::format("%1%:%2%(%3%)---%4%")%strDevtype%sDevName%sDevid%sReason);
        Action_sendsms(action,smscontent);
    }
        break;
    case ACTP_TELE://打电话
        break;
    case ACTP_SEND_CLIENT://通知客户端
        break;
    case ACTP_OPEN_DEVICE://开设备
        break;
    case ACTP_CLOSE_DEVICE://关设备
        break;
    case ACTP_SWITCH_ANTTEN://天线切换
        break;
    case ACTP_SOUND_LIGHT_ALARM:{//声光告警
        if(action.iIshaveParam && action.map_Params.size()>0){

            GetInst(SvcMgr).SendActionCommand(action.map_Params,"action",action.iActionType);
        }
    }
        break;
    }
}

//短信发送动作
void device_session::Action_sendsms(LinkAction &action, string sendcontent)
{
    if(action.map_Params.size()<1)
        return;
    vector<string> phonelist;
    map<int,vector<ActionParam> >::iterator iter = action.map_Params.begin();
    for(;iter!=action.map_Params.end();++iter)
    {
        vector<ActionParam>::iterator viter = (*iter).second.begin();
        for(;viter!=(*iter).second.end();++viter)
        {
            if((*viter).iParamType==0)
            {
                UserInformation tmpUser;
                if(GetInst(DataBaseOperation).GetUserInfoByNumber((*viter).strParamValue,tmpUser))
                    phonelist.push_back(tmpUser.sTelephone);
            }
        }
    }
    GetInst(SvcMgr).SendSMSContent(phonelist,sendcontent);
}


void device_session::paser_action(vector<LinkAction> &vaction,string &devId,DeviceMonitorItem &ItemInfo,CurItemAlarmInfo &curAlarm)
{
    static  char str_time[64];
    tm *local_time = localtime(&curAlarm.startTime);
    strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", local_time);
    vector<LinkAction>::iterator vaciter = vaction.begin();
    for(;vaciter!=vaction.end();++vaciter){
        doAction((*vaciter),GetInst(LocalConfig).local_station_id(),devId,modleInfos_.mapDevInfo[devId].sDevName,
                 modleInfos_.mapDevInfo[devId].iDevType, ItemInfo,str_time,curAlarm.sReason);
    }

}

}
