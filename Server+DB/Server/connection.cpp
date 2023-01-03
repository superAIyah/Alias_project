#include "connection.h"

#include <utility>
#include <vector>
#include <boost/bind/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

#include <string>
#include <regex>

#define WORD_PACK_SIZE 10

std::string serialize_settings(Request request_) {
	std::stringstream ss;
	ss << "settings\r\n" << request_.parameters.at("level") << "\r\n" << request_.parameters.at("num_players")
	   << "\r\n" << request_.parameters.at("num_teams") << "\r\n" << request_.parameters["round_duration"] << "\r\n\r\n";
	return ss.str();
}

Connection::Connection(boost::asio::io_context &io_context) : strand_(boost::asio::make_strand(io_context)), socket_(strand_) {}

tcp::socket &Connection::socket() { return socket_; }

void Connection::start() {
	read();
}

void Connection::write(tcp::socket& to_socket, std::string response, bool callback) {
	boost::asio::async_write(to_socket, boost::asio::buffer(response.data(), response.size()),
	                         boost::bind(&Connection::handle_write, shared_from_this(), callback,
	                                     boost::asio::placeholders::error));
}

void Connection::read(){
	boost::asio::async_read_until(socket_, buffer_, "\r\n\r\n",
	                              boost::bind(&Connection::handle_read, shared_from_this(), boost::asio::placeholders::error,
	                                          boost::asio::placeholders::bytes_transferred));
}

void Connection::handle_read(const boost::system::error_code &e, std::size_t bytes_transferred) {
	if (!e) {
		//парсинг
		Request request;
		request.parse_request(buffer_);

		if (request.method == "auth")
			authorization(request);
		else if (request.method == "settings")
			settings(request, e);
		else if (request.method == "msg")
			msg(request, e);
		else if (request.method == "round")
			round(request, e);
	}
//	если пользователь вышел из приложения
	else if (e.message() == "End of file") {
		player_left();
	}
}

void Connection::handle_write(bool callback, const boost::system::error_code &e) {
	if (!e) {
		if (callback) {
			try {
				read();
			} catch (...) {
				std::cout << "error caught!";
			}
		}
		else return;
	}
	else {
		std::cout << "Multiple Writing  error!\n" << user_login_ << " closed app!\n";

		if (!user_login_.empty()) {
			int game_id = Server->players_online[user_login_].first;
//		если пользователь был в игре
			if (game_id != -1) {
				int team_id = Server->players_online[user_login_].second;
				unsigned int num_players_in_team = Server->Games[game_id].team_sockets[team_id].size();
//			удаляем его из игры
				for (int i = 0; i < num_players_in_team; ++i) {
					if (Server->Games[game_id].team_sockets[team_id][i].second == user_login_) {
						Server->Games[game_id].team_sockets[team_id].erase(Server->Games[game_id].team_sockets[team_id].begin() + i);
						break;
					}
				}
//			уменьшаем количество игроков в игре
				Server->Games[game_id].num_players -= 1;
			}
			Server->players_online.erase(user_login_);
		}
	}
}

void Connection::player_left(){
	std::cout << user_login_ << " closed app\n\n";
	if (!user_login_.empty()) {
		int game_id = Server->players_online[user_login_].first;
//		если пользователь был в игре
		if (game_id != -1) {
			int team_id = Server->players_online[user_login_].second;
			unsigned int num_players_in_team = Server->Games[game_id].team_sockets[team_id].size();
//			удаляем его из игры
			for (int i = 0; i < num_players_in_team; ++i) {
				if (Server->Games[game_id].team_sockets[team_id][i].second == user_login_) {
					Server->Games[game_id].team_sockets[team_id].erase(Server->Games[game_id].team_sockets[team_id].begin() + i);
					break;
				}
			}
//			уменьшаем количество игроков в игре
			Server->Games[game_id].num_players -= 1;
		}
		Server->players_online.erase(user_login_);
	}
}

void Connection::authorization(Request request) {
	std::string user_login = request.parameters["user_login"];
	std::string password = request.parameters["password"];
	//если пользователь есть в БД
	int in_DB = Server->UDBM->has_user(user_login, password);
	if (in_DB == USER_FOUND) {
		//если пользователь уже онлайн
		if (Server->players_online.contains(user_login))
			auth_already_online();
			//если пользователь не онлайн
		else
			auth_not_online(user_login);
	}
	else if (in_DB == USER_NOT_FOUND) {
		auth_new_client(user_login, password);
	}
	else if (in_DB == WRONG_PASSWORD) {
		auth_wrong_pwd();
	}
}

void Connection::auth_already_online() {
	std::stringstream ss;
	ss << "auth\r\n";
	ss << "auth_already_online\r\n\r\n";
	std::string buffer = ss.str();
	write(socket_, buffer);
}

void Connection::auth_wrong_pwd() {
	std::stringstream ss;
	ss << "auth\r\n";
	ss << "wrong_password\r\n\r\n";
	std::string buffer = ss.str();
	write(socket_, buffer);
}

void Connection::auth_not_online(const std::string &user_login) {
	//запоминаем логин
	user_login_ = user_login;
	//добавляем в онлайн
	Server->players_online[user_login] = {-1, -1};
	std::stringstream ss;
	ss << "auth\r\n";
	ss << "ok\r\n\r\n";
	std::string buffer = ss.str();
	write(socket_, buffer);

	update_stats(socket_, user_login_, true);
}

void Connection::auth_new_client(const std::string &user_login, std::string password) {
	//запоминаем логин
	user_login_ = user_login;
	//добавляем в БД
	Server->UDBM->add_user(User(user_login, "", std::move(password)));
	//добавляем в онлайн
	Server->players_online[user_login] = {-1, -1};

	std::stringstream ss;
	ss << "auth\r\n";
	ss << "ok\r\n\r\n";
	std::string buffer = ss.str();
	write(socket_, buffer);

	update_stats(socket_, user_login_, true);
}

void Connection::update_stats(tcp::socket &sock, std::string user_login, bool last) {
	std::stringstream ss;

	UserInfo info = Server->UDBM->GetInfo(std::move(user_login));
	ss << "user_info\r\n" << info.num_of_wins << "\r\n" << info.num_of_losses << "\r\n" << info.player_rating << "\r\n\r\n";
	std::string buffer = ss.str();
	write(sock, buffer, last);
}

void Connection::settings(Request request, const boost::system::error_code &e) {
	//слепляет в строку ключ
	std::string settings_str = serialize_settings(request);
	//сокеты и логины
	std::vector<std::pair<tcp::socket *, std::string>> room;
	int num_teams = std::stoi(request.parameters.at("num_teams"));
	int num_players = std::stoi(request.parameters.at("num_players"));
	//количество игроков нужное для начала игры
	int num_players_needed = num_players * num_teams;
	//добавляет в линию ожидания
	std::vector<std::pair<tcp::socket *, std::string>> *waiting_players_ptr = &(Server->WaitingLine[settings_str].players);

	if (std::find(waiting_players_ptr->begin(), waiting_players_ptr->end(), std::make_pair(&socket_, request.parameters.at("user_login"))) ==
	    waiting_players_ptr->end())
		waiting_players_ptr->emplace_back(&socket_, request.parameters.at("user_login"));

	//если этот клиент пришел последним, и теперь в комнате достаточно человек
	if (Server->WaitingLine.at(settings_str).players.size() == num_players_needed) {
		//заполняем комнату
		for (int i = 0; i < num_players_needed; ++i) {
			if (Server->players_online.contains((*waiting_players_ptr)[i].second))
				room.push_back((*waiting_players_ptr)[i]);
			else {
				waiting_players_ptr->erase(waiting_players_ptr->begin() + i);
				return handle_write(true, e);
			}
		}

		//id игры
		int new_game_id;

		//последний+1
		if (!Server->Games.empty())
			new_game_id = Server->Games.rbegin()->first + 1;
		else { new_game_id = 0; }

		//добавляет игроков в новую игру
		for (int i = 0; i < num_players_needed; i++) {
			int team_id = i % num_teams + 1;
			Server->Games[new_game_id].team_sockets[team_id].push_back(room[i]);
			//запись id игры и команды в players_online
			Server->players_online[room[i].second] = {new_game_id, team_id};
		}
		if (num_teams == 1)
			create_logins_board(Server->Games[new_game_id].team_sockets[1], new_game_id);
		else
			create_teams_board(num_teams, new_game_id);

		//добавление слов
		for (int i = 1; i <= num_teams; ++i) {
			//добавляет слова
			std::vector<Word> new_words = Server->WDBM->get_words(std::stoi(request.parameters.at("level")),
			                                                      WORD_PACK_SIZE);
			for (const auto &elem: new_words)
				Server->Games[new_game_id].team_words[i].push(elem);

			//текущее слово которое отгадывает команда
			Server->Games[new_game_id].cur_words[i] = Server->Games[new_game_id].team_words[i].front();
		}

		//количество игроков в комнате
		Server->Games[new_game_id].num_players = num_players_needed;
		//количество раундов равно количеству игроков в команде, чтобы все игроки побывали ведущими
		Server->Games[new_game_id].rounds_remaining = num_players - 1;

		//задали ведущих
		for (int i = 1; i <= num_teams; ++i)
			Server->Games[new_game_id].hosts[i] = Server->Games[new_game_id].team_sockets[i][0];

		//отправка сообщений
		//для всех игроков в комнате
		for (int i = 0; i != room.size(); i++) {
			//команда игрока
			int line_place = i;
			int team_id = line_place % num_teams + 1;

			//сообщение-ответ
			std::stringstream ss;
			ss << "settings\r\n";
			ss << new_game_id << "\r\n" << team_id << "\r\n";
			if (num_teams == 1) {
				for (int j = 0; j < num_players_needed; ++j)
					ss << room[j].second << "\r\n";
				ss << "\r\n";
			}
			else {
				for (int j = 0; j < num_players; ++j)
					ss << Server->Games[new_game_id].team_sockets[team_id][j].second << "\r\n";
				ss << "\r\n";
			}

			std::string buffer = ss.str();
			write(*(room[i].first), buffer, (i == room.size() - 1));
		}

		//убирает игроков из линии ожидания
		for (int i = 0; i < num_players_needed; ++i)
			Server->WaitingLine.at(settings_str).players.erase(Server->WaitingLine.at(settings_str).players.begin());

		//отправка нового слова ведущему
		for (int i = 1; i <= Server->Games[new_game_id].team_sockets.size(); ++i)
			send_kw_to_host(new_game_id, i);

		//длительность раунда в сек
		int round_duration = std::stoi(request.parameters["round_duration"]);
		Server->Games[new_game_id].round_duration = round_duration;

		//обнуление счетчика для синхронизации раунда
		Server->Games[new_game_id].clients_responded = 0;
		//обновление таймера
		Server->Games[new_game_id].round_end = time(nullptr) + round_duration;
	}
	else
		handle_write(true, e);
}

void Connection::create_teams_board(int num_teams, int game_id) {
	for (int i = 1; i <= num_teams; i++)
		Server->Games[game_id].leader_board[std::to_string(i)] = 0;
}

void Connection::create_logins_board(const std::vector<std::pair<tcp::socket *, std::string>> &team_sockets, int game_id) {
	for (const auto& socket_login : team_sockets)
		Server->Games[game_id].leader_board[socket_login.second] = 0;
}

void Connection::msg(Request request, const boost::system::error_code &e) {
	int game_id = std::stoi(request.parameters.at("game_id"));
	int team_id = std::stoi(request.parameters.at("team_id"));
	std::string text = request.parameters.at("text");
	std::string login = request.parameters.at("user_login");
	std::string word = Server->Games[game_id].team_words[team_id].front().word;
	std::string stem = Server->Games[game_id].team_words[team_id].front().stem;

	//если время раунда еще не закончилось, отправляем сообщение
	if (time(nullptr) < Server->Games[game_id].round_end) {
		//если сообщение от ведущего
		if (request.parameters.at("who") == "host") {
			struct information info = {game_id, team_id, login, text, stem};
			msg_from_host(info, e);
		}
			//если игрок - не ведущий
		else {
			struct information info = {game_id, team_id, login, text, word};
			msg_from_player(info, e);
		}
	}
		//если время раунда закончилось, ничего не отправляем
	else { handle_write(true, e); }
}

std::string Connection::text_msg(const information& info) {
	std::stringstream ss;
	ss << "msg\r\n" << info.login << "\r\n" << info.text << "\r\n\r\n";
	return ss.str();
}

void Connection::msg_from_host(const information& info, const boost::system::error_code &e) {
	//есть ли слово в сообщении
	bool has_stem = info.text.find(info.str) != std::string::npos;
	//если есть
	if (has_stem) {
		std::vector<std::pair<tcp::socket *, std::string>> players = Server->Games[info.game_id].team_sockets[info.team_id];
		std::string buffer = "warning\r\nNo spoilers!!!\r\n\r\n";
		write(socket_, buffer);
	}
		//если слова нет
	else {
		//msg:user_login:text
		std::vector<std::pair<tcp::socket *, std::string>> players = Server->Games[info.game_id].team_sockets[info.team_id];
		//сообщение-ответ
		std::string buffer = text_msg(info);
		//отправка ответа всем клиентам в команде
		for (size_t i = 0; i < players.size(); i++) {
			write(*(players[i].first), buffer, (i == players.size() - 1));
		}
	}
}

void Connection::msg_from_player(const information& info, const boost::system::error_code &e) {
	//есть ли слово в сообщении
	bool has_word = info.text.find(info.str) != std::string::npos;
	//если есть
	if (has_word) {
		//если отгадали, всем:
		//guess:user_login:team_id:text:user_pts:host_pts
		//затем ведущему:
		//keyword:new_keyword

		//сообщение-ответ
		std::string buffer = guess_msg(info);
		std::map<int, std::vector<std::pair<tcp::socket *, std::string>>> teams = Server->Games[info.game_id].team_sockets;
		//отправка всем игрокам в комнате
		for (int i = 1; i <= teams.size(); i++) {
			for (const auto& player : teams[i]) {
				write(*(player.first), buffer, false);
			}
		}
		//отправка нового слова ведущему
		send_kw_to_host(info.game_id, info.team_id);
	}
	else {
		//msg:user_login:text
		std::vector<std::pair<tcp::socket *, std::string>> players = Server->Games[info.game_id].team_sockets[info.team_id];
		//сообщение-ответ
		std::string buffer = text_msg(info);
		//отправка всем игрокам в команде
		for (size_t i = 0; i < players.size(); i++) {
			write(*(players[i].first), buffer, (i == players.size() - 1));
		}
	}
}

std::string Connection::guess_msg(const information& info) {
	if (Server->Games[info.game_id].team_sockets.size() == 1) {
		Server->Games[info.game_id].leader_board[info.login] += std::stoi(USER_GUESS_POINTS);
		Server->Games[info.game_id].leader_board[Server->Games[info.game_id].hosts[info.team_id].second] += std::stoi(HOST_GUESS_POINTS);
	}
	else
		Server->Games[info.game_id].leader_board[std::to_string(info.team_id)] += std::stoi(HOST_GUESS_POINTS);
	std::stringstream ss;
	ss << "guess\r\n"
	   << info.login << "\r\n" << info.team_id << "\r\n" << info.text << "\r\n" << USER_GUESS_POINTS << "\r\n"
	   << HOST_GUESS_POINTS << "\r\n\r\n";
	return ss.str();
}

void Connection::send_kw_to_host(int game_id_, int team_id_) {
	Server->Games[game_id_].team_words[team_id_].pop();
	std::stringstream ss;
	ss << "keyword\r\n";
	ss << Server->Games[game_id_].team_words[team_id_].front().word;
	ss << "\r\n\r\n";
	std::string buffer = ss.str();

	write(*(Server->Games[game_id_].hosts[team_id_].first), buffer);
}

std::string round_msg(const std::string& host_login) {
	std::stringstream ss;
	ss << "round\r\n";
	ss << host_login;
	ss << "\r\n\r\n";
	return ss.str();
}

void Connection::round(Request request, const boost::system::error_code &e) {
	int game_id = std::stoi(request.parameters["game_id"]);
	//заполнения счетчика клиентов, отчитавшихся о завершении раунда
	Server->Games[game_id].clients_responded += 1;

	//если данный клиент был последним, кто отчитался, и теперь отчитались все
	if (Server->Games[game_id].clients_responded == Server->Games[game_id].num_players) {
		//если раунды еще остались, игра не закончена
		if (Server->Games[game_id].rounds_remaining != 0) {
			update_hosts(game_id);

			//для всех команд
			for (int i = 1; i <= Server->Games[game_id].team_sockets.size(); ++i) {
				//сокеты всех игроков в команде
				std::vector<tcp::socket *> sockets2send;
				for (const auto& player : Server->Games[game_id].team_sockets[i])
					sockets2send.push_back(player.first);

				//сообщение-ответ
				std::string buffer = round_msg(Server->Games[game_id].hosts[i].second);

				//отправка ответа
				for (auto sock: sockets2send) {
					write(*sock, buffer, false);
				}
			}

			//длительность раунда в сек
			int round_duration = Server->Games[game_id].round_duration;

			//обновление таймера
			Server->Games[game_id].round_end = time(nullptr) + round_duration;

			//обнуление счетчика для синхронизации раунда
			Server->Games[game_id].clients_responded = 0;

			//отправка нового слова ведущему
			for (int i = 1; i <= Server->Games[game_id].team_sockets.size(); ++i)
				send_kw_to_host(game_id, i);
		}
			//игра закончена
		else {
			sendDB(game_id);
			//сообщение-ответ
			std::stringstream ss;
			ss << "gameover\r\n\r\n";
			std::string buffer = ss.str();

			//сокеты всех игроков в игре
			std::vector<tcp::socket *> sockets2send;
			for (int i = 1; i <= Server->Games[game_id].team_sockets.size(); ++i) {
				for (int j = 0; j < Server->Games[game_id].team_sockets[i].size(); ++j) {
//					sockets2send.push_back(Server->Games[game_id].team_sockets[i][j].first);
					tcp::socket *sock = Server->Games[game_id].team_sockets[i][j].first;
					std::string login = Server->Games[game_id].team_sockets[i][j].second;
					//обнуляем game_id & team_id для игрока
					Server->players_online[login] = {-1, -1};
					write(*sock, buffer, false);
					update_stats(*sock, login,
					             (i == Server->Games[game_id].team_sockets.size() && j == Server->Games[game_id].team_sockets[i].size() - 1));
				}
			}
			//удаляет комнату
			Server->Games.erase(game_id);
		}
	}
	else {
		handle_write(true, e);
	}
}

void Connection::update_hosts(int game_id) {
	Server->Games[game_id].rounds_remaining -= 1;
	//обновили ведущих
	for (int i = 1; i <= Server->Games[game_id].team_sockets.size(); ++i) {
		for (int j = 0; j < Server->Games[game_id].team_sockets[i].size(); ++j) {
			if (Server->Games[game_id].team_sockets[i][j].second == Server->Games[game_id].hosts[i].second) {
				if (j == Server->Games[game_id].team_sockets[i].size() - 1)
					Server->Games[game_id].hosts[i] = Server->Games[game_id].team_sockets[i][0];
				else
					Server->Games[game_id].hosts[i] = Server->Games[game_id].team_sockets[i][j + 1];
				break;
			}
		}
	}
}

bool cmp(std::pair<std::string, int> &a, std::pair<std::string, int> &b) {
	return a.second >= b.second;
}

void Connection::sendDB(int game_id) {
	std::map<std::string, int> leader_board = Server->Games[game_id].leader_board;
	std::vector<std::pair<std::string, int> > leader_board_vec;
	for (auto &it: leader_board)
		leader_board_vec.emplace_back(it);
	sort(leader_board_vec.begin(), leader_board_vec.end(), cmp);
	int score = leader_board_vec.begin()->second;
	if (Server->Games[game_id].team_sockets.size() == 1)
		for (const auto& i: leader_board_vec) {
			if (score == i.second)
				Server->UDBM->UpdateUser(i.first, true);
			else
				Server->UDBM->UpdateUser(i.first, false);
		}
	else
		for (const auto& i: leader_board_vec) {
			int team_id = std::stoi(i.first);
			std::vector<std::pair<tcp::socket *, std::string>> team_sockets = Server->Games[game_id].team_sockets[team_id];
			for (const auto& j: team_sockets)
				if (score == i.second)
					Server->UDBM->UpdateUser(j.second, true);
				else
					Server->UDBM->UpdateUser(j.second, false);
		}
}
