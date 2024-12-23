#include "order_management.h"
#include "websocket_server.h"
#include "utils.h"
#include <thread>
#include <iostream>
#include <cassert>

// Function to run the WebSocket server in a separate thread
void run_websocket_server(WebSocketServer &server, uint16_t port)
{
    server.start(port);
}

int main()
{
    // Step 1: Instantiate OrderManagement class, which will handle authentication internally
    std::cout << "Initializing OrderManagement and Authenticating..." << std::endl;
    OrderManagement order_manager; // This will handle loading config and authentication

    // Step 2: Test Order Placement (Now with order_id)
    std::cout << "\nTesting Order Placement..." << std::endl;
    order_manager.place_order("ETH-PERPETUAL", 45.0, 1);

    // Step 3: Test Order Cancellation
    std::cout << "\nTesting Order Cancellation..." << std::endl;
    auto orders = order_manager.get_all_orders();
    if (!orders.empty())
    {
        order_manager.cancel_order(orders[0].order_id);
    }
    else
    {
        std::cerr << "No orders placed to cancel." << std::endl;
    }

    // Step 4: View Current Orders
    std::cout << "\nCurrent Orders:" << std::endl;
    for (const auto &order : order_manager.get_all_orders())
    {
        std::cout << "Order ID: " << order.order_id
                  << ", Symbol: " << order.symbol
                  << ", Price: " << order.price
                  << ", Quantity: " << order.quantity
                  << ", Status: " << order.status << std::endl;
    }

    // Step 5: Starting WebSocket server (simulate WebSocket functionality)
    WebSocketServer server;
    server.set_order_manager(&order_manager); // Set the order manager here before starting the server.

    std::cout << "\nStarting WebSocket server on port 9002..." << std::endl;
    std::thread server_thread(run_websocket_server, std::ref(server), 9002);

    std::cout << "\nWebSocket server running. Use a WebSocket client to connect to ws://localhost:9002" << std::endl;
    std::cout << "Send 'place_order' to place an order.\n";
    std::cout << "Send 'cancel_order' to cancel an order.\n";

    // Simulate server interactions by keeping the main thread alive
    std::cout << "\nPress Enter to stop the server...\n";
    std::cin.get();

    // Terminate server
    server_thread.join();

    return 0;
}