#pragma once
#ifndef TRANSMMITER_H
#define TRANSMMITER_H

#include "../../../../DataType.h"
#include "../../../message.h"
#include "../../../../StructDef.h"
#include"../../../../utility.h"
#include "../../../../StationConfig.h"
#include <QString>
#include "./snmp_pp/snmp_pp.h"
#include "../../device_session.h"
const char LDPCQAM[][16]={"0.8&4QAMNR","0.4&4QAM","0.6&4QAM",
                          "0.8&4QAM","0.4&16QAM","0.6&16QAM",
                          "0.8&16QAM","0.8&32QAM","0.4&64QAM",
                          "0.6&64QAM","0.8&64QAM"};
//const char chPN[][32]={"",""};//{"420固定相位","420旋转相位","595",
//                       //"945固定相位","945旋转相位"};
//const char CPILOT[][32]={"",""};//{"单载波关导频","单载波开导频","多载波无导频"};
//const char ADPC[][32]={"",""};//{"关闭","开启","更新"};
const char ADPCCheckInfo[][32]={"OK","FEEDBACK LINK TOO LARGE","FEEDBACK LINK TOO SMALL",
                                "","POWER UNSTABLE","ADPC ERROR","IN_B TOO SMALL",
                                "TIMEOUT(15分钟)"};
const char AGCstate[][10]={"OFF","INT","A_IN","IN_A"};
//const char GPSState[][32]={"",""};//{"初始化","未知状态","正常","短路","开路"};

#define DTMB_CHPN_STR(Y)  GetInst(StationConfig).get_dictionary_value("s_sh_tsmt_chpn",Y)
#define DTMB_CPILOT_STR(Y)  GetInst(StationConfig).get_dictionary_value("s_sh_tsmt_cpilot",Y)
#define DTMB_BASE_STR(Y)  GetInst(StationConfig).get_dictionary_value("s_dtmb_base",Y)
#define DTMB_GPS_STR(Y)  GetInst(StationConfig).get_dictionary_value("s_sh_tsmt_gps_s",Y)
#define DTMB_RF_MOD_STR(Y)  GetInst(StationConfig).get_dictionary_value("s_sh_tsmt_rf_mod",Y)
#define DTMB_DTMB_MOD_STR(Y)  GetInst(StationConfig).get_dictionary_value("s_dtmb_mod",Y)
namespace hx_net
{
	class Transmmiter
	{
	public:
        Transmmiter();
        virtual ~Transmmiter();

        virtual int  check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number){return -1;}
        virtual int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,
                                     int nDataLen,int& runstate){return -1;}
        virtual int  decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target,int& runstate){return -1;}
        virtual int  decode_msg_body(const string &data,DevMonitorDataPtr data_ptr,CmdType cmdType,int number,
                                     int &runstate){return -1;}
        virtual bool IsStandardCommand(){return false;}
        virtual void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit){}
        virtual void GetSignalCommand(int cmmType,int nIndex,CommandUnit &cmdUnit){}
        virtual void GetSignalCommand(map<int,string> mapParam,CommandUnit &cmdUnit){}
        virtual void GetAllCmd(CommandAttribute &cmdAll){}
        virtual int  decode_msg_body(DevMonitorDataPtr data_ptr,int nDataLen){return -1;}
        virtual bool isLastQueryCmd(){return true;}
        virtual bool isMultiQueryCmd(){return false;}
        virtual bool exec_cmd(Snmp *snmp,int cmdtype,CTarget *target){return false;}
		
	};
}
#endif // TRANSMMITER_H
