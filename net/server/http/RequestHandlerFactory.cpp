#include "CommonPrecomp.h"
#include "RequestHandlerFactory.h"
#include "RequestHandler.h"

request_handler_factory::~request_handler_factory()
{
}

request_handler_ptr request_handler_factory::create()
{
	request_handler_ptr request_handler_(new request_handler());
	request_handler_lst.push_back(request_handler_);
	return request_handler_;
}

void request_handler_factory::destroy(request_handler* handler)
{
	for (std::list<request_handler_ptr>::iterator iter = request_handler_lst.begin(); 
		iter != request_handler_lst.end(); iter++)
	{
		if (iter->get() == handler)
		{
			request_handler_lst.erase(iter);
			return;
		}
	}
}