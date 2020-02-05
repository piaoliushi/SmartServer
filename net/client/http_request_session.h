#ifndef HTTP_REQUEST_SESSION_H
#define HTTP_REQUEST_SESSION_H
#include "../net_session.h"
#include "../taskqueue.h"
#include <urdl/read_stream.hpp>
#include <urdl/istream.hpp>
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
                                           map<int,DeviceMonitorItem> &mapMonitorItem,bool bImmediately=false);
    //上报http消息到上级平台(执行结果)
    void send_http_excute_result_messge_to_platform(string sDevid,int devType,string sTime,int ndevState,const string &desc);

    //上报http消息到上级平台(告警)
    void send_http_alarm_messge_to_platform(string sDevid,int nDevType,int nMod,CurItemAlarmInfo &alarmInfo,string &reason);

    //上报http消息到上级平台(通讯异常告警)
    void send_http_alarm_messge_to_platform(string sDevid,int nMod,CurItemAlarmInfo &alarmInfo,string &reason);

    //查询dtmb节目配置
    //void query_dtmb_program_config();
protected:
    bool isExit();
    void setExit();
    //void  read_handler(const boost::system::error_code& ec, std::size_t length);
    void  open_handler(const boost::system::error_code& ec);
    //判断当前时间是否需要上传
    bool  is_need_report_data(time_t &oldtime);
private:

    boost::asio::io_service    &http_io_service_;
    boost::shared_ptr<TaskQueue<pair<string,string> > > _taskqueueptr;//任务队列

    boost::recursive_mutex         deal_thread_mutex_;//http上报对象互斥量
    boost::shared_ptr<boost::thread> deal_thread_;//网络监听线程
    bool d_bExit_;
    bool asycFlag_;
    //urdl::read_stream http_stream_;
    urdl::istream  http_stream2_;


    boost::recursive_mutex         http_env_stream_mutex_; //动环上报对象互斥量
    time_t                         env_report_span_;       //动环上报时间统计
    xml_document<>                 xml_env_reportMsg;      //动环临时缓存上报数据
    map<int,xml_node<>* >          xml_env_mapQualityMsg;  //动环缓存设备节点
    //map<string,xml_node<>*>        xml_env_mapQualityMsgEx;//动环缓存设备节点
    map<string,xml_node<>*>        xml_env_mapDevMsg;      //动环缓存设备记录

    boost::recursive_mutex         http_tsmt_stream_mutex_;//发射机上报对象互斥量
    time_t                         tsmt_report_span_;      //发射机上报时间统计
    xml_document<>                 xml_tsmt_reportMsg;     //发射机临时缓存上报数据
    map<string,xml_node<>* >       xml_tsmt_mapQualityMsg; //发射机缓存设备节点
    map<string,xml_node<>*>        xml_tsmt_mapDevMsg;     //发射机缓存设备记录

    boost::recursive_mutex         http_link_stream_mutex_;//链路上报对象互斥量
    time_t                         link_report_span_;      //链路上报时间统计
    xml_document<>                 xml_link_reportMsg;     //链路临时缓存上报数据
    map<string,xml_node<>* >       xml_link_mapQualityMsg; //链路缓存设备节点
    map<string,xml_node<>*>        xml_link_mapDevMsg;     //链路缓存设备记录

    boost::recursive_mutex         http_envex_stream_mutex_; //动环扩展（UPS）上报对象互斥量
    time_t                         envex_report_span_;       //动环扩展（UPS）上报时间统计
    xml_document<>                 xml_envex_reportMsg;      //动环扩展（UPS）临时缓存上报数据
    map<string,xml_node<>*>        xml_envex_mapQualityMsg;  //动环扩展（UPS）缓存设备节点
    map<string,xml_node<>*>        xml_envex_mapDevMsg;      //动环扩展（UPS）缓存设备记录

    boost::recursive_mutex         http_switch_stream_mutex_; //切换器上报对象互斥量
    time_t                         switch_report_span_;       //切换器上报时间统计
    xml_document<>                 xml_switch_reportMsg;      //切换器临时缓存上报数据
    map<string,xml_node<>*>        xml_switch_mapQualityMsg;  //切换器缓存设备节点
    map<string,xml_node<>*>        xml_switch_mapDevMsg;      //切换器缓存设备记录
};

}


#endif // HTTP_REQUEST_SESSION_H
