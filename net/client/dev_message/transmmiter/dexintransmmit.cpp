#include "dexintransmmit.h"
namespace hx_net{
         DeXinTransmmit::DeXinTransmmit(int subprotocol,int addresscode)
         :Transmmiter()
         ,m_subprotocol(subprotocol)
         ,m_addresscode(addresscode)
         {
         }

         DeXinTransmmit::~DeXinTransmmit()
         {

         }

         int DeXinTransmmit::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
         {
             switch(m_subprotocol)
             {
             case DX_DIG_TV:
             {
                         if(data[0]==0xAA && data[1]==m_addresscode)
                             return ((data[4]<<8)|data[3])+1;
                         else
                             return -1;
                     }
             }
             return -1;
         }

         int DeXinTransmmit::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
         {
             switch(m_subprotocol)
             {
             case DX_DIG_TV:
                 return OnDigTVData(data,data_ptr,nDataLen,runstate);
             }
             return -1;
         }

         bool DeXinTransmmit::IsStandardCommand()
         {
             switch(m_subprotocol)
             {
             case DX_DIG_TV:

                 return true;
             }
             return false;
         }

         void DeXinTransmmit::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
         {
             switch(m_subprotocol)
                 {
                 case DX_DIG_TV:
                     {
                         if(lpParam->cparams_size()<1)
                             return;
                         if(atoi(lpParam->cparams(0).sparamvalue().c_str())==0)
                         {
                             cmdUnit.ackLen = 5;
                             cmdUnit.commandLen = 11;
                             cmdUnit.commandId[0] = 0xAA;
                             cmdUnit.commandId[1] = m_addresscode;
                             cmdUnit.commandId[2] = 0x0F;
                             cmdUnit.commandId[3] = 0x05;
                             cmdUnit.commandId[4] = 0x00;
                             cmdUnit.commandId[5] = 0x09;
                             cmdUnit.commandId[6] = 0x0A;
                             cmdUnit.commandId[7] = 0x01;
                             cmdUnit.commandId[8] = 0x00;
                             cmdUnit.commandId[9] = 0x01;
                             cmdUnit.commandId[10] = cmdUnit.commandId[0]^cmdUnit.commandId[1]^cmdUnit.commandId[2]^cmdUnit.commandId[3]^cmdUnit.commandId[4]^
                                 cmdUnit.commandId[5]^cmdUnit.commandId[6]^cmdUnit.commandId[7]^cmdUnit.commandId[8]^cmdUnit.commandId[9];
                         }
                     }
                     break;
                 default:
                     break;
                 }
         }

         void DeXinTransmmit::GetAllCmd(CommandAttribute &cmdAll)
         {
             switch(m_subprotocol)
                 {

                 case DX_DUT_8413:
                     {
                         CommandUnit tmUnit;
                         tmUnit.commandLen = 6;
                         tmUnit.ackLen = 446;
                         tmUnit.commandId[0] = 0x7E;
                         tmUnit.commandId[1] = 0xE7;
                         tmUnit.commandId[2] = 0x00;
                         tmUnit.commandId[3] = 0x02;
                         tmUnit.commandId[4] = 0x11;
                         tmUnit.commandId[5] = 0x0D;
                         vector<CommandUnit> vtUnit;
                         vtUnit.push_back(tmUnit);
                         cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;

                         tmUnit.commandId[4] = 0x1E;
                         vector<CommandUnit> vtHTurnOnUnit;
                         vtHTurnOnUnit.push_back(tmUnit);
                         cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtHTurnOnUnit;
                         tmUnit.commandId[4] = 0x1F;
                         vector<CommandUnit> vtTurnOffUnit;
                         vtTurnOffUnit.push_back(tmUnit);
                         cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnOffUnit;

                         tmUnit.commandId[4] = 0x16;
                         vector<CommandUnit> vtResetUnit;
                         vtResetUnit.push_back(tmUnit);
                         cmdAll.mapCommand[MSG_DEV_RESET_OPR] = vtResetUnit;
                     }
                     break;
                 case DX_DIG_TV:
                     {
                         CommandUnit tmUnit;
                         tmUnit.ackLen = 5;
                         tmUnit.commandLen = 6;
                         tmUnit.commandId[0] = 0xAA;
                         tmUnit.commandId[1] = m_addresscode;
                         tmUnit.commandId[2] = 0x43;
                         tmUnit.commandId[3] = 0x00;
                         tmUnit.commandId[4] = 0x00;
                         tmUnit.commandId[5] = tmUnit.commandId[0]^tmUnit.commandId[1]^tmUnit.commandId[2]^tmUnit.commandId[3]^tmUnit.commandId[4];
                         vector<CommandUnit> vtUnit;
                         vtUnit.push_back(tmUnit);
                         cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
                         tmUnit.commandLen = 11;
                         tmUnit.commandId[2] = 0x0F;
                         tmUnit.commandId[3] = 0x05;
                         tmUnit.commandId[5] = 0x01;
                         tmUnit.commandId[6] = 0x0A;
                         tmUnit.commandId[7] = 0x01;
                         tmUnit.commandId[8] = 0x00;
                         tmUnit.commandId[9] = 0x00;
                         tmUnit.commandId[10] = tmUnit.commandId[0]^tmUnit.commandId[1]^tmUnit.commandId[2]^tmUnit.commandId[3]^tmUnit.commandId[4]^
                             tmUnit.commandId[5]^tmUnit.commandId[6]^tmUnit.commandId[7]^tmUnit.commandId[8]^tmUnit.commandId[9];
                         vector<CommandUnit> vtTurnOffUnit;
                         vtTurnOffUnit.push_back(tmUnit);
                         cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnOffUnit;
                         tmUnit.commandId[9] = 0x11;
                         tmUnit.commandId[10] = tmUnit.commandId[0]^tmUnit.commandId[1]^tmUnit.commandId[2]^tmUnit.commandId[3]^tmUnit.commandId[4]^
                             tmUnit.commandId[5]^tmUnit.commandId[6]^tmUnit.commandId[7]^tmUnit.commandId[8]^tmUnit.commandId[9];
                         vector<CommandUnit> vtHTurnOnUnit;
                         vtHTurnOnUnit.push_back(tmUnit);
                         cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtHTurnOnUnit;
                     }
                     break;
                 default:
                     break;
             }
         }

         int DeXinTransmmit::OnDigTVData(unsigned char *data,DevMonitorDataPtr data_ptr,int nDataLen,int& runstate)
         {
             if(data[2]!=0x44)
                     return -2;
                 int lastCount = nDataLen;
                 data=data+5;
                 lastCount-=5;
                 int nmodNum=0;
                 while(lastCount>5)
                 {
                     int nParam = (data[1]<<8)|data[0];
                     int nPaLen = (data[3]<<8)|data[2];
                     if(lastCount<(nPaLen+4))
                         break;
                     //调用单元解析
                     UnitData(data,data_ptr,nParam,nPaLen,nmodNum,runstate);
                     data+=(nPaLen+4);
                     lastCount-=(nPaLen+4);
                 }
                 return 0;
         }

         void DeXinTransmmit::UnitData(unsigned char *data,DevMonitorDataPtr data_ptr, int iparam, int ipalen, int &imodnum,int& runstate)
         {
             int idata = 0;
             DataInfo dainfo;
             QString saveStr;
             for(int i=ipalen;i>0;--i)
             {
                 idata = (idata<<8)|data[3+i];
             }
             switch(iparam)
             {
                 case 0x0201:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata*0.001;
                         data_ptr->mValues[0] = dainfo;
                     }
                     break;
                 case 0x0202:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata*0.1;
                         data_ptr->mValues[1] = dainfo;
                     }
                     break;
                 case 0x0203:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata;
                         data_ptr->mValues[3] = dainfo;
                     }
                     break;
                 case 0x0204:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata*0.1;
                         data_ptr->mValues[4] = dainfo;
                     }
                     break;
                 case 0x0205:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata*0.1;
                         data_ptr->mValues[2] = dainfo;
                     }
                     break;
                 case 0x0206:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata;
                         data_ptr->mValues[5] = dainfo;
                     }
                     break;
                 case 0x0208:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata*0.1;
                         data_ptr->mValues[6] = dainfo;
                     }
                     break;
                 case 0x0209:
                     {

                         dainfo.bType = true;
                         dainfo.fValue = Getbit(idata,0);
                         data_ptr->mValues[7] = dainfo;
                     }
                     break;
                 case 0x020A:
                     {
                         dainfo.bType = true;
                         dainfo.fValue = Getbit(idata,0);
                         data_ptr->mValues[8] = dainfo;
                         dainfo.fValue = Getbit(idata,1);
                         data_ptr->mValues[9] = dainfo;
                         dainfo.fValue = Getbit(idata,4);
                         if(Getbit(idata,4)==0)
                            runstate = dev_shutted_down;
                         else
                             runstate = dev_running;
                         data_ptr->mValues[10] = dainfo;
                     }
                     break;
                 case 0x020B:
                     {
                         dainfo.bType = true;
                         dainfo.fValue = Getbit(idata,0);
                         data_ptr->mValues[11] = dainfo;
                         dainfo.fValue = Getbit(idata,6);
                         data_ptr->mValues[12] = dainfo;
                         dainfo.fValue = Getbit(idata,12);
                         data_ptr->mValues[13] = dainfo;
                         dainfo.fValue = Getbit(idata,13);
                         data_ptr->mValues[14] = dainfo;
                         dainfo.bType = false;
                         dainfo.fValue = (idata>>14)&0x03;
                         data_ptr->mValues[15] = dainfo;
                     }
                     break;
                 case 0x0009:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata;
                         data_ptr->mValues[16] = dainfo;
                     }
                     break;
                 case 0x0312:
                     {
                         dainfo.bType = true;
                         dainfo.fValue = (idata==0 ? 0:1);
                         data_ptr->mValues[17] = dainfo;
                     }
                     break;
                 case 0x0313:
                     {
                         dainfo.bType = true;
                         dainfo.fValue = (idata==0 ? 0:1);
                         data_ptr->mValues[18] = dainfo;
                     }
                     break;
                 case 0x0301:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata*0.1;
                         data_ptr->mValues[19] = dainfo;
                     }
                     break;
                 case 0x0302:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata*0.1;
                         data_ptr->mValues[20] = dainfo;
                     }
                     break;
                 case 0x0303:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata;
                         data_ptr->mValues[21] = dainfo;
                     }
                     break;
                 case 0x0304:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata;
                         data_ptr->mValues[22] = dainfo;
                     }
                     break;
                 case 0x0305:
                     {
                         dainfo.bType = true;
                         dainfo.fValue = idata==0 ? 0:1;
                         data_ptr->mValues[23] = dainfo;
                     }
                     break;
                 case 0x0310:
                     {
                         dainfo.bType = true;
                         dainfo.fValue = Getbit(idata,0);
                         data_ptr->mValues[24] = dainfo;
                         dainfo.fValue = Getbit(idata,1);
                         data_ptr->mValues[25] = dainfo;
                         dainfo.fValue = Getbit(idata,4);
                         data_ptr->mValues[26] = dainfo;
                         dainfo.fValue = Getbit(idata,5);
                         data_ptr->mValues[27] = dainfo;
                         dainfo.fValue = Getbit(idata,6);
                         data_ptr->mValues[28] = dainfo;
                         dainfo.fValue = Getbit(idata,7);
                         data_ptr->mValues[29] = dainfo;
                         dainfo.fValue = Getbit(idata,15);
                         data_ptr->mValues[30] = dainfo;
                         dainfo.fValue = Getbit(idata,16);
                         data_ptr->mValues[31] = dainfo;
                         dainfo.fValue = Getbit(idata,17);
                         data_ptr->mValues[32] = dainfo;
                         dainfo.fValue = Getbit(idata,18);
                         data_ptr->mValues[33] = dainfo;
                         dainfo.fValue = Getbit(idata,19);
                         data_ptr->mValues[34] = dainfo;
                         dainfo.fValue = Getbit(idata,20);
                         data_ptr->mValues[35] = dainfo;
                         dainfo.fValue = Getbit(idata,21);
                         data_ptr->mValues[36] = dainfo;
                     }
                     break;
                 case 0x000A:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata;
                         data_ptr->mValues[37] = dainfo;
                     }
                     break;
                 case 0x0400:
                     imodnum = idata;
                     break;
                 case 0x0401:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata*0.1;
                         data_ptr->mValues[38+10*(imodnum-1)] = dainfo;
                     }
                     break;
                 case 0x0402:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata;
                         data_ptr->mValues[39+10*(imodnum-1)] = dainfo;
                     }
                     break;
                 case 0x0403:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata;
                         data_ptr->mValues[40+10*(imodnum-1)] = dainfo;
                     }
                     break;
                 case 0x0404:
                     {
                         for(int i=0;i<ipalen/2;++i)
                         {
                             dainfo.bType = false;
                             dainfo.fValue = (idata>>16*(ipalen/2-i-1))&0xFFFF;
                             data_ptr->mValues[41+10*(imodnum-1)+i] = dainfo;
                         }
                     }
                     break;
                 case 0x0405:
                     {
                         dainfo.bType = true;
                         dainfo.fValue = Getbit(idata,0);
                         data_ptr->mValues[45+10*(imodnum-1)] = dainfo;
                         dainfo.bType = true;
                         dainfo.fValue = Getbit(idata,1);
                         data_ptr->mValues[46+10*(imodnum-1)]=dainfo;
                     }
                     break;
                 case 0x0406:
                     {
                         dainfo.bType = false;
                         dainfo.fValue =idata*0.1;
                         data_ptr->mValues[46+10*(imodnum-1)]=dainfo;
                     }
                     break;
                 case 0x000B:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata;
                         data_ptr->mValues[78]=dainfo;
                     }
                     break;
                 case 0x0700:
                     {
                         imodnum = idata;
                     }
                     break;
                 case 0x0701:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata*0.1;
                         data_ptr->mValues[79+5*(imodnum-1)]=dainfo;
                     }
                     break;
                 case 0x0702:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata*0.1;
                         data_ptr->mValues[80+5*(imodnum-1)]=dainfo;
                     }
                     break;
                 case 0x0703:
                     {
                         dainfo.bType = true;
                         dainfo.fValue = Getbit(idata,0);
                         data_ptr->mValues[81+5*(imodnum-1)]=dainfo;
                         dainfo.fValue = Getbit(idata,1);
                         data_ptr->mValues[82+5*(imodnum-1)]=dainfo;
                     }
                     break;
                 case 0x0704:
                     {
                         dainfo.bType = false;
                         dainfo.fValue = idata*0.1;
                         data_ptr->mValues[83+5*(imodnum-1)]=dainfo;
                     }
                     break;
                 }
         }
}
