#include "device_message.h"
#include "MsgHandleAgent.h"
namespace hx_net
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
	int othdev_message::check_normal_msg_header(HMsgHandlePtr agentPtr,int msgLen,CmdType cmdType,int number)
	{
         if(msgLen>0)
            move_w_ptr(msgLen);
        int nResult = agentPtr->check_msg_header(r_ptr(),valid_msg_len(),cmdType,number);
        if(nResult>0 && space()<nResult)
            data_.resize(data_.size()+nResult-space()+1);//重新扩充消息长度
		return nResult;
	}

    //用于非标准协议多指令解析

    int othdev_message::check_msg_header(HMsgHandlePtr agentPtr, int msgLen, CmdType cmdType, int number)
    {
        if(msgLen>0)
            move_w_ptr(msgLen);
        int nResult = agentPtr->check_msg_header(r_ptr(),valid_msg_len(),cmdType,number);
        if(nResult>0 && space()<nResult)
        {
            data_.resize(data_.size()+nResult-space()+1);//重新扩充消息长度
        }
        if(nResult>0){
            move_r_ptr(nResult);
        }
        return nResult;
    }

    int othdev_message::decode_msg(HMsgHandlePtr agentPtr, DevMonitorDataPtr data_ptr, int msgLen, int &iaddcode)
    {
        int nResult = agentPtr->decode_msg_body(r_ptr(),data_ptr,valid_msg_len(),iaddcode);
        reset();
        return nResult;
    }



    int othdev_message::decode_msg_body(HMsgHandlePtr agentPtr,DevMonitorDataPtr data_ptr,int msgLen,int &iaddcode)
	{
        if(msgLen>0)
            move_w_ptr(msgLen);
        int nResult = agentPtr->decode_msg_body(r_ptr(),data_ptr,valid_msg_len(),iaddcode);
        if(nResult<=0)
            reset();
		return nResult;
	}


	int othdev_message::valid_msg_len()
	{
		return w_pos_-r_pos_;
	}

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

