#ifndef DEV_UPLOAD_MESSAGE_H
#define DEV_UPLOAD_MESSAGE_H
#pragma once
#include <vector>
#include "../../DataType.h"
using namespace std;
//class HDevAgent;
namespace net
{
	class dev_upload_message;
	typedef boost::shared_ptr<dev_upload_message> uploadMsgPtr;
    typedef boost::shared_ptr<std::vector<unsigned char> > unchar_ptr;
    //typedef boost::shared_ptr<HDevAgent>        HDevAgentPtr;
	class MsgHandleAgent;
	typedef boost::shared_ptr<MsgHandleAgent>        HMsgHandlePtr;
	class dev_upload_message //:public message
	{
	public:
		dev_upload_message(int nSize);
		~dev_upload_message(void);
	public:
		void reset();

		vector<unsigned char>& get_msg();

		boost::uint8_t* tailed();

		bool decode_header();

		boost::uint8_t* body();

		int   bodySize();

		int check_msg_body();

		int decode_msg(HMsgHandlePtr agentPtr,DevMonitorDataPtr data_ptr_,int msgLen);

		boost::uint8_t cur_commandType();



		int valid_msg_len();

		void  set_length(int nlen);
		size_t space();
		unsigned char* data();
		unsigned char* w_ptr();
		void move_w_ptr(int nlen);
		void move_r_ptr(int nlen);//add by lk2012-12-18
		unsigned char* r_ptr();

	private:
		vector<unsigned char>    data_;
		size_t                   body_length_;
		boost::uint8_t           commandCode_;
		size_t          w_pos_;
		size_t          r_pos_;
	};

}

#endif
