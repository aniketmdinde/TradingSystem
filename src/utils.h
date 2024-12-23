#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <utility>
#include <nlohmann/json.hpp>

struct Position
{
    std::string instrument_name;
    std::string direction;
    double average_price;
    double delta;
    double estimated_liquidation_price;
    double floating_profit_loss;
    double index_price;
    double initial_margin;
    double leverage;
    double maintenance_margin;
    double mark_price;
    double open_orders_margin;
    double realized_funding;
    double realized_profit_loss;
    double settlement_price;
    double size;
    double size_currency;
    double total_profit_loss;
    std::string kind;
};

std::pair<std::string, std::string> load_config();

std::string authenticate_with_deribit(const std::string &client_id, const std::string &client_secret);

std::string place_order_on_deribit(const std::string &auth_token, const std::string &symbol, double price, int quantity, const std::string &order_type = "limit");
bool cancel_order_on_deribit(const std::string &auth_token, const std::string &order_id);
bool modify_order_on_deribit(const std::string &auth_token, const std::string &order_id, double new_price, int new_quantity);
nlohmann::json get_order_book_from_deribit(const std::string &auth_token, const std::string &instrument_name, int depth);
std::vector<Position> view_positions_from_deribit(const std::string &auth_token, const std::string &currency, const std::string &kind);

#endif // UTILS_H