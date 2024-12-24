#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <mutex>
#include "order_management.h"

class WebSocketServer
{
public:
    WebSocketServer();
    void start(uint16_t port);
    void set_order_manager(OrderManagement *manager);

private:
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg);

    websocketpp::server<websocketpp::config::asio> m_server;
    std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> m_connections;
    std::mutex m_connections_mutex;
    OrderManagement *order_manager;
};

#endif // WEBSOCKET_SERVER_H