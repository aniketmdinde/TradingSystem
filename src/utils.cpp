#include "utils.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

using json = nlohmann::json;

// Load configuration (client_id and client_secret)
std::pair<std::string, std::string> load_config()
{
    const std::string config_path = "../../config/config.json";
    std::ifstream config_file(config_path);

    if (!config_file.is_open())
    {
        std::cerr << "Error: Unable to open config file at " << config_path << std::endl;
        return {"", ""};
    }

    try
    {
        json config_json;
        config_file >> config_json;

        std::string client_id = config_json.at("client_id").get<std::string>();
        std::string client_secret = config_json.at("client_secret").get<std::string>();

        return {client_id, client_secret};
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error parsing config file: " << e.what() << std::endl;
        return {"", ""};
    }
}

// Write callback for handling the response data
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// Authenticate with Deribit and obtain an access token
std::string authenticate_with_deribit(const std::string &client_id, const std::string &client_secret)
{
    std::string url = "https://test.deribit.com/api/v2/public/auth?client_id=" + client_id + "&client_secret=" + client_secret + "&grant_type=client_credentials";
    std::string response;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return {};
        }
        curl_easy_cleanup(curl);
    }

    auto json_response = json::parse(response, nullptr, false);
    if (json_response.contains("result") && json_response["result"].contains("access_token"))
    {
        return json_response["result"]["access_token"];
    }

    std::cerr << "Authentication failed. Response: " << response << std::endl;
    return {};
}

// Place an order on Deribit (buy order by default)
std::string place_order_on_deribit(const std::string &auth_token, const std::string &symbol, double price, int quantity, const std::string &order_type)
{
    std::string url = "https://test.deribit.com/api/v2/private/buy?amount=" + std::to_string(quantity) +
                      "&instrument_name=" + symbol + "&price=" + std::to_string(price) +
                      "&type=" + order_type + "&label=market";

    std::string response;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + auth_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return {};
        }
        curl_easy_cleanup(curl);
    }

    auto json_response = json::parse(response, nullptr, false);
    if (json_response.contains("result") && json_response["result"].contains("order"))
    {
        std::string order_id = json_response["result"]["order"]["order_id"];
        std::cout << "Order placed successfully. Deribit Order ID: " << order_id << std::endl;
        return order_id;
    }
    else
    {
        std::cerr << "Failed to place order. Response: " << response << std::endl;
        return {};
    }
}

// Cancel an order on Deribit using order ID
bool cancel_order_on_deribit(const std::string &auth_token, const std::string &order_id)
{
    std::string url = "https://test.deribit.com/api/v2/private/cancel?order_id=" + order_id;
    std::string response;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + auth_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return false;
        }
        curl_easy_cleanup(curl);
    }

    try
    {
        auto json_response = json::parse(response, nullptr, false);
        if (json_response.contains("result") && json_response["result"].is_object())
        {
            auto result = json_response["result"];
            if (result.contains("order_state") && result["order_state"] == "cancelled")
            {
                std::cout << "Order canceled successfully. Order ID: " << result["order_id"] << std::endl;
                return true;
            }
        }
        std::cerr << "Failed to cancel order. Response: " << response << std::endl;
        return false;
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "Error parsing response: " << e.what() << std::endl;
        return false;
    }
}

// Modify an order on Deribit by changing price and quantity
bool modify_order_on_deribit(const std::string &auth_token, const std::string &order_id, double new_price, int new_quantity)
{
    std::string url = "https://test.deribit.com/api/v2/private/edit";
    std::string response;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + auth_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // Prepare the JSON body
        std::string data = "{\"order_id\": \"" + order_id + "\", \"price\": " + std::to_string(new_price) +
                           ", \"amount\": " + std::to_string(new_quantity) + "}";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L); // Change to POST request
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str()); // Send the JSON body
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return false;
        }
        curl_easy_cleanup(curl);
    }

    // Parse the response and check for successful order modification
    auto json_response = json::parse(response, nullptr, false);
    if (json_response.contains("result") && json_response["result"].contains("order_id"))
    {
        std::cout << "Order modified successfully. New Order ID: " << json_response["result"]["order_id"] << std::endl;
        return true;
    }
    else
    {
        std::cerr << "Failed to modify order. Response: " << response << std::endl;
        return false;
    }
}

// Get the order book for a given instrument from Deribit
json get_order_book_from_deribit(const std::string &auth_token, const std::string &instrument_name, int depth)
{
    std::string url = "https://test.deribit.com/api/v2/public/get_order_book?depth=" + std::to_string(depth) + "&instrument_name=" + instrument_name;
    std::string response;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + auth_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return {};
        }
        curl_easy_cleanup(curl);
    }

    // Parse the response and check for successful order book retrieval
    auto json_response = json::parse(response, nullptr, false);
    if (json_response.contains("result"))
    {
        return json_response["result"];
    }
    else
    {
        std::cerr << "Failed to get order book. Response: " << response << std::endl;
        return {};
    }
}

// Function to view positions from Deribit API
std::vector<Position> view_positions_from_deribit(const std::string &auth_token, const std::string &currency, const std::string &kind)
{
    CURL *curl;
    CURLcode res;
    std::string read_buffer;
    std::vector<Position> orders;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl)
    {
        // Set the URL and headers
        curl_easy_setopt(curl, CURLOPT_URL, ("https://test.deribit.com/api/v2/private/get_positions?currency=" + currency + "&kind=" + kind).c_str());
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + auth_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

        // Perform the request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
        }
        else
        {
            // Parse the JSON response using nlohmann::json
            try
            {
                auto json_response = json::parse(read_buffer, nullptr, false); // Use nlohmann's json::parse

                if (json_response.is_object() && json_response.contains("result"))
                {
                    for (const auto &item : json_response["result"])
                    {
                        Position pos;
                        pos.instrument_name = item["instrument_name"].get<std::string>();
                        pos.direction = item["direction"].get<std::string>();
                        pos.average_price = item["average_price"].get<double>();
                        pos.delta = item["delta"].get<double>();
                        pos.estimated_liquidation_price = item["estimated_liquidation_price"].is_null() ? 0.0 : item["estimated_liquidation_price"].get<double>();
                        pos.floating_profit_loss = item["floating_profit_loss"].get<double>();
                        pos.index_price = item["index_price"].get<double>();
                        pos.initial_margin = item["initial_margin"].get<double>();
                        pos.leverage = item["leverage"].get<double>();
                        pos.maintenance_margin = item["maintenance_margin"].get<double>();
                        pos.mark_price = item["mark_price"].get<double>();
                        pos.open_orders_margin = item["open_orders_margin"].get<double>();
                        pos.realized_funding = item["realized_funding"].get<double>();
                        pos.realized_profit_loss = item["realized_profit_loss"].get<double>();
                        pos.settlement_price = item["settlement_price"].get<double>();
                        pos.size = item["size"].get<double>();
                        pos.size_currency = item["size_currency"].get<double>();
                        pos.total_profit_loss = item["total_profit_loss"].get<double>();
                        pos.kind = item["kind"].get<std::string>();

                        orders.push_back(pos);
                    }
                }
                else
                {
                    std::cerr << "Invalid JSON response" << std::endl;
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            }
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return orders;
}
