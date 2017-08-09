#include "zctransmmit.h"
namespace hx_net{
   ZcTransmmit::ZcTransmmit(int subprotocol,int addresscode)
   :Transmmiter()
   ,m_subprotocol(subprotocol)
   ,m_addresscode(addresscode)
   {
   }

   int ZcTransmmit::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
   {
       switch(m_subprotocol)
       {
       case ZHC_618F:
       case ZHC_10KWTV:
       {
           if(data[0]==0x10 && data[1]==0x02)
               return ((data[13]<<8)|data[12]);
           else
               return RE_HEADERROR;
       }
           break;
       }
       return RE_NOPROTOCOL;
   }

   int ZcTransmmit::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
   {
       switch(m_subprotocol)
       {
       case ZHC_618F:
           return Zhc618F_Data(data,data_ptr,nDataLen,runstate);
       case ZHC_10KWTV:
           return Zhc10KWTv_Data(data,data_ptr,nDataLen,runstate);
       }
       return RE_NOPROTOCOL;
   }

   bool ZcTransmmit::IsStandardCommand()
   {
       switch(m_subprotocol)
       {
       case ZHC_618F:
       case ZHC_10KWTV:
           return true;
       }
       return false;
   }

   void ZcTransmmit::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
   {
       switch(m_subprotocol)
       {
       case ZHC_618F:
       {

       }
           break;
       }
   }

   void ZcTransmmit::GetAllCmd(CommandAttribute &cmdAll)
   {
       switch(m_subprotocol)
       {
       case ZHC_618F:
       case ZHC_10KWTV:
       {
           CommandUnit tmUnit;
           tmUnit.commandId[0] = 0x10;
           tmUnit.commandId[1] = 0x02;
           tmUnit.commandId[2] = 0x00;
           tmUnit.commandId[3] = 0x00;
           tmUnit.commandId[4] = 0x00;
           tmUnit.commandId[5] = 0x00;
           tmUnit.commandId[6] = 0x00;
           tmUnit.commandId[7] = 0x00;
           tmUnit.commandId[8] = 0x00;
           tmUnit.commandId[9] = 0x00;
           tmUnit.commandId[10] = 0x04;
           tmUnit.commandId[11] = 0x04;
           tmUnit.commandId[12] = 0x04;
           tmUnit.commandId[13] = 0x00;
           unsigned short crcret = CalcCRC16_USB(&tmUnit.commandId[2],12);
           tmUnit.commandId[14] = crcret&0xFF;
           tmUnit.commandId[15] = ((crcret & 0xFF00)>>8);
           tmUnit.commandId[16] = 0x10;
           tmUnit.commandId[17] = 0x03;
           tmUnit.commandLen = 18;
           tmUnit.ackLen = 14;
           vector<CommandUnit> vtUnit;
           vtUnit.push_back(tmUnit);
           cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
           tmUnit.commandId[11] = 0x05;
           tmUnit.commandId[12] = 0x05;
           tmUnit.commandId[14] = 0x01;
           crcret = CalcCRC16_USB(&tmUnit.commandId[2],13);
           tmUnit.commandId[15] = crcret&0xFF;
           tmUnit.commandId[16] = ((crcret & 0xFF00)>>8);
           tmUnit.commandId[17] = 0x10;
           tmUnit.commandId[18] = 0x03;
           tmUnit.commandLen = 19;
           vector<CommandUnit> vtTurnOnUnit;
           vtTurnOnUnit.push_back(tmUnit);
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtTurnOnUnit;
           tmUnit.commandId[14] = 0x00;
           crcret = CalcCRC16_USB(&tmUnit.commandId[2],13);
           tmUnit.commandId[15] = crcret&0xFF;
           tmUnit.commandId[16] = ((crcret & 0xFF00)>>8);
           vector<CommandUnit> vtTurnOffUnit;
           vtTurnOffUnit.push_back(tmUnit);
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnOffUnit;
       }
           break;
       }
   }

   int ZcTransmmit::Zhc618F_Data(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
   {
       unsigned char ucFunc = data[10];
       unsigned char ucSubFunc = data[11];
       unsigned char cDes[2]={0};
       cDes[0]=0x10;
       cDes[1]=0x03;

       int nDatapos = kmp(data,nDataLen,cDes,2);
       if(nDatapos<0)
       {
           int dataindex = 0;
           for(int i=14;i<nDataLen-1;++i)
           {
               if(data[i]==0x10 && data[i+1]==0x10)
               {
                  dataindex++;
                  i++;
               }
           }
           return dataindex;
       }
       if(ucFunc!=0x04 || ucSubFunc!=0x04)
           return RE_CMDACK;
       //开始整理数据
       unsigned char cdata[100];
       int dataindex = 0;
       for(int i=14;i<nDataLen-1;++i)
       {
           if(data[i]==0x10 && data[i+1]==0x10)
           {
               i+=1;
           }
           cdata[dataindex] = data[i];
           dataindex++;
       }
       //偏移14字节到数据区
       data = data+14;
       DataInfo dainfo;
       dainfo.bType = true;
       dainfo.fValue = cdata[0];
       runstate = cdata[0]==0? 1:0;
       data_ptr->mValues[3] = dainfo;
       dainfo.bType = false;
       dainfo.fValue = ((cdata[2]<<8)|cdata[1])*0.1;
       data_ptr->mValues[4] = dainfo;
       dainfo.fValue = ((cdata[4]<<8)|cdata[3])*0.1;
       data_ptr->mValues[5] = dainfo;
       dainfo.fValue = ((cdata[6]<<8)|cdata[5])*0.1;
       data_ptr->mValues[6] = dainfo;
       dainfo.fValue = ((cdata[8]<<8)|cdata[7])*0.1;
       data_ptr->mValues[7] = dainfo;
       if((data[9]&0x80)==0x00)
       {
           dainfo.fValue = cdata[9];
       }
       else
       {
           dainfo.fValue = (cdata[9]&0x7F)*(-1);
       }
       data_ptr->mValues[8] = dainfo;
       dainfo.fValue = ((cdata[11]<<8)|cdata[10])*0.1;
       data_ptr->mValues[9] = dainfo;
       dainfo.fValue = ((cdata[13]<<8)|cdata[12])*0.1;
       data_ptr->mValues[10] = dainfo;
       dainfo.bType = true;
       for(int i=0;i<5;++i)
       {
           dainfo.fValue = Getbit(cdata[14],i)==0 ? 1:0;
           data_ptr->mValues[11+i] = dainfo;
       }
       dainfo.bType = false;
       dainfo.fValue = ((cdata[16]<<8)|cdata[15])*0.001;
       data_ptr->mValues[0] = dainfo;
       dainfo.fValue = ((cdata[18]<<8)|cdata[17])*0.1;
       data_ptr->mValues[16] = dainfo;
       dainfo.fValue = ((cdata[20]<<8)|cdata[19])*0.01;
       data_ptr->mValues[17] = dainfo;
       data_ptr->mValues[1] = dainfo;
       if((data[9]&0x80)==0x00)
       {
           dainfo.fValue = cdata[21];
       }
       else
       {
           dainfo.fValue = (cdata[21]&0x7F)*(-1);
       }
       data_ptr->mValues[18] = dainfo;
       dainfo.fValue = ((cdata[23]<<8)|cdata[22])*0.1;
       data_ptr->mValues[19] = dainfo;
       dainfo.fValue = cdata[24];
       data_ptr->mValues[20] = dainfo;
       for(int i=0;i<4;++i)
       {
           dainfo.fValue = cdata[25+i]*0.1;
           data_ptr->mValues[21+i] = dainfo;
       }
       dainfo.bType = true;
       for(int i=0;i<8;++i)
       {
           dainfo.fValue = Getbit(cdata[29],i)==0 ? 1:0;
           data_ptr->mValues[25+i] = dainfo;
       }
       for(int i=0;i<3;++i)
       {
           dainfo.fValue = Getbit(cdata[30],i)==0 ? 1:0;
           data_ptr->mValues[33+i] = dainfo;
       }
       dainfo.bType = false;
       if(data_ptr->mValues[0].fValue*1000>data_ptr->mValues[1].fValue)
       {
           dainfo.fValue = sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
       }
       else
           dainfo.fValue = 1.0;
       return 0;
   }

   int ZcTransmmit::Zhc10KWTv_Data(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
   {
       unsigned char ucFunc = data[10];
       unsigned char ucSubFunc = data[11];
       unsigned char cDes[2]={0};
       cDes[0]=0x10;
       cDes[1]=0x03;

       int nDatapos = kmp(data,nDataLen,cDes,2);
       if(nDatapos<0)
       {
           int dataindex = 0;
           for(int i=14;i<nDataLen-1;++i)
           {
               if(data[i]==0x10 && data[i+1]==0x10)
               {
                  dataindex++;
                  i++;
               }
           }
           return dataindex;
       }
       if(ucFunc!=0x04 || ucSubFunc!=0x04)
           return RE_CMDACK;
       //开始整理数据
       unsigned char cdata[100];
       int dataindex = 0;
       for(int i=14;i<nDataLen-1;++i)
       {
           if(data[i]==0x10 && data[i+1]==0x10)
           {
               i+=1;
           }
           cdata[dataindex] = data[i];
           dataindex++;
       }
       //偏移14字节到数据区
       data = data+14;
       DataInfo dainfo;
       dainfo.bType = true;
       dainfo.fValue = cdata[0];
       runstate = cdata[0]==0? 1:0;
       data_ptr->mValues[3] = dainfo;
       dainfo.bType = false;
       dainfo.fValue = ((cdata[2]<<8)|cdata[1])*0.1;
       data_ptr->mValues[4] = dainfo;
       dainfo.fValue = dainfo.fValue*0.1;
       data_ptr->mValues[0] = dainfo;
       dainfo.fValue = ((cdata[4]<<8)|cdata[3])*0.1;
       data_ptr->mValues[5] = dainfo;
       dainfo.fValue = ((cdata[6]<<8)|cdata[5])*0.1;
       data_ptr->mValues[6] = dainfo;
       dainfo.fValue = ((cdata[8]<<8)|cdata[7])*0.1;
       data_ptr->mValues[7] = dainfo;
       if((data[9]&0x80)==0x00)
       {
           dainfo.fValue = cdata[9];
       }
       else
       {
           dainfo.fValue = (cdata[9]&0x7F)*(-1);
       }
       data_ptr->mValues[8] = dainfo;
       dainfo.fValue = ((cdata[11]<<8)|cdata[10])*0.1;
       data_ptr->mValues[9] = dainfo;
       dainfo.fValue = ((cdata[13]<<8)|cdata[12])*0.1;
       data_ptr->mValues[10] = dainfo;
       dainfo.bType = true;
       for(int i=0;i<5;++i)
       {
           dainfo.fValue = Getbit(cdata[14],i)==0 ? 1:0;
           data_ptr->mValues[11+i] = dainfo;
       }
       dainfo.bType = false;
       //dainfo.fValue = ((cdata[16]<<8)|cdata[15])*0.001;
       //data_ptr->mValues[0] = dainfo;
       dainfo.fValue = ((cdata[18]<<8)|cdata[17])*0.1;
       data_ptr->mValues[16] = dainfo;
       dainfo.fValue = ((cdata[20]<<8)|cdata[19])*0.01;
       data_ptr->mValues[17] = dainfo;
       data_ptr->mValues[1] = dainfo;
       if((data[9]&0x80)==0x00)
       {
           dainfo.fValue = cdata[21];
       }
       else
       {
           dainfo.fValue = (cdata[21]&0x7F)*(-1);
       }
       data_ptr->mValues[18] = dainfo;
       dainfo.fValue = ((cdata[23]<<8)|cdata[22])*0.1;
       data_ptr->mValues[19] = dainfo;
       dainfo.fValue = cdata[24];
       data_ptr->mValues[20] = dainfo;
       for(int i=0;i<4;++i)
       {
           dainfo.fValue = cdata[25+i]*0.1;
           data_ptr->mValues[21+i] = dainfo;
       }
       dainfo.bType = true;
       for(int i=0;i<8;++i)
       {
           dainfo.fValue = Getbit(cdata[29],i)==0 ? 1:0;
           data_ptr->mValues[25+i] = dainfo;
       }
       for(int i=0;i<3;++i)
       {
           dainfo.fValue = Getbit(cdata[30],i)==0 ? 1:0;
           data_ptr->mValues[33+i] = dainfo;
       }
       dainfo.bType = false;
       if(data_ptr->mValues[0].fValue*1000>data_ptr->mValues[1].fValue)
       {
           dainfo.fValue = sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
       }
       else
           dainfo.fValue = 1.0;
       return 0;
   }
}
