#include "Link_message.h"
#include"../../../utility.h"
#include "snmp_pp.h"

#define NUM_SYS_VBS	6
//信号电平
#define rflevel	"1.3.6.1.4.1.8201.5.7.1.1.4.0"
//锁定状态
#define signalLock	"1.3.6.1.4.1.8201.5.7.1.1.1.0"
//误码率
#define singalber	"1.3.6.1.4.1.8201.5.7.1.1.2.0"
//载噪比
#define signalcn	"1.3.6.1.4.1.8201.5.7.1.1.3.0"
//统计码率
#define totalrate    "1.3.6.1.4.1.8201.5.7.1.1.5.0"
//频率
#define frequency	"1.3.6.1.4.1.8201.5.7.1.1.7.0"

namespace hx_net
{

    Link_message::Link_message(session_ptr pSession,DeviceInfo &devInfo)
		:m_pSession(pSession)
        ,d_devInfo(devInfo)
	{
	}

    Link_message::~Link_message(void)
	{
	}
    int Link_message::check_msg_header(unsigned char *data,int nDataLen,CmdType cmdType,int number)
	{
        return RE_UNKNOWDEV;
	}

     int Link_message::decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target)
     {
         switch(d_devInfo.nDevProtocol)
         {
         case LINK_DEVICE:
             {
                 switch (d_devInfo.nSubProtocol)
                 {
                 case LINK_STATELITE_RECEVIE:{
                     return parse_SatelliteReceive_data(snmp,data_ptr,target);
                     }
                     break;
                 default:
                     return RE_NOPROTOCOL;
                 }
             }
             break;
         }
         return RE_UNKNOWDEV;
     }

    int Link_message::decode_msg_body(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen)
	{
		return RE_UNKNOWDEV;
	}

    bool Link_message::IsStandardCommand()
	{
		return false;
	}
	
    void Link_message::GetSignalCommand(devCommdMsgPtr lpParam,CommandUnit &cmdUnit)
	{
	}

    void Link_message::GetAllCmd( CommandAttribute &cmdAll )
	{
    }

    int  Link_message::parse_SatelliteReceive_data(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target)
    {
        Pdu pdu;
        Vb vbl[NUM_SYS_VBS];
        vbl[0].set_oid(rflevel);
        vbl[1].set_oid(signalLock);
        vbl[2].set_oid(singalber);
        vbl[3].set_oid(signalcn);
        vbl[4].set_oid(totalrate);
        vbl[5].set_oid(frequency);
        for (int i=0; i<NUM_SYS_VBS;i++)
            pdu += vbl[i];
        int status = snmp->get(pdu, *target);
        if (status){
            return -1;
        }else{
            int vbcount = pdu.get_vb_count();
            int index = 0; //解析位置
            DataInfo dainfo;
            if ( vbcount == NUM_SYS_VBS ) {
                pdu.get_vblist(vbl, vbcount);
                 string s=vbl[0].get_printable_value();
                 dainfo.sValue = s;
                 dainfo.bType=false;
                 dainfo.fValue =  atof(s.c_str());
                 data_ptr->mValues[index++] = dainfo;

                dainfo.bType=true;
                s=vbl[1].get_printable_value();
                int nRlt = s.find("CB F8 B6 A8");
                if(nRlt!=-1){//信号锁
                    dainfo.sValue = "1";
                    dainfo.fValue=1.0f;
                }
                else{
                    dainfo.sValue = "0";
                    dainfo.fValue=0.0f;
                }
                data_ptr->mValues[index++] = dainfo;

                dainfo.bType=false;
                s=vbl[2].get_printable_value();
                dainfo.sValue = s;
                string_replace(s,"dB","");
                string_replace(s," ","");
                if(s=="N/A")
                    dainfo.fValue=0.0f;
                else{
                    dainfo.fValue =  atof(s.c_str());
                }
                data_ptr->mValues[index++] = dainfo;

                dainfo.bType=false;
                s=vbl[3].get_printable_value();
                dainfo.sValue = s;
                string_replace(s,"dB","");
                string_replace(s," ","");
                dainfo.fValue = atof(s.c_str());
                data_ptr->mValues[index++] = dainfo;

                s=vbl[4].get_printable_value();
                dainfo.sValue = s;
                dainfo.fValue = atof(s.c_str());
                data_ptr->mValues[index++] = dainfo;

                s=vbl[5].get_printable_value();
                dainfo.sValue = s;
                string_replace(s,"MHz","");
                string_replace(s," ","");
                dainfo.fValue = atof(s.c_str());
                data_ptr->mValues[index++] = dainfo;

                return 0;
            }
       }
        return -1;

    }

}
