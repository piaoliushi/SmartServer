#include "ckangtetransmmit.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#pragma execution_character_set("utf-8")
#include <sstream>


using namespace boost::property_tree;
using namespace boost::gregorian;

namespace hx_net{
     CKangteTransmmit::CKangteTransmmit(int subprotocol,int addresscode)
         :Transmmiter()
         ,m_subprotocol(subprotocol)
         ,m_addresscode(addresscode)
     {
     }

     int CKangteTransmmit::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
     {
         switch(m_subprotocol)
         {
         case KT_TV_Netport:{
             if(data[0]==0x55 && data[1]==0xAA)
                 return RE_SUCCESS;
             else
             {
                 unsigned char cDes[2]={0};
                 cDes[0]=0x55;
                 cDes[1]=0xAA;
                 return kmp(data,nDataLen,cDes,2);
             }
         }
         default:
             return RE_NOPROTOCOL;
         }
     }

     int CKangteTransmmit::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
     {
         switch(m_subprotocol)
         {
         case KT_TV_Netport:
             return KTTvNetPortData(data,data_ptr,nDataLen,runstate);
         }
         return RE_NOPROTOCOL;
     }

     int CKangteTransmmit::decode_msg_body(const string &data,DevMonitorDataPtr data_ptr,CmdType cmdType,int number, int &runstate)
     {
         switch(m_subprotocol)
         {
         case KT_HTTP:
             return KT_Http_NetPortData(data,data_ptr,cmdType,number,runstate);
         }
         return RE_NOPROTOCOL;
     }

     bool CKangteTransmmit::IsStandardCommand()
     {
         switch(m_subprotocol)
         {
         default:
             return false;
         }
     }

     void CKangteTransmmit::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
     {
         switch(m_subprotocol)
         {
         case KT_HTTP:
             break;
         default:
             break;
         }
     }

     void CKangteTransmmit::GetSignalCommand(int cmmType,int nIndex,CommandUnit &cmdUnit){
         switch(m_subprotocol)
         {
         case KT_HTTP:{
                if(cmmType == MSG_DEVICE_QUERY){
                    cmdUnit.sCommandId = "/trans.json";
                    cmdUnit.commandLen = cmdUnit.sCommandId.length();
                    cmdUnit.ackLen = 0;
                }else if(cmmType == MSG_TRANSMITTER_TURNON_OPR){
                    cmdUnit.sCommandId = "/ctrl.cgi?action=txon";
                    cmdUnit.commandLen = cmdUnit.sCommandId.length();
                    cmdUnit.ackLen = 0;
                }else if(cmmType == MSG_TRANSMITTER_TURNOFF_OPR){
                    cmdUnit.sCommandId = "/ctrl.cgi?action=txoff";
                    cmdUnit.commandLen = cmdUnit.sCommandId.length();
                    cmdUnit.ackLen = 0;
                }
         }
             break;
         default:
             break;
         }
     }

     void CKangteTransmmit::GetAllCmd(CommandAttribute &cmdAll)
     {
         switch(m_subprotocol)
         {
         case KT_TV_Netport:{
             CommandUnit tmUnit;
             tmUnit.commandLen = 11;
             tmUnit.ackLen = 0;
             tmUnit.commandId[0] = 0x55;
             tmUnit.commandId[1] = 0xAA;
             tmUnit.commandId[2] = 0x00;
             tmUnit.commandId[3] = 0x07;
             tmUnit.commandId[4] = 0x06;
             tmUnit.commandId[5] = 0x00;
             tmUnit.commandId[6] = 0x00;
             tmUnit.commandId[7] = 0x00;
             tmUnit.commandId[8] = 0x00;
             tmUnit.commandId[9] = m_addresscode;
             tmUnit.commandId[10] = 0x00;
             vector<CommandUnit> vtUnit;
             vtUnit.push_back(tmUnit);
             cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;
             tmUnit.commandId[4] = 0x03;
             vector<CommandUnit> vtTurnOnUnit;
             vtTurnOnUnit.push_back(tmUnit);
             cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtTurnOnUnit;
             tmUnit.commandId[4] = 0x04;
             vector<CommandUnit> vtTurnOffUnit;
             vtTurnOffUnit.push_back(tmUnit);
             cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnOffUnit;
         }
             break;
         case KT_HTTP:{
             CommandUnit tmUnit;
             tmUnit.sCommandId = "/trans.json";
             tmUnit.commandLen = tmUnit.sCommandId.length();
             tmUnit.ackLen = 0;
             vector<CommandUnit> vtUnit;
             vtUnit.push_back(tmUnit);
             cmdAll.mapCommand[MSG_DEVICE_QUERY] = vtUnit;

             tmUnit.sCommandId = "/ctrl.cgi?action=txon";
             tmUnit.commandLen = tmUnit.sCommandId.length();
             tmUnit.ackLen = 0;
             vector<CommandUnit> vtTurnOnUnit;
             vtTurnOnUnit.push_back(tmUnit);
             cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR] = vtTurnOnUnit;

             tmUnit.sCommandId = "/ctrl.cgi?action=txoff";
             tmUnit.commandLen = tmUnit.sCommandId.length();
             tmUnit.ackLen = 0;
             vector<CommandUnit> vtTurnOffUnit;
             vtTurnOffUnit.push_back(tmUnit);
             cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR] = vtTurnOffUnit;


         }
             break;
         }
     }

     int CKangteTransmmit::KTTvNetPortData(unsigned char *data, DevMonitorDataPtr data_ptr,
                                           int nDataLen, int &runstate)
     {
         int indexpos=0;
         float fwd =float((256*data[21] + data[22])*0.001);
         float refpwd  = float((256*data[23] + data[24])*0.001);
         DataInfo dainfo;
         dainfo.bType = false;
         dainfo.fValue = fwd;
         data_ptr->mValues[indexpos++] = dainfo;
         dainfo.fValue = refpwd;
         data_ptr->mValues[indexpos++] = dainfo;
         if(fwd*1000<=refpwd)
         {
             dainfo.fValue = 1.0;
         }
         else
         {
             dainfo.fValue = sqrt((fwd*1000+refpwd)/(fwd*1000-refpwd));
         }
         data_ptr->mValues[indexpos++] = dainfo;
         dainfo.fValue = float(256*data[25] + data[26]);
         data_ptr->mValues[indexpos++] = dainfo;
         dainfo.fValue = float((256*data[27] + data[28])*0.01);
         data_ptr->mValues[indexpos++] = dainfo;
         int nRemote = (256*data[29] + data[30]);
         dainfo.bType = true;
         if(nRemote>2000)
         {
             dainfo.fValue = 0;
         }
         else
         {
             dainfo.fValue = 1;
         }
         data_ptr->mValues[indexpos++] = dainfo;
         dainfo.bType = false;
         for(int i=0;i<8;++i)
         {
             for(int k=0;k<4;++k)
             {
                 dainfo.fValue = data[42+i*20+k*2];
                 data_ptr->mValues[indexpos++] = dainfo;
                 dainfo.fValue = data[43+i*20+k*2];
                 data_ptr->mValues[indexpos++] = dainfo;
             }
             dainfo.fValue = data[50+20*i];
             data_ptr->mValues[indexpos++] = dainfo;
             dainfo.fValue = data[51+20*i];
             data_ptr->mValues[indexpos++] = dainfo;
         }
         dainfo.bType = true;
         for(int j=0;j<4;j++)
         {
             dainfo.fValue = (float)((Getbit(data[202],j)==0)? 1:0);
             data_ptr->mValues[indexpos++] = dainfo;
         }
         char pf[5]={0};
         pf[0]=data[223];
         pf[1]=data[224];
         pf[2]=data[225];
         pf[3]=data[226];
         pf[4]=data[227];
         dainfo.bType = false;
         dainfo.fValue = (atof(pf));
         data_ptr->mValues[indexpos++] = dainfo;
         dainfo.fValue = data[228];
         data_ptr->mValues[indexpos++] = dainfo;
         dainfo.fValue = data[229];
         data_ptr->mValues[indexpos++] = dainfo;
         dainfo.fValue = data[230];
         data_ptr->mValues[indexpos++] = dainfo;
         return 0;
     }


     int CKangteTransmmit::KT_Http_NetPortData(const string &data,DevMonitorDataPtr data_ptr,
                                               CmdType cmdType,int number, int &runstate)
     {
         std::stringstream ss;
         ss << data;
         //string vswr_const_str = "驻波比  ";
         //StringTrim(vswr_const_str);
         boost::property_tree::ptree pt,p1,p2;
         //stream << data;
         try
         {
             boost::property_tree::read_json<ptree>( ss, pt);
             p1 = pt.get_child("param");

             for (ptree::iterator it = p1.begin(); it != p1.end(); ++it)
             {
                 p2 = it->second; //first为空
                 string strKey = p2.get<string>("desc");

                 gb2312ToUtf8(strKey);


                 StringTrim(strKey);
                 if(strKey == "Status"){
                      string strValue = p2.get<string>("val");
                      StringTrim(strValue);
                      DataInfo dainfo;
                      dainfo.bType = true;
                      dainfo.fValue = 0;
                      if(strValue == "ON"){
                          dainfo.fValue = 1;
                          runstate = dev_running;
                      }else if(strValue == "OFF"){
                          dainfo.fValue = 0;
                          runstate = dev_shutted_down;
                      }
                      data_ptr->mValues[3] = dainfo;
                 }else if(strKey == "Output Power" || strKey == "输出功率"){
                     string strValue = p2.get<string>("val");
                     StringTrim(strValue);
                     DataInfo dainfo;
                     dainfo.bType = false;
                     dainfo.fValue = atof(strValue.c_str());
                     dainfo.sValue = strValue;
                     data_ptr->mValues[0] = dainfo;
                 }else if(strKey == "Reflect Power" || strKey == "反射功率"){
                     string strValue = p2.get<string>("val");
                     StringTrim(strValue);
                     DataInfo dainfo;
                     dainfo.bType = false;
                     dainfo.fValue = atof(strValue.c_str());
                     dainfo.sValue = strValue;
                     data_ptr->mValues[1] = dainfo;
                 }else if(strKey == "VSWR" || strKey+" " == "驻波比 "){//|| strKey == vswr_const_str
                     string strValue = p2.get<string>("val");
                     StringTrim(strValue);
                     DataInfo dainfo;
                     dainfo.bType = false;
                     dainfo.fValue = atof(strValue.c_str());
                     dainfo.sValue = strValue;
                     data_ptr->mValues[2] = dainfo;
                 }else if(strKey == "Amp.1 Output"){
                     string strValue = p2.get<string>("val");
                     StringTrim(strValue);
                     DataInfo dainfo;
                     dainfo.bType = false;
                     dainfo.fValue = atof(strValue.c_str());
                     dainfo.sValue = strValue;
                     data_ptr->mValues[4] = dainfo;
                 }else if(strKey == "Amp.2 Output"){
                     string strValue = p2.get<string>("val");
                     StringTrim(strValue);
                     DataInfo dainfo;
                     dainfo.bType = false;
                     dainfo.fValue = atof(strValue.c_str());
                     dainfo.sValue = strValue;
                     data_ptr->mValues[5] = dainfo;
                 }else if(strKey == "Amp.3 Output"){
                     string strValue = p2.get<string>("val");
                     StringTrim(strValue);
                     DataInfo dainfo;
                     dainfo.bType = false;
                     dainfo.fValue = atof(strValue.c_str());
                     dainfo.sValue = strValue;
                     data_ptr->mValues[6] = dainfo;
                 }else if(strKey == "Amp.4 Output"){
                     string strValue = p2.get<string>("val");
                     StringTrim(strValue);
                     DataInfo dainfo;
                     dainfo.bType = false;
                     dainfo.fValue = atof(strValue.c_str());
                     dainfo.sValue = strValue;
                     data_ptr->mValues[7] = dainfo;
                 }else if(strKey+" " == "输出检波 "){
                     string strValue = p2.get<string>("val");
                     StringTrim(strValue);
                     DataInfo dainfo;
                     dainfo.bType = false;
                     dainfo.fValue = atof(strValue.c_str());
                     dainfo.sValue = strValue;
                     data_ptr->mValues[8] = dainfo;
                 }else if(strKey+" " == "反射检波 "){
                     string strValue = p2.get<string>("val");
                     StringTrim(strValue);
                     DataInfo dainfo;
                     dainfo.bType = false;
                     dainfo.fValue = atof(strValue.c_str());
                     dainfo.sValue = strValue;
                     data_ptr->mValues[9] = dainfo;
                 }
             }

             return 0;
         }
         catch(...)
         {
             cout<<"The JSON file cannot be parsed because the format is incorrect."<<endl;
             return -1;

         }

         return -1;
     }


}
