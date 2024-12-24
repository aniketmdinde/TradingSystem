#include "websocket_server.h"
#include "utils.h"
#include <iostream>
#include <set>
#include <mutex>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

    // Send the welcome message
    std::string welcome_msg = "Welcome to the Trading System WebSocket Server!\n";
    welcome_msg += "Here are the available commands and their syntax:\n";

    // Syntax for each command
    welcome_msg += "1. place_order <symbol> <price> <quantity>\n";
    welcome_msg += "2. modify_order <order_id> <new_price> <new_quantity>\n";
    welcome_msg += "3. cancel_order <order_id>\n";
    welcome_msg += "4. view_orders\n";
    welcome_msg += "5. track_order <order_id>\n";
    welcome_msg += "6. view_positions <symbol> <instrument_type>\n";
    welcome_msg += "7. view_order_book <symbol> <depth>\n";

    m_server.send(hdl, welcome_msg, websocketpp::frame::opcode::text);
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
        try
        {
            std::string response_msg;

            std::istringstream stream(payload);
            std::string command;
            stream >> command;

            if (command == "place_order")
            {
                std::string symbol = "ETH-PERPETUAL";
                double price = 400.0;
                int quantity = 1;
                stream >> symbol >> price >> quantity;

                std::cout << "Placing order: " << symbol << " at price " << price << " for quantity " << quantity << std::endl;

                bool success = order_manager->place_order(symbol, price, quantity);
                if (success)
                {
                    response_msg = "Your order for " + std::to_string(quantity) + " " + symbol + " at price " + std::to_string(price) + " has been placed successfully.\n";
                }
                else
                {
                    response_msg = "Error: Failed to place order. Please try again.\n";
                }
            }
            else if (command == "cancel_order")
            {
                std::string order_id;
                stream >> order_id;

                if (!order_id.empty())
                {
                    std::cout << "Canceling order with ID: " << order_id << std::endl;

                    bool success = order_manager->cancel_order(order_id);
                    if (success)
                    {
                        response_msg = "Your order with ID " + order_id + " has been canceled successfully.\n";
                    }
                    else
                    {
                        response_msg = "Error: Failed to cancel order. Please check the order ID and try again.\n";
                    }
                }
                else
                {
                    response_msg = "Error: Order ID is required to cancel an order.\n";
                }
            }
            else if (command == "modify_order")
            {
                std::string order_id;
                double new_price = 450.0;
                int new_quantity = 2;
                stream >> order_id >> new_price >> new_quantity;

                if (!order_id.empty())
                {
                    std::cout << "Modifying order " << order_id << " to price " << new_price << " and quantity " << new_quantity << std::endl;

                    bool success = order_manager->modify_order(order_id, new_price, new_quantity);
                    if (success)
                    {
                        response_msg = "Your order with ID " + order_id + " has been updated to price " + std::to_string(new_price) + " and quantity " + std::to_string(new_quantity) + ".\n";
                    }
                    else
                    {
                        response_msg = "Error: Failed to modify order. Please check the order ID and try again.\n";
                    }
                }
                else
                {
                    response_msg = "Error: Order ID is required to modify an order.\n";
                }
            }
            else if (command == "track_order")
            {
                std::string order_id;
                stream >> order_id;

                if (!order_id.empty())
                {
                    std::cout << "Tracking order with ID: " << order_id << std::endl;

                    bool success = order_manager->track_order(order_id);
                    if (success)
                    {
                        response_msg = "Your order with ID " + order_id + " is being tracked successfully.\n";
                    }
                    else
                    {
                        response_msg = "Error: Failed to track order. Please check the order ID and try again.\n";
                    }
                }
                else
                {
                    response_msg = "Error: Order ID is required to track an order.\n";
                }
            }
            else if (command == "view_orders")
            {
                auto orders = order_manager->get_all_orders();
                if (orders.empty())
                {
                    response_msg = "You have no orders at the moment.\n";
                }
                else
                {
                    response_msg = "Here are your current orders:\n";
                    for (const auto &order : orders)
                    {
                        response_msg += "Order ID: " + order.order_id + ", Symbol: " + order.symbol +
                                        ", Price: " + std::to_string(order.price) +
                                        ", Quantity: " + std::to_string(order.quantity) + ", Status: " + order.status + "\n";
                    }
                }
            }
            else if (command == "view_order_book")
            {
                std::string symbol = "ETH-PERPETUAL";
                int depth = 5;
                stream >> symbol >> depth;

                if (!symbol.empty() && depth > 0)
                {
                    std::cout << "Fetching order book for symbol: " << symbol << " with depth: " << depth << std::endl;

                    // Retrieve the auth token from the order manager
                    std::string auth_token = order_manager->get_auth_token();
                    if (auth_token.empty())
                    {
                        response_msg = "Error: Authentication token is missing.";
                    }
                    else
                    {
                        // Fetch order book from Deribit using the utility function
                        json order_book = get_order_book_from_deribit(auth_token, symbol, depth);

                        if (order_book.is_null())
                        {
                            response_msg = "Error: Order book is empty or could not be fetched.";
                        }
                        else
                        {
                            response_msg = "Order book for " + symbol + " with depth " + std::to_string(depth) + ":\n";

                            // Add bids to the response
                            if (order_book.contains("bids") && order_book["bids"].is_array())
                            {
                                response_msg += "Bids:\n";
                                for (const auto &bid : order_book["bids"])
                                {
                                    response_msg += "  Price: " + std::to_string(bid[0].get<double>()) +
                                                    ", Quantity: " + std::to_string(bid[1].get<double>()) + "\n";
                                }
                            }
                            else
                            {
                                response_msg += "  No bids available.\n";
                            }

                            // Add asks to the response
                            if (order_book.contains("asks") && order_book["asks"].is_array())
                            {
                                response_msg += "Asks:\n";
                                for (const auto &ask : order_book["asks"])
                                {
                                    response_msg += "  Price: " + std::to_string(ask[0].get<double>()) +
                                                    ", Quantity: " + std::to_string(ask[1].get<double>()) + "\n";
                                }
                            }
                            else
                            {
                                response_msg += "  No asks available.\n";
                            }
                        }
                    }
                }
                else
                {
                    response_msg = "Error: Symbol and depth are required to view the order book.\n";
                }
            }
            else if (command == "view_positions")
            {
                std::string symbol = "ETH-PERPETUAL";
                std::string instrument_type = "future";
                stream >> symbol >> instrument_type;

                if (!symbol.empty() && !instrument_type.empty())
                {
                    std::cout << "Fetching positions for symbol: " << symbol << " and instrument type: " << instrument_type << std::endl;

                    // Retrieve the auth token from the order manager
                    std::string auth_token = order_manager->get_auth_token();
                    if (auth_token.empty())
                    {
                        response_msg = "Error: Authentication token is missing.";
                    }
                    else
                    {
                        // Fetch positions from Deribit using the utility function
                        auto positions = view_positions_from_deribit(auth_token, symbol, instrument_type);

                        if (positions.empty())
                        {
                            response_msg = "Error: No positions found or could not be fetched.\n";
                        }
                        else
                        {
                            response_msg = "Positions for " + symbol + " with instrument type " + instrument_type + ":\n";

                            for (const auto &position : positions)
                            {
                                response_msg += "Position ID: " + position.instrument_name + ", Quantity: " + std::to_string(position.size) +
                                                ", Average Price: " + std::to_string(position.average_price) + ", Floating P&L: " +
                                                std::to_string(position.floating_profit_loss) + "\n";
                            }
                        }
                    }
                }
                else
                {
                    response_msg = "Error: Symbol and instrument type are required to view positions.\n";
                }
            }
            else
            {
                response_msg = "Error: Unknown command. Please check the command and try again.\n";
            }

            m_server.send(hdl, response_msg, websocketpp::frame::opcode::text);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error processing command: " << e.what() << std::endl;
            std::string error_response = "Error: There was an issue processing your request. Please try again.";
            m_server.send(hdl, error_response, websocketpp::frame::opcode::text);
        }
    }
    else
    {
        std::cerr << "Order manager not set." << std::endl;
        std::string error_response = "Error: The system is not ready. Please try again later.";
        m_server.send(hdl, error_response, websocketpp::frame::opcode::text);
    }
}