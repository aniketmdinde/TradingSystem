#ifndef ORDER_MANAGEMENT_H
#define ORDER_MANAGEMENT_H

#include <string>
#include <vector>
#include <utility>
#include <nlohmann/json.hpp>

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

    bool place_order(const std::string &symbol, double price, int quantity);
    bool modify_order(const std::string &order_id, double new_price, int new_quantity);
    bool cancel_order(const std::string &order_id);
    bool track_order(const std::string &order_id);
    std::vector<Order> get_all_orders();
    std::vector<Order> get_open_orders();
    bool view_positions(const std::string &currency, const std::string &kind);
    bool get_order_book(const std::string &instrument_name, int depth);
    bool handle_orderbook_update(const nlohmann::json &orderbook_data);
    std::string get_auth_token() const { return auth_token; }

private:
    std::vector<Order> orders;
    std::string client_id;
    std::string client_secret;
    std::string auth_token;

    void authenticate();
};

#endif // ORDER_MANAGEMENT_H
