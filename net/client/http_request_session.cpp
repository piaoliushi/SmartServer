#include "http_request_session.h"
#include "./protocol/bohui_protocol.h"
#include "./protocol/bohui_const_define.h"
#include "LocalConfig.h"
namespace hx_net {

http_request_session::http_request_session(boost::asio::io_service& io_service,bool bAsycFlag)
    :http_io_service_(io_service)
    ,asycFlag_(bAsycFlag)//
    ,_taskqueueptr(new TaskQueue< pair<string,string> >)
    ,d_bExit_(false)
    ,http_stream_(http_io_service_)
{
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
              urdl::read_stream http_stream_(http_io_service_);
              //调用子类work，处理具体任务
               urdl::option_set common_options;
               common_options.set_option(urdl::http::max_redirects(0));
               http_stream_.set_option(urdl::http::request_method("POST"));
               http_stream_.set_option(urdl::http::request_content_type("text/plain"));
               http_stream_.set_option(urdl::http::request_content(task_.second));
               http_stream_.set_options(common_options);
               http_stream_.set_ignore_return_content(true);
              try
              {
                  cout<<task_.second<<endl;
                  if(asycFlag_==true){
                        http_stream_.async_open(task_.first,boost::bind(&http_request_session::open_handler,
                                                                         this,boost::asio::placeholders::error));
                  }else{
                      boost::system::error_code ec;
                      //cout<<"http_stream_.open----------start!!!"<<endl;
                      http_stream_.open(task_.first, ec);
                      //cout<<"task size:------"<<_taskqueueptr->get_Task_Size()<<"-------"<<ec.message()<<endl;
                      http_stream_.close();
                  }
              }
              catch (...)
              {
                  http_stream_.close();
                  std::cerr << "open url error ! "<< std::endl;
              }

          }
     } while (!isExit());
 }

 void http_request_session::open_handler(const boost::system::error_code& ec)
 {

   if (!ec)
   {

       //std::cerr << "open URL ok !!!: ";
        http_stream_.close();
       //http_stream_.async_read_some(boost::asio::buffer(data_),boost::bind(&http_request_session::read_handler, this,
       //                                                                                         boost::asio::placeholders::error,
        //                                                                                        boost::asio::placeholders::bytes_transferred));


   }else {

     //std::cerr << "Unable to open URL: ";
     http_stream_.close();
     std::cerr << ec.message() << std::endl;
   }

 }

/* void http_request_session::read_handler(const boost::system::error_code& ec, std::size_t length)
 {
     if (!ec)
     {
         //http_stream_.close();
         std::cerr << length << std::endl;

     }
     else
     {
         //http_stream_.close();
       std::cerr << "Unable to open URL: ";
       std::cerr << ec.message() << std::endl;
     }
     //http_stream_.close();
 }*/
 //上报http消息到上级平台(数据)
 void http_request_session::send_http_data_messge_to_platform(string sDevid,int nDevType,DevMonitorDataPtr &curData,
                                                              map<int,DeviceMonitorItem> &mapMonitorItem)
 {
     string sReportMsg;
     Bohui_Protocol  bh_ptcl;
     bh_ptcl.createReportDataMsg(-1,sDevid,nDevType,curData,mapMonitorItem,sReportMsg);
     if(sReportMsg.empty()==false)
         putHttpMessage(GetInst(LocalConfig).report_svc_url(),sReportMsg);
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
     bh_ptcl.createReportAlarmDataMsg(-1,BH_POTO_CommunicationReport,sDevid,alarmInfo,nMod,reason,sReportMsg);
     if(sReportMsg.empty()==false)
         putHttpMessage(GetInst(LocalConfig).report_svc_url(),sReportMsg);
 }

 //上报http消息到上级平台(执行结果)
 void http_request_session::send_http_excute_result_messge_to_platform(string sDevid,string sTime,int ndevState,const string &desc)
 {
     string sReportMsg;
     Bohui_Protocol  bh_ptcl;
     bh_ptcl.creatExcutResultReportMsg(-1,BH_POTO_CmdStatusReport,sDevid,sTime,ndevState,desc,sReportMsg);
     if(sReportMsg.empty()==false)
         putHttpMessage(GetInst(LocalConfig).report_svc_url(),sReportMsg);
 }
 //查询dtmb前端频率,节目配置
 void http_request_session::query_dtmb_program_config()
 {
     string sReportMsg;
     Bohui_Protocol  bh_ptcl;
     bh_ptcl.creatQueryDtmbPrgInfoMsg(sReportMsg);
     if(sReportMsg.empty()==false)
         putHttpMessage(GetInst(LocalConfig).report_svc_url(),sReportMsg);
 }

}
