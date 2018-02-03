#include "websocket_server.h"
#include <boost/property_tree/json_parser.hpp>
#include "../../protocol/protocol.pb.h"
#include "../share_ptr_object_define.h"
#include "../../MsgDefine.h"
#include "../SvcMgr.h"
#include "../../database/DataBaseOperation.h"
#include "../config.h"
#include "../../LocalConfig.h"

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using namespace hx_net;
using namespace db;
websocket_server::websocket_server()
{
    m_server.init_asio();

    m_server.set_open_handler(bind(&websocket_server::on_open,this,::_1));
    m_server.set_close_handler(bind(&websocket_server::on_close,this,::_1));
    m_server.set_message_handler(bind(&websocket_server::on_message,this,::_1,::_2));

    m_server.get_alog().clear_channels(websocketpp::log::alevel::frame_header |
            websocketpp::log::alevel::frame_payload |
            websocketpp::log::alevel::control);
}


void websocket_server::on_open(connection_hdl hdl) {

    //boost::recursive_mutex::scoped_lock conlock(m_connection_lock);
    //boost::lock_guard<boost::mutex> guard(m_connection_lock);
    //m_connections.push_back(hdl);

}

void websocket_server::on_close(connection_hdl hdl) {

    _unregister_user(hdl);

}

void websocket_server::on_message(connection_hdl hdl, ws_server::message_ptr msg) {

    webSocketMsgPtr curWebPtr(new WebSocketMessage);
    curWebPtr->ParseFromArray(msg->get_payload().c_str(),msg->get_payload().size());
    int msgType = curWebPtr->nmsgtype();

    switch(msgType)
    {
        case MSG_LOGIN_REQ:{
            loginAckMsgPtr sloginAck(new LoginAck);
            string sUsr = curWebPtr->msgloginreq().susrname();
            string sPsw = curWebPtr->msgloginreq().susrpsw();
            bool bLoginRlt = _user_login(sUsr,sPsw,*sloginAck);
            curWebPtr->Clear();
            curWebPtr->set_smsgtype("login_ack");
            curWebPtr->set_nmsgtype(MSG_LOGIN_ACK);
            curWebPtr->mutable_msgloginack()->CopyFrom(*sloginAck);
            boost::recursive_mutex::scoped_lock conlock(m_connection_lock);
            m_server.send(hdl,curWebPtr->SerializeAsString(),websocketpp::frame::opcode::binary);
            ws_server::connection_ptr connection = m_server.get_con_from_hdl(hdl);
            if(bLoginRlt == true){
                tcp::endpoint client_endpoint = connection->get_raw_socket().remote_endpoint();
                GetInst(SvcMgr).get_notify()->OnClientOnline(client_endpoint.address().to_string(),client_endpoint.port());
                m_list_endpoint[connection] = client_endpoint;
                if(_register_user(sUsr,hdl) == true)
                    GetInst(SvcMgr).get_notify()->OnClientLogin(client_endpoint.address().to_string(),
                                                                client_endpoint.port(),sUsr);
            }else{
                websocketpp::lib::error_code ec;
                m_server.close(hdl, websocketpp::close::status::going_away, "", ec);
                if (ec) {
                    std::cout << "> Error closing connection " << ": "
                              << ec.message() << std::endl;
                }

            }

        }
            break;
        case MSG_DEV_TURNON_OPR:
        case MSG_DEV_TURNOFF_OPR:{
                devCommdMsgPtr commandmsg_(new DeviceCommandMsg);
                commandmsg_->CopyFrom(curWebPtr->msgcommandreq());
                string sUsr = commandmsg_->soperuser();
                string sDevId = commandmsg_->sdevid();

                e_ErrorCode nReult = GetInst(SvcMgr).excute_command(sDevId,msgType,sUsr,commandmsg_);

            }
            break;
        case MSG_TRANSMITTER_TURNON_OPR://发射机开启(默认高功率开机)
        case MSG_TRANSMITTER_MIDDLE_POWER_TURNON_OPR://中功率开机
        case MSG_TRANSMITTER_LOW_POWER_TURNON_OPR://低功率开机
        case MSG_TRANSMITTER_TURNOFF_OPR://发射机关闭
        case MSG_ANTENNA_HTOB_OPR:
        case MSG_ANTENNA_BTOH_OPR:
        case MSG_TRANSMITTER_RISE_POWER_OPR://发射机升功率
        case MSG_TRANSMITTER_REDUCE_POWER_OPR://发射机降功率
        case MSG_SET_FREQUENCY_OPR:
        case MSG_SEARCH_FREQUENCY_OPR:
        case MSG_0401_SWITCH_OPR:
        case MSG_CONTROL_MOD_SWITCH_OPR:
        case MSG_ADJUST_TIME_SET_OPR:
        case MSG_GENERAL_COMMAND_OPR:
        case MSG_SWITCH_AUDIO_CHANNEL_OPR://切换音频通道
        case MSG_DEV_RESET_OPR:{

            string sUsr = curWebPtr->msgcommandreq().soperuser();
            string sDevId = curWebPtr->msgcommandreq().sdevid();
            map<int,string> mapParam;//保存参数
            for(int i=0;i<curWebPtr->msgcommandreq().cparams().size();++i)
                mapParam[i] = curWebPtr->msgcommandreq().cparams(0).sparamvalue();
            e_ErrorCode nReult = GetInst(SvcMgr).start_exec_task(sDevId,sUsr,msgType,mapParam);

            devCommdRsltPtr ackMsg(new DeviceCommandResultNotify);
            ackMsg->set_sstationid(curWebPtr->msgcommandreq().sstationid());
            ackMsg->set_sdevid(curWebPtr->msgcommandreq().sdevid());
            ackMsg->set_sdevname(curWebPtr->msgcommandreq().sdevname());
            ackMsg->set_edevtype(curWebPtr->msgcommandreq().edevtype());
            ackMsg->set_eerrorid(nReult);
            ackMsg->set_soperuser(sUsr);

            curWebPtr->Clear();
            curWebPtr->set_smsgtype("command_reached_notify");
            if(msgType>=MSG_TRANSMITTER_TURNON_OPR && msgType<=MSG_TRANSMITTER_LOW_POWER_TURNON_OPR)
                curWebPtr->set_nmsgtype(MSG_TRANSMITTER_TURNON_ACK);
            else
                curWebPtr->set_nmsgtype(msgType+1);//ack紧随opr
            curWebPtr->mutable_commandresultnty()->CopyFrom(*ackMsg);
            boost::recursive_mutex::scoped_lock conlock(m_connection_lock);
            m_server.send(hdl,curWebPtr->SerializeAsString(),websocketpp::frame::opcode::binary);
        }
    }

}

void websocket_server::send_message(googleMsgPtr sMsg)
{
    boost::recursive_mutex::scoped_lock conlock(m_connection_lock);
    con_list::iterator it = m_connections.begin();
    for (;it!=m_connections.end();++it) {
        try
        {
            m_server.send((*it).second,sMsg->SerializeAsString(),websocketpp::frame::opcode::binary);
        }catch(...){
            cout<<"websocket send error"<<endl;
        }

    }
}

void websocket_server::run(uint16_t port) {
    m_server.listen(boost::asio::ip::tcp::v4(),port);
    m_server.start_accept();
    m_server.run();
}

void websocket_server::stop(){

    m_server.stop();
}

bool websocket_server::_register_user(string sUser,connection_hdl hdl)
{
    boost::recursive_mutex::scoped_lock conlock(m_connection_lock);
    con_list::iterator iter = m_connections.find(sUser);
    if(iter == m_connections.end()){
        m_connections.insert(pair<string,connection_hdl>(sUser,hdl));
        return true;
    }
    return false;
}

bool websocket_server::_is_register_user(string sUser)
{

    boost::recursive_mutex::scoped_lock conlock(m_connection_lock);
    con_list::iterator iter = m_connections.find(sUser);
    if(iter != m_connections.end()){
        return true;
    }
    return false;
}

void websocket_server::_unregister_user(connection_hdl hdl)
{
    boost::recursive_mutex::scoped_lock conlock(m_connection_lock);
    con_list::iterator iter=m_connections.begin();
    for(;iter!=m_connections.end();++iter){
        if((*iter).second.lock() == hdl.lock()){
            ws_server::connection_ptr connection = m_server.get_con_from_hdl(hdl);
            con_list_endpoint::iterator iterCon = m_list_endpoint.find(connection);
            if(iterCon!=m_list_endpoint.end()){
                tcp::endpoint client_endpoint = iterCon->second;
                GetInst(SvcMgr).get_notify()->OnClientOffline(client_endpoint.address().to_string(),
                                                              client_endpoint.port());
            }
            m_connections.erase(iter);
            return;
        }
    }
}

bool websocket_server::_user_login(string sUser,string sPassword,LoginAck &loginAck)
{
    if(_is_register_user(sUser)){
        loginAck.set_eresult(EC_USR_REPEAT_LOGIN);//已登陆
        return false;
    }
    UserInformation tmpUser;
    std::string sstationid = GetInst(LocalConfig).local_station_id();
    if(!GetInst(DataBaseOperation).GetUserInfo(sUser,tmpUser))
        loginAck.set_eresult(EC_FAILED);//获取用户信息失败（访问数据库出错）
    else
    {
        if(tmpUser.sName == "")
            loginAck.set_eresult(EC_USR_NOT_FOUND);//未找到此用户
        else if(sPassword != tmpUser.sPassword)
            loginAck.set_eresult(EC_USR_PSW_ERROR);//密码错误
        else
        {
            loginAck.set_eresult(EC_OK);
            loginAck.set_eusrlevel(tmpUser.nControlLevel);
            loginAck.set_usrnumber(tmpUser.sNumber);
            loginAck.set_usrname(tmpUser.sName.c_str());
            loginAck.set_usrpsw(tmpUser.sPassword.c_str());
            loginAck.set_usrjobnumber(tmpUser.sJobNumber);
            loginAck.set_usrheadship(tmpUser.sHeadship.c_str());
            //查找该用户授权设备，并插入到设备-〉用户映射表
            vector<string> debnumber;
            if(GetInst(DataBaseOperation).GetAllAuthorizeDevByUser(tmpUser.sNumber,debnumber))
            {
                vector<string>::iterator itersNum = debnumber.begin();
                for(;itersNum!=debnumber.end();++itersNum)
                {
                    //判断设备是否属于本地台站或者是上级台站直连下级台站设备
                    DevBaseInfo devBaseInfo;
                    GetInst(SvcMgr).dev_base_info(sstationid,devBaseInfo,*itersNum);

                    map<int,map<int,CurItemAlarmInfo> > curAlarm;
                    con_state  netState = GetInst(SvcMgr).get_dev_net_state(sstationid,*itersNum);
                    dev_run_state   runState = GetInst(SvcMgr).get_dev_run_state(sstationid,*itersNum);
                    GetInst(SvcMgr).get_dev_alarm_state(sstationid,*itersNum,curAlarm);

                    //con_state  netState = GetInst(SvcMgr).get_dev_net_state(devBaseInfo.sStationNumber,*itersNum);
                    //dev_run_state   runState = GetInst(SvcMgr).get_dev_run_state(devBaseInfo.sStationNumber,*itersNum);
                    //GetInst(SvcMgr).get_dev_alarm_state(devBaseInfo.sStationNumber,*itersNum,curAlarm);


                    //收集设备连接信息
                    DevNetStatus *dev_n_s = loginAck.add_cdevcurnetstatus();
                    dev_n_s->set_sstationid(devBaseInfo.sStationNumber);//sstationid
                    dev_n_s->set_sdevid(*itersNum);
                    dev_n_s->set_sdevname(devBaseInfo.sDevName.c_str());
                    dev_n_s->set_edevtype(devBaseInfo.nDevType);
                    dev_n_s->set_enetstatus((DevNetStatus_e_NetStatus)netState);
                    //收集设备运行信息
                    DevWorkStatus *dev_run_s = loginAck.add_cdevcurworkstatus();
                    dev_run_s->set_sstationid(devBaseInfo.sStationNumber);//sstationid
                    dev_run_s->set_sdevid(*itersNum);
                    dev_run_s->set_edevtype(devBaseInfo.nDevType);
                    dev_run_s->set_sdevname(devBaseInfo.sDevName.c_str());
                    dev_run_s->set_eworkstatus((DevWorkStatus_e_WorkStatus)runState);
                    //收集设备报警信息
                    map<int,map<int,CurItemAlarmInfo> >::iterator iter = curAlarm.begin();
                    if(iter!=curAlarm.end())
                    {
                        DevAlarmStatus *dev_alarm_s = loginAck.add_cdevcuralarmstatus();
                        dev_alarm_s->set_sstationid(devBaseInfo.sStationNumber);//sstationid
                        dev_alarm_s->set_sdevid(*itersNum);
                        dev_alarm_s->set_sdevname(devBaseInfo.sDevName.c_str());
                        dev_alarm_s->set_edevtype(devBaseInfo.nDevType);
                        dev_alarm_s->set_nalarmcount(curAlarm.size());
                        for(;iter!=curAlarm.end();++iter)
                        {
                            map<int,CurItemAlarmInfo>::iterator iter_a = iter->second.begin();
                            for(;iter_a!=iter->second.end();++iter_a){
                                if(iter_a->second.bNotifyed==false)
                                    continue;
                                DevAlarmStatus_eCellAlarmMsg *pCellAlarm = dev_alarm_s->add_ccellalarm();
                                //std::string scellid = str(boost::format("%1%")%iter->first);
                                pCellAlarm->set_scellid(iter->first);
                                pCellAlarm->set_sdesp(iter_a->second.sReason);
                                char str_time[64];
                                tm *local_time = localtime(&iter_a->second.startTime);
                                strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", local_time);
                                pCellAlarm->set_sstarttime(str_time);
                                pCellAlarm->set_ccellstatus(e_AlarmStatus(iter_a->first));
                            }
                        }
                    }
                }
            }

            return true;
        }
    }

    return false;
}
