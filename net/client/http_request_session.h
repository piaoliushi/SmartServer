#ifndef HTTP_REQUEST_SESSION_H
#define HTTP_REQUEST_SESSION_H
#include "../net_session.h"
#include <urdl/read_stream.hpp>
namespace hx_net {
class http_request_session;
typedef boost::shared_ptr<http_request_session>  http_request_session_ptr;
typedef boost::weak_ptr<http_request_session>    http_request_session_weak_ptr;
class http_request_session
{
public:
    http_request_session(boost::asio::io_service& io_service);
    virtual ~http_request_session(void);
    //打开url
    void open(std::string sUrl,std::string sData="",std::string sRqstType="POST");

    //上报http消息到上级平台(数据)
    void send_http_data_messge_to_platform(string sDevid,devDataNfyMsgPtr &dataPtr);
    //上报http消息到上级平台(执行结果)
    void send_http_excute_result_messge_to_platform(string sDevid,devCommdRsltPtr &commdRsltPtr);
    //上报http消息到上级平台(告警)
    void send_http_alarm_messge_to_platform(string sDevid,devAlarmNfyMsgPtr &alarmPtr);
    //上报http消息到上级平台(连接状态)
    void send_http_data_messge_to_platform(string sDevid,devNetNfyMsgPtr &netPtr);
    //http连接发送handler
    void http_open_handler(const boost::system::error_code& ec);
    //http发送
    void send_http_message(string &sMessage);

protected:
    void  read_handler(const boost::system::error_code& ec, std::size_t length);
    void  open_handler(const boost::system::error_code& ec);
private:
    boost::recursive_mutex         http_stream_mutex_;//http上报对象互斥量
    urdl::read_stream http_stream_;
};

}


#endif // HTTP_REQUEST_SESSION_H
