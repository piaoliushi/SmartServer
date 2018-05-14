#ifndef   STRUCT_DEF_H
#define  STRUCT_DEF_H

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
using namespace std;


typedef enum RESOLVE_TAG  //resolve
{
    RE_SUCCESS    =  0, //解析正确
    RE_HEADERROR  = -1, //数据头错误
    RE_NOPROTOCOL = -2, //未定义协议
    RE_UNKNOWDEV  = -3, //未定义的设备类型
    RE_CMDACK     = -4, //命令回复，无需解析
}ResolveErrorCode;

typedef struct CMDBUF_TAG
{
	char* CmdData;//命令字节
	int Length;//命令长度
	int CmdNum;//命令个数
}CmdDataBuf,*pCmdDataBuf;
//命令类型
typedef enum CMDTYPE
{
	NO_CMDTYPE = -1,//未知命令
    CMD_QUERY  =  0,//查询命令
    CMD_OPEN   =  1,//开机命令
    CMD_CLOSE  =  2,//关机命令
	CMD_ADJUST_TIME = 3,//校时指令
	CMD_SWITCH_CHANNEL = 4,//切换通道指令
}CmdType,*pCmdType;

//单个数据结构

#define  LASTCMD     100
typedef struct CMDUnit
{
    CMDUnit()
	{
		commandLen=0;
		ackLen=0;
	}
	unsigned char commandId[128];
    string        sCommandId;
	int           commandLen;
	int           ackLen;
}CommandUnit;
//控制命令属性
typedef struct
{
    int ninterval;                     //命令之间发送时间间隔(ms)
    map<int,vector<CommandUnit> > mapCommand;//命令
}CommandAttribute;

typedef boost::shared_ptr<CommandAttribute>    CommandAttrPtr;


typedef struct
{
	unsigned char uHeadFlag[4]; //固定的命令头 48 58 44 51/4F/43
	//             H  X  D 查询/开机/关机
	int uiLength;  // 00 00 00 08 整个数据包的长度，包括该命令头部分.
}AckHeader,*pAckHeader;

//协议类型
typedef enum PROTOCOL
{
    BEIJ_GME              = 0,  //吉兆系列
    BEIJING_BEIGUANG      = 1,  //北广系列
    CHENGDU_KT_CG         = 2,  //成都成广、凯腾系列
    SHANGUANG             = 3,  //陕广系列
    HAGUANG               = 4,  //哈广系列
    ANSHAN                = 5,  //鞍山系列
    HANGCHUN              = 6,  //杭州杭醇系列
    HUIXIN                = 7,  //汇鑫系列
    ITALY_TECHNO          = 8,  //意大利TECHNO系列
    EKA                   = 9,  //EKA系列
    GE_RS                 = 10, //德国RS
    DLDZ                  = 11, //大连东芝
    EDA9033               = 12, //EDA电表
    WS2032                = 13, //温湿度计
    CHENGDU_KANGTE_N      = 14, //成都康特
    LIAONING_HS           = 15, //辽宁发射机厂
    SHANGHAI_MZ           = 16, //上海明珠
    HARRIS                = 17, //哈里斯
    DE_XIN                = 18, //德芯
    ELECTRIC_104          = 19, //电力104规约
    ANTENNA_CONTROL       = 20, //天线控制器
    TSC_TMIEDEV           = 21, //校时设备
    LINK_DEVICE           = 22, //链路设备
    SHANGHAI_ALL_BAND     = 23, //上海功放机器
    MEDIA_DEVICE          = 24, //媒体设备
    GLSQ                  = 26, //桂林思奇
    GSBR                  = 27, //高斯贝尔
    ZHC                   = 29, //众传
}Protocol,*pProtocol;

typedef enum ANTENAPROTOCOL
{
    HX_MD981 = 0,
}AntennaSubProtocol;

typedef enum GMEPROTOCOL
{
	BEIJING_JIZHAO_FM     = 0, //吉兆调频 1024	
	BEIJING_JIZHAO_DIGTV  = 1, //吉兆数字电视
    BEIJING_JIZHAO_SIMTV  = 2, //吉兆模拟电视
    GME_SIMTV1014A        = 3, //吉兆1014A模拟电视发射机 256
    GME_SIMTV_MWAVE       = 4, //吉兆分米波调频电视1kw 256
    GME_FEN_MWAVE3KW      = 5, //吉兆分米波调频电视3kw 256
    GME_MMAVE1033_3KW     = 6, //吉兆米波调频电视3kw,1033系列 256
	GME_1F33E             = 7,
    GME_SIMTV             = 8, //吉兆模拟电视取全部数据
    GME_DIGTV             = 9, //吉兆数字国标
	GME_DIGTV_6           = 10,
    GME_CDR_1KW           = 11,//cdr1kw
    GME_CDR_10KW          = 12,
    GME_GB_300W           = 13,
}GmeSubProtocol,*pGmeSubProtocol;

typedef enum BGPROTOCOL
{
	BEIJING_BEIGUANG_300W = 0,
	BEIGUANG_FM_3KW       = 1,
	BEIGUANG_TV_1KW       = 2,
	BEIGUANG_FM_1KW       = 3,
	BEIGUANG_AM_10KW      = 4,
	BEIJING_BEIGUANG_100W = 5,
	BEIGUANG_FM_5KW       = 6,
	BEIGUANG_FM_10KW      = 7,
	BEIGUANG_AM_1KW       = 8,
}BgSubProtocol,*pBgSubProtocol;

typedef enum CDPROTOCOL
{
    CHENGDU_KAITENG         = 0,//凯腾非300瓦
    CHENGDU_KAITENG_300W    = 1,
    CHENGDU_CHENGGUANG      = 2,
    CHENGDU_KT_DIG          = 3,
    CHENGDU_KAITENG_TV10KW  = 4,
    CHENGDU_XINGUANG        = 5,
    CHENGDU_XINGUANG_247    = 6,
    CHENGDU_CHENGGUANG_DIG  = 7,//成广1Kw数字
    CHENGDU_KAITENG_KFS_II  = 8,//凯腾KFS-II-813 1Kw
    CHENGDU_KAITENG_50W_AMP = 9,//50W功放直放站

}CdSubProtocol;

typedef enum SGPROTOCOL
{
	SHANXI_762_3_5KW      = 0,//陕西762 3/5KW
	SHANXI_762_10KW       = 1,//陕西762 10KW
	SHANGUANG_FM_300W     = 2,//陕西广播100/300W
	SHANGUANG_AM_50KW     = 3,//陕广中波50kw
	SHANXI_CD_3_5KW       = 4,//陕西762 3/5KW川大协议
    SHANXI_AM_10KW        = 5,//中波10Kw
    SHANXI_3KW_PDM        = 6 //如意3KwPDM
}SgSubProtocol;

typedef enum HGPROTOCOL
{
	HAERBIN_HG_50kw       = 0,//哈尔滨50KW
	HAERBIN_HG_100kw      = 1,//哈尔滨100KW
    HAERBIN_HG_10kw       = 2,//哈尔滨10KW
    HAERBIN_HGEX_10kw     = 3,//哈尔滨10KW 扩展
}HgSubProtocol;

typedef enum ASPROTOCOL
{
	ANSHAN_JIAHUI         = 0,//鞍山家惠1KW
	ANSHAN_JIAHUI_3KW_TV  = 1,//鞍山家惠通用电视3KW
}AsSubProtocol;

typedef enum HCPROTOCOL
{
	HangChun_HCF5KW       = 0,//杭州杭醇5KW
    HC_CDR_100_300W       = 1,//杭州杭醇CDR100/300W
    HC_CDR_1KW            = 2,//杭州杭醇CDR1Kw
    HC_CDR_3KW            = 3,//杭州杭醇CDR3Kw
    HC_CDR_10KW           = 4,//杭州杭醇CDR10Kw
}HcSubProtocol;

typedef enum HXPROTOCOL
{
	HUIXIN_993            = 0,//汇鑫993数据采集单元
	ANHUI_994             = 1,//安徽汇鑫994NEW
    HUIXIN_996            = 2,

}HxSubPrototcol;

typedef enum ITPROTOCOL
{
	ITALY_TECHNO_S        = 0,//意大利TECHNO
	TECHNO_UNICOM         = 1,//TECHNO系列 UNICOM发射机
	ELENOS                = 2,//ELENOS
}ItSubProtocol;

//意大利EKA
typedef enum EKAPROTOCOL
{
	EKA_3KFM              = 0,//eka 3kw
	EKA_6KFM              = 1,
	EKA_MIRA_FM           = 2,
}EkaSubProtocol;

//德国RS
typedef enum RSPROTOCOL
{
	GERMANY_RS            = 0,//德国RS
    GER_DIG_ONE           = 1,//RS数字发射机，单频点
    GER_DIG_TWO           = 2,//RS数字发射机，双频点
}RsSubProtocol;

//大连东芝
typedef enum DZPROTOCOL
{
	DLDZ_SIMTV            = 0,//大连东芝
	DLDZ_SIMTV2           = 1,//大连东芝(包含11个功放)
}DzSubProtocol;

//EDA
typedef enum EDAPROTOCOL
{
	Eda9033_A            = 0,//9033A型电表
	Eda9003_F            = 1,//9033F
	ST_UPS_3C3           = 2,//山特UPS
	C2000_MH08           = 3,//康耐德电路电压检测
 	STATIC_SWITCH        = 4,//静态转换开关
	ZXJY_BACK            = 5,//中心继远
    PAINUO_SPM33         = 6,//珠海派诺SPM33电表
    KSTAR_UPS            = 7,//科士达UPS
    YINGJIA_EM400        = 8,//广州盈嘉EM400电表
    YISITE_EA66          = 9,//易事特UPS
    ACR_NET_EM           = 10,//安科瑞网络电力仪表
    HX_ELEC_CTR          = 11,//汇鑫电源控制
    ACR_PZ               = 12,//安科瑞网络电力仪表
    ACR_ARD_2L           = 13,//安科瑞网络电力仪表
    ST_UPS_C6_20         = 14,//山特c6
    DSE_7320_POWER       = 15,//DSE7320发电机
    ABB_104              = 100,//ABB104电表
}EdaSubProtocol;

//环境设备
typedef enum WSPROTOCOL
{
	WS2032_A            = 0,
	THB11RS             = 1,
	AC_103_CTR          = 2,
	KD40_IO             = 3,
	C2000_M21A          = 4,
	FRT_X06A            = 5,
    C2000_A2_8020       = 6,
    C2000_SDD8020_BB3   = 7,
    NT511_AD            = 8,

}WsSubProtocol;

//康特
typedef enum KANGTEPROTOCOL
{
	KT_TV_Netport      = 0,//康特电视网口
    KT_HTTP            = 1,//康特http
}KangTeSubProtocol;

//辽宁数字
typedef enum LIAOPROTOCOL
{
	LNSZ_HS           = 0,//辽宁数字HSFM-3 3KW   
    LNSZ_10_5Kw       = 1,//辽宁数字调频5/10Kw
    LNSZ_169          = 2,//辽宁dtmb
}LngdSubProtocol;

//上海明珠
typedef enum SHANGHAIPROTOCOL
{
	MZ_DTV1000        = 0,//明珠数字电视1000
}MzSubProtocol;

//哈里斯
typedef enum HarPROTOCOL
{
    HARRIS_AM50       = 0,//哈里斯中波50Kw
    HARRIS_ZXB        = 1,//
    HARRIS_SNMP       = 2,//SNMP协议
}HarSubPrototcol;

//德鑫
typedef enum DxPROTOCOL
{
    DX_DUT_8413 = 0,
    DX_DIG_TV   = 1
}DxSubProtocol;


typedef enum TIMEROTOCOL
{
    TFS_001 = 0
}TimeSubProtocol;

//链路设备
typedef enum LINK_DEVICE
{
    LINK_STATELITE_RECEVIE  = 0,//卫星接收机
    LINK_TEST_RECEVIE       = 3,//SNMP测试
    LINK_SING_NET_ADAPTER   = 4,//网络适配器
    LINK_DMP_SWITCH         = 5,//dmp切换器
    LINK_ASI_ADAPTER        = 8,//asi适配器
    LINK_WEILE_AVSP_DECODER = 9,//伟乐avs+接收解码器
    LINK_HX_0401_AV         = 10,//汇鑫0401AV+切换器
    LINK_HX_0401_DA         = 11,//汇鑫0401DA切换器
    LINK_HX_0401_DABS       = 12,//汇鑫0401DABS切换器
    LINK_WEILE_AVSP_ADAPTER = 13,//伟乐avs+适配器
    LINK_SMSX_ASI_ADAPTER   = 14,//数码视讯ASI复用器
    LINK_SMSX_ASI_ENCODER   = 15,//数码视讯ASI编码码器
    LINK_NORMAL_SNMP_DEV    = 16,//通用snmp设备
}LinkDeviceSubProtocol;

//上海全波
typedef enum SHANGHAIPROTOCL
{
    All_Band_Pa  = 0,
    All_Band_Exc = 1,
}ShSubProtocol;

//汇鑫媒体设备
typedef enum  MEDIASUBPROTOCL_TAG
{
    MEDIA_DTMB = 0,
    MD_740P    = 1,//汇鑫MD740+
    MD_740BD_II= 2,//汇鑫MD740两路板卡
    MD_760BD_IV= 3,//汇鑫MD760板卡
    DTMB_BD    = 4,//DTMB板卡

}MediaSubProtocol;
//桂林思奇
typedef enum GLSQ_TAG
{
    GLSQ_T = 0,//思奇T型发射机
}GlsqSubProtocol;

//高斯贝尔
typedef enum GSBR_TAG
{
    GSBR_SNMP = 0,
}GsbrSubProtocol;

//众传
typedef enum ZHC_TAG
{
    ZHC_618F   = 0,
    ZHC_10KWTV = 1,
    ZHC_3KWFM  = 2
}ZhcSybProtocol;

#endif
