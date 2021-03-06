#pragma once
#include "../device_session.h"
#include "../device_message.h"
using namespace std;

namespace hx_net
{
	#define  Getbit(x,n)  ((x>>n) & 0x01)
	class base_message
	{
	public:
		base_message(void);
		virtual ~base_message(void);
	public:
        virtual void SetProtocol(int mainprotocol,int subprotocol){}
        virtual int check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number){return -1;}
        virtual int decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode){return -1;}
        virtual int decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target){return -1;}
        virtual int decode_http_msg(const string &data,DevMonitorDataPtr data_ptr,CmdType cmdType,int number){return -1;}
        virtual int PreHandleMsg(){return -1;}
        virtual void input_params(const vector<string> &vParam){}
		virtual bool IsStandardCommand(){return false;}
        virtual void GetAllCmd(CommandAttribute &cmdAll){}
        virtual void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit){}
        virtual void GetSignalCommand(int cmmType,int nIndex,CommandUnit &cmdUnit){}
		virtual bool isRegister(){return true;}
        virtual void getRegisterCommand(CommandUnit &cmdUnit){}
		virtual int start(){return -1;}
		virtual int stop(){return -1;}
		virtual	bool is_auto_run(){return false;}

        //virtual int  getChannelCount(){return 0;}
        //virtual bool isBelongChannel(int nChnnel,int monitorItemId){return false;}
        //virtual bool isMonitorChannel(int nChnnel,DevMonitorDataPtr curDataPtr){return true;}

        virtual int getBelongChannelIdFromMonitorItem(int monitorItemId){return 0;}

		virtual bool ItemValueIsAlarm(DevMonitorDataPtr curDataPtr,int monitorItemId,dev_alarm_state &curState){return false;}

        virtual int cur_dev_state(){return -1;}

        //执行命令
        virtual void exec_task_now(int icmdType,string sUser,e_ErrorCode &eErrCode,map<int,string> &mapParam,int nMode=0,
                                   bool bSnmp=false,Snmp *snmp=NULL,CTarget *target=NULL){}
        //执行通用命令
        virtual void exec_general_task(int icmdType,string sUser,devCommdMsgPtr lpParam,
                                       e_ErrorCode &eErrCode){}
        //执行联动命令
        virtual void exec_action_task_now(map<int,vector<ActionParam> > &param,int actionType,
                                          string sUser,e_ErrorCode &eErrCode){}

        virtual void start_task_timeout_timer(){}
        //获得运行状态
        virtual int  get_run_state(){return dev_unknown;}
        //设置运行状态
        virtual void set_run_state(int curState){}
        virtual void reset_run_state(){}

        virtual bool dev_can_excute_cmd(){return false;}

        //是否需要清除告警
        virtual bool  is_need_clear_alarm(){return false;}

        //设备运行状态是否准备好
        virtual bool device_run_detect_is_ok(){return true;}

        //添加新告警
        virtual bool  add_new_alarm(string sPrgName,int alarmId,int nState,time_t  startTime){return false;}
        virtual bool  add_new_data(string sIp,int nChannel,DevMonitorDataPtr &mapData){return false;}

        virtual void get_relate_dev_info(string &sStationId,string &sDevId,string &sAttenaId){}
        //是否是天线代理
        virtual bool is_anttena_agent(){return false;}
        //是否是软件一键开机
        virtual bool is_soft_onekey_open(){return false;}
        //是否是996一键开机
        virtual bool is_996_onekey_open(){return false;}
	public:
        static void Char2Hex(unsigned char ch, char* szHex);
		static void CharStr2HexStr(unsigned char const* pucCharStr, string &pszHexStr, int iSize);
	private:
        //HDevAgentPtr       dev_agent_;//协议解析和数据验证对象

	};
}
