#include "websocket_server.h"
#include <iostream>
#include <set>
#include <mutex>
#include <algorithm>

WebSocketServer::WebSocketServer() : order_manager(nullptr)
{
    m_server.init_asio();
    m_server.set_open_handler(std::bind(&WebSocketServer::on_open, this, std::placeholders::_1));
    m_server.set_close_handler(std::bind(&WebSocketServer::on_close, this, std::placeholders::_1));
    m_server.set_message_handler(std::bind(&WebSocketServer::on_message, this, std::placeholders::_1, std::placeholders::_2));
}

void WebSocketServer::start(uint16_t port)
{
    try
    {
        m_server.listen(port);
        m_server.start_accept();
        m_server.run();
    }
    catch (const websocketpp::exception &e)
    {
        std::cerr << "WebSocket++ error: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
}

void WebSocketServer::set_order_manager(OrderManagement *manager)
{
    order_manager = manager;
}

void WebSocketServer::on_open(websocketpp::connection_hdl hdl)
{
    std::lock_guard<std::mutex> lock(m_connections_mutex);
    m_connections.insert(hdl);
    std::cout << "Client connected." << std::endl;
}

void WebSocketServer::on_close(websocketpp::connection_hdl hdl)
{
    std::lock_guard<std::mutex> lock(m_connections_mutex);
    m_connections.erase(hdl);
    std::cout << "Client disconnected." << std::endl;
}

void WebSocketServer::on_message(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg)
{
    std::string payload = msg->get_payload();

    payload.erase(0, payload.find_first_not_of(" \t\r\n"));
    payload.erase(payload.find_last_not_of(" \t\r\n") + 1);

    std::cout << "Received message: '" << payload << "'" << std::endl;

    if (payload.empty())
    {
        std::cerr << "Received empty message." << std::endl;
        return;
    }

    if (order_manager)
    {
        if (payload == "place_order")
        {
            std::cout << "Placing order..." << std::endl;
            order_manager->place_order("BTCUSD", 45000.0, 2);
        }
        else if (payload == "cancel_order")
        {
            std::cout << "Canceling order..." << std::endl;
            order_manager->cancel_order("order1");
        }
        else
        {
            std::cerr << "Unknown message: " << payload << std::endl;
        }
    }
    else
    {
        std::cerr << "Order manager not set." << std::endl;
    }
}