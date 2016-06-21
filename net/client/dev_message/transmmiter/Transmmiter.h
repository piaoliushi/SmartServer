#pragma once
#ifndef TRANSMMITER_H
#define TRANSMMITER_H

#include "../../../../DataType.h"
#include "../../../message.h"
#include "../../../../StructDef.h"
#include"../../../../utility.h"

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
        virtual bool IsStandardCommand(){return false;}
        virtual void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit){}
        virtual void GetAllCmd(CommandAttribute &cmdAll){}
        virtual int decode_msg_body(DevMonitorDataPtr data_ptr,int nDataLen){return -1;}
        virtual bool isLastQueryCmd(){return true;}
        virtual bool isMultiQueryCmd(){return false;}
		
	};
}
#endif // TRANSMMITER_H
