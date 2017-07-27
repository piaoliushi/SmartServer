//#include "CommonPrecomp.h"
#include "RequestHandlerFactory.h"
#include "RequestHandler.h"
#include "../../../database/DataBaseOperation.h"
#include "../../config.h"
#include "../../share_ptr_object_define.h"
#include "../../SvcMgr.h"
using namespace db;
using namespace hx_net;
request_handler_factory::~request_handler_factory()
{
}

request_handler_ptr request_handler_factory::create()
{
    boost::recursive_mutex::scoped_lock llock(mutex_);
    request_handler_ptr request_handler_(new request_handler);
    request_handler_lst.push_back(request_handler_);
	return request_handler_;
}

void request_handler_factory::destroy(request_handler_ptr handler)
{
    boost::recursive_mutex::scoped_lock llock(mutex_);
    for (std::list<request_handler_ptr>::iterator iter = request_handler_lst.begin();
        iter != request_handler_lst.end(); iter++){
        if (*iter== handler)
        {
            request_handler_lst.erase(iter);
            return;
        }
    }
}

//注册数据回调
void request_handler_factory::register_data_callback(string  sIpAddress,HMsgHandlePtr pMsgHandle )
{
    boost::recursive_mutex::scoped_lock lock(callback_mutex_);
    if(map_callback_ptr_.find(sIpAddress) == map_callback_ptr_.end())
        map_callback_ptr_[sIpAddress] = pMsgHandle;

}

HMsgHandlePtr   request_handler_factory::get_callback_by_ip(string sIpAddress)
{
    boost::recursive_mutex::scoped_lock lock(callback_mutex_);
    if(map_callback_ptr_.find(sIpAddress)!=map_callback_ptr_.end())
        return map_callback_ptr_[sIpAddress];
    return HMsgHandlePtr();
}

 bool  request_handler_factory::add_new_alarm(string sIp,string sPrgName,int alarmId,int nState,time_t  startTime)
 {
    HMsgHandlePtr curPtr =  get_callback_by_ip(sIp);
    if(curPtr!=NULL)
        curPtr->add_new_alarm(sPrgName,alarmId,nState,startTime);
    return true;
 }

  bool request_handler_factory::add_new_data(string sIp,int nChannel,DevMonitorDataPtr &mapData)
  {
      HMsgHandlePtr curPtr =  get_callback_by_ip(sIp);
      if(curPtr!=NULL)
          curPtr->add_new_data(sIp,nChannel,mapData);
      return true;
  }
