#ifndef TRANSMITTER_MESSAGE
#define TRANSMITTER_MESSAGE

#pragma once

#include <vector>
//#include "DevAgent.h"
//#include "../net_session.h"
#include "../../DataType.h"
#include "../net_session.h"
using namespace std;

namespace hx_net
{
	class othdev_message;
	typedef boost::shared_ptr<othdev_message> othdevMsgPtr;
//---------------------------------othdev----------------------------------------------//
	class MsgHandleAgent;
	typedef boost::shared_ptr<MsgHandleAgent>        HMsgHandlePtr;
//	typedef boost::shared_ptr<HDevAgent>                   HDevAgentPtr;
	typedef boost::shared_ptr<CommandAttribute>    CommandAttrPtr;
    //class session;
    //typedef boost::shared_ptr<net_session> session_ptr;
    class net_session;
    typedef boost::shared_ptr<net_session>  session_ptr;
    //typedef boost::weak_ptr<net_session>    session_weak_ptr;
	class othdev_message
	{
	public:
		othdev_message(int nSize);
		~othdev_message();
		public:
		void reset()
		{
			w_pos_=0;
			r_pos_=0;
		}

        void   setsession(const session_ptr& _session);
        void   getsession(session_ptr& _session);

        //int check_normal_msg_header(HDevAgentPtr agentPtr,int msgLen,CmdType cmdType=CMD_QUERY,int number=0);
		
        //int decode_msg_body(HDevAgentPtr agentPtr,pTsmtAgentMsgPtr data_ptr_,int msgLen);

        //int check_msg_header(HDevAgentPtr agentPtr,int msgLen,CmdType cmdType,int number);

		int check_normal_msg_header(HMsgHandlePtr agentPtr,int msgLen,CmdType cmdType,int number);

		int decode_msg_body(HMsgHandlePtr agentPtr,DevMonitorDataPtr data_ptr,int msgLen);

        //bool decode_msg(HDevAgentPtr agentPtr,DevMonitorDataPtr data_ptr_);

		void  set_length(int nlen){data_.resize(nlen);}

		size_t space();

		unsigned char* data(){return &(data_[0]);}

		unsigned char* w_ptr(){return data()+ w_pos_;}

		void move_w_ptr(int nlen);
		void move_r_ptr(int nlen);
		int  valid_msg_len();
        unsigned char* r_ptr();
	private:
		vector<unsigned char>    data_;
		size_t          w_pos_;
		size_t          r_pos_;
        boost::weak_ptr<net_session>    session_;
	};
}

#endif
