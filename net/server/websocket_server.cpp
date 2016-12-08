#include "websocket_server.h"
#include <boost/property_tree/json_parser.hpp>
#include "../../protocol/protocol.pb.h"
#include "../share_ptr_object_define.h"


using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using namespace hx_net;
websocket_server::websocket_server()
{
    m_server.init_asio();

    m_server.set_open_handler(bind(&websocket_server::on_open,this,::_1));
    m_server.set_close_handler(bind(&websocket_server::on_close,this,::_1));
    m_server.set_message_handler(bind(&websocket_server::on_message,this,::_1,::_2));
}


void websocket_server::on_open(connection_hdl hdl) {

    boost::recursive_mutex::scoped_lock conlock(m_connection_lock);
    m_connections.push_back(hdl);
}

void websocket_server::on_close(connection_hdl hdl) {

   boost::recursive_mutex::scoped_lock conlock(m_connection_lock);
   std::vector<connection_hdl>::iterator iter = m_connections.begin();
   for(;iter!=m_connections.end();++iter){
       if((*iter).lock() == hdl.lock()){
           m_connections.erase(iter);
           return;
       }
   }

}

void websocket_server::on_message(connection_hdl hdl, ws_server::message_ptr msg) {

    boost::recursive_mutex::scoped_lock conlock(m_connection_lock);
    con_list::iterator it = m_connections.begin();
    //ws_server::message_ptr msg(new ws_server:(s,websocketpp::frame::opcode::TEXT,500));
//    LoginReq rlogin;
//    rlogin.ParseFromArray(msg->get_payload().c_str(),msg->get_payload().size());
//    std::string sUsr = rlogin.susrname();
//    std::string sPsw = rlogin.susrpsw();

    loginAckMsgPtr sloginAck(new LoginAck);
    sloginAck->set_eresult(EC_OK);
    sloginAck->set_usrname("test1");
    //unsigned msg_size = sloginAck->ByteSize();

    //xxx.resize(msg_size);
    //sloginAck->SerializeToArray(&xxx[0],msg_size);
    //DeviceCommandMsg commandmsg;
    //commandmsg.ParseFromArray(msg->get_payload().c_str(),msg->get_payload().size());
    //std::string sUsr = commandmsg.cparams(1).sparamname();
    for (;it!=m_connections.end();++it) {//msg->get_payload().size(),
        m_server.send(*it,sloginAck->SerializeAsString(),websocketpp::frame::opcode::binary);
    }
}

void websocket_server::send_message(googleMsgPtr sMsg)
{
    boost::recursive_mutex::scoped_lock conlock(m_connection_lock);
    con_list::iterator it = m_connections.begin();
    for (;it!=m_connections.end();++it) {
        m_server.send(*it,sMsg->SerializeAsString(),websocketpp::frame::opcode::binary);
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
