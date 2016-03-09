#include "CommonPrecomp.h"
#include "RequestHandler.h"
#include "RequestHandlerFactory.h"

request_handler::request_handler()
: uri_()
, content_()
, content_length_(0)
, read_content_length_(0)
{
	std::cout << "request_handler constructor!" << std::endl;
}

request_handler::~request_handler()
{
	std::cout << "request_handler destructor!" << std::endl;
}

void request_handler::start(const hx_http_server::request &request, hx_http_server::connection_ptr connection)
{
	if (request.method == "POST") 
	{
		hx_http_server::request::headers_container_type::iterator found =
			boost::find_if(request.headers, is_content_length());
		if (found == request.headers.end()) 
		{
			connection->set_status(hx_http_server::connection::bad_request);
			connection->set_headers(boost::make_iterator_range(common_headers, common_headers+3));
			connection->write(bad_request, boost::bind(&request_handler::write_callback, this, _1));
			return;
		}

		uri_ = http::destination(request);
		content_length_ = boost::lexical_cast<std::size_t>(found->value);

		connection->read(boost::bind(&request_handler::read_callback, this, _1, _2, _3, _4));
		return;
	} 
	else 
	{
		connection->set_status(hx_http_server::connection::bad_request);
		connection->set_headers(boost::make_iterator_range(common_headers, common_headers+3));
		connection->write(bad_request, boost::bind(&request_handler::write_callback, this, _1));
	}
}

void request_handler::read_callback(hx_http_server::connection::input_range input, boost::system::error_code const & ec, 
									                               std::size_t bytes_transferred, hx_http_server::connection_ptr connection)
{
	if (ec)
	{
		std::cerr << "Error: " << ec << std::endl;
		// todo others.
		return;
	}

	read_content_length_ += bytes_transferred;
	content_ += std::string(input.begin(), input.end());
	if (read_content_length_ < content_length_)
	{
		connection->read(boost::bind(&request_handler::read_callback, this, _1, _2, _3, _4));
		return;
	}

	if ("/videoProcessing/start" == uri_)
	{
		// todo: decode the body(content) by creating a message of application logic layer
		std::string response_body = "<Message Code=\"0000\" Descript=\"OK.\"></Message>";

		std::vector<hx_http_server::response_header> headers(common_headers, common_headers+3);
		headers[2].value = boost::lexical_cast<std::string>(response_body.size());
		connection->set_status(hx_http_server::connection::ok);
		connection->set_headers(boost::make_iterator_range(headers.begin(), headers.end()));
		connection->write(response_body, boost::bind(&request_handler::write_callback, this, _1));
	}
	else
	{
		connection->set_status(hx_http_server::connection::bad_request);
		connection->set_headers(boost::make_iterator_range(common_headers, common_headers+3));
		connection->write(bad_request, boost::bind(&request_handler::write_callback, this, _1));
	}
}

void request_handler::write_callback(boost::system::error_code const & ec) 
{
	request_handler_factory::get_mutable_instance().destroy(this);
}


hx_http_server::response_header request_handler::common_headers[3] = {
	{"Connection","close"}
	,{"Content-Type", "text/plain"}
	,{"Content-Length", "0"}
};

std::string request_handler::bad_request("Bad Request.");