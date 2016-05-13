#ifndef _REQUEST_HANDLER_FACTORY_H_
#define _REQUEST_HANDLER_FACTORY_H_

#include "RequestHandler.h"

class request_handler_factory : public boost::serialization::singleton<request_handler_factory>
{
public:
	request_handler_ptr create();
    void destroy(request_handler_ptr handler);//request_handler *
	~request_handler_factory();

private:
    boost::recursive_mutex            mutex_;
	std::list<request_handler_ptr> request_handler_lst;
};


#endif // _REQUEST_HANDLER_FACTORY_H_
