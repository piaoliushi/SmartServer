#ifndef MESSAGE_H
#define MESSAGE_H
#pragma once

#include "MsgDefine.h" 
#include "./protocol/protocol.pb.h"
#include "share_ptr_object_define.h"
typedef std::vector<boost::uint8_t> data_buffer;
namespace hx_net
{
class net_session;
typedef boost::shared_ptr<hx_net::net_session>  session_ptr;
typedef boost::weak_ptr<hx_net::net_session>    session_weak_ptr;
class message
{
public:

    message(void);
    message(const message& msg);

    data_buffer& data();
		void   reset_head_size();
		void   reset_size(int msglen);
		size_t length()const;
		void   setsession(const session_ptr& _session);
		void   getsession(session_ptr& _session);

		boost::uint8_t* body();

		int   bodySize();

		size_t body_length() const;

		void   set_body_length(size_t length);

		bool   decode_header();

		bool   is_full_message();

		//解码google message
		bool   decode_google_message(googleMsgPtr msgPtr);

		bool   check_body(size_t bytes_transferred);

		//编码google message
		bool   encode_google_message(e_MsgType _type,googleMsgPtr gMsgPtr);
		
		message&  operator =(message &_msg);

	    packHeadPtr msg()const;

		//验证是否是flash 沙箱消息
		bool is_flash_check_message();

	private:
        data_buffer               data_;
        size_t                        body_length_;
        session_weak_ptr      session_;
		packHeadPtr              msg_;

		//flash 沙箱验证
		bool                     bflash_check_;
    };
    typedef boost::shared_ptr<hx_net::message> msgPointer;
}

#endif
