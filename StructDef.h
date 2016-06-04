#ifndef   STRUCT_DEF_H
#define  STRUCT_DEF_H

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
using namespace std;



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
	CMD_QUERY  = 0,//查询命令
	CMD_OPEN   = 1,//开机命令
	CMD_CLOSE  = 2,//关机命令
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
	int           commandLen;
	int           ackLen;
}CommandUnit;
//控制命令属性
typedef struct
{
    /*vector<CommandUnit> queryComm;     //查询命令(可能多条)
	vector<CommandUnit> turnonComm;    //开机命令(可能多条)
	vector<CommandUnit> turnoffComm;   //关机命令(可能多条)
	vector<CommandUnit> uppowerComm;   //升功率命令(可能多条)
	vector<CommandUnit> lowpowerComm;  //降功率命令(可能多条)
	vector<CommandUnit> resetComm;     //复位命令(可能多条)
	vector<CommandUnit> switchComm;    //切换激励器1命令(可能多条)
    vector<CommandUnit> switch2Comm;   //切换激励器2率命令(可能多条)*/
	//vector<CommandUnit> adjustDevTime; //调整设备时间

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
	BEIJ_GME                      = 0, //吉兆系列
	BEIJING_BEIGUANG     = 1, //北广系列
	CHENGDU_KT_CG       = 2, //成都成广、凯腾系列
	SHANGUANG               = 3, //陕广系列
	HAGUANG                    = 4, //哈广系列
	ANSHAN                       = 5, //鞍山系列
	HANGCHUN                 = 6, //杭州杭醇系列
	HUIXIN                           = 7, //汇鑫系列
	ITALY_TECHNO            = 8, //意大利TECHNO系列
	EKA                                 = 9, //EKA系列
	GE_RS                             = 10,//德国RS
	DLDZ                               = 11,//大连东芝
	EDA9033                        = 12,//EDA电表
	WS2032                          = 13,//温湿度计
	CHENGDU_KANGTE_N = 14,//成都康特
	LIAONING_HS                = 15,//辽宁发射机厂
	SHANGHAI_MZ             = 16,//上海明珠
	HARRIS                           = 17, //哈里斯
	DE_XIN                            = 18, //德芯
    ELECTRIC                         =19,  //电力规约
    ANTENNA_CONTROL          = 20, //天线控制器
    TSC_TMIEDEV                  = 21, //校时设备
    LINK_DEVICE                   = 22,  //链路设备
    SHANGHAI_ALL_BAND      = 23,   //上海功放机器
    MEDIA_DEVICE                 = 24   //媒体设备

}Protocol,*pProtocol;

typedef enum ANTENAPROTOCOL
{
    HX_MD981 = 0,
}AntennaSubProtocol;

typedef enum GMEPROTOCOL
{
	BEIJING_JIZHAO_FM     = 0, //吉兆调频 1024	
	BEIJING_JIZHAO_DIGTV  = 1, //吉兆数字电视
	BEIJING_JIZHAO_SIMTV  = 2, 
	GME_SIMTV1014A        = 3,//吉兆1014A模拟电视发射机 256
	GME_SIMTV_MWAVE       = 4,//吉兆分米波调频电视1kw 256
	GME_FEN_MWAVE3KW      = 5,//吉兆分米波调频电视3kw 256	
	GME_MMAVE1033_3KW     = 6,//吉兆米波调频电视3kw,1033系列 256	
	GME_1F33E             = 7,
	GME_SIMTV             = 8,//吉兆模拟电视取全部数据
	GME_DIGTV             = 9, //吉兆数字国标
	GME_DIGTV_6           = 10,
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
	CHENGDU_KAITENG       = 0,//凯腾非300瓦
	CHENGDU_KAITENG_300W  = 1,
	CHENGDU_CHENGGUANG    = 2,
	CHENGDU_KT_DIG        = 3,
	CHENGDU_KAITENG_TV10KW= 4,
    CHENGDU_XINGUANG      = 5,
    CHENGDU_XINGUANG_247 = 6,
}CdSubProtocol;
typedef enum SGPROTOCOL
{
	SHANXI_762_3_5KW      = 0,//陕西762 3/5KW
	SHANXI_762_10KW       = 1,//陕西762 10KW
	SHANGUANG_FM_300W     = 2,//陕西广播100/300W
	SHANGUANG_AM_50KW     = 3,//陕广中波50kw
	SHANXI_CD_3_5KW       = 4,//陕西762 3/5KW川大协议
    SHANXI_AM_10KW        = 5 //中波10Kw
}SgSubProtocol;
typedef enum HGPROTOCOL
{
	HAERBIN_HG_50kw       = 0,//哈尔滨50KW
	HAERBIN_HG_100kw      = 1,//哈尔滨100KW
}HgSubProtocol;
typedef enum ASPROTOCOL
{
	ANSHAN_JIAHUI         = 0,//鞍山家惠1KW
	ANSHAN_JIAHUI_3KW_TV  = 1,//鞍山家惠通用电视3KW
}AsSubProtocol;
typedef enum HCPROTOCOL
{
	HangChun_HCF5KW       = 0,//杭州杭醇5KW
}HcSubProtocol;
typedef enum HXPROTOCOL
{
	HUIXIN_993            = 0,//汇鑫993数据采集单元
	ANHUI_994             = 1,//安徽汇鑫994NEW
	HUIXIN_JWT            = 2,//金网通
	HUIXIN_HN             = 3,//海纳
	HUIXIN_992            = 4,//环境采集设备
	HUIXIN_760            = 5,//
	HUIXIN_0401_AV        = 6,
	HUIXIN_0401_DA        = 7,
	HUIXIN_0401_SKY       = 8,
	HUIXIN_DATA_MGR       = 9, //综合数据管理器
	HUIXIN_993_EX         = 10,
	HUIXIN_6300           = 11, //音频处理器
	HUIXIN_9020           = 12, //多路调幅度测试仪
	HUIXIN_740P           = 13, //740+
	HUIXIN_730P           = 14, //730+
	HUIXIN_761            = 15, //761(新版760)
	HUIXIN_0214           = 16,  //数字切换器
	HUIXIN_0401_DABS      = 17,
	HUIXIN_0804D          = 18 //矩阵

}HxSubPrototcol;
typedef enum ITPROTOCOL
{
	ITALY_TECHNO_S        = 0,//意大利TECHNO
	TECHNO_UNICOM         = 1,//TECHNO系列 UNICOM发射机
	ELENOS                = 2,//ELENOS
}ItSubProtocol;
typedef enum EKAPROTOCOL
{
	EKA_3KFM              = 0,//eka 3kw
	EKA_6KFM              = 1,
	EKA_MIRA_FM           = 2,
}EkaSubProtocol;
typedef enum RSPROTOCOL
{
	GERMANY_RS            = 0,//德国RS
}RsSubProtocol;
typedef enum DZPROTOCOL
{
	DLDZ_SIMTV            = 0,//大连东芝
	DLDZ_SIMTV2           = 1,//大连东芝(包含11个功放)
}DzSubProtocol;
typedef enum EDAPROTOCOL
{
	Eda9033_A            = 0,//9033A型电表
	Eda9003_F            = 1,//9033F
	ST_UPS_3C3           = 2,//山特UPS
	C2000_MH08           = 3,//康耐德电路电压检测
 	STATIC_SWITCH        = 4,//静态转换开关
	ZXJY_BACK            = 5,//中心继远
}EdaSubProtocol;
typedef enum WSPROTOCOL
{
	WS2032_A            = 0,
	THB11RS             = 1,
	AC_103_CTR          = 2,
	KD40_IO             = 3,
	C2000_M21A          = 4,
	FRT_X06A            = 5,
}WsSubProtocol;
typedef enum KANGTEPROTOCOL
{
	KT_TV_Netport      = 0,//康特电视网口
}KangTeSubProtocol;
typedef enum LIAOPROTOCOL
{
	LNSZ_HS           = 0,//辽宁数字HSFM-3 3KW   
    LNSZ_10_5Kw    = 1,//辽宁数字调频5/10Kw
    LNSZ_169         =2,//辽宁dtmb
}LngdSubProtocol;
typedef enum SHANGHAIPROTOCOL
{
	MZ_DTV1000        = 0,//明珠数字电视1000
}MzSubProtocol;
typedef enum HarPROTOCOL
{
	HARRIS_AM50       = 0,//哈里斯中波50Kw
}HarSubPrototcol;

typedef enum DxPROTOCOL
{
	DX_DUT_8413 = 0
}DxSubProtocol;


typedef enum ElECTRICPROTOCOL
{
	ELECTRIC_104     = 0, 
	ELECTRIC_101     = 1, 
}ElectricSubProtocol,*pElectricSubProtocol;

typedef enum TIMEROTOCOL
{
    TFS_001 = 0
}TimeSubProtocol;

typedef enum LINK_DEVICE
{
    LINK_STATELITE_RECEVIE = 0,//卫星接收机
}LinkDeviceSubProtocol;

typedef enum RESOLVE_TAG  //resolve
{
    RE_SUCCESS = 0,  //解析正确
    RE_CMDACK  = -1, //命令回复，无需解析
    RE_NOPROTOCOL = -2,//未定义协议
    RE_UNKNOWDEV  = -3,//未定义的设备类型
    RE_HEADERROR  = -4,//数据头错误
}ResolveErrorCode;

typedef enum SHANGHAIPROTOCL
{
    All_Band_Pa = 0,
}ShSubProtocol;

typedef enum  MEDIASUBPROTOCL_TAG
{
    MEDIA_DTMB = 0,
}MediaSubProtocol;

#endif
