#ifndef _REQUEST_HANDLER_H_
#define _REQUEST_HANDLER_H_
#include"../../../protocol/bohui_protocol.h"
#include"CommonPrecomp.h"
#include <boost/network/protocol/http/server.hpp>
struct web_handler;
typedef boost::network::http::async_server<web_handler> hx_http_server;
typedef boost::shared_ptr<hx_http_server>  hx_http_server_ptr;

class request_handler
{
public:
	struct is_content_length {
		template <class Header>
		bool operator()(Header const & header) {
			return boost::iequals(header.name, "content-length");
		}
	};

	request_handler();
	~request_handler();

    void start(const hx_http_server::request &request, hx_http_server::connection_ptr connection);

	void read_callback(hx_http_server::connection::input_range input, boost::system::error_code const & ec, 
		                               std::size_t bytes_transferred, hx_http_server::connection_ptr connection);

	void write_callback(boost::system::error_code const & ec);

private:
	std::string uri_;
	std::string content_;
    //std::string response_body_;
	std::size_t content_length_;
	std::size_t read_content_length_;
    Bohui_Protocol  bohui_protocol_;

private:
	static hx_http_server::response_header common_headers[3];
	static std::string bad_request;
};

typedef boost::shared_ptr<request_handler> request_handler_ptr;


#endif // _REQUEST_HANDLER_H_
