#include "http_request_session.h"
#include "./protocol/bohui_protocol.h"
#include "./protocol/bohui_const_define.h"
#include "LocalConfig.h"
#include "yaolog.h"
#include "utility.h"
#include <QString>
#include <QTextCodec>
namespace hx_net {

http_request_session::http_request_session(boost::asio::io_service& io_service,bool bAsycFlag)
    :http_io_service_(io_service)
    ,asycFlag_(bAsycFlag)//
    ,_taskqueueptr(new TaskQueue< pair<string,string> >)
    ,d_bExit_(false)
    //,http_stream_(http_io_service_)

{
    env_report_span_  = time(0);
    envex_report_span_ = env_report_span_;
    tsmt_report_span_ = env_report_span_;
    link_report_span_ = env_report_span_;
    switch_report_span_ = env_report_span_;
    deal_thread_.reset(new boost::thread(boost::bind(&http_request_session::openUrl,this)));
}

http_request_session::~http_request_session(void)
{
     setExit();
     if(deal_thread_!=NULL)
     {
         if(deal_thread_->joinable())
         {
             deal_thread_->interrupt();
             deal_thread_->join();
         }
     }

 }

 //提交httpTask
 void http_request_session::putHttpMessage(std::string sUrl,std::string &sData)
 {
     //如果没有开启http服务则不进行数据上传
     if(GetInst(LocalConfig).http_svc_use()==false)
         return;
     if(!isExit())
        _taskqueueptr->SubmitTask(pair<string,string>(sUrl,sData));
 }

 bool http_request_session::isExit()
 {
     boost::recursive_mutex::scoped_lock lock(deal_thread_mutex_);
     return d_bExit_;
 }

 void http_request_session::setExit()
 {
     boost::recursive_mutex::scoped_lock lock(deal_thread_mutex_);
     d_bExit_ = true;
 }


  //开始连接
 void http_request_session::openUrl()
 {

     do
     {
         //从队列中取任务进行处理
         pair<string,string> task_ = _taskqueueptr->GetTask();
         if(!task_.first.empty()){
              //urdl::read_stream http_stream_(http_io_service_);
              //urdl::istream http_stream2_();
              //调用子类work，处理具体任务
               urdl::option_set common_options;
               common_options.set_option(urdl::http::max_redirects(1));
               http_stream2_.set_option(urdl::http::request_method("POST"));
               http_stream2_.set_option(urdl::http::request_content_type("text/plain"));
               http_stream2_.set_option(urdl::http::request_content(task_.second));
               http_stream2_.set_options(common_options);
               //http_stream2_.set_ignore_return_content(true);
              try
              {

                    string sCovStr;
#ifdef Q_OS_WIN
                  UTF8_to_GB2312_Win(task_.second.c_str(),sCovStr);
#else
                    sCovStr= task_.second;
                    utf8ToGb2312(sCovStr);
#endif

                 cout<<sCovStr<<endl;
                 // LOG__("info",task_.second.c_str());
                  if(asycFlag_==true){
                        //http_stream_.async_open(task_.first,boost::bind(&http_request_session::open_handler,
                        //                                                 this,boost::asio::placeholders::error));
                  }else{
                      boost::system::error_code ec;
                      //cout<<"http_stream_.open----------start!!!"<<endl;
                      //LOG__("info", "http_stream_.open----------start!!!");
                      http_stream2_.open_timeout(2000);
                      http_stream2_.open(task_.first);
                      if(!http_stream2_){
                          // If the open operation timed out then:
                          //if(http_stream2_.error() == boost::system::errc::timed_out)
                              //LOG__("info", "http_stream_.open timeout");
                      }
                      cout<<"http_stream_.open success ---task size:--"<<_taskqueueptr->get_Task_Size()<<"---"<<ec.message()<<endl;
                      //LOG__("info", "http_stream_.open success ---task size:--%d",_taskqueueptr->get_Task_Size());
                      http_stream2_.close();
                  }
              }
              catch (boost::system::error_code& e)
              {
                  http_stream2_.close();
                  //std::cerr<< "open url error ! " << std::endl;
                  LOG__("info", "open url error ! ");
              }

              boost::this_thread::sleep(boost::posix_time::millisec(50));
          }

     } while (!isExit());
 }

 void http_request_session::open_handler(const boost::system::error_code& ec)
 {

   if (!ec)
   {

       //std::cerr << "open URL ok !!!: ";
        //http_stream_.close();
   }else {

     //std::cerr << "Unable to open URL: ";
     //http_stream_.close();
     std::cerr << ec.message() << std::endl;
   }

 }

 //判断当前时间是否需要上传
 bool http_request_session::is_need_report_data(time_t &oldtime)
 {
     time_t tmCurTime;
     time(&tmCurTime);
     double ninterval = difftime(tmCurTime,oldtime);

     int nReportSpan = GetInst(LocalConfig).report_span();
     if(ninterval>=0 && ninterval<nReportSpan)//间隔保存时间 need amend;
          return false;

     //cout<<"nReportSpan----oldtime="<<oldtime<<"---curtime="<<tmCurTime<<"---interval="<<ninterval<<endl;
     //oldtime = tmCurTime;//外部赋值

     return true;
 }




 //上报http消息到上级平台(数据)
 void http_request_session::send_http_data_messge_to_platform(string sDevid,int nDevType,DevMonitorDataPtr &curData,
                                                              map<int,DeviceMonitorItem> &mapMonitorItem,bool bImmediately)
 {
     //动环数据收集发送
      if(nDevType>DEVICE_TRANSMITTER && nDevType<DEVICE_GS_RECIVE && nDevType!=DEVICE_SWITCH){
          //ups单独处理，适配博汇新动环协议
          if(nDevType==DEVICE_UPS)
          {
              boost::recursive_mutex::scoped_lock lock(http_envex_stream_mutex_);
              if(is_need_report_data(envex_report_span_) || bImmediately==true){
                  //发送数据
                  string sReportMsg;
                  Bohui_Protocol  bh_ptcl;
                  xml_node<>* pHeadMsg=NULL;//消息头节点
                  if(bh_ptcl.createReportHeadMsg(xml_envex_reportMsg,pHeadMsg,BH_POTO_EnvQualityReport)) {

                      if(xml_envex_mapDevMsg.find(sDevid)==xml_envex_mapDevMsg.end()){

                          bh_ptcl.appendUpsReportBodyMsg(xml_envex_reportMsg,xml_envex_mapQualityMsg,sDevid,
                                                                  nDevType,curData,mapMonitorItem);

                          xml_envex_mapDevMsg[sDevid] = pHeadMsg;
                      }

                      map<string,xml_node<>* >::iterator iter = xml_envex_mapQualityMsg.begin();
                      for(;iter!=xml_envex_mapQualityMsg.end();++iter)
                          pHeadMsg->append_node(iter->second);

                      rapidxml::print(std::back_inserter(sReportMsg), xml_envex_reportMsg, 0);
                  }

                  if(sReportMsg.empty()==false)
                      putHttpMessage(GetInst(LocalConfig).report_svc_url(),sReportMsg);

                  xml_envex_reportMsg.clear();
                  xml_envex_mapQualityMsg.clear();
                  xml_envex_mapDevMsg.clear();
                  envex_report_span_ = time(0);

              }else{
                  //添加数据,且检查该设备在时间段内是否已经添加过了
                  Bohui_Protocol  bh_ptcl;
                  if(xml_envex_mapDevMsg.find(sDevid) == xml_envex_mapDevMsg.end()){

                    bh_ptcl.appendUpsReportBodyMsg(xml_envex_reportMsg,xml_envex_mapQualityMsg,sDevid,
                                                               nDevType,curData,mapMonitorItem);

                    xml_envex_mapDevMsg[sDevid] = NULL;
                  }
              }
          }
          else{

           boost::recursive_mutex::scoped_lock lock(http_env_stream_mutex_);
           if(is_need_report_data(env_report_span_) || bImmediately==true){
               //发送数据
               string sReportMsg;
               Bohui_Protocol  bh_ptcl;
               xml_node<>* pHeadMsg=NULL;//消息头节点
               int bh_xml_root_name = BH_POTO_EnvQualityRealtimeReport;
               if(nDevType==DEVICE_UPS)
                   bh_xml_root_name = BH_POTO_EnvQualityReport;
               if(bh_ptcl.createReportHeadMsg(xml_env_reportMsg,pHeadMsg,bh_xml_root_name)) {

                   if(xml_env_mapDevMsg.find(sDevid)==xml_env_mapDevMsg.end()){

                       bh_ptcl.appendPowerEnvReportBodyMsg(xml_env_reportMsg,xml_env_mapQualityMsg,sDevid,
                                                               nDevType,curData,mapMonitorItem);
                       xml_env_mapDevMsg[sDevid] = pHeadMsg;
                   }
                  map<int,xml_node<>* >::iterator iter = xml_env_mapQualityMsg.begin();
                  for(;iter!=xml_env_mapQualityMsg.end();++iter)
                     pHeadMsg->append_node(iter->second);


                  rapidxml::print(std::back_inserter(sReportMsg), xml_env_reportMsg, 0);
               }

               if(sReportMsg.empty()==false)
                   putHttpMessage(GetInst(LocalConfig).report_svc_url(),sReportMsg);

               xml_env_reportMsg.clear();
               xml_env_mapQualityMsg.clear();
               xml_env_mapDevMsg.clear();

               env_report_span_ = time(0);

           }
           else{
               //添加数据,且检查该设备在时间段内是否已经添加过了
               Bohui_Protocol  bh_ptcl;
               if(xml_env_mapDevMsg.find(sDevid) == xml_env_mapDevMsg.end()){


                 bh_ptcl.appendPowerEnvReportBodyMsg(xml_env_reportMsg,xml_env_mapQualityMsg,sDevid,
                                                    nDevType,curData,mapMonitorItem);
                 xml_env_mapDevMsg[sDevid] = NULL;
               }
            }
         }

      }
      else if(nDevType == DEVICE_TRANSMITTER){//发射机
          boost::recursive_mutex::scoped_lock lock(http_tsmt_stream_mutex_);
          if(is_need_report_data(tsmt_report_span_)|| bImmediately==true){
              //发送数据
              string sReportMsg;
              Bohui_Protocol  bh_ptcl;
              xml_node<>* pHeadMsg=NULL;//消息头节点
              if(bh_ptcl.createReportHeadMsg(xml_tsmt_reportMsg,pHeadMsg,BH_POTO_QualityRealtimeReport)) {
                  if(xml_tsmt_mapDevMsg.find(sDevid)==xml_tsmt_mapDevMsg.end()){

                      bh_ptcl.appendTransmitterReportBodyMsg(xml_tsmt_reportMsg,xml_tsmt_mapQualityMsg,sDevid,
                                                          curData,mapMonitorItem);
                  }
                  map<string,xml_node<>* >::iterator iter = xml_tsmt_mapQualityMsg.begin();
                  for(;iter!=xml_tsmt_mapQualityMsg.end();++iter)
                      pHeadMsg->append_node(iter->second);
                  rapidxml::print(std::back_inserter(sReportMsg), xml_tsmt_reportMsg, 0);
              }

              if(sReportMsg.empty()==false){
                  putHttpMessage(GetInst(LocalConfig).report_svc_url(),sReportMsg);
              }
              xml_tsmt_reportMsg.clear();
              xml_tsmt_mapQualityMsg.clear();
              xml_tsmt_mapDevMsg.clear();

              tsmt_report_span_ = time(0);

          }else{
              //添加数据,且检查该设备在时间段内是否已经添加过了
              Bohui_Protocol  bh_ptcl;
              if(xml_tsmt_mapDevMsg.find(sDevid) == xml_tsmt_mapDevMsg.end()){

                  bh_ptcl.appendTransmitterReportBodyMsg(xml_tsmt_reportMsg,xml_tsmt_mapQualityMsg,sDevid,
                                                      curData,mapMonitorItem);
                  xml_tsmt_mapDevMsg[sDevid] = NULL;
              }
          }

      }
#if 0
      else if(nDevType==DEVICE_SWITCH)
      {
          //切换器设备
          boost::recursive_mutex::scoped_lock lock(http_switch_stream_mutex_);
          if(is_need_report_data(switch_report_span_)|| bImmediately==true){
              //发送数据
              string sReportMsg;
              Bohui_Protocol  bh_ptcl;
              xml_node<>* pHeadMsg=NULL;//消息头节点
              if(bh_ptcl.createReportHeadMsg(xml_switch_reportMsg,pHeadMsg,BH_POTO_SwitchDevStatusReport)) {
                  if(xml_switch_mapDevMsg.find(sDevid)==xml_switch_mapDevMsg.end()){

                      bh_ptcl.appendSwtichReportBodyMsg(xml_switch_reportMsg,xml_switch_mapQualityMsg,sDevid,
                                                              nDevType,curData,mapMonitorItem);
                  }

                  map<string,xml_node<>* >::iterator iter = xml_switch_mapQualityMsg.begin();
                  for(;iter!=xml_switch_mapQualityMsg.end();++iter)
                      pHeadMsg->append_node(iter->second);

                  rapidxml::print(std::back_inserter(sReportMsg), xml_switch_reportMsg, 0);

              }

              if(sReportMsg.empty()==false)
                  putHttpMessage(GetInst(LocalConfig).report_svc_url(),sReportMsg);

              xml_switch_reportMsg.clear();
              xml_switch_mapQualityMsg.clear();
              xml_switch_mapDevMsg.clear();
              switch_report_span_ = time(0);

          }else{
              Bohui_Protocol  bh_ptcl;
              if(xml_switch_mapDevMsg.find(sDevid) == xml_switch_mapDevMsg.end()){


                bh_ptcl.appendSwtichReportBodyMsg(xml_switch_reportMsg,xml_switch_mapQualityMsg,sDevid,
                                                   nDevType,curData,mapMonitorItem);
                xml_switch_mapDevMsg[sDevid] = NULL;
              }
          }
      }
#endif
      else if((nDevType >= DEVICE_GS_RECIVE && nDevType <= DEVICE_ADAPTER)|| nDevType == DEVICE_MEDIA ||nDevType==DEVICE_SWITCH)
      {
          int nDeviceType = nDevType;
          if(nDeviceType == DEVICE_SWITCH)
              nDeviceType = DEVICE_POWER;//博汇切换器设备116

          //链路设备
          boost::recursive_mutex::scoped_lock lock(http_link_stream_mutex_);
          if(is_need_report_data(link_report_span_)|| bImmediately==true){

              //发送数据
              string sReportMsg;
              Bohui_Protocol  bh_ptcl;
              xml_node<>* pHeadMsg=NULL;//消息头节点
              if(bh_ptcl.createReportHeadMsg(xml_link_reportMsg,pHeadMsg,BH_POTO_LinkDevQualityReport)) {
                  if(xml_link_mapDevMsg.find(sDevid)==xml_link_mapDevMsg.end()){

                      bh_ptcl.appendLinkReportBodyMsg(xml_link_reportMsg,xml_link_mapQualityMsg,sDevid,nDeviceType,
                                                          curData,mapMonitorItem);
                  }
                  map<string,xml_node<>* >::iterator iter = xml_link_mapQualityMsg.begin();
                  for(;iter!=xml_link_mapQualityMsg.end();++iter)
                      pHeadMsg->append_node(iter->second);
                  rapidxml::print(std::back_inserter(sReportMsg), xml_link_reportMsg, 0);
              }

              if(sReportMsg.empty()==false){
                  putHttpMessage(GetInst(LocalConfig).report_svc_url(),sReportMsg);
                  cout<<"link dev putHttpMessage--devid="<<sDevid<<endl;
              }
              else
                  cout<<"sReportMsg is empty----stop put!"<<endl;


              xml_link_reportMsg.clear();
              xml_link_mapQualityMsg.clear();
              xml_link_mapDevMsg.clear();
              link_report_span_ = time(0);

          }else{
              //添加数据,且检查该设备在时间段内是否已经添加过了
              Bohui_Protocol  bh_ptcl;
              if(xml_link_mapDevMsg.find(sDevid) == xml_link_mapDevMsg.end()){

                  bh_ptcl.appendLinkReportBodyMsg(xml_link_reportMsg,xml_link_mapQualityMsg,
                                                  sDevid,nDeviceType,curData,mapMonitorItem);
                  xml_link_mapDevMsg[sDevid] = NULL;
              }
          }
      }

 }

 //上报http消息到上级平台(告警)
 void http_request_session::send_http_alarm_messge_to_platform(string sDevid,int nDevType,int nMod,CurItemAlarmInfo &alarmInfo,string &reason)
 {
     string sReportMsg;
     Bohui_Protocol  bh_ptcl;
     //类型分类判断
     int temType = -1;
     if(nDevType>DEVICE_TRANSMITTER && nDevType<DEVICE_GS_RECIVE)
         temType = BH_POTO_EnvAlarmReport;//动环设备
     else if(nDevType>=DEVICE_GS_RECIVE)
         temType = BH_POTO_LinkDevAlarmReport;//链路设备
     else if(nDevType==DEVICE_TRANSMITTER)
         temType = BH_POTO_QualityAlarmReport;//发射机设备
     if(temType>=0){

        bh_ptcl.createReportAlarmDataMsg(-1,temType,sDevid,alarmInfo,nMod,reason,sReportMsg);
        if(sReportMsg.empty()==false)
            putHttpMessage(GetInst(LocalConfig).report_svc_url(),sReportMsg);
     }
 }

 //上报http消息到上级平台(通讯异常告警)
 void http_request_session::send_http_alarm_messge_to_platform(string sDevid,int nMod,CurItemAlarmInfo &alarmInfo,string &reason)
 {
     string sReportMsg;
     Bohui_Protocol  bh_ptcl;
     bh_ptcl.createReportAlarmDataMsg(-1,BH_POTO_CommunicationReport,sDevid,alarmInfo,nMod,reason,sReportMsg);//,alarmInfo.alarmLevel
     if(sReportMsg.empty()==false)
         putHttpMessage(GetInst(LocalConfig).report_svc_url(),sReportMsg);
 }

 //上报http消息到上级平台(执行结果)
 void http_request_session::send_http_excute_result_messge_to_platform(string sDevid,int devType,string sTime,int ndevState,const string &desc)
 {
     string sReportMsg;
     Bohui_Protocol  bh_ptcl;
     if(devType == 116)//切换通知
     {
         bh_ptcl.creatDeviceExcutResultReportMsg(-1,BH_POTO_CmdStatusReport,sDevid,devType,sTime,ndevState,desc,sReportMsg);
     }else{
         bh_ptcl.creatExcutResultReportMsg(-1,BH_POTO_CmdStatusReport,sDevid,sTime,ndevState,desc,sReportMsg);
     }

     if(sReportMsg.empty()==false)
         putHttpMessage(GetInst(LocalConfig).report_svc_url(),sReportMsg);
 }

 //查询dtmb前端频率,节目配置
#if 0
 void http_request_session::query_dtmb_program_config()
 {
     string sReportMsg;
     Bohui_Protocol  bh_ptcl;
     bh_ptcl.creatQueryDtmbPrgInfoMsg(sReportMsg);
     if(sReportMsg.empty()==false)
         putHttpMessage(GetInst(LocalConfig).report_svc_url(),sReportMsg);
 }
#endif
}
