#ifndef BOOST_ASIO_SERVER_ASYNC_HTTP_SERVER_H
#define BOOST_ASIO_SERVER_ASYNC_HTTP_SERVER_H

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <ctime>

namespace http {
    namespace server3 {

        class server;
    }
}

#include "connection.h"
#include "request.h"
#include "response.h"
#include "router.h"

#include "DBConnection.h"
#include "UserDBManager.h"
#include "WordDBManager.h"

const std::string HOST = "localhost";
const std::string SCHEMA_NAME = "test";
const std::string USERNAME1 = "server_1";
const std::string PWD1 = "1111";
const std::string USERNAME2 = "server_2";
const std::string PWD2 = "2222";

struct Table{
    std::string settings;
    std::map<int, std::vector< std::pair<boost::asio::ip::tcp::socket*, std::string>>> team_sockets;
    std::map<int, std::vector<std::string>> team_words;
    std::map<int, std::string> cur_words;
    std::map<int, boost::asio::ip::tcp::socket*> hosts;
    time_t round_end;
    int rounds_remaining;
};

struct Lobby{
    std::vector<std::pair<boost::asio::ip::tcp::socket*, std::string>> players;
};

namespace http {
    namespace server3 {

        class server
                : private boost::noncopyable {
        public:
            explicit server(const std::string &address, const std::string &port,
                            std::size_t thread_pool_size);

            /// Run the server's io_context loop.
            void run();

            std::map<std::string, Lobby> WaitingLine;

            std::map<int, Table> Games;

            IUserDBManager* UDBM;
            IWordDBManager* WDBM;
        private:
            /// Initiate an asynchronous accept operation.
            void start_accept();

            /// Handle completion of an asynchronous accept operation.
            void handle_accept(const boost::system::error_code &e);

            /// Handle a request to stop the server.
            void handle_stop();

            /// The number of threads that will call io_context::run().
            std::size_t thread_pool_size_;

        private:

            /// The io_context used to perform asynchronous operations.
            boost::asio::io_context io_context_;

            /// The signal_set is used to register for process termination notifications.
            boost::asio::signal_set signals_;

            /// Acceptor used to listen for incoming connections.
            boost::asio::ip::tcp::acceptor acceptor_;

            /// The next connection to be accepted.
            boost::shared_ptr<Connection> new_connection_;

            /// The handler for all incoming requests.
            Router<std::string (*)(const Request &request)> request_router;

            std::map<int, time_t> round_time;
        };

    } // namespace server3
} // namespace http

#endif //BOOST_ASIO_SERVER_ASYNC_HTTP_SERVER_H
