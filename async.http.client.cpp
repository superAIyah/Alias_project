#include "async.http.client.h"

using boost::asio::ip::tcp;


void Worker::process() {
//	app_->exec();
	io_context_->run();
}

Request Client::parse(std::string req_data) {
	Request request;
	std::vector<std::string> request_arr;

//	for testing
//	boost::split(request_arr, req_data, [](char c) { return c == ':'; });

//  for deploy
	size_t pos = 0;
	std::string token;
	while ((pos = req_data.find("\r\n")) != std::string::npos) {
		token = req_data.substr(0, pos);
		request_arr.push_back(token);
		req_data.erase(0, pos + 2);
	}


	request.method = request_arr[0];

	if (request.method == "settings") {
		game_id_ = std::stoi(request_arr[1]);
		team_id_ = std::stoi(request_arr[2]);

		for (int i = 3; i < request_arr.size(); ++i) {
			team_players.push_back(request_arr[i]);
		}
		host_login = team_players[0];
		if (num_teams_>1){
			for(int i=1; i<=num_teams_; ++i){
				leaderboard_.leaders.push_back(Leader("Team "+std::to_string(i), 0, false));
			}
		}
		else {
			for(int i=0; i<team_players.size(); ++i){
				leaderboard_.leaders.push_back(Leader(team_players[i], 0, (i==0)));
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

		if(num_teams_>1){
			leaderboard_.leaders[std::stoi(request.parameters["team_id"])-1].points += host_pts;
		}
		else {
			int user_pts = std::stoi(request_arr[4]);
			for(int i=0; i<leaderboard_.size; ++i){
				if(leaderboard_.leaders[i].name == request.parameters["user_login"]){
					leaderboard_.leaders[i].points += user_pts;
				}
				if(leaderboard_.leaders[i].host){
					leaderboard_.leaders[i].points += host_pts;
				}
			}
		}

	}
	if (request.method == "round") {
		host_login = request_arr[1];
		if(num_teams_==1){
			for(int i=0; i<leaderboard_.size; ++i){
				leaderboard_.leaders[i].host = (leaderboard_.leaders[i].name == host_login);
			}
		}
	}
	if (request.method == "auth") {
		request.parameters["status"] = request_arr[1];
	}

	return request;
}


Client::Client(boost::asio::io_context &io_context, const std::string &server_, const std::string &port_, QThread* thread) :
		resolver_(io_context), socket_(io_context), server(server_), port(port_), thread_(thread) {
	w = new MainWindow(this);
	w->show();
	// Start an asynchronous resolve to translate the server and service names
	// into a list of endpoints.
	resolver_.async_resolve(server, port,
	                        boost::bind(&Client::handle_resolve, this,
	                                    boost::asio::placeholders::error,
	                                    boost::asio::placeholders::results));
}


std::string Client::serialize_auth(std::string user_login) {
	return "auth\r\n" + user_login + "\r\n";
}

std::string Client::serialize_settings(GameConfig settings) {
	return "settings\r\n" + user_login_ + "\r\n" + settings.GetSettings();
}

std::string Client::serialize_msg(Message msg) {
        return "msg\r\n" + user_login_ + "\r\n" + std::to_string(game_id_) + "\r\n" + std::to_string(team_id_) + "\r\n" + msg.msg + "\r\n" + (user_login_ == host_login ? "host" : "not_host") + "\r\n";
}

std::string Client::serialize_round() {
        return "round\r\n" + std::to_string(game_id_) + "\r\n";
}

void Client::send_auth(std::string user_login) {
	user_login_ = user_login;
	std::string str = serialize_auth(user_login);
	boost::asio::async_write(socket_, boost::asio::buffer(str.data(), str.size()),
	                         boost::bind(&Client::handle_multiwrite, this,
	                                     boost::asio::placeholders::error));
}

void Client::send_settings(GameConfig settings, int num_teams, int round_duration_) {
	num_teams_ = num_teams;
	round_duration = round_duration_;

	std::string str = serialize_settings(settings);
	boost::asio::async_write(socket_, boost::asio::buffer(str.data(), str.size()),
	                         boost::bind(&Client::handle_multiwrite, this,
	                                     boost::asio::placeholders::error));
}

void Client::send_msg(std::string text) {
	Message msg(user_login_, true, text);
	std::string str = serialize_msg(msg);
	boost::asio::async_write(socket_, boost::asio::buffer(str.data(), str.size()),
	                         boost::bind(&Client::handle_multiwrite, this,
	                                     boost::asio::placeholders::error));
}

void Client::send_round() {
	std::string str = serialize_round();
	boost::asio::async_write(socket_, boost::asio::buffer(str.data(), str.size()),
	                         boost::bind(&Client::handle_multiwrite, this,
	                                     boost::asio::placeholders::error));
}

void Client::handle_resolve(const boost::system::error_code &err, const tcp::resolver::results_type &endpoints) {
	if (!err) {
		// Attempt a Client to each endpoint in the list until we
		// successfully establish a Client.
		boost::asio::async_connect(socket_, endpoints,
		                           boost::bind(&Client::handle_write, this,
		                                       boost::asio::placeholders::error));
	}
	else {
		std::cout << "Error1: " << err.message() << "\n";
	}
}

void Client::handle_read(const boost::system::error_code &err) {
	if (!err) {
		//                вывод в консоль
		std::cout << response_buf_.data() << std::endl;

//                парсинг
		Request request = parse(std::string(response_buf_.data()));

//              очистка буфера
		response_buf_.fill('\0');

		if (request.method == "auth") {
			if (request.parameters["status"] == "ok") {
				//  Вызываем функцию ФЕДИ! (для открытия окна)
				w->next_window();
				handle_write(err);
			}
			else if (request.parameters["status"] == "already_online") {
				//  Вызываем функцию ФЕДИ! (для вывода ошибки)
				handle_write(err);
			}
		}

		if (request.method == "settings") {
			w->configWindow->next_window();
			w->configWindow->gameWindow->UpdateLeaderboard(leaderboard_);
//			w->configWindow->gameWindow->timeController->timer->moveToThread(thread_);
			handle_write(err);
		}

		if (request.method == "msg") {
			w->configWindow->gameWindow->UpdateMessages(Message(request.parameters["user_login"],
			                                                    (request.parameters["user_login"] == user_login_),
			                                                    request.parameters["text"]));
			handle_write(err);
		}

		if (request.method == "guess") {
			if(std::stoi(request.parameters["team_id"]) == team_id_) {
				w->configWindow->gameWindow->UpdateMessages(Message(request.parameters["user_login"],
				                                                    (request.parameters["user_login"] == user_login_),
				                                                    request.parameters["text"]));
			}
			w->configWindow->gameWindow->UpdateLeaderboard(leaderboard_);
			handle_write(err);
		}

		if (request.method == "round") {
			w->configWindow->gameWindow->UpdateLeaderboard(leaderboard_);
			w->configWindow->gameWindow->NewRound();
			handle_write(err);
		}

		if (request.method == "game_over") {

		}

		if (request.method == "keyword") {
			w->configWindow->gameWindow->UpdateKeyword(request.parameters["new_keyword"]);
			handle_write(err);
		}

		if (request.method == "warning") {
			w->configWindow->gameWindow->ShowWarning();
			handle_write(err);
		}
	}
	else {
		std::cout << "Error2: " << err.message() << "\n";
	}
}

void Client::handle_write(const boost::system::error_code &err) {
	if (!err) {
		// Read the response status line. The response_ streambuf will
		// automatically grow to accommodate the entire line. The growth may be
		// limited by passing a maximum size to the streambuf constructor.
		socket_.async_read_some(boost::asio::buffer(response_buf_),
		                        boost::bind(&Client::handle_read, this,
		                                    boost::asio::placeholders::error));
	}
	else {
		std::cout << "Error3: " << err.message() << "\n";
	}
}

void Client::handle_multiwrite(const boost::system::error_code &e) {
	if (!e) {
		return;
	}
	else {
		std::cout << "Multiple Writing  error!\n" << user_login_ << " closed app!\n";
	}
}

int main(int argc, char *argv[]) {
	try {
		if (argc != 3) {
			std::cout << "Usage: async_client <server> <port> <path>\n";
			std::cout << "Example:\n";
			std::cout << "  async_client www.boost.org /LICENSE_1_0.txt\n";
			return 1;
		}

		QApplication a(argc, argv);
		//std::cout << w.configWindow->gameWindow->per << std::endl;

		boost::asio::io_context io_context;

		QThread* thread = new QThread;
		Worker* worker = new Worker(io_context, &a);
		worker->moveToThread(thread);

		Client c(io_context, argv[1], argv[2], thread);

		QObject::connect(thread, SIGNAL(started()), worker, SLOT(process()));

		thread->start();

		a.exec();

	}
	catch (std::exception &e) {
		std::cout << "Exception: " << e.what() << "\n";
	}

	return 0;
}
