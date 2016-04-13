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

        virtual int  check_msg_header(unsigned char *data,int nDataLen){}
        virtual int  decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate){}
        virtual bool IsStandardCommand(){}
        virtual void GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit){}
        virtual void GetAllCmd(CommandAttribute &cmdAll){}
        //int kmp(const unsigned char *matcher, int mlen, const unsigned char *pattern, int plen);

        //unsigned short CRC16_A001(unsigned char * ptr, int len);
        //unsigned short CRC16_CCITT( unsigned char * ptr, int len );
    //protected:
      //  void kmp_init(const unsigned char *pattern, int pattern_size,int *pi);
		
	};
}
#endif // TRANSMMITER_H
