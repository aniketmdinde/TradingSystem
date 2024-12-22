#include <iostream>
#include <boost/asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <httplib.h>

void test_boost_asio()
{
    try
    {
        boost::asio::io_context io_context;
        boost::asio::steady_timer timer(io_context, boost::asio::chrono::seconds(1));
        timer.wait();
        std::cout << "Boost.Asio test passed.\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Boost.Asio test failed: " << ex.what() << "\n";
    }
}

void test_websocketpp()
{
    try
    {
        websocketpp::server<websocketpp::config::asio> server;
        server.init_asio();
        std::cout << "WebSocket++ test passed.\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "WebSocket++ test failed: " << ex.what() << "\n";
    }
}

void test_nlohmann_json()
{
    try
    {
        nlohmann::json json_obj = {{"key", "value"}, {"number", 42}};
        std::cout << "nlohmann::json test passed. JSON: " << json_obj.dump() << "\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "nlohmann::json test failed: " << ex.what() << "\n";
    }
}

void test_spdlog()
{
    try
    {
        spdlog::info("spdlog test passed.");
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cerr << "spdlog test failed: " << ex.what() << "\n";
    }
}

void test_httplib()
{
    try
    {
        httplib::Client cli("https://www.google.com/");
        auto res = cli.Get("/");
        if (res && res->status == 200)
        {
            std::cout << "cpp-httplib test passed. Status: " << res->status << "\n";
        }
        else
        {
            std::cerr << "cpp-httplib test failed. Unable to fetch example.com\n";
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << "cpp-httplib test failed: " << ex.what() << "\n";
    }
}

int main()
{
    std::cout << "Testing installed libraries...\n";

    test_boost_asio();
    test_websocketpp();
    test_nlohmann_json();
    test_spdlog();
    test_httplib();

    std::cout << "Library testing completed.\n";
    return 0;
}