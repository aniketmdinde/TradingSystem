#ifndef ORDER_MANAGEMENT_H
#define ORDER_MANAGEMENT_H

#include <string>
#include <vector>
#include <utility> // For std::pair

// Order struct to hold order details
struct Order
{
    std::string order_id;
    std::string symbol;
    double price;
    int quantity;
    std::string status; // Status of the order, e.g., "Pending", "Completed"
};

// Class to manage orders
class OrderManagement
{
public:
    OrderManagement();
    void place_order(const std::string &symbol, double price, int quantity);
    void cancel_order(const std::string &order_id);
    void track_order(const std::string &order_id);
    std::vector<Order> get_all_orders();

private:
    std::vector<Order> orders; // List of all orders
    std::string client_id;
    std::string client_secret;
};

#endif // ORDER_MANAGEMENT_H