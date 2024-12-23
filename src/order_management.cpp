#include "order_management.h"
#include "utils.h"
#include <iostream>
#include <algorithm>

OrderManagement::OrderManagement(const std::string &auth_token)
{
    this->auth_token = auth_token;

    // Optionally, you can still load client_id and client_secret if needed for other operations
    auto [client_id, client_secret] = load_config();
    this->client_id = client_id;
    this->client_secret = client_secret;

    // No need to authenticate if the token is passed
    if (auth_token.empty())
    {
        std::cerr << "Auth token is empty!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

OrderManagement::OrderManagement()
{
    // Load client_id and client_secret for authentication or API usage
    auto [client_id, client_secret] = load_config();
    this->client_id = client_id;
    this->client_secret = client_secret;

    // Authenticate to fetch the API token
    authenticate();
}

void OrderManagement::authenticate()
{
    // Fetch API token and set auth_token
    auth_token = authenticate_with_deribit(client_id, client_secret);
    if (auth_token.empty())
    {
        std::cerr << "Authentication failed! Check your credentials." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void OrderManagement::place_order(const std::string &symbol, double price, int quantity)
{
    // Attempt to place the order via Deribit API and capture the order ID
    std::string order_id = place_order_on_deribit(auth_token, symbol, price, quantity);
    if (!order_id.empty())
    {
        // Add the new order to the list if API call succeeds
        Order new_order = {order_id, symbol, price, quantity, "Pending"};
        orders.push_back(new_order);
        std::cout << "Placed order: " << order_id << " for " << quantity << " " << symbol
                  << " at " << price << std::endl;
    }
    else
    {
        std::cerr << "Failed to place order for " << symbol << "!" << std::endl;
    }
}

void OrderManagement::cancel_order(const std::string &order_id)
{
    // Find the order locally by order_id
    auto it = std::find_if(orders.begin(), orders.end(), [&order_id](const Order &order)
                           { return order.order_id == order_id; });

    if (it != orders.end())
    {
        // Attempt to cancel the order via Deribit API
        bool success = cancel_order_on_deribit(auth_token, it->order_id);
        if (success)
        {
            it->status = "Canceled";
            std::cout << "Canceled order: " << order_id << std::endl;
        }
        else
        {
            std::cerr << "Failed to cancel order: " << order_id << std::endl;
        }
    }
    else
    {
        std::cerr << "Order with ID " << order_id << " not found." << std::endl;
    }
}

void OrderManagement::modify_order(const std::string &order_id, double new_price, int new_quantity)
{
    // Find the order locally by order_id
    auto it = std::find_if(orders.begin(), orders.end(), [&order_id](const Order &order)
                           { return order.order_id == order_id; });

    if (it != orders.end())
    {
        // Log the attempt to modify the order
        std::cout << "Attempting to modify order with ID: " << order_id
                  << " to new price: " << new_price << " and new quantity: " << new_quantity << std::endl;

        // Attempt to modify the order via Deribit API
        bool success = modify_order_on_deribit(auth_token, it->order_id, new_price, new_quantity);

        if (success)
        {
            // Update the local order with the new price and quantity
            it->price = new_price;
            it->quantity = new_quantity;
            std::cout << "Successfully modified order: " << order_id << " to new price: "
                      << new_price << " and new quantity: " << new_quantity << std::endl;
        }
        else
        {
            std::cerr << "Failed to modify order: " << order_id << std::endl;
        }
    }
    else
    {
        std::cerr << "Order with ID " << order_id << " not found." << std::endl;
    }
}

void OrderManagement::track_order(const std::string &order_id)
{
    // Check order status (placeholder logic for now)
    auto it = std::find_if(orders.begin(), orders.end(), [&order_id](const Order &order)
                           { return order.order_id == order_id; });

    if (it != orders.end())
    {
        std::cout << "Order " << order_id << " is " << it->status << std::endl;
    }
    else
    {
        std::cerr << "Order with ID " << order_id << " not found." << std::endl;
    }
}

std::vector<Order> OrderManagement::get_all_orders()
{
    return orders;
}

std::vector<Order> OrderManagement::get_open_orders()
{
    std::vector<Order> open_orders;
    for (const auto &order : orders)
    {
        if (order.status == "Pending" || order.status == "Active")
        {
            open_orders.push_back(order);
        }
    }
    return open_orders;
}

void OrderManagement::view_positions(const std::string &currency, const std::string &kind)
{
    std::cout << "Fetching current positions from Deribit..." << std::endl;

    std::vector<Position> positions = view_positions_from_deribit(auth_token, currency, kind);

    // Check if positions were fetched successfully
    if (positions.empty())
    {
        std::cout << "No positions found or error fetching data." << std::endl;
    }
    else
    {
        std::cout << "Current Positions:" << std::endl;
        // Print each position
        for (const auto &pos : positions)
        {
            std::cout << "Instrument: " << pos.instrument_name
                      << ", Direction: " << pos.direction
                      << ", Average Price: " << pos.average_price
                      << ", Delta: " << pos.delta
                      << ", Estimated Liquidation Price: " << pos.estimated_liquidation_price
                      << ", Floating Profit/Loss: " << pos.floating_profit_loss
                      << ", Index Price: " << pos.index_price
                      << ", Initial Margin: " << pos.initial_margin
                      << ", Leverage: " << pos.leverage
                      << ", Maintenance Margin: " << pos.maintenance_margin
                      << ", Mark Price: " << pos.mark_price
                      << ", Open Orders Margin: " << pos.open_orders_margin
                      << ", Realized Funding: " << pos.realized_funding
                      << ", Realized Profit/Loss: " << pos.realized_profit_loss
                      << ", Settlement Price: " << pos.settlement_price
                      << ", Size: " << pos.size
                      << ", Size Currency: " << pos.size_currency
                      << ", Total Profit/Loss: " << pos.total_profit_loss
                      << ", Kind: " << pos.kind << std::endl;
        }
    }
}

// Function to get the order book for a specific instrument
void OrderManagement::get_order_book(const std::string &instrument_name, int depth)
{
    if (auth_token.empty())
    {
        std::cerr << "Auth token is missing. Please authenticate first." << std::endl;
        return;
    }

    // Call the get_order_book_from_deribit function
    auto order_book = get_order_book_from_deribit(auth_token, instrument_name, depth);
    if (order_book.is_null())
    {
        std::cerr << "Failed to retrieve order book for " << instrument_name << std::endl;
        return;
    }

    std::cout << "Order book for " << instrument_name << " (depth " << depth << "):\n";

    // Printing the order book contents
    std::cout << "Asks:\n";
    for (const auto &ask : order_book["asks"])
    {
        std::cout << "Price: " << ask[0] << ", Quantity: " << ask[1] << std::endl;
    }

    std::cout << "Bids:\n";
    for (const auto &bid : order_book["bids"])
    {
        std::cout << "Price: " << bid[0] << ", Quantity: " << bid[1] << std::endl;
    }
}