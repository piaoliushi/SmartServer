#include "Electric_message.h"
#include "./104/iec104_types.h"
#include "./104/iec104.h"
namespace hx_net
{
    Electric_message::Electric_message(session_ptr  pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo)
		:m_pSession(pSession)
        ,d_devInfo(devInfo)
		,d_test_send_timer_(io_service)
		,d_interrogation_send_timer_(io_service)
		,m_Register(false)
		,d_stopdt(1)
		,d_testfr(1)
		,d_num_s(0)
		,d_num_r(0)
	{
	}

	Electric_message::~Electric_message(void)
	{
	}

    void Electric_message::SetProtocol(int mainprotocol,int subprotocol)
    {
        switch(mainprotocol){
        case EDA9033:
            break;
        case ELECTRIC:{
            switch (subprotocol)
            {
            case ELECTRIC_101:
                m_Register =true;
                break;
            case ELECTRIC_104:
                m_Register = false;
                break;
            }
        }
            break;
        }

        m_Subprotocol = subprotocol;
        m_mainprotocol =  mainprotocol;

    }

	bool Electric_message::isRegister()
	{
		return m_Register;
	}

    int Electric_message::check_msg_header(unsigned char *data,int nDataLen)
    {
        switch(m_mainprotocol){
        case EDA9033:{
            switch (m_Subprotocol)
            {
            case ZXJY_BACK:
            {
                if(data[0]==0x00)
                {
                    int nBodyLen = ((data[5]<<8)|(data[4]))*((data[7]<<8)|(data[6]));
                    if(nDataLen == nBodyLen+8)
                        return 0;
                }else
                    return -1;
            }
            }
        }
            break;
        case ELECTRIC:{
            switch (m_Subprotocol)
            {
            case ELECTRIC_104: {
                if(data[0]==START) {
                    int nBodyLen = data[1];
                    if(nBodyLen < IEC_APDU_MIN || nBodyLen > IEC_APDU_MAX)
                        return -1;
                    else
                        return nBodyLen;
                     }else
                        return -1;
                    }
             }
          }
            break;
        }
        return -1;
	}

	int Electric_message::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
        switch(m_mainprotocol){
        case EDA9033:
            break;
        case ELECTRIC:{
               switch (m_Subprotocol)
                {
                 case ELECTRIC_104:
                         return parse_104_data(data,data_ptr,nDataLen);
                }
             }
            break;
        }
        return -1;
	}

	bool Electric_message::IsStandardCommand()
	{
        switch(m_mainprotocol){
        case EDA9033:{
            switch (m_Subprotocol)
            {
            case Eda9003_F:
                return false;
            case ZXJY_BACK:
                return true;
            }
            return false;
        }
            break;
        case ELECTRIC:{
            switch (m_Subprotocol)
            {
            case ELECTRIC_104:
            case ELECTRIC_101:
                return true;
            }
        }
            break;
        }
        return false;
	}
	
	void Electric_message::getRegisterCommand(CommandUnit &cmdUnit)
	{
        switch(m_mainprotocol){
        case EDA9033:
            break;
        case ELECTRIC:{
            switch (m_Subprotocol)
            {
            case ELECTRIC_104:
                break;
            case ELECTRIC_101:
                return;
            }
        }
            break;
        }
    }


	int Electric_message::parse_104_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		//stop_test_send_timer();//停止发送测试帧
		//start_test_send_timer();//重新启动发送测试帧

		int ret = -1;
		struct iec_buf *buf = (struct iec_buf *) &data[0];

		unsigned char xx[128];
		memset(xx,0,nDataLen);
		memcpy(xx,data,nDataLen);
		switch (frame_type(&(buf->h))) 
		{
		case FRAME_TYPE_I:
			if (d_stopdt) 
				break;
			ret = iecsock_iframe_recv(buf, data_ptr);
			break;
		case FRAME_TYPE_S:
			ret = iecsock_sframe_recv(&(buf->h));
			break;
		case FRAME_TYPE_U:
			ret = iecsock_uframe_recv(&(buf->h));
			break;
		}
		return ret;
	}

	void Electric_message::start_test_send_timer(unsigned long nSeconds)
	{
		d_test_send_timer_.expires_from_now(boost::posix_time::seconds(nSeconds));
		d_test_send_timer_.async_wait(boost::bind(&Electric_message::test_time_event,
			this,boost::asio::placeholders::error));
	}

	void Electric_message::stop_test_send_timer()
	{
		m_test_send_count=0;
		d_test_send_timer_.cancel();
	}

	int Electric_message::start()
	{
		d_stopdt=1;
		iecsock_uframe_send(STARTACT);
		return 0;
	}


	int Electric_message::stop()
	{
		iecsock_uframe_send(STOPACT);
		return 0;
	}

	void Electric_message::test_time_event(const boost::system::error_code& error)
	{
		if(error!= boost::asio::error::operation_aborted)
		{
			if(m_test_send_count>10)
				iecsock_uframe_send(STOPACT);
			else
			{
				d_stopdt=1;
				m_test_send_count++;
				iecsock_uframe_send(TESTACT);
			}

			start_test_send_timer();
		}
	}

	int Electric_message::iecsock_iframe_recv(struct iec_buf *buf,DevMonitorDataPtr data_ptr)
	{

			struct iec_object obj[IEC_OBJECT_MAX];
			u_char cause, test, pn, *cp, t;
			int n;
			unsigned short caddr;
			size_t nDataLen = buf->length-IEC_APDU_MIN;
			//unsigned char *data = 
			int ret = iecasdu_parse(obj, &t, &caddr, &n, &cause, &test, &pn, 
				IOA_ADDRLEN, COM_ADDRLEN, buf->data-IEC_APCI_LEN, nDataLen);

			struct iec_unit_id *unitp = (struct iec_unit_id *) (&(buf->data));

			cout<<"t->"<<(int)t<<"--caddr->"<<(int)caddr<<"--n->"<<(int)n<<"--cause->"<<(int)cause<<"--test->"<<(int)test<<"--pn->"<<(int)pn<<endl;
			//接收序号与发送序号是否相同
			if(buf->h.ic.ns != d_num_r)
				return -1;
			d_num_r = (d_num_r + 1) % 32767;

			iecsock_sframe_send();

			//iecsock_run_ackw_queue(s, h->ic.nr);
//		s->va = h->ic.nr;
//		if (s->va == s->vs) {
// 			t1_timer_stop(s);
//		}
	//	t2_timer_stop(s);
		//t2_timer_start(s);

// 		if (!check_ns(s, h->ic.ns))
// 			return -1;
// 
// 		s->vr = (s->vr + 1) % 32767;
// 		if ((s->vr - s->va_peer + 32767) % 32767 == s->w) {
// 			iecsock_sframe_send(s);
// 			s->va_peer = s->vr;
// 		}

		return 0;
	}

	int Electric_message::iecsock_sframe_recv(struct iechdr *h)
	{
// 		struct iechdr *h;
// 		h = &buf->h;
// 		buf->data_len = h->length - 2;
		if(h->ic.nr != d_num_s)
			return -1;
		d_num_s = (d_num_s + 1) % 32767;
// 		if (!check_nr(s, h->ic.nr))
// 			return -1;
// 
// 		//iecsock_run_ackw_queue(s, h->ic.nr);
// 		s->va = h->ic.nr;
// 		if (s->va == s->vs) {
// 			t1_timer_stop(s);
// 			if (s->hooks.transmit_wakeup)
// 				s->hooks.transmit_wakeup(s);
// 			else if (default_hooks.transmit_wakeup)
// 				default_hooks.transmit_wakeup(s);
// 		}

		return 0;
	}

	int Electric_message::iecsock_uframe_recv(struct iechdr *h)
	{
		switch(uframe_func(h))
		{
	case STARTACT://主站不接受数据请求
			return -1;
	case STARTCON://主站置数据请求响应标志
		{
			iecsock_iframe_send();
			d_stopdt = 0;
		}
		break;
	case STOPACT://主站不接受停止请求
		return -1;
	case STOPCON://主站置停止请求响应标志
		d_stopdt = 1;
		break;
	case TESTACT:
		iecsock_uframe_send(TESTCON);
		d_testfr = 0;
		break;
	case TESTCON://主站接受从站test响应
		if (!d_testfr)
			return -1;
		d_testfr = 0;
		break;
		}

		return 0;
	}

	void Electric_message::iecsock_uframe_send(enum uframe_func func)
	{
		struct iec_buf buf;
		memset(&buf, 0, sizeof(struct iec_buf));
		buf.start = START;
		buf.length = sizeof(struct iec_u);
		buf.h.uc.ft = 3;
		if (func == STARTACT)
			buf.h.uc.start_act = 1;
		else if (func == STARTCON)
			buf.h.uc.start_con = 1;
		else if (func == STOPACT)
			buf.h.uc.stop_act = 1;
		else if (func == STOPCON)
			buf.h.uc.stop_con = 1;
		else if (func == TESTACT)
			buf.h.uc.test_act = 1;
		else if (func == TESTCON)
			buf.h.uc.test_con = 1;
		if(m_pSession!=NULL)
		{
			m_pSession->sendRawMessage((unsigned char*)(&buf),IEC_APCI_LEN);
		}
	}

	void Electric_message::iecsock_sframe_send()
	{
		struct iec_buf buf;
		memset(&buf, 0, sizeof(iec_buf));
		buf.start = START;
		buf.length = sizeof(struct iec_s);
		buf.h.sc.ft = 1;
		buf.h.sc.nr = d_num_r;	
		if(m_pSession!=NULL)
		{
			m_pSession->sendRawMessage((unsigned char*)(&buf),IEC_APCI_LEN);
		}
	}
	//总召唤发送
	void Electric_message::iecsock_iframe_send()
	{
		struct iec_buf buf;
		memset(&buf, 0, sizeof(buf));
		buf.start = START;
		buf.length =0x0e;
		buf.h.ic.nr = d_num_r;
		buf.h.ic.ns = d_num_s;	
		buf.asdu_h.type = INTERROGATION;
		buf.asdu_h.num =1;
		buf.asdu_h.cause = ACTIVATION;
		buf.asdu_h.pn = 0;
		buf.asdu_h.sq =0;
		buf.asdu_h.t =0;
		buf.asdu_h.ca=0x01;
		buf.asdu_h.oa=0;
		buf.asdu_h.sq=0x00;
		buf.data[3]=0x14;
		if(m_pSession!=NULL)
		{
			m_pSession->sendRawMessage((unsigned char*)(&buf),16);
		}

		d_num_s = (d_num_s + 1) % 32767;
	}
}
