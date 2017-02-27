#pragma once
#include "base_message.h"
using namespace std;

namespace hx_net
{
class Media_message:public base_message
{
public:
    Media_message(session_ptr pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo,
                  pDevicePropertyExPtr devProperty);
    ~Media_message(void);
public:
    int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
    int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
    bool IsStandardCommand();
    void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
	void GetAllCmd(CommandAttribute &cmdAll);
    //添加媒体数据新告警
    bool  add_new_alarm(string sPrgName,int alarmId,int nState,time_t  startTime);
    //添加媒体设备指标数据
    bool  add_new_data(string sIp,int nChannel,DevMonitorDataPtr &mapData);
protected:
    void record_alarm_and_notify(string &prgName,int nMod,CurItemAlarmInfo &curAlarm);
private:
    session_ptr         m_pSession;//关联连接对象
    DeviceInfo           &d_devInfo;//设备信息
    pDevicePropertyExPtr d_devProperty;
    boost::asio::deadline_timer     task_timeout_timer_;//控制任务执行定时器
    boost::recursive_mutex          alarm_mutex_;
    //<频率,<告警类型,告警信息> >
    map<string ,map<int,CurItemAlarmInfo > >  mapProgramAlarm;//节目告警信息
    map<int,time_t>                               tmLastSaveTime;//数据保存时间
};
}
