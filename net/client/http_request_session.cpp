#include "http_request_session.h"
namespace hx_net {

http_request_session::http_request_session(boost::asio::io_service& io_service)
    :http_stream_(io_service)
{
}

 http_request_session::~http_request_session(void)
 {

 }

 //开始连接
 void http_request_session::open(std::string sUrl,std::string sData,std::string sRqstType)
 {
     urdl::option_set common_options;
     // Prevent HTTP redirections.
     common_options.set_option(urdl::http::max_redirects(0));
     http_stream_.set_option(urdl::http::request_method("POST"));
     http_stream_.set_option(urdl::http::request_content_type("text/plain"));
     http_stream_.set_option(urdl::http::request_content("Hello, world!"));
     http_stream_.set_options(common_options);
     http_stream_.async_open(sUrl, boost::bind(&http_request_session::open_handler,
                                               this,boost::asio::placeholders::error));
 }

 void http_request_session::open_handler(const boost::system::error_code& ec)
 {
   if (!ec)
   {
     // URL successfully opened.
     //  boost::array<char, 512> data;
     // boost::asio::async_read(http_stream_, boost::asio::buffer(data),boost::bind(&http_request_session::read_handler, this,
      //                                                                                          boost::asio::placeholders::error,
      //                                                                                          boost::asio::placeholders::bytes_transferred));

   }
   else
   {
     std::cerr << "Unable to open URL: ";
     std::cerr << ec.message() << std::endl;
   }

 }

 /*void http_request_session::read_handler(const boost::system::error_code& ec, std::size_t length)
 {

 }

 //上报http消息到上级平台(数据)
 void http_request_session::send_http_data_messge_to_platform(string sDevid,int  &dataPtr)
 {

 }*/

 //上报http消息到上级平台(数据)
 void http_request_session::send_http_data_messge_to_platform(string sDevid,devDataNfyMsgPtr &dataPtr)
 {

 }

}

