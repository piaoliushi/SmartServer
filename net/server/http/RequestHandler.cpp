#include "CommonPrecomp.h"
#include "RequestHandler.h"
#include "RequestHandlerFactory.h"
#include "../../SvcMgr.h"
using namespace hx_net;
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
        std::string response_body ="Hello,World!";
        connection->set_status(hx_http_server::connection::ok);
        common_headers[2].value = boost::lexical_cast<std::string>(response_body.size());
        connection->set_headers(boost::make_iterator_range(common_headers, common_headers + 3));
        connection->write(response_body, boost::bind(&request_handler::write_callback, this, _1));

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
    content_ += std::string(input.begin(), bytes_transferred);
	if (read_content_length_ < content_length_)
	{
		connection->read(boost::bind(&request_handler::read_callback, this, _1, _2, _3, _4));
		return;
	}

    if ("/" == uri_)
	{
        //string response_body,sUrl;
        cout<<content_.c_str()<<endl;
        cout<<"content_size:------"<<read_content_length_<<endl;
        if(bohui_protocol_.parseDataFromStr(content_,d_response_body,d_sUrl))
        {
            connection->set_status(hx_http_server::connection::ok);
            common_headers[2].value = boost::lexical_cast<std::string>(1);//response_body.size()
            connection->set_headers(boost::make_iterator_range(common_headers, common_headers + 3));
            string sResp=" ";
            connection->write(sResp, boost::bind(&request_handler::write_callback, this, _1));//response_body

        }
        else
            connection->write(bad_request, boost::bind(&request_handler::write_callback, this, _1));
	}
	else
	{
		connection->set_status(hx_http_server::connection::bad_request);
        connection->set_status(hx_http_server::connection::ok);
        common_headers[2].value = boost::lexical_cast<std::string>(bad_request.size());
		connection->set_headers(boost::make_iterator_range(common_headers, common_headers+3));
        connection->write(bad_request, boost::bind(&request_handler::write_callback, this, _1));
	}
}

void request_handler::write_callback(boost::system::error_code const & ec) 
{
    GetInst(SvcMgr).response_http_msg(d_sUrl,d_response_body);
    request_handler_factory::get_mutable_instance().destroy(shared_from_this());
}


hx_http_server::response_header request_handler::common_headers[3] = {
	{"Connection","close"}
	,{"Content-Type", "text/plain"}
	,{"Content-Length", "0"}
};

std::string request_handler::bad_request("Bad Request.");
