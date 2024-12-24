#include "order_management.h"
#include "websocket_server.h"
#include "utils.h"
#include <thread>
#include <iostream>
#include <cassert>
#include <chrono>

void run_websocket_server(WebSocketServer &server, uint16_t port)
{
    server.start(port);
}

int main()
{
    OrderManagement order_manager;

    WebSocketServer server;
    server.set_order_manager(&order_manager);

    std::cout << "\nStarting WebSocket server on port 9002..." << std::endl;
    std::thread server_thread(run_websocket_server, std::ref(server), 9002);

    std::cout << "\nWebSocket server running. Use a WebSocket client to connect to ws://localhost:9002" << std::endl;
    std::cout << "Send 'place_order' to place an order.\n";
    std::cout << "Send 'cancel_order' to cancel an order.\n";
    std::cout << "Send 'modify_order' to modify an order.\n";
    std::cout << "Send 'view_orders' to view all orders.\n";
    std::cout << "Send 'view_order_book' to view the order book.\n";
    std::cout << "Send 'view_positions' to view current positions.\n";
    std::cout << "\nPress Enter to stop the server...\n";

    std::cin.get();

    server_thread.join();

    return 0;
}