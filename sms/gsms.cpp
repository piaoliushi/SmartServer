#include "gsms.h"
//#include <QWaitCondition>
Gsms::Gsms(QObject *parent) :
    QObject(parent)
  ,pQSerialport_ptr_(new QSerialPort(this))
  ,str_SCA("")
{
    m_nSendIn = 0;
    m_nSendOut = 0;
    m_nRecvIn = 0;
    m_nRecvOut = 0;
    b_run_ = false;
    nTrycount_ = 0;
    //  _SmThreadptr.reset(new boost::thread(boost::bind(&Gsms::SmThread,this)));
    pTimerOut_ = new QTimer(this);
    pTimerTryInit_ = new QTimer(this);
    nstate = stUnknow;
    connect(pTimerOut_,SIGNAL(timeout()),this,SLOT(check_timer()));
    connect(pTimerTryInit_,SIGNAL(timeout()),this,SLOT(try_timer()));
}

Gsms::~Gsms()
{
    if(pTimerOut_->isActive())
    {
        pTimerOut_->stop();
    }
    delete pTimerOut_;
    pTimerOut_ = NULL;
    if(pTimerTryInit_->isActive())
    {
        pTimerTryInit_->stop();
    }
    delete pTimerTryInit_;
    pTimerTryInit_ = NULL;
    boost::recursive_mutex::scoped_lock lock(m_run_mutex);
    b_run_ = false;
    disconnect(pQSerialport_ptr_.get(),SIGNAL(readyRead()),this,SLOT(comrecive()));
 //   CloseComm();
}

bool Gsms::OpenCom(int nPort, int nBaudRate)
{
    if(!pQSerialport_ptr_)
    {
        pQSerialport_ptr_ = boost::shared_ptr<QSerialPort>(new QSerialPort(this));
    }
    if(pQSerialport_ptr_->isOpen())
        return true;
    QString sComStr;
#ifdef Q_OS_WIN
    sComStr = QString("COM%1").arg(nPort);
#else
    sComStr = QString("/dev/ttyO%1").arg(nPort);
   // sComStr = QString("/dev/ttyS0");
#endif
    pQSerialport_ptr_->setPortName(sComStr);
    if(!pQSerialport_ptr_->open(QIODevice::ReadWrite))
        return false;
    pQSerialport_ptr_->setBaudRate(nBaudRate);
    pQSerialport_ptr_->setParity(QSerialPort::NoParity);
    pQSerialport_ptr_->setDataBits(QSerialPort::Data8);
    pQSerialport_ptr_->setStopBits(QSerialPort::OneStop);
    pQSerialport_ptr_->setFlowControl(QSerialPort::NoFlowControl);
    connect(pQSerialport_ptr_.get(),SIGNAL(readyRead()), this, SLOT(comrecive()));
    return true;
}

bool Gsms::CloseComm()
{
    if(pQSerialport_ptr_ && pQSerialport_ptr_->isOpen())
    {
        pQSerialport_ptr_->close();
    }
    return true;
}

int Gsms::ReadComm(char *pData, int nLength,int msec)
{
   int result = 0;
   if(pQSerialport_ptr_)
   {
       result = pQSerialport_ptr_->read(pData,nLength);
   }
   return result;
}

int Gsms::WriteComm(const char *pData, int nLength)
{
    if(pQSerialport_ptr_)
    {
        if(pQSerialport_ptr_->waitForReadyRead(10))
        {
            pQSerialport_ptr_->readAll();
        }
        int nwrite = pQSerialport_ptr_->write(pData,nLength);
        if(pQSerialport_ptr_->waitForBytesWritten(5))
            return nwrite;
        else
            return 0;
    }
    return 0;
}

bool Gsms::gsmInit()
{
  //  char ans[128];		// 应答串
    boost::recursive_mutex::scoped_lock lock(data_mutex);
    nstate=stIsHaveModelAck;
    // 测试GSM-MODEM的存在性
    WriteComm("AT\r", 3);
     //"AT\r"
    pTimerOut_->start(1000);
    return true;
}

void Gsms::PutSendMessage(SM_PARAM *pSmParam)
{
    boost::recursive_mutex::scoped_lock lock(m_csSend_mutex);
    memcpy(&m_SmSend[m_nSendIn], pSmParam, sizeof(SM_PARAM));
    m_nSendIn++;
    if (m_nSendIn >= MAX_SM_SEND)
        m_nSendIn = 0;
}

bool Gsms::GetSendMessage(SM_PARAM *pSmParam)
{
    bool fSuccess = false;
    boost::recursive_mutex::scoped_lock lock(m_csSend_mutex);
    if (m_nSendOut != m_nSendIn)
    {
        memcpy(pSmParam, &m_SmSend[m_nSendOut], sizeof(SM_PARAM));

        m_nSendOut++;
        if (m_nSendOut >= MAX_SM_SEND)
            m_nSendOut = 0;
        fSuccess = true;
    }
    return fSuccess;
}

void Gsms::PutRecvMessage(SM_PARAM *pSmParam, int nCount)
{
    boost::recursive_mutex::scoped_lock lock(m_csRecv_mutex);

    for (int i = 0; i < nCount; i++)
    {
        memcpy(&m_SmRecv[m_nRecvIn], pSmParam, sizeof(SM_PARAM));
        m_nRecvIn++;
        if (m_nRecvIn >= MAX_SM_RECV)
            m_nRecvIn = 0;
        pSmParam++;
    }
}

bool Gsms::GetRecvMessage(SM_PARAM *pSmParam)
{
    bool fSuccess = false;
    boost::recursive_mutex::scoped_lock lock(m_csRecv_mutex);
    if (m_nRecvOut != m_nRecvIn)
    {
        memcpy(pSmParam, &m_SmRecv[m_nRecvOut], sizeof(SM_PARAM));

        m_nRecvOut++;
        if (m_nRecvOut >= MAX_SM_RECV)
            m_nRecvOut = 0;
        fSuccess = true;
    }
    return fSuccess;
}

void Gsms::SendSMSContent(string sSmsc, string PhoneNumber, string AlarmContent)
{
    SM_PARAM SmParam;

    memset(&SmParam, 0, sizeof(SM_PARAM));

    if(sSmsc.substr(0,2)!="86")
        sSmsc = "86" + sSmsc;
    if(PhoneNumber.substr(0,2)!="86")
        PhoneNumber = "86" + PhoneNumber;
    // 填充短消息结构
    if(str_SCA.length()<13)
        strcpy(SmParam.SCA, sSmsc.c_str());
    else
        strcpy(SmParam.SCA, str_SCA.c_str());
    strcpy(SmParam.TPA, PhoneNumber.c_str());
    strcpy(SmParam.TP_UD, AlarmContent.c_str());
    SmParam.TP_PID = 0;
    SmParam.TP_DCS = GSM_UCS2;

    // 发送短消息
    PutSendMessage(&SmParam);
}

void Gsms::SmThread()
{
    int nMsg;				// 收到短消息条数
    int nDelete;			// 目前正在删除的短消息编号
    SM_BUFF buff;			// 接收短消息列表的缓冲区
    SM_PARAM param[256];	// 发送/接收短消息缓冲区
    time_t tmOrg, tmNow;		// 上次和现在的时间，计算超时用

    //cout<<"thread start"<<endl;
    {
        boost::recursive_mutex::scoped_lock lock(m_run_mutex);
        b_run_ = true;
    }
    // 初始状态
    nState = stBeginRest;
    while (nState != stExitThread)
    {
        switch(nState)
        {
        case stBeginRest:
            time(&tmOrg);
            nState = stContinueRest;
            break;
        case stContinueRest:
        {

            if (GetSendMessage(&param[0]))
                nState = stSendMessageRequest;
        }
            break;
        case stSendMessageRequest:
            time(&tmOrg);
            gsmSendMessage(&param[0]);
            memset(&buff, 0, sizeof(SM_BUFF));
            time(&tmOrg);
            nState = stSendMessageResponse;

            break;
        case stSendMessageResponse:
           {
             time(&tmNow);
             boost::recursive_mutex::scoped_lock lock(m_cmdresult_mutex);
             if(n_cmdresult_==0){
                 cout<<"Send sms error------GSM_OK----stSendMessageResponse----n_cmdresult_="<<n_cmdresult_<<endl;
                 nState = stBeginRest;
             }
             else if(n_cmdresult_==1){
                 nState = stSendMessageWaitIdle;
             }
             else
             {
                 double ninterval = difftime(tmNow,tmOrg);
                 if(ninterval>=20){
                     nState = stSendMessageWaitIdle;
                 }
             }
           }
             break;
        case stSendMessageWaitIdle:
            nState = stSendMessageRequest;		// 直到发送成功为止
            //nState = stSendMessageWaitIdle;
            break;
        case stReadMessageRequest:
            gsmReadMessageList();
            memset(&buff, 0, sizeof(SM_BUFF));
            time(&tmOrg);
            nState = stReadMessageResponse;
            break;
        case stReadMessageResponse:{
            time(&tmNow);
            int nRslt = gsmGetResponse(&buff);
            switch (nRslt)
            {
                case GSM_OK:{
                    nMsg = gsmParseMessageList(param, &buff);
                    if (nMsg > 0)
                    {
                        PutRecvMessage(param, nMsg);
                        nDelete = 0;
                        nState = stDeleteMessageRequest;
                    }
                    else
                        nState = stBeginRest;
                    }
                    break;
                case GSM_ERR:
                    nState = stBeginRest;
                    break;
                default:{
                    double ninterval = difftime(tmNow,tmOrg);
                    if (ninterval >= 15)		// 15秒超时
                        nState = stBeginRest;
                    }
                    break;
            }
        }
            break;
        case stDeleteMessageRequest:
            if (nDelete < nMsg)
            {
                gsmDeleteMessage(param[nDelete].index);
                memset(&buff, 0, sizeof(SM_BUFF));
                time(&tmOrg);
                nState = stDeleteMessageResponse;
            }
            else
            {
                nState = stBeginRest;
            }
            break;
        case stDeleteMessageResponse:
            time(&tmNow);
            switch (gsmGetResponse(&buff))
            {
                case GSM_OK:
                    nDelete++;
                    nState = stDeleteMessageRequest;
                    break;
                case GSM_ERR:
                    nState = stDeleteMessageWaitIdle;
                    break;
                default:
                    double ninterval = difftime(tmNow,tmOrg);
                    if (ninterval >= 5)		// 5秒超时
                    {
                        nState = stBeginRest;
                    }
                    break;
            }
            break;
        case stDeleteMessageWaitIdle:
            nState = stDeleteMessageRequest;		// 直到删除成功为止
            break;
        }
        {
            boost::recursive_mutex::scoped_lock lock(m_run_mutex);
            if(!b_run_)
                nState = stExitThread;
        }
    }
}

void Gsms::GetCenterNumber()
{
    boost::recursive_mutex::scoped_lock lock(data_mutex);
    nstate=stReadCSCAack;
    WriteComm("AT+CSCA?\r",9);
    pTimerOut_->start(2000);
}

void Gsms::Run()
{
    _SmThreadptr.reset(new boost::thread(boost::bind(&Gsms::SmThread,this)));
}

int Gsms::gsmSendMessage(SM_PARAM *pSrc)
{
    int nPduLength;		// PDU串长度
    unsigned char nSmscLength;	// SMSC串长度
    char cmd[16];		// 命令串
    memset(pdu,0,512);
    nPduLength = gsmEncodePdu(pSrc, pdu);	// 根据PDU参数，编码PDU串
    strcat(pdu, "\x01a");		// 以Ctrl-Z结束

    gsmString2Bytes(pdu, &nSmscLength, 2);	// 取PDU串中的SMSC信息长度
    nSmscLength++;
    sprintf(cmd, "AT+CMGS=%d\r", nPduLength / 2 - nSmscLength);	// 生成命令
    n_cmdresult_=-1;
    boost::recursive_mutex::scoped_lock lock(data_mutex);
    nstate=stSendMessageRequest;
    int nwlen = WriteComm(cmd, strlen(cmd));	// 先输出命令串
    return -1;
}

int Gsms::gsmReadMessageList()
{
    boost::recursive_mutex::scoped_lock lock(data_mutex);
    nstate=stReadMessageResponse;
    return WriteComm("AT+CMGL\r", 8);
}

int Gsms::gsmDeleteMessage(int index)
{
    char cmd[16];		// 命令串

    sprintf(cmd, "AT+CMGD=%d\r", index);	// 生成命令

    // 输出命令串
    return WriteComm(cmd, strlen(cmd));
}

int Gsms::gsmGetResponse(SM_BUFF *pBuff)
{
    int nLength=0;		// 串口收到的数据长度
    int nState;

    // 从串口读数据，追加到缓冲区尾部
    if(pQSerialport_ptr_->waitForReadyRead(5000))
            nLength = ReadComm(&pBuff->data[pBuff->len], 128);
    pBuff->len += nLength;

    // 确定GSM MODEM的应答状态
    nState = GSM_WAIT;
    if ((nLength > 0) && (pBuff->len >= 4))
    {
        if (strncmp(&pBuff->data[pBuff->len - 4], "OK\r\n", 4) == 0)  nState = GSM_OK;
        else if (strstr(pBuff->data, "+CMS ERROR") != NULL) nState = GSM_ERR;
    }
    return nState;
}

int Gsms::gsmParseMessageList(SM_PARAM *pMsg, SM_BUFF *pBuff)
{
    int nMsg;			// 短消息计数值
    char* ptr;			// 内部用的数据指针

    nMsg = 0;
    ptr = pBuff->data;

    // 循环读取每一条短消息, 以"+CMGL:"开头
    while((ptr = strstr(ptr, "+CMGL:")) != NULL)
    {
        ptr += 6;		// 跳过"+CMGL:", 定位到序号
        sscanf(ptr, "%d", &pMsg->index);	// 读取序号
//		TRACE("  index=%d\n",pMsg->index);

        ptr = strstr(ptr, "\r\n");	// 找下一行
        if (ptr != NULL)
        {
            ptr += 2;		// 跳过"\r\n", 定位到PDU

            gsmDecodePdu(ptr, pMsg);	// PDU串解码

            pMsg++;		// 准备读下一条短消息
            nMsg++;		// 短消息计数加1
        }
    }

    return nMsg;
}

void Gsms::get_sendmsg_cmd_ack()
{
    cach_receive_.clear();

    QByteArray qarray = pQSerialport_ptr_->readAll();

    if(qarray.size()==4 && strncmp(qarray.constData(),"\r\n> ",4)==0)
    {
        boost::recursive_mutex::scoped_lock lock(data_mutex);
        nstate=stSendMessageResponse;
        WriteComm(pdu, strlen(pdu));		// 得到肯定回答，继续输出PDU串
    }
    else
    {
        boost::recursive_mutex::scoped_lock lock(m_cmdresult_mutex);
        n_cmdresult_ = 1;
    }
}

void Gsms::get_Response_cmd_ack()
{
    QByteArray qarray = pQSerialport_ptr_->readAll();
    cach_receive_.append(qarray);
    cout<<"get_Response_cmd_ack:-----readAll===size="<<cach_receive_.size()<<"---value="<<cach_receive_.constData()<<endl;
    if (strstr(cach_receive_.constData(), "OK\r\n") != NULL)
    {
        cach_receive_.clear();
        boost::recursive_mutex::scoped_lock lock(m_cmdresult_mutex);
        n_cmdresult_ = 0;
        emit S_state(3,true);
    }
    else if (strstr(cach_receive_.constData(), "ERROR") != NULL)
    {
         cach_receive_.clear();
         boost::recursive_mutex::scoped_lock lock(m_cmdresult_mutex);
         n_cmdresult_ = 1;
         emit S_state(3,false);
    }
    else
    {
        boost::recursive_mutex::scoped_lock lock(m_cmdresult_mutex);
        n_cmdresult_ = -1;
    }
}

void Gsms::get_CSCA_cmd_ack()
{
    if(pTimerOut_->isActive())
        pTimerOut_->stop();
    QByteArray qarray = pQSerialport_ptr_->readAll();
  //  cout<<qarray.data()<<endl;
    cach_receive_.append(qarray);
    if(strstr(cach_receive_.constData(),"ERROR")!=NULL)
    {
        cach_receive_.clear();
        emit S_state(2,false);
        if(nTrycount_<5)
            pTimerTryInit_->start(60000+60000*nTrycount_);
        return;
    }
    if(strstr(cach_receive_.constData(), "CSCA") != NULL)
   {
        if(strstr(cach_receive_.constData(), "OK\r\n") != NULL)
        {
            nTrycount_ = 0;
            if(strstr(cach_receive_.constData(), "\",") != NULL)
            {
                char * charpos = strstr(cach_receive_.data(),"86");
                if(charpos!=NULL)
                {
                    str_SCA = string(charpos);
                    str_SCA = str_SCA.substr(0,str_SCA.find("\""));
                    cach_receive_.clear();
                    Run();
                    emit S_state(2,true);
                }
            }
        }
   }
}

void Gsms::get_have_cmd_ack()
{
    if(pTimerOut_->isActive())
        pTimerOut_->stop();
    QByteArray qarray = pQSerialport_ptr_->readAll();
    if(strstr(qarray.constData(),"OK")==NULL)
    {
     //   emit S_have_model(false);
     //   emit S_init_result(false);
        emit S_state(0,false);
        if(nTrycount_<5)
            pTimerTryInit_->start(60000+60000*nTrycount_);
    }
    else
    {
        //emit S_have_model(true);
        emit S_state(0,true);
        boost::recursive_mutex::scoped_lock lock(data_mutex);
        nstate=stAteAck;
        WriteComm("ATE0\r", 5);
        nTrycount_ = 0;
        pTimerOut_->start(1000);
    }
}

void Gsms::get_ate_cmd_ack()
{
    if(pTimerOut_->isActive())
        pTimerOut_->stop();
    QByteArray qarray = pQSerialport_ptr_->readAll();
    if(strstr(qarray.constData(),"OK")==NULL)
        //emit S_init_result(false);
    {
        emit S_state(1,false);
        if(nTrycount_<5)
            pTimerTryInit_->start(60000+60000*nTrycount_);
    }
    else
    {
        boost::recursive_mutex::scoped_lock lock(data_mutex);
        nstate=stCmgfAck;
        WriteComm("AT+CMGF=0\r",10);
        nTrycount_ = 0;
        pTimerOut_->start(1000);
    }
}

void Gsms::get_cmgf_cmd_ack()
{
    if(pTimerOut_->isActive())
        pTimerOut_->stop();
    QByteArray qarray = pQSerialport_ptr_->readAll();
    if(strstr(qarray.constData(),"OK")==NULL)
      //  emit S_init_result(false);
    {
        emit S_state(1,false);
        if(nTrycount_<5)
            pTimerTryInit_->start(60000+60000*nTrycount_);
    }
    else
    {
       // emit S_init_result(true);
        emit S_state(1,true);
        nTrycount_ = 0;
        GetCenterNumber();
    }
}


void Gsms::comrecive()
{
    int cur_state=stUnknow;
    {
        boost::recursive_mutex::scoped_lock lock(data_mutex);
        cur_state = nstate;
    }
    switch (cur_state) {
    case stSendMessageRequest:
        get_sendmsg_cmd_ack();
        break;
    case stSendMessageResponse:
        get_Response_cmd_ack();
        break;
    case stReadCSCAack:
        get_CSCA_cmd_ack();
        break;
    case stIsHaveModelAck:
        get_have_cmd_ack();
        break;
    case stAteAck:
        get_ate_cmd_ack();
        break;
    case stCmgfAck:
        get_cmgf_cmd_ack();
        break;
    default:
        pQSerialport_ptr_->readAll();
        break;
    }
}

void Gsms::check_timer()
{
    pTimerOut_->stop();
    int curState=-1;
    {
        boost::recursive_mutex::scoped_lock lock(data_mutex);
        curState = nstate;
    }
    switch (curState) {
    case stIsHaveModelAck:{
     //   emit S_have_model(false);
     //   emit S_init_result(false);
        emit S_state(0,false);
    }
        break;
    case stAteAck:{
      //  emit S_init_result(false);
        emit S_state(1,false);
    }
        break;
    case stCmgfAck:{
      //  emit S_init_result(false);
        emit S_state(1,false);
    }
        break;
    case stReadCSCAack:{
        //超时信号
        emit S_state(2,false);
    }
        break;
    default:
        break;
    }
    if(nTrycount_<5)
        pTimerTryInit_->start(60000+60000*nTrycount_);
}

void Gsms::try_timer()
{
    pTimerTryInit_->stop();
    nTrycount_++;
    int curState=-1;
    {
        boost::recursive_mutex::scoped_lock lock(data_mutex);
        curState = nstate;
    }
    switch (curState) {
    case stIsHaveModelAck:{
        gsmInit();
    }
        break;
    case stAteAck:{
        WriteComm("ATE0\r", 5);
    }
        break;
    case stCmgfAck:{

    }
        break;
    case stReadCSCAack:{

    }
        break;
    default:
        break;
    }
}

int Gsms::gsmBytes2String(const unsigned char *pSrc, char *pDst, int nSrcLength)
{
    const char tab[]="0123456789ABCDEF";	// 0x0-0xf的字符查找表
    for (int i = 0; i < nSrcLength; i++)
    {
        *pDst++ = tab[*pSrc >> 4];		// 输出高4位
        *pDst++ = tab[*pSrc & 0x0f];	// 输出低4位
        *pSrc++;
    }
    // 输出字符串加个结束符
    *pDst = '\0';
    // 返回目标字符串长度
    return (nSrcLength * 2);
}

int Gsms::gsmString2Bytes(const char *pSrc, unsigned char *pDst, int nSrcLength)
{
    for (int i = 0; i < nSrcLength; i += 2)
    {
        // 输出高4位
        if ((*pSrc >= '0') && (*pSrc <= '9'))
        {
            *pDst = (*pSrc - '0') << 4;
        }
        else
        {
            *pDst = (*pSrc - 'A' + 10) << 4;
        }

        pSrc++;

        // 输出低4位
        if ((*pSrc>='0') && (*pSrc<='9'))
        {
            *pDst |= *pSrc - '0';
        }
        else
        {
            *pDst |= *pSrc - 'A' + 10;
        }

        pSrc++;
        pDst++;
    }

    // 返回目标数据长度
    return (nSrcLength / 2);
}

int Gsms::gsmEncode7bit(const char *pSrc, unsigned char *pDst, int nSrcLength)
{
    int nSrc;		// 源字符串的计数值
    int nDst;		// 目标编码串的计数值
    int nChar;		// 当前正在处理的组内字符字节的序号，范围是0-7
    unsigned char nLeft;	// 上一字节残余的数据

        // 计数值初始化
    nSrc = 0;
    nDst = 0;

        // 将源串每8个字节分为一组，压缩成7个字节
        // 循环该处理过程，直至源串被处理完
        // 如果分组不到8字节，也能正确处理
    while (nSrc < nSrcLength)
    {
            // 取源字符串的计数值的最低3位
        nChar = nSrc & 7;

            // 处理源串的每个字节
        if(nChar == 0)
        {
                // 组内第一个字节，只是保存起来，待处理下一个字节时使用
           nLeft = *pSrc;
        }
        else
        {
                // 组内其它字节，将其右边部分与残余数据相加，得到一个目标编码字节
            *pDst = (*pSrc << (8-nChar)) | nLeft;
                // 将该字节剩下的左边部分，作为残余数据保存起来
            nLeft = *pSrc >> nChar;
                // 修改目标串的指针和计数值
            pDst++;
            nDst++;
        }

            // 修改源串的指针和计数值
        pSrc++;
        nSrc++;
    }
        // 返回目标串长度
    return nDst;
}

int Gsms::gsmDecode7bit(const unsigned char *pSrc, char *pDst, int nSrcLength)
{
    int nSrc;		// 源字符串的计数值
    int nDst;		// 目标解码串的计数值
    int nByte;		// 当前正在处理的组内字节的序号，范围是0-6
    unsigned char nLeft;	// 上一字节残余的数据

    // 计数值初始化
    nSrc = 0;
    nDst = 0;

    // 组内字节序号和残余数据初始化
    nByte = 0;
    nLeft = 0;

    // 将源数据每7个字节分为一组，解压缩成8个字节
    // 循环该处理过程，直至源数据被处理完
    // 如果分组不到7字节，也能正确处理
    while(nSrc<nSrcLength)
    {
        // 将源字节右边部分与残余数据相加，去掉最高位，得到一个目标解码字节
        *pDst = ((*pSrc << nByte) | nLeft) & 0x7f;

        // 将该字节剩下的左边部分，作为残余数据保存起来
        nLeft = *pSrc >> (7-nByte);

        // 修改目标串的指针和计数值
        pDst++;
        nDst++;

        // 修改字节计数值
        nByte++;

        // 到了一组的最后一个字节
        if(nByte == 7)
        {
            // 额外得到一个目标解码字节
            *pDst = nLeft;

            // 修改目标串的指针和计数值
            pDst++;
            nDst++;

            // 组内字节序号和残余数据初始化
            nByte = 0;
            nLeft = 0;
        }

        // 修改源串的指针和计数值
        pSrc++;
        nSrc++;
    }

    // 输出字符串加个结束符
    *pDst = '\0';

    // 返回目标串长度
    return nDst;
}

int Gsms::gsmEncode8bit(const char *pSrc, unsigned char *pDst, int nSrcLength)
{
    memcpy(pDst, pSrc, nSrcLength);
    return nSrcLength;
}

int Gsms::gsmDecode8bit(const unsigned char *pSrc, char *pDst, int nSrcLength)
{
    memcpy(pDst, pSrc, nSrcLength);
        // 输出字符串加个结束符
    *pDst = '\0';
    return nSrcLength;
}

int Gsms::gsmEncodeUcs2(const char *pSrc, unsigned char *pDst, int nSrcLength)
{
    QString pstr = QString::fromLocal8Bit(pSrc);
    wchar_t wchar[128];
   int nDstLength = pstr.toWCharArray(&wchar[0]);
   // 高低字节对调，输出
   for(int i=0; i<nDstLength; i++)
   {
       *pDst++ = wchar[i] >> 8;		// 先输出高位字节
       *pDst++ = wchar[i] & 0xff;		// 后输出低位字节
   }
   return nDstLength*2;
}

int Gsms::gsmDecodeUcs2(const unsigned char *pSrc, char *pDst, int nSrcLength)
{
    wchar_t wchar[128];	// UNICODE串缓冲区

        // 高低字节对调，拼成UNICODE
    for(int i=0; i<nSrcLength/2; i++)
    {
        wchar[i] = *pSrc++ << 8;	// 先高位字节
        wchar[i] |= *pSrc++;		// 后低位字节
    }
    QString pstr = QString::fromWCharArray(&wchar[0]);
    const char * cpdst = pstr.toStdString().c_str();
    for(int i=0;i<pstr.toStdString().length();++i)
    {
        *pDst++=cpdst[i];
    }
    pDst[pstr.toStdString().length()]='\0';
    return pstr.toStdString().length();
}

int Gsms::gsmInvertNumbers(const char *pSrc, char *pDst, int nSrcLength)
{
    int nDstLength;		// 目标字符串长度
    char ch;			// 用于保存一个字符

        // 复制串长度
    nDstLength = nSrcLength;

        // 两两颠倒
    for(int i=0; i<nSrcLength;i+=2)
    {
        ch = *pSrc++;		// 保存先出现的字符
        *pDst++ = *pSrc++;	// 复制后出现的字符
        *pDst++ = ch;		// 复制先出现的字符
    }

        // 源串长度是奇数吗？
    if(nSrcLength & 1)
    {
        *(pDst-2) = 'F';	// 补'F'
        nDstLength++;		// 目标串长度加1
    }

        // 输出字符串加个结束符
    *pDst = '\0';
        // 返回目标字符串长度
    return nDstLength;
}

int Gsms::gsmSerializeNumbers(const char *pSrc, char *pDst, int nSrcLength)
{
    int nDstLength;		// 目标字符串长度
    char ch;			// 用于保存一个字符

    // 复制串长度
    nDstLength = nSrcLength;

    // 两两颠倒
    for(int i=0; i<nSrcLength;i+=2)
    {
        ch = *pSrc++;		// 保存先出现的字符
        *pDst++ = *pSrc++;	// 复制后出现的字符
        *pDst++ = ch;		// 复制先出现的字符
    }

    // 最后的字符是'F'吗？
    if(*(pDst-1) == 'F')
    {
        pDst--;
        nDstLength--;		// 目标字符串长度减1
    }

    // 输出字符串加个结束符
    *pDst = '\0';

    // 返回目标字符串长度
    return nDstLength;
}

int Gsms::gsmEncodePdu(const SM_PARAM *pSrc, char *pDst)
{
    int nLength;			// 内部用的串长度
    int nDstLength;			// 目标PDU串长度
    unsigned char buf[256];	// 内部用的缓冲区

    // SMSC地址信息段
    nLength = strlen(pSrc->SCA);	// SMSC地址字符串的长度
    buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;	// SMSC地址信息长度
    buf[1] = 0x91;		// 固定: 用国际格式号码
    nDstLength = gsmBytes2String(buf, pDst, 2);		// 转换2个字节到目标PDU串
    nDstLength += gsmInvertNumbers(pSrc->SCA, &pDst[nDstLength], nLength);	// 转换SMSC号码到目标PDU串

    // TPDU段基本参数、目标地址等
    nLength = strlen(pSrc->TPA);	// TP-DA地址字符串的长度
    buf[0] = 0x11;					// 是发送短信(TP-MTI=01)，TP-VP用相对格式(TP-VPF=10)
    buf[1] = 0;						// TP-MR=0
    buf[2] = (char)nLength;			// 目标地址数字个数(TP-DA地址字符串真实长度)
    buf[3] = 0x91;					// 固定: 用国际格式号码
    nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 4);		// 转换4个字节到目标PDU串
    nDstLength += gsmInvertNumbers(pSrc->TPA, &pDst[nDstLength], nLength);	// 转换TP-DA到目标PDU串

    // TPDU段协议标识、编码方式、用户信息等
    nLength = strlen(pSrc->TP_UD);	// 用户信息字符串的长度
    buf[0] = pSrc->TP_PID;			// 协议标识(TP-PID)
    buf[1] = pSrc->TP_DCS;			// 用户信息编码方式(TP-DCS)
    buf[2] = 0;						// 有效期(TP-VP)为5分钟
    if(pSrc->TP_DCS == GSM_7BIT)
    {
        // 7-bit编码方式
        buf[3] = nLength;			// 编码前长度
        nLength = gsmEncode7bit(pSrc->TP_UD, &buf[4], nLength+1) + 4;	// 转换TP-DA到目标PDU串
    }
    else if(pSrc->TP_DCS == GSM_UCS2)
    {
        // UCS2编码方式
        buf[3] = gsmEncodeUcs2(pSrc->TP_UD, &buf[4], nLength);	// 转换TP-DA到目标PDU串
        nLength = buf[3] + 4;		// nLength等于该段数据长度
    }
    else
    {
        // 8-bit编码方式
        buf[3] = gsmEncode8bit(pSrc->TP_UD, &buf[4], nLength);	// 转换TP-DA到目标PDU串
        nLength = buf[3] + 4;		// nLength等于该段数据长度
    }
    nDstLength += gsmBytes2String(buf, &pDst[nDstLength], nLength);		// 转换该段数据到目标PDU串

    // 返回目标字符串长度
    return nDstLength;
}

int Gsms::gsmDecodePdu(const char *pSrc, SM_PARAM *pDst)
{
    int nDstLength;			// 目标PDU串长度
    unsigned char tmp;		// 内部用的临时字节变量
    unsigned char buf[256];	// 内部用的缓冲区

    // SMSC地址信息段
    gsmString2Bytes(pSrc, &tmp, 2);	// 取长度
    tmp = (tmp - 1) * 2;	// SMSC号码串长度
    pSrc += 4;			// 指针后移，忽略了SMSC地址格式
    gsmSerializeNumbers(pSrc, pDst->SCA, tmp);	// 转换SMSC号码到目标PDU串
    pSrc += tmp;		// 指针后移

    // TPDU段基本参数
    gsmString2Bytes(pSrc, &tmp, 2);	// 取基本参数
    pSrc += 2;		// 指针后移

    // 取回复号码
    gsmString2Bytes(pSrc, &tmp, 2);	// 取长度
    if(tmp & 1) tmp += 1;	// 调整奇偶性
    pSrc += 4;			// 指针后移，忽略了回复地址(TP-RA)格式
    gsmSerializeNumbers(pSrc, pDst->TPA, tmp);	// 取TP-RA号码
    pSrc += tmp;		// 指针后移

    // TPDU段协议标识、编码方式、用户信息等
    gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_PID, 2);	// 取协议标识(TP-PID)
    pSrc += 2;		// 指针后移
    gsmString2Bytes(pSrc, (unsigned char*)&pDst->TP_DCS, 2);	// 取编码方式(TP-DCS)
    pSrc += 2;		// 指针后移
    gsmSerializeNumbers(pSrc, pDst->TP_SCTS, 14);		// 服务时间戳字符串(TP_SCTS)
    pSrc += 14;		// 指针后移
    gsmString2Bytes(pSrc, &tmp, 2);	// 用户信息长度(TP-UDL)
    pSrc += 2;		// 指针后移
    if(pDst->TP_DCS == GSM_7BIT)
    {
        // 7-bit解码
        nDstLength = gsmString2Bytes(pSrc, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// 格式转换
        gsmDecode7bit(buf, pDst->TP_UD, nDstLength);	// 转换到TP-DU
        nDstLength = tmp;
    }
    else if(pDst->TP_DCS == GSM_UCS2)
    {
        // UCS2解码
        nDstLength = gsmString2Bytes(pSrc, buf, tmp * 2);			// 格式转换
        nDstLength = gsmDecodeUcs2(buf, pDst->TP_UD, nDstLength);	// 转换到TP-DU
    }
    else
    {
        // 8-bit解码
        nDstLength = gsmString2Bytes(pSrc, buf, tmp * 2);			// 格式转换
        nDstLength = gsmDecode8bit(buf, pDst->TP_UD, nDstLength);	// 转换到TP-DU
    }

    // 返回目标字符串长度
    return nDstLength;

}

