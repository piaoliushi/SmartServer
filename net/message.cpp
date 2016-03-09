#include "message.h"
#include "session.h"


namespace net
{
	message::message(void)
		:body_length_(0)
		,data_(HeadSize)
		,msg_((packHeadPtr)(&data_[0]))
	{
	}

	message::message(const message& msg)
	{
		data_ = msg.data_;
		body_length_ = msg.body_length_;
		msg_ = (packHeadPtr)(&data_[0]);
		session_ = msg.session_;
	}

	void message::reset_head_size()
	{
		data_.resize(HeadSize);
	}

	void message::reset_size(int msglen)
	{
		data_.resize(msglen);
	}

	data_buffer& message::data()
	{
		return data_;
	}

	size_t message::length()const
	{
		return HeadSize+body_length_;
	}

	void message::setsession(const session_ptr& _session)
	{
		session_ = _session;
	}
	void message::getsession(session_ptr& _session)
	{
		_session = session_.lock();
	}

	boost::uint8_t* message::body()
	{
		return &data_[HeadSize];
	}

	int message::bodySize()
	{
		return body_length_;
	}

	size_t message::body_length() const
	{
		return body_length_;
	}
	void message::set_body_length(size_t _length)
	{
		body_length_ = _length;
		data_.resize(HeadSize+_length);
		if(body_length_>MaxBodySize)
			body_length_=MaxBodySize;
	}

	bool message::decode_header()
	{
		//验证是否是flash 沙箱消息
		if(data_[0]=='<')
		{
			body_length_ = 7;
			bflash_check_ = true;
			data_.resize(23);
			return true;
		}
		msg_ = (packHeadPtr)(&data_[0]);
		if(msg_->packsize<HeadSize || msg_->packsize>HeadSize+MaxBodySize)
		{
			body_length_ =0;
			return false;
		}
		body_length_ = msg_->packsize-HeadSize;
		data_.resize(HeadSize+body_length_);//重新设定消息长度
		bflash_check_ = false;//非flash 沙箱消息
		return true;
	}

	bool message::is_full_message()
	{
		if(body_length_>0)
			return false;
		else
			return true;
	}
	
	//验证是否是flash 沙箱消息
	bool message::is_flash_check_message()
	{
		return bflash_check_;
	}


	bool  message::check_body(size_t bytes_transferred)
	{
		msg_ = (packHeadPtr)(&data_[0]);
		if(	msg_->packsize<HeadSize||
			bytes_transferred!=msg_->packsize-HeadSize)
		{
			body_length_=0;
			return false;
		}
		//body_length_ = msg_->packsize-HeadSize;
		return true;
	}

	bool  message::decode_google_message(googleMsgPtr msgPtr)
	{
		if(msgPtr)
			return msgPtr->ParseFromArray(&data_[HeadSize],body_length_);
		else
			return false;
	}

	bool  message::encode_google_message(e_MsgType _type,googleMsgPtr gMsgPtr)
	{
		//无需打包消息body
		if(!gMsgPtr)
		{
			msg_ = (packHeadPtr)(&data_[0]);
			msg_->type = _type;
			msg_->packsize = HeadSize;
			return true;
		}
		else
		{
			
			unsigned msg_size = gMsgPtr->ByteSize();
			data_.resize(HeadSize+msg_size);
			if(gMsgPtr->SerializeToArray(&data_[HeadSize],msg_size))
			{
				msg_ = (packHeadPtr)(&data_[0]);
				msg_->type = _type;
				msg_->packsize = msg_size+HeadSize;
				set_body_length(msg_size);
				return true;
			}
			else
				return false;
		}
		return false;
	}

	message& message::operator =(message( &_msg))
	{
		data_ = _msg.data_;
		body_length_ = _msg.body_length_;		
		msg_ = (packHeadPtr)(&data_[0]);
		session_ = _msg.session_;
		return (*this);
	}

	 packHeadPtr message::msg()const
	{
		return msg_;
	}
}

