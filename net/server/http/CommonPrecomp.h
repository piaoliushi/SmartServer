#ifndef _COMMON_PRECOMP_H_
#define _COMMON_PRECOMP_H_

#include <boost/shared_ptr.hpp>
#include <boost/network/protocol/http/server.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/serialization/singleton.hpp>

namespace http = boost::network::http;
namespace utils = boost::network::utils;

struct test_handler;
typedef boost::network::http::async_server<test_handler> hx_http_server;
typedef boost::shared_ptr<hx_http_server>  hx_http_server_ptr;

#endif // _COMMON_PRECOMP_H_