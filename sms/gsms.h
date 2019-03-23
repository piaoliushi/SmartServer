#ifndef GSMS_H
#define GSMS_H

#include <QObject>
#define GSM_7BIT		0
#define GSM_8BIT		4
#define GSM_UCS2		8

// 应答状态
#define GSM_WAIT		0		// 等待，不确定
#define GSM_OK			1		// OK
#define GSM_ERR			-1		// ERROR

#include <QString>
#include <QtSerialPort/QSerialPort>
#include <boost/shared_ptr.hpp>

#include <boost/thread.hpp>
#include <QTimer>
using namespace std;
#define MAX_SM_SEND		128		// 发送队列长度
#define MAX_SM_RECV		128		// 接收队列长度
// 短消息参数结构，编码/解码共用
// 其中，字符串以'\0'结尾
typedef struct {
    char SCA[16];			// 短消息服务中心号码(SMSC地址)
    char TPA[16];			// 目标号码或回复号码(TP-DA或TP-RA)
    char TP_PID;			// 用户信息协议标识(TP-PID)
    char TP_DCS;			// 用户信息编码方式(TP-DCS)
    char TP_SCTS[16];		// 服务时间戳字符串(TP_SCTS), 接收时用到
    char TP_UD[160];		// 原始用户信息(编码前或解码后的TP-UD)
    short index;			// 短消息序号，在读取时用到
} SM_PARAM;

// 读取应答的缓冲区
typedef struct {
    int len;
    char data[16384];
} SM_BUFF;

class Gsms : public QObject
{
    Q_OBJECT
public:
    enum {
        stUnknow = -1,
        stBeginRest = 0,				// 开始休息/延时
        stContinueRest,				// 继续休息/延时
        stSendMessageRequest,		// 发送短消息
        stSendMessageResponse,		// 读取短消息列表到缓冲区
        stSendMessageWaitIdle,		// 发送不成功，等待GSM就绪
        stReadMessageRequest,		// 发送读取短消息列表的命令
        stReadMessageResponse,		// 读取短消息列表到缓冲区
        stDeleteMessageRequest,		// 删除短消息
        stDeleteMessageResponse,	// 删除短消息
        stDeleteMessageWaitIdle,	// 删除不成功，等待GSM就绪
        stReadCSCAack,              // 读短信中心号码返回
        stIsHaveModelAck,
        stAteAck,
        stCmgfAck,
        stSmsssAck,
        stRssirep_Ack,
        stExitThread				// 退出
    } nState;				// 处理过程的状态
    enum{
        tyGsm    = 0,//通用GSM
        tyHwCdma = 1,//华为CDMA323
        tyWt     = 2,//旺通中间件
    }SmsType;
    explicit Gsms(int mdtype=0,QObject *parent = 0);
    ~Gsms();
    bool OpenCom(int nPort,int nBaudRate=57600);
    bool CloseComm();
    int ReadComm(char* pData, int nLength,int msec=5000);
    int WriteComm(const char* pData, int nLength);
    bool gsmInit();

    void PutSendMessage(SM_PARAM* pSmParam);	// 将短消息放入发送队列
    bool GetSendMessage(SM_PARAM* pSmParam);	// 从发送队列中取一条短消息
    void PutRecvMessage(SM_PARAM* pSmParam, int nCount);	// 将短消息放入接收队列
    bool GetRecvMessage(SM_PARAM* pSmParam);	// 从接收队列中取一条短消息
    void SendSMSContent(string sSmsc,string PhoneNumber,string AlarmContent);
    void SmThread();	// 短消息收发处理子线程
    void GetCenterNumber();
    void Run();

    int gsmSendMessage(SM_PARAM* pSrc);

    int gsmReadMessageList();
    int gsmDeleteMessage(int index);
    int gsmGetResponse(SM_BUFF* pBuff);
    int gsmParseMessageList(SM_PARAM* pMsg, SM_BUFF* pBuff);
    bool IsRun(){boost::recursive_mutex::scoped_lock lock(m_run_mutex);
                 return b_run_;}
    int GetModleType(){return modle_type;}
signals:
 //   void S_have_model(bool bhave);
 //   void S_init_result(bool bSucces);
 //   void S_have_GSM_card(bool bhavegsm);
    //stype---0:设备有无;1:设备初始化；2：是否有电话卡;3:短信是否发送成功
    void S_state(int stype,bool bresult);
public slots:
    void comrecive();
    void check_timer();
    void try_timer();

private://GSM
    void GsmComdata(int istate);
    void get_sendmsg_cmd_ack();
    void get_Response_cmd_ack();
    void get_CSCA_cmd_ack();
    void get_have_cmd_ack();
    void get_ate_cmd_ack();
    void get_cmgf_cmd_ack();
    int gsmBytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength);
    int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength);
    int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength);
    int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength);
    int gsmEncode8bit(const char* pSrc, unsigned char* pDst, int nSrcLength);
    int gsmDecode8bit(const unsigned char* pSrc, char* pDst, int nSrcLength);
    int gsmEncodeUcs2(const char* pSrc, unsigned char* pDst, int nSrcLength);
    int gsmDecodeUcs2(const unsigned char* pSrc, char* pDst, int nSrcLength);
    int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength);
    int gsmSerializeNumbers(const char* pSrc, char* pDst, int nSrcLength);
    int gsmEncodePdu(const SM_PARAM* pSrc, char* pDst);
    int gsmDecodePdu(const char* pSrc, SM_PARAM* pDst);
private://CDMA-HW
    void HwcdmaComdata(int istate);
    void get_have_cdma_ack();
    void get_rssirep_cmd_ack();
    void get_ate_cdma_ack();
    void get_cmgf_cdma_ack();
    void get_smsss_cmd_ack();
    void get_sysinfo_ack();
    void get_sendmsg_cdma_ack();
    void get_Response_cdma_ack();
private:
    int m_nSendIn;		// 发送队列的输入指针
    int m_nSendOut;		// 发送队列的输出指针

    int m_nRecvIn;		// 接收队列的输入指针
    int m_nRecvOut;		// 接收队列的输出指针

    SM_PARAM m_SmSend[MAX_SM_SEND];		// 发送短消息队列
    SM_PARAM m_SmRecv[MAX_SM_SEND];		// 接收短消息队列

    boost::recursive_mutex m_csSend_mutex;
    boost::recursive_mutex m_csRecv_mutex;
    boost::shared_ptr<QSerialPort> pQSerialport_ptr_;
    boost::shared_ptr<boost::thread> _SmThreadptr;
    boost::recursive_mutex m_run_mutex;
    bool b_run_;
    string str_SCA;//
    boost::recursive_mutex m_cmdresult_mutex;
    int n_cmdresult_;
    boost::recursive_mutex data_mutex;
    int nstate;
    char pdu[512];
    QTimer *pTimerOut_;
    QTimer *pTimerTryInit_;
    int nTrycount_;//尝试计数
    QByteArray cach_receive_;
    int modle_type;
    int m_nSendPduLen;
};

#endif // GSMS_H
