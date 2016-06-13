#ifndef _REQUEST_HANDLER_FACTORY_H_
#define _REQUEST_HANDLER_FACTORY_H_

#include "RequestHandler.h"
#include "../../client/device_message.h"
using namespace hx_net;
class request_handler_factory : public boost::serialization::singleton<request_handler_factory>
{
public:
	request_handler_ptr create();
    void destroy(request_handler_ptr handler);
	~request_handler_factory();
    //注册数据回调
    void register_data_callback(string  sIpAddress,HMsgHandlePtr pMsgHandle );
    HMsgHandlePtr   get_callback_by_ip(string sIpAddress);
    bool  add_new_alarm(string sIp,string sPrgName,int alarmId,int nState,time_t  startTime);
    bool add_new_data(string sIp,int nChannel,DevMonitorDataPtr &mapData);
protected:

private:
    boost::recursive_mutex            mutex_;
	std::list<request_handler_ptr> request_handler_lst;

    boost::recursive_mutex          callback_mutex_;
    map<string,HMsgHandlePtr>  map_callback_ptr_;
};


#endif // _REQUEST_HANDLER_FACTORY_H_
