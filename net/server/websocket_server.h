#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <set>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "../share_ptr_object_define.h"

typedef websocketpp::server<websocketpp::config::asio> ws_server;

using websocketpp::connection_hdl;
using boost::asio::ip::tcp;
using namespace  hx_net;

class websocket_server
{

public:
    websocket_server();

    void on_open(connection_hdl hdl);

    void on_close(connection_hdl hdl);

    void on_message(connection_hdl hdl, ws_server::message_ptr msg);

    void run(uint16_t port);

    void stop();

    void send_message(googleMsgPtr sMsg);

protected:
    bool _user_login(string sUser,string sPassword,LoginAck &loginAck);
    bool _register_user(string sUser,connection_hdl hdl);
    bool _is_register_user(string sUser);
    void _unregister_user(connection_hdl hdl);
private:
    //typedef std::set<connection_hdl,std::owner_less<connection_hdl> > con_list;
    //typedef std::vector<connection_hdl> con_list;
    typedef std::map<string,connection_hdl> con_list;//用户登陆列表
    typedef std::map<ws_server::connection_ptr,tcp::endpoint> con_list_endpoint;//连接标识列表
    ws_server m_server;
    con_list m_connections;
    con_list_endpoint m_list_endpoint;
    boost::recursive_mutex  m_connection_lock;//recursive_mutex
};

#endif // WEBSOCKET_SERVER_H
