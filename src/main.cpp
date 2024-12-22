#include "order_management.h"
#include "websocket_server.h"
#include <thread>
#include <iostream>

// Function to run the WebSocket server in a separate thread
void run_websocket_server(WebSocketServer &server, uint16_t port)
{
    server.start(port);
}

int main()
{
    // Create an instance of OrderManagement
    OrderManagement order_manager;

    // Create and configure WebSocket server
    WebSocketServer server;
    server.set_order_manager(&order_manager); // Set the order manager here before starting the server.

    // Test OrderManagement functionalities
    std::cout << "Testing Order Management:" << std::endl;
    order_manager.place_order("AAPL", 150.5, 10);
    order_manager.place_order("GOOGL", 2800.0, 5);
    order_manager.track_order("order1");
    order_manager.cancel_order("order1");
    order_manager.track_order("order1");

    std::cout << "\nAll Orders:" << std::endl;
    for (const auto &order : order_manager.get_all_orders())
    {
        std::cout << "Order ID: " << order.order_id
                  << ", Symbol: " << order.symbol
                  << ", Price: " << order.price
                  << ", Quantity: " << order.quantity
                  << ", Status: " << order.status << std::endl;
    }

    std::cout << "\nStarting WebSocket server on port 9002..." << std::endl;

    // Run WebSocket server in a separate thread
    std::thread server_thread(run_websocket_server, std::ref(server), 9002);

    std::cout << "\nWebSocket server running. Use a WebSocket client to connect to ws://localhost:9002" << std::endl;
    std::cout << "Send 'place_order' to place an order.\n";
    std::cout << "Send 'cancel_order' to cancel an order.\n";

    // Simulate server interactions by keeping the main thread alive
    std::cout << "\nPress Enter to stop the server...\n";
    std::cin.get();

    // Terminate server (optional handling can be implemented here)
    server_thread.join();

    return 0;
}