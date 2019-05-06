#include "zttransmmit.h"

namespace hx_net
{
   ZtTransmmit::ZtTransmmit(int subprotocol,int addresscode)
       :Transmmiter()
       ,m_subprotocol(subprotocol)
       ,m_addresscode(addresscode)
   {
   }

   int ZtTransmmit::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
   {
       switch(m_subprotocol)
       {
       case ZT_AM_1KW:
       case ZT_AM_10KW:{
           if(data[0]==0xAA && data[1]==0x55)
                           return data[5]+3;
                       else
                           return RE_HEADERROR;
       }
       default:
           return RE_NOPROTOCOL;
       }
       return RE_NOPROTOCOL;
   }

   int ZtTransmmit::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
   {
       switch(m_subprotocol)
       {
       case ZT_AM_10KW:
           return ZtAm10KwData(data,data_ptr,nDataLen,runstate);
       case ZT_AM_1KW:
           return ZtAm1KwData(data,data_ptr,nDataLen,runstate);
       default:
           return RE_NOPROTOCOL;
       }
       return RE_NOPROTOCOL;
   }

   bool ZtTransmmit::IsStandardCommand()
   {
       switch(m_subprotocol)
       {
       case ZT_AM_10KW:
       case ZT_AM_1KW:
           return true;
       default:
           return false;
       }
       return false;
   }

   void ZtTransmmit::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
   {

   }

   void ZtTransmmit::GetAllCmd(CommandAttribute &cmdAll)
   {
       CommandUnit tmUnit;
       switch(m_subprotocol)
       {
       case ZT_AM_1KW:
       case ZT_AM_10KW:{
           tmUnit.commandLen = 8;
           tmUnit.ackLen = 6;
           tmUnit.commandId[0] = 0xAA;
           tmUnit.commandId[1] = 0x55;
           tmUnit.commandId[2] = 0x00;
           tmUnit.commandId[3] = m_addresscode;
           tmUnit.commandId[4] = 0x01;
           tmUnit.commandId[5] = (tmUnit.commandId[2]+tmUnit.commandId[3]+tmUnit.commandId[4])&0xFF;
           tmUnit.commandId[6] = 0xCC;
           tmUnit.commandId[7] = 0x33;
           cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
           tmUnit.ackLen = 0;
           tmUnit.commandId[4] = 0x0B;
           tmUnit.commandId[5] = (tmUnit.commandId[2]+tmUnit.commandId[3]+tmUnit.commandId[4])&0xFF;
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
           tmUnit.commandId[4] = 0x0C;
           tmUnit.commandId[5] = (tmUnit.commandId[2]+tmUnit.commandId[3]+tmUnit.commandId[4])&0xFF;
           cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
           tmUnit.commandId[4] = 0x0D;
           tmUnit.commandId[5] = (tmUnit.commandId[2]+tmUnit.commandId[3]+tmUnit.commandId[4])&0xFF;
           cmdAll.mapCommand[MSG_TRANSMITTER_RISE_POWER_OPR].push_back(tmUnit);
           tmUnit.commandId[4] = 0x0E;
           tmUnit.commandId[5] = (tmUnit.commandId[2]+tmUnit.commandId[3]+tmUnit.commandId[4])&0xFF;
           cmdAll.mapCommand[MSG_TRANSMITTER_REDUCE_POWER_OPR].push_back(tmUnit);
       }
           break;
       default:
           break;
       }
   }

   int ZtTransmmit::ZtAm10KwData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
   {
       if(data[4]!=0x01)
               return RE_CMDACK;
       DataInfo dtinfo;
       dtinfo.bType = false;
       int ivalue=0;
       ivalue = data[8]+data[9]*256;
       if(ivalue>2052)
       {
           dtinfo.fValue = (pow((ivalue/460.00),3.00)+(11.00/39.00)*pow((ivalue-2052),1.00/3.00))*0.1;
       }
       else
       {
           dtinfo.fValue = pow((ivalue/460.00),2.00)*(ivalue/460.00)*0.1;
       }
       data_ptr->mValues[0] = dtinfo;
       ivalue = data[10]+data[11]*256;
       if(ivalue>205)
       {
           dtinfo.fValue = (ivalue-205)/0.41;
       }
       else{
           dtinfo.fValue = 0;
       }
       data_ptr->mValues[1] = dtinfo;
       if(data_ptr->mValues[0].fValue*1000>data_ptr->mValues[1].fValue)
       {
           dtinfo.fValue =  sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
       }
       else
       {
           dtinfo.fValue = 0;
       }
       data_ptr->mValues[2] = dtinfo;
       ivalue = data[6]+data[7]*256;
       dtinfo.fValue = ivalue*0.0125;
       data_ptr->mValues[3] = dtinfo;
       ivalue = data[12]+data[13]*256;
       dtinfo.fValue = ivalue/31.67;
       data_ptr->mValues[4] = dtinfo;
       ivalue = data[14]+data[15]*256;
       if(ivalue>500)
       {
           dtinfo.fValue = ivalue/30.75+170;
       }
       else
       {
           dtinfo.fValue = 0;
       }
       data_ptr->mValues[5] = dtinfo;
       ivalue = data[16]+data[17]*256;
       if(ivalue>100)
       {
           dtinfo.fValue = ivalue/27.774+11.43;
       }
       else
       {
           dtinfo.fValue = 0;
       }
       data_ptr->mValues[6] = dtinfo;
       ivalue = data[18]+data[19]*256;
       dtinfo.fValue = ivalue*693/40960.00;
       data_ptr->mValues[7] = dtinfo;
       ivalue = data[20]+data[21]*256;
       dtinfo.fValue = ivalue*693/40960.00;
       data_ptr->mValues[8] = dtinfo;
       ivalue = data[22]+data[23]*256;
       dtinfo.fValue = ivalue;
       data_ptr->mValues[9] = dtinfo;
       dtinfo.bType = true;
       dtinfo.fValue = data[25]==1 ? dev_running:dev_shutted_down;
       data_ptr->mValues[10] = dtinfo;
       runstate = data[25]==1 ? dev_running:dev_shutted_down;
       dtinfo.fValue = 0;
       for(int i=11;i<26;++i)
       {
          data_ptr->mValues[i] = dtinfo;
       }
       if(data[24]!=0)
       {
           dtinfo.fValue = 1;
           data_ptr->mValues[data[24]+10] = dtinfo;
       }
       return RE_SUCCESS;
   }

   int ZtTransmmit::ZtAm1KwData(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
   {
       if(data[4]!=0x01)
               return RE_CMDACK;
       DataInfo dtinfo;
       dtinfo.bType = false;
       int ivalue=0;
       ivalue = data[8]+data[9]*256;
       dtinfo.fValue = (80.79*pow(ivalue*2.95/4096.00,3)+181.49*pow(ivalue*2.95/4096.00,2)+59.37*(ivalue*2.95/4096.00))*0.001;
       data_ptr->mValues[0] = dtinfo;
       ivalue = data[10]+data[11]*256;
       dtinfo.fValue = 80.79*pow(ivalue*2.95/4096.00,3)+181.49*pow(ivalue*2.95/4096.00,2)+59.37*(ivalue*2.95/4096.00);
       data_ptr->mValues[1] = dtinfo;
       if(data_ptr->mValues[0].fValue*1000>data_ptr->mValues[1].fValue)
       {
           dtinfo.fValue =  sqrt((data_ptr->mValues[0].fValue*1000+data_ptr->mValues[1].fValue)/(data_ptr->mValues[0].fValue*1000-data_ptr->mValues[1].fValue));
       }
       else
       {
           dtinfo.fValue = 0;
       }
       data_ptr->mValues[2] = dtinfo;
       ivalue = data[6]+data[7]*256;
       dtinfo.fValue = ivalue/134.00;
       data_ptr->mValues[3] = dtinfo;
       ivalue = data[12]+data[13]*256;
       if(ivalue>1273)
       dtinfo.fValue = ivalue/31.67;
       data_ptr->mValues[4] = dtinfo;
       ivalue = data[14]+data[15]*256;
       if(ivalue>500)
       {
           dtinfo.fValue = ivalue/30.75+170;
       }
       else
       {
           dtinfo.fValue = 0;
       }
       data_ptr->mValues[5] = dtinfo;
       ivalue = data[16]+data[17]*256;

       dtinfo.fValue = ivalue/26.83;

       data_ptr->mValues[6] = dtinfo;
       ivalue = data[18]+data[19]*256;
       dtinfo.fValue = ivalue*693/40960.00;
       data_ptr->mValues[7] = dtinfo;
       ivalue = data[20]+data[21]*256;
       dtinfo.fValue = ivalue*693/40960.00;
       data_ptr->mValues[8] = dtinfo;
       ivalue = data[22]+data[23]*256;
       dtinfo.fValue = ivalue;
       data_ptr->mValues[9] = dtinfo;
       dtinfo.bType = true;
       dtinfo.fValue = data[25]==1 ? dev_running:dev_shutted_down;
       data_ptr->mValues[10] = dtinfo;
       runstate = data[25]==1 ? dev_running:dev_shutted_down;
       dtinfo.fValue = 0;
       for(int i=11;i<26;++i)
       {
          data_ptr->mValues[i] = dtinfo;
       }
       if(data[24]!=0)
       {
           dtinfo.fValue = 1;
           data_ptr->mValues[data[24]+10] = dtinfo;
       }
       return RE_SUCCESS;
   }
}
