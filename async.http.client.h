#ifndef ALIAS_ASYNC_HTTP_CLIENT_H
#define ALIAS_ASYNC_HTTP_CLIENT_H
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "gameconfig.h"
#include "Message.h"

class Client;

#include "mainwindow.h"
#include "iclientinterface.h"
#include <QApplication>

#include "response.h"
#include "request.h"

using boost::asio::ip::tcp;


class Client {
public:
	Client(boost::asio::io_context &io_context, const std::string &server_, const std::string &port_);


	std::string serialize_auth(std::string user_login);

	std::string serialize_settings(GameConfig settings);

	std::string serialize_msg(Message msg);

	std::string serialize_round();

	void send_auth(std::string user_login);

	void send_settings(GameConfig settings);

	void send_msg(Message msg);

	void send_round();

	void handle_resolve(const boost::system::error_code &err,
	                    const tcp::resolver::results_type &endpoints);

	void handle_read(const boost::system::error_code &err);

	void handle_write(const boost::system::error_code &err);

	void handle_multiwrite(const boost::system::error_code &e);

	void run();

private:
	tcp::resolver resolver_;
	tcp::socket socket_;

	std::array<char, 8192> response_buf_;

	Request request_;
	Response response_;

	MainWindow *w;

	std::string user_login_;
	std::string game_id_;
	std::string team_id_;

	bool host_;


	std::vector<boost::shared_ptr<std::thread>> threads;
	/// The io_context used to perform asynchronous operations.
	boost::asio::io_context io_context_;

	std::string server;
	std::string port;
};

#endif //ALIAS_ASYNC_HTTP_CLIENT_H
