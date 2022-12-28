#ifndef BOOST_ASIO_SERVER_ASYNC_HTTP_SERVER_H
#define BOOST_ASIO_SERVER_ASYNC_HTTP_SERVER_H

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <ctime>
#include <queue>

class server;

#include "connection.h"
#include "request.h"

#include "DBConnection.h"
#include "UserDBManager.h"
#include "WordDBManager.h"

typedef boost::asio::ip::tcp tcp;

const std::string HOST = "localhost";
const std::string SCHEMA_NAME = "test";
const std::string USERNAME1 = "server_1";
const std::string PWD1 = "1111";
const std::string USERNAME2 = "server_2";
const std::string PWD2 = "2222";

#define USER_GUESS_POINTS "2"
#define HOST_GUESS_POINTS "1"

struct Table {
	int num_players;
	std::map<int, std::vector<std::pair<tcp::socket *, std::string>>> team_sockets;
	std::map<int, std::queue<Word>> team_words;
	std::map<int, Word> cur_words;
	std::map<int, std::pair<tcp::socket *, std::string>> hosts;
	std::map<std::string, int> leader_board;
	int round_duration;
	time_t round_end;
	int rounds_remaining;
	int clients_responded;
};

struct Lobby {
	std::vector<std::pair<tcp::socket *, std::string>> players;
};


class server : private boost::noncopyable {
public:
	explicit server(const std::string &address, const std::string &port, std::size_t thread_pool_size);

	/// Run the server's io_context loop.
	void run();

	std::map<std::string, Lobby> WaitingLine;

	std::map<int, Table> Games;

	IUserDBManager *UDBM;
	IWordDBManager *WDBM;

//	ключ - логин, значения - game_id & team_id (по -1 если не в игре)
	std::map<std::string, std::pair<int, int>> players_online;
private:
	/// Initiate an asynchronous accept operation.
	void start_accept();

	/// Handle completion of an asynchronous accept operation.
	void handle_accept(boost::shared_ptr<Connection>& new_connection, const boost::system::error_code &e);

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
	tcp::acceptor acceptor_;
};


#endif //BOOST_ASIO_SERVER_ASYNC_HTTP_SERVER_H
