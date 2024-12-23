#include "utils.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

using json = nlohmann::json;

std::pair<std::string, std::string> load_config()
{
    // Path to the configuration file
    const std::string config_path = "../../config/config.json";

    // Open the configuration file
    std::ifstream config_file(config_path);
    if (!config_file.is_open())
    {
        std::cerr << "Error: Unable to open config file at " << config_path << std::endl;
        return {"", ""};
    }

    try
    {
        // Parse the JSON
        json config_json;
        config_file >> config_json;

        // Extract client_id and client_secret
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

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

std::string authenticate_with_deribit(const std::string &client_id, const std::string &client_secret)
{
    std::string url = "https://test.deribit.com/api/v2/public/auth?client_id=" + client_id + "&client_secret=" + client_secret + "&grant_type=client_credentials";
    std::string response;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); // Change to GET request
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

    // Parse and validate the API response
    auto json_response = json::parse(response, nullptr, false);
    if (json_response.contains("result") && json_response["result"].contains("access_token"))
    {
        return json_response["result"]["access_token"];
    }

    std::cerr << "Authentication failed. Response: " << response << std::endl;
    return {};
}

std::string place_order_on_deribit(const std::string &auth_token, const std::string &symbol, double price, int quantity, const std::string &order_type)
{
    // Build the URL with query parameters as per the Deribit API documentation
    std::string url = "https://test.deribit.com/api/v2/private/buy?amount=" + std::to_string(quantity) +
                      "&instrument_name=" + symbol + "&price=" + std::to_string(price) +
                      "&type=" + order_type + "&label=market";

    std::string response;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        // Set up headers for authentication
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + auth_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // Set the URL, headers, and response handling for cURL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); // Using GET method as per the example
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return {}; // Return empty string on failure
        }
        curl_easy_cleanup(curl);
    }

    // Parse the response JSON
    auto json_response = json::parse(response, nullptr, false);
    if (json_response.contains("result") && json_response["result"].contains("order"))
    {
        // Extract the order_id and return it
        std::string order_id = json_response["result"]["order"]["order_id"];
        std::cout << "Order placed successfully. Deribit Order ID: " << order_id << std::endl;
        return order_id; // Return the order ID
    }
    else
    {
        std::cerr << "Failed to place order. Response: " << response << std::endl;
        return {}; // Return empty string if the order placement failed
    }
}

// Function to cancel order on Deribit
bool cancel_order_on_deribit(const std::string &auth_token, const std::string &order_id)
{
    std::string url = "https://test.deribit.com/api/v2/private/cancel?order_id=" + order_id;
    std::string response;
    CURL *curl = curl_easy_init();

    if (curl)
    {
        struct curl_slist *headers = nullptr;
        // Set the Authorization and Content-Type headers
        headers = curl_slist_append(headers, ("Authorization: Bearer " + auth_token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // Set curl options
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET"); // Explicitly specify GET method here
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return false;
        }
        curl_easy_cleanup(curl);
    }

    // Parse and validate the API response
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
        return false; // Ensure we return false here if the response is invalid or cancellation failed
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "Error parsing response: " << e.what() << std::endl;
        return false; // Ensure we return false if parsing fails
    }
}