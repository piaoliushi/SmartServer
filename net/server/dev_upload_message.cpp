//#include "StdAfx.h"
#include "MsgDefine.h"
#include "dev_upload_message.h"
//#include "DevAgent.h"
#include "./net/client/MsgHandleAgent.h"
//设备上传消息，长度固定
namespace net
{
	dev_upload_message::dev_upload_message(int nSize)
		:data_(nSize)//该size设定为对应协议设定的缓冲区大小
		,body_length_(0)
		,w_pos_(0)
		,r_pos_(0)
	{

	}

	dev_upload_message::~dev_upload_message(void)
	{
	}

	//初始化该消息对应的发射机id
	void dev_upload_message::reset()
	{
		w_pos_=0;
		r_pos_=0;
	}
	//解码消息头
	bool dev_upload_message::decode_header()
	{
		DevUploadHeadPtr msg_ = (DevUploadHeadPtr)(&data_[0]);
		if(msg_->startCode != 0x7e)
			return false;
		if(msg_->msgBodyLen<=0 || msg_->msgBodyLen>MaxBodySize)
		{
			body_length_ =0;
			return false;
		}
		commandCode_ = msg_->commandCode;//保存当前命令码
		move_w_ptr(UPLOAD_MSG_HEAD_SIZE);//移动写指针
		body_length_ = msg_->msgBodyLen;
		data_.resize(UPLOAD_MSG_HEAD_SIZE+msg_->msgBodyLen);
		return true;
	}

	boost::uint8_t dev_upload_message::cur_commandType()
	{
		return commandCode_;
	}

	boost::uint8_t* dev_upload_message::body()
	{
		return &data_[UPLOAD_MSG_HEAD_SIZE];
	}

	boost::uint8_t* dev_upload_message::tailed()
	{
		return &data_[UPLOAD_MSG_HEAD_SIZE+body_length_-1];
	}

	int dev_upload_message::bodySize()
	{
		return body_length_;
	}

	vector<unsigned char>& dev_upload_message::get_msg()
	{
		return data_;
	}


	//解码消息
	int dev_upload_message::decode_msg(HMsgHandlePtr agentPtr,DevMonitorDataPtr data_ptr_,int msgLen)
	{
		move_w_ptr(msgLen);
		//int validLen = valid_msg_len();
		if(data_ptr_==NULL)
			return -1;
		int nResult = agentPtr->decode_msg_body(r_ptr(),data_ptr_,valid_msg_len());
		if(nResult==0)
		{
			if(data_ptr_.get()->Length<=0)
				return -1;
		}
		return 0;
	}

	int dev_upload_message::valid_msg_len()
	{
		return w_pos_-r_pos_;
	}


	void  dev_upload_message::set_length(int nlen)
	{
		data_.resize(nlen);
	}
	unsigned char* dev_upload_message::data()
	{
		return &data_[0];
	}

	void dev_upload_message::move_w_ptr(int nlen)
	{
		if((w_pos_+nlen)<=data_.size())
		{
			w_pos_=w_pos_+nlen;
		}
	}

	void dev_upload_message::move_r_ptr(int nlen)
	{
		if((r_pos_+nlen)<data_.size())
		{
			r_pos_=r_pos_+nlen;
		}
	}

	size_t dev_upload_message::space()
	{
		return data_.size()-w_pos_;
	}

	unsigned char* dev_upload_message::w_ptr()
	{
		return data()+ w_pos_;
	}

	unsigned char* dev_upload_message::r_ptr()
	{
		return data() +r_pos_;
	}

	
}

