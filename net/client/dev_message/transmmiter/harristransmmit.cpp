#include "harristransmmit.h"
namespace hx_net{
         CHarrisTransmmit::CHarrisTransmmit(Tsmt_message_ptr ptsmessage,int subprotocol,int addresscode)
         :Transmmiter()
         ,m_subprotocol(subprotocol)
         ,m_addresscode(addresscode)
         ,m_pmessage(ptsmessage)
         {
             initOid();
         }

         int CHarrisTransmmit::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
         {
             switch(m_subprotocol)
             {
             case HARRIS_AM50:{
                 if(data[0]==0x48 && data[1]==0x42)
                     return 0;
                 else
                 {
                     unsigned char cDes[2]={0};
                     cDes[0]=0x48;
                     cDes[1]=0x42;
                     return kmp(data,nDataLen,cDes,2);
                 }
             }
                 break;
             default:
                 break;
             }
             return RE_UNKNOWDEV;
         }

         int CHarrisTransmmit::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
         {
             switch(m_subprotocol)
             {
             case HARRIS_AM50:{
                 return OnHarrisAm50(data,data_ptr,nDataLen,runstate);
             }
                 break;
             default:
                 break;
             }
             return RE_UNKNOWDEV;
         }
         int CHarrisTransmmit::decode_msg_body(Snmp *snmp,DevMonitorDataPtr data_ptr,CTarget *target,int& runstate)
         {
             switch(m_subprotocol)
             {
             case HARRIS_SNMP:
                 return get_snmp(snmp,data_ptr,target);
                 break;
             default:
                 break;
             }
             return RE_UNKNOWDEV;
         }

         bool CHarrisTransmmit::IsStandardCommand()
         {
             return false;
         }

         void CHarrisTransmmit::GetAllCmd(CommandAttribute &cmdAll)
         {
             switch(m_subprotocol)
             {
             case HARRIS_AM50:
             {
                 CommandUnit tmUnit;
                 tmUnit.commandLen = 11;
                 tmUnit.ackLen = 396;
                 tmUnit.commandId[0] = 0x48;
                 tmUnit.commandId[1] = 0x42;
                 tmUnit.commandId[2] = 0x00;
                 tmUnit.commandId[3] = 0x09;
                 tmUnit.commandId[4] = 0x00;
                 tmUnit.commandId[5] = 0x00;
                 tmUnit.commandId[6] = 0x46;
                 tmUnit.commandId[7] = 0x00;
                 tmUnit.commandId[8] = 0x00;
                 tmUnit.commandId[9] = 0xDB;
                 tmUnit.commandId[10] = 0x75;
                 cmdAll.mapCommand[MSG_DEVICE_QUERY].push_back(tmUnit);
                 tmUnit.commandLen = 14;
                 tmUnit.commandId[3] = 0x0C;
                 tmUnit.commandId[6] = 0x03;
                 tmUnit.commandId[7] = 0x0A;
                 tmUnit.commandId[9] = 0x71;
                 tmUnit.commandId[10] = 0x00;
                 tmUnit.commandId[11] = 0x00;
                 tmUnit.commandId[12] = 0x60;
                 tmUnit.commandId[13] = 0xC0;
                 cmdAll.mapCommand[MSG_TRANSMITTER_TURNON_OPR].push_back(tmUnit);
                 tmUnit.commandId[9] = 0x70;
                 tmUnit.commandId[12] = 0x70;
                 tmUnit.commandId[13] = 0xE1;
                 cmdAll.mapCommand[MSG_TRANSMITTER_TURNOFF_OPR].push_back(tmUnit);
                 tmUnit.commandId[9] = 0x77;
                 tmUnit.commandId[12] = 0x00;
                 tmUnit.commandId[13] = 0x06;
                 cmdAll.mapCommand[MSG_TRANSMITTER_RISE_POWER_OPR].push_back(tmUnit);
                 tmUnit.commandId[9] = 0x78;
                 tmUnit.commandId[12] = 0xF1;
                 tmUnit.commandId[13] = 0xE9;
                 cmdAll.mapCommand[MSG_TRANSMITTER_REDUCE_POWER_OPR].push_back(tmUnit);
             }
                 break;
             default:
                 break;
             }
         }

         void CHarrisTransmmit::harris_Callback(int reason, Snmp *session, Pdu &pdu, SnmpTarget &target)
         {

             switch(m_subprotocol)
             {
             case HARRIS_SNMP:
             {
                 harris_decode(reason,session,pdu,target);
             }
                 break;
             default:
                 break;
             }
         }

         bool CHarrisTransmmit::exec_cmd(Snmp *snmp, int cmdtype, CTarget *target)
         {
             switch(m_subprotocol)
             {
             case HARRIS_SNMP:
             {
                 Vb vbl;
                 Pdu cmdpdu;
                 vbl.set_oid(Oid("1.3.6.1.4.1.290.9.2.1.1.5.3.2.0"));
                 switch(cmdtype)
                 {
                 case MSG_TRANSMITTER_TURNON_OPR:{
                     vbl.set_value(1);
                 }
                     break;
                 case MSG_TRANSMITTER_TURNOFF_OPR:{
                     vbl.set_value(2);
                 }
                     break;
                 case MSG_TRANSMITTER_RISE_POWER_OPR:{
                     vbl.set_value(3);
                 }
                     break;
                 case MSG_TRANSMITTER_REDUCE_POWER_OPR:{
                     vbl.set_value(4);
                 }
                     break;
                 default:
                     return false;
                 }

                 cmdpdu+=vbl;
                 OctetStr community("private");
                 target->set_writecommunity(community);
                 int iresult =snmp->set(cmdpdu,*target);
                 if(iresult!=SNMP_CLASS_SUCCESS)
                     return false;
             }
                 break;
             default:
                 break;
             }
             return true;
         }

         void CHarrisTransmmit::initOid()
         {
             switch(m_subprotocol)
             {
             case HARRIS_SNMP:{
                 map_Oid["1.3.6.1.4.1.290.9.2.1.1.4.3.1.0"] = 0;
                 Vb vbl;
                 vbl.set_oid(Oid("1.3.6.1.4.1.290.9.2.1.1.4.3.1.0"));
                 query_pdu += vbl;
                 map_Oid["1.3.6.1.4.1.290.9.2.1.1.4.3.2.0"] = 1;
                 vbl.set_oid(Oid("1.3.6.1.4.1.290.9.2.1.1.4.3.2.0"));
                 query_pdu += vbl;
                 map_Oid["1.3.6.1.4.1.290.9.2.1.1.4.3.3.0"] = 2;
                 vbl.set_oid(Oid("1.3.6.1.4.1.290.9.2.1.1.4.3.3.0"));
                 query_pdu += vbl;
                 map_Oid["1.3.6.1.4.1.290.9.2.1.1.4.3.4.0"] = 3;
                 vbl.set_oid(Oid("1.3.6.1.4.1.290.9.2.1.1.4.3.4.0"));
                 query_pdu += vbl;
                 map_Oid["1.3.6.1.4.1.290.9.2.1.1.4.2.1.0"] = 4;
                 vbl.set_oid(Oid("1.3.6.1.4.1.290.9.2.1.1.4.2.1.0"));
                 query_pdu += vbl;
                 map_Oid["1.3.6.1.4.1.290.9.2.1.1.3.1.1.0"] = 5;
                 vbl.set_oid(Oid("1.3.6.1.4.1.290.9.2.1.1.3.1.1.0"));
                 query_pdu += vbl;
             }
                 break;
             default:
                 break;
             }
         }

         int CHarrisTransmmit::OnHarrisAm50(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
         {
             int HiByte;  //高字节
             int LoByte;  //低字节
             int tmp;
             float ftmp;
             int indexpos = 0;
             DataInfo dtinfo;
             dtinfo.bType = false;
             HiByte = data[9];
             LoByte = data[10];
             dtinfo.fValue = (HiByte*256+LoByte)*0.001;
             data_ptr->mValues[indexpos++] = dtinfo;
             HiByte = data[11];
             LoByte = data[12];
             dtinfo.fValue = (HiByte*256+LoByte);
             data_ptr->mValues[indexpos++] = dtinfo;
             *(((char*)(&ftmp) + 0)) = data[13];
             *(((char*)(&ftmp) + 1)) = data[14];
             *(((char*)(&ftmp) + 2)) = data[15];
             *(((char*)(&ftmp) + 3)) = data[16];
             if(data_ptr->mValues[0].fValue==0 || data_ptr->mValues[1].fValue==0)
                 dtinfo.fValue = 1.0;
             else
                 dtinfo.fValue = ftmp;
             data_ptr->mValues[indexpos++] = dtinfo;
             for(int i=0;i<4;++i)
             {
                 dtinfo.fValue = data[17+2*i]*256+data[18+2*i];
                 data_ptr->mValues[indexpos++] = dtinfo;
             }
             dtinfo.fValue = data[73]*256+data[74];
             data_ptr->mValues[indexpos++] = dtinfo;
             dtinfo.fValue = data[85]*256+data[86];
             data_ptr->mValues[indexpos++] = dtinfo;
             dtinfo.fValue = data[97]*256+data[98];
             data_ptr->mValues[indexpos++] = dtinfo;
             dtinfo.fValue = data[109]*256+data[110];
             data_ptr->mValues[indexpos++] = dtinfo;
             dtinfo.fValue = data[121]*256+data[122];
             data_ptr->mValues[indexpos++] = dtinfo;
             for(int i=0;i<4;++i)
             {
                 dtinfo.fValue = data[251+2*i]*256+data[252+2*i];
                 data_ptr->mValues[indexpos++] = dtinfo;
             }
             dtinfo.fValue = data[267]*256+data[268];
             data_ptr->mValues[indexpos++] = dtinfo;
             dtinfo.bType = true;
             for(int i=0;i<2;++i)
             {
                 dtinfo.fValue = (data[269+2*i]*256+data[270+2*i]+1)%2;
                 data_ptr->mValues[indexpos++] = dtinfo;
             }
             return 0;
         }
         void harris_aysnc_callback(int reason, Snmp *session,
                                  Pdu &pdu, SnmpTarget &target, void *cd)
         {
           /*  if(reason != SNMP_CLASS_ASYNC_RESPONSE && reason != SNMP_CLASS_NOTIFICATION){
                 cout << "Failed to issue SNMP aysnc_callback: (" << reason  << ") "
                        << session->error_msg(reason) << endl;
                 return;
             }*/

             if (cd)
                 ((CHarrisTransmmit*)cd)->harris_Callback(reason, session, pdu, target);
         }

         int CHarrisTransmmit::get_snmp(Snmp *snmp, DevMonitorDataPtr data_ptr, CTarget *target)
         {
             {
                 boost::recursive_mutex::scoped_lock lock(data_mutex);
                 if(data_ptr)
                     curdata_ptr = data_ptr;
             }
             int status = snmp->get(query_pdu,*target,harris_aysnc_callback,this);
             if(status)
                 return -1;

             return 0;
         }

         void CHarrisTransmmit::harris_decode(int reason, Snmp *session, Pdu &pdu, SnmpTarget &target)
         {
             int pdu_error = pdu.get_error_status();
             std::cout<<"harris_decode"<<endl;
             if (pdu_error)
             {

                 std::cout<<"ERROR"<<endl;
                 return;
             }
             if (pdu.get_vb_count() == 0)
             {

                 std::cout<<"COUNT==0"<<endl;
                 return;
             }
             int vbcount = pdu.get_vb_count();
             boost::recursive_mutex::scoped_lock lock(data_mutex);
             for(int i=0;i<vbcount;++i)
             {
                 DataInfo dainfo;
                 Vb nextVb;
                 pdu.get_vb(nextVb, i);
                 string cur_oid = nextVb.get_printable_oid();
                 string cur_value =nextVb.get_printable_value();
                 dainfo.bType = true;
                 dainfo.fValue = atof(cur_value.c_str());
                 map<string,int>::iterator iter = map_Oid.find(cur_oid);
                 std::cout<<cur_oid<<endl;
                 if(iter!=map_Oid.end())
                 {
                     if((*iter).second<4)
                     {
                         dainfo.bType = false;
                         dainfo.fValue = dainfo.fValue*0.001;
                     }
                     curdata_ptr->mValues[(*iter).second] = dainfo;
                 }
             }
             m_pmessage->aysnc_data(curdata_ptr);
         }
}
