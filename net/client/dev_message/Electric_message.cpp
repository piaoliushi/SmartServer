#include "./net/SvcMgr.h"
#include "Electric_message.h"
#include "./104/iec104_types.h"
#include "./104/iec104.h"
#include"../../../utility.h"
#include <QString>
namespace hx_net
{
Electric_message::Electric_message(session_ptr  pSession,boost::asio::io_service& io_service,DeviceInfo &devInfo)
    :base_message()
    ,d_devInfo(devInfo)
    ,d_test_send_timer_(io_service)
    ,d_interrogation_send_timer_(io_service)
    //,m_Register(false)
    ,d_stopdt(1)
    ,d_testfr(1)
    ,d_num_s(0)
    ,d_num_r(0)
    ,m_Uo(250)
    ,m_Io(5)
    ,m_Ubb(1.0)//电压比
    ,m_Ibb(20.0)//电流比
{
    m_pSession = boost::dynamic_pointer_cast<device_session>(pSession);

    if(d_devInfo.map_DevProperty.find("VRANG")!=d_devInfo.map_DevProperty.end())
        m_Uo = atoi(d_devInfo.map_DevProperty["VRANG"].property_value.c_str());
    if(d_devInfo.map_DevProperty.find("ARANG")!=d_devInfo.map_DevProperty.end())
        m_Io = atoi(d_devInfo.map_DevProperty["ARANG"].property_value.c_str());
    if(d_devInfo.map_DevProperty.find("VRATIO")!=d_devInfo.map_DevProperty.end())
        m_Ubb = atof(d_devInfo.map_DevProperty["VRATIO"].property_value.c_str());
    if(d_devInfo.map_DevProperty.find("ARATIO")!=d_devInfo.map_DevProperty.end())
        m_Ibb = atof(d_devInfo.map_DevProperty["ARATIO"].property_value.c_str());
    if(IsStandardCommand())
        d_curData_ptr.reset(new Data);
}

Electric_message::~Electric_message(void)
{
}

void Electric_message::SetProtocol(int mainprotocol,int subprotocol)
{
    switch(mainprotocol){
    case EDA9033:
        break;
    }

    m_Subprotocol = subprotocol;
    m_mainprotocol =  mainprotocol;

}

bool Electric_message::isRegister()
{
    return true;//m_Register;
}

int Electric_message::check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number)
{
    switch(d_devInfo.nDevProtocol){
    case EDA9033:{
        switch (d_devInfo.nSubProtocol)
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
        case Eda9033_A:
        {
            if(nDataLen<2)
                return -1;
            if(data[0] == 0x4C && data[1] == 0x57)
                return 0;
            else
            {
                unsigned char cDes[2]={0};
                cDes[0]=0x4C;
                cDes[1]=0x57;
                return kmp(data,nDataLen,cDes,2);
            }
        }
        case KSTAR_UPS:
        {
            if(nDataLen<3)
                return -1;
            if(data[0]==d_devInfo.iAddressCode && data[1]==0x04)
                return data[2]+2;
            else
                return -1;
        }
        case PAINUO_SPM33:
        {
            if(nDataLen<3)
                return -1;
            if(data[0]==d_devInfo.iAddressCode && data[1]==0x03)
                return data[2]+2;
            else
                return -1;
        }
        case YINGJIA_EM400:
        {
            if(nDataLen<3)
                return -1;
            if(data[0]==d_devInfo.iAddressCode && (data[1]==0x03 || data[1]==0x04
                                                   || data[1]==0x01 || data[1]==0x02))
                return data[2]+2;
            else
                return -1;
        }

        case ABB_104: {
            if(data[0]==START) {
                int nBodyLen = data[1];
                if(nBodyLen < IEC_APDU_MIN || nBodyLen > IEC_APDU_MAX)
                    return -1;
                else
                    return nBodyLen;
            }else
                return -1;
        }
        case HX_ELEC_CTR:{
            if(data[0]==0x55 && data[2]==0xF3)
            {
                if(data[1]< 0x99 )
                    return RE_SUCCESS;
            }
            else
            {
                unsigned char cDes[3]={0};
                cDes[0]=0x55;
                cDes[1] = 0x65;
                cDes[2] = 0xF3;
                return kmp(data,nDataLen,cDes,3);
            }
            break;
        }
        case HX_MD9703:
        {
            if(data[0]==0xAA && data[2]==0x97)
                return data[5]*256+data[6];
            else
                return -1;
        }
        case YISITE_EA66:
        {
            if(nDataLen < 3)
                return -1;
            if(data[0]==d_devInfo.iAddressCode && (data[1]==0x02 || data[1]==0x04))
                return data[2]+2;
            else
                return -1;
        }
        case YISITE_EA800:
        {
            if(data[0]==d_devInfo.iAddressCode && data[1]==0x04)
                return data[2]+2;
            else
                return -1;
        }
        case ACR_NET_EM:
        case ACR_PZ:
        case ACR_ARD_2L:{
            if(data[0]==d_devInfo.iAddressCode && data[1]==0x03)
                return data[2]+2;
            else
                return -1;
        }
        case YISITE_EA800II:
        case ST_UPS_C6_20:{
            if(data[0] == 0x28)
                return 0;
            else
            {
                unsigned char cDes[1]={0};
                cDes[0]=0x28;
                return kmp(data,nDataLen,cDes,1);
            }
        }
        case DSE_7320_POWER:
        {
            if(data[0]==d_devInfo.iAddressCode && data[1]==0x03)
                return data[2]+2;
            else
                return -1;
        }
        case SF_3P_MANOSTAT:
        {
            if(data[0]==0x7E && data[1]==0x33 && data[2]==0x32)
            {
                return AsciiToInt(data[10])*256+AsciiToInt(data[11])*16+AsciiToInt(data[12])+5;
            }
            else
                return -1;
        }
        case SF_DJ_UPS:
        {
            if(data[0] == 0x21)
                return 0;
            else
            {
                unsigned char cDes[1]={0};
                cDes[0]=0x21;
                return kmp(data,nDataLen,cDes,1);
            }
        }
        }
    }
        break;
    }
    return -1;
}

int Electric_message::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode)
{
    if(data_ptr!=NULL)
        d_curData_ptr = data_ptr;
    switch(d_devInfo.nDevProtocol){
    case EDA9033:
        switch (d_devInfo.nSubProtocol)
        {
            case Eda9033_A:
                return decode_Eda9033A(data,d_curData_ptr,nDataLen,iaddcode);
            case PAINUO_SPM33://标准head/body
            {
                int iresult = decode_SPM33(data,d_curData_ptr,nDataLen,iaddcode);
                GetResultData(d_curData_ptr);
                m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                return iresult;
            }
            case YINGJIA_EM400://标准head/body
            {
                int iresult = decode_EM400(data,d_curData_ptr,nDataLen,iaddcode);
                GetResultData(d_curData_ptr);
                m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                return iresult;
            }
            case KSTAR_UPS://标准head/body
            {
                int iresult = decode_KSTUPS(data,d_curData_ptr,nDataLen,iaddcode);
                GetResultData(d_curData_ptr);
                m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                return iresult;
            }
            case ABB_104://标准head/body
            {
                int iresult = parse_104_data(data,d_curData_ptr,nDataLen,iaddcode);
                //m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                return iresult;
            }
            case YISITE_EA66:
            {
                int iresult = decode_EA66(data,d_curData_ptr,nDataLen,iaddcode);
                GetResultData(d_curData_ptr);
                m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                return iresult;
            }
            case YISITE_EA800://decode_EA800
            {
                int iresult = decode_EA800(data,d_curData_ptr,nDataLen,iaddcode);
                GetResultData(d_curData_ptr);
                m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                return iresult;
            }
            case ACR_NET_EM:{
                int iresult = decode_AcrNetEm(data,d_curData_ptr,nDataLen,iaddcode);
                GetResultData(d_curData_ptr);
                m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                return iresult;
            }
            case HX_ELEC_CTR:
            {
                int iresult = decode_Elecctr(data,d_curData_ptr,nDataLen,iaddcode);
                m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                return iresult;
            }
            case ACR_PZ:{
                int iresult = decode_Acr_PZ(data,d_curData_ptr,nDataLen,iaddcode);
                m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                return iresult;
            }
            case ACR_ARD_2L:
            {
                int iresult = decode_Acr_Ard2L(data,d_curData_ptr,nDataLen,iaddcode);
                m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                return iresult;
            }
            case ST_UPS_C6_20:
                 return decode_ST_C6_20Ks(data,d_curData_ptr,nDataLen,iaddcode);
            case DSE_7320_POWER:
            {
                int iresult = decode_DSE_7320(data,d_curData_ptr,nDataLen,iaddcode);
                m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                return iresult;
            }
            case HX_MD9703:
            {
                int iresult = decode_hx9703(data,d_curData_ptr,nDataLen,iaddcode);
                m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                return iresult;
            }
            case SF_3P_MANOSTAT:
            {
                int iresult = decode_Sf3pmanostat(data,d_curData_ptr,nDataLen,iaddcode);
                m_pSession->start_handler_data(iaddcode,d_curData_ptr);
                return iresult;
            }
            case SF_DJ_UPS:
                return decode_Sfdjups(data,d_curData_ptr,nDataLen,iaddcode);
            case YISITE_EA800II:
                return decode_EA800II(data,d_curData_ptr,nDataLen,iaddcode);

        }
        break;
    }
    return -1;
}

bool Electric_message::IsStandardCommand()
{
    switch(d_devInfo.nDevProtocol){
    case EDA9033:{
        switch (d_devInfo.nSubProtocol)
        {
        case Eda9003_F:
            return false;
        case ZXJY_BACK:
            return true;
        case Eda9033_A:
            return false;
        case KSTAR_UPS:
        case PAINUO_SPM33:
        case YINGJIA_EM400:
        case YISITE_EA66:
        case ACR_NET_EM:
        case ACR_PZ:
        case ACR_ARD_2L:
        case DSE_7320_POWER:
        case YISITE_EA800:
        case HX_MD9703:
        case SF_3P_MANOSTAT:
            return true;
        case ABB_104:
            return true;
        }
        return false;
    }
        break;
    }
    return false;
}

void Electric_message::getRegisterCommand(CommandUnit &cmdUnit)
{
    switch(d_devInfo.nDevProtocol){
    case EDA9033:
        break;
    }
}

int Electric_message::parse_104_data(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode)
{

    int ret = -1;
    struct iec_buf *buf = (struct iec_buf *) &data[0];

    iaddcode = d_devInfo.iAddressCode;
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

    //cout<<"t->"<<(int)t<<"--caddr->"<<(int)caddr<<"--n->"<<(int)n<<"--cause->"<<(int)cause<<"--test->"<<(int)test<<"--pn->"<<(int)pn<<endl;
    //接收序号与发送序号是否相同
    if(buf->h.ic.ns != d_num_r)
        return -1;
    d_num_r = (d_num_r + 1) % 32767;

    iecsock_sframe_send();


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

void Electric_message::GetAllCmd( CommandAttribute &cmdAll )
{
    switch(d_devInfo.nDevProtocol){
    case EDA9033:
    {
        switch(d_devInfo.nSubProtocol)
        {
        case Eda9033_A:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 45;
            tmUnit.commandLen = 7;
            tmUnit.commandId[0] = 0x4C;
            tmUnit.commandId[1] = 0x57;
            tmUnit.commandId[2] = d_devInfo.iAddressCode;
            tmUnit.commandId[3] = 0x30;
            tmUnit.commandId[4] = 0x0E;
            tmUnit.commandId[5] = tmUnit.commandId[2]+tmUnit.commandId[3]+tmUnit.commandId[4];
            tmUnit.commandId[6] = 0x0D;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case PAINUO_SPM33:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 3;
            tmUnit.commandLen = 8;
            tmUnit.commandId[0] = d_devInfo.iAddressCode;
            tmUnit.commandId[1] = 0x03;
            tmUnit.commandId[2] = 0x00;
            tmUnit.commandId[3] = 0x00;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x33;
            unsigned short uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case YINGJIA_EM400:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 3;
            tmUnit.commandLen = 8;
            tmUnit.commandId[0] = d_devInfo.iAddressCode;
            tmUnit.commandId[1] = 0x03;
            tmUnit.commandId[2] = 0x07;
            tmUnit.commandId[3] = 0xD0;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x19;
            unsigned short uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[1] = 0x04;
            tmUnit.commandId[2] = 0x00;
            tmUnit.commandId[3] = 0x77;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x0C;
            uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[1] = 0x01;
            tmUnit.commandId[2] = 0x00;
            tmUnit.commandId[3] = 0x0A;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x02;
            uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[1] = 0x02;
            tmUnit.commandId[2] = 0x00;
            tmUnit.commandId[3] = 0x64;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x04;
            uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case KSTAR_UPS:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 3;
            tmUnit.commandLen = 8;
            tmUnit.commandId[0] = d_devInfo.iAddressCode;
            tmUnit.commandId[1] = 0x04;
            tmUnit.commandId[2] = 0x75;
            tmUnit.commandId[3] = 0x30;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x29;
            unsigned short uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case ABB_104:{
            CommandUnit tmUnit;
            tmUnit.ackLen = 2;//IEC_APCI_LEN
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case YISITE_EA66:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 3;
            tmUnit.commandLen = 8;
            tmUnit.commandId[0] = d_devInfo.iAddressCode;
            tmUnit.commandId[1] = 0x04;
            tmUnit.commandId[2] = 0x00;
            tmUnit.commandId[3] = 0x01;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x2D;
            unsigned short uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[1] = 0x02;
            tmUnit.commandId[3] = 0x00;
            tmUnit.commandId[5] = 0x3E;
            uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case YISITE_EA800:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 3;
            tmUnit.commandLen = 8;
            tmUnit.commandId[0] = d_devInfo.iAddressCode;
            tmUnit.commandId[1] = 0x04;
            tmUnit.commandId[2] = 0x00;
            tmUnit.commandId[3] = 0x02;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x44;
            unsigned short uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
        case ACR_NET_EM:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 3;
            tmUnit.commandLen = 8;
            tmUnit.commandId[0] = d_devInfo.iAddressCode;
            tmUnit.commandId[1] = 0x03;
            tmUnit.commandId[2] = 0x00;
            tmUnit.commandId[3] = 0x22;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x1D;
            unsigned short uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case ACR_PZ:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 3;
            tmUnit.commandLen = 8;
            tmUnit.commandId[0] = d_devInfo.iAddressCode;
            tmUnit.commandId[1] = 0x03;
            tmUnit.commandId[2] = 0x00;
            tmUnit.commandId[3] = 0x00;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x04;
            unsigned short uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case ACR_ARD_2L:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 3;
            tmUnit.commandLen = 8;
            tmUnit.commandId[0] = d_devInfo.iAddressCode;
            tmUnit.commandId[1] = 0x03;
            tmUnit.commandId[2] = 0x00;
            tmUnit.commandId[3] = 0x00;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x09;
            unsigned short uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case HX_ELEC_CTR:{
            CommandUnit tmUnit;
            tmUnit.commandLen = 5;

            tmUnit.ackLen = 104;
            tmUnit.commandId[0]=0x55;
            tmUnit.commandId[1]=0x02;
            tmUnit.commandId[2]=0xF3;
            tmUnit.commandId[3]=0xF3;
            tmUnit.commandId[4]=0x00;
            vector<CommandUnit> vtUnit;
            vtUnit.push_back(tmUnit);
            cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
        }
            break;
        case ST_UPS_C6_20:{
            CommandUnit tmUnit;
            tmUnit.commandLen = 3;

            tmUnit.ackLen = 110;
            tmUnit.commandId[0]=0x51;
            tmUnit.commandId[1]=0x36;
            tmUnit.commandId[2]=0x0D;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.ackLen = 80;
            tmUnit.commandId[0]=0x57;
            tmUnit.commandId[1]=0x41;
            tmUnit.commandId[2]=0x0D;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);

            tmUnit.commandLen = 4;
            tmUnit.ackLen = 0;
            tmUnit.commandId[0]=0x53;
            tmUnit.commandId[1]=0x2E;
            tmUnit.commandId[2]=0x32;
            tmUnit.commandId[3]=0x0D;
            cmdAll.mapCommand[MSG_DEV_TURNOFF_OPR].push_back(tmUnit);

        }
            break;
        case DSE_7320_POWER:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 3;
            tmUnit.commandLen = 8;
            tmUnit.commandId[0] = d_devInfo.iAddressCode;
            tmUnit.commandId[1] = 0x03;
            tmUnit.commandId[2] = 0x04;
            tmUnit.commandId[3] = 0x07;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x1C;
            unsigned short uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.commandId[2] = 0x06;
            tmUnit.commandId[3] = 0x00;
            tmUnit.commandId[5] = 0x18;
            uscrc = CRC16_A001(tmUnit.commandId,6);
            tmUnit.commandId[6] = (uscrc&0x00FF);
            tmUnit.commandId[7] = ((uscrc & 0xFF00)>>8);
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case HX_MD9703:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 7;
            tmUnit.commandLen = 13;
            tmUnit.commandId[0] = 0xAA;
            tmUnit.commandId[1] = 0x97;
            tmUnit.commandId[2] = 0x11;
            tmUnit.commandId[3] = 0x00;
            tmUnit.commandId[4] = 0x00;
            tmUnit.commandId[5] = 0x00;

            tmUnit.commandId[6] = 0x06;
            tmUnit.commandId[7] = 0x00;
            tmUnit.commandId[8] = 0x00;
            tmUnit.commandId[9] = 0x00;
            tmUnit.commandId[10] = 0x00;
            tmUnit.commandId[11] = 0x00;
            tmUnit.commandId[12] = 0x55;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case SF_3P_MANOSTAT:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 13;
            tmUnit.commandLen = 20;
            tmUnit.commandId[0] = 0x7E;
            tmUnit.commandId[1] = 0x33;
            tmUnit.commandId[2] = 0x32;

            unsigned char chadd[10]={'0','0','0','0','0','0','0','0','0','\0'};

            IntToAscii(d_devInfo.iAddressCode,&chadd[0]);
            tmUnit.commandId[3] = chadd[1];
            tmUnit.commandId[4] = chadd[0];

            tmUnit.commandId[5] = 0x32;
            tmUnit.commandId[6] = 0x36;
            tmUnit.commandId[7] = 0x34;
            tmUnit.commandId[8] = 0x32;
            tmUnit.commandId[9] = 0x45;
            tmUnit.commandId[10] = 0x30;
            tmUnit.commandId[11] = 0x30;
            tmUnit.commandId[12] = 0x32;
            tmUnit.commandId[13] = 0x30;
            tmUnit.commandId[14] = 0x30;
            int icheck = SF_checkSum(&tmUnit.commandId[1],14);
            unsigned char chsum[10]={'0','0','0','0','0','0','0','0','0','\0'};
            IntToAscii(icheck,&chsum[0]);
            tmUnit.commandId[15] = chsum[3];
            tmUnit.commandId[16] = chsum[2];
            tmUnit.commandId[17] = chsum[1];
            tmUnit.commandId[18] = chsum[0];
            tmUnit.commandId[19] = 0x0D;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
        }
            break;
        case SF_DJ_UPS:
        {
            CommandUnit tmUnit;
            tmUnit.ackLen = 41;
            tmUnit.commandLen = 3;
            tmUnit.commandId[0] = 0x47;
            tmUnit.commandId[1] = 0x31;
            tmUnit.commandId[2] = 0x0D;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
            tmUnit.ackLen = 73;
            tmUnit.commandId[1] = 0x33;
            cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);

            tmUnit.commandLen = 4;
            tmUnit.ackLen = 0;
            tmUnit.commandId[0]=0x53;
            tmUnit.commandId[1]=0x2E;
            tmUnit.commandId[2]=0x32;
            tmUnit.commandId[3]=0x0D;
            cmdAll.mapCommand[MSG_DEV_TURNOFF_OPR].push_back(tmUnit);
        }
            break;
        case YISITE_EA800II:
               {
                   CommandUnit tmUnit;
                   tmUnit.commandLen = 3;

                   tmUnit.ackLen = 47;
                   tmUnit.commandId[0]=0x51;
                   tmUnit.commandId[1]=0x36;
                   tmUnit.commandId[2]=0x0D;
                   cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
                   tmUnit.commandLen = 4;
                   tmUnit.ackLen = 0;
                   tmUnit.commandId[0]=0x53;
                   tmUnit.commandId[1]=0x2E;
                   tmUnit.commandId[2]=0x32;
                   tmUnit.commandId[3]=0x0D;
                   cmdAll.mapCommand[MSG_DEV_TURNOFF_OPR].push_back(tmUnit);
               }
            break;
        }
    }
        break;
    }
}

void Electric_message::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
{
    switch (d_devInfo.nSubProtocol) {
    case HX_ELEC_CTR:
    {
        if(lpParam->cparams_size()<0)
            break;
        cmdUnit.commandLen = 7;
        cmdUnit.ackLen = 0;
        cmdUnit.commandId[0]=0x55;
        cmdUnit.commandId[1]=0x03;
        cmdUnit.commandId[2]=0xF2;
        cmdUnit.commandId[3]=atoi(lpParam->cparams(0).sparamvalue().c_str());
        cmdUnit.commandId[4]=0x01;
        cmdUnit.commandId[5]=cmdUnit.commandId[2]^cmdUnit.commandId[3]^cmdUnit.commandId[4];
        cmdUnit.commandId[6]=0x00;
    }
        break;
    case ST_UPS_C6_20:{
        cmdUnit.commandLen = 4;
        cmdUnit.ackLen = 0;

        cmdUnit.commandId[0]=0x53;
        cmdUnit.commandId[1]=0x2E;
        cmdUnit.commandId[2]=0x32;
        cmdUnit.commandId[3]=0x0D;
    }
        break;
    case HX_MD9703:
    {
        if(lpParam->cparams_size()==2)
        {
            cmdUnit.commandLen = 13;
            cmdUnit.ackLen = 0;
            cmdUnit.commandId[0] = 0xAA;
            cmdUnit.commandId[1] = 0x97;
            cmdUnit.commandId[2] = 0x22;
            cmdUnit.commandId[3] = 0x00;
            cmdUnit.commandId[4] = 0x00;
            cmdUnit.commandId[5] = 0x00;
            cmdUnit.commandId[6] = 0x06;
            cmdUnit.commandId[7] =atoi(lpParam->cparams(0).sparamvalue().c_str());
            cmdUnit.commandId[8] = atoi(lpParam->cparams(1).sparamvalue().c_str());
            cmdUnit.commandId[9] = 0x00;
            cmdUnit.commandId[10] = 0x00;
            cmdUnit.commandId[11] = cmdUnit.commandId[7]+cmdUnit.commandId[8];
            cmdUnit.commandId[12] = 0x55;
        }
    }
        break;
    default:
        break;
    }
}

void Electric_message::GetSignalCommand(int nChannel, CommandUnit &cmdUnit)
{
    switch (d_devInfo.nSubProtocol) {
    case HX_ELEC_CTR:
    {
        cmdUnit.commandLen = 7;
        cmdUnit.ackLen = 0;
        cmdUnit.commandId[0]=0x55;
        cmdUnit.commandId[1]=0x03;
        cmdUnit.commandId[2]=0xF2;
        cmdUnit.commandId[3]=nChannel;
        cmdUnit.commandId[4]=0x01;
        cmdUnit.commandId[5]=cmdUnit.commandId[2]^cmdUnit.commandId[3]^cmdUnit.commandId[4];
        cmdUnit.commandId[6]=0x00;
    }
        break;
    case ST_UPS_C6_20:{
        cmdUnit.commandLen = 4;
        cmdUnit.ackLen = 0;

        cmdUnit.commandId[0]=0x53;
        cmdUnit.commandId[1]=0x2E;
        cmdUnit.commandId[2]=0x32;
        cmdUnit.commandId[3]=0x0D;
    }
        break;
    default:
        break;
    }
}

void Electric_message::exec_task_now(int icmdType, string sUser, e_ErrorCode &eErrCode,map<int,string> &mapParam,
                                     bool bSnmp, Snmp *snmp, CTarget *target)
{
    CommandUnit cmdUnit;
    cmdUnit.commandLen=0;
    if(mapParam.size()>0){
        int nChannel = atoi(mapParam[0].c_str());
        GetSignalCommand(nChannel,cmdUnit);
    }
    if(cmdUnit.commandLen>0)
    {
        eErrCode = EC_OK;
        m_pSession->send_cmd_to_dev(cmdUnit,eErrCode);
        m_pSession->set_opr_state(d_devInfo.sDevNum,dev_no_opr);
    }
}

void Electric_message::exec_general_task(int icmdType, string sUser, devCommdMsgPtr lpParam, e_ErrorCode &eErrCode)
{
    CommandUnit cmdUnit;
    cmdUnit.commandLen=0;
    GetSignalCommand(lpParam,cmdUnit);
    if(cmdUnit.commandLen>0)
    {
        eErrCode = EC_OK;
        m_pSession->send_cmd_to_dev(cmdUnit,eErrCode);
    }
}

bool Electric_message::is_auto_run()
{

    switch(d_devInfo.nDevProtocol){
        case EDA9033:{
            switch(d_devInfo.nSubProtocol)
            {
            case ABB_104:
                return true;
            }
        }
    }

    return false;
}

void Electric_message::GetResultData(DevMonitorDataPtr data_ptr)
{
    map<int,DeviceMonitorItem>::iterator iter = d_devInfo.map_MonitorItem.begin();
    for(;iter!=d_devInfo.map_MonitorItem.end();++iter)
    {
        map<int,DataInfo>::iterator diter = data_ptr->mValues.find((*iter).first);
        if(diter!=data_ptr->mValues.end())
        {

            if((*iter).second.iItemType == 0){
                data_ptr->mValues[(*iter).first].fValue *= (*iter).second.dRatio;
                if(data_ptr->mValues[(*iter).first].sValue.empty())
                    data_ptr->mValues[(*iter).first].sValue = QString::number(data_ptr->mValues[(*iter).first].fValue,'g',2).toStdString();
            }
            else {
                if((*iter).second.dRatio==0)
                    data_ptr->mValues[(*iter).first].fValue = data_ptr->mValues[(*iter).first].fValue==1.0f ? 0:1;
            }
        }
    }
}

int Electric_message::decode_Eda9033A( unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int &iaddcode )
{
    int indexpos =0;
    iaddcode = data[2];
    DataInfo dainfo;
    //A相电压、电流
    dainfo.bType = false;
    dainfo.fValue = (float)((data[5]<<8)|data[6])/10000*m_Uo*m_Ubb;
    data_ptr->mValues[indexpos++] = dainfo;
    dainfo.fValue = (float)((data[7]<<8)|data[8])/10000*m_Io*m_Ibb;
    data_ptr->mValues[indexpos++] = dainfo;
    //B相电压、电流
    dainfo.fValue = (float)((data[9]<<8)|data[10])/10000*m_Uo*m_Ubb;
    data_ptr->mValues[indexpos++] = dainfo;
    dainfo.fValue = (float)((data[11]<<8)|data[12])/10000*m_Io*m_Ibb;
    data_ptr->mValues[indexpos++] = dainfo;
    //C相电压、电流
    dainfo.fValue = (float)((data[13]<<8)|data[14])/10000*m_Uo*m_Ubb;
    data_ptr->mValues[indexpos++] = dainfo;
    dainfo.fValue = (float)((data[15]<<8)|data[16])/10000*m_Io*m_Ibb;
    data_ptr->mValues[indexpos++] = dainfo;
    //总有功功率值
    dainfo.fValue = (float)(((data[17]&0x7F)<<8)|data[18])/10000*3*m_Uo*m_Ubb*m_Io*m_Ibb;
    data_ptr->mValues[indexpos++] = dainfo;
    //总无功功率值
    dainfo.fValue = (float)(((data[19]&0x7F)<<8)|data[20])/10000*3*m_Uo*m_Ubb*m_Io*m_Ibb;
    data_ptr->mValues[indexpos++] = dainfo;
    //总功率因数值
    dainfo.fValue = (float)(((data[21]&0x7F)<<8)|data[22])/10000;
    data_ptr->mValues[indexpos++] = dainfo;
    //A相有功功率值
    dainfo.fValue = (float)(((data[23]&0x7F)<<8)|data[24])/10000*m_Uo*m_Ubb*m_Io*m_Ibb;
    data_ptr->mValues[indexpos++] = dainfo;
    //B相有功功率值
    dainfo.fValue = (float)(((data[25]&0x7F)<<8)|data[26])/10000*m_Uo*m_Ubb*m_Io*m_Ibb;
    data_ptr->mValues[indexpos++] = dainfo;
    //C相有功功率值
    dainfo.fValue = (float)(((data[27]&0x7F)<<8)|data[28])/10000*m_Uo*m_Ubb*m_Io*m_Ibb;//29 30
    data_ptr->mValues[indexpos++] = dainfo;
    //有功总电能

    dainfo.fValue = (float)(((data[31]&0x7F)<<24)|(data[32]<<16)|(data[33]<<8)|(data[34]))
            *m_Uo*m_Ubb*m_Io*m_Ibb/3000/3600;
    data_ptr->mValues[indexpos++] = dainfo;

    //正向有功总电能

    dainfo.fValue = (float)((data[35]<<24)|(data[36]<<16)|(data[37]<<8)|(data[38]))
            *m_Uo*m_Ubb*m_Io*m_Ibb/3000/3600;
    data_ptr->mValues[indexpos++] = dainfo;
    //反向有功总电能

    dainfo.fValue = (float)((data[39]<<24)|(data[40]<<16)|(data[41]<<8)|(data[42]))
            *m_Uo*m_Ubb*m_Io*m_Ibb/3000/3600;
    data_ptr->mValues[indexpos++] = dainfo;

    dainfo.bType = true;
    if(data_ptr->mValues[1].fValue>1 || data_ptr->mValues[3].fValue>1
            || data_ptr->mValues[5].fValue>1)
    {
        dainfo.fValue = 1.0;
    }
    else
        dainfo.fValue = 0.0;
    data_ptr->mValues[indexpos++] = dainfo;
    dainfo.bType = false;
    //AB相电压
    dainfo.fValue = (float)sqrt(data_ptr->mValues[0].fValue*data_ptr->mValues[0].fValue+data_ptr->mValues[2].fValue*data_ptr->mValues[2].fValue+data_ptr->mValues[0].fValue*data_ptr->mValues[2].fValue);
    data_ptr->mValues[indexpos++] = dainfo;
    //BC
    dainfo.fValue = (float)sqrt(data_ptr->mValues[4].fValue*data_ptr->mValues[4].fValue+data_ptr->mValues[2].fValue*data_ptr->mValues[2].fValue+data_ptr->mValues[4].fValue*data_ptr->mValues[2].fValue);
    data_ptr->mValues[indexpos++] = dainfo;
    //CA
    dainfo.fValue = (float)sqrt(data_ptr->mValues[0].fValue*data_ptr->mValues[0].fValue+data_ptr->mValues[4].fValue*data_ptr->mValues[4].fValue+data_ptr->mValues[0].fValue*data_ptr->mValues[4].fValue);
    data_ptr->mValues[indexpos++] = dainfo;
    return RE_SUCCESS;
}

int Electric_message::decode_SPM33(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen,int &iaddcode)
{
    int indexpos =0;
    iaddcode = data[0];
    DataInfo dainfo;
    dainfo.bType = false;
    for(int i=0;i<6;++i)//6个电压
    {
        dainfo.fValue = ((data[3+i*2]<<8)|data[4+i*2])*0.01;
        data_ptr->mValues[indexpos++] = dainfo;
    }
    for(int i=0;i<3;++i)//3个电流
    {
        dainfo.fValue = ((data[15+i*2]<<8)|data[16+i*2])*0.001;
        data_ptr->mValues[indexpos++] = dainfo;
    }
    dainfo.fValue = ((data[21]<<8)|data[22]);//零序电流
    data_ptr->mValues[indexpos++] = dainfo;
    int signedBit = 0;
    for(int j=0;j<2;++j)//2个总功率
    {
        signedBit = Getbit(data[26+j*4],7);
        dainfo.fValue = (((((((data[26+j*4]&0x7F)<<8)|data[25+4*j])<<8)|data[24+4*j])<<8)|data[23+4*j])*0.1;
        if(signedBit==1)
            dainfo.fValue = dainfo.fValue*(-1);
        data_ptr->mValues[indexpos++] = dainfo;
    }
    //功率因数
    signedBit = Getbit(data[31],7);
    dainfo.fValue = (((data[31]&0x7F)<<8)|data[32])*0.001;
    if(signedBit==1)
        dainfo.fValue = dainfo.fValue*(-1);
    data_ptr->mValues[indexpos++] = dainfo;
    for(int i=0;i<6;++i)//6个三相功率
    {
        signedBit = Getbit(data[33+2*i],7);
        dainfo.fValue = (((data[33+2*i]&0x7F)<<8)|data[34+2*i])*0.1;
        if(signedBit==1)
            dainfo.fValue = dainfo.fValue*(-1);
        data_ptr->mValues[indexpos++] = dainfo;
    }
    for(int i=0;i<3;++i)//3个三相功率因数
    {
        signedBit = Getbit(data[45+2*i],7);
        dainfo.fValue = (((data[45+2*i]&0x7F)<<8)|data[46+2*i])*0.001;
        if(signedBit==1)
            dainfo.fValue = dainfo.fValue*(-1);
        data_ptr->mValues[indexpos++] = dainfo;
    }
    //频率
    dainfo.fValue = (((data[51])<<8)|data[52])*0.01;
    data_ptr->mValues[indexpos++] = dainfo;
    for(int i=0;i<6;++i)
    {
        dainfo.fValue = (((((((data[56+4*i])<<8)|data[55+4*i])<<8)|data[54+4*i])<<8)|data[53+4*i])*0.1;
        data_ptr->mValues[indexpos++] = dainfo;
    }
    //4字节无效
    short sstate = ((data[81]<<8)|data[82]);
    dainfo.bType = true;
    for(int i=1;i<10;++i)
    {
        dainfo.fValue = Getbit(sstate,i);
        data_ptr->mValues[indexpos++] = dainfo;
    }
    dainfo.bType = false;
    dainfo.fValue = ((data[83]<<8)|data[84]);
    data_ptr->mValues[indexpos++] = dainfo;
    //2字节保留
    //2个平均电压
    for(int i=0;i<2;++i)
    {
        dainfo.fValue = ((data[87+i*2]<<8)|data[88+i*2])*0.01;
        data_ptr->mValues[indexpos++] = dainfo;
    }
    dainfo.fValue = ((data[91]<<8)|data[92])*0.001;
    data_ptr->mValues[indexpos++] = dainfo;
    dainfo.fValue = ((data[93]<<8)|data[94])*0.001;
    data_ptr->mValues[indexpos++] = dainfo;
    for(int i=0;i<3;++i)
    {
        dainfo.fValue = ((data[95+i*2]<<8)|data[96+i*2])*0.01;
        data_ptr->mValues[indexpos++] = dainfo;
    }
    dainfo.fValue = ((((((data[104]<<8)|data[103])<<8)|data[102])<<8)|data[101])*0.01;
    data_ptr->mValues[indexpos++] = dainfo;
    return RE_SUCCESS;
}

int Electric_message::decode_KSTUPS(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    int indexpos =0;
    iaddcode = data[0];
    DataInfo dainfo;
    dainfo.bType = false;
    //7个电压和频率
    for(int i=0;i<7;++i)
    {
        dainfo.fValue = ((data[3+i*2]<<8)|data[4+i*2])*0.1;
        data_ptr->mValues[indexpos++] = dainfo;
    }

    for(int i=0;i<3;++i)
    {
        dainfo.fValue = ((data[17+i*2]<<8)|data[18+i*2])*0.01;
        data_ptr->mValues[indexpos++] = dainfo;
    }
    for(int i=0;i<10;++i)
    {
        dainfo.fValue = ((data[23+i*2]<<8)|data[24+i*2])*0.1;
        data_ptr->mValues[indexpos++] = dainfo;
    }
    for(int i=0;i<3;++i)
    {
        dainfo.fValue = ((data[43+i*2]<<8)|data[44+i*2]);
        data_ptr->mValues[indexpos++] = dainfo;
    }
    for(int i=0;i<3;++i)
    {
        dainfo.fValue = ((data[49+i*2]<<8)|data[50+i*2])*0.01;
        data_ptr->mValues[indexpos++] = dainfo;
    }
    for(int i=0;i<10;++i)
    {
        dainfo.fValue = ((data[55+i*2]<<8)|data[56+i*2])*0.1;
        data_ptr->mValues[indexpos++] = dainfo;
    }
    for(int i=0;i<2;++i)
    {
        dainfo.fValue = ((data[75+i*2]<<8)|data[76+i*2]);
        data_ptr->mValues[indexpos++] = dainfo;
    }
    for(int i=0;i<2;++i)
    {
        dainfo.fValue = ((data[79+i*2]<<8)|data[80+i*2])*0.1;
        data_ptr->mValues[indexpos++] = dainfo;
    }
    dainfo.fValue = ((data[83]<<8)|data[84]);
    data_ptr->mValues[indexpos++] = dainfo;
    return RE_SUCCESS;
}

int Electric_message::decode_EM400(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen,int &iaddcode)
{
    int indexpos =0;
    iaddcode = data[0];
    DataInfo dainfo;
    int cmdnum = data[1];
    switch(cmdnum)
    {
    case 1:
    {
        indexpos = 26;
        dainfo.bType = true;
        for(int i=0;i<2;++i)
        {
            dainfo.fValue = Getbit(data[3],i);
            data_ptr->mValues[indexpos+i] = dainfo;
        }
    }
        break;
    case 2:
    {
        indexpos = 28;
        dainfo.bType = true;
        for(int i=0;i<4;++i)
        {
            dainfo.fValue = Getbit(data[3],i);
            data_ptr->mValues[indexpos+i] = dainfo;
        }
    }
        break;
    case 3:
    {
        dainfo.bType = false;
        for(int i=0;i<6;++i)
        {
            dainfo.fValue = ((data[15+i*2]<<8)|data[16+i*2])*0.1*m_Ubb;
            data_ptr->mValues[indexpos++] = dainfo;
        }

        for(int i=0;i<3;++i)
        {
            dainfo.fValue = ((data[7+i*2]<<8)|data[8+i*2])*0.001*m_Ibb;
            data_ptr->mValues[indexpos++] = dainfo;
        }
        dainfo.fValue = ((data[27]<<8)|data[28])*0.01;
        data_ptr->mValues[indexpos++] = dainfo;
        dainfo.bType = false;
        dainfo.fValue = ((data[35]<<8)|data[36])*0.001;
         data_ptr->mValues[indexpos++] = dainfo;
        for(int i=0;i<3;++i)
        {
            dainfo.fValue = ((data[29+2*i]<<8)|data[30+2*i])*m_Ubb*m_Ibb*0.001;
            data_ptr->mValues[indexpos++] = dainfo;
        }
        dainfo.bType = false;
        dainfo.fValue = ((((((data[37]<<8)|data[38])<<8)|data[39])<<8)|data[40])*0.1;
        data_ptr->mValues[32] = dainfo;
        dainfo.fValue = ((((((data[41]<<8)|data[42])<<8)|data[43])<<8)|data[44])*0.1;
        data_ptr->mValues[33] = dainfo;
    }
        break;
    case 4:
    {
        indexpos = 14;
        dainfo.bType = false;

        for(int i=0;i<3;++i)
        {
            dainfo.fValue = ((data[3+2*i]<<8)|data[4+2*i])*0.001;
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<9;++i)
        {
            dainfo.fValue = ((data[9+2*i]<<8)|data[10+2*i])*m_Ubb*m_Ibb*0.001;
            data_ptr->mValues[indexpos++] = dainfo;
        }

    }
        break;
    }

    return RE_SUCCESS;
}

int Electric_message::decode_EA66(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    int indexpos =0;
    iaddcode = data[0];
    DataInfo dainfo;
    int cmdnum = data[1];
    switch (cmdnum) {
    case 0x04:
    {
        dainfo.bType = false;
        for(int i=0;i<3;++i)
        {
            dainfo.fValue = (data[3+2*i]<<8)|data[4+2*i];
            dainfo.sValue = (QString("%1V").arg(dainfo.fValue)).toStdString();
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<3;++i)
        {
            dainfo.fValue = ((data[9+2*i]<<8)|data[10+2*i])*0.1;
            dainfo.sValue = (QString("%1Hz").arg(dainfo.fValue)).toStdString();
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<3;++i)
        {
            dainfo.fValue = (data[15+2*i]<<8)|data[16+2*i];
            dainfo.sValue = (QString("%1V").arg(dainfo.fValue)).toStdString();
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<3;++i)
        {
            dainfo.fValue = ((data[21+2*i]<<8)|data[22+2*i])*0.1;
            dainfo.sValue = (QString("%1Hz").arg(dainfo.fValue)).toStdString();
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<3;++i)
        {
            dainfo.fValue = ((data[27+2*i]<<8)|data[28+2*i])*0.1;
            dainfo.sValue = (QString("%1V").arg(dainfo.fValue)).toStdString();
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<3;++i)
        {
            dainfo.fValue = ((data[33+2*i]<<8)|data[34+2*i])*0.1;
            dainfo.sValue = (QString("%1A").arg(dainfo.fValue)).toStdString();
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<3;++i)
        {
            dainfo.fValue = ((data[39+2*i]<<8)|data[40+2*i])*0.1;
            dainfo.sValue = (QString("%1Hz").arg(dainfo.fValue)).toStdString();
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<3;++i)
        {
            dainfo.fValue = ((data[45+2*i]<<8)|data[46+2*i]);
            dainfo.sValue = (QString("%1Kw").arg(dainfo.fValue)).toStdString();
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<3;++i)
        {
            dainfo.fValue = ((data[51+2*i]<<8)|data[52+2*i]);
            dainfo.sValue = (QString("%1KVA").arg(dainfo.fValue)).toStdString();
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<3;++i)
        {
            dainfo.fValue = ((data[57+2*i]<<8)|data[58+2*i]);
            dainfo.sValue = (QString("%1%").arg(dainfo.fValue)).toStdString();
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<2;++i)
        {
            dainfo.fValue = ((data[63+2*i]<<8)|data[64+2*i]);
            dainfo.sValue = (QString("%1V").arg(dainfo.fValue)).toStdString();
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<2;++i)
        {
            dainfo.fValue = ((data[67+2*i]<<8)|data[68+2*i])*0.1;
            dainfo.sValue = (QString("%1A").arg(dainfo.fValue)).toStdString();
            data_ptr->mValues[indexpos++] = dainfo;
        }
        dainfo.fValue = ((data[71]<<8)|data[72])*0.1;
   //     dainfo.sValue = (QString("%1℃").arg(dainfo.fValue)).toStdString();
        data_ptr->mValues[indexpos++] = dainfo;
        for(int i=0;i<2;++i)
        {
            dainfo.fValue = ((data[73+2*i]<<8)|data[74+2*i]);
            dainfo.sValue = (QString("%1V").arg(dainfo.fValue)).toStdString();
            data_ptr->mValues[indexpos++] = dainfo;
        }
        dainfo.fValue = ((data[77]<<8)|data[78]);
        dainfo.sValue = (QString("%1%").arg(dainfo.fValue)).toStdString();
        data_ptr->mValues[indexpos++] = dainfo;
        dainfo.fValue = ((data[79]<<8)|data[80]);
        dainfo.sValue = (QString("%1min").arg(dainfo.fValue)).toStdString();
        data_ptr->mValues[indexpos++] = dainfo;
        for(int i=0;i<4;++i)
        {
            dainfo.fValue = ((data[80+2*i]<<8)|data[81+2*i]);
   //         dainfo.sValue = (QString("%1℃").arg(dainfo.fValue)).toStdString();
            data_ptr->mValues[indexpos++] = dainfo;
        }
        dainfo.fValue = ((data[89]<<8)|data[90])*0.1;
   //     dainfo.sValue = (QString("%1℃").arg(dainfo.fValue)).toStdString();
        data_ptr->mValues[indexpos++] = dainfo;
        dainfo.fValue = ((data[91]<<8)|data[92]);
       /* switch (data[92]) {
        case 1:
        {
            dainfo.sValue = QString("待机模式").toStdString();
        }
            break;
        case 2:
        {
            dainfo.sValue = QString("旁路模式").toStdString();
        }
            break;
        case 3:
        {
            dainfo.sValue = QString("市电模式").toStdString();
        }
            break;
        case 4:
        {
            dainfo.sValue = QString("电池模式").toStdString();
        }
            break;
        case 5:
        {
            dainfo.sValue = QString("电池自检").toStdString();
        }
            break;
        case 6:
        {
            dainfo.sValue = QString("故障模式").toStdString();
        }
            break;
        case 7:
        {
            dainfo.sValue = QString("变频模式").toStdString();
        }
            break;
        case 8:
        {
            dainfo.sValue = QString("紧急关机模式").toStdString();
        }
            break;
        default:
            break;
        }*/
        data_ptr->mValues[indexpos++] = dainfo;
    }
        break;
    case 0x02:
    {
        indexpos = 45;
       dainfo.bType = true;
       for(int i=0;i<8;++i)
       {
           if(i==4)
               continue;
           dainfo.fValue = Getbit(data[3],i);
           data_ptr->mValues[indexpos++] = dainfo;
       }
       for(int i=0;i<8;++i)
       {
           dainfo.fValue = Getbit(data[4],i);
           data_ptr->mValues[indexpos++] = dainfo;
       }
       for(int i=0;i<8;++i)
       {
           if(i==4)
               continue;
           dainfo.fValue = Getbit(data[5],i);
           data_ptr->mValues[indexpos++] = dainfo;
       }
       for(int i=0;i<8;++i)
       {
           dainfo.fValue = Getbit(data[6],i);
           data_ptr->mValues[indexpos++] = dainfo;
       }
       for(int i=0;i<8;++i)
       {
           dainfo.fValue = Getbit(data[7],i);
           data_ptr->mValues[indexpos++] = dainfo;
       }
       for(int i=0;i<8;++i)
       {
           dainfo.fValue = Getbit(data[8],i);
           data_ptr->mValues[indexpos++] = dainfo;
       }
       for(int i=0;i<5;++i)
       {
           dainfo.fValue = Getbit(data[8],i);
           data_ptr->mValues[indexpos++] = dainfo;
       }
       for(int i=0;i<8;++i)
       {
           if(i==5)
               continue;
           dainfo.fValue = Getbit(data[9],i);
           data_ptr->mValues[indexpos++] = dainfo;
       }
       for(int i=0;i<6;++i)
       {
           dainfo.fValue = Getbit(data[10],i);
           data_ptr->mValues[indexpos++] = dainfo;
       }
    }
        break;
    default:
        break;
    }
    return RE_SUCCESS;
}


int Electric_message::decode_AcrNetEm(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
   iaddcode = data[0];
   DataInfo dainfo;
   dainfo.bType = true;
   for(int i=0;i<6;++i)
   {
       dainfo.fValue = Getbit(data[3],i);
       data_ptr->mValues[26+i] = dainfo;
   }
   dainfo.bType = false;
   int nDpt = data[5];
   int nDct = data[6];
   int nDpq = data[7];
   float Ubb = pow(10.00,nDpt-4);
   float Ibb = pow(10.00,nDct-4);
   float Pbb = pow(10.00,nDpq-4);
   for(int i=0;i<6;++i)
   {
       dainfo.fValue = (data[9+2*i]*256+data[10+2*i])*Ubb;
       data_ptr->mValues[i] = dainfo;
   }
   for(int i=0;i<3;++i)
   {
       dainfo.fValue = (data[21+2*i]*256+data[22+2*i])*Ibb;
       data_ptr->mValues[6+i] = dainfo;
   }
   for(int i=0;i<8;++i)
   {
       dainfo.fValue = (data[27+2*i]*256+data[28+2*i])*Pbb;
       data_ptr->mValues[9+i] = dainfo;
   }
   for(int i=0;i<4;++i)
   {
       dainfo.fValue = (data[43+2*i]*256+data[44+2*i])*0.001;
       data_ptr->mValues[17+i] = dainfo;
   }
   for(int i=0;i<4;++i)
   {
       dainfo.fValue = (data[51+2*i]*256+data[52+2*i])*Pbb;
       data_ptr->mValues[21+i] = dainfo;
   }
   dainfo.fValue = (data[59]*256+data[60])*0.01;
   data_ptr->mValues[25] = dainfo;
   return RE_SUCCESS;
}

int Electric_message::decode_Elecctr(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    iaddcode = d_devInfo.iAddressCode;
    DataInfo dainfo;
    dainfo.bType = true;
    dainfo.fValue = 1.0;
    data_ptr->mValues[0] = dainfo;
    return RE_SUCCESS;
}

int Electric_message::decode_Acr_PZ(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    iaddcode = data[0];
    DataInfo dainfo;
    dainfo.bType = false;
    int digbit = (data[9]*256+data[10]);
    float fbit = pow(10.00,digbit-3);
    for(int i=0;i<3;++i)
    {
        dainfo.fValue = (data[3+2*i]*256+data[4+2*i])*fbit;
        data_ptr->mValues[i] = dainfo;
    }
    return RE_SUCCESS;
}

int Electric_message::decode_Acr_Ard2L(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    iaddcode = data[0];
    DataInfo dainfo;
    dainfo.bType = false;
    int digbit = (data[20]);
    for(int i=0;i<3;++i)
    {
        dainfo.fValue = data[4+2*i]/float(digbit);
        data_ptr->mValues[i*2] = dainfo;
        dainfo.fValue = data[3+2*i]/float(digbit);
        data_ptr->mValues[2*i+1] = dainfo;
    }
    return RE_SUCCESS;
}

//执行联动命令
void Electric_message::exec_action_task_now(map<int,vector<ActionParam> > &param,int actionType,
                                            string sUser,e_ErrorCode &eErrCode)
{
    switch(actionType)
    {
    case ACTP_CLOSE_DEVICE:{
        if(m_pSession!=NULL){
            eErrCode = EC_OK;
            if(param.size()>=2){
                int param_2 = atoi(param[1][0].strParamValue.c_str());
                m_pSession->send_cmd_to_dev(d_devInfo.sDevNum,MSG_DEV_TURNOFF_OPR,param_2,eErrCode);
            }
            else
            {
                m_pSession->send_cmd_to_dev(d_devInfo.sDevNum,MSG_DEV_TURNOFF_OPR,0,eErrCode);
            }
        }
    }
        break;
    }


}

int Electric_message::decode_ST_C6_20Ks(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    int indexpos =0,iReadNum=1,iLastNum=(nDataLen-1);
    char cNum[10]={0};
    iaddcode = d_devInfo.iAddressCode;
    DataInfo dainfo;
    dainfo.bType = false;
    for(int i=0;i<16;++i)//解析前16个模拟量
    {
        for(int jpos=0;iLastNum>0 && jpos<10;++jpos) //读取每个模拟量有效数值
        {
            if(data[iReadNum]!=0x20)
            {
                cNum[jpos] = data[iReadNum];
                ++iReadNum;
                --iLastNum;
            }
            else
            {
                ++iReadNum;
                --iLastNum;
                break;
            }
        }
        if(cNum[0]==0x2D)
            dainfo.fValue = 0;
        else
            dainfo.fValue = float(atof(cNum));
        data_ptr->mValues[i] = dainfo;
        memset(&cNum,0,10);
    }
    //查找下个命令数据头
    unsigned char cDes[1]={0};
    cDes[0]=0x28;
    cDes[1]=d_devInfo.iAddressCode;
    indexpos = kmp(&data[iReadNum],iLastNum,cDes,1);
    if(indexpos>=0)
    {
        iReadNum = iReadNum+indexpos+1;
        iLastNum = iLastNum-(indexpos+1);
        for(int i=0;i<12;++i)
        {
            for(int jpos=0;iLastNum>0 && jpos<10;++jpos) //读取每个模拟量有效数值
            {
                if(data[iReadNum]!=0x20)
                {
                    cNum[jpos] = data[iReadNum];
                    ++iReadNum;
                    --iLastNum;
                }
                else
                {
                    ++iReadNum;
                    --iLastNum;
                    break;
                }
            }
            if(cNum[0]==0x2D)
                dainfo.fValue = 0;
            else
                dainfo.fValue = float(atof(cNum));
            data_ptr->mValues[i+16] = dainfo;
            memset(&cNum,0,10);
        }
        char cState;
        dainfo.bType = true;
        for(int i=0;i<5;++i){
            cState = data[iReadNum++];
            if(cState=='0')
                dainfo.fValue = 0;
            else
                dainfo.fValue = 1;
            data_ptr->mValues[i+28] = dainfo;
            --iLastNum;
        }
    }
    return RE_SUCCESS;
}

int Electric_message::decode_DSE_7320(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    int indexpos =0;
    iaddcode = d_devInfo.iAddressCode;
    DataInfo dainfo;
    dainfo.bType = false;
    int datalen = data[2];
    if(datalen==48)
    {
        indexpos=14;
        int sbit = Getbit(data[3],7);
        if(sbit==0)
            sbit = 1;
        else
            sbit = -1;
        dainfo.fValue = ((data[3]&0x7F)*256*256*256+data[4]*256*256+data[5]*256+data[6])*sbit;
        data_ptr->mValues[indexpos++] = dainfo;
        for(int i=0;i<4;++i)
        {
            dainfo.fValue = (data[7+4*i]*256*256*256+data[8+4*i]*256*256+data[9+4*i]*256+data[10+4*i]);
            data_ptr->mValues[indexpos++] = dainfo;
        }

        for(int i=0;i<4;++i)
        {
            sbit = Getbit(data[23+4*i],7);
            if(sbit==0)
                sbit = 1;
            else
                sbit = -1;
            dainfo.fValue = ((data[23+4*i]&0x7F)*256*256*256+data[24+4*i]*256*256+data[25+4*i]*256+data[26+4*i])*sbit;
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<4;++i)
        {
            sbit = Getbit(data[39+2*i],7);
            if(sbit==0)
                sbit = 1;
            else
                sbit = -1;
            int ivalue = (data[39+2*i]&0x7F)*256+data[40+2*i];
            ivalue = ivalue>100 ? 100:ivalue;
            dainfo.fValue = ivalue*sbit*0.01;
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<2;++i)
        {
            sbit = Getbit(data[47+2*i],7);
            if(sbit==0)
                sbit = 1;
            else
                sbit = -1;
            dainfo.fValue = ((data[47+2*i]&0x7F)*256+data[48+2*i])*sbit*0.1;
            data_ptr->mValues[indexpos++] = dainfo;
        }
    }
    else
    {
        dainfo.fValue = (data[3]*256+data[4])*0.1;
        data_ptr->mValues[indexpos++] = dainfo;
        for(int i=0;i<10;++i)
        {
            dainfo.fValue = (data[5+4*i]*256*256*256+data[6+4*i]*256*256+data[7+4*i]*256+data[8+4*i])*0.1;
            data_ptr->mValues[indexpos++] = dainfo;
        }
        for(int i=0;i<3;++i)
        {
            //45
            int sbit = Getbit(data[45+4*i],7);
            if(sbit==0)
                sbit = 1;
            else
                sbit = -1;
            dainfo.fValue = ((data[45+4*i]&0x7F)*256*256*256+data[46+4*i]*256*256+data[47+4*i]*256+data[48+4*i])*sbit;
            data_ptr->mValues[indexpos++] = dainfo;
        }
    }
    return RE_SUCCESS;
}

int Electric_message::decode_EA800(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    int indexpos =0;
    iaddcode = data[0];
    DataInfo dainfo;
    dainfo.bType = false;
    for(int i=0;i<3;++i)
    {
        dainfo.fValue = ((data[3+2*i]<<8)|data[4+2*i])*0.01;
        dainfo.sValue = (QString("%1V").arg(dainfo.fValue)).toStdString();
        data_ptr->mValues[indexpos++] = dainfo;
    }
    dainfo.fValue = ((data[15]<<8)|data[16])*0.01;
    dainfo.sValue = (QString("%1HZ").arg(dainfo.fValue)).toStdString();
    data_ptr->mValues[indexpos++] = dainfo;
    dainfo.fValue = ((data[23]<<8)|data[24])*0.01;
    dainfo.sValue = (QString("%1V").arg(dainfo.fValue)).toStdString();
    data_ptr->mValues[indexpos++] = dainfo;
    dainfo.fValue = ((data[41]<<8)|data[42])*0.01;
    dainfo.sValue = (QString("%1HZ").arg(dainfo.fValue)).toStdString();
    data_ptr->mValues[indexpos++] = dainfo;
    dainfo.fValue = ((data[45]<<8)|data[46])*0.01;
    dainfo.sValue = (QString("%1V").arg(dainfo.fValue)).toStdString();
    data_ptr->mValues[indexpos++] = dainfo;
    dainfo.fValue = ((data[57]<<8)|data[58])*0.01;
    dainfo.sValue = (QString("%1HZ").arg(dainfo.fValue)).toStdString();
    data_ptr->mValues[indexpos++] = dainfo;
    dainfo.fValue = ((data[59]<<8)|data[60])*0.01;
    dainfo.sValue = (QString("%1V").arg(dainfo.fValue)).toStdString();
    data_ptr->mValues[indexpos++] = dainfo;
    dainfo.fValue = ((data[131]<<8)|data[132])*0.01;
    dainfo.sValue = (QString("%1V").arg(dainfo.fValue)).toStdString();
    data_ptr->mValues[indexpos++] = dainfo;
    dainfo.fValue = ((data[137]<<8)|data[138])*0.01;
    dainfo.sValue = (QString("%1HZ").arg(dainfo.fValue)).toStdString();
    data_ptr->mValues[indexpos++] = dainfo;
    return RE_SUCCESS;
}

int Electric_message::decode_hx9703(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    int indexpos =0;
    iaddcode = d_devInfo.iAddressCode;
    DataInfo dainfo;
    dainfo.bType = true;
    for(int i=0;i<8;++i)
    {
        dainfo.fValue = Getbit(data[7],i);
        data_ptr->mValues[indexpos++] = dainfo;
    }
    dainfo.fValue = data[8];
    data_ptr->mValues[indexpos++] = dainfo;
    return RE_SUCCESS;
}

int Electric_message::decode_Sf3pmanostat(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    iaddcode = AsciiToInt(data[3])*16+AsciiToInt(data[4]);
    int indexpos =0;
    DataInfo dainfo;
    dainfo.bType = false;
    //模拟量数据从下标15开始
    int datapos = 15;
    for(int i=0;i<12;++i)
    {
        dainfo.fValue = ((AsciiToInt(data[15+4*i])*16+AsciiToInt(data[16+4*i]))*256
                +AsciiToInt(data[17+4*i])*16+AsciiToInt(data[18+4*i]))*0.01;
        data_ptr->mValues[indexpos++] = dainfo;
    }
    return RE_SUCCESS;
}

int Electric_message::decode_Sfdjups(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    int indexpos =0,iReadNum=1,iLastNum=(nDataLen-1);
    char cNum[10]={0};
    iaddcode = d_devInfo.iAddressCode;
    DataInfo dainfo;
    dainfo.bType = false;
    for(int i=0;i<8;++i)//解析前8个模拟量
    {
        for(int jpos=0;iLastNum>0 && jpos<10;++jpos) //读取每个模拟量有效数值
        {
            if(data[iReadNum]!=0x20)
            {
                cNum[jpos] = data[iReadNum];
                ++iReadNum;
                --iLastNum;
            }
            else
            {
                ++iReadNum;
                --iLastNum;
                break;
            }
        }
        if(cNum[0]==0x2D)
            dainfo.fValue = 0;
        else
            dainfo.fValue = float(atof(cNum));
        data_ptr->mValues[i] = dainfo;
        memset(&cNum,0,10);
    }
    //查找下个命令数据头
    unsigned char cDes[1]={0};
    cDes[0]=0x21;
    cDes[1]=d_devInfo.iAddressCode;
    indexpos = kmp(&data[iReadNum],iLastNum,cDes,1);
    if(indexpos>=0)
    {
        iReadNum = iReadNum+indexpos+1;
        iLastNum = iLastNum-(indexpos+1);
        for(int i=0;i<12;++i)
        {
            for(int jpos=0;iLastNum>0 && jpos<10;++jpos) //读取每个模拟量有效数值
            {
                if(data[iReadNum]!=0x20 && data[iReadNum]!=0x2F)
                {
                    cNum[jpos] = data[iReadNum];
                    ++iReadNum;
                    --iLastNum;
                }
                else
                {
                    ++iReadNum;
                    --iLastNum;
                    break;
                }
            }
            if(cNum[0]==0x2D)
                dainfo.fValue = 0;
            else
                dainfo.fValue = float(atof(cNum));
            data_ptr->mValues[i+8] = dainfo;
            memset(&cNum,0,10);
        }
    }
    return RE_SUCCESS;
}

int Electric_message::decode_EA800II(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &iaddcode)
{
    int indexpos =0,iReadNum=1,iLastNum=(nDataLen-1);
    char cNum[10]={0};
    iaddcode = d_devInfo.iAddressCode;
    DataInfo dainfo;
    dainfo.bType = false;
    for(int i=0;i<7;++i)
    {
        for(int jpos=0;iLastNum>0 && jpos<10;++jpos) //读取每个模拟量有效数值
        {
            if(data[iReadNum]!=0x20)
            {
                cNum[jpos] = data[iReadNum];
                ++iReadNum;
                --iLastNum;
            }
            else
            {
                ++iReadNum;
                --iLastNum;
                break;
            }
        }
        if(i!=1)
        {
            if(cNum[0]==0x2D)
                dainfo.fValue = 0;
            else
                dainfo.fValue = float(atof(cNum));
            data_ptr->mValues[indexpos++] = dainfo;
        }
        memset(&cNum,0,10);
    }
    dainfo.bType = true;
    for(int j=0;j<8;++j)
    {
        if(iReadNum>nDataLen)
            break;
        dainfo.fValue = AsciiToInt(data[iReadNum]);
        data_ptr->mValues[indexpos++] = dainfo;
        ++iReadNum;
    }
    return RE_SUCCESS;
}


}
