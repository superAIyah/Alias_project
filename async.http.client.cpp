#include "async.http.client.h"

using boost::asio::ip::tcp;

Request parse(std::string req_data) {
	Request request;
	std::vector<std::string> request_arr;

//	for testing
//	boost::split(request_arr, req_data, [](char c) { return c == ':'; });

//  for deploy
//	std::regex rx("[^\\s]+\r\n");
//	std::sregex_iterator re_it(req_data.begin(), req_data.end(), rx), rxend;
//
//	while(re_it != rxend)
//	{
//		request_arr.push_back(re_it->str());
//		++re_it;
//	}

	size_t pos = 0;
	std::string token;
	while ((pos = req_data.find("\r\n")) != std::string::npos) {
		token = req_data.substr(0, pos);
//		std::cout << token << std::endl;
		request_arr.push_back(token);
		req_data.erase(0, pos + 2);
	}
	request_arr.push_back(req_data);


	request.method = request_arr[0];

	if (request.method == "settings") {
		request.parameters["user_login"] = request_arr[1];
		request.parameters["level"] = request_arr[2];
		request.parameters["num_players"] = request_arr[3];
		request.parameters["num_teams"] = request_arr[4];
		request.parameters["round_duration"] = request_arr[5];
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
		request.parameters["user_pts"] = request_arr[4];
		request.parameters["host_pts"] = request_arr[5];
	}
	if (request.method == "round") {
		request.parameters["game_id"] = request_arr[1];
	}
	if (request.method == "auth") {
		request.parameters["status"] = request_arr[1];
	}

	return request;
}


Client::Client(boost::asio::io_context &io_context, const std::string &server_, const std::string &port_) :
							resolver_ (io_context), socket_(io_context), server(server_), port(port_) {
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
	return "msg\r\n" + user_login_ + "\r\n" + game_id_ + "\r\n" + team_id_ + "\r\n" + msg.msg + "\r\n" + (host_ ? "host" : "not_host");
}

std::string Client::serialize_round() {
	return "round\r\n" + game_id_;
}

void Client::send_auth(std::string user_login) {
	std::string str = serialize_auth(user_login);
	boost::asio::async_write(socket_, boost::asio::buffer(str.data(), str.size()),
	                         boost::bind(&Client::handle_multiwrite, this,
	                                     boost::asio::placeholders::error));
}

void Client::send_settings(GameConfig settings) {
	std::string str = serialize_settings(settings);
	boost::asio::async_write(socket_, boost::asio::buffer(str.data(), str.size()),
	                         boost::bind(&Client::handle_multiwrite, this,
	                                     boost::asio::placeholders::error));
}

void Client::send_msg(Message msg) {
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

void Client::handle_resolve(const boost::system::error_code &err,
                    const tcp::resolver::results_type &endpoints) {
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
			}
			else if (request.parameters["status"] == "already_online") {
				//  Вызываем функцию ФЕДИ! (для вывода ошибки)
			}
		}

		if (request.method == "settings") {

		}

		if (request.method == "msg") {

		}

		if (request.method == "guess") {

		}

		if (request.method == "round") {

		}

		if (request.method == "game_over") {

		}

		if (request.method == "keyword") {

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

void Client::run() {
	boost::shared_ptr<std::thread> newthread(new std::thread(
			boost::bind(&boost::asio::io_context::run, &io_context_)));
	threads.push_back(newthread);

	threads[0]->join();

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

		Client c(io_context, argv[1], argv[2]);

		boost::shared_ptr<std::thread> newthread(new std::thread(
				boost::bind(&boost::asio::io_context::run, &io_context)));
		std::vector<boost::shared_ptr<std::thread>> threads;

		threads.push_back(newthread);

		a.exec();

		threads[0]->join();
//		c.run();


	}
	catch (std::exception &e) {
		std::cout << "Exception: " << e.what() << "\n";
	}

	return 0;
}