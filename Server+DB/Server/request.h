#ifndef BOOST_ASIO_SERVER_REQUEST_H
#define BOOST_ASIO_SERVER_REQUEST_H
#include <unordered_map>
#include <iostream>

struct Request
{
    std::string method;
    std::unordered_map<std::string, std::string> parameters;

	void parse_request(boost::asio::streambuf &req_data) {
		std::vector<std::string> request_arr;

		std::istream is(&req_data);
		std::string token;

		while (true) {
			std::getline(is, token, '\r');
			if (!token.empty()) {
				request_arr.push_back(token);
				std::cout<<token<<'\n';
				std::getline(is, token);
			}
			else {
				std::getline(is, token);
				std::cout<<'\n';
				break;
			}
		}

		method = request_arr[0];

		if (method == "settings") {
			parameters["user_login"] = request_arr[1];
			parameters["level"] = request_arr[2];
			parameters["num_players"] = request_arr[3];
			parameters["num_teams"] = request_arr[4];
			parameters["round_duration"] = request_arr[5];
		}
		if (method == "msg") {
			parameters["user_login"] = request_arr[1];
			parameters["game_id"] = request_arr[2];
			parameters["team_id"] = request_arr[3];
			parameters["text"] = request_arr[4];
			parameters["who"] = request_arr[5];//ведущий или отгадывающий
		}
		if (method == "round") {
			parameters["game_id"] = request_arr[1];
		}
		if (method == "auth") {
			parameters["user_login"] = request_arr[1];
			parameters["password"] = request_arr[2];
		}
	}
};

#endif //BOOST_ASIO_SERVER_REQUEST_H
