#pragma once
#include <map>
//#include "device_message.h"
#include "../share_ptr_object_define.h"
#include "DataTypeDefine.h"
#include "../net_session.h"
//#include "device_session.h"

using namespace std;
class Snmp;
class CTarget;
namespace hx_net
{
    //class session;
	class MsgHandleAgentImpl;
	class MsgHandleAgent
	{
	public:
        MsgHandleAgent(session_ptr conPtr,boost::asio::io_service& io_service,DeviceInfo &devInfo);
		~MsgHandleAgent(void);
	public:
        bool Init(pDevicePropertyExPtr devProperty);
        bool Finit();
		int start();
		int stop();
		bool is_auto_run();
        int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
        int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
        int  decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target);
        //http消息解析
        int  decode_http_msg(const string &data,DevMonitorDataPtr data_ptr,CmdType cmdType,int number);

		void input_params(const vector<string> &vParam);
        int   PreHandleMsg();
		bool IsStandardCommand();
		void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
        void GetSignalCommand(int cmmType,int nIndex,CommandUnit &cmdUnit);
        void GetAllCmd(CommandAttribute &cmdAll);
        int   getChannelCount();
		bool isBelongChannel(int nChnnel,int monitorItemId);
		bool isMonitorChannel(int nChnnel,DevMonitorDataPtr curDataPtr);
		bool ItemValueIsAlarm(DevMonitorDataPtr curDataPtr,int monitorItemId,dev_alarm_state &curState);

		bool isRegister();
		void getRegisterCommand(CommandUnit &cmdUnit);

        int   cur_dev_state();
        void exec_task_now(int icmdType,string sUser,e_ErrorCode &eErrCode,int nChannel=0,
                           bool bSnmp=false,Snmp *snmp=NULL,CTarget *target=NULL);

        //执行通用指令
        void exec_general_task(int icmdType,string sUser,devCommdMsgPtr lpParam,
                                               e_ErrorCode &eErrCode);
        //执行联动命令
        void exec_action_task_now(map<int,vector<ActionParam> > &param,int actionType,string sUser,e_ErrorCode &eErrCode);


        void start_task_timeout_timer();
        //获得运行状态
        int get_run_state();
        //设置运行状态
        void reset_run_state();
        //添加新告警
        bool  add_new_alarm(string sPrgName,int alarmId,int nState,time_t  startTime);
        bool  add_new_data(string sIp,int nChannel,DevMonitorDataPtr &mapData);
	private:
		MsgHandleAgentImpl *m_msgImpl;
	};
    typedef boost::shared_ptr<hx_net::MsgHandleAgent>        HMsgHandlePtr;
}

