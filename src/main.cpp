#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

void worker_thread(int id)
{
    std::cout << "Thread " << id << " is running\n";
}

void on_open(websocketpp::connection_hdl hdl)
{
    std::cout << "Connection opened\n";
}

void on_message(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg)
{
    std::cout << "Received message: " << msg->get_payload() << std::endl;
}

int main()
{
    boost::asio::io_context io_context;

    std::cout << "Boost.Asio IO context set up successfully!\n";

    boost::thread_group threads;
    for (int i = 0; i < 4; ++i)
    {
        threads.create_thread(boost::bind(worker_thread, i));
    }

    // WebSocket++ server setup
    websocketpp::server<websocketpp::config::asio> server;

    server.set_open_handler(&on_open);
    server.set_message_handler(&on_message);

    // Initialize the server to listen on port 9002
    server.init_asio(&io_context);
    server.listen(9002);
    server.start_accept();

    std::cout << "WebSocket++ server started on port 9002\n";

    // Run the Boost.Asio io_context in the main thread
    io_context.run();

    threads.join_all();
    std::cout << "All threads finished!\n";

    return 0;
}