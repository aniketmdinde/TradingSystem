#include "order_management.h"
#include "utils.h"
#include <iostream>
#include <algorithm>

OrderManagement::OrderManagement()
{
    // Load client_id and client_secret for authentication or API usage
    auto [client_id, client_secret] = load_config();
    this->client_id = client_id;
    this->client_secret = client_secret;
}

void OrderManagement::place_order(const std::string &symbol, double price, int quantity)
{
    // Generate a unique order ID (for simplicity, we'll use a simple counter here)
    std::string order_id = "order" + std::to_string(orders.size() + 1);

    // Create a new order
    Order new_order = {order_id, symbol, price, quantity, "Pending"};

    // Add the new order to the list
    orders.push_back(new_order);

    std::cout << "Placed order: " << order_id << " for " << quantity << " " << symbol
              << " at " << price << std::endl;
}

void OrderManagement::cancel_order(const std::string &order_id)
{
    // Find the order by ID
    auto it = std::find_if(orders.begin(), orders.end(), [&order_id](const Order &order)
                           { return order.order_id == order_id; });

    if (it != orders.end())
    {
        it->status = "Canceled";
        std::cout << "Canceled order: " << order_id << std::endl;
    }
    else
    {
        std::cerr << "Order with ID " << order_id << " not found." << std::endl;
    }
}

void OrderManagement::track_order(const std::string &order_id)
{
    // Find the order by ID
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