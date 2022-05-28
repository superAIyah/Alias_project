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
#include <QThread>
#include <QObject>
#include "GUI/window_configs/headers/gameconfig.h"
#include "Message.h"
#include "LeaderBoard.h"
#include "Leader.h"

class Client;

#include "GUI/window_autorise/headers/mainwindow.h"
//#include "iclientinterface.h"
#include <QApplication>

#include "response.h"
#include "request.h"

using boost::asio::ip::tcp;

class Worker : public QObject {
Q_OBJECT

private:
	boost::asio::io_context* io_context_;
	QApplication* app_;

public:
	Worker(boost::asio::io_context &io_context, QApplication* app) : io_context_(&io_context), app_(app) {}
	~Worker() = default;

public slots:
	void process();
};

class Client : QObject {
//	Q_OBJECT
public:
	Client(boost::asio::io_context &io_context, const std::string &server_, const std::string &port_, QThread* thread);

	void send_auth(std::string user_login);

	void send_settings(GameConfig settings, int num_teams, int round_duration_);

	void send_msg(std::string text);

	void send_round();

	void handle_resolve(const boost::system::error_code &err,
	                    const tcp::resolver::results_type &endpoints);

	void handle_read(const boost::system::error_code &err);

	void handle_write(const boost::system::error_code &err);

	void handle_multiwrite(const boost::system::error_code &e);

	int RoundDuration(){return round_duration;}

private:
	Request parse(std::string req_data);

	std::string serialize_auth(std::string user_login);

	std::string serialize_settings(GameConfig settings);

	std::string serialize_msg(Message msg);

	std::string serialize_round();

	tcp::resolver resolver_;
	tcp::socket socket_;

	std::array<char, 8192> response_buf_;

	Request request_;
	Response response_;

	MainWindow *w;

	std::string user_login_;
	int game_id_;
	int team_id_;
	std::vector<std::string> team_players;
	int round_duration;
	LeaderBoard leaderboard_;
	std::string host_login;
	bool is_host_;

	int num_teams_;

	/// The io_context used to perform asynchronous operations.
	boost::asio::io_context io_context_;

	std::string server;
	std::string port;

	QThread* thread_;
};

#endif //ALIAS_ASYNC_HTTP_CLIENT_H
