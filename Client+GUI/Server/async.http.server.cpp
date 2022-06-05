
#include "async.http.server.h"

#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <thread>


std::string HandlerSendMes(const Request &request) {
	// use data from request
	Response mes2send;
	if (request.parameters.at("is_word") == "true") {
		mes2send.method = "useful";
		mes2send.parameters["user_pts"] = USER_GUESS_POINTS;
		mes2send.parameters["host_pts"] = HOST_GUESS_POINTS;
	}
	else {
		mes2send.method = "msg";
		mes2send.parameters["text"] = request.parameters.at("text");
	}

	mes2send.parameters["user_login"] = request.parameters.at("user_login");
	mes2send.parameters["team_id"] = request.parameters.at("team_id");
	return Response2String(mes2send);
}


server::server(const std::string &address, const std::string &port,
               std::size_t thread_pool_size)
		: thread_pool_size_(thread_pool_size),
		  signals_(io_context_),
		  acceptor_(io_context_),
		  new_connection_() {
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

//            заполнение респонсов для авторизации и тд
	request_router.addHandler("msg", HandlerSendMes);

	// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
	boost::asio::ip::tcp::resolver resolver(io_context_);
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen();

	start_accept();
}

void server::run() {
	// Create a pool of threads to run all of the io_contexts.
	std::vector<boost::shared_ptr<std::thread> > threads;
	for (std::size_t i = 0; i < thread_pool_size_; ++i) {
		boost::shared_ptr<std::thread> thread(new std::thread(
				boost::bind(&boost::asio::io_context::run, &io_context_)));
		threads.push_back(thread);
	}

	// Wait for all threads in the pool to exit.
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();
}

void server::start_accept() {
	Connection *conn = new Connection(io_context_, request_router);
	conn->SetServer(this);
	new_connection_.reset(conn);
	acceptor_.async_accept(new_connection_->socket(),
	                       boost::bind(&server::handle_accept, this,
	                                   boost::asio::placeholders::error));
}

void server::handle_accept(const boost::system::error_code &e) {
	if (!e) {
		new_connection_->start();
	}

	start_accept();
}

void server::handle_stop() {
	io_context_.stop();
}


#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/lexical_cast.hpp>

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "Russian");
//    std::locale::global(std::locale(".KOI8-R"));
	try {
		// Check command line arguments.
		if (argc != 4) {
			std::cerr << "Usage: http_server <address> <port> <threads>\n";
			std::cerr << "  For IPv4, try:\n";
			std::cerr << "    receiver 0.0.0.0 80 1 \n";
			std::cerr << "  For IPv6, try:\n";
			std::cerr << "    receiver 0::0 80 1 \n";
			return 1;
		}

		// Initialise the server.
		std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[3]);
		server s(argv[1], argv[2], num_threads);

		// Run the server until stopped.
		s.run();
	}
	catch (std::exception &e) {
		std::cerr << "exception: " << e.what() << "\n";
	}

	return 0;
}