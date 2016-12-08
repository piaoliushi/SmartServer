#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <set>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> ws_server;

using websocketpp::connection_hdl;


class websocket_server
{

public:
    websocket_server();

    void on_open(connection_hdl hdl);

    void on_close(connection_hdl hdl);

    void on_message(connection_hdl hdl, ws_server::message_ptr msg);

    void run(uint16_t port);

    void stop();
private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl> > con_list;

    ws_server m_server;
    con_list m_connections;
};

#endif // WEBSOCKET_SERVER_H
