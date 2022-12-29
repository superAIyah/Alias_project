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
#include "gameconfig.h"
#include "Message.h"
#include "LeaderBoard.h"
#include "Leader.h"

class Client;

#include "../GUI/window_autorise/headers/mainwindow.h"
#include <QApplication>

#include "request.h"

using boost::asio::ip::tcp;

class Worker : public QObject {
Q_OBJECT

private:
	boost::asio::io_context *io_context_;
	QApplication *app_;

public:
	Worker(boost::asio::io_context &io_context, QApplication *app) : io_context_(&io_context), app_(app) {}

	~Worker() override = default;

public slots:
	void process();
signals:
	void finished();
};

class Client : QObject {
//	Q_OBJECT
public:
	Client(boost::asio::io_context &io_context, std::string server_, std::string port_, QThread *thread);

	void send_auth(const std::string& user_login, const std::string& pwd);

	void send_settings(GameConfig settings, int num_teams, int round_duration_);

	void send_msg(std::string text);

	void send_round();

	void handle_resolve(const boost::system::error_code &err,
	                    const tcp::resolver::results_type &endpoints);

	void handle_read(const boost::system::error_code &err);

	void handle_write(bool callback, const boost::system::error_code &err);

	int RoundDuration() const { return round_duration; }
	
	std::string getNick() { return user_login_; }

private:
	void write(tcp::socket& to_socket, const std::string& response, bool callback);

	void read();

	Request parse(boost::asio::streambuf& req_data);

	static std::string serialize_auth(const std::string& user_login, const std::string& pwd);

	std::string serialize_settings(GameConfig settings);

	std::string serialize_msg(const Message& msg);

	std::string serialize_round() const;

	tcp::resolver resolver_;
	tcp::socket socket_;

	boost::asio::streambuf response_buf_;

	Request request_;

	MainWindow *w;

	std::string user_login_;
	int game_id_{};
	int team_id_{};
	std::vector<std::string> team_players;
	int round_duration{};
	LeaderBoard leaderboard_;
	std::string host_login;
	int num_teams_{};
	bool sent_round = false;

	/// The io_context used to perform asynchronous operations.
	boost::asio::io_context io_context_;

	std::string server;
	std::string port;

	QThread *thread_;
};

#endif //ALIAS_ASYNC_HTTP_CLIENT_H
