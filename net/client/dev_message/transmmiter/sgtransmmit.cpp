#include "sgtransmmit.h"
namespace hx_net{
      SgTransmmit::SgTransmmit(int subprotocol,int addresscode)
          :Transmmiter()
          ,m_subprotocol(subprotocol)
          ,m_addresscode(addresscode)
      {
      }

      int SgTransmmit::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
      {
          switch(m_subprotocol)
          {
          case SHANXI_762_3_5KW:{
              if(nDataLen<2)
                  return -1;
              if(number == 0)
              {
                  if(data[0] == m_addresscode && data[1] == 0x02)
                      return 0;
                  else
                  {
                      unsigned char cDes[2]={0};
                      cDes[0] = m_addresscode;
                      cDes[1] = 0x02;
                      return kmp(data,nDataLen,cDes,2);
                  }
              }
              else
              {
                  if(data[0] == m_addresscode && data[1] == 0x01)
                      return 0;
                  else
                  {
                      unsigned char cDes[2]={0};
                      cDes[0] = m_addresscode;
                      cDes[1] = 0x01;
                      return kmp(data,nDataLen,cDes,2);
                  }
              }
          }
              break;
          case SHANXI_762_10KW:
          case SHANXI_CD_3_5KW:{
              if(nDataLen<2)
                  return -1;
              if(data[0] == 0xBB && data[1] == 0xF0)
                  return 0;
              else
              {
                  unsigned char cDes[2]={0};
                  cDes[0] = 0xBB;
                  cDes[1] = 0xF0;
                  return kmp(data,nDataLen,cDes,2);
              }
          }
              break;
          case SHANGUANG_FM_300W:{
              if(nDataLen<2)
                  return -1;
              if(data[0] == m_addresscode && data[1] == 0x41)
                  return 0;
              else
              {
                  unsigned char cDes[2]={0};
                  cDes[0] = m_addresscode;
                  cDes[1] = 0x41;
                  return kmp(data,nDataLen,cDes,2);
              }
          }
              break;
          case SHANGUANG_AM_50KW:{
              if(nDataLen<2)
                  return -1;
              if(number == 0)
              {
                  if(data[0] == m_addresscode && data[1] == 0x02)
                      return 0;
                  else
                  {
                      unsigned char cDes[2]={0};
                      cDes[0] = m_addresscode;
                      cDes[1] = 0x02;
                      return kmp(data,nDataLen,cDes,2);
                  }
              }
              else
              {
                  if(data[30] == m_addresscode && data[31] == 0x01)
                      return 0;
                  else
                  {
                      unsigned char cDes[2]={0};
                      cDes[0] = m_addresscode;
                      cDes[1] = 0x01;
                      return kmp(data,nDataLen,cDes,2);
                  }
              }
          }
              break;
          case SHANXI_AM_10KW:{
              if(data[0]==0xAA && data[1]==0xBB && data[2]==0xCC && data[3]==0xDD)
              {
                  if(data[15]!=0xD9)
                      return 0;
                  else
                      return 11;
              }
              else
              {
                  unsigned char cDes[4]={0};
                  cDes[0]=0xAA;
                  cDes[1]=0xBB;
                  cDes[2]=0xCC;
                  cDes[3]=0xDD;
                  int nPos = kmp(data,nDataLen,cDes,4);
                  return nPos;
              }
          }
              break;
          case SHANXI_3KW_PDM:{
              if(data[0]==0xCA && data[1]==m_addresscode)
              {
                  return data[3]+2;
              }
              else{
                  return RE_HEADERROR;
              }
          }
              break;
          }
          return RE_NOPROTOCOL;
      }

      int SgTransmmit::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
      {
          switch(m_subprotocol)
          {
          case SHANXI_762_3_5KW:
              return SX762_5KWData(data,data_ptr,nDataLen,runstate);
          case SHANXI_762_10KW:
              break;
          case SHANGUANG_FM_300W:
              break;
          case SHANGUANG_AM_50KW:
              break;
          case SHANXI_CD_3_5KW:
              break;
          case SHANXI_AM_10KW:
              break;
          case SHANXI_3KW_PDM:
              return RY_3KwPdmData(data,data_ptr,nDataLen,runstate);
          }
          return RE_NOPROTOCOL;
      }

      bool SgTransmmit::IsStandardCommand()
      {
          switch(m_subprotocol)
          {
          case SHANXI_3KW_PDM:
              return true;
          }
          return false;
      }

      void SgTransmmit::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
      {

      }

      void SgTransmmit::GetAllCmd(CommandAttribute &cmdAll)
      {
          switch(m_subprotocol)
          {
          case SHANXI_762_3_5KW:{
              CommandUnit tmUnit;
              tmUnit.commandId[0] = 0x00;
              tmUnit.commandId[1] = 0x66;
              tmUnit.commandLen = 2;
              tmUnit.ackLen = 0;
              cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
              tmUnit.commandId[1] = 0x65;
              cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
              tmUnit.commandId[1] = 0x6E;
              cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
              tmUnit.commandId[1] = 0x6D;
              cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
          }
              break;
          case SHANXI_CD_3_5KW:
          case SHANXI_762_10KW:{
              CommandUnit tmUnit;
              tmUnit.commandId[0] = 0xBB;
              tmUnit.commandId[1] = 0xF0;
              tmUnit.commandId[2] = 0x00;
              tmUnit.commandId[3] = 0x00;
              tmUnit.commandId[4] = 0x00;
              tmUnit.commandLen = 5;
              tmUnit.ackLen = 292;
              cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
              tmUnit.ackLen = 0;
              tmUnit.commandId[1] = 0xFA;
              cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
              tmUnit.commandId[1] = 0xFB;
              cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
          }
              break;
          case SHANGUANG_FM_300W:{
              CommandUnit tmUnit;
              tmUnit.commandId[0] = 0x01;
              tmUnit.commandId[1] = 0x41;
              tmUnit.commandId[2] = 0x00;
              tmUnit.commandId[3] = 0x00;
              tmUnit.commandId[4] = 0x00;
              tmUnit.commandId[5] = 0x40;
              tmUnit.commandLen = 6;
              tmUnit.ackLen = 0;
              cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
          }
              break;
          case SHANGUANG_AM_50KW:{
              CommandUnit tmUnit;
              tmUnit.commandId[0] = m_addresscode;
              tmUnit.commandId[1] = 0x66;
              tmUnit.commandLen = 2;
              tmUnit.ackLen = 30;
              cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
              tmUnit.commandId[1] = 0x65;
              tmUnit.ackLen = 8;
              cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
              tmUnit.ackLen = 0;
              tmUnit.commandId[1] = 0x6E;
              cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
              tmUnit.commandId[1] = 0x6F;
              cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
              tmUnit.commandId[1] = 0x70;
              cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
              tmUnit.commandId[1] = 0x6D;
              cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
              tmUnit.commandId[1] = 0x71;
              cmdAll.mapCommand[MSG_TRANSMITTER_RISE_POWER_OPR].push_back(tmUnit);
              tmUnit.commandId[1] = 0x72;
              cmdAll.mapCommand[MSG_TRANSMITTER_REDUCE_POWER_OPR].push_back(tmUnit);
          }
              break;
          case SHANXI_AM_10KW:{
              CommandUnit tmUnit;
              tmUnit.commandLen = 11;
              tmUnit.ackLen = 50;
              tmUnit.commandId[0] = 0xAA;
              tmUnit.commandId[1] = 0xBB;
              tmUnit.commandId[2] = 0xCC;
              tmUnit.commandId[3] = 0xDD;
              tmUnit.commandId[4] = 0xD2;
              tmUnit.commandId[5] = 0xD2;
              tmUnit.commandId[6] = 0x03;
              tmUnit.commandId[7] = 0xDD;
              tmUnit.commandId[8] = 0xCC;
              tmUnit.commandId[9] = 0xBB;
              tmUnit.commandId[10] = 0xEE;
              cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
              tmUnit.ackLen = 27;
              tmUnit.commandId[4] = 0xD1;
              tmUnit.commandId[5] = 0xD1;
              cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
              tmUnit.ackLen = 11;
              tmUnit.commandId[4] = 0xD4;
              tmUnit.commandId[5] = 0xD4;
              cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
              tmUnit.commandId[4] = 0xD5;
              tmUnit.commandId[5] = 0xD5;
              cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
              tmUnit.commandId[4] = 0xD6;
              tmUnit.commandId[5] = 0xD6;
              cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
              tmUnit.commandId[4] = 0xD3;
              tmUnit.commandId[5] = 0xD3;
              cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
              tmUnit.commandId[4] = 0xD7;
              tmUnit.commandId[5] = 0xD7;
              cmdAll.mapCommand[MSG_DEV_RESET_OPR].push_back(tmUnit);
          }
              break;
          case SHANXI_3KW_PDM:{
              CommandUnit tmUnit;
              tmUnit.commandLen = 6;
              tmUnit.ackLen = 4;
              tmUnit.commandId[0] = 0xCA;
              tmUnit.commandId[1] = m_addresscode;
              tmUnit.commandId[2] = 0x10;
              tmUnit.commandId[3] = 0x00;
              tmUnit.commandId[4] = 0x00;
              tmUnit.commandId[5] = 0xDA;
              cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
              tmUnit.commandId[2] = 0x1D;
              tmUnit.commandId[4] = 0x0D;
              cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
              tmUnit.commandId[2] = 0x1E;
              tmUnit.commandId[4] = 0x0E;
              cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
              tmUnit.commandId[2] = 0x18;
              tmUnit.commandId[4] = 0x08;
              cmdAll.mapCommand[MSG_TRANSMITTER_RISE_POWER_OPR].push_back(tmUnit);
              tmUnit.commandId[2] = 0x17;
              tmUnit.commandId[4] = 0x07;
              cmdAll.mapCommand[MSG_TRANSMITTER_REDUCE_POWER_OPR].push_back(tmUnit);
          }
              break;
          }
      }

      int SgTransmmit::SX762_5KWData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
      {
          return RE_SUCCESS;
      }

      int SgTransmmit::RY_3KwPdmData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
      {
          if(data[2]!=0x10)
              return RE_CMDACK;
          DataInfo dainfo;
          int nbase=4;
          char data1=AsciiToInt(data[nbase+6]);
          char data2=AsciiToInt(data[nbase+7]);
          char data3=AsciiToInt(data[nbase+8]);
          char data4=AsciiToInt(data[nbase+9]);
          char data5=AsciiToInt(data[nbase+10]);
          char data6=AsciiToInt(data[nbase+11]);
          dainfo.bType = false;
          dainfo.fValue = (((data2&0x0F)<<4)|(data1&0x0F))*0.01+(((((((data6&0x0F)<<4)|(data5&0x0F))<<4)|(data4&0x0F))<<4)|(data3&0x0F));
          data_ptr->mValues[0] = dainfo;
          data1=AsciiToInt(data[nbase+12]);
          data2=AsciiToInt(data[nbase+13]);
          data3=AsciiToInt(data[nbase+14]);
          data4=AsciiToInt(data[nbase+15]);
          data5=AsciiToInt(data[nbase+16]);
          data6=AsciiToInt(data[nbase+17]);
          dainfo.fValue = (((data2&0x0F)<<4)|(data1&0x0F))*0.01+(((((((data6&0x0F)<<4)|(data5&0x0F))<<4)|(data4&0x0F))<<4)|(data3&0x0F));
          data_ptr->mValues[1] = dainfo;
          if(data_ptr->mValues[0].fValue>data_ptr->mValues[1].fValue)
          {
              dainfo.fValue = sqrt((data_ptr->mValues[0].fValue+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue-data_ptr->mValues[1].fValue));
          }
          else{
              dainfo.fValue = 0;
          }
          data_ptr->mValues[2] = dainfo;
          data1=AsciiToInt(data[nbase]);
          data2=AsciiToInt(data[nbase+1]);
          data3=AsciiToInt(data[nbase+2]);
          data4=AsciiToInt(data[nbase+3]);
          data5=AsciiToInt(data[nbase+4]);
          data6=AsciiToInt(data[nbase+5]);
          dainfo.fValue = (((data2&0x0F)<<4)|(data1&0x0F))*0.01+(((((((data6&0x0F)<<4)|(data5&0x0F))<<4)|(data4&0x0F))<<4)|(data3&0x0F));
          data_ptr->mValues[3] = dainfo;
          for(int i=0;i<5;++i)
          {
              data1=AsciiToInt(data[nbase+18+6*i]);
              data2=AsciiToInt(data[nbase+19+6*i]);
              data3=AsciiToInt(data[nbase+20+6*i]);
              data4=AsciiToInt(data[nbase+21+6*i]);
              data5=AsciiToInt(data[nbase+22+6*i]);
              data6=AsciiToInt(data[nbase+23+6*i]);
              dainfo.fValue = (((data2&0x0F)<<4)|(data1&0x0F))*0.01+(((((((data6&0x0F)<<4)|(data5&0x0F))<<4)|(data4&0x0F))<<4)|(data3&0x0F));
              data_ptr->mValues[4+i] = dainfo;
          }
          for(int j=0;j<2;++j)
          {
              data3=AsciiToInt(data[nbase+48+4*j]);
              data4=AsciiToInt(data[nbase+49+4*j]);
              data5=AsciiToInt(data[nbase+50+4*j]);
              data6=AsciiToInt(data[nbase+51+4*j]);
              dainfo.fValue = ((((((data6&0x0F)<<4)|(data5&0x0F))<<4)|(data4&0x0F))<<4)|(data3&0x0F);
              data_ptr->mValues[9+j] = dainfo;
          }
          dainfo.bType = true;
          for(int k=0;k<3;++k)
          {
              data1=AsciiToInt(data[nbase+56+k]);
              for(int i=0;i<4;++i)
              {
                  dainfo.fValue = (Getbit(data1,i)==0 ? 1:0);
                  data_ptr->mValues[11+k*4+i] = dainfo;
              }
          }
          return RE_SUCCESS;
      }
}
