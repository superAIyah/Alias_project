#ifndef BOOST_ASIO_SERVER_CONNECTION_H
#define BOOST_ASIO_SERVER_CONNECTION_H


class Connection;


#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <memory>
#include <algorithm>

#include "request.h"
#include "async.http.server.h"


struct information
{
	int game_id;
	int team_id;
	std::string login;
	std::string text;
	std::string str;
};

typedef boost::asio::ip::tcp tcp;

class Connection : public boost::enable_shared_from_this<Connection>, private boost::noncopyable {
public:
	/// Construct a connection with the given io_context.
	explicit Connection(boost::asio::io_context &io_context);

	void SetServer(server *server) { Server = server; }

	/// Get the socket associated with the connection.
	tcp::socket &socket();

	/// Start the first asynchronous operation for the connection.
	void start();

	typedef boost::shared_ptr<Connection> pointer;

	///	Create Shared_ptr to new connection
	static pointer create(boost::asio::io_context& io_context)
	{
		return pointer(new Connection(io_context));
	}

private:
	/// Write to socket_
	void write(tcp::socket& to_socket, std::string response, bool callback = true);

	/// Read from socket_
	void read();

	/// Handle completion of a read operation.
	void handle_read(const boost::system::error_code &e, std::size_t bytes_transferred);

	void handle_write(bool callback, const boost::system::error_code &e);

	void send_kw_to_host(int game_id_, int team_id_);

	void authorization(Request request);
	void auth_already_online();
	void auth_wrong_pwd();
	void auth_not_online(const std::string &user_login);
	void auth_new_client(const std::string &user_login, std::string password);

	void settings(Request request, const boost::system::error_code &e);
	void update_stats(tcp::socket &sock, std::string user_login, bool last);

	void msg(Request request, const boost::system::error_code &e);
	static std::string text_msg(const information& info);
	void msg_from_host(const information& info, const boost::system::error_code &e);
	void msg_from_player(const information& info, const boost::system::error_code &e);
	std::string guess_msg(const information& info);

	void round(Request request, const boost::system::error_code &e);
	void update_hosts(int game_id);

	void create_logins_board(const std::vector<std::pair<tcp::socket *, std::string>> &team_sockets, int game_id);
	void create_teams_board(int num_teams, int game_id);
	void sendDB(int game_id);

	void player_left();
private:
	/// Strand to ensure the connection's handlers are not called concurrently.
	boost::asio::strand<boost::asio::io_context::executor_type> strand_;

	/// Socket for the connection.
	tcp::socket socket_;

	std::string user_login_;

	/// Buffer for incoming data.
	boost::asio::streambuf buffer_;

	/// The incoming request.
	Request request_;

	server *Server;
};


#endif //BOOST_ASIO_SERVER_CONNECTION_H
