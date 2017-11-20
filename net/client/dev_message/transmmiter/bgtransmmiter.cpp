#include "bgtransmmiter.h"
namespace hx_net
{
   BgTransmmiter::BgTransmmiter(int subprotocol,int addresscode)
       :Transmmiter()
       ,m_subprotocol(subprotocol)
       ,m_addresscode(addresscode)
   {
   }

   int BgTransmmiter::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
   {
       switch(m_subprotocol)
       {
       case BEIJING_BEIGUANG_300W:
       case BEIJING_BEIGUANG_100W:
       {
           if(data[0] == 0x7D)
               return 0;
           else
           {
               unsigned char cDes[1]={0};
               cDes[0]=0x7D;
               return kmp(data,nDataLen,cDes,1);
           }
       }
       case BEIGUANG_FM_3KW:
       case BEIGUANG_FM_5KW:
       case BEIGUANG_FM_10KW:
       {
           if(nDataLen<2)
               return -1;
           if(data[0] == m_addresscode && data[1] == 0x3e)
               return data[2]-1;
           else
           {
//               unsigned char cDes[2]={0};
//               cDes[0]=m_addresscode;
//               cDes[1]=0x3e;
//               return kmp(data,nDataLen,cDes,2);
               return RE_HEADERROR;
           }
       }
       case BEIGUANG_TV_1KW:
       {
           if(((data[0]&0xF0)>>4)==m_addresscode)
               return 0;
           else
           {
               for(int i=1;i<nDataLen;++i)
               {
                   if(((data[i]&0xF0)>>4)==m_addresscode)
                       return i;
               }
           }
       }
       case BEIGUANG_FM_1KW:
       {
           if(nDataLen<2)
               return -1;
           if(data[0] == 0xFE && data[1] == 0xFE)
           {
               if(data[3]!=0xFD)
                   return 0;
               else
                   return 4;
           }
           else
           {
               unsigned char cDes[2]={0};
               cDes[0]=0xFE;
               cDes[1]=0xFE;
               return kmp(data,nDataLen,cDes,2);
           }
       }
       case BEIGUANG_AM_10KW:
       {
           if(nDataLen<3)
               return -1;
           if(data[0] == 0x57 && data[1] == 0x32 && data[2] == 0x31)
               return 0;
           else
           {
               unsigned char cDes[3]={0};
               cDes[0]=0x57;
               cDes[1]=0x32;
               cDes[2]=0x31;
               return kmp(data,nDataLen,cDes,3);
           }
       }
       case BEIGUANG_AM_1KW:
       {
           if(data[0] == 0x06 && data[1] == 0x02)
               return 0;
           else
           {
               unsigned char cDes[2]={0};
               cDes[0] = 0x06;
               cDes[1] = 0x02;
               return kmp(data,number,cDes,2);
           }
       }
       default:
           return RE_NOPROTOCOL;
       }
       return RE_NOPROTOCOL;
   }

   int BgTransmmiter::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
   {
       switch(m_subprotocol)
       {
       case BEIJING_BEIGUANG_300W:
       {
           return BG300Data(data,data_ptr,nDataLen,runstate);
       }
       case BEIJING_BEIGUANG_100W:
       {
       }
       case BEIGUANG_FM_3KW:
       {
           return BeiGuangFm3KWData(data,data_ptr,nDataLen,runstate);
       }
       case BEIGUANG_FM_5KW:
       {
           return BeiGuangFm5KWData(data,data_ptr,nDataLen,runstate);
       }
       case BEIGUANG_FM_10KW:
       {
           return BeiGuangFm10KWData(data,data_ptr,nDataLen,runstate);
       }
       case BEIGUANG_TV_1KW:
       {

       }
       case BEIGUANG_FM_1KW:
       {
           return OnBeiguangFM1KW(data,data_ptr,nDataLen,runstate);
       }
       case BEIGUANG_AM_10KW:
       {

       }
       case BEIGUANG_AM_1KW:
       {
           return BGAm1KwData(data,data_ptr,nDataLen,runstate);
       }
       default:
           return RE_NOPROTOCOL;
       }
       return RE_NOPROTOCOL;
   }

   bool BgTransmmiter::IsStandardCommand()
   {
       switch(m_subprotocol)
       {
       case BEIGUANG_FM_3KW:
       case BEIGUANG_FM_5KW:
       case BEIGUANG_FM_10KW:
           return true;
       default:
           break;
       }
       return false;
   }

   void BgTransmmiter::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
   {

   }

   void BgTransmmiter::GetAllCmd(CommandAttribute &cmdAll)
   {
       CommandUnit tmUnit;
       switch(m_subprotocol)
       {
       case BEIJING_BEIGUANG_300W:
       case BEIJING_BEIGUANG_100W:
       {
           tmUnit.commandLen = 8;
           tmUnit.ackLen = 18;
           tmUnit.commandId[0] = 0x52;
           tmUnit.commandId[1] = m_addresscode;
           tmUnit.commandId[2] = 0x06;
           tmUnit.commandId[3] = 0xFF;
           tmUnit.commandId[4] = 0xFF;
           tmUnit.commandId[5] = 0xFF;
           tmUnit.commandId[6] = 0xFF;
           tmUnit.commandId[7] = tmUnit.commandId[0]^tmUnit.commandId[1]^tmUnit.commandId[2];
           cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
           tmUnit.ackLen = 0;
           tmUnit.commandId[2] = 0x01;
           tmUnit.commandId[7] = tmUnit.commandId[0]^tmUnit.commandId[1]^tmUnit.commandId[2];
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
           tmUnit.commandId[2] = 0x02;
           tmUnit.commandId[7] = tmUnit.commandId[0]^tmUnit.commandId[1]^tmUnit.commandId[2];
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
       }
           break;
       case BEIGUANG_FM_3KW:
       case BEIGUANG_FM_5KW:
       case BEIGUANG_FM_10KW:
       {
           tmUnit.commandLen = 7;
           tmUnit.ackLen = 3;
           tmUnit.commandId[0] = m_addresscode;
           tmUnit.commandId[1] = 0x3C;
           tmUnit.commandId[2] = 0x05;
           tmUnit.commandId[3] = 0x01;
           tmUnit.commandId[4] = 0x00;
           tmUnit.commandId[5] = 0x05;
           tmUnit.commandId[6] = 0x3D;
           cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
           tmUnit.ackLen = 0;
           tmUnit.commandLen = 9;
           tmUnit.commandId[2] = 0x07;
           tmUnit.commandId[3] = 0x0A;
           tmUnit.commandId[5] = 0x06;
           tmUnit.commandId[6] = 0x36;
           tmUnit.commandId[7] = 0x00;
           tmUnit.commandId[8] = 0x01;
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
           tmUnit.commandId[5] = 0x07;
           tmUnit.commandId[8] = 0x00;
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
       }
           break;
       case BEIGUANG_TV_1KW:
       {
           tmUnit.ackLen = 18;
           tmUnit.commandId[0] = ((m_addresscode<<4)|3);
           tmUnit.commandId[1] = 0x00;
           tmUnit.commandId[2] = 0x00;
           tmUnit.commandLen = 3;
           cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
           tmUnit.ackLen = 0;
           tmUnit.commandId[0] = ((m_addresscode<<4)|0);
           tmUnit.commandLen = 1;
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
           tmUnit.commandId[0] = ((m_addresscode<<4)|1);
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
       }
           break;
       case BEIGUANG_FM_1KW:
       {
           tmUnit.ackLen = 14;
           tmUnit.commandId[0] = 0xFE;
           tmUnit.commandId[1] = 0xFE;
           tmUnit.commandId[2] = m_addresscode;
           tmUnit.commandId[3] = 0x01;
           tmUnit.commandLen = 4;
           cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
           tmUnit.ackLen = 0;
           tmUnit.commandId[3] = 0x03;
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
           tmUnit.commandId[3] = 0x02;
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
       }
           break;
       case BEIGUANG_AM_10KW:
           break;
       case BEIGUANG_AM_1KW:
       {
           tmUnit.ackLen = 16;
           tmUnit.commandId[0] = m_addresscode;
           tmUnit.commandId[1] = 0x02;
           tmUnit.commandLen = 2;
           cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
           tmUnit.ackLen = 5;
           tmUnit.commandId[1] = 0x01;
           cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
           tmUnit.ackLen = 0;
           tmUnit.commandId[1] = 0x0A;
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
           tmUnit.commandId[1] = 0x08;
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
           tmUnit.commandId[1] = 0x09;
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
       }

       }
   }

   int BgTransmmiter::BG300Data(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
   {
       int tmp;
       tmp = data[3]*256+data[2];
       DataInfo dtinfo;
       dtinfo.bType = false;
       dtinfo.fValue = tmp*0.0001;
       data_ptr->mValues[0] = dtinfo;
       dtinfo.fValue = (data[5]*256+data[4])*0.1;
       data_ptr->mValues[1] = dtinfo;
       if(tmp*0.1<=data_ptr->mValues[1].fValue)
           dtinfo.fValue = 0;
       else
       {
           dtinfo.fValue = sqrt((tmp*0.1+data_ptr->mValues[1].fValue)/(tmp*0.1-data_ptr->mValues[1].fValue));
       }
       data_ptr->mValues[2] = dtinfo;
       dtinfo.fValue = (data[7]*256+data[6])*0.1;
       data_ptr->mValues[3] = dtinfo;
       dtinfo.fValue = (data[9]*256+data[8])*0.1;
       data_ptr->mValues[4] = dtinfo;
       dtinfo.bType = true;
       for(int i=7;i>=0;--i)
       {
           dtinfo.fValue = Getbit(data[12],i);
           data_ptr->mValues[12-i] = dtinfo;
       }
       for(int i =7; i>= 6 ; --i)
       {
           dtinfo.fValue = Getbit(data[12],i);
           data_ptr->mValues[20-i] = dtinfo;
       }
       return RE_SUCCESS;
   }

   int BgTransmmiter::BGAm1KwData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
   {
       DataInfo dtinfo;
       dtinfo.bType = false;
       dtinfo.fValue = data[7];
       data_ptr->mValues[0] = dtinfo;
       dtinfo.fValue = data[6];
       data_ptr->mValues[1] = dtinfo;
       dtinfo.fValue = 1;
       data_ptr->mValues[2] = dtinfo;
       dtinfo.fValue = data[2];
       data_ptr->mValues[3] = dtinfo;
       dtinfo.fValue = data[4];
       data_ptr->mValues[4] = dtinfo;
       dtinfo.fValue = data[5];
       data_ptr->mValues[5] = dtinfo;
       dtinfo.fValue = data[8];
       data_ptr->mValues[6] = dtinfo;
       dtinfo.fValue = data[9];
       data_ptr->mValues[7] = dtinfo;
       dtinfo.fValue = data[10];
       data_ptr->mValues[8] = dtinfo;
       dtinfo.fValue = data[11];
       data_ptr->mValues[9] = dtinfo;
       dtinfo.fValue = data[12];
       data_ptr->mValues[10] = dtinfo;
       dtinfo.fValue = data[13];
       data_ptr->mValues[11] = dtinfo;
       dtinfo.fValue = data[14];
       data_ptr->mValues[12] = dtinfo;
       dtinfo.fValue = data[15];
       data_ptr->mValues[13] = dtinfo;
       dtinfo.bType = true;
       dtinfo.fValue = Getbit(data[18],0)==0? 1:0;
       data_ptr->mValues[14] = dtinfo;
       dtinfo.fValue = Getbit(data[18],2)==0? 1:0;
       data_ptr->mValues[15] = dtinfo;
       dtinfo.fValue = Getbit(data[18],3)==0? 1:0;
       data_ptr->mValues[16] = dtinfo;
       dtinfo.fValue = Getbit(data[18],4);
       data_ptr->mValues[17] = dtinfo;
       dtinfo.fValue = Getbit(data[18],5)==0? 1:0;
       data_ptr->mValues[18] = dtinfo;
       dtinfo.fValue = Getbit(data[18],6);
       data_ptr->mValues[19] = dtinfo;
       dtinfo.fValue = Getbit(data[18],7);
       data_ptr->mValues[20] = dtinfo;
       for(int i=0;i<6;++i)
       {
           dtinfo.fValue = Getbit(data[19],i);
           data_ptr->mValues[21+i] = dtinfo;
       }
       return RE_SUCCESS;
   }

   int BgTransmmiter::OnBeiguangFM1KW(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
   {
       if(data[3]==0xFD)
           return RE_CMDACK;
       int indexpos =0;
       DataInfo dtinfo;
       dtinfo.bType = false;
       if(data[5] >= 204 && data[5] <255)
           dtinfo.fValue = data[5]*data[4]*0.0225*0.001;
       else if(data[5] >= 178 && data[5] <204)
           dtinfo.fValue = data[5]*data[5]*0.0235*0.001;
       else if(data[5] >= 153 && data[5] <178)
           dtinfo.fValue = data[5]*data[5]*0.0245*0.001;
       else if(data[5] >= 131 && data[5] <153)
           dtinfo.fValue = data[5]*data[5]*0.025*0.001;
       else if(data[5] >= 114 && data[5] <131)
           dtinfo.fValue = data[5]*data[5]*0.026*0.001;
       else if (data[5] >= 49 && data[5] < 114)
           dtinfo.fValue = data[5]*data[5]*0.030*0.001;
       else if (data[5] >= 0 && data[5] <49)
           dtinfo.fValue = data[5]*data[5]*0.049*0.001;
       data_ptr->mValues[indexpos++] = dtinfo;
       if(data[6] >= 42 && data[6] <255)
           dtinfo.fValue = data[6]*0.526;
       else if(data[6] >= 34 && data[6] <42)
           dtinfo.fValue = data[6]*0.501;
       else if(data[6] >= 20 && data[6] <34)
           dtinfo.fValue = data[6]*0.455;
       else if(data[6] >= 8 && data[6] <20)
           dtinfo.fValue = data[6]*0.440;
       else if(data[6] >= 0 && data[6] <8)
           dtinfo.fValue = data[6]*0.510;
       data_ptr->mValues[indexpos++] = dtinfo;
       if(data_ptr->mValues[0].fValue*1000>0 && data_ptr->mValues[0].fValue*1000>data_ptr->mValues[1].fValue)
           dtinfo.fValue = sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
       else
           dtinfo.fValue = 0;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = data[8]/2.45;
       data_ptr->mValues[indexpos++] = dtinfo;
       for(int i=0;i<4;++i)
       {
           dtinfo.fValue = data[9+i]*0.052;
           data_ptr->mValues[indexpos++] = dtinfo;
       }
       dtinfo.fValue = data[13]/2.55;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.bType = true;
       for(int i =0 ;i <4;i++)
       {
           dtinfo.fValue = Getbit(data[4],i)==1 ? 0:1;
           data_ptr->mValues[indexpos++] = dtinfo;
       }
       for(int i =5 ;i <8;i++)
       {
           dtinfo.fValue = Getbit(data[4],i)==1 ? 0:1;
           data_ptr->mValues[indexpos++] = dtinfo;
       }
       dtinfo.fValue = Getbit(data[4],4)==1 ? 0:1;
       data_ptr->mValues[indexpos++] = dtinfo;
       return RE_SUCCESS;
   }

   int BgTransmmiter::BeiGuangFm3KWData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
   {
       if(nDataLen<110)
           return RE_HEADERROR;
       DataInfo dtinfo;
       dtinfo.bType = false;
       int indexpos = 0;
       int outptpower = data[8]*256+data[7];
       dtinfo.fValue = outptpower*0.001;
       data_ptr->mValues[indexpos++] = dtinfo;
       int refpower = data[10]*256+data[9];
       dtinfo.fValue = refpower;
       data_ptr->mValues[indexpos++] = dtinfo;
       if(outptpower>refpower)
       {
           dtinfo.fValue = sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
       }
       else
           dtinfo.fValue = 0;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = data[12]*256+data[11];
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = (data[14]*256+data[13])*0.01;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = (data[16]*256+data[15])*0.1;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = (data[18]*256+data[17])*0.1;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.bType = true;
       for(int i=0;i<4;++i)
       {
           if(i!=2)
           {
               dtinfo.fValue = Getbit(data[19],i);
               data_ptr->mValues[indexpos++] = dtinfo;
           }
       }
       dtinfo.bType = false;
       dtinfo.fValue = (data[64]*256+data[63])*0.01;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = (data[66]*256+data[65])*0.01;
       data_ptr->mValues[indexpos++] = dtinfo;
       for(int i=0;i<3;++i)
       {
           dtinfo.fValue = (data[72+17*i]*256+data[71+17*i]);
           data_ptr->mValues[indexpos++] = dtinfo;
           dtinfo.fValue = (data[74+17*i]*256+data[73+17*i])*0.1;
           data_ptr->mValues[indexpos++] = dtinfo;
       }
       return RE_SUCCESS;
   }

   int BgTransmmiter::BeiGuangFm5KWData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
   {
       if(nDataLen<143)
           return RE_HEADERROR;
       DataInfo dtinfo;
       dtinfo.bType = false;
       int indexpos = 0;
       int outptpower = data[8]*256+data[7];
       dtinfo.fValue = outptpower*0.001;
       data_ptr->mValues[indexpos++] = dtinfo;
       int refpower = data[10]*256+data[9];
       dtinfo.fValue = refpower;
       data_ptr->mValues[indexpos++] = dtinfo;
       if(outptpower>refpower)
       {
           dtinfo.fValue = sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
       }
       else
           dtinfo.fValue = 0;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = data[12]*256+data[11];
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = (data[14]*256+data[13])*0.01;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = (data[16]*256+data[15])*0.1;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = (data[18]*256+data[17])*0.1;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.bType = true;
       for(int i=0;i<4;++i)
       {
           if(i!=2)
           {
               dtinfo.fValue = Getbit(data[19],i);
               data_ptr->mValues[indexpos++] = dtinfo;
           }
       }
       dtinfo.bType = false;
       dtinfo.fValue = (data[64]*256+data[63])*0.01;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = (data[66]*256+data[65])*0.01;
       data_ptr->mValues[indexpos++] = dtinfo;
       for(int i=0;i<5;++i)
       {
           dtinfo.fValue = (data[72+17*i]*256+data[71+17*i]);
           data_ptr->mValues[indexpos++] = dtinfo;
           dtinfo.fValue = (data[74+17*i]*256+data[73+17*i])*0.1;
           data_ptr->mValues[indexpos++] = dtinfo;
       }
       return RE_SUCCESS;
   }

   int BgTransmmiter::BeiGuangFm10KWData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
   {
       if(nDataLen<228)
           return RE_HEADERROR;
       DataInfo dtinfo;
       dtinfo.bType = false;
       int indexpos = 0;
       int outptpower = data[8]*256+data[7];
       dtinfo.fValue = outptpower*0.001;
       data_ptr->mValues[indexpos++] = dtinfo;
       int refpower = data[10]*256+data[9];
       dtinfo.fValue = refpower;
       data_ptr->mValues[indexpos++] = dtinfo;
       if(outptpower>refpower)
       {
           dtinfo.fValue = sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
       }
       else
           dtinfo.fValue = 0;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = data[12]*256+data[11];
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = (data[14]*256+data[13])*0.01;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = (data[16]*256+data[15])*0.1;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = (data[18]*256+data[17])*0.1;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.bType = true;
       for(int i=0;i<4;++i)
       {
           if(i!=2)
           {
               dtinfo.fValue = Getbit(data[19],i);
               data_ptr->mValues[indexpos++] = dtinfo;
           }
       }
       dtinfo.bType = false;
       dtinfo.fValue = (data[64]*256+data[63])*0.01;
       data_ptr->mValues[indexpos++] = dtinfo;
       dtinfo.fValue = (data[66]*256+data[65])*0.01;
       data_ptr->mValues[indexpos++] = dtinfo;
       for(int i=0;i<10;++i)
       {
           dtinfo.fValue = (data[72+17*i]*256+data[71+17*i]);
           data_ptr->mValues[indexpos++] = dtinfo;
           dtinfo.fValue = (data[74+17*i]*256+data[73+17*i])*0.1;
           data_ptr->mValues[indexpos++] = dtinfo;
       }
       return RE_SUCCESS;
   }
}
