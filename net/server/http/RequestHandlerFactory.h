#ifndef _REQUEST_HANDLER_FACTORY_H_
#define _REQUEST_HANDLER_FACTORY_H_

#include "RequestHandler.h"

class request_handler_factory : public boost::serialization::singleton<request_handler_factory>
{
public:
	request_handler_ptr create();
    void destroy(request_handler_ptr handler);
	~request_handler_factory();
    bool  add_new_alarm(string sIp,string sPrgName,int alarmId,int nState,time_t  startTime);
protected:
     void  record_alarm_and_notify(string &prgName,int nMod,CurItemAlarmInfo &curAlarm);
     void send_alarm_state_message(string sStationid,string sDevid,string sDevName,
                                            int nCellId,int devType,int  alarmState,
                                            string sStartTime,int alarmCount,string sReason);
     bool check_ip(string sIp,string &sDevId);
private:
    boost::recursive_mutex            mutex_;
	std::list<request_handler_ptr> request_handler_lst;

    boost::recursive_mutex          alarm_mutex_;
    map<string,map<string ,map<int,CurItemAlarmInfo > > > mapProgramAlarm;//节目告警信息
};


#endif // _REQUEST_HANDLER_FACTORY_H_
