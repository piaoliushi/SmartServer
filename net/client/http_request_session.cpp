#include "http_request_session.h"
#include "./protocol/bohui_protocol.h"
#include "./protocol/bohui_const_define.h"
#include "LocalConfig.h"
namespace hx_net {

http_request_session::http_request_session(boost::asio::io_service& io_service)
    :http_stream_(io_service)
{
}

 http_request_session::~http_request_session(void)
 {

 }

 //开始连接
 void http_request_session::openUrl(std::string sUrl,std::string sData,std::string sRqstType)
 {
     urdl::option_set common_options;
     // Prevent HTTP redirections.
     common_options.set_option(urdl::http::max_redirects(0));

     boost::recursive_mutex::scoped_lock lock(http_stream_mutex_);
     http_stream_.set_option(urdl::http::request_method(sRqstType));
     http_stream_.set_option(urdl::http::request_content_type("text/plain"));
     http_stream_.set_option(urdl::http::request_content(sData));
     http_stream_.set_options(common_options);
     http_stream_.async_open(sUrl, boost::bind(&http_request_session::open_handler,
                                               this,boost::asio::placeholders::error));
 }

 void http_request_session::open_handler(const boost::system::error_code& ec)
 {
   if (!ec)
   {
        http_stream_.close();
     // URL successfully opened.
     //  boost::array<char, 512> data;
     // boost::asio::async_read(http_stream_, boost::asio::buffer(data),boost::bind(&http_request_session::read_handler, this,
      //                                                                                          boost::asio::placeholders::error,
      //                                                                                          boost::asio::placeholders::bytes_transferred));

   }else {
       http_stream_.close();
     std::cerr << "Unable to open URL: ";
     std::cerr << ec.message() << std::endl;
   }

 }

 //上报http消息到上级平台(数据)
 void http_request_session::send_http_data_messge_to_platform(string sDevid,int nDevType,DevMonitorDataPtr &curData,
                                                              map<int,DeviceMonitorItem> &mapMonitorItem)
 {
     string sReportMsg;
     Bohui_Protocol  bh_ptcl;
     bh_ptcl.createReportDataMsg(-1,sDevid,nDevType,curData,mapMonitorItem,sReportMsg);
     if(sReportMsg.empty()==false)
         openUrl(GetInst(LocalConfig).relay_svc_ip(),sReportMsg);
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
     else if(temType>=DEVICE_GS_RECIVE)
         temType = BH_POTO_LinkDevAlarmReport;//链路设备
     else if(temType==DEVICE_TRANSMITTER)
         temType = BH_POTO_QualityAlarmReport;//发射机设备
     if(temType>=0){
        bh_ptcl.createReportAlarmDataMsg(-1,temType,sDevid,alarmInfo,nMod,reason,sReportMsg);
        if(sReportMsg.empty()==false)
            openUrl(GetInst(LocalConfig).relay_svc_ip(),sReportMsg);
     }
 }

 //上报http消息到上级平台(通讯异常告警)
 void http_request_session::send_http_alarm_messge_to_platform(string sDevid,int nMod,CurItemAlarmInfo &alarmInfo,string &reason)
 {
     string sReportMsg;
     Bohui_Protocol  bh_ptcl;
     bh_ptcl.createReportAlarmDataMsg(-1,BH_POTO_CommunicationReport,sDevid,alarmInfo,nMod,reason,sReportMsg);
     if(sReportMsg.empty()==false)
         openUrl(GetInst(LocalConfig).relay_svc_ip(),sReportMsg);
 }

 //上报http消息到上级平台(执行结果)
 void http_request_session::send_http_excute_result_messge_to_platform(string sDevid,string &sTime,int ndevState,const string &desc)
 {
     string sReportMsg;
     Bohui_Protocol  bh_ptcl;
     bh_ptcl.creatExcutResultReportMsg(-1,BH_POTO_CmdStatusReport,sDevid,sTime,ndevState,desc,sReportMsg);
     if(sReportMsg.empty()==false)
         openUrl(GetInst(LocalConfig).relay_svc_ip(),sReportMsg);
 }

}

