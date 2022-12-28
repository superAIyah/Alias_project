#include "async.http.server.h"

#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <thread>


server::server(const std::string &address, const std::string &port, std::size_t thread_pool_size) : thread_pool_size_(thread_pool_size),
                                                                                                    signals_(io_context_), acceptor_(io_context_) {
	std::vector<std::unique_ptr<DBConnection>> DBconnections;
	DBconnections.push_back(std::make_unique<DBConnection>(HOST, USERNAME1, PWD1, SCHEMA_NAME));
	DBconnections.push_back(std::make_unique<DBConnection>(HOST, USERNAME2, PWD2, SCHEMA_NAME));
	std::unique_ptr<IConnectionPool> ConnPool = std::make_unique<ConnectionPool>();
	ConnPool->FillPool(std::move(DBconnections));
	ConnectionProxy ConnProxy = ConnPool->get_connection();

	UDBM = new UserDBManager(*ConnProxy);
	WDBM = new WordDBManager(*ConnProxy);

	// Register to handle the signals that indicate when the server should exit.
	signals_.add(SIGINT);   // остановка процесса с терминала
	signals_.add(SIGTERM);  // сигнал от kill
	signals_.async_wait(boost::bind(&server::handle_stop, this));

	// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
	tcp::resolver resolver(io_context_);
	tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen();

	start_accept();
}

void server::run() {
	// Create a pool of threads to run all the io_contexts.
	std::vector<boost::shared_ptr<std::thread> > threads;
	for (std::size_t i = 0; i < thread_pool_size_; ++i) {
		boost::shared_ptr<std::thread> thread(new std::thread(
				boost::bind(&boost::asio::io_context::run, &io_context_)));
		threads.push_back(thread);
	}

	// Wait for all threads in the pool to exit.
	for (const auto& thread : threads)
		thread->join();
}

void server::start_accept() {
	Connection::pointer new_connection = Connection::create(io_context_);
	new_connection->SetServer(this);
	acceptor_.async_accept(new_connection->socket(),
	                       boost::bind(&server::handle_accept, this, new_connection,boost::asio::placeholders::error));
}

void server::handle_accept(boost::shared_ptr<Connection>& new_connection, const boost::system::error_code &e) {
	if (!e) {
		new_connection->start();
	}

	start_accept();
}

void server::handle_stop() {
	io_context_.stop();
}


#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "Russian");
	try {
		// Check command line arguments.
		if (argc != 4) {
			std::cerr << "Usage: http_server <address> <port> <threads>\n";
			std::cerr << "  For IPv4, try:\n";
			std::cerr << "    receiver 0.0.0.0 8080 1 \n";
			std::cerr << "  For IPv6, try:\n";
			std::cerr << "    receiver 0::0 80 1 \n";
			return 1;
		}

		// Initialise the server.
		auto num_threads = boost::lexical_cast<std::size_t>(argv[3]);
		server s(argv[1], argv[2], num_threads);

		// Run the server until stopped.
		s.run();
	}
	catch (std::exception &e) {
		std::cerr << "exception: " << e.what() << "\n";
	}

	return 0;
}