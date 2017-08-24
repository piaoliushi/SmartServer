#ifndef HTTP_REQUEST_SESSION_H
#define HTTP_REQUEST_SESSION_H
#include "../net_session.h"
#include "../taskqueue.h"
#include <urdl/read_stream.hpp>
#include "./rapidxml/rapidxml.hpp"
#include "./rapidxml/rapidxml_utils.hpp"
#include "./rapidxml/rapidxml_print.hpp"

using namespace rapidxml;
namespace hx_net {
class http_request_session;
typedef boost::shared_ptr<http_request_session>  http_request_session_ptr;
typedef boost::weak_ptr<http_request_session>    http_request_session_weak_ptr;
class http_request_session
{
public:
    http_request_session(boost::asio::io_service& io_service,bool bAsycFlag);
    virtual ~http_request_session(void);
    //提交httpTask
    void putHttpMessage(std::string sUrl,std::string &sData);
    //打开url
    void openUrl();
    //上报http消息到上级平台(数据)
    void send_http_data_messge_to_platform(string sDevid,int nDevType,DevMonitorDataPtr &curData,
                                           map<int,DeviceMonitorItem> &mapMonitorItem);
    //上报http消息到上级平台(执行结果)
    void send_http_excute_result_messge_to_platform(string sDevid,string sTime,int ndevState,const string &desc);
    //上报http消息到上级平台(告警)
    void send_http_alarm_messge_to_platform(string sDevid,int nDevType,int nMod,CurItemAlarmInfo &alarmInfo,string &reason);
    //上报http消息到上级平台(通讯异常告警)
    void send_http_alarm_messge_to_platform(string sDevid,int nMod,CurItemAlarmInfo &alarmInfo,string &reason);
    //查询dtmb节目配置
    void query_dtmb_program_config();
protected:
    bool isExit();
    void setExit();
    //void  read_handler(const boost::system::error_code& ec, std::size_t length);
    void  open_handler(const boost::system::error_code& ec);
    //判断当前时间是否需要上传
    bool  is_need_report_data();
private:

    boost::asio::io_service    &http_io_service_;
    boost::shared_ptr<TaskQueue<pair<string,string> > > _taskqueueptr;//任务队列

    boost::recursive_mutex         deal_thread_mutex_;//http上报对象互斥量
    boost::shared_ptr<boost::thread> deal_thread_;//网络监听线程
    bool d_bExit_;
    bool asycFlag_;
    urdl::read_stream http_stream_;


    boost::recursive_mutex         http_stream_mutex_;//http上报对象互斥量
    time_t   report_span_;//动环上报时间统计

    xml_document<> xml_reportMsg;//临时缓存上报数据
    map<int,xml_node<>* >   xml_mapQualityMsg;//缓存设备节点
    map<string,xml_node<>*> xml_mapDevMsg;//缓存设备数据
};

}


#endif // HTTP_REQUEST_SESSION_H
