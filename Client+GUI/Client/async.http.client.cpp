#include "async.http.client.h"

#include <utility>

using boost::asio::ip::tcp;


void Worker::process() {
//	app_->exec();
	io_context_->run();
	emit finished();
}

Request Client::parse(boost::asio::streambuf &req_data) {
	Request request;
	std::vector<std::string> request_arr;

//	for testing
//	boost::split(request_arr, req_data, [](char c) { return c == ':'; });

//  for deploy
	std::istream is(&req_data);
	std::string token;

	while (true) {
		std::getline(is, token, '\r');
		if (!token.empty()) {
			request_arr.push_back(token);
			std::cout << token << '\n';
			std::getline(is, token);
		}
		else {
			std::getline(is, token);
			std::cout << '\n';
			break;
		}
	}

	request.method = request_arr[0];

	if (request.method == "settings") {
		game_id_ = std::stoi(request_arr[1]);
		team_id_ = std::stoi(request_arr[2]);

		for (int i = 3; i < request_arr.size(); ++i) {
			team_players.push_back(request_arr[i]);
		}
		host_login = team_players[0];
		if (num_teams_ > 1) {
			for (int i = 1; i <= num_teams_; ++i) {
				leaderboard_.leaders.emplace_back("Team " + std::to_string(i), 0, false);
			}
		}
		else {
			for (int i = 0; i < team_players.size(); ++i) {
				leaderboard_.leaders.emplace_back(team_players[i], 0, (i == 0));
			}
		}
		leaderboard_.size = leaderboard_.leaders.size();
	}
	if (request.method == "keyword") {
		request.parameters["new_keyword"] = request_arr[1];
	}
	if (request.method == "warning") {
		request.parameters["warning_text"] = request_arr[1];
	}
	if (request.method == "msg") {
		request.parameters["user_login"] = request_arr[1];
		request.parameters["text"] = request_arr[2];
	}
	if (request.method == "guess") {
		request.parameters["user_login"] = request_arr[1];
		request.parameters["team_id"] = request_arr[2];
		request.parameters["text"] = request_arr[3];

		int host_pts = std::stoi(request_arr[5]);

		if (num_teams_ > 1) {
			leaderboard_.leaders[std::stoi(request.parameters["team_id"]) - 1].points += host_pts;
		}
		else {
			int user_pts = std::stoi(request_arr[4]);
			for (int i = 0; i < leaderboard_.size; ++i) {
				if (leaderboard_.leaders[i].name == request.parameters["user_login"]) {
					leaderboard_.leaders[i].points += user_pts;
				}
				if (leaderboard_.leaders[i].host) {
					leaderboard_.leaders[i].points += host_pts;
				}
			}
		}

	}
	if (request.method == "round") {
		host_login = request_arr[1];
		if (num_teams_ == 1) {
			for (int i = 0; i < leaderboard_.size; ++i) {
				leaderboard_.leaders[i].host = (leaderboard_.leaders[i].name == host_login);
			}
		}
	}
	if (request.method == "auth") {
		request.parameters["status"] = request_arr[1];
	}
	if (request.method == "user_info") {
		request.parameters["num_of_wins"] = request_arr[1];
		request.parameters["num_of_losses"] = request_arr[2];
		request.parameters["player_rating"] = request_arr[3];
	}

	return request;
}


Client::Client(boost::asio::io_context &io_context, std::string server_, std::string port_, QThread *thread) :
		resolver_(io_context), socket_(io_context), server(std::move(server_)), port(std::move(port_)), thread_(thread) {
	w = new MainWindow(this);
	w->show();
	// Start an asynchronous resolve to translate the server and service names
	// into a list of endpoints.
	resolver_.async_resolve(server, port,
	                        boost::bind(&Client::handle_resolve, this,
	                                    boost::asio::placeholders::error,
	                                    boost::asio::placeholders::results));
}

void Client::write(tcp::socket &to_socket, const std::string &request, bool callback) {
	boost::asio::async_write(to_socket, boost::asio::buffer(request.data(), request.size()),
	                         boost::bind(&Client::handle_write, this, callback,
	                                     boost::asio::placeholders::error));
}

void Client::read() {
	boost::asio::async_read_until(socket_, response_buf_, "\r\n\r\n",
	                              boost::bind(&Client::handle_read, this, boost::asio::placeholders::error));
}

std::string Client::serialize_auth(const std::string &user_login, const std::string &pwd) {
	return "auth\r\n" + user_login + "\r\n" + pwd + "\r\n\r\n";
}

std::string Client::serialize_settings(GameConfig settings) {
	return "settings\r\n" + user_login_ + "\r\n" + settings.GetSettings() + "\r\n";
}

std::string Client::serialize_msg(const Message &msg) {
	return "msg\r\n" + user_login_ + "\r\n" + std::to_string(game_id_) + "\r\n" + std::to_string(team_id_) + "\r\n" + msg.msg + "\r\n" +
	       (user_login_ == host_login ? "host" : "not_host") + "\r\n\r\n";
}

std::string Client::serialize_round() const {
	return "round\r\n" + std::to_string(game_id_) + "\r\n\r\n";
}

void Client::send_auth(const std::string &user_login, const std::string &pwd) {
	user_login_ = user_login;
	std::string str = serialize_auth(user_login, pwd);
	write(socket_, str, false);
}

void Client::send_settings(GameConfig settings, int num_teams, int round_duration_) {
	num_teams_ = num_teams;
	round_duration = round_duration_;

	std::string str = serialize_settings(std::move(settings));
	write(socket_, str, false);
}

void Client::send_msg(std::string text) {
	Message msg(user_login_, true, std::move(text));
	std::string str = serialize_msg(msg);
	write(socket_, str, false);
}

void Client::send_round() {
	if (!sent_round) {
		std::cout << "round sent\n\n";
		sent_round = true;
		std::string str = serialize_round();
		write(socket_, str, false);
	}
}

void Client::handle_resolve(const boost::system::error_code &err, const tcp::resolver::results_type &endpoints) {
	if (!err) {
		// Attempt a Client to each endpoint in the list until we
		// successfully establish a Client.
		boost::asio::async_connect(socket_, endpoints,
		                           boost::bind(&Client::handle_write, this, true,
		                                       boost::asio::placeholders::error));
	}
	else {
		std::cout << "Error1: " << err.message() << "\n";
	}
}

void Client::handle_read(const boost::system::error_code &err) {
	if (!err) {
//                парсинг
		Request request = parse(response_buf_);

		if (request.method == "auth") {
			if (request.parameters["status"] == "ok") {
				//  Вызываем функцию GUI (для открытия окна)
				w->NextWindow();
			}
			else if (request.parameters["status"] == "already_online") {
				// TODO:
				//  Вызываем функцию ФЕДИ! (для вывода ошибки)
			}
			else if (request.parameters["status"] == "wrong_password") {
				// TODO:
				//  Вызываем функцию ФЕДИ! (для вывода ошибки)
			}
		}

		if (request.method == "settings") {
			w->configWindow->NextWindow();
			w->configWindow->gameWindow->UpdateLeaderboard(leaderboard_);
		}

		if (request.method == "msg") {
			w->configWindow->gameWindow->UpdateMessages(Message(request.parameters["user_login"],
			                                                    (request.parameters["user_login"] == user_login_),
			                                                    request.parameters["text"]));
		}

		if (request.method == "guess") {
			if (std::stoi(request.parameters["team_id"]) == team_id_) {
				w->configWindow->gameWindow->UpdateMessages(Message(request.parameters["user_login"],
				                                                    (request.parameters["user_login"] == user_login_),
				                                                    request.parameters["text"]));
			}
			w->configWindow->gameWindow->UpdateLeaderboard(leaderboard_);
		}

		if (request.method == "round") {
			w->configWindow->gameWindow->UpdateLeaderboard(leaderboard_);
			w->configWindow->gameWindow->NewRound();
			sent_round = false;
		}

		if (request.method == "gameover") {
			w->configWindow->gameWindow->ShowConfig();
			leaderboard_.Clear();
			team_players.clear();
			sent_round = false;
		}

		if (request.method == "keyword") {
			w->configWindow->gameWindow->UpdateKeyword(request.parameters["new_keyword"]);
		}

		if (request.method == "warning") {
			w->configWindow->gameWindow->ShowWarning();
		}

		if (request.method == "user_info") {
			w->configWindow->update_stats(user_login_, std::stoi(request.parameters["num_of_wins"]), std::stoi(request.parameters["num_of_losses"]),
			                              (unsigned int) std::stoi(request.parameters["player_rating"]));
		}

		handle_write(true, err);
	}
	else {
		std::cout << "Error2: " << err.message() << "\n";
	}
}

void Client::handle_write(bool callback, const boost::system::error_code &err) {
	if (!err) {
		if (callback) {
//			boost::asio::async_read_until(socket_, response_buf_, "\r\n\r\n",
//			                              boost::bind(&Client::handle_read, this, boost::asio::placeholders::error));
			read();
		}
		else return;
	}
	else {
		std::cout << "Error3: " << err.message() << "\n";
	}
}

int main(int argc, char *argv[]) {
	try {
		if (argc != 3) {
			std::cout << "Usage: async_client <server> <port>\n";
			std::cout << "Example:\n";
			std::cout << "Alias 127.0.0.1 8080\n";
			return 1;
		}

		QApplication a(argc, argv);
		//std::cout << w.configWindow->gameWindow->per << std::endl;

		boost::asio::io_context io_context;

		auto *thread = new QThread;
		auto *worker = new Worker(io_context, &a);
		worker->moveToThread(thread);

		Client c(io_context, argv[1], argv[2], thread);

		QObject::connect(thread, SIGNAL(started()), worker, SLOT(process()));
		QObject::connect(worker, &Worker::finished, thread, &QThread::quit);
		QObject::connect(worker, &Worker::finished, worker, &Worker::deleteLater);
		QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

		thread->start();

		QApplication::exec();

	}
	catch (std::exception &e) {
		std::cout << "Exception: " << e.what() << "\n";
	}

	return 0;
}