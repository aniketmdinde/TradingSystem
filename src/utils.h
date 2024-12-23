#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <utility>

std::pair<std::string, std::string> load_config();

std::string authenticate_with_deribit(const std::string &client_id, const std::string &client_secret);

std::string place_order_on_deribit(const std::string &auth_token, const std::string &symbol, double price, int quantity, const std::string &order_type = "limit");
bool cancel_order_on_deribit(const std::string &auth_token, const std::string &order_id);
bool modify_order_on_deribit(const std::string &auth_token, const std::string &order_id, double new_price, int new_quantity);

#endif // UTILS_H