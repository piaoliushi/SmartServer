#include "gsbrtransmmit.h"
//#include "./snmp_pp/snmp_pp.h"
//#define rflevel	"1.3.6.1.4.1.8201.5.7.1.1.4.0"
namespace hx_net{
          GsbrTransmmit::GsbrTransmmit(int subprotocol,int addresscode)
                        :Transmmiter()
                        ,m_subprotocol(subprotocol)
                        ,m_addresscode(addresscode)
          {
              if(subprotocol==GSBR_SNMP)
              {
                  Vb vbl[16];
                  vbl[0].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.1.0"));
                  vbl[1].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.2.0"));
                  vbl[2].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.3.0"));
                  vbl[3].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.4.0"));
                  vbl[4].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.5.0"));
                  vbl[5].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.6.0"));
                  vbl[6].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.7.0"));
                  vbl[7].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.8.0"));
                  vbl[8].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.9.0"));
                  vbl[9].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.10.0"));
                  vbl[10].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.11.0"));
                  vbl[11].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.12.0"));
                  vbl[12].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.13.0"));
                  vbl[13].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.14.0"));
                  vbl[14].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.15.0"));
                  vbl[15].set_oid(Oid(".1.3.6.1.4.1.17409.100.3.2.1.16.0"));
                  for (int i=0; i<16;i++)
                      snmp_pdu += vbl[i];
              }
          }


          hx_net::GsbrTransmmit::~GsbrTransmmit()
          {

          }

          int GsbrTransmmit::check_msg_header(unsigned char *data, int nDataLen, CmdType cmdType, int number)
          {
              switch(m_subprotocol)
              {
              default:
                  break;
              }
              return RE_UNKNOWDEV;
          }

          int GsbrTransmmit::decode_msg_body(unsigned char *data, DevMonitorDataPtr data_ptr, int nDataLen, int &runstate)
          {
              switch(m_subprotocol)
              {
              default:
                  break;
              }
              return RE_UNKNOWDEV;
          }

          int GsbrTransmmit::decode_msg_body(Snmp *snmp, DevMonitorDataPtr data_ptr, CTarget *target, int &runstate)
          {
              switch(m_subprotocol)
              {
              case GSBR_SNMP:
                  return get_snmp(snmp,data_ptr,target);
              default:
                  break;
              }
              return RE_UNKNOWDEV;
          }

          bool GsbrTransmmit::IsStandardCommand()
          {
              return false;
          }

          void GsbrTransmmit::GetSignalCommand(devCommdMsgPtr lpParam, CommandUnit &cmdUnit)
          {
              switch(m_subprotocol)
              {
              default:
                  break;
              }
          }

          void GsbrTransmmit::GetAllCmd(CommandAttribute &cmdAll)
          {
              switch(m_subprotocol)
              {
              default:
                  break;
              }
          }

          int GsbrTransmmit::get_snmp(Snmp *snmp, DevMonitorDataPtr data_ptr, CTarget *target)
          {
              int status = snmp->get(snmp_pdu,*target);
              if(status)
                  return -1;
              else
              {
                  int vbcount = snmp_pdu.get_vb_count();

                  for (int i=0; i<vbcount; i++)
                  {
                      DataInfo dainfo;
                      Vb nextVb;
                      snmp_pdu.get_vb(nextVb, i);
                      string cur_oid = nextVb.get_printable_oid();
                      string cur_value =nextVb.get_printable_value();
                      dainfo.bType = false;
                      dainfo.fValue = atof(cur_value.c_str());
                      if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.1.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[0] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.2.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[1] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.3.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[2] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.4.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[3] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.5.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[4] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.6.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[5] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.7.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[6] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.8.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[7] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.9.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[8] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.10.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[9] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.11.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[10] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.12.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[11] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.13.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[12] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.14.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[13] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.15.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[14] = dainfo;
                      }
                      else if(cur_oid==".1.3.6.1.4.1.17409.100.3.2.1.16.0")
                      {
                          dainfo.fValue = dainfo.fValue*0.01;
                          data_ptr->mValues[15] = dainfo;
                      }
                  }
                  return 0;
              }
              return -1;
          }
}
