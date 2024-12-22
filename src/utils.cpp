#include "utils.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

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
        nlohmann::json config_json;
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