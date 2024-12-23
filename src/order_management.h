#ifndef ORDER_MANAGEMENT_H
#define ORDER_MANAGEMENT_H

#include <string>
#include <vector>
#include <utility>

struct Order
{
    std::string order_id;
    std::string symbol;
    double price;
    int quantity;
    std::string status;
};

class OrderManagement
{
public:
    OrderManagement();
    OrderManagement(const std::string &auth_token);

    void place_order(const std::string &symbol, double price, int quantity);
    void cancel_order(const std::string &order_id);
    void modify_order(const std::string &order_id, double new_price, int new_quantity);
    void track_order(const std::string &order_id);
    std::vector<Order> get_all_orders();
    std::vector<Order> get_open_orders();
    void view_positions(const std::string &currency, const std::string &kind);
    void get_order_book(const std::string &instrument_name, int depth);

private:
    std::vector<Order> orders;
    std::string client_id;
    std::string client_secret;
    std::string auth_token;

    void authenticate();
};

#endif // ORDER_MANAGEMENT_H
