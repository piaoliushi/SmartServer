#ifndef _COMMON_PRECOMP_H_
#define _COMMON_PRECOMP_H_



#include <boost/network/protocol/http/server.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/serialization/singleton.hpp>
#include <boost/shared_ptr.hpp>
//namespace http = boost::network::http;
//namespace utils = boost::network::utils;


//struct web_handler;
using boost::asio::ip::tcp;
//namespace net = boost::network;
namespace http = boost::network::http;
//namespace utils = boost::network::utils;

struct web_handler;
typedef boost::network::http::async_server<web_handler> hx_http_server;
typedef boost::shared_ptr<hx_http_server>  hx_http_server_ptr;

//typedef  boost::network::http::client   hx_http_client;
//typedef  boost::shared_ptr<hx_http_client> hx_http_client_ptr;

//#include "./RequestHandler.h"
//#include "./RequestHandlerFactory.h"






//#include <boost/network/protocol/http/client.hpp>





#endif // _COMMON_PRECOMP_H_
