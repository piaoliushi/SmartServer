#pragma once
#include "MsgHandleAgentImpl.h"
#include "./dev_message/base_message.h"

namespace hx_net
{
	class MsgHandleAgentImpl
	{
	public:
        MsgHandleAgentImpl(session_ptr pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo);
		~MsgHandleAgentImpl(void);
	public:
        bool Init(pDevicePropertyExPtr devProperty);
		bool Finit();
		int start();
		int stop();
		bool is_auto_run();
		int PreHandleMsg();
        int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number);
        int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode);
        int  decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target);
        //http消息解析
        int  decode_http_msg(const string &data,DevMonitorDataPtr data_ptr,CmdType cmdType,int number);


		void input_params(const vector<string> &vParam);
		bool IsStandardCommand();
        void GetAllCmd(CommandAttribute &cmdAll);
		void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit);
        void GetSignalCommand(int cmmType,int nIndex,CommandUnit &cmdUnit);
        //int getChannelCount();
        //bool isBelongChannel(int nChnnel,int monitorItemId);
        //bool isMonitorChannel(int nChnnel,DevMonitorDataPtr curDataPtr);
        int getBelongChannelIdFromMonitorItem(int monitorItemId);

		bool ItemValueIsAlarm(DevMonitorDataPtr curDataPtr,int monitorItemId,dev_alarm_state &curState);

		bool isRegister();
		void getRegisterCommand(CommandUnit &cmdUnit);
        int  cur_dev_state();
        void exec_task_now(int icmdType,string sUser,e_ErrorCode &eErrCode,map<int,string> &mapParam,int nMode=0,
                           bool bSnmp=false,Snmp *snmp=NULL,CTarget *target=NULL);

        //执行通用指令
        void exec_general_task(int icmdType,string sUser,devCommdMsgPtr lpParam,
                                               e_ErrorCode &eErrCode);
        //执行联动命令
        void exec_action_task_now(map<int,vector<ActionParam> > &param,int actionType,string sUser,e_ErrorCode &eErrCode);

        void start_task_timeout_timer();
        //获得运行状态
        int  get_run_state();
        //设置运行状态
        void set_run_state(int curState);
        //复位运行状态
        void reset_run_state();

        bool dev_can_excute_cmd();
        //是否需要清除告警
        bool  is_need_clear_alarm();

        bool  device_run_detect_is_ok();
        //添加新告警
        bool  add_new_alarm(string sPrgName,int alarmId,int nState,time_t  startTime);
        bool  add_new_data(string sIp,int nChannel,DevMonitorDataPtr &mapData);

        //获取关联机信息
        void  get_relate_dev_info(string &sStationId,string &sDevId,string &sAttenaId);

        //是否是天线代理
        bool is_anttena_agent();
        //是否是软件一键开机
        bool is_soft_onekey_open();
        //是否是996一键开机
        bool is_996_onekey_open();

    public:
		base_message *m_pbaseMsg;
        session_ptr m_pSessionPtr;
		boost::asio::io_service    &m_io_service;
        DeviceInfo     &m_devInfo;
	};
}
