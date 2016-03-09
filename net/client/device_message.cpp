#include "device_message.h"
#include "MsgHandleAgent.h"
namespace net
{
	othdev_message::othdev_message(int nSize)
		:data_(nSize)
		,w_pos_(0)
		,r_pos_(0)
	{
	}
	othdev_message::~othdev_message()
	{
	}

	void othdev_message::setsession(const session_ptr& _session)
	{
		session_ = _session;
	}
	void othdev_message::getsession(session_ptr& _session)
	{
		_session = session_.lock();
	}
	//检查通用消息头,解析库保存当前解析的命令id
	//如果：A）nResult=-1，解析头失败，B）>0返回为消息体长度
    /*int othdev_message::check_normal_msg_header(HDevAgentPtr agentPtr,int msgLen,CmdType cmdType,int number)
	{
		move_w_ptr(msgLen);
		bool bValidData=false;
		int nResult = agentPtr->HxCheckHeader(r_ptr(),valid_msg_len(),bValidData,cmdType,number);
		return nResult;
    }*/

	int othdev_message::check_normal_msg_header(HMsgHandlePtr agentPtr,int msgLen,CmdType cmdType,int number)
	{
		move_w_ptr(msgLen);
		int nResult = agentPtr->check_msg_header(r_ptr(),valid_msg_len());
		if(nResult>0)
			data_.resize(msgLen+nResult+1);//重新扩充消息长度
		return nResult;
	}

	int othdev_message::decode_msg_body(HMsgHandlePtr agentPtr,DevMonitorDataPtr data_ptr,int msgLen)
	{
		move_w_ptr(msgLen);
		int nResult = agentPtr->decode_msg_body(r_ptr(),data_ptr,valid_msg_len());
		
		reset();
		return nResult;
	}

    /*int othdev_message::decode_msg_body(HDevAgentPtr agentPtr,pTsmtAgentMsgPtr data_ptr_,int msgLen)
	{
		move_w_ptr(msgLen);
		reset();
		return -1;
	}

	int othdev_message::check_msg_header(HDevAgentPtr agentPtr,int msgLen,CmdType cmdType,int number)
	{
		bool bValid;
		move_w_ptr(msgLen);
		int nResult = agentPtr->HxCheckHeader(r_ptr(),valid_msg_len(),bValid,cmdType,number);
		if(nResult>0 && nResult<valid_msg_len())
		{
			if(bValid==false)//如果为无效数据，则不移动读指针
				move_r_ptr(nResult);
			else
			{
				return nResult;
			}
		}
		else if(nResult>0 && nResult==valid_msg_len())
		{
			reset();
		}
		return nResult;

    }*/

	int othdev_message::valid_msg_len()
	{
		return w_pos_-r_pos_;
	}

    /*bool othdev_message::decode_msg(HDevAgentPtr agentPtr,DevMonitorDataPtr data_ptr_)
	{
		agentPtr->HxPaseData(data_ptr_.get(),(LPBYTE)data(),w_pos_);
		if(data_ptr_->Length<=0)
			return false;
		else
		{
			reset();
		}
		return true;

		
    }*/

	size_t othdev_message::space()
	{
		size_t sSpaceSize = data_.size()-w_pos_;
		return sSpaceSize;
	}
	unsigned char* othdev_message::r_ptr()
	{
		return data() + r_pos_;
	}

	void othdev_message::move_r_ptr(int nlen)
	{
		if((r_pos_+nlen)<data_.size())
		{
			r_pos_=r_pos_+nlen;
		}
	}

	void othdev_message::move_w_ptr(int nlen)
	{
		if((w_pos_+nlen)<data_.size())
		{
			w_pos_=w_pos_+nlen;
		}
	}
}

