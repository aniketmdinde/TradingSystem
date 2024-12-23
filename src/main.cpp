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
    // Simulate loading configuration for authentication (client_id and client_secret)
    auto [client_id, client_secret] = load_config();
    std::cout << "Loaded client_id: " << client_id << std::endl;

    // Step 1: Test Authentication
    std::cout << "\nTesting Deribit Authentication..." << std::endl;
    std::string auth_token = authenticate_with_deribit(client_id, client_secret);

    if (auth_token.empty())
    {
        std::cerr << "Authentication failed!" << std::endl;
        return -1; // Exit if authentication fails
    }
    std::cout << "Authentication successful. Token: " << auth_token << std::endl;

    // Step 2: Test Order Placement (Now with order_id)
    std::cout << "\nTesting Order Placement..." << std::endl;
    std::string order_id = place_order_on_deribit(auth_token, "ETH-PERPETUAL", 45.0, 1);
    assert(!order_id.empty() && "Order placement failed.");

    std::cout << "Order placed with ID: " << order_id << std::endl;

    // Step 3: Test Order Cancellation
    std::cout << "\nTesting Order Cancellation..." << std::endl;
    bool cancel_order_success = cancel_order_on_deribit(auth_token, order_id);
    assert(cancel_order_success && "Order cancellation failed.");

    // Test invalid cancellation
    std::cout << "\nTesting Invalid Order Cancellation..." << std::endl;
    bool cancel_invalid_order_success = cancel_order_on_deribit(auth_token, "invalid_order_id");
    assert(!cancel_invalid_order_success && "Expected cancellation to fail.");

    // Step 4: Create and test OrderManagement class
    std::cout << "\nTesting Order Management..." << std::endl;
    OrderManagement order_manager;

    // Test placing an order via OrderManagement (order_id returned by place_order_on_deribit)
    order_manager.place_order("ETH-PERPETUAL", 45.0, 1);
    order_manager.place_order("ETH-PERPETUAL", 32.0, 5);

    // Test tracking orders using order_id returned from place_order
    std::cout << "Tracking orders by order_id:\n";
    for (const auto &order : order_manager.get_all_orders())
    {
        order_manager.track_order(order.order_id);
    }

    // Test canceling orders
    std::cout << "Canceling orders:\n";
    for (const auto &order : order_manager.get_all_orders())
    {
        if (order.status != "Canceled") // Only cancel pending orders
        {
            order_manager.cancel_order(order.order_id);
        }
    }

    // Test listing all orders
    std::cout << "\nAll Orders after cancellation:" << std::endl;
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